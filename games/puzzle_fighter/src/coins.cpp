// SPDX-License-Identifier: GPL-3.0-only
// Copyright (C) 2026 Tommy McLeroy
//
// Super Puzzle Fighter II Turbo: free play, and the operator's settings.
#include "spf2.h"

#include "board.h"

namespace cps2recomp {

bool ModeNeedsFreePlay(); // modes.cpp

namespace {
    using namespace spf2;

    bool forced = false; // free play is on because of the feature, not the operator's settings
} // namespace

// The game has free play of its own: one of the operator's coin settings. When the game starts it reads
// that setting and, for free play, sets kFreePlay and the two coin words; from then on Start lets a player
// in without a coin and the screen says "FREE PLAY" where it would count credits. The feature leaves the
// operator's settings alone and sets what the game sets. (A training session and the menu's other modes that go to a match need no coins either.)
void CoinsFrame(Machine &) {
    if (FeatureOn("free_play") || FeatureOn("training_mode") || ModeNeedsFreePlay()) {
        if (!Ram8(kFreePlay)) {
            SetRam8(kFreePlay, 1);
            SetRam16(kCoinsPerCredit1, 0x0101);
            SetRam16(kCoinsPerCredit2, 0x0101);
            forced = true;
        }
    } else if (forced) {
        // Switched off: Start needs a coin again (the coin words are put right by the game at its next reset)
        SetRam8(kFreePlay, Ram8(kSettings) == kCoinFreePlay);
        forced = false;
    }
}

void RegisterCoins() {
    // The settings memory, read and written directly (board.h). The two routines are Capcom's for every
    // CPS-2 game; these are where this game has them and their variables.
    SettingsMemory memory;
    memory.readWords = HookAt(0x01E62C);
    memory.writeWords = HookAt(0x01E6F4);
    memory.buffer = -0x6F70;
    memory.count = -0x6F6A;
    memory.start = -0x6F66;
    memory.busy = 0x87;
    AddSettingsMemory(memory);
    TheGame().features.push_back({"free_play", "Game", "Free play", "Start works without a coin.", kFeatureEnhanced});
}

} // namespace cps2recomp
