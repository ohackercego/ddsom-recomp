// SPDX-License-Identifier: GPL-3.0-only
// Copyright (C) 2026 Tommy McLeroy
//
// Super Gem Fighter Mini Mix: free play (an optional feature: the launcher's Features page and the in-game menu
// switch it).
#include "pocket.h"

namespace cps2recomp {

namespace {
    using namespace pocket;

    bool forced = false; // free play is on because of the feature, not the operator's settings
} // namespace

// The game has free play of its own: one of the operator's coin modes (pocket.h). The feature leaves the operator's
// settings alone and sets what the game sets for that mode; from then on Start lets a player in without a coin and
// the screen says "FREE PLAY" where it would count credits.
void CoinsFrame(Machine &) {
    if (FeatureOn("free_play")) {
        if (!Ram8(kFreePlay)) {
            SetRam8(kFreePlay, 1);
            SetRam16(kCoinsPerCredit1, 0x0101);
            SetRam16(kCoinsPerCredit2, 0x0101);
            forced = true;
        }
    } else if (forced) {
        // Switched off: Start needs a coin again (the coin words are put right by the game at its next reset)
        SetRam8(kFreePlay, Ram8(kCoinMode) == kCoinFreePlay);
        forced = false;
    }
}

void RegisterCoins() {
    TheGame().features.push_back({"free_play", "Game", "Free play", "Start works without a coin.", kFeatureEnhanced});
}

} // namespace cps2recomp
