// SPDX-License-Identifier: GPL-3.0-only
// Copyright (C) 2026 Tommy McLeroy
//
// Vampire Savior: its music, for a custom soundtrack (port/soundtrack.h, machine/soundtrack.h): where the sound
// program keeps its table of songs, and the pieces named (the game's own list: family.h).
#include "game.h"
#include "vsav.h"

namespace cps2recomp {

void RegisterMusic() {
    TheGame().songTable = vsav::family::kSongTable;
    TheGame().songCount = vsav::family::kSongCount;
    for (const vsav::family::Song &song : vsav::family::kSongs) {
        TheGame().songs.push_back({song.number, song.name, song.title, song.once});
    }
}

} // namespace cps2recomp
