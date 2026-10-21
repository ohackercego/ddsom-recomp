// SPDX-License-Identifier: GPL-3.0-only
//
// Street Fighter Alpha: free play (an optional feature: the launcher's Features page and the in-game menu switch
// it).
#include "sfa.h"

namespace cps2recomp {

// The game is kept in credits, as a coin put in would: it goes to the title and waits for Start, and Start lets a
// player in. Two, so that a second player can always join.
void CoinsFrame(Machine &) {
    if (FeatureOn("free_play") && Ram8(sfa::kCredits) < 2) {
        SetRam8(sfa::kCredits, 2);
    }
}

void RegisterCoins() {
    TheGame().features.push_back({"free_play", "Game", "Free play", "Start works without a coin.", kFeatureEnhanced});
}

} // namespace cps2recomp
