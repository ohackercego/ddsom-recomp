// SPDX-License-Identifier: GPL-3.0-only
// Copyright (C) 2026 Tommy McLeroy. Part of cps2recomp.
//
// The fighting modes of a fighting game's "Alternative modes" page (battle.h): survival, the endless battle and
// the ring out, on what a game's sources describe. First written for Vampire Savior, then Street Fighter Alpha 2.
#include "battle.h"

#include "spare.h"
#include "state.h"

#include <algorithm>
#include <array>
#include <cstdio>
#include <cstring>
#include <random>
#include <string>

namespace cps2recomp {

namespace {
    battle::Description d; // the game's (battle::Register)

    constexpr const char *kModes = "Alternative modes", *kSurvival = "Survival", *kEndless = "Endless battle";
    constexpr const char *kRingOut = "Ring out";

    constexpr int kStartMost = 9000;        // frames after which getting to a round is given up
    constexpr int kAfter = 230;             // frames a decided round is left to play out before the next match
    constexpr int kFade = 24;               //   the last of which, and the next match's first, go to and from black
    constexpr int kIdle = 600;              // frames without a press after which the game takes a fighter back
    constexpr int kOnTheWay = 4;            // frames a frame that the next match's machine runs until its round is on
    constexpr int kOverFor = 480;           // frames the count is shown for when survival is over
    // The endless battle's stage: one stage; or the fighters drawn over a stage that another machine shows,
    // which can then change under them: by fading into another, or by scrolling on into the next
    enum { kStageOne, kStageShuffle, kStageScroll, kStageBehind }; // (the last: behind the fighters, and never changing by itself)
    // The ring out
    constexpr int kFallFor = 56;             // frames a fighter that is out is seen flung out and falling for
    // The way down: the winner jumps off the old stage, and at the top of its jump goes on in the next match (its
    // machine's camera held a picture's height above that stage, the old stage's picture above it), the camera
    // following it down to the next stage's floor, onto which it lands
    constexpr int kDropReady = 2, kDropReadyFalling = 4; // frames a frame the next match's machines run on their way to
                                                         // its round, while the fight goes on and while the one out falls
    constexpr int kDropJump = 8;             // the frame of the way down at which the winner jumps off the old stage
    constexpr int kDropInside = 48;          // the winner is kept at least this far inside the picture's edges
    constexpr int kDropFollow = 4;           // the camera goes a quarter of the way to where it follows the winner a frame
    constexpr int kDropShown = 3;            // the frame of it from which the next match's fighters are drawn
    constexpr int kDropMost = 400;           // frames after which the way down is over, landed or not
    constexpr int kShuffleMinutes[] = {1, 2, 3, 5, 10}; // of fighting on a stage before it fades into another (the menu's choice)
    constexpr int kBlend = 150;                 //   and frames the fade takes
    constexpr int32_t kScrollSpeeds[] = {64, 128, 256}; // the scrolling stage's speed, in 256ths of a pixel a frame: slow, medium, fast
    constexpr size_t kSoundAhead = 16;          // samples of another machine's sound kept in hand
    constexpr uint16_t kAnyButton = kLeft | kRight | kUp | kDown | kPunch1 | kPunch2 | kPunch3 | kKick1 | kKick2 | kKick3;

    uint32_t Player(int n) {
        return d.player1 + static_cast<uint32_t>(n) * d.playerSize;
    }
    int CameraX() {
        return static_cast<int16_t>(Ram16(d.cameraX));
    }
    int CameraY() {
        return static_cast<int16_t>(Ram16(d.cameraY));
    }
    MatchWanted &Wanted() {
        return d.wanted();
    }
    CameraWanted &CameraAt() {
        return d.cameraAt();
    }
    // The camera's ends and the picture's width, in 256ths of a pixel, for the picture being played in (a wide
    // picture's camera stops short of where the board's does)
    int32_t LeftEnd() {
        return (d.cameraLeft + d.wideMargin()) * 256;
    }
    int32_t RightEnd() {
        return (d.cameraRight - d.wideMargin()) * 256;
    }
    int PictureWidth() {
        return Video::kWidth + 2 * d.wideSide();
    }
    int32_t Across() {
        return PictureWidth() * 256;
    }
    int Arena() { // where the fighters' own camera is kept while the stage scrolls
        return (d.cameraLeft + d.cameraRight) / 2;
    }

    enum class Kind : int32_t { None, Survival, Endless, RingOut };
    enum class Phase : int32_t { Starting, Fighting, Decided, Over, Falling, Dropping };
    // (the menu's choices can be changed while the battle goes on: read each frame)
    int ShuffleAfter() {
        return kShuffleMinutes[std::clamp(FeatureValue("endless_shuffle"), 0, static_cast<int>(std::size(kShuffleMinutes)) - 1)] * 60 * 60;
    }
    int32_t ScrollBy() {
        return kScrollSpeeds[std::clamp(FeatureValue("endless_scroll"), 0, static_cast<int>(std::size(kScrollSpeeds)) - 1)];
    }

    struct State {
        Kind kind = Kind::None;
        Phase phase = Phase::Starting;
        int32_t frames = 0;              // on the way to the first round
        int32_t since = 0;               // frames in this phase
        int32_t stage = 0;               // 1 the first; 0: the game's choice
        int32_t characters[2] = {-1, -1};
        int32_t colours = -1;            // the player's character's colours, kept from match to match
        bool computer[2] = {false, false};
        int32_t idle[2] = {0, 0};
        int32_t score = 0;
        int32_t winner = 0;
        int32_t health = 0, ceiling = 0; // survival: what the player has left
        int32_t fadeIn = 0;              // frames of coming from black still to go
        uint32_t random = 1;
        // The stage behind the fighters, when another machine shows it
        int32_t way = kStageOne;
        int32_t back = 0, backNext = 0;  // its stage, and the one that comes after (0: none yet)
        int32_t backFor = 0;             // frames of fighting on it
        int32_t blend = 0;               // shuffle: how far the fade to the next has got (of kBlend)
        int32_t backCamera = 0;          // scroll: where its picture's left edge is, in 256ths of a pixel
        int32_t slide = 0;               //   and how far the next stage has come in
        // The ring out
        int32_t out = -1;                // who is out and falling (0, 1)
        int32_t outSide = 0;             //   over which end (-1 the left, 1 the right)
        int32_t dropStage = 0;           // the stage the next match is on
        bool flung = false;              // the one out has been flung out (player.cpp)
        bool handing = false;            // the winner is to be put in the next match at its next frame
        bool dropped = false;            //   and is in it
        bool landed = false;             //   and has landed there
        int32_t dropCameraX = 0, dropCameraY = 0; // where the next match's camera is (up and down: as its fighters stand)
        int32_t raised = 0;              // how far above that its camera is held (256ths of a pixel)
        int32_t raisedShown = 0;         //   and was for the picture its frame makes (that of the frame before)
        int32_t handedY = 0, dropY = 0;  // the winner's height in the next match as it was put there, and now
        int32_t flightX = 0, flightY = 0; // where it is put there (RingOutMove)
        std::array<uint8_t, battle::kMostRecord> flight = {}; // and its record as it was on the old stage
    } g;
    Spare g_dropBack;                    // the ring out: the machine of the next match's stage
    bool g_open = false;                 // the ring out: the port's machine is running a fight with open ends
    bool g_portFrame = false;            //   the port's machine is running (not a spare)
    bool g_dropIn = false;               //   the next match's machine is running, and the winner is to come down
    Spare g_next;
    Spare g_back, g_backNext;            // the machines of the stage behind and of the one after
    // A machine at the character choice with both players in (the first a spare came to): where every later match
    // and stage is begun, not from switching on (that took about 2,000 frames to a round, this a few hundred)
    std::vector<uint8_t> g_atChoice;
    uint32_t Random();
    // A spare begun again: from the character choice, once one has been there
    void Begin(Spare &spare) {
        spare.Clear();
        spare.state = g_atChoice;
    }
    // A spare's controls on its way to its round (Description::toRound); the first time one is at the character
    // choice, it is kept (before its frame) to begin the others from, and one begun from there is given a random
    // number of its own
    bool ToRound(Machine &machine, int32_t &frames) {
        if (frames == 0 && !g_atChoice.empty() && d.random) {
            SetRam16(d.random, static_cast<uint16_t>(Random()));
        }
        if (g_atChoice.empty() && d.choosing()) {
            Archive ar(g_atChoice, true);
            machine.Serialize(ar);
        }
        return d.toRound(machine, frames, false);
    }
    // Sound another machine has made and that has not been heard yet (two machines' frames do not make quite
    // the same number of samples)
    struct Stream {
        std::vector<int16_t> waiting;
        void Start() {
            waiting.assign(kSoundAhead * 2, 0);
        }
        void Add(const std::vector<int16_t> &sound) {
            waiting.insert(waiting.end(), sound.begin(), sound.end());
        }
        // Mixed into `sound` (as many samples as it has), `gain` of 256
        void Mix(std::vector<int16_t> &sound, int gain) {
            if (waiting.size() < sound.size()) {
                waiting.resize(sound.size(), 0);
            }
            for (size_t i = 0; i < sound.size(); i++) {
                sound[i] = static_cast<int16_t>(std::clamp(sound[i] + waiting[i] * gain / 256, -32768, 32767));
            }
            waiting.erase(waiting.begin(), waiting.begin() + static_cast<std::ptrdiff_t>(sound.size()));
        }
    } g_backSound, g_backNextSound, g_dropSound, g_dropBackSound;
    bool g_fast = false;
    size_t g_soundBefore = 0;

    void Fast(bool on) {
        if (on != g_fast) {
            g_fast = on;
            SetFastForward(on);
        }
    }

    void End() {
        if (g.kind == Kind::None) {
            return;
        }
        g = State();
        g_next.Clear();
        g_back.Clear();
        g_backNext.Clear();
        g_dropBack.Clear();
        g_atChoice.clear();
        g_open = false;
        Wanted() = MatchWanted();
        CameraAt() = CameraWanted();
        Fast(false);
        SetHud({});
    }

    uint32_t Random() {
        g.random ^= g.random << 13;
        g.random ^= g.random >> 17;
        g.random ^= g.random << 5;
        return g.random;
    }
    // A character at random: not `not1`, not `not2`
    int AnyCharacter(int not1 = -1, int not2 = -1) {
        for (;;) {
            const int c = static_cast<int>(Random() % static_cast<uint32_t>(d.characterSpan));
            if (d.choosable(c) && c != not1 && c != not2) {
                return c;
            }
        }
    }
    // A feature's choice of character: 0 at random, then the characters in the order of their numbers
    int ChosenCharacter(const char *feature) {
        int choice = FeatureValue(feature);
        if (choice <= 0) {
            return -1;
        }
        return d.rosterNumber(choice);
    }
    // The endless battle's stage: its feature's choices are two ways of changing it, then the stages
    int ChosenStage() {
        const int choice = FeatureValue("endless_stage");
        return choice >= 2 && choice < 2 + d.stages ? choice - 1 : static_cast<int>(Random() % static_cast<uint32_t>(d.stages)) + 1;
    }
    int AnyStage(int notThis) {
        for (;;) {
            const int stage = static_cast<int>(Random() % static_cast<uint32_t>(d.stages)) + 1;
            if (stage != notThis) {
                return stage;
            }
        }
    }
    bool Layered() {
        return g.way != kStageOne;
    }

    void Start(Machine &machine, Kind kind) {
        d.endModes();
        End();
        SetFeature("training_mode", false);
        g.kind = kind;
        g.phase = Phase::Starting;
        g.random = std::random_device()() | 1;
        if (kind != Kind::Endless) {
            g.characters[0] = -1; // (the player's choice)
            g.characters[1] = AnyCharacter();
            g.computer[1] = true;
            g.stage = 0;
            if (kind == Kind::RingOut) {
                g.way = kStageBehind;
                g.back = AnyStage(0);
                g.dropStage = AnyStage(g.back);
                g_backSound.Start();
            }
        } else {
            g.characters[0] = ChosenCharacter("endless_one");
            g.characters[1] = ChosenCharacter("endless_two");
            if (g.characters[0] < 0) {
                g.characters[0] = AnyCharacter(g.characters[1]);
            }
            if (g.characters[1] < 0) {
                g.characters[1] = AnyCharacter(g.characters[0]);
            }
            g.computer[0] = g.computer[1] = true;
            g.stage = ChosenStage();
            const int choice = FeatureValue("endless_stage");
            g.way = choice == 0 ? kStageShuffle : choice == 1 ? kStageScroll : kStageOne;
            if (Layered()) {
                g.back = g.stage;
                g.backCamera = LeftEnd();
                g_backSound.Start();
            }
        }
        machine.Reset();
    }
    void StartSurvival(Machine &machine) {
        Start(machine, Kind::Survival);
    }
    void StartEndless(Machine &machine) {
        Start(machine, Kind::Endless);
    }
    void StartRingOut(Machine &machine) {
        Start(machine, Kind::RingOut);
    }
    bool Chooses() {
        return g.kind != Kind::Endless; // (the player chooses a character)
    }

    void Want() {
        Wanted().stage = g.stage;
        Wanted().characters[0] = g.characters[0];
        Wanted().characters[1] = g.characters[1];
        Wanted().colours[0] = g.colours;
    }

    // Both players stay players that are in the game (Description::plays), or the panel would count down for the
    // one that is not to come back
    void BothPlay() {
        if (d.plays) {
            for (int n = 0; n < 2; n++) {
                SetRam8(Player(n) + d.plays, 2);
            }
        }
    }
    // Who the game plays, told to the game: the players' records say
    void SayWhoPlays(bool nobody = false) {
        for (int n = 0; n < 2; n++) {
            SetRam8(Player(n) + d.computer, g.computer[n] && !nobody ? 1 : 0);
        }
        BothPlay();
    }

    void BeforeFrame(Machine &machine) {
        g_soundBefore = machine.audio.size();
        Want();
        uint16_t &one = machine.players[0], &two = machine.players[1];
        if (g.phase == Phase::Starting) {
            // Survival: the player chooses a character, at the game's own pace; the rest goes by unseen
            const bool choosing = Chooses() && d.choosing();
            Fast(!choosing);
            if (d.toRound(machine, g.frames, Chooses()) && (!Layered() || g_back.ready)) {
                Fast(false);
                g.phase = Phase::Fighting;
                g.since = 0;
                g.fadeIn = kFade;
                g.characters[0] = Ram8(Player(0) + d.character);
                g.characters[1] = Ram8(Player(1) + d.character);
                if (Chooses() && d.colours) {
                    g.colours = Ram8(Player(0) + d.colours);
                }
                if (Layered()) {
                    d.playSound(d.tuneOff); // (the music is the stage's, from its own machine)
                }
                if (g.kind == Kind::RingOut) {
                    g.characters[1] = AnyCharacter(g.characters[1]); // (from here: who the next match is against)
                    Begin(g_next);
                    Begin(g_dropBack); // (the stage behind has been to the character choice by now)
                }
            } else if (g.frames > kStartMost && !choosing) {
                End();
            }
            return;
        }
        if (g.phase == Phase::Over) {
            one = two = 0;
            return;
        }
        if (g.phase == Phase::Falling || g.phase == Phase::Dropping) {
            // Nobody fights on; the winner jumps off, over the end the loser went over
            one = two = 0;
            if (g.phase == Phase::Dropping && g.since >= kDropJump && g.since < kDropJump + 4) {
                one = static_cast<uint16_t>(kUp | (g.outSide < 0 ? kLeft : kRight));
            }
            SayWhoPlays(true);
            return;
        }
        g_open = g.kind == Kind::RingOut;
        if (g.kind == Kind::Endless) {
            // A press takes a fighter over; none for a while gives it back
            const uint16_t held[2] = {one, two};
            for (int n = 0; n < 2; n++) {
                if (held[n] & kAnyButton) {
                    g.computer[n] = false;
                    g.idle[n] = 0;
                } else if (!g.computer[n] && ++g.idle[n] > kIdle) {
                    g.computer[n] = true;
                }
            }
        }
        // (a fighter the game plays hears nothing of the controls; nobody's Start or coin does anything)
        one = g.computer[0] ? 0 : one & kAnyButton;
        two = g.computer[1] ? 0 : two & kAnyButton;
        SayWhoPlays();
        if (g.way == kStageScroll) {
            // The floor moves under the fighters: they and what they have thrown are carried left with it,
            // and the picture's left edge (the fighters' camera stays where it is) pushes them along
            CameraAt().x = Arena();
            const uint32_t by = static_cast<uint32_t>(ScrollBy()) << 8;
            for (int n = 0; n < d.playerRecords; n++) {
                const uint32_t record = d.player1 + static_cast<uint32_t>(n) * d.playerSize;
                if (Ram8(record + d.active)) {
                    SetRam32(record + d.x, Ram32(record + d.x) - by);
                }
            }
            for (int n = 0; n < d.thrownRecords; n++) {
                const uint32_t record = d.thrown + static_cast<uint32_t>(n) * d.thrownSize;
                if (Ram8(record + d.active)) {
                    SetRam32(record + d.x, Ram32(record + d.x) - by);
                }
            }
        }
    }

    // --- the stage behind the fighters ---

    // One of the stage machines for a frame: on its way to its round, or showing its stage with the camera
    // where it is wanted
    void RunStage(Machine &machine, Spare &spare, int stage, int cameraX, int cameraY, Made *made) {
        RunSpare(machine, spare, [&](Machine &m) {
            Wanted().stage = stage;
            if (!spare.ready) {
                spare.ready = ToRound(m, spare.frames);
            }
            if (spare.ready) {
                m.players[0] = m.players[1] = 0;
                m.video.hideObject = d.notTheStage;
                CameraAt().x = cameraX;
                CameraAt().y = cameraY;
            }
        }, made);
    }

    // The fighters laid over a stage's picture: the pixels the fighters' machine drew an object at. `show` of
    // 256 is how much of them is seen. The player `falls` (-1: nobody), falling off the stage, is drawn behind
    // the other.
    void Over(std::vector<uint32_t> &stage, const std::vector<uint32_t> &fighters, int show, int falls = -1) {
        const uint32_t sets = falls >= 0 ? d.playerSets(falls) : 0;
        const uint32_t keep = static_cast<uint32_t>(256 - show), put = static_cast<uint32_t>(show);
        if (stage.size() != fighters.size()) {
            return; // (two pictures of different widths: a frame in which one was not yet the stage's)
        }
        const int width = static_cast<int>(fighters.size() / Video::kHeight);
        for (int pass = sets ? 0 : 1; pass < 2; pass++) { // (the one falling first: behind the other)
            for (int y = 0; y < Video::kHeight; y++) {
                for (int x = 0; x < width; x++) {
                    const uint32_t f = fighters[static_cast<size_t>(y) * width + static_cast<size_t>(x)];
                    if (!(f & 0x80000000)) {
                        continue;
                    }
                    const bool falling = (sets >> ((f >> 24) & 0x1F)) & 1;
                    if (falling != (pass == 0)) {
                        continue;
                    }
                    uint32_t &b = stage[static_cast<size_t>(y) * width + static_cast<size_t>(x)];
                    b = show >= 256 ? f & 0xFFFFFF :
                        ((((b & 0xFF00FF) * keep + (f & 0xFF00FF) * put) >> 8) & 0xFF00FF) | ((((b & 0x00FF00) * keep + (f & 0x00FF00) * put) >> 8) & 0x00FF00);
                }
            }
        }
    }
    // After the fighters' frame: the stage machines' frames, and the picture and sound put together. `show`
    // of 256 is how much of the fighters is seen (they fade when one match follows another).
    void Layer(Machine &machine, int show) {
        static Made fighters, back, next;
        Take(machine, g_soundBefore, fighters);
        const int cameraY = CameraY();
        const int cameraX = g.way == kStageScroll ? g.backCamera / 256 : CameraX();
        const bool fighting = g.phase != Phase::Starting;

        // The stage after this one: wanted when this one has been fought on long enough (shuffle), or as
        // soon as this one is the one shown (scroll)
        if (fighting && !g.backNext && (g.way == kStageScroll || (g.way == kStageShuffle && g.backFor >= ShuffleAfter()))) {
            g.backNext = AnyStage(g.back);
            Begin(g_backNext);
            g_backNextSound.Start();
        }
        RunStage(machine, g_back, g.back, cameraX, cameraY, &back);
        if (!g_back.ready) {
            RunStage(machine, g_back, g.back, cameraX, cameraY, &back); // (twice as fast as the fighters' machine on the way)
            Give(machine, fighters);
            return;
        }
        g_backSound.Add(back.sound);
        bool nextShown = false;
        if (g.backNext) {
            const bool wasReady = g_backNext.ready;
            // (on its way to its round: three frames a frame; when the stages scroll by, more if that is what
            // has it ready when the shown stage has been crossed, so that the scrolling does not wait for it. A
            // wide picture's stage is crossed sooner, and at the fast speed three were too few.)
            int onTheWay = 3;
            if (g.way == kStageScroll) {
                const int32_t framesLeft = std::max((RightEnd() - g.backCamera) / std::max(ScrollBy(), 1), 1);
                onTheWay = std::clamp((d.stageToRound - g_backNext.frames + framesLeft - 1) / framesLeft, 3, 6);
            }
            for (int n = 0; n < (wasReady ? 1 : onTheWay); n++) {
                RunStage(machine, g_backNext, g.backNext, g.way == kStageScroll ? LeftEnd() / 256 : cameraX, cameraY, &next);
                if (g_backNext.ready) {
                    break;
                }
            }
            if (wasReady) {
                g_backNextSound.Add(next.sound);
                nextShown = true;
            }
        }
        if (!fighting) {
            g_backSound.waiting.resize(kSoundAhead * 2); // (not heard yet)
            Give(machine, fighters);
            return;
        }
        g.backFor++;

        // The stage's picture: one stage, or two (faded, or side by side)
        int nextGain = 0;
        nextShown = nextShown && next.picture.size() == back.picture.size();
        if (nextShown && g.way == kStageShuffle) {
            g.blend = std::min(g.blend + 1, kBlend);
            nextGain = g.blend * 256 / kBlend;
            for (size_t i = 0; i < back.picture.size(); i++) {
                const uint32_t a = back.picture[i], b = next.picture[i];
                const uint32_t rb = (((a & 0xFF00FF) * static_cast<uint32_t>(256 - nextGain) + (b & 0xFF00FF) * static_cast<uint32_t>(nextGain)) >> 8) & 0xFF00FF;
                const uint32_t gr = (((a & 0x00FF00) * static_cast<uint32_t>(256 - nextGain) + (b & 0x00FF00) * static_cast<uint32_t>(nextGain)) >> 8) & 0x00FF00;
                back.picture[i] = rb | gr;
            }
        } else if (g.way == kStageScroll) {
            // (onwards: across this stage, then this stage out and the next in, if that one is ready)
            const int32_t by = ScrollBy();
            if (g.slide > 0 || g.backCamera + by >= RightEnd()) {
                g.backCamera = RightEnd();
                g.slide = nextShown ? std::min(g.slide + by, Across()) : 0;
            } else {
                g.backCamera += by;
            }
            const int width = static_cast<int>(back.picture.size() / Video::kHeight);
            const int slide = std::min(g.slide / 256, width);
            nextGain = slide * 256 / width;
            if (slide > 0) {
                for (int y = 0; y < Video::kHeight; y++) {
                    uint32_t *row = &back.picture[static_cast<size_t>(y) * width];
                    memmove(row, row + slide, static_cast<size_t>(width - slide) * sizeof(uint32_t));
                    memcpy(row + width - slide, &next.picture[static_cast<size_t>(y) * width], static_cast<size_t>(slide) * sizeof(uint32_t));
                }
            }
        }
        const bool falling = g.out >= 0 && (g.phase == Phase::Falling || g.phase == Phase::Dropping || g.phase == Phase::Over);
        Over(back.picture, fighters.picture, show, falling ? g.out : -1);
        fighters.picture = back.picture;
        // The sound: the fighters', and the stages' music, one fading into the other
        g_backSound.Mix(fighters.sound, 256 - nextGain);
        if (nextShown) {
            g_backNextSound.Mix(fighters.sound, nextGain);
        }
        // The next stage is all there is: it is the stage now
        if ((g.way == kStageShuffle && g.blend >= kBlend) || (g.way == kStageScroll && g.slide >= Across())) {
            g_back = std::move(g_backNext);
            g_backNext.Clear();
            g_backSound = g_backNextSound;
            g.back = g.backNext;
            g.backNext = 0;
            g.backFor = 0;
            g.blend = 0;
            g.slide = 0;
            g.backCamera = LeftEnd();
        }
        Give(machine, fighters);
    }

    // The picture nearer black by `part` of 256
    void Darken(Machine &machine, int part) {
        const int keep = 256 - std::clamp(part, 0, 256);
        uint32_t *picture = machine.Picture();
        for (int i = 0; i < machine.PictureWidth() * Video::kHeight; i++) {
            const uint32_t p = picture[i];
            picture[i] = ((((p & 0xFF00FF) * static_cast<uint32_t>(keep)) >> 8) & 0xFF00FF) | ((((p & 0x00FF00) * static_cast<uint32_t>(keep)) >> 8) & 0x00FF00);
        }
    }

    // The next match made ready on the spare machine, a few frames a frame until its round is on
    void MakeNext(Machine &machine) {
        for (int n = 0; n < kOnTheWay && !g_next.ready && g_next.frames <= kStartMost; n++) {
            RunSpare(machine, g_next, [](Machine &spare) {
                Want();
                g_next.ready = ToRound(spare, g_next.frames);
            });
        }
    }

    // --- the ring out ---

    // Is a fighter out? Knocked about (not its own master), it has gone past where it would have been kept in at
    // an end of the stage (RingOutLetsOut): its side past the picture's edge.
    void LookForOut() {
        const int camera = CameraX();
        for (int n = 0; n < 2 && g.out < 0; n++) {
            if (Ram8(Player(n) + d.control) == d.controlOwn) {
                continue;
            }
            const int x = static_cast<int16_t>(Ram16(Player(n) + d.x)), half = Ram16(Player(n) + d.halfWidth);
            const int margin = d.wideMargin();
            const int side = camera <= d.cameraLeft + margin && x - half < camera - margin ? -1 :
                             camera >= d.cameraRight - margin && x + half > camera + Video::kWidth + margin ? 1 : 0;
            if (side) {
                g.out = n;
                g.outSide = side;
                g.phase = Phase::Falling;
                g.since = 0;
                g.score += n == 1;
            }
        }
    }

    // The next match, and its stage, made ready while this one is fought (who it is against is known: the
    // player's character against someone new)
    void MakeDrop(Machine &machine) {
        const int frames = g.phase == Phase::Falling ? kDropReadyFalling : kDropReady;
        for (int n = 0; n < frames && !g_next.ready && g_next.frames <= kStartMost; n++) {
            RunSpare(machine, g_next, [](Machine &spare) {
                Want();
                g_next.ready = ToRound(spare, g_next.frames);
            }, nullptr, [](Machine &) {
                // (where its camera is with the fighters standing: the way down's bottom)
                g.dropCameraX = CameraX();
                g.dropCameraY = CameraY();
            });
        }
        for (int n = 0; n < frames && !g_dropBack.ready && g_dropBack.frames <= kStartMost; n++) {
            RunSpare(machine, g_dropBack, [](Machine &spare) {
                Wanted().stage = g.dropStage;
                g_dropBack.ready = ToRound(spare, g_dropBack.frames);
            });
        }
    }

    // At the top of its jump on the old stage the winner is handed over to the next match, where it is in the
    // picture: as high above the next stage's floor as the camera is held, and more (RingOutMove puts it there, at
    // the next match's next frame)
    void HandOver() {
        const uint32_t winner = Player(0);
        if (g.dropped || g.handing || g.since <= kDropJump + 2 || !d.atJumpTop(winner)) {
            return;
        }
        const int across = static_cast<int16_t>(Ram16(winner + d.x)) - CameraX();
        g.flightX = g.dropCameraX + std::clamp(across, kDropInside, PictureWidth() - kDropInside);
        g.flightY = static_cast<int16_t>(Ram16(winner + d.y)) - CameraY() + g.dropCameraY + Video::kHeight;
        for (uint32_t n = 0; n < std::min(d.playerSize, battle::kMostRecord); n++) {
            g.flight[n] = Ram8(winner + n);
        }
        g.handedY = g.flightY;
        g.handing = true;
    }

    // The way down, after the picture and sound of the old stage are put together (with the winner in it until it is
    // handed over): the next match's machines run, the camera follows the winner down, and the picture is the old
    // stage going up and out as the next one comes up from below, the fighters over both.
    void Drop(Machine &machine) {
        static Made old, fighters, stage;
        Take(machine, g_soundBefore, old);
        const int height = Video::kHeight;
        if (g.since == 1) {
            g_dropSound.Start();
            g_dropBackSound.Start();
        }
        // (the camera: held up till then; then, as the winner falls, down as far as it has fallen)
        if (g.dropped) {
            const int32_t follow = std::clamp((height - (g.handedY - g.dropY)) * 256, 0, height * 256);
            g.raised += (follow - g.raised) / kDropFollow;
            if (std::abs(follow - g.raised) < 256) {
                g.raised = follow;
            }
        }
        const int32_t shown = g.raisedShown; // (a machine's picture is the frame before's: drawn where its camera was then)
        g.raisedShown = g.raised;
        RunSpare(machine, g_next, [](Machine &m) {
            m.video.hideObject = d.notTheFighters;
            m.video.layerMask = 1;
            m.video.markObjects = true;
            m.players[0] = 0;
            for (int n = 0; n < 2; n++) {
                SetRam8(Player(n) + d.computer, 0);
            }
            BothPlay();
            if (g.since == 1) {
                d.playSound(d.tuneOff);
            }
            CameraAt().y = g.dropCameraY + g.raised / 256;
            CameraAt().beyondEnds = true;
            g_dropIn = g.handing;
            g.handing = false;
        }, &fighters, [](Machine &) {
            g_dropIn = false;
            g.dropCameraX = CameraX();
            g.dropY = static_cast<int16_t>(Ram16(Player(0) + d.y));
            g.landed = g.dropped && !d.inAir(Player(0));
        });
        RunStage(machine, g_dropBack, g.dropStage, g.dropCameraX, g.dropCameraY, &stage);
        g_dropSound.Add(fighters.sound);
        g_dropBackSound.Add(stage.sound);

        const int up = height - shown / 256;
        if (old.picture.size() == stage.picture.size()) {
            if (up > 0) {
                const size_t width = old.picture.size() / Video::kHeight, row = width * sizeof(uint32_t);
                memmove(old.picture.data(), old.picture.data() + static_cast<size_t>(up) * width, static_cast<size_t>(height - up) * row);
                memcpy(old.picture.data() + static_cast<size_t>(height - up) * width, stage.picture.data(), static_cast<size_t>(up) * row);
            }
            if (g.since >= kDropShown) { // (the first pictures are from before the camera was held up)
                Over(old.picture, fighters.picture, 256);
            }
        }
        const int gain = up * 256 / height;
        for (int16_t &sample : old.sound) {
            sample = static_cast<int16_t>(sample * (256 - gain) / 256);
        }
        g_dropSound.Mix(old.sound, gain);
        g_dropBackSound.Mix(old.sound, gain);
        Give(machine, old);
    }

    // At the bottom: the next match's machines are the ones seen from here, and the match after it is begun
    void Landed(Machine &machine) {
        Promote(machine, g_next);
        Begin(g_next);
        g_back = std::move(g_dropBack);
        Begin(g_dropBack);
        g_backSound = g_dropBackSound;
        g.back = g.dropStage;
        g.dropStage = AnyStage(g.back);
        g.characters[1] = AnyCharacter(Ram8(Player(1) + d.character));
        g.out = -1;
        g.flung = false;
        g.handing = false;
        g.dropped = false;
        g.landed = false;
        g.phase = Phase::Fighting;
        g.since = 0;
    }

    void Frame(Machine &machine) {
        g_portFrame = false; // (from here the machines run are spares, until the port's next frame)
        g_open = false;
        if (g.phase != Phase::Starting && !d.inGame()) {
            End(); // (the board was reset)
            return;
        }
        std::vector<HudLine> lines;
        int dark = 0;         // how far the fighters (or, with one stage, the whole picture) are faded, of 256
        bool layered = false; // the picture and sound have been put together already
        if (g.kind == Kind::RingOut && g.phase != Phase::Starting && g.phase != Phase::Over) {
            // (there is no knock-down here: a fighter is out, or the fight goes on)
            if (g.phase == Phase::Fighting) {
                LookForOut();
                MakeDrop(machine);
            } else if (g.phase == Phase::Falling) {
                g.since++;
                MakeDrop(machine);
                if (g.since >= kFallFor && g.out == 0) {
                    g.phase = Phase::Over;
                    g.since = 0;
                    if (g.score > FeatureValue("ringout_best")) {
                        SetFeatureValue("ringout_best", std::min(g.score, 99));
                    }
                } else if (g.since >= kFallFor && g_next.ready && g_dropBack.ready) {
                    g.phase = Phase::Dropping;
                    g.since = 0;
                    g.raised = g.raisedShown = Video::kHeight * 256; // (the next match's camera held up from the first)
                } else if (g_next.frames > kStartMost || g_dropBack.frames > kStartMost) {
                    End();
                    return;
                }
            } else if (g.phase == Phase::Dropping) {
                g.since++;
                HandOver();
            }
        } else if (g.phase == Phase::Fighting) {
            const uint8_t result = Ram8(d.result);
            if (result) {
                g.winner = result == 2 ? 1 : 0; // (both down at once: the first player's)
                g.phase = Phase::Decided;
                g.since = 0;
                Begin(g_next);
                if (g.kind == Kind::Survival) {
                    if (result != 1) {
                        g.phase = Phase::Over;
                        if (g.score > FeatureValue("survival_best")) {
                            SetFeatureValue("survival_best", std::min(g.score, 99));
                        }
                    } else {
                        g.score++;
                        const auto left = [](uint32_t which) {
                            const int now = static_cast<int16_t>(Ram16(Player(0) + which));
                            return std::min<int>(std::max(now, 0) + d.healthBack, d.healthFull);
                        };
                        g.health = left(d.health);
                        g.ceiling = d.ceilingShown ? g.health : std::max(left(d.healthCeiling), g.health);
                        g.characters[1] = AnyCharacter(g.characters[1]);
                    }
                } else {
                    // The one knocked down is replaced (by the one chosen for that side in the menu, if that
                    // is not who has just lost)
                    const int loser = 1 - g.winner;
                    const int chosen = ChosenCharacter(loser == 0 ? "endless_one" : "endless_two");
                    g.characters[loser] = chosen >= 0 && chosen != g.characters[loser] ? chosen : AnyCharacter(g.characters[0], g.characters[1]);
                    g.score++;
                    if (!Layered()) {
                        g.stage = ChosenStage();
                    }
                }
            }
        } else if (g.phase == Phase::Decided) {
            g.since++;
            MakeNext(machine);
            if (g.since > kAfter - kFade) {
                dark = (g.since - (kAfter - kFade)) * 256 / kFade;
            }
            if (g.since >= kAfter && g_next.ready) {
                if (Layered()) {
                    Layer(machine, 0); // (this frame's stage and sound, before the machine is another)
                    layered = true;
                }
                Promote(machine, g_next);
                if (g.kind == Kind::Survival) {
                    SetRam16(Player(0) + d.health, static_cast<uint16_t>(g.health));
                    SetRam16(Player(0) + d.healthCeiling, static_cast<uint16_t>(g.ceiling));
                }
                if (Layered()) {
                    d.playSound(d.tuneOff);
                }
                dark = 256;
                g.phase = Phase::Fighting;
                g.since = 0;
                g.fadeIn = kFade;
            } else if (g_next.frames > kStartMost) {
                End();
                return;
            }
        } else if (g.phase == Phase::Over) {
            g.since++;
            char text[64];
            const bool ring = g.kind == Kind::RingOut;
            lines.push_back({0.5f, 0.36f, 0, ring ? "RING OUT" : "SURVIVAL OVER"});
            snprintf(text, sizeof(text), ring ? "Ring outs: %d" : "Opponents beaten: %d", g.score);
            lines.push_back({0.5f, 0.46f, 0, text});
            snprintf(text, sizeof(text), "Best: %d", std::max(g.score, FeatureValue(ring ? "ringout_best" : "survival_best")));
            lines.push_back({0.5f, 0.54f, 0, text});
            if (g.since > kOverFor) {
                End();
                machine.Reset();
                return;
            }
        }
        if (g.phase == Phase::Fighting && g.fadeIn > 0) {
            dark = std::max(dark, g.fadeIn * 256 / kFade);
            g.fadeIn--;
        }
        if (Layered()) {
            if (!layered) {
                // (on the way down the winner is seen over the old stage until the next match's picture has it)
                Layer(machine, g.phase == Phase::Dropping && g.dropped ? 0 : 256 - dark);
            }
            if (g.phase == Phase::Dropping) {
                Drop(machine);
                if ((g.landed && g.raised == 0) || g.since >= kDropMost) {
                    Landed(machine);
                }
            }
        } else if (dark) {
            Darken(machine, dark);
        }
        if (g.kind == Kind::Survival && g.phase != Phase::Starting && g.phase != Phase::Over) {
            char text[32];
            snprintf(text, sizeof(text), "Beaten: %d", g.score);
            lines.push_back({0.5f, 0.21f, 0, text});
        }
        SetHud(std::move(lines));
        SetBoxes({});
    }
} // namespace

bool BattleMode() {
    return g.kind != Kind::None;
}
void EndBattle() {
    End();
}
bool BattleNoClock() {
    return g.kind == Kind::Endless || g.kind == Kind::RingOut;
}
bool BattleNoKnockOut() {
    return g.kind == Kind::RingOut;
}

void BattleBeforeFrame(Machine &machine) {
    g_portFrame = true;
    if (g.kind != Kind::None) {
        BeforeFrame(machine);
    }
}

void BattleFrame(Machine &machine) {
    if (g.kind != Kind::None) {
        Frame(machine);
    }
    // What the next frame of the port's machine draws: all of it; or no panel; or the fighters alone, marked,
    // to be laid over the stage that another machine shows
    // The ring out: the shadow of the one who went over the end does not stay on the floor
    static int noShadow = -1;
    const int out = g.kind == Kind::RingOut ? g.out : -1;
    if (d.noShadow && out != noShadow) {
        noShadow = out;
        d.noShadow(machine, out);
    }
    if (g.kind != Kind::None || !d.stageViewer()) {
        const bool fighting = (g.kind == Kind::Endless || g.kind == Kind::RingOut) && g.phase != Phase::Starting;
        const bool layered = fighting && Layered();
        machine.video.hideObject = layered ? d.notTheFighters : fighting ? d.thePanel : nullptr;
        machine.video.layerMask = layered ? 1 : 0xF;
        machine.video.markObjects = layered;
    }
}

void BattleState(Archive &ar) {
    ar.Mark(0x42415454);
    ar.Pod(g);
    ar.Vec(g_next.state);
    ar.Pod(g_next.ready);
    ar.Pod(g_next.frames);
    for (Spare *spare : {&g_back, &g_backNext, &g_dropBack}) {
        ar.Vec(spare->state);
        ar.Pod(spare->ready);
        ar.Pod(spare->frames);
    }
    if (!ar.Saving() && !RunningAhead()) { // (not when run-ahead puts the frame back: runtime/game.h)
        g_fast = false;
        g_backSound.Start();
        g_backNextSound.Start();
    }
}

RingOut RingOutMove(uint32_t player) {
    RingOut ringOut;
    if (g.kind != Kind::RingOut) {
        return ringOut;
    }
    if (g_dropIn && player == Player(0)) {
        g.dropped = true;
        ringOut.move = RingOut::kDropsIn;
        ringOut.x = g.flightX;
        ringOut.y = g.flightY;
        ringOut.record = g.flight.data();
    } else if (g_portFrame && g.out >= 0 && player == Player(g.out) &&
               (g.phase == Phase::Falling || g.phase == Phase::Dropping || g.phase == Phase::Over)) {
        ringOut.move = g.flung ? RingOut::kFalling : RingOut::kFlungOut;
        ringOut.side = g.outSide;
        g.flung = true;
    }
    return ringOut;
}

// In a ring out fight a player that is not its own master (reeling from a hit, guarding one, thrown) is not
// kept in at a side where the picture is at the stage's end: it goes over the end (the game's sources ask where the
// game keeps the players in).
bool RingOutLetsOut(uint32_t player) {
    if (!g_open || (player != Player(0) && player != Player(1)) || Ram8(player + d.control) == d.controlOwn) {
        return false;
    }
    const int x = static_cast<int16_t>(Ram16(player + d.x)), half = Ram16(player + d.halfWidth);
    const int camera = CameraX();
    const int margin = d.wideMargin(); // (the picture's edges and the camera's ends)
    const bool left = camera <= d.cameraLeft + margin && x - half <= camera - margin;
    const bool right = camera >= d.cameraRight - margin && x + half >= camera + Video::kWidth + margin;
    return left || right;
}

void battle::Register(const Description &description) {
    d = description;
    Game &game = TheGame();
    game.pages.push_back({kSurvival, kModes, "One opponent after another on the health you have left. How many can you beat?"});
    game.pages.push_back({kEndless, kModes, "Two fighters the game plays, for as long as you like. Touch the controls to take one over."});
    game.commands.push_back({"survival_start", kSurvival, "Start",
                             "Choose a character; then one opponent after another, each on a knock-down. Your health is what the "
                             "fight before left you, and a little back. Over the first time you go down.",
                             StartSurvival});
    {
        // (kept with the features so that it is remembered; the numbers are its choices)
        static std::vector<std::string> numbers;
        std::vector<const char *> choices;
        for (int n = 0; n < 100; n++) {
            numbers.push_back(std::to_string(n));
        }
        for (const std::string &number : numbers) {
            choices.push_back(number.c_str());
        }
        game.features.push_back({"survival_best", kSurvival, "Best", "The most opponents beaten in one survival. (Left puts it back.)", 0, choices});
    }
    game.pages.push_back({kRingOut, kModes, "No health: knock your opponent over the end of the stage, and jump down to the next."});
    game.commands.push_back({"ringout_start", kRingOut, "Start",
                             "Choose a character. Nobody loses health: the ends of a stage are open, and a fighter knocked over "
                             "one falls. Knock your opponent out and you jump down to the next stage; over when you fall.",
                             StartRingOut});
    {
        static std::vector<std::string> numbers;
        std::vector<const char *> choices;
        for (int n = 0; n < 100; n++) {
            numbers.push_back(std::to_string(n));
        }
        for (const std::string &number : numbers) {
            choices.push_back(number.c_str());
        }
        game.features.push_back({"ringout_best", kRingOut, "Best", "The most ring outs in one go. (Left puts it back.)", 0, choices});
    }
    game.commands.push_back({"endless_start", kEndless, "Start",
                             "Two fighters played by the game: no bars, no clock, and the one knocked down is replaced. Press "
                             "anything to take a fighter over; ten idle seconds give it back.",
                             StartEndless});
    game.features.push_back({"endless_one", kEndless, "Fighter 1", "Who is on player 1's side: someone new each time, or always the same.", 0, d.fighterChoices});
    game.features.push_back({"endless_two", kEndless, "Fighter 2", "Who is on player 2's side: someone new each time, or always the same.", 0, d.fighterChoices});
    game.features.push_back({"endless_stage", kEndless, "Stage",
                             "Where they fight. Shuffle: a stage that fades into another every so often. Scroll: the stages "
                             "go by, one after another, and the fighters are carried along. Or one stage.",
                             0, d.stageChoices});
    game.features.push_back({"endless_shuffle", kEndless, "Shuffle every",
                             "Shuffle: how long they fight on a stage before it fades into another.", 0,
                             {"1 minute", "2 minutes", "3 minutes", "5 minutes", "10 minutes"}});
    game.features.push_back({"endless_scroll", kEndless, "Scroll speed", "Scroll: how fast the stages go by.", 0, {"Slow", "Medium", "Fast"}});
}

} // namespace cps2recomp
