// SPDX-License-Identifier: GPL-3.0-only
// Copyright (C) 2026  Part of cps2recomp.
//
// Dungeons & Dragons: Shadow over Mystara (ddsom) — family facts for recomp.py.
// This is a fresh port (no source-sharing with another game yet), so there are
// no At(0x...) places to remap and no character roster facts filled in. As
// hand-written sources (src/) are added, the game's own places can be listed here.
#pragma once

#include <array>
#include <cstdint>

#include "game.h"

namespace ddsom::family {

// No translated blocks reference another game's At(0x...) places yet.
inline constexpr std::array<int, 0> kPlaces = {};

} // namespace ddsom::family
