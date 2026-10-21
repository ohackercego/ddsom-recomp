// SPDX-License-Identifier: GPL-3.0-only
// Copyright (C) 2026 Tommy McLeroy
//
// Cadillacs and Dinosaurs: its music, for a custom soundtrack (port/soundtrack.h, machine/soundtrack.h). The sound
// program keeps a table of its songs at 0x8006, four bytes a song, after a word of how many there are (0x106);
// a song whose data starts with a 0 is music.
#include "dino.h"

namespace cps2recomp {

void RegisterMusic() {
    TheGame().songTable = 0x8006;
    TheGame().songCount = 0x106;
    // Found by playing: attract mode, each episode from its start, and a continue (CPS2RECOMP_SOUNDTRACK_LOG), and
    // --music-test (with the test switch held) for the ones the board plays once (`true`). The game plays a tune for
    // a kind of place, so several are heard in more than one episode; each is named by the first. Those not heard
    // yet are listed by their numbers only.
    TheGame().songs = {
        {0x24, "story", "Attract mode: the story, first part"},
        {0x25, "story_2", "Attract mode: the story, second part", true},
        {0x00, "episode", "An episode's title and map"},
        {0x01, "episode_1_start", "Episode 1's opening words"},
        {0x02, "rooftop", "Episode 1: the top of the building"},
        {0x03, "mansion", "Episode 1: the mansion (and episode 2's sawmill, episode 6's start)"},
        {0x04, "street", "Episode 1: 47th Street (and episode 7)"},
        {0x05, "jungle", "Episode 2: the jungle (and episode 6)"},
        {0x06, "swamp", "Episode 2: the mud swamp"},
        {0x07, "waste_land", "Episode 3: the waste land"},
        {0x09, "harbour", "Episode 4's start (and episode 8's underground)"},
        {0x0A, "indoors", "Episode 4: indoors"},
        {0x0B, "episode_4_later", "Episode 4, further on"},
        {0x17, "grassland", "Episode 5: the grassland"},
        {0x0C, "laboratory", "Episode 8: the laboratory"},
        {0x20, "boss_words", "A boss's words before its fight"},
        {0x0E, "boss", "Boss fight (Vice T., episode 1)"},
        {0x0F, "boss_2", "Boss fight (heard in episode 2)"},
        {0x10, "boss_3", "Boss fight (Hogg, episode 3)"},
        {0x0D, "boss_4", "Boss fight (heard against Blade)"},
        {0x18, "results", "An episode's results", true},
        {0x13, "drive_on", "On to the next episode (and attract mode's start)"},
        {0x1B, "continue", "Continue?"},
        {0x14, "fanfare_14", "A short fanfare (where it plays is not known yet)", true},
        {0x1D, "fanfare_1d", "A short fanfare (where it plays is not known yet)", true},
        {0x1E, "fanfare_1e", "A short fanfare (where it plays is not known yet)", true},
        {0x22, "fanfare_22", "A piece the board plays once (where is not known yet)", true},
        {0x26, "fanfare_26", "A short fanfare (where it plays is not known yet)", true},
    };
}

} // namespace cps2recomp
