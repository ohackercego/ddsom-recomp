// SPDX-License-Identifier: GPL-3.0-only
// Copyright (C) 2026 Tommy McLeroy
//
// Super Puzzle Fighter II Turbo: the alternative modes besides training (training.cpp), each started from the
// menu's "Alternative modes" page and ended when the board is reset or another is started.
//
// - Attract mode: the game's own show on a loop, with no coin in. Its parts are steps of the game's first step
//   word (spf2.h, kGameOn): the notices (2 to 12), the introduction (14: the logo, a character's introduction, a
//   demonstration match, the title), the how-to-play dojo (16, 18), the score ranking (1A, 1C), another
//   demonstration match (1E, 20), the notices, the title and the ranking again (22 to 2C), and back to the
//   start (2E, 30). "The intro only", "the matches only" and "the dojo only" leave the rest out by moving the
//   step on where a part that is wanted ends, as the game does it.
// - CPU battle: a two-player match in which the computer plays both sides, one after another
//   with the characters chosen or at random. The board is started and the way to the match is worked unseen;
//   when the match is over (the winner's screen shown) the board is started again for the next.
// - The colour editor's viewer (colours.cpp): the same way into a match, with the character to be edited as
//   player 1's; then nobody plays (both players' turns are left out, turn.cpp) and player 1's field holds a
//   gem of every kind and colour, so that the characters' and the gems' colours can be seen while they change.
#include "spf2.h"

#include "machine.h"
#include "state.h"

#include <random>

namespace cps2recomp {

namespace {
    using namespace spf2;

    // The menu's pages
    constexpr const char *kModes = "Alternative modes", *kAttract = "Attract mode", *kWatch = "CPU battle";

    // The choice of characters (select.cpp has them all): the two characters on the screen, each a record like a
    // player's, and what each player's cursor is on
    constexpr uint32_t kOnScreen[2] = {kVars + 0xB00, kVars + 0xC00};
    constexpr uint32_t kChosen[2] = {0xFF0C82, 0xFF0C86};
    // The players choosing (bits: both, in the two-player mode), and those who have chosen a character
    constexpr uint32_t kChoosingNow = kVars + 0x8D, kCharactersChosen = 0xFF0C8D;
    // The computer's level in a player's record (difficulty.cpp), 0 to 31
    constexpr uint32_t kComputerLevel = 0xA8;

    enum class Mode : int32_t { None, Attract, Watch, Viewer };
    struct State {
        Mode mode = Mode::None;
        bool starting = false; // on the way: the board is starting up and the way to a match is worked (unseen)
        int32_t frames = 0;    //   for so many frames
        int32_t characters[2] = {0, 1}; // the match's characters (Watch, Viewer)
        bool inMatch = false;  // the match has begun (Watch: its end starts the next)
        bool filled = false;   // the viewer's field has been filled
    } g;
    bool g_fast = false; // this file asked for the game to run unseen

    constexpr int kStartMost = 6000; // frames after which the way to a match is given up

    void Fast(bool on) {
        if (on != g_fast) {
            g_fast = on;
            SetFastForward(on);
        }
    }

    void End() {
        if (g.mode == Mode::None) {
            return;
        }
        g = State();
        Fast(false);
    }

    // (the first step word is 2 in the notices of the game's show as well: a game has players in)
    bool InGame() {
        return Ram16(kGameOn) == 2 && Ram8(kPlayersIn) != 0;
    }
    bool ChoosingMode() {
        return InGame() && Ram16(kGameStep) == kGameChoosing && Ram16(kMatchStep) == 2 && Ram16(kRoundStep) == kChoosingMode;
    }
    bool ChoosingCharacters() {
        return InGame() && Ram16(kGameStep) == kGameChoosing && Ram16(kMatchStep) == 2 && Ram16(kRoundStep) == kChoosingCharacters;
    }
    bool InMatch() {
        return InGame() && Ram16(kGameStep) == kGameMatch;
    }
    bool RoundPlayed() {
        return InMatch() && Ram16(kMatchStep) == kMatchRound && Ram16(kRoundStep) == kRoundPlayed;
    }

    // --- attract mode ---

    // The parts of the game's loop, by kGameOn (each part is two steps: a set-up, then the part)
    constexpr uint16_t kNotices = 0x02, kIntro = 0x14, kDojo = 0x16, kRanking = 0x1A, kDemo = 0x1E, kAfterDemo = 0x22;
    enum { kShowAll, kShowIntro, kShowMatches, kShowDojo };
    // Byte: which of eight demonstration matches the one after the dojo is (characters and stage, 002CF4), in twos;
    // stepped on as the loop goes on (002610), which the matches only leave out (the game masks it)
    constexpr uint32_t kDemoNumber = kVars - 0x7398;
    constexpr uint64_t kBoardStarted = 300; // frames: the board's start is well on (the warning; the loop starts at 458)

    void StartAttract(Machine &machine) {
        End();
        SetFeature("training_mode", false);
        g.mode = Mode::Attract;
        g.starting = true;
        machine.Reset();
    }

    // The game's step moved to another part of the loop, as the game does it: the step, with the steps under it
    // and their timers cleared
    void GoTo(uint16_t part) {
        SetRam16(kGameOn, part);
        for (uint32_t at = kGameOn + 2; at < kSubStep + 4; at += 2) {
            SetRam16(at, 0);
        }
    }

    void AttractFrame() {
        const uint16_t part = Ram16(kGameOn);
        const int show = FeatureValue("attract_show");
        if (show == kShowAll || part < kNotices) {
            return; // (0: the loop's own start, between the board's start and the notices)
        }
        const uint16_t from = show == kShowIntro ? kIntro : show == kShowMatches ? kDemo : kDojo;
        const uint16_t to = show == kShowIntro ? kDojo : show == kShowMatches ? kAfterDemo : kRanking;
        if (part < from || part >= to) {
            // (the next of the demonstration matches, as the game steps on to it where the part left out begins)
            if (show == kShowMatches && part >= to) {
                SetRam8(kDemoNumber, static_cast<uint8_t>(Ram8(kDemoNumber) + 2));
            }
            GoTo(from);
        }
    }

    // --- the way to a match (Watch, Viewer) ---

    // Start, the two-player mode and both characters chosen, with the game running unseen. (Free play is on
    // meanwhile: coins.cpp.)
    void StartToMatch(Machine &machine, Mode mode) {
        g.mode = mode;
        g.starting = true;
        g.frames = 0;
        g.inMatch = false;
        g.filled = false;
        machine.Reset();
        Fast(true);
    }

    // The controls on the way: Start for player 1, the two-player mode, Start for player 2, then each player's
    // character put under the cursor and chosen with a button (a hidden one too, as the codes would)
    void Boot(Machine &machine) {
        uint16_t &one = machine.players[0], &two = machine.players[1];
        const bool press = g.frames % 16 < 6;
        one = two = 0;
        if (++g.frames > kStartMost) {
            End();
            return;
        }
        if (ChoosingMode()) {
            one = !press ? 0 : Ram8(kModeCursor) == 0 ? kRight : kPunch1;
        } else if (ChoosingCharacters() && Ram8(kChoosingNow) == 3) {
            for (int n = 0; n < 2; n++) {
                if (!(Ram8(kCharactersChosen) & (1 << n))) {
                    SetRam8(kChosen[n], static_cast<uint8_t>(g.characters[n]));
                    SetRam8(kOnScreen[n] + kCharacter, static_cast<uint8_t>(g.characters[n]));
                    (n == 0 ? one : two) = press ? kPunch1 : 0;
                    break;
                }
            }
        } else if (ChoosingCharacters()) {
            two = press ? kStart : 0;
        } else if (InMatch()) {
            // (the match's screens before the round: a button hurries them)
            one = press ? kPunch1 : 0;
        } else {
            one = press ? kStart : 0;
        }
    }

    // --- CPU battle ---

    constexpr int kCharacterCount = 11; // the eight, and the hidden three (8 Devilot, 9 Akuma, 10 Dan)
    constexpr uint8_t kStrengths[] = {4, 12, 20, 31};

    int Pick(const char *feature, int other) {
        static std::mt19937 random(std::random_device{}());
        const int choice = FeatureValue(feature);
        if (choice > 0) {
            return choice - 1;
        }
        // Random: one of the eight (and not the other side's)
        int character;
        do {
            character = std::uniform_int_distribution<int>(0, 7)(random);
        } while (character == other);
        return character;
    }

    void StartWatch(Machine &machine) {
        End();
        SetFeature("training_mode", false);
        g.characters[0] = Pick("watch_1", -1);
        g.characters[1] = Pick("watch_2", g.characters[0]);
        StartToMatch(machine, Mode::Watch);
    }

    void WatchFrame(Machine &machine) {
        if (InMatch()) {
            g.inMatch = true;
            // Both sides the computer's, at the strength chosen
            for (int n = 0; n < 2; n++) {
                SetRam8(Player(n) + kComputer, 1);
                SetRam8(Player(n) + kComputerLevel, kStrengths[FeatureValue("watch_strength") % 4]);
            }
        } else if (g.inMatch) {
            // The match is over (the winner has been shown): the next
            const Mode mode = g.mode;
            g.characters[0] = Pick("watch_1", -1);
            g.characters[1] = Pick("watch_2", g.characters[0]);
            StartToMatch(machine, mode);
        }
    }

    // --- the colour editor's viewer ---

    // Player 1's field: a row for each colour (red, blue, yellow, green: the game's palettes 0-3), each with
    // gems, a crash gem and a counter gem; and the diamond
    void FillField() {
        const uint32_t player = Player(0);
        for (int column = 0; column < kFieldColumns; column++) {
            for (int row = 0; row < kFieldRows; row++) {
                SetRam16(Cell(player, column, row), 0);
                SetRam16(Cell(player, column, row) + kFieldExtra, 0);
            }
        }
        constexpr uint16_t kColours[] = {kRed, kBlue, kYellow, kGreen};
        for (int row = 0; row < 4; row++) {
            const uint16_t colour = kColours[row];
            const uint16_t gems[kFieldColumns] = {colour, colour, static_cast<uint16_t>(kCrash + colour),
                                                  static_cast<uint16_t>(kCounter | ((colour << 4 | 5) << 8)), colour, colour};
            for (int column = 0; column < kFieldColumns; column++) {
                SetRam16(Cell(player, column, row), gems[column]);
            }
        }
        SetRam16(Cell(player, 2, 4), kDiamond);
    }

    void ViewerFrame() {
        if (RoundPlayed() && !g.filled) {
            g.filled = true;
            FillField();
        }
    }
} // namespace

// The menu's modes, for the files that need to know: training.cpp starts its own way in, title.cpp leaves the
// skipping to them
bool AttractMode() {
    return g.mode == Mode::Attract;
}
bool ColourViewer() {
    return g.mode == Mode::Viewer;
}
bool InAMode() {
    return g.mode != Mode::None || FeatureOn("training_mode");
}
// Free play while a mode works its way to a match (coins.cpp)
bool ModeNeedsFreePlay() {
    return g.mode == Mode::Watch || g.mode == Mode::Viewer;
}
// In the viewer nobody plays: both players' turns are left out (turn.cpp)
bool ViewerStandsBy() {
    return g.mode == Mode::Viewer && g.filled;
}
// The viewer: started for a character (the colour editor's), and the character it shows
void StartColourViewer(Machine &machine, int character) {
    End();
    SetFeature("training_mode", false);
    g.characters[0] = character;
    g.characters[1] = character == 6 ? 0 : 6; // (Felicia, whose colours are the plainest, beside; Morrigan beside her)
    StartToMatch(machine, Mode::Viewer);
}
int ColourViewerCharacter() {
    return g.mode == Mode::Viewer ? g.characters[0] : -1;
}
bool ColourViewerReady() {
    return g.mode == Mode::Viewer && g.filled;
}
void EndModes() {
    End();
}

void ModesBeforeFrame(Machine &machine) {
    if (g.mode == Mode::Attract) {
        // Start or a coin ends it, and is passed on; nothing else reaches the game
        if ((machine.players[0] | machine.players[1]) & (kStart | kCoin)) {
            End();
        } else {
            machine.players[0] = machine.players[1] = 0;
        }
    } else if (g.mode == Mode::Watch || g.mode == Mode::Viewer) {
        if (g.starting) {
            Boot(machine);
        } else if (g.mode == Mode::Watch) {
            // (nothing a person does reaches the game but the menu)
            machine.players[0] = machine.players[1] = 0;
        }
    }
}

void ModesFrame(Machine &machine) {
    if (g.mode == Mode::Attract) {
        // The board was reset (Machine::frame is 0 again) or a game begun: attract mode is over
        // (the board's own start steps the word 2 to 8 too, before it is 0 again and the loop begins)
        if (g.starting) {
            g.starting = !(Ram16(kGameOn) == 0 && machine.frame > kBoardStarted);
            Fast(g.starting && FeatureValue("attract_show") != kShowAll); // (unseen when only a part is wanted)
        } else if (machine.frame <= 1 || InGame()) {
            End();
        }
        if (g.mode == Mode::Attract && !g.starting) {
            AttractFrame();
        }
        return;
    }
    if (g.mode == Mode::None) {
        return;
    }
    if (g.starting && RoundPlayed()) {
        g.starting = false;
        Fast(false);
    }
    if (g.mode == Mode::Watch) {
        WatchFrame(machine);
    } else {
        ViewerFrame();
    }
}

void ModesState(Archive &ar) {
    ar.Mark(0x4D4F4445);
    ar.Pod(g);
    if (!ar.Saving() && !RunningAhead()) {
        Fast(g.mode != Mode::None && g.starting);
    }
}

void RegisterModes() {
    Game &game = TheGame();
    game.pages.push_back({kAttract, kModes, "The game's own show on a loop, with no coin in: the introduction, the title, "
                                            "demonstration matches, the how-to-play dojo and the rankings."});
    game.pages.push_back({kWatch, kModes, "The computer plays both sides, match after match."});
    game.commands.push_back({"attract_start", kAttract, "Start",
                             "Starts attract mode: the game's own show, over and over. Start (or a coin) ends it.", StartAttract});
    game.features.push_back({"attract_show", kAttract, "Shows",
                             "What attract mode plays: everything the board does, the introduction over and over (with its "
                             "demonstration match and the title), one demonstration match after another, or the how-to-play "
                             "dojo.",
                             0, {"Everything", "The intro only", "The matches only", "The dojo only"}});
    game.commands.push_back({"watch_start", kWatch, "Start",
                             "Starts a two-player match that the computer plays on both sides; when it is over the next "
                             "begins. Reset ends it.",
                             StartWatch});
    const std::vector<const char *> characters = {"Random", "Morrigan", "Chun-Li", "Ryu", "Ken", "Hsien-Ko", "Donovan",
                                                  "Felicia", "Sakura", "Devilot", "Akuma", "Dan"};
    game.features.push_back({"watch_1", kWatch, "Player 1", "Player 1's character, or Random: one of the eight, a different one each match.", 0,
                             characters});
    game.features.push_back({"watch_2", kWatch, "Player 2", "Player 2's character, or Random: one of the eight, a different one each match.", 0,
                             characters});
    game.features.push_back({"watch_strength", kWatch, "Strength",
                             "How well the computer plays both sides: its level, which the game counts from 0 to 31 (here 4, "
                             "12, 20 or 31). The game's own opponents grow stronger along the ladder in the same way.",
                             0, {"Weak", "Middling", "Strong", "Strongest"}});
    TheGame().noRunAhead = [] { return g.mode != Mode::None; };
}

} // namespace cps2recomp
