// SPDX-License-Identifier: GPL-3.0-only
// Copyright (C) 2026 Tommy McLeroy. Part of cps2recomp.
//
// A test of what the port reads from the keyboard for each player: it holds a key down and says what
// each player gets for it. The port's own code is used (port/bindings.h), so this is what the game
// would get.
//
//   bindings_test [players] [buttons]
#include "../../../port/bindings.h"

#include <algorithm>
#include <cstdio>
#include <string>
#include <vector>

int main(int argc, char **argv) {
    const int players = argc > 1 ? atoi(argv[1]) : 4;
    const std::string buttons = argc > 2 ? argv[2] : "Attack | Jump | Magic";
    const port::BoundTable &table = port::MakeBoundTable(players, buttons, false);
    const recomp::Bindings b = recomp::DefaultBindings(table.table);
    printf("the table: %d players, %d actions\n", table.table.players, table.table.count);

    // (the port's own reading of the keyboard: a key held, and what each player gets)
    std::vector<bool> keys(SDL_SCANCODE_COUNT, false);
    const auto read = [&](int player) {
        uint16_t pad = 0;
        for (int action = 0; action < table.table.count; action++) {
            const SDL_Scancode key = b.key[player][action];
            if (key != SDL_SCANCODE_UNKNOWN && keys[key]) {
                pad |= table.bits[action];
            }
        }
        return pad;
    };
    // (each key in turn: which players get something for it, and what)
    for (int action = 0; action < table.table.count; action++) {
        for (int player = 0; player < players; player++) {
            const SDL_Scancode key = b.key[player][action];
            if (key == SDL_SCANCODE_UNKNOWN) {
                continue;
            }
            std::fill(keys.begin(), keys.end(), false);
            keys[key] = true;
            uint16_t pads[4] = {};
            for (int p = 0; p < players; p++) {
                pads[p] = read(p);
            }
            printf("%-22s (the player %d's %s):", SDL_GetScancodeName(key), player + 1,
                   table.actions[action].name);
            for (int p = 0; p < players; p++) {
                if (pads[p]) {
                    printf(" the player %d gets %04x", p + 1, pads[p]);
                }
            }
            printf("\n");
        }
    }
    return 0;
}
