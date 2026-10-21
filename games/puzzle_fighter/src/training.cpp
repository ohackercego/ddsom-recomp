// SPDX-License-Identifier: GPL-3.0-only
// Copyright (C) 2026 Tommy McLeroy
//
// Super Puzzle Fighter II Turbo: training. A session is a two-player match in which player 2 is a dummy:
// it stands by (its side of the screen does nothing) unless told to play, the player's attacks are shown
// in numbers, and the round can be started over at any time. Features are in the launcher and the in-game
// menu; the menu's "Training mode" starts a session.
#include "spf2.h"

#include "machine.h"
#include "state.h"

#include <cstdio>
#include <string>
#include <vector>

namespace cps2recomp {

bool DummyStandsBy();
void EndModes(); // modes.cpp
void DiamondNext(uint32_t player); // pair.cpp
bool PairPlace(uint32_t player, int columns[2], int rows[2], uint16_t gems[2]); // steer.cpp

namespace {
    using namespace spf2;

    // Getting to a session from switching on: Start, the two-player mode chosen, and on to the choice of
    // characters (the game runs unseen meanwhile)
    bool g_boot = false;
    int g_bootFrames = 0;
    constexpr int kBootMost = 4000; // frames after which it gives up
    // The machine as it was when the round's play began, for starting over
    std::vector<uint8_t> g_roundStart;
    struct State {
        bool played = false;      // the round was being played the frame before
        uint8_t stepBefore = 0;   // player 1's step the frame before
        int turns = 0;            // player 1's turns this round
        unsigned shown[2] = {};   // the break last shown for each player (its serial), and for how long
        int shownFor[2] = {};
        int turnTotal[2] = {};    // counter gems made in the turn so far
    } g;
    constexpr int kReadoutFrames = 240;
    constexpr int kDummyAttackEvery = 4; // the dummy attacks at the start of every so many of player 1's turns
    constexpr int kDummyAttacks[] = {0, 3, 6, 12, 24, 48};

    bool ChoosingMode() {
        return Ram16(kGameOn) == 2 && Ram16(kGameStep) == kGameChoosing && Ram16(kMatchStep) == 2 && Ram16(kRoundStep) == kChoosingMode;
    }
    bool ChoosingCharacters() {
        return Ram16(kGameOn) == 2 && Ram16(kGameStep) == kGameChoosing && Ram16(kMatchStep) == 2 && Ram16(kRoundStep) == kChoosingCharacters;
    }
    bool RoundPlayed() {
        return Ram16(kGameOn) == 2 && Ram16(kGameStep) == kGameMatch && Ram16(kMatchStep) == kMatchRound && Ram16(kRoundStep) == kRoundPlayed;
    }

    // The menu's "Training mode": the board switched off and on, and the way to the character choice worked
    void StartTraining(Machine &machine) {
        EndModes();
        SetFeature("training_mode", true);
        machine.Reset();
        g_boot = true;
        g_bootFrames = 0;
        g_roundStart.clear();
        g = State();
        SetFastForward(true);
    }

    // What the controls do on the way there
    void Boot(Machine &machine) {
        uint16_t &one = machine.players[0], &two = machine.players[1];
        const bool press = g_bootFrames % 16 < 6;
        one = two = 0;
        // (both players choosing: the byte select.cpp calls kChoosing. The players-in byte is not 3 here.)
        if ((ChoosingCharacters() && Ram8(kVars + 0x8D) == 3) || ++g_bootFrames > kBootMost) {
            g_boot = false;
            SetFastForward(false);
        } else if (ChoosingMode()) {
            one = !press ? 0 : Ram8(kModeCursor) == 0 ? kRight : kPunch1;
        } else if (ChoosingCharacters()) {
            two = press ? kStart : 0;
        } else {
            one = press ? kStart : 0;
        }
    }

    // "Start over": the round as it began
    void StartOver(Machine &machine) {
        if (g_roundStart.empty() || !FeatureOn("training_mode")) {
            return;
        }
        Archive ar(g_roundStart, false);
        machine.Serialize(ar);
        g = State();
        g.played = true;
    }

    void BeforeFrame(Machine &machine) {
        if (g_boot) {
            Boot(machine);
            return;
        }
        if (FeatureOn("training_mode") && ChoosingCharacters() && (Ram8(kCharactersChosen) & 1)) {
            // Player 1 has chosen: its controls work player 2's choice as well (training with one set of controls)
            machine.players[1] |= machine.players[0] & (kRight | kLeft | kUp | kDown | kPunch1 | kPunch2);
        }
    }

    void Readout(std::vector<HudLine> &lines) {
        for (int n = 0; n < 2; n++) {
            const BreakNote &note = LastBreak(n);
            if (note.serial != g.shown[n]) {
                g.shown[n] = note.serial;
                g.shownFor[n] = kReadoutFrames;
                g.turnTotal[n] = note.chain <= 1 ? note.attack : g.turnTotal[n] + note.attack;
            }
            if (g.shownFor[n] <= 0 || note.serial == 0) {
                continue;
            }
            g.shownFor[n]--;
            const float x = n == 0 ? 0.17f : 0.83f;
            char text[96];
            if (note.chain > 1) {
                snprintf(text, sizeof(text), "CHAIN %d  %d GEMS", note.chain, note.gems);
            } else {
                snprintf(text, sizeof(text), "%d GEMS", note.gems);
            }
            lines.push_back({x, 0.10f, 0, text});
            snprintf(text, sizeof(text), "%d PTS  ATTACK %d", note.score, note.attack);
            lines.push_back({x, 0.15f, 0, text});
            if (note.cancelled) {
                snprintf(text, sizeof(text), "CANCELS %d  SENDS %d", note.cancelled, note.waiting);
            } else {
                snprintf(text, sizeof(text), "TURN'S TOTAL %d", g.turnTotal[n]);
            }
            lines.push_back({x, 0.20f, 0, text});
        }
    }

    // The input display: player 1's controls as the turn reads them, the newest at the top, each with the
    // frames it has been held. The stick in numpad notation (5 is the middle, 4 left, 6 right, 2 down), A and
    // B the two buttons that turn the pair. (Only shown: not part of a save state.)
    struct Held {
        uint16_t pad;
        int frames;
    };
    std::vector<Held> g_inputs;
    constexpr size_t kInputsShown = 9;
    void InputDisplay(std::vector<HudLine> &lines) {
        constexpr uint16_t kStickBits = 0x000F, kButtonA = 0x0100, kButtonB = 0x0200;
        const uint16_t pad = Ram16(Player(0) + kPad) & (kStickBits | kButtonA | kButtonB);
        if (g_inputs.empty() || g_inputs.front().pad != pad) {
            g_inputs.insert(g_inputs.begin(), {pad, 0});
            if (g_inputs.size() > kInputsShown) {
                g_inputs.pop_back();
            }
        }
        if (g_inputs.front().frames < 999) {
            g_inputs.front().frames++;
        }
        float y = 0.34f;
        for (const Held &held : g_inputs) {
            // (the stick's bits: 1 right, 2 left, 4 down, 8 up)
            const int across = held.pad & 1 ? 1 : held.pad & 2 ? -1 : 0, up = held.pad & 8 ? 1 : held.pad & 4 ? -1 : 0;
            char text[32];
            snprintf(text, sizeof(text), "%3d  %d %s%s", held.frames, 5 + across + 3 * up, held.pad & kButtonA ? "A" : " ",
                     held.pad & kButtonB ? "B" : " ");
            lines.push_back({0.83f, y, 0, text});
            y += 0.05f;
        }
    }

    // The landing guide: outlines where the two gems of the pair being steered would come to rest if it
    // were left to fall now (each in its own column: the gems of a pair lying flat part when one has
    // further to fall)
    constexpr int kFieldLeft[2] = {16, 272}, kFieldFloor = 208, kCellPixels = 16;
    void LandingGuide() {
        std::vector<HudBox> boxes;
        if (FeatureOn("landing_guide") && RoundPlayed()) {
            for (int n = 0; n < 2; n++) {
                const uint32_t player = Player(n);
                int columns[2], rows[2];
                uint16_t gems[2];
                if (Ram8(player + kComputer) || (n == 1 && DummyStandsBy()) || !PairPlace(player, columns, rows, gems)) {
                    continue;
                }
                // How high each column is filled, and the lower gem of an upright pair first
                const int order[2] = {rows[0] <= rows[1] ? 0 : 1, rows[0] <= rows[1] ? 1 : 0};
                int lands[2] = {};
                for (const int gem : order) {
                    if (columns[gem] < 0 || columns[gem] >= kFieldColumns) {
                        continue;
                    }
                    int height = 0;
                    while (height < kFieldRows && Ram16(Cell(player, columns[gem], height)) != 0) {
                        height++;
                    }
                    if (gem == order[1] && columns[0] == columns[1]) {
                        height = lands[order[0]] + 1;
                    }
                    lands[gem] = height;
                    static const uint8_t kColours[5][3] = {{255, 255, 255}, {80, 140, 255}, {255, 230, 60}, {70, 220, 90}, {255, 80, 80}};
                    const int colour = gems[gem] == 5 ? 0 : (gems[gem] & kColour) <= 4 ? (gems[gem] & kColour) : 0;
                    boxes.push_back({kFieldLeft[n] + kCellPixels * columns[gem], kFieldFloor - kCellPixels * (height + 1), kCellPixels,
                                     kCellPixels, kColours[colour][0], kColours[colour][1], kColours[colour][2], 70});
                }
            }
        }
        SetBoxes(std::move(boxes));
    }

    // The cheat that gets a one-player game to its ending: forty counter gems from player 1 with every pair,
    // and player 1 cannot lose
    void QuickWin() {
        static uint8_t before = 0xFF;
        const uint8_t step = RoundPlayed() ? Ram8(Player(0) + kStep) : 0xFF;
        if (FeatureOn("quick_win") && step == kStepNewPair && before != kStepNewPair) {
            SetRam8(Player(0) + kAttack, 40);
            // (and a field that never fills: emptied when anything stands above the ninth row)
            bool high = false;
            for (int column = 0; column < kFieldColumns; column++) {
                high = high || Ram16(Cell(Player(0), column, 9)) != 0;
            }
            for (int column = 0; high && column < kFieldColumns; column++) {
                for (int row = 0; row < kFieldRows; row++) {
                    SetRam16(Cell(Player(0), column, row), 0);
                    SetRam16(Cell(Player(0), column, row) + kFieldExtra, 0);
                }
            }
        }
        before = step;
    }

    // The hidden challenger comes in place of the seventh opponent for a player who has done four things
    // in the game so far (PROGRESS.md; the game's test is at 00BD38). The cheat says a person has done them.
    void EarnChallenger() {
        constexpr uint32_t kQuickWin = 0xB3, kBestChain = 0xAE, kBestBroken = 0xB0, kBigFinishes = 0xB2; // bytes
        if (!FeatureOn("challenger")) {
            return;
        }
        for (int n = 0; n < 2; n++) {
            const uint32_t player = Player(n);
            if (Ram8(player + kComputer)) {
                continue;
            }
            SetRam8(player + kQuickWin, 0xFF);
            SetRam8(player + kBestChain, Ram8(player + kBestChain) < 4 ? 4 : Ram8(player + kBestChain));
            SetRam8(player + kBestBroken, Ram8(player + kBestBroken) < 20 ? 20 : Ram8(player + kBestBroken));
            SetRam8(player + kBigFinishes, Ram8(player + kBigFinishes) < 1 ? 1 : Ram8(player + kBigFinishes));
        }
    }

    void Frame(Machine &machine) {
        LandingGuide();
        QuickWin();
        EarnChallenger();
        if (!FeatureOn("training_mode") && !FeatureOn("attack_readout")) {
            if (g.played) {
                SetHud({});
            }
            g = State();
            return;
        }
        const bool played = RoundPlayed();
        if (!played) {
            if (g.played) {
                SetHud({});
            }
            g = State();
            return;
        }
        const bool training = FeatureOn("training_mode");
        if (!g.played) {
            // The round's play begins: the machine is kept for starting over
            g = State();
            g.played = true;
            if (training) {
                g_roundStart.clear();
                Archive ar(g_roundStart, true);
                machine.Serialize(ar);
            }
        }
        const uint8_t step = Ram8(Player(0) + kStep);
        if (training) {
            if (step == kStepLost) {
                // Player 1's field has filled up: the round starts over
                StartOver(machine);
                return;
            }
            if (step == kStepNewPair && g.stepBefore != kStepNewPair) {
                // A turn of player 1's begins
                g.turns++;
                if (DummyStandsBy()) {
                    // (nothing takes what player 1 sends, so it is thrown away once the turn that made it is over)
                    SetRam8(Player(0) + kAttack, 0);
                    const int gems = kDummyAttacks[FeatureValue("dummy_attack") % 6];
                    if (gems && g.turns % kDummyAttackEvery == 0 && Ram8(Player(1) + kAttack) == 0) {
                        SetRam8(Player(1) + kAttack, static_cast<uint8_t>(gems));
                    }
                }
            }
        }
        g.stepBefore = step;
        std::vector<HudLine> lines;
        if (FeatureOn("attack_readout")) {
            Readout(lines);
        }
        if (training && FeatureOn("input_display")) {
            InputDisplay(lines);
        } else {
            g_inputs.clear();
        }
        SetHud(std::move(lines));
    }
} // namespace

// In a training session player 2 stands by unless told to play: its turns are left out (turn.cpp)
bool DummyStandsBy() {
    return FeatureOn("training_mode") && FeatureValue("dummy") == 0;
}

void TrainingBeforeFrame(Machine &machine) {
    BeforeFrame(machine);
}

void TrainingFrame(Machine &machine) {
    Frame(machine);
}

void TrainingState(Archive &ar) {
    ar.Mark(0x5452414E);
    ar.Pod(g);
    ar.Vec(g_roundStart);
}

// The menu's pages
constexpr const char *kModes = "Alternative modes", *kTraining = "Training mode", *kDummy = "Dummy";

void RegisterTraining() {
    Game &game = TheGame();
    // The menu's "Alternative modes" page, at its head: ways of running the game that the board does not have
    game.pages.push_back({kModes, "", "Other ways to run the game: training, attract mode, the computer against itself, the colour editor.", true});
    game.pages.push_back({kTraining, kModes, "A two-player match that nobody loses, and the aids for practising in it."});
    game.pages.push_back({kDummy, kTraining, "What player 2 does in a training session."});
    game.commands.push_back({"training_start", kTraining, "Start",
                             "Starts a training session: a two-player match, straight to the choice of characters (choose player "
                             "1's, then player 2's with the same controls). Player 2 stands by, and the round starts over when "
                             "your field fills up.",
                             StartTraining});
    game.commands.push_back({"training_restart", kTraining, "Start over",
                             "In a training session: the round as it began. Backspace or a gamepad's left shoulder does this while playing.",
                             StartOver, kKeyRestart});
    game.commands.push_back({"training_diamond", kTraining, "A diamond next",
                             "In a training session: the pair after the one shown as next brings the diamond.",
                             [](Machine &) {
                                 if (FeatureOn("training_mode")) {
                                     DiamondNext(Player(0));
                                 }
                             }});
    game.features.push_back({"training_crash", kTraining, "Crash gems",
                             "In a training session, what player 1's pairs bring: as the game deals them, a crash gem in every "
                             "pair, or none at all (to build without setting anything off).",
                             0, {"As the game deals", "In every pair", "None"}});
    game.commands.push_back({"training_clear", kTraining, "Empty my field",
                             "In a training session: takes every gem out of player 1's field.",
                             [](Machine &) {
                                 if (!FeatureOn("training_mode") || !RoundPlayed()) {
                                     return;
                                 }
                                 for (int column = 0; column < kFieldColumns; column++) {
                                     for (int row = 0; row < kFieldRows; row++) {
                                         SetRam16(Cell(Player(0), column, row), 0);
                                         SetRam16(Cell(Player(0), column, row) + kFieldExtra, 0);
                                     }
                                 }
                             }});
    game.features.push_back({"training_speed", kTraining, "Pairs fall",
                             "In a training session, how fast player 1's pairs fall: faster as the round goes on, as the game "
                             "has it; as at the start of a round throughout; or slowly, for time to think. Takes effect "
                             "with the next pair.",
                             0, {"As the game has it", "As at the start", "Slowly"}});
    game.features.push_back({"training_mode", kTraining, "Training session",
                             "Player 2 is a dummy (the Dummy page says what it does) and the round starts over when player 1's "
                             "field fills up. (\"Start\" above switches this on and takes the game to the "
                             "character choice; Reset ends it.)",
                             kFeatureSession});
    game.features.push_back({"input_display", kTraining, "Input display",
                             "In a training session, player 1's controls as the game reads them, the newest at the top, each with the "
                             "frames it was held: the stick in numpad notation (5 is the middle, 4 left, 6 right, 2 down), A and B the "
                             "two buttons that turn the pair. Shown over player 2's field."});
    game.features.push_back({"attack_readout", kTraining, "Attack readout",
                             "Over a player's field after each break: the gems broken and the chain, the score, the counter gems "
                             "the break made, and how many of the opponent's it cancelled or the turn's total."});
    game.features.push_back({"landing_guide", "Game", "Landing guide",
                             "Outlines where the two gems of the pair you are steering would come to rest if you let it fall now."});
    game.features.push_back({"dummy", kDummy, "Player 2",
                             "In a training session: player 2 stands by (its side does nothing and takes no counter gems), or "
                             "plays with its own controls.",
                             0, {"Stands by", "Plays"}});
    game.features.push_back({"dummy_attack", kDummy, "Attacks with",
                             "While it stands by, player 2 sends this many counter gems at the start of every fourth turn of "
                             "yours, to practise cancelling them. They fall in the pattern of player 2's character.",
                             0, {"Nothing", "3 counter gems", "6", "12", "24", "48"}});
    game.features.push_back({"no_attack_2", "Cheats", "Player 2 cannot attack", "Player 2's breaks (the opponent's, in a one-player game) make no counter gems."});
    game.features.push_back({"no_attack", "Cheats", "Player 1 cannot attack", "Player 1's breaks make no counter gems."});
    game.features.push_back({"challenger", "Cheats", "Hidden challenger appears", "Against the computer, the hidden challenger comes in place of the seventh opponent, as for a player who has earned it (if no continue has been used)."});
    game.features.push_back({"quick_win", "Cheats", "Player 1 wins quickly", "Player 1 sends forty counter gems with every pair, and player 1's field is emptied before it can fill."});
}

} // namespace cps2recomp
