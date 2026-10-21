// SPDX-License-Identifier: GPL-3.0-only
// Copyright (C) 2026 Tommy McLeroy
//
// Super Puzzle Fighter II Turbo: "Straight to the title" (the Game page). When the board starts (switched on,
// or reset from the menu), everything before the title is run unseen and unheard, as fast as it goes: the
// board's memory test, the version and the warning, the QSound and Capcom notices, and the introduction (the
// logo, a character's introduction, a demonstration match). The picture comes back with the title, a second
// or so later; nothing of the game is changed, only not shown.
//
// The game's first step word (spf2.h, kGameOn) goes 2, 4, 6 and 8 while the board starts (the version, the
// warning), 0 again, and then into the loop of its show: 2 to 12 (the notices), 14 the introduction, whose
// own step (kRoundStep) is 18 when the title comes, then 16 and 18 (the how-to-play dojo). Only the board's
// start begins the skipping (Machine::frame is 0 again at a reset): the step is 0 for a frame or two between
// the screens of a game too. A step past the introduction ends it, and so does a Start pressed meanwhile.
// The modes that reset the board to get somewhere of their own skip it themselves.
#include "spf2.h"

#include "machine.h"

namespace cps2recomp {

bool InAMode(); // modes.cpp

namespace {
    using namespace spf2;

    constexpr uint16_t kIntroduction = 0x14, kTitle = 0x18;
    constexpr int kSkipMost = 4000; // frames after which the skipping is given up

    bool g_skipping = false;
    bool g_fast = false; // this file asked for the game to run unseen
    int g_frames = 0;

    void Fast(bool on) {
        if (on != g_fast) {
            g_fast = on;
            SetFastForward(on);
        }
    }
} // namespace

void TitleFrame(Machine &machine) {
    // (a mode started: its start is its own, and so is running unseen)
    if (InAMode()) {
        g_skipping = g_fast = false;
        return;
    }
    const uint16_t show = Ram16(kGameOn);
    if (machine.frame <= 1 && show == 0 && !g_skipping) {
        g_skipping = FeatureOn("skip_intro");
        g_frames = 0;
    }
    if (g_skipping) {
        const bool title = show == kIntroduction && Ram16(kRoundStep) >= kTitle;
        const bool start = (machine.players[0] | machine.players[1]) & kStart;
        if (title || show > kIntroduction || start || ++g_frames > kSkipMost) {
            g_skipping = false;
        }
    }
    Fast(g_skipping);
}

void RegisterTitle() {
    TheGame().features.push_back({"skip_intro", "Game", "Straight to the title",
                                  "When the game starts, and when it is reset, goes straight to the title: the board's memory test, "
                                  "the notices, the introduction and its demonstration match are run unseen, in a second or so.",
                                  0});
}

} // namespace cps2recomp
