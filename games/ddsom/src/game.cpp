// SPDX-License-Identifier: GPL-3.0-only
// Copyright (C) 2026  Part of cps2recomp.
//
// Dungeons & Dragons: Shadow over Mystara — what the game's own sources add to the port: free play and
// how many players can play.
//
// Modeled after games/avsp/src/game.cpp.
#include "ddsom.h"

#include "lift.h"

namespace cps2recomp {

namespace {
    using namespace ddsom;

    // --- Free play ---
    // The game takes a credit when a game begins (01F82A and 01F826 clear the counters, 0E0562 clears
    // the first). The port's free play keeps the credit counters from going down, so a game can always
    // begin: the credit is put back as soon as the game has taken it, which is what the board's own free
    // play does (the game's test menu has a "Free Play" setting, and the board's coin mode is what puts
    // the credit back in the counter).
    //
    // The counters are words (the credits of each chute: the first at FF806C, the second at FF806E), and
    // the game takes one when a player presses Start. With the board set for four players and two
    // chutes, each player has a chute of their own, so both are kept up.
    void FreePlay() {
        if (FeatureOn("free_play")) {
            if (Ram16(kCredits) < 4) {
                SetRam16(kCredits, 4);
            }
            if (Ram16(kCredits2) < 4) {
                SetRam16(kCredits2, 4);
            }
        }
    }

    // --- How many players can play ---
    // The board's own setting: the "Chute type" of the game's operator menu (Configuration, System),
    // which says how many players can play at once and how many coin chutes they have. The routine at
    // 02D358 fills the game's settings from the record the EEPROM holds, and the record's second byte
    // (FF801D) is the index of the setting the board is set to, and the game's table at 2D40E turns it
    // into the chute type the game plays with.
    //
    // The port's own setting chooses the index the board is set to: 2 is the board's own (two players,
    // two chutes, multi) and 0x0A is four players, two chutes (found by driving the game's own menu and
    // reading the memory).
    //
    // The routine is written again as C++ in board.cpp (the rewrite ApplyBoardSettings), which is where
    // the choice is put into the record.

    void Frame(Machine &machine) {
        FreePlay();
    }
} // namespace

void RegisterGame() {
    Game &game = TheGame();
    RegisterBoard();
    RegisterCases();
    game.frame = Frame;
}

} // namespace cps2recomp
