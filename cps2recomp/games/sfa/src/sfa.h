// SPDX-License-Identifier: GPL-3.0-only
//
// Street Fighter Alpha: names for what is known of the game's memory and routines, each found by experiment in
// this game. Its variables are reached from A5 = FF8000, as in Alpha 2, but most are elsewhere: Alpha 2 is not
// made from this game's program, so nothing is taken from there unchecked.
#pragma once

#include "game.h"
#include "lift.h"
#include "machine.h"

#include <cstdint>

namespace sfa {

using cps2recomp::M68K;

// --- where the game is ---

// Words from A5, each a step (in twos) of the one before, as in Alpha 2: the first is 2 once a game is being
// played (and the board's start and its show step through it before: title.cpp); the next 2 while the players
// choose their characters and 4 during the match; then 4 while a round is on, and under that 2 while it is being
// fought (0 its introduction). Traced with --watch.
constexpr uint32_t kMode = 0xFF8000, kGameStep = 0xFF8004, kMatchStep = 0xFF8008, kRoundStep = 0xFF800C;

// The round's clock: seconds left, a byte (99 at a round's start, 004378; a second off at 004B3A, and below
// nothing time is up). Found by RAM dumps against the clock shown (91, 87, 83, 79) and --watch: FF365D is the
// picture's copy of it, in BCD. Alpha 2's is at FF8109.
constexpr uint32_t kRoundClock = 0xFFAE09;

// Credits paid for and not yet used, a byte (found by RAM dumps after one, two and three coins put in: 0, 1, 2,
// 3; the board's settings memory starts at one coin a credit). Alpha 2 keeps its own at FF8076.
constexpr uint32_t kCredits = 0xFF8170;

// --- the players ---

// Each player's record, 0x400 long, the first at FF8400 (as in Alpha 2)
constexpr uint32_t kPlayer1 = 0xFF8400, kPlayerSize = 0x400;
inline uint32_t Player(int n) {
    return kPlayer1 + static_cast<uint32_t>(n) * kPlayerSize;
}
// A player's health, a word (Alpha 2's is at +50); kHealthShown follows it a little later: the bar on the
// screen. Found by RAM dumps through a fight: both 90 at the round's start and going down with each hit; the
// hit's routine takes the damage off at 00FA06 (sub.w d3,0x40(a1)), a throw's at 021148.
constexpr uint32_t kHealth = 0x40, kHealthShown = 0x42;
constexpr uint16_t kHealthFull = 0x90;

// --- for hooks ---

inline void SetLow16(uint32_t &reg, uint32_t value) {
    reg = (reg & 0xFFFF0000) | (value & 0xFFFF);
}

} // namespace sfa
