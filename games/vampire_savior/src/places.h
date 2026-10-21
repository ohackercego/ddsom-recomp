// SPDX-License-Identifier: GPL-3.0-only
// Copyright (C) 2026 Tommy McLeroy
//
// The places these sources name in the game's program and memory are Vampire Savior's. Capcom made two more
// games from the same program, Vampire Savior 2 and Vampire Hunter 2 (games/vsav2, games/vhunt2), which build
// these sources too: in them, most routines, tables and variables are somewhere else. At() gives where the one
// named is in the game being built, from that game's own list (its family.h; tools/places.py makes it). For
// Vampire Savior there is no list: a place is where it is said to be.
//
// Both are worked out when the program is compiled (consteval): a place that the game's list does not have
// stops the build there.
#pragma once

#include "family.h"

#include <cstdint>

namespace vsav {

// A place in the ROM (code or a table) or in work RAM (FFxxxx), as Vampire Savior has it
consteval uint32_t At(uint32_t place) {
    if (family::kPlaces.size() == 0) {
        return place;
    }
    size_t low = 0, high = family::kPlaces.size();
    while (low < high) {
        const size_t middle = (low + high) / 2;
        if (family::kPlaces[middle].vsav < place) {
            low = middle + 1;
        } else {
            high = middle;
        }
    }
    if (low == family::kPlaces.size() || family::kPlaces[low].vsav != place) {
        throw "this game's family.h does not say where this place is (tools/places.py)";
    }
    return family::kPlaces[low].here;
}

// A variable of the game's, as a distance from A5 (FF8000), as Vampire Savior has it
consteval int32_t AtA5(int32_t offset) {
    return static_cast<int32_t>(At((0xFF8000u + static_cast<uint32_t>(offset)) & 0xFFFFFF)) - 0xFF8000;
}

} // namespace vsav
