// SPDX-License-Identifier: GPL-3.0-only
// Copyright (C) 2026 Tommy McLeroy. Part of cps2recomp.
//
// The "Alternative modes" page, attract mode and the stage viewer (alternative.h), made from Vampire Savior's and Street
// Fighter Alpha 2's, which were the same but for where each game keeps things.
#include "alternative.h"

#include "spare.h"
#include "state.h"

#include <algorithm>
#include <cstring>
#include <iterator>

namespace cps2recomp::modes {

namespace {
    Description d;
    MatchWanted g_wanted;
    CameraWanted g_camera;

    enum class Mode : int32_t { None, Attract, Viewer };
    struct State {
        Mode mode = Mode::None;
        bool starting = false;  // on the way there: the board is starting up (the game runs unseen)
        int32_t frames = 0;     //   and for how many frames
        int32_t stage = 0;      // the viewer: the stage shown (1 the first)
        int32_t camera = 0;     //   where the picture's left edge is wanted, in 256ths of a pixel
        int32_t direction = 1;  //   which way it scrolls by itself
        int32_t music = 0;      //   the "Music" choice that is playing
        // The infinite stage: the stage that comes next is kept running beside the one shown
        bool infinite = false;
        int32_t next = 0;       // the stage that comes next
        bool nextReady = false; //   its round is being fought (until then it is on its way there)
        int32_t nextFrames = 0; //   frames it has run on the way
        int32_t slide = 0;      // how far the next stage has come into the picture, in 256ths of a pixel
    } g;
    std::vector<uint8_t> g_other;      // the machine of the stage that comes next, as a save state
    std::vector<uint8_t> g_mine;       // (the shown stage's, while the other's runs)
    std::vector<int16_t> g_otherSound; // sound the next stage has made and that has not been heard yet
    std::vector<int16_t> g_carry;      // sound to come before the next frame's
    size_t g_soundBefore = 0;          // how much sound the machine held before the frame
    bool g_fast = false;               // this file asked for the game to run unseen
    int g_chosenStage = -1;            // the menu's stage for the viewer when last looked at
    bool g_hid = false;                // this file hid objects from the picture

    constexpr int kStartMost = 9000;   // frames after which getting to a stage is given up
    constexpr int32_t kStick = 384;    // a frame's scrolling with the stick held, in 256ths of a pixel
    constexpr int32_t kScroll[] = {0, 64, 128, 256}; //   and by itself: slow, medium, fast
    constexpr int kOnTheWay = 3;       // frames a frame that the next stage runs until its round is on
    constexpr size_t kSoundAhead = 16; // samples of the next stage's sound kept in hand

    int Stages() {
        return static_cast<int>(d.stageNames.size());
    }
    int WideSide() {
        return d.wideSide ? d.wideSide() : 0;
    }
    int WideMargin() {
        return d.wideMargin ? d.wideMargin() : 0;
    }
    uint32_t Player(int n) {
        return d.player1 + static_cast<uint32_t>(n) * d.playerSize;
    }

    void Fast(bool on) {
        if (on != g_fast) {
            g_fast = on;
            SetFastForward(on);
        }
    }

    void EndThis() {
        if (g.mode == Mode::None) {
            return;
        }
        g = State();
        g_chosenStage = -1;
        g_wanted = MatchWanted();
        g_other.clear();
        g_otherSound.clear();
        g_carry.clear();
        Fast(false);
    }

    // --- attract mode ---

    void StartAttract(Machine &machine) {
        EndThis();
        EndBattle();
        SetFeature("training_mode", false);
        g.mode = Mode::Attract;
        g.starting = true;
        machine.Reset();
    }

    // --- the stage viewer ---

    // The camera's ends, in 256ths of a pixel, for the picture being played in (a wide picture's camera stops short
    // of where the board's does)
    int32_t LeftEnd() {
        return (d.cameraLeft + WideMargin()) * 256;
    }
    int32_t RightEnd() {
        return (d.cameraRight - WideMargin()) * 256;
    }
    int32_t Across() { // the picture's width: how far a stage slides to be gone
        return (Video::kWidth + 2 * WideSide()) * 256;
    }

    // The players put where the camera then goes to `camera` (it keeps the middle of the two in the middle of the
    // picture)
    void Place(int32_t camera) {
        const uint32_t middle = static_cast<uint32_t>(camera / 256 + d.apart);
        const uint32_t x[2] = {middle - static_cast<uint32_t>(d.apart / 2), middle + static_cast<uint32_t>(d.apart / 2)};
        for (int n = 0; n < 2; n++) {
            if (d.xLong) {
                SetRam32(Player(n) + d.x, x[n] << 16);
            } else {
                SetRam16(Player(n) + d.x, static_cast<uint16_t>(x[n]));
            }
        }
    }

    void StartViewer(Machine &machine) {
        EndThis();
        EndBattle();
        SetFeature("training_mode", false);
        g.mode = Mode::Viewer;
        g.starting = true;
        g_chosenStage = FeatureValue("viewer_stage");
        g.infinite = g_chosenStage == Stages();
        g.stage = g.infinite ? 1 : std::clamp(g_chosenStage, 0, Stages() - 1) + 1;
        g.next = g.stage % Stages() + 1;
        g.camera = g.infinite ? LeftEnd() : (LeftEnd() + RightEnd()) / 2;
        g_wanted.stage = g.stage;
        machine.Reset();
    }

    void ViewerBeforeFrame(Machine &machine) {
        g_soundBefore = machine.audio.size();
        g_wanted.stage = g.stage;
        if (g.starting) {
            Fast(true);
            if (ToRound(machine, g.frames)) {
                g.starting = false;
                Fast(false);
            } else if (g.frames > kStartMost) {
                EndThis();
            }
            if (g.starting || g.mode != Mode::Viewer) {
                return;
            }
        }
        const uint16_t stick = (machine.players[0] | machine.players[1]) & (kLeft | kRight);
        machine.players[0] = machine.players[1] = 0;
        const int scroll = std::clamp(FeatureValue("viewer_scroll"), 0, static_cast<int>(std::size(kScroll)) - 1);
        const int32_t byStick = stick == kRight ? kStick : stick == kLeft ? -kStick : 0;
        if (g.infinite) {
            // Always onwards by itself; with the stick, back as well, as far as this stage's start. The stage is
            // scrolled to its end, and then slides out as the next comes in (if that one is ready).
            int32_t by = scroll == 0 ? byStick : kScroll[scroll];
            if (g.slide > 0 || (g.camera + by >= RightEnd() && by > 0)) {
                by -= RightEnd() - g.camera;
                g.camera = RightEnd();
                g.slide = g.nextReady ? std::clamp(g.slide + by, 0, Across()) : 0;
            } else {
                g.camera = std::max(g.camera + by, LeftEnd());
            }
        } else {
            g.camera += scroll == 0 ? byStick : g.direction * kScroll[scroll];
            if (g.camera >= RightEnd()) {
                g.camera = RightEnd();
                g.direction = -1;
            } else if (g.camera <= LeftEnd()) {
                g.camera = LeftEnd();
                g.direction = 1;
            }
        }
        Place(g.camera);
    }

    // The infinite stage, after the shown stage's frame: a frame of the next stage's machine (or a few, while it is
    // on its way to its round), and the picture and the sound made of the two.
    void RunNext(Machine &machine) {
        const int width = machine.PictureWidth();
        const size_t pixels = static_cast<size_t>(width) * Video::kHeight;
        const Video::Wide wide = machine.video.wide;
        static std::vector<uint32_t> shown;
        shown.assign(machine.Picture(), machine.Picture() + pixels);
        g_soundBefore = std::min(g_soundBefore, machine.audio.size());
        std::vector<int16_t> sound(machine.audio.begin() + static_cast<std::ptrdiff_t>(g_soundBefore), machine.audio.end());
        machine.audio.resize(g_soundBefore);
        const uint16_t held[2] = {machine.players[0], machine.players[1]};

        // The shown stage's machine put away, the next stage's taken out (or switched on)
        g_mine.clear();
        {
            Archive ar(g_mine, true);
            machine.Serialize(ar);
        }
        if (g_other.empty()) {
            machine.Reset();
            g.nextReady = false;
            g.nextFrames = 0;
            g_otherSound.assign(kSoundAhead * 2, 0);
        } else {
            Archive ar(g_other, false);
            machine.Serialize(ar);
        }
        g_wanted.stage = g.next;
        machine.video.hideObject = d.notTheStage;
        machine.video.layerMask = d.viewerLayers;
        if (d.wideForSpare) {
            d.wideForSpare(machine); // (its picture as wide as the shown stage's)
        }
        for (int n = 0; n < kOnTheWay; n++) {
            machine.audio.resize(g_soundBefore);
            g.nextReady = g.nextReady || ToRound(machine, g.nextFrames);
            if (g.nextReady) {
                machine.players[0] = machine.players[1] = 0;
                Place(LeftEnd());
            }
            machine.RunFrame();
            if (g.nextReady || g.nextFrames > kStartMost) {
                break;
            }
        }
        if (g.nextReady) {
            g_otherSound.insert(g_otherSound.end(), machine.audio.begin() + static_cast<std::ptrdiff_t>(g_soundBefore), machine.audio.end());
        }
        machine.audio.resize(g_soundBefore);

        // The picture: the shown stage moved left by the slide, the next stage after it
        const int slide = std::clamp(g.slide / 256, 0, width);
        if (slide > 0 && machine.PictureWidth() == width) {
            const uint32_t *next = machine.Picture();
            for (int y = 0; y < Video::kHeight; y++) {
                uint32_t *row = &shown[static_cast<size_t>(y) * width];
                memmove(row, row + slide, static_cast<size_t>(width - slide) * sizeof(uint32_t));
                memcpy(row + width - slide, next + static_cast<size_t>(y) * width, static_cast<size_t>(slide) * sizeof(uint32_t));
            }
        }
        // The sound: one fades into the other as the next stage comes in. As many samples as the shown stage made are
        // taken of the next stage's (the two machines' frames do not make quite the same number).
        const size_t count = sound.size();
        if (g_otherSound.size() < count) {
            g_otherSound.resize(count, 0);
        }
        const int32_t nextGain = slide * 256 / width, shownGain = 256 - nextGain;
        for (size_t i = 0; i < count; i++) {
            const int32_t mixed = (sound[i] * shownGain + (g.nextReady ? g_otherSound[i] : 0) * nextGain) / 256;
            sound[i] = static_cast<int16_t>(std::clamp(mixed, -32768, 32767));
        }
        g_otherSound.erase(g_otherSound.begin(), g_otherSound.begin() + static_cast<std::ptrdiff_t>(count));

        if (g.slide >= Across()) {
            // The next stage has come all the way in: its machine is the one the port runs from here, and the stage
            // after it is switched on
            g.stage = g.next;
            g.next = g.stage % Stages() + 1;
            g.camera = LeftEnd();
            g.slide = 0;
            g.nextReady = false;
            g.nextFrames = 0;
            g_other.clear();
            g_carry = g_otherSound; // (what it has made and was not yet heard comes first)
            g_otherSound.clear();
        } else {
            g_other.clear();
            {
                Archive ar(g_other, true);
                machine.Serialize(ar);
            }
            Archive ar(g_mine, false);
            machine.Serialize(ar);
        }
        g_wanted.stage = g.stage;
        machine.players[0] = held[0];
        machine.players[1] = held[1];
        machine.video.wide = wide;
        if (d.wideForPort) {
            d.wideForPort(machine);
        }
        memcpy(machine.Picture(), shown.data(), pixels * sizeof(uint32_t));
        machine.audio.insert(machine.audio.end(), sound.begin(), sound.end());
    }

    uint16_t Tune(int choice) { // 1 the first stage's; after the stages, the other tunes
        if (choice >= 1 && choice <= Stages()) {
            return choice <= static_cast<int>(d.stageTunes.size()) ? d.stageTunes[static_cast<size_t>(choice - 1)] : 0;
        }
        const int other = choice - Stages() - 1;
        return other >= 0 && other < static_cast<int>(d.otherTunes.size()) ? d.otherTunes[static_cast<size_t>(other)] : 0;
    }

    void ViewerFrame(Machine &machine) {
        if (g.starting) {
            return;
        }
        // Another stage chosen in the menu: the viewer goes there. (Not when what is shown is not what the menu says
        // for another reason: a save state of the viewer loaded.)
        const int chosen = FeatureValue("viewer_stage");
        if (g_chosenStage >= 0 && chosen != g_chosenStage) {
            StartViewer(machine);
            return;
        }
        g_chosenStage = chosen;
        if (g.infinite) {
            if (!g_carry.empty()) {
                machine.audio.insert(machine.audio.begin() + static_cast<std::ptrdiff_t>(std::min(g_soundBefore, machine.audio.size())),
                                     g_carry.begin(), g_carry.end());
                g_carry.clear();
            }
            RunNext(machine); // (each stage with its own tune)
        } else {
            const int music = FeatureValue("viewer_music"); // 0: the stage's own; then the stages' tunes; then the others
            if (music != g.music) {
                g.music = music;
                const uint16_t tune = Tune(music == 0 ? g.stage : music);
                if (tune) {
                    PlaySound(d.tuneOff);
                    PlaySound(tune);
                }
            }
        }
        SetHud({});
        SetBoxes({});
    }

    void TrainingCommand(Machine &machine) {
        if (d.startTraining) {
            d.startTraining(machine);
        }
    }
} // namespace

const Description &Described() {
    return d;
}
MatchWanted &Wanted() {
    return g_wanted;
}
CameraWanted &CameraAt() {
    return g_camera;
}

bool RoundFought() {
    return Ram16(d.steps) == 2 && Ram16(d.steps + 4) == d.gameMatch && Ram16(d.steps + 8) == d.matchRound &&
           Ram16(d.steps + 12) == d.roundFought;
}

bool Choosing() {
    return Ram16(d.steps) == 2 && Ram16(d.steps + 4) == d.gameChoosing && Ram8(d.playersIn) == 3;
}

bool ToRound(Machine &machine, int32_t &frames, bool choose) {
    if (RoundFought()) {
        return true;
    }
    if (Ram8(d.credits) < 2) {
        SetRam8(d.credits, 2);
    }
    const bool press = frames++ % 16 < 6;
    const uint8_t in = Ram8(d.playersIn);
    const uint16_t one = machine.players[0];
    machine.players[0] = machine.players[1] = 0;
    if (!press) {
        // (nothing)
    } else if (d.startsTogether) {
        machine.players[0] = machine.players[1] = in == 3 ? d.choose : kStart;
    } else if (!(in & 1)) {
        machine.players[0] = kStart;
    } else if (!(in & 2)) {
        machine.players[1] = kStart;
    } else {
        machine.players[0] = machine.players[1] = d.choose;
    }
    if (choose && in == 3) {
        // (player 1's own controls, but for Start and the coin, which would do other things here)
        machine.players[0] = one & static_cast<uint16_t>(~(kStart | kCoin));
    }
    return false;
}

void PlaySound(uint16_t code) {
    const uint16_t at = Ram16(d.soundQueueEnd) & 0xFF0;
    SetRam32(d.soundQueue + at, code);
    SetRam32(d.soundQueue + at + 4, 0);
    SetRam32(d.soundQueue + at + 8, 0);
    SetRam16(d.soundQueueEnd, static_cast<uint16_t>((at + 0x10) & 0xFF0));
}

void GoTo(uint16_t part) {
    SetRam16(d.steps, part);
    for (uint32_t at = d.steps + 2; at < d.steps + 0x10; at += 2) {
        SetRam16(at, 0);
    }
}

std::vector<const char *> StageChoices(std::initializer_list<const char *> before, std::initializer_list<const char *> after) {
    std::vector<const char *> choices(before);
    choices.insert(choices.end(), d.stageNames.begin(), d.stageNames.end());
    choices.insert(choices.end(), after);
    return choices;
}

bool AttractMode() {
    return g.mode == Mode::Attract;
}
bool StageViewer() {
    return g.mode == Mode::Viewer;
}
int StageViewerStage() {
    return g.mode == Mode::Viewer ? g_wanted.stage : 0;
}
void End() {
    EndThis();
}

void BeforeFrame(Machine &machine) {
    if (g.mode == Mode::Attract) {
        // Start or a coin ends it, and is passed on; nothing else reaches the game
        if ((machine.players[0] | machine.players[1]) & (kStart | kCoin)) {
            EndThis();
        } else {
            machine.players[0] = machine.players[1] = 0;
        }
    } else if (g.mode == Mode::Viewer) {
        ViewerBeforeFrame(machine);
    }
}

void Frame(Machine &machine) {
    // The board was reset (the menu's Reset, or another mode starting), or a game begun: the mode is over
    if (g.mode == Mode::Attract) {
        if (g.starting) {
            g.frames++;
            g.starting = !(d.attractBegun && d.attractBegun(g.frames));
            Fast(g.starting && FeatureValue("attract_show") != kShowAll); // (the board starting up, unseen when only a part is wanted)
        } else if (d.attractOver && d.attractOver()) {
            EndThis();
        }
    } else if (g.mode == Mode::Viewer && !g.starting && Ram16(d.steps) == 0) {
        EndThis();
    }
    if (g.mode == Mode::Attract && !g.starting) {
        const int show = FeatureValue("attract_show");
        Fast(show != kShowAll && d.attractStep && d.attractStep(show));
    } else if (g.mode == Mode::Viewer) {
        ViewerFrame(machine);
    }
    // (only what this file set is put back)
    const bool viewing = g.mode == Mode::Viewer && !g.starting;
    if (viewing || g_hid) {
        machine.video.hideObject = viewing ? d.notTheStage : nullptr;
        machine.video.layerMask = viewing ? d.viewerLayers : 0xF;
        g_hid = viewing;
    }
}

void State(Archive &ar) {
    ar.Mark(0x4D4F4445);
    ar.Pod(g);
    ar.Vec(g_other);
    if (!ar.Saving() && !RunningAhead()) { // (not when run-ahead puts the frame back: runtime/game.h)
        Fast(false);
        g_otherSound.assign(kSoundAhead * 2, 0);
        g_carry.clear();
        g_chosenStage = -1;
        g_wanted.stage = g.stage;
    }
}

void Register(const Description &description) {
    d = description;
    // (what is wanted of the machine that is running is kept around a spare's frame, and fresh for it)
    static MatchWanted kept;
    static CameraWanted keptCamera;
    SetSpareKeeping({[](Machine &) {
                         kept = g_wanted;
                         keptCamera = g_camera;
                         if (d.keep) {
                             d.keep();
                         }
                     },
                     [](Machine &m) {
                         g_wanted = MatchWanted();
                         g_camera = CameraWanted();
                         if (d.fresh) {
                             d.fresh(m);
                         }
                         if (d.wideForSpare) {
                             d.wideForSpare(m);
                         }
                     },
                     [](Machine &m) {
                         if (d.wideForPort) {
                             d.wideForPort(m);
                         }
                         g_wanted = kept;
                         g_camera = keptCamera;
                         if (d.putBack) {
                             d.putBack(m);
                         }
                     }});
    cps2recomp::Game &game = TheGame();
    // The page at the head of the menu, and a page under it for each mode (and the dummy's under training's)
    game.pages.push_back({kModes, "", d.modesWords, true});
    game.pages.push_back({kTraining, kModes, "A fight with no clock that nobody loses, and the aids for practising in it."});
    game.pages.push_back({kDummy, kTraining, "What player 2 does in a training session."});
    game.pages.push_back({kAttract, kModes, d.attractWords});
    game.pages.push_back({kViewer, kModes, "A stage with nobody on it, to look at and listen to."});
    game.commands.push_back({"training_start", kTraining, "Start",
                             "Straight to the character choice with two players (player 1's controls choose both), then a fight "
                             "with no clock that nobody loses.",
                             TrainingCommand});
    game.commands.push_back({"attract_start", kAttract, "Start",
                             "Starts attract mode: the game's own show, over and over. Start (or a coin) ends it.", StartAttract});
    game.features.push_back({"attract_show", kAttract, "Shows", d.attractShowWords, 0, {"Everything", "The intro only", "The fights only"}});
    game.commands.push_back({"viewer_start", kViewer, "Start",
                             "Shows the stage chosen below with nobody on it. Left and right scroll it, or it scrolls by itself. "
                             "Reset ends it.",
                             StartViewer});
    game.features.push_back({"viewer_stage", kViewer, "Stage",
                             "The stage the viewer shows, or Infinite: every stage in turn, each scrolling into the next with its "
                             "own tune. Changing it while viewing goes to the new one.",
                             0, StageChoices({}, {"Infinite"})});
    game.features.push_back({"viewer_scroll", kViewer, "Scrolling",
                             "How the viewer moves across the stage: with the stick, or by itself from one end to the other and back "
                             "(Infinite: always onwards).",
                             0, {"With the stick", "Slow", "Medium", "Fast"}});
    std::vector<const char *> music = StageChoices({"The stage's own"});
    music.insert(music.end(), d.otherTuneNames.begin(), d.otherTuneNames.end());
    game.features.push_back({"viewer_music", kViewer, "Music",
                             "The tune that plays in the viewer: the stage's own, or another's. (Infinite plays each stage's own.)", 0,
                             music});
}

} // namespace cps2recomp::modes
