// SPDX-License-Identifier: GPL-3.0-only
// Copyright (C) 2026 Tommy McLeroy
//
// Vampire Savior: what the modes share (modes.h): getting a machine to a round, the camera put where a mode wants
// it, and telling objects apart (the machines kept as save states and run beside the port's are the toolkit's:
// runtime/spare.h).
#include "modes.h"

#include "state.h"
#include "widescreen.h"

#include <algorithm>
#include <cstring>

namespace cps2recomp {

namespace {
    using namespace vsav;

    int g_noShadow = -1; // the player whose shadow is not drawn (-1: nobody)

    // The object list counts across in 1024 columns, and a player's shadow is drawn this far on from where the
    // player stands in the picture (found from the list: Morrigan standing at 360 in the picture, her shadow's
    // pieces from 384 to 464). A player far off the picture can have its shadow come round into it.
    constexpr int kShadowColumns = 0x400, kShadowOn = 64;
    constexpr uint16_t kOffPicture = 0x300; // (a column the picture never shows, wide or not)
    int ShadowColumn(int player) {
        return (static_cast<int16_t>(Ram16(Player(player) + kX)) - static_cast<int16_t>(Ram16(kCameraX)) + kShadowOn) & (kShadowColumns - 1);
    }
    int Apart(int a, int b) {
        const int d = (a - b) & (kShadowColumns - 1);
        return std::min(d, kShadowColumns - d);
    }
    // When the board takes the object list (Video::objectsTaken): the pieces of shadow nearer the player whose
    // shadow is not drawn than the other player are put where the picture never shows
    void DropShadow(uint16_t *list, int &last) {
        if (g_noShadow < 0) {
            return;
        }
        const int mine = ShadowColumn(g_noShadow), other = ShadowColumn(1 - g_noShadow);
        for (int i = 0; i <= last; i += 4) {
            const uint32_t tile = list[i + 2] + ((list[i + 1] & 0x6000u) << 3);
            const uint16_t attributes = list[i + 3];
            if (!IsShadow(tile, attributes)) {
                continue;
            }
            const int across = ((attributes >> 8) & 0x0F) + 1;
            const int middle = (list[i] + across * 8) & (kShadowColumns - 1);
            if (Apart(middle, mine) < Apart(middle, other)) {
                list[i] = static_cast<uint16_t>((list[i] & ~(kShadowColumns - 1)) | kOffPicture); // (its level against the scroll screens kept)
            }
        }
    }
} // namespace

void NoShadow(Machine &machine, int player) {
    g_noShadow = player;
    machine.video.objectsTaken = player >= 0 ? DropShadow : nullptr;
}

// (the modes' machine of the toolkit's: fight/modes.h)
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
int NoShadowNow() {
    return g_noShadow;
}

bool IsShadow(uint32_t tile, uint16_t attributes) {
    return (attributes & 0x1F) == 22 && tile >= family::kShadowTiles[0] && tile < family::kShadowTiles[1];
}
bool NotTheStage(uint32_t tile, uint16_t attributes) {
    return (attributes & 0x1F) < 22 || IsShadow(tile, attributes);
}
bool ThePanel(uint32_t tile, uint16_t attributes) {
    // (the players' faces beside the bars are in the players' own colours: told by the panel's tiles)
    return (attributes & 0x1F) < 5 || (tile >= family::kPanelTiles[0] && tile < family::kPanelTiles[1]);
}
bool NotTheFighters(uint32_t tile, uint16_t attributes) {
    return ThePanel(tile, attributes) || ((attributes & 0x1F) >= 22 && !IsShadow(tile, attributes));
}

void RegisterMachines() {
    // (around a spare's frame, what is wanted, the camera, the wide picture and the shadow not drawn are kept by the
    // toolkit's modes: modes.cpp says what of this file's)
}

} // namespace cps2recomp
