// SPDX-License-Identifier: GPL-3.0-only
// Copyright (C) 2026 Tommy McLeroy
//
// Super Gem Fighter Mini Mix: its music, for a custom soundtrack (port/soundtrack.h, machine/soundtrack.h). The
// sound program (driver 1.30, as Vampire Savior's) keeps a table of its songs at 0x9006, four bytes a song: the word
// at 0x9000 is how many (0x7ED), and a song whose data starts with a 0 is music.
#include "game.h"

namespace cps2recomp {

void RegisterMusic() {
    TheGame().songTable = 0x9006;
    TheGame().songCount = 0x7ED;
    // Found by playing (CPS2RECOMP_SOUNDTRACK_LOG, with pictures where each starts): a fight on each stage, the
    // one-player ladder to its end with Ken, a two-player game, and attract mode. The ladder's story and ending were
    // Ken's; whether each character has its own is not known yet.
    TheGame().songs = {
        // Each stage's own (pocket.h: stage n's is n + 1)
        {0x01, "felicia", "Felicia's stage (the shop)"},
        {0x02, "ibuki", "Ibuki's stage (the big screen)"},
        {0x03, "hsienko", "Hsien-Ko's stage (the snowy village)"},
        {0x04, "chunli_sakura", "Chun-Li's and Sakura's stage (the elephant)"},
        {0x05, "morrigan_dan", "Morrigan's and Dan's stage (Chinatown)"},
        {0x06, "ryu", "Ryu's stage (the forge)"},
        {0x07, "tessa_akuma", "Tessa's and Akuma's stage (the castle)"},
        {0x08, "ken_zangief", "Ken's and Zangief's stage (the hall)"},
        // Between the fights
        {0x2E, "qsound", "The QSound logo", true},
        {0x09, "capcom", "The maker's logo", true},
        {0x0A, "attract", "The attract mode's introduction"},
        {0x0B, "player_select", "Player select"},
        {0x12, "story", "The story before the first fight"},
        {0x0C, "versus", "Versus (the next opponent, one player)"},
        {0x0F, "versus_two", "Versus (two players)"},
        {0x0E, "winner", "Winner", true},
        {0x10, "last_fight", "Before the last fight"},
        {0x22, "ending", "Ending"},
        {0x2D, "staff_roll", "Staff roll"},
        {0x1E, "game_over", "Game over (the name entry)"},
        {0x1F, "ranking", "Score ranking"},
    };
}

} // namespace cps2recomp
