// SPDX-License-Identifier: GPL-3.0-only
// Copyright (C) 2026 Tommy McLeroy
//
// Cadillacs and Dinosaurs (dino, World 930201): names for what is known of the game's memory, with how each was
// found (PROGRESS.md has more). The game keeps its variables at A5 = FF8000.
#pragma once

#include "game.h"
#include "lift.h"
#include "machine.h"

namespace cps2recomp::dino {

constexpr uint32_t kVars = 0xFF8000; // A5

// Each of the three coin chutes' records (16 bytes from FF847A; 006C62 walks them), with its credits at +3 (byte;
// found from dumps after coins). A new board has a chute for each player, each with credits of its own.
constexpr uint32_t kChute = kVars + 0x47A, kChuteSize = 0x10, kChuteCredits = 3;

// The board's settings as the game reads them: three bytes laid out as the DIP switches of the CPS-1 boards before
// it (FFF77A-FFF77C), which 0AC86E packs from the settings the test menu shows (FFF75A on: kept at FFF76D-FFF76F) as
// the board starts, and 0A86D8 copies again when the test menu is left. The fields (from 0AC86E; their values as
// the test menu's System Configuration page has them, stepped through):
//   B, bits 0-2: game difficulty, 0-7 (level 0 extra easy ... 3 normal ... 7 hardest; a new board: 3)
//   B, bits 3-4: game difficulty 2, 0-3 (easy, normal, hard, very hard; 1)
//   B, bits 5-6: extend, 0-3 (none; 300,000 then 700,000; 500,000 then 1,000,000; only 1,000,000; 2)
//   C, bits 0-1: number of players (lives), 0-3 for 1-4 (1)
//   C, bit 5: demo sound (off); C, bit 6: continue (on)
constexpr uint32_t kSwitches = kVars + 0x777A, kBoardSwitches = kVars + 0x776D;

// A value that goes 10, 30, 40, 50 as the board starts (traced): 50 when the title is drawn (about frame 312). It
// counts on through the rest of the show, so only the board's start is told by it.
constexpr uint32_t kShow = kVars + 0x35;
constexpr uint8_t kShowTitle = 0x50;

// The game's step (byte: 4 as a game begins (the choice of characters), 0C while it is played, 08 between stages, 1C
// at a stage's end, 10 14 18 its game over, up to the show's next demonstration; 0 in the attract, and 0C in its
// demonstrations) and the stage (byte, 0-7; 00ABE8 steps it on as a stage ends; 00004CEE clears it, with the rest
// of the game's variables, as a game begins: 00A9EC, returning to 00A9F2)
constexpr uint32_t kStep = kVars + 0x4D0, kStage = kVars + 0x4D9;
constexpr uint8_t kStepGameOver = 0x10;
constexpr int kStages = 8;

// The stage's clock: minutes and seconds (BCD bytes), counted down at 0103C0 each time the frame count (byte, from
// 0x32) runs out
constexpr uint32_t kClockMinutes = kVars + 0x4E8, kClockSeconds = kVars + 0x4E9, kClockFrames = kVars + 0x4EA;

// The players: three records of 0x180 bytes from FFB274 (the code finds them as 3274(a5), 33F4(a5), 3574(a5))
constexpr int kPlayers = 3;
constexpr uint32_t kPlayer = kVars + 0x3274, kPlayerSize = 0x180;
inline uint32_t Player(int n) {
    return kPlayer + kPlayerSize * static_cast<uint32_t>(n);
}
constexpr uint32_t kInPlay = 0x00;  // byte: 1 while the player is in the game
constexpr uint32_t kCharacter = 0x03; // byte: who it is (0 Jack, 1 Mustapha, 2 Hannah, 3 Mess: 0014EE takes its animations
                                      // by it; put in as a game begins, it is that character)
constexpr uint32_t kX = 0x0C;       // word: the place across, in the stage (as the camera's: 0211D8 follows it)
constexpr uint32_t kHealth = 0x6D;  // byte: health (found standing still while hit: 64 down to 0, 64 again with a life)
constexpr uint8_t kHealthFull = 0x64;
constexpr uint32_t kLives = 0xA3;   // byte: lives left after this one (01BFD6 takes one)
constexpr uint32_t kScore = 0x100;  // long: the score, in BCD (found from dumps while it went up; a copy at +110)

} // namespace cps2recomp::dino
