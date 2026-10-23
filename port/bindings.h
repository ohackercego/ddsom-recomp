// SPDX-License-Identifier: GPL-3.0-only
// Copyright (C) 2026 Tommy McLeroy. Part of cps2recomp.
//
// Which key and which gamepad button does what, for each player: the launcher's Controllers page writes
// the player's choices to settings.ini and the game reads them. The format and its reading are the recomp
// ports' own (recomp-common/launcher/src/bindings.h); here are this toolkit's actions and its usual layout.
//
//   [keyboard1]         a key by SDL's name for it ("Up", "A", "Return"), or "none"
//   up = Up
//   [gamepad1]          a button or a trigger by SDL's name ("a", "leftshoulder", "righttrigger"), or "none"
//   lp = x
//
// (and [keyboard2], [gamepad2] for player 2, and so on for a game with more). What is not in the file is the usual
// layout (MakeBoundTable). The first gamepad is player 1's, the second player 2's, and so on; a gamepad's left stick
// always works as the directions.
#pragma once

#include <SDL3/SDL.h>

#include <algorithm>
#include <cstdint>
#include <cstdlib>
#include <deque>
#include <string>
#include <vector>

#include "../recomp-common/launcher/src/bindings.h"

namespace port {

// What can be bound: the name in settings.ini, what the launcher calls it, and its bit of a player's
// controls (cps2recomp::Button in machine/machine.h; frontend_sdl.cpp checks they agree)
struct BoundAction {
    const char *name;
    const char *title;
    uint16_t bit;
};
inline constexpr BoundAction kBoundActions[] = {
    {"up", "Up", 1 << 3},          {"down", "Down", 1 << 2},        {"left", "Left", 1 << 1},
    {"right", "Right", 1 << 0},    {"lp", "Light punch", 1 << 4},   {"mp", "Medium punch", 1 << 5},
    {"hp", "Hard punch", 1 << 6},  {"lk", "Light kick", 1 << 7},    {"mk", "Medium kick", 1 << 8},
    {"hk", "Hard kick", 1 << 9},   {"start", "Start", 1 << 10},     {"coin", "Coin", 1 << 11},
};
inline constexpr int kBoundActionCount = static_cast<int>(sizeof(kBoundActions) / sizeof(kBoundActions[0]));
inline constexpr int kFirstButton = 4, kButtons = 6; // (the six buttons' place among them)
inline constexpr int kBoundPlayersMost = recomp::kBindMaxPlayers;

using recomp::Bindings;
using recomp::PadInput;

// A table of what can be bound, and each action's bit (its order is the table's, which need not have them all)
struct BoundTable {
    recomp::BindAction actions[kBoundActionCount];
    uint16_t bits[kBoundActionCount];
    recomp::BindTable table;
};

// Which table: how many players the game has (game.cfg's `players`, two unless it says), and its own names for
// its buttons (game.cfg's `buttons`, "Attack|Jump|Shoot": a game with fewer than six names them; none: the six
// of a fighting game, punches and kicks).
//   A game's own table (`shared` false) has only the buttons it names, called so.
//   The table every game shares in the collection (`shared` true) has all six, each called as a fighting game
//   calls it and, after a dot, what the other games call it ("Light punch · Attack"), and as many players as the
//   game with the most. Its usual layout is a fighting game's, whatever the names.
// The usual layout. Keyboard, player 1: arrows; A S D punches (or the first three buttons), Z X C kicks; 1 Start;
// 5 a coin. Players 2 to 4 have no keys until some are chosen. A gamepad is laid out like a six-button panel:
// punches on the top row (West, North, right shoulder), kicks below (South, East, right trigger); Start; Back puts
// a coin in. A game that names its buttons has them (on its own) as a beat-'em-up's pad has them: the first on
// West, the second on South (jump, say), the third on East, then North and the shoulder and trigger.
//
// (the keyboard of the players after the first, for a game with more than one: each has its own block of keys,
// so that a keyboard alone is enough to play with the others. Player 2: I J K L for the four ways, U O P for the
// buttons, 2 for start, 6 for a coin. Player 3: the keypad's arrows, KP 1 KP 2 KP 3 for the buttons, KP 0 for start,
// KP . for a coin. Player 4: T F G H for the four ways, Y U I for the buttons, 8 for start, 9 for a coin, and the
// keys of the third player and of the fourth are the ones a gamepad does not use, so that a gamepad and a keyboard
// can be used at the same time. What is chosen in the launcher's Controllers page still has the last word.)
inline const BoundTable &MakeBoundTable(int players, const std::string &buttons, bool shared) {
    static constexpr SDL_Scancode kKeys[kBoundActionCount] = {
        SDL_SCANCODE_UP, SDL_SCANCODE_DOWN, SDL_SCANCODE_LEFT, SDL_SCANCODE_RIGHT, SDL_SCANCODE_A, SDL_SCANCODE_S,
        SDL_SCANCODE_D,  SDL_SCANCODE_Z,    SDL_SCANCODE_X,    SDL_SCANCODE_C,     SDL_SCANCODE_1, SDL_SCANCODE_5,
    };
    // (the keyboard of a game with a table of its own: the first player has the left shift for a coin
    // and the return for start, so that the number row is free for the players after the first. A game
    // that shares the collection's table keeps the usual layout, where the first player has 1 for start
    // and 5 for a coin.)
    static constexpr SDL_Scancode kGameKeys[kBoundActionCount] = {
        SDL_SCANCODE_UP, SDL_SCANCODE_DOWN, SDL_SCANCODE_LEFT, SDL_SCANCODE_RIGHT, SDL_SCANCODE_A, SDL_SCANCODE_S,
        SDL_SCANCODE_D,  SDL_SCANCODE_Z,    SDL_SCANCODE_X,    SDL_SCANCODE_C,     SDL_SCANCODE_RETURN, SDL_SCANCODE_LSHIFT,
    };
    // (the keyboard of the players after the first, so that a keyboard alone is enough to play with
    // them. The order of a table is up, down, left, right, and the buttons after them.
    // Each player after the first has keys of its own that no other player uses: the first player keeps
    // the arrows and the letters next to them (A S D and Z X C), so a game with more than one player
    // still plays the way it did when it had only one, and the others take what is left over.
    //   Player 2: the number pad's arrows, I O P and K L ; for the six buttons, 2 start, 3 coin.
    //   Player 3: T G F H for the four ways (T up, G down, F left, H right), V B N M , . for the six
    //             buttons, the number pad's Enter to start and + for a coin.
    //   Player 4: the number row's 8 4 6 9 for the four ways, Q W E and R Y U for the six buttons,
    //             7 start, 0 coin.
    // (a game with a table of its own gives its first player the return and the left shift, so the
    // number row is free; the layout above is conflict-free either way, since the first player's own
    // keys are only the arrows, A S D, Z X C and, at most, 1 and 5.)
    static constexpr SDL_Scancode kKeys2[kBoundActionCount] = {
        SDL_SCANCODE_KP_8, SDL_SCANCODE_KP_2, SDL_SCANCODE_KP_4, SDL_SCANCODE_KP_6, SDL_SCANCODE_I, SDL_SCANCODE_O,
        SDL_SCANCODE_P,    SDL_SCANCODE_K,    SDL_SCANCODE_L,    SDL_SCANCODE_SEMICOLON, SDL_SCANCODE_2, SDL_SCANCODE_3,
    };
    static constexpr SDL_Scancode kKeys3[kBoundActionCount] = {
        SDL_SCANCODE_T, SDL_SCANCODE_G, SDL_SCANCODE_F, SDL_SCANCODE_H, SDL_SCANCODE_V, SDL_SCANCODE_B,
        SDL_SCANCODE_N, SDL_SCANCODE_M, SDL_SCANCODE_COMMA, SDL_SCANCODE_PERIOD, SDL_SCANCODE_KP_ENTER, SDL_SCANCODE_KP_PLUS,
    };
    static constexpr SDL_Scancode kKeys4[kBoundActionCount] = {
        SDL_SCANCODE_8, SDL_SCANCODE_4, SDL_SCANCODE_6, SDL_SCANCODE_9, SDL_SCANCODE_Q, SDL_SCANCODE_W,
        SDL_SCANCODE_E, SDL_SCANCODE_R, SDL_SCANCODE_Y, SDL_SCANCODE_U, SDL_SCANCODE_7, SDL_SCANCODE_0,
    };
    static constexpr PadInput kPad[kBoundActionCount] = {
        {SDL_GAMEPAD_BUTTON_DPAD_UP, -1},   {SDL_GAMEPAD_BUTTON_DPAD_DOWN, -1},      {SDL_GAMEPAD_BUTTON_DPAD_LEFT, -1},
        {SDL_GAMEPAD_BUTTON_DPAD_RIGHT, -1}, {SDL_GAMEPAD_BUTTON_WEST, -1},          {SDL_GAMEPAD_BUTTON_NORTH, -1},
        {SDL_GAMEPAD_BUTTON_RIGHT_SHOULDER, -1}, {SDL_GAMEPAD_BUTTON_SOUTH, -1},     {SDL_GAMEPAD_BUTTON_EAST, -1},
        {-1, SDL_GAMEPAD_AXIS_RIGHT_TRIGGER}, {SDL_GAMEPAD_BUTTON_START, -1},        {SDL_GAMEPAD_BUTTON_BACK, -1},
    };
    static constexpr PadInput kNamedPad[kButtons] = {
        {SDL_GAMEPAD_BUTTON_WEST, -1}, {SDL_GAMEPAD_BUTTON_SOUTH, -1}, {SDL_GAMEPAD_BUTTON_EAST, -1},
        {SDL_GAMEPAD_BUTTON_NORTH, -1}, {SDL_GAMEPAD_BUTTON_RIGHT_SHOULDER, -1}, {-1, SDL_GAMEPAD_AXIS_RIGHT_TRIGGER},
    };
    // (made once for each kind asked for, and kept: the launcher and the game point into them)
    static std::deque<std::pair<std::string, BoundTable>> made;
    static std::deque<std::string> titles;
    players = std::max(1, std::min(players, kBoundPlayersMost));
    const std::string key = std::to_string(players) + (shared ? "s" : "o") + buttons;
    for (const auto &[which, table] : made) {
        if (which == key) {
            return table;
        }
    }
    std::vector<std::string> names;
    for (size_t from = 0; !buttons.empty() && names.size() < kButtons;) {
        const size_t bar = buttons.find('|', from);
        std::string name = buttons.substr(from, bar == std::string::npos ? std::string::npos : bar - from);
        name.erase(0, name.find_first_not_of(' '));
        name.erase(name.find_last_not_of(' ') + 1);
        names.push_back(name);
        if (bar == std::string::npos) {
            break;
        }
        from = bar + 1;
    }
    made.emplace_back(key, BoundTable{});
    BoundTable &t = made.back().second;
    int count = 0;
    for (int action = 0; action < kBoundActionCount; action++) {
        const int button = action - kFirstButton;
        const bool isButton = button >= 0 && button < kButtons;
        const bool named = isButton && button < static_cast<int>(names.size()) && !names[button].empty();
        // (a game with a table of its own binds only the buttons it names: the rest are not its buttons,
        // and binding them would put keys under a button the game has not got. A game with no names at
        // all, or the collection's shared table, binds all six)
        if (isButton && !shared && !names.empty() && !named) {
            continue; // (a button the game has not got)
        }
        // (a game that names some of its buttons but not all six: the ones it has not named keep the
        // fighting game's title, so that the launcher's Controllers page still shows a row for them)
        const char *title = kBoundActions[action].title;
        if (named) {
            titles.push_back(shared ? std::string(title) + " \u00b7 " + names[button] : names[button]);
            title = titles.back().c_str();
        }
        const PadInput pad = !shared && named ? kNamedPad[button] : kPad[action];
        recomp::BindAction &a = t.actions[count];
        a.name = kBoundActions[action].name;
        a.title = title;
        // (a game with a table of its own has the first player's coin on the left shift and its start on
        // the return, so that the number row is free for the players after the first)
        const SDL_Scancode *first = !shared ? kGameKeys : kKeys;
        a.key[0] = first[action];
        // (the players after the first have their own keys: a keyboard alone is enough to play with them)
        a.key[1] = kKeys2[action];
        a.key[2] = kKeys3[action];
        a.key[3] = kKeys4[action];
        a.pad[0] = a.pad[1] = a.pad[2] = a.pad[3] = pad;
        t.bits[count++] = kBoundActions[action].bit;
    }
    t.table = {t.actions, count, players};
    return t;
}

// The game's own table (shared false), or the collection's (true): see MakeBoundTable
#ifdef GAME_PLAYERS
inline int GamePlayers() {
    return atoi(GAME_PLAYERS);
}
#else
inline int GamePlayers() {
    return 2;
}
#endif
#ifdef GAME_BUTTONS
inline const char *GameButtons() {
    return GAME_BUTTONS;
}
#else
inline const char *GameButtons() {
    return "";
}
#endif

// The player's choices from settings.ini (the usual layout where it says nothing, or there is no file)
inline Bindings LoadBindings(const std::string &settingsPath, const BoundTable &table) {
    return recomp::LoadBindings(settingsPath, table.table);
}

// Some player has this gamepad input bound (then it does not also do what it does otherwise while playing)
inline bool PadInputBound(const Bindings &b, const BoundTable &table, PadInput input) {
    return recomp::PadInputBound(b, table.table, input);
}

} // namespace port
