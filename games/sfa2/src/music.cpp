// SPDX-License-Identifier: GPL-3.0-only
// Copyright (C) 2026 Tommy McLeroy
//
// Street Fighter Alpha 2: its music, for a custom soundtrack (port/soundtrack.h, machine/soundtrack.h). The sound program (driver
// 1.05C) keeps a table of its songs at 0x9006, four bytes a song; a song whose data starts with a 0 is music.
#include "game.h"

namespace cps2recomp {

void RegisterMusic() {
    TheGame().songTable = 0x9006;
    TheGame().songCount = 0x400;
    // Found by playing: each stage, the arcade ladder to the end with every character, a lost game and attract mode
    // (CPS2RECOMP_SOUNDTRACK_LOG), and --music-test for the ones the board plays once (`true`: quiet before 90 s)
    TheGame().songs = {
        // Each character's theme, played on their stage (modes.h, kStageTunes)
        {0x01, "ryu", "Ryu's theme"},
        {0x02, "ken", "Ken's theme"},
        {0x03, "chunli", "Chun-Li's theme"},
        {0x04, "sagat", "Sagat's theme"},
        {0x05, "sodom", "Sodom's theme"},
        {0x06, "adon", "Adon's theme"},
        {0x07, "birdie", "Birdie's theme"},
        {0x08, "guy", "Guy's theme"},
        {0x09, "charlie", "Charlie's theme"},
        {0x0A, "rose", "Rose's theme"},
        {0x0B, "akuma", "Akuma's theme"},
        {0x0C, "bison", "M. Bison's theme"},
        {0x0D, "dan", "Dan's theme"},
        {0x0E, "dhalsim", "Dhalsim's theme"},
        {0x0F, "zangief", "Zangief's theme"},
        {0x10, "gen", "Gen's theme"},
        {0x11, "rolento", "Rolento's theme"},
        {0x12, "sakura", "Sakura's theme"},
        // Between the fights
        {0x3E, "title", "Title (with the QSound logo)", true},
        {0x13, "attract", "The attract mode's introduction of the fighters", true},
        {0x14, "player_select", "Player select"},
        {0x15, "versus", "Versus screen", true},
        {0x16, "winner", "Winner (the winner's quote)", true},
        {0x17, "continue", "Continue?"},
        {0x18, "challenger", "Here comes a new challenger", true},
        {0x19, "game_over", "Game over", true},
        {0x1A, "fanfare_1a", "A short fanfare (where it plays is not known yet)", true},
        {0x1B, "ranking", "Score ranking", true},
        {0x34, "rival", "Before the last fight (the rival's words)"},
        {0x35, "sagat_final", "Sagat's last fight"},
        // The endings (some have two parts) and the staff roll that follows them
        {0x1C, "ryu_ending", "Ryu's ending", true},
        {0x1D, "ryu_ending_2", "Ryu's ending, second part"},
        {0x1E, "ken_ending", "Ken's ending"},
        {0x1F, "chunli_ending", "Chun-Li's ending"},
        {0x3A, "sagat_ending", "Sagat's ending"},
        {0x20, "sagat_ending_2", "Sagat's ending, second part", true},
        {0x21, "sodom_ending", "Sodom's ending"},
        {0x22, "adon_ending", "Adon's ending"},
        {0x23, "adon_ending_2", "Adon's ending, second part"},
        {0x24, "birdie_ending", "Birdie's ending"},
        {0x25, "guy_ending", "Guy's ending", true},
        {0x26, "charlie_ending", "Charlie's ending"},
        {0x36, "charlie_ending_2", "Charlie's ending, second part"},
        {0x27, "rose_ending", "Rose's ending", true},
        {0x28, "akuma_ending", "Akuma's ending"},
        {0x37, "akuma_ending_2", "Akuma's ending, second part"},
        {0x29, "bison_ending", "M. Bison's ending"},
        {0x2A, "dan_ending", "Dan's ending", true},
        {0x2B, "dhalsim_ending", "Dhalsim's ending"},
        {0x2C, "zangief_ending", "Zangief's ending"},
        {0x38, "zangief_ending_2", "Zangief's ending, second part"},
        {0x2D, "gen_ending", "Gen's ending"},
        {0x2E, "gen_ending_2", "Gen's ending, second part"},
        {0x2F, "rolento_ending", "Rolento's ending"},
        {0x30, "sakura_ending", "Sakura's ending"},
        {0x31, "sakura_ending_2", "Sakura's ending, second part"},
        {0x32, "staff_roll_a", "Staff roll (after Ken's, Sodom's and Guy's endings)"},
        {0x33, "staff_roll_b", "Staff roll (after Chun-Li's, Charlie's, Adon's, Rose's, M. Bison's, Sagat's, Rolento's and Gen's)"},
        {0x39, "staff_roll_c", "Staff roll (after Ryu's, Birdie's, Dan's, Sakura's, Dhalsim's and Zangief's)"},
    };
}

} // namespace cps2recomp
