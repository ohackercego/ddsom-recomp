// SPDX-License-Identifier: GPL-3.0-only
// Copyright (C) 2026 Tommy McLeroy
//
// Super Puzzle Fighter II Turbo: the board's settings (the operator's, in its test menu: Configuration) as
// features on the launcher's Board page. Each is "As the board is set" unless chosen, and a choice is what the
// test menu would have set: the game reads its settings from the settings memory once, as the board starts
// (01DE52, called at 001070), into the variables below; the hook at 001076, where that call returns, puts the
// choices in those variables. The board's settings memory itself is not changed, nor what the test menu shows
// and saves.
//
// The settings record (FF0F90, as the settings memory holds it) and the variable each goes to (from A5):
//   +01 continue        98  0 off, 1 on            +10 difficulty  A0  0 easiest ... 3 medium ... 7 hardest
//   +03 demo sound      99  0 off, 1 on            +11 damage      A1  0 low, 1 normal, 2 high, 3 maximum
//                                                  +12 game speed  DE  0 slow, 1 normal, 2 fast, 3 fastest
//                                                  +13 game mode   A5  0-3: types A-D (B standard; D is one
//                                                                       match, and has no continue)
//                                                  +14 1P rounds   A2  1, 2, 3: one, three, five rounds
//                                                  +15 2P rounds   A3  the same
//                                                  +16 join-in     A4  0 off, 1 on
//                                                  +17 handicap    BB  0 off, 1 on
// (the values and their names from the test menu's pages, stepped through; the defaults as a new settings
// memory has them: medium, normal, normal, type B, 1 round, 3 rounds, on, on)
#include "spf2.h"

#include "machine.h"

#include <iterator>

namespace cps2recomp {

namespace {
    using namespace spf2;

    constexpr const char *kPage = "Board";
    constexpr uint8_t kTypeOneMatch = 3; // (game mode D: the game clears continue for it, 01DF64)

    struct Setting {
        const char *name;
        uint32_t variable;
        int first; // the value of the first choice after "As the board is set"
    };
    constexpr Setting kSettings[] = {
        {"board_difficulty", kVars + 0xA0, 0}, {"board_damage", kVars + 0xA1, 0}, {"board_speed", kVars + 0xDE, 0},
        {"board_mode", kVars + 0xA5, 0},       {"board_rounds_1p", kVars + 0xA2, 1}, {"board_rounds_2p", kVars + 0xA3, 1},
        {"board_join_in", kVars + 0xA4, 0},    {"board_handicap", kVars + 0xBB, 0}, {"board_continue", kVars + 0x98, 0},
        {"board_demo_sound", kVars + 0x99, 0},
    };

    // The board's own values of what was chosen, to put back for its test menu
    uint8_t g_board[std::size(kSettings)];
    uint8_t g_continue = 0;
    bool g_chosen = false; // the choices are in the variables
    bool g_again = false;  // they were taken out for the test menu, and go back in when a game starts

    // The choices put in the variables (the board's own kept first)
    void PutChoices() {
        g_chosen = false;
        for (size_t n = 0; n < std::size(kSettings); n++) {
            g_board[n] = Ram8(kSettings[n].variable);
        }
        g_continue = Ram8(kVars + 0x98); // (game mode D clears it below)
        for (const Setting &setting : kSettings) {
            const int choice = FeatureValue(setting.name);
            if (choice > 0) {
                SetRam8(setting.variable, static_cast<uint8_t>(setting.first + choice - 1));
                g_chosen = true;
            }
        }
        if (g_chosen && Ram8(kVars + 0xA5) == kTypeOneMatch) {
            SetRam8(kVars + 0x98, 0); // (no continue in one-match mode, as the game has it)
        }
    }

    // 001076: the board's settings have been read
    bool SettingsRead(M68K &) {
        g_again = false;
        PutChoices();
        return false; // (and on with the game's own code)
    }
    // 00A046: a game begins (it copies difficulty, speed and continue from the variables, 00A0A8 on)
    bool GameBegins(M68K &) {
        if (g_again) {
            g_again = false;
            PutChoices();
        }
        return false;
    }
} // namespace

// Before every frame. The board's test switch pressed (its test menu shows, changes and saves the variables)
// puts the board's own values back, so that what the test menu saves is the board's; leaving the test menu does
// not read the settings again, so the choices go back in when a game next begins (GameBegins; demo sound, which
// the show uses, at the next reset)
void BoardBeforeFrame(Machine &machine) {
    if (g_chosen && machine.testSwitch) {
        for (size_t n = 0; n < std::size(kSettings); n++) {
            SetRam8(kSettings[n].variable, g_board[n]);
        }
        SetRam8(kVars + 0x98, g_continue);
        g_chosen = false;
        g_again = true;
    }
}

void RegisterBoard() {
    Game &game = TheGame();
    game.pages.push_back({kPage, "", "The board's own settings, as its test menu has them (Configuration): each as the board is "
                                     "set unless chosen here. Taken when the game starts or is reset."});
    constexpr const char *kAsSet = "As the board is set";
    game.features.push_back({"board_difficulty", kPage, "Difficulty", "How well the computer plays against you.", 0,
                             {kAsSet, "Easiest", "Very easy", "Easy", "Medium", "Medium hard", "Hard", "Very hard", "Hardest"}});
    game.features.push_back({"board_damage", kPage, "Damage level", "How many counter gems an attack sends.", 0,
                             {kAsSet, "Low", "Normal", "High", "Maximum"}});
    game.features.push_back({"board_speed", kPage, "Game speed", "How fast the pairs fall.", 0, {kAsSet, "Slow", "Normal", "Fast", "Fastest"}});
    game.features.push_back({"board_mode", kPage, "Game mode", "The board's game types; B is the standard one, D one match only.", 0,
                             {kAsSet, "Type A", "Type B (standard)", "Type C", "Type D (one match)"}});
    game.features.push_back({"board_rounds_1p", kPage, "Rounds against the computer", "Rounds a match against the computer lasts at most.", 0,
                             {kAsSet, "1 round", "3 rounds", "5 rounds"}});
    game.features.push_back({"board_rounds_2p", kPage, "Rounds, two players", "Rounds a match between two players lasts at most.", 0,
                             {kAsSet, "1 round", "3 rounds", "5 rounds"}});
    game.features.push_back({"board_join_in", kPage, "Join-in", "Whether a second player can join a game against the computer.", 0,
                             {kAsSet, "Off", "On"}});
    game.features.push_back({"board_handicap", kPage, "Handicap", "Whether two players choose handicap levels.", 0, {kAsSet, "Off", "On"}});
    game.features.push_back({"board_continue", kPage, "Continue", "Whether a game over can be continued.", 0, {kAsSet, "Off", "On"}});
    game.features.push_back({"board_demo_sound", kPage, "Demo sound", "Whether the attract mode plays sound.", 0, {kAsSet, "Off", "On"}});
    AddHook(HookAt(0x001076), "SettingsRead", SettingsRead);
    AddHook(HookAt(0x00A046), "GameBegins", GameBegins);
}

} // namespace cps2recomp
