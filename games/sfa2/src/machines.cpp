// SPDX-License-Identifier: GPL-3.0-only
// Copyright (C) 2026 Tommy McLeroy
//
// Street Fighter Alpha 2: what the modes share (modes.h): getting a machine to a round, sounds asked for, telling
// objects apart, and what is wanted of the machine that is running (the machines kept as save states and run
// beside the port's are the toolkit's: runtime/spare.h).
#include "modes.h"

#include "state.h"

#include <algorithm>
#include <cstring>

namespace cps2recomp {

// (the modes' machine of the toolkit's: fight/alternative.h)
MatchWanted &Wanted() {
    return modes::Wanted();
}
CameraWanted &CameraAt() {
    return modes::CameraAt();
}
bool RoundFought() {
    return modes::RoundFought();
}
bool ToRound(Machine &machine, int32_t &frames, bool choose) {
    return modes::ToRound(machine, frames, choose);
}
void PlaySound(uint16_t code) {
    modes::PlaySound(code);
}

bool ThePanel(uint32_t tile, uint16_t attributes) {
    // (the bars, the names, the clock and the faces, and the gauges below: by their tiles, in whatever colours)
    // (0A400-0A6FF only outside the players' colour sets, 11-25: Charlie's own pictures are there too, and were
    // taken for the panel, so that the modes that lay the fighters over another stage lost him in some attacks)
    const unsigned set = attributes & 0x1F;
    return (tile >= 0x1A400 && tile < 0x1AB00) || (tile >= 0x26A00 && tile < 0x26B00) ||
           (tile >= 0x0A400 && tile < 0x0A700 && (set < 11 || set > 25));
}
bool NotTheStage(uint32_t tile, uint16_t attributes) {
    return (attributes & 0x1F) < 27 || ThePanel(tile, attributes);
}

void RegisterMachines() {
    // (around a spare's frame, what is wanted, the camera and the wide picture are kept by the toolkit's modes)
}

} // namespace cps2recomp
