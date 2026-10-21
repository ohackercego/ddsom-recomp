// SPDX-License-Identifier: GPL-3.0-only
// Copyright (C) 2026 Tommy McLeroy
//
// Vampire Savior Definitive: what its sources share (src/graft.cpp, the characters taken in; src/rules.cpp, the
// rules Vampire Savior 2's engine has for them).
#pragma once

#include "lift.h"

#include <cstdint>

namespace cps2recomp::definitive {

// The characters taken from Vampire Savior 2, by their numbers there (and here)
inline constexpr uint8_t kTaken[] = {0x10, 0x11, 0x13};
inline constexpr const char *kTakenNames[] = {"Phobos", "Pyron", "Donovan"};
inline constexpr uint8_t kPhobos = 0x10, kPyron = 0x11, kDonovan = 0x13;
bool IsTaken(uint8_t character);
// A player's record (FF8400, FF8800) whose character is taken in; anything else, false
bool TakenPlayer(uint32_t record);

// (graft.cpp) a table of this game's read for a character taken in, or extended: the hook where it is read
bool OffsetTable(M68K &c);
// (rules.cpp) the hooks of the rules; before graft.cpp's, which leaves alone the places these have
void RegisterRules();
// (choosing.cpp) the character choice: the three's places, under the grid
void RegisterChoosing();

} // namespace cps2recomp::definitive
