// SPDX-License-Identifier: GPL-3.0-only
// Copyright (C) 2026 Tommy McLeroy
//
// Super Gem Fighter Mini Mix: the hidden characters, Dan and Akuma, chosen with no code (the Game page's "Hidden
// characters"): Start held as a player chooses Ryu gives Akuma, as it chooses Ken Dan. The choice is the game's own
// (017598: a button pressed on the cursor's character), with the character changed where the game takes it (0175AE);
// the button still says the colours.
#include "pocket.h"

namespace cps2recomp {

namespace {
    using namespace pocket;

    constexpr uint8_t kRyu = 0, kKen = 1, kDan = 10, kAkuma = 11;
    bool g_start[2] = {false, false}; // each player's Start held this frame

    int PlayerOf(uint32_t record) {
        record &= 0xFFFFFF;
        return record == Player(0) ? 0 : record == Player(1) ? 1 : -1;
    }

    // 0175AE: move.b 0x381(a6),d0, the character the player has chosen (A6)
    bool Chosen(M68K &c) {
        const uint32_t player = c.a[6] & 0xFFFFFF;
        const int n = PlayerOf(player);
        if (n < 0 || !FeatureOn("hidden_characters") || !g_start[n]) {
            return false;
        }
        const uint8_t who = Ram8(player + kCharacter);
        if (who == kRyu || who == kKen) {
            SetRam8(player + kCharacter, who == kRyu ? kAkuma : kDan);
        }
        return false;
    }
} // namespace

void ChoosingBeforeFrame(Machine &machine) {
    for (int n = 0; n < 2; n++) {
        g_start[n] = (machine.players[n] & kStart) != 0;
    }
}

void RegisterChoosing() {
    AddHook(HookAt(0x0175AE), "Chosen", Chosen);
    TheGame().features.push_back({"hidden_characters", "Game", "Hidden characters",
                                  "The two hidden characters with no code: hold Start as you choose Ryu for Akuma, or Ken for Dan "
                                  "(the button still chooses the colours)."});
}

} // namespace cps2recomp
