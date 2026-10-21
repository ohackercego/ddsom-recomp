// SPDX-License-Identifier: GPL-3.0-only
// Copyright (C) 2026 Tommy McLeroy
//
// Alien vs. Predator (avspu, USA 940520): names for what is known of the game's memory, with how each was found
// (PROGRESS.md has more). The game keeps its variables at A5 = FF8000.
#pragma once

#include "game.h"
#include "lift.h"
#include "machine.h"

namespace cps2recomp::avsp {

constexpr uint32_t kVars = 0xFF8000; // A5

// The show's step (word): 2-C as the board starts, 0, then the attract's parts, each a step of four (the step between
// is the next part's setting up), from 4 to 68 and round again from 0; 2 for good while a game is played (from the
// choice of characters to the game over). And the step's own step (word: the title is drawn at 8). Traced.
constexpr uint32_t kMode = kVars + 0x00, kModeStep = kVars + 0x04;
// The character whose file the show's next "characters" part shows (word, 0-3; the demonstration after it moves it
// on: 004E9A)
constexpr uint32_t kShowCharacter = kVars - 0x6FF6;
constexpr uint16_t kModeTitle = 0x10, kModeGame = 0x02, kModeShowEnd = 0x6C;
// The show's parts by their steps (pictures of each, traced over a whole loop): the notices (the EPA's, QSound's),
// the story, the title, a character's file and how to play them, a demonstration, the best scores
enum class ShowPart { Other, Notices, Story, Title, Characters, Demonstration, Scores };
constexpr ShowPart Part(uint16_t step) {
    switch (step) {
    case 0x04: case 0x08: case 0x38: case 0x3C: return ShowPart::Notices;
    case 0x0C: case 0x40: return ShowPart::Story;
    case 0x10: case 0x24: case 0x44: case 0x58: return ShowPart::Title;
    case 0x14: case 0x18: case 0x28: case 0x2C: case 0x48: case 0x4C: case 0x5C: case 0x60: return ShowPart::Characters;
    case 0x1C: case 0x30: case 0x50: case 0x64: return ShowPart::Demonstration;
    case 0x20: case 0x34: case 0x54: case 0x68: return ShowPart::Scores;
    default: return ShowPart::Other;
    }
}

// The credits (byte, 0-9; found from dumps after one, two and three coins; 0016B4 holds it at 9)
constexpr uint32_t kCredits = kVars + 0x18C;

// The settings as the game uses them, taken from the settings memory's record (FF0EF0) by 003C5E at the board's
// start (called at 000D1A, returns to 000D20) and again when the test menu is left (0B3C64, returns to 0B3C6A).
// The test menu changes these and saves them. Values as the test menu's pages have them (stepped through):
constexpr uint32_t kSetCoin = kVars + 0x1CD;       // 0-7 1C1C, 1C2C, 1C3C, 1C4C, 1C6C, 2C1C, 3C1C, 4C1C; 8 2C start 1C continue; 9 free play
constexpr uint32_t kSetContinue = kVars + 0x1BE;   // 0 off, 1 on (a new board: on)
constexpr uint32_t kSetDemoSound = kVars + 0x1AD;  // 0 off, 1 on (off)
constexpr uint32_t kSetPlayerMode = kVars + 0x1CE; // 0 three players, three chutes; 1 three, one chute; 2 two, one chute (two)
constexpr uint32_t kSetDifficulty = kVars + 0x1CF; // 0-7: the test menu's 1-8 (4)
constexpr uint32_t kSetExtend = kVars + 0x1D0;     // 0 none; 1-3 only at 100000, 400000, 1000000; 4-7 first and every
                                                   // 100000/400000, 400000/500000, 800000/600000, 1200000/1000000 (1)
constexpr uint32_t kSetLives = kVars + 0x1D1;      // 0-3 start and continue with 1, 2, 3, 5; 4-7 start 1, 2, 3, 5 and
                                                   // continue with one more (4: start 1, continue 2)
// ... and what 003C5E works out from the player mode (the game reads these, not kSetPlayerMode): a third player,
// and a chute of each player's own
constexpr uint32_t kThreePlayers = kVars + 0x1C5, kThreeChutes = kVars + 0x1C6;

// The scene the game is in (word, two for each: 0, 2, 4...; cleared before a game, set to 2 by 0082DC as the players start
// after round 1's story, stepped on by 00C860 and the like) and the round shown for it (word, from the table at 00CCA0: scenes 0-2 are round 1, 3-4 round 2, 5
// round 3, 6-7 round 4, 8 round 5, 9-13 round 6, 14 on round 7)
constexpr uint32_t kScene = kVars + 0x1E8, kRound = kVars + 0x222;
constexpr int kRounds = 7;
constexpr uint16_t kRoundFirstScene[kRounds] = {0, 3, 5, 6, 8, 9, 14};

// The round's clock: minutes (byte) and seconds (BCD byte), counted down at 0085A4 each time the frame count
// (byte, from 0x3C) runs out; it goes on only while the minutes are not negative ("Hurry up")
constexpr uint32_t kClockFrames = kVars + 0x221, kClockMinutes = kVars + 0x224, kClockSeconds = kVars + 0x225;

// The players: three records of 0x100 bytes from FF8380 (01CC3E walks them)
constexpr int kPlayers = 3;
constexpr uint32_t kPlayer = kVars + 0x380, kPlayerSize = 0x100;
inline uint32_t Player(int n) {
    return kPlayer + kPlayerSize * static_cast<uint32_t>(n);
}
constexpr uint32_t kInPlay = 0x00;    // byte: 1 while the player is in the game
constexpr uint32_t kX = 0x10;         // word: the place across, in the stage (as the camera's: 0B0C22 follows it)
constexpr uint32_t kHealth = 0x41;    // byte: health, down to 0; FF once down (found standing still while hit)
constexpr uint32_t kHealthMax = 0x45; // byte: what it starts at (0x65)
constexpr uint32_t kLives = 0xF1;     // byte: lives left after this one ("x1"; found as it went 0, 1, 2)
constexpr uint32_t kCharacter = 0xD2;  // byte: who it is (0 Predator Warrior, 1 Maj. Schaefer, 2 Predator Hunter, 3 Lt.
                                       // Kurosawa: 01B0C4 takes its animations by it; put in as a game begins, it is that character)
constexpr uint32_t kScore = 0xE0;     // long: the score, in BCD (as shown at the top: found from dumps while it went up)

} // namespace cps2recomp::avsp
