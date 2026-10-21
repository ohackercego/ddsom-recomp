// SPDX-License-Identifier: GPL-3.0-only
// Copyright (C) 2026 Tommy McLeroy
//
// Super Gem Fighter Mini Mix: the board's settings (the operator's, in its test menu: Configuration, System and Game)
// as features on the launcher's Board page, as in the Alpha games. Each is "As the board is set" unless chosen, and a
// choice is what the test menu would have set: the game reads its settings from the settings memory once, as the
// board starts (001796, called at 001088), into the variables below; the hook at 00108E, where that call returns,
// puts the choices in those variables. The board's settings memory itself is not changed, nor what the test menu
// shows and saves.
//
// The settings record (A5-7398, as the settings memory holds it) and the variable each goes to (from A5):
//   +01 continue      118  0 off, 1 on (on)    +10 difficulty  120  0 easy, 1 normal, 2-6 hard 1-5, 7 expert (hard 3)
//   +03 demo sound    119  0 off, 1 on (on)    +11 damage      121  0 low, 1 normal, 2 high, 3 maximum (normal)
//                                              +12 timer speed 160  0 slow, 1 normal, 2 fast, 3 fastest (slow)
//                                              +13 game speed  15B  0-3 turbo 1-4, 4 free select (free select)
//                                              +14 max round   122  1, 2, 3: one, three, five rounds (three)
//                                              +15 event       125  0 off, 1 one match (no continue) (off)
//                                              +16 join-in     124  0 off, 1 on (on)
// (the values and their names from the test menu's pages, each stepped through with the variable read after each
// step; in brackets the value the test menu shows as the board's own; 00183C on copies them)
#include "pocket.h"

#include "machine.h"
#include "state.h"

#include <iterator>

namespace cps2recomp {

namespace {
    constexpr uint32_t kVars = 0xFF8000; // (A5)
    constexpr const char *kPage = "Board";

    struct Setting {
        const char *name;
        uint32_t variable;
        int first; // the value of the first choice after "As the board is set"
    };
    constexpr Setting kSettings[] = {
        {"board_difficulty", kVars + 0x120, 0}, {"board_damage", kVars + 0x121, 0},   {"board_timer", kVars + 0x160, 0},
        {"board_speed", kVars + 0x15B, 0},      {"board_rounds", kVars + 0x122, 1},   {"board_event", kVars + 0x125, 0},
        {"board_join_in", kVars + 0x124, 0},    {"board_continue", kVars + 0x118, 0}, {"board_demo_sound", kVars + 0x119, 0},
    };
    constexpr uint32_t kContinue = kVars + 0x118, kEvent = kVars + 0x125;

    // The board's own values of what was chosen, to put back for its test menu. (All four are kept with the game's
    // state, BoardState: they say what the variables in RAM hold, so a state loaded or run-ahead's frame taken back
    // must bring them back with the RAM.)
    uint8_t g_board[std::size(kSettings)];
    uint8_t g_continue = 0;
    bool g_chosen = false; // the choices are in the variables
    bool g_again = false;  // they were taken out for the test menu, and go back in when a game begins

    // The choices put in the variables (the board's own kept first)
    void PutChoices() {
        g_chosen = false;
        for (size_t n = 0; n < std::size(kSettings); n++) {
            g_board[n] = Ram8(kSettings[n].variable);
        }
        g_continue = Ram8(kContinue); // (a one-match event clears it below)
        for (const Setting &setting : kSettings) {
            const int choice = FeatureValue(setting.name);
            if (choice > 0) {
                SetRam8(setting.variable, static_cast<uint8_t>(setting.first + choice - 1));
                g_chosen = true;
            }
        }
        if (g_chosen && Ram8(kEvent)) {
            SetRam8(kContinue, 0); // (no continue in the one-match event, as the game has it: 0018B6)
        }
    }

    // 00108E: the board's settings have been read
    bool SettingsRead(M68K &) {
        g_again = false;
        PutChoices();
        return false; // (and on with the game's own code)
    }
    // 0079D8: a game begins (it copies difficulty to A5+13E, 007A64)
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
// not read the settings again, so the choices go back in when a game next begins (demo sound, which the show
// uses, at the next reset)
void BoardBeforeFrame(Machine &machine) {
    if (g_chosen && machine.testSwitch) {
        for (size_t n = 0; n < std::size(kSettings); n++) {
            SetRam8(kSettings[n].variable, g_board[n]);
        }
        SetRam8(kContinue, g_continue);
        g_chosen = false;
        g_again = true;
    }
}

void BoardState(Archive &ar) {
    ar.Mark(0x424F4152); // BOAR
    ar.Bytes(g_board, sizeof g_board);
    ar.Pod(g_continue);
    ar.Pod(g_chosen);
    ar.Pod(g_again);
}

void RegisterBoard() {
    Game &game = TheGame();
    game.pages.push_back({kPage, "", "The board's own settings, as its test menu has them (Configuration): each as the board is set "
                                     "unless chosen here. Taken when the game starts or is reset."});
    constexpr const char *kAsSet = "As the board is set";
    game.features.push_back({"board_difficulty", kPage, "Difficulty", "How well the computer fights.", 0,
                             {kAsSet, "Easy", "Normal", "Hard 1", "Hard 2", "Hard 3", "Hard 4", "Hard 5", "Expert"}});
    game.features.push_back({"board_damage", kPage, "Damage level", "How much damage a hit does.", 0,
                             {kAsSet, "Low", "Normal", "High", "Maximum"}});
    game.features.push_back({"board_timer", kPage, "Timer speed", "How fast the round's clock runs.", 0,
                             {kAsSet, "Slow", "Normal", "Fast", "Fastest"}});
    game.features.push_back({"board_speed", kPage, "Game speed",
                             "The game's speed: one of four turbo speeds, or a free choice of speed for the players.", 0,
                             {kAsSet, "Turbo 1", "Turbo 2", "Turbo 3", "Turbo 4", "Free select"}});
    game.features.push_back({"board_rounds", kPage, "Max round", "Rounds a match lasts at most.", 0,
                             {kAsSet, "1 round", "3 rounds", "5 rounds"}});
    game.features.push_back({"board_event", kPage, "Event", "Off, or one match only (and no continue).", 0, {kAsSet, "Off", "One match"}});
    game.features.push_back({"board_join_in", kPage, "Join-in", "Whether a second player can join a game against the computer.", 0,
                             {kAsSet, "Off", "On"}});
    game.features.push_back({"board_continue", kPage, "Continue", "Whether a game over can be continued.", 0, {kAsSet, "Off", "On"}});
    game.features.push_back({"board_demo_sound", kPage, "Demo sound", "Whether the attract mode plays sound.", 0, {kAsSet, "Off", "On"}});
    AddHook(HookAt(0x00108E), "SettingsRead", SettingsRead);
    AddHook(HookAt(0x0079D8), "GameBegins", GameBegins);
}

} // namespace cps2recomp
