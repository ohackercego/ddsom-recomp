// SPDX-License-Identifier: GPL-3.0-only
// Copyright (C) 2026 Tommy McLeroy
//
// Super Gem Fighter Mini Mix: "Straight to the title" (the Game page), as in the Alpha games. When the board starts
// (switched on, or reset from the menu), everything before the title is run unseen and unheard, as fast as it goes:
// the board's memory test, the warning, the notices and the introduction. The picture comes back with the title;
// nothing of the game is changed, only not shown.
//
// The game's step (pocket.h, kMode) is 0 while the board starts. It then goes 2, 4 (the warning, frames 95-276), 6,
// 8 (traced, frame 459), and into the loop of its show: 2, 0C, 0E, ..., 18 the introduction, whose own step
// (kRoundStep) counts up in twos to 4C when the title builds up from white (frame 4887), then 1A and on. With a coin
// in, or free play, it goes from that 2 to 2 and 4 instead (frames 470-471): the title that asks for Start. A step
// past the introduction ends the skipping too. Only the board's start begins it (Machine::frame is 0 again at a
// reset): the step is 0 for a few frames between the screens of a game too.
#include "alternative.h"
#include "pocket.h"

namespace cps2recomp {

namespace {
    using namespace pocket;

    constexpr uint16_t kBooted = 0x08, kIntroduction = 0x18, kTitle = 0x4C;
    constexpr int kSkipMost = 8000; // frames after which the skipping is given up

    bool g_skipping = false;
    bool g_booted = false; // the board's own start (the warning) is over
    bool g_fast = false;   // this file asked for the game to run unseen
    int g_frames = 0;

    void Fast(bool on) {
        if (on != g_fast) {
            g_fast = on;
            SetFastForward(on);
        }
    }
} // namespace

void TitleFrame(Machine &machine) {
    const uint16_t mode = Ram16(kMode);
    // (only the board's first frame: the step is 0 again between screens of a game, and a game must never be
    // run unseen. A reset while it skips starts it again: what the last start had seen is not this one's)
    if (machine.frame <= 1 && mode == 0) {
        g_skipping = FeatureOn("skip_intro") && !modes::AttractMode(); // (attract mode is the show itself)
        g_booted = false;
        g_frames = 0;
    }
    // (the feature switched off while it skips: the picture comes back at once)
    if (g_skipping && !FeatureOn("skip_intro")) {
        g_skipping = false;
    }
    if (g_skipping) {
        g_booted = g_booted || mode == kBooted;
        const bool paid = Ram8(kCredits) != 0 || Ram8(kFreePlay) != 0;
        const bool title = (mode == kIntroduction && Ram16(kRoundStep) >= kTitle) ||
                           (g_booted && mode != kBooted && mode != 0 && paid);
        if (title || (g_booted && mode > kIntroduction) || ++g_frames > kSkipMost) {
            g_skipping = false;
        }
    }
    Fast(g_skipping);
}

void RegisterTitle() {
    TheGame().features.push_back({"skip_intro", "Game", "Straight to the title",
                                  "When the game starts, and when it is reset, goes straight to the title: the board's memory test, "
                                  "the notices and the introduction are run unseen, in a second or so.",
                                  0});
}

} // namespace cps2recomp
