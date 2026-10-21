// SPDX-License-Identifier: GPL-3.0-only
// Copyright (C) 2026 Tommy McLeroy
//
// Alien vs. Predator: its music, for a custom soundtrack (port/soundtrack.h, machine/soundtrack.h). The sound
// program keeps a table of its songs at 0x9006, four bytes a song, after a word of how many there are (0x38D);
// a song whose data starts with a 0 is music.
#include "avsp.h"

namespace cps2recomp {

void RegisterMusic() {
    TheGame().songTable = 0x9006;
    TheGame().songCount = 0x38D;
    // Found by playing: attract mode, each round from its start, and a continue (CPS2RECOMP_SOUNDTRACK_LOG), and
    // --music-test (with the test switch held) for the ones the board plays once (`true`). A song heard in more
    // than one place is named by the first. Those not heard yet are listed by their numbers only.
    TheGame().songs = {
        {0x1A, "story", "The story (attract mode, and before a game)"},
        {0x05, "player_select", "Player select"},
        {0x07, "round_1", "Round 1: City of Despair (and round 7)"},
        {0x16, "round_2_start", "Round 2's title and first words"},
        {0x06, "round_2", "Round 2: War in the Underpass"},
        {0x09, "round_2_later", "Round 2, further on (and round 4)"},
        {0x15, "round_3_start", "Before round 3 (and round 5): the radio's words"},
        {0x08, "round_3", "Round 3 (and round 5)"},
        {0x0A, "round_4", "Round 4: the hive"},
        {0x17, "round_6_start", "Round 6's title: Nightmare in the Lab"},
        {0x0B, "round_6", "Round 6"},
        {0x1C, "special_stage", "Special stage"},
        {0x18, "round_7_start", "Round 7's title: Hunt's End"},
        {0x1F, "boss_appears", "A boss appears (its name shown)"},
        {0x0F, "boss", "Boss fight (Chrysalis, round 1; Stalker, round 3)"},
        {0x10, "boss_2", "Boss fight (Razor Claws, round 2)"},
        {0x11, "boss_3", "Boss fight (round 5's drop ship)"},
        {0x04, "results", "A round's results", true},
        {0x01, "continue", "Continue?"},
        {0x19, "qsound", "The QSound notice", true},
        {0x20, "capcom", "Capcom's notice", true},
        {0x1B, "fanfare_1b", "A long piece the board plays once (where is not known yet: an ending?)", true},
        {0x03, "fanfare_03", "A short fanfare (where it plays is not known yet)", true},
    };
}

} // namespace cps2recomp
