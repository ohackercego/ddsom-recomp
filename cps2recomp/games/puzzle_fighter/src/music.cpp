// SPDX-License-Identifier: GPL-3.0-only
// Copyright (C) 2026 Tommy McLeroy
//
// Super Puzzle Fighter II Turbo: its music, for a custom soundtrack (port/soundtrack.h, machine/soundtrack.h). The sound program (driver
// 2.01b) keeps a table of its songs at 0x6005, four bytes a song; a song whose data starts with a 0 is music.
#include "game.h"

namespace cps2recomp {

void RegisterMusic() {
    TheGame().songTable = 0x6005;
    TheGame().songCount = 0x500;
    // Found by playing: the arcade ladder, a lost game, two players and attract mode (CPS2RECOMP_SOUNDTRACK_LOG), and
    // --music-test for the ones the board plays once (`true`: quiet before 90 s)
    TheGame().songs = {
        // Each character's, played when they are the opponent (in the game's order of them; Devilot's and Dan's by it:
        // the arcade ladder reaches neither)
        {0x01, "morrigan", "Morrigan's theme"},
        {0x02, "chunli", "Chun-Li's theme"},
        {0x03, "ryu", "Ryu's theme"},
        {0x04, "ken", "Ken's theme"},
        {0x05, "hsienko", "Hsien-Ko's theme"},
        {0x06, "donovan", "Donovan's theme"},
        {0x07, "felicia", "Felicia's theme"},
        {0x08, "sakura", "Sakura's theme"},
        {0x09, "devilot", "Devilot's theme"},
        {0x0A, "akuma", "Akuma's theme"},
        {0x0B, "dan", "Dan's theme"},
        // Around the matches
        {0x1E, "qsound", "The QSound logo", true},
        {0x0C, "title", "Title", true},
        {0x0D, "dojo", "The Karate Puzzle Dojo (how to play)"},
        {0x0E, "select", "Mode and character select"},
        {0x0F, "logo", "The game's logo before each match", true},
        {0x10, "winner", "Winner", true},
        {0x11, "intermission", "Intermission"},
        {0x12, "continue", "Continue"},
        {0x13, "challenger", "Here comes a new challenger", true},
        {0x14, "game_over", "Game over", true},
        {0x15, "ranking", "Score ranking", true},
        {0x16, "best_player", "Score ranking: the best player", true},
        {0x17, "staff_roll", "Ending and staff roll"},
    };
}

} // namespace cps2recomp
