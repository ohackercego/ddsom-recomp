// SPDX-License-Identifier: GPL-3.0-only
// Copyright (C) 2026  Part of cps2recomp.
//
// Dungeons & Dragons: Shadow over Mystara (ddsom, USA 950101): names for what is known of the game's memory,
// with how each was found (PROGRESS.md has more). The game keeps its variables at A5 = FF8000, like the other
// CPS-2 games in this toolkit (avsp).
//
// How each was found (the game's own code, in the deciphered program; work/ddsom/program.lst is the
// disassembly, with the program's addresses in it):
//   - the boot sets A5 = FF8000 at 00039E, and every variable is at an offset from it;
//   - the credit counter: 0005E2 adds 1 to it when a coin is put in (the coin's bit, read from the
//     board's input port, reaches it there), and 01F82A and 0E0562 clear it when a credit is spent;
//   - the second chute's counter is the word after it;
//   - the boot writes 8 to the word after that (0003C2), which is the board's own limit of credits;
//   - the board's settings, as the game reads them (the routine at 02D358 fills them from the
//     settings record the EEPROM holds):
//       FF801C the record's first byte, FF801D the record's second (the index of the setting),
//       FF801E.. the game's own settings,
//       FF8021 the "Chute type" (how many players can play, and how many coin chutes),
//       FF8022 the "Coin mode" (how many credits a coin gives), FF8020 the "Continue".
//     The routine 02D3FA takes the chute type and the coin mode from a table of the game at 2D40E,
//     indexed by the record's second byte, and the game shows them in its own operator menu.
#pragma once

#include "game.h"
#include "machine.h"

namespace cps2recomp::ddsom {

// The game's variables (A5 = FF8000)
constexpr uint32_t kVars = 0xFF8000;

// --- The board's settings memory, as the game uses them ---
// The credits: a word, the number of coins in the first chute. 0005E2 adds one to it (a coin), 01F82A
// and 0E0562 take one away (a game begins).
constexpr uint32_t kCredits = kVars + 0x06C;
// The second chute's credits (a word), the same way.
constexpr uint32_t kCredits2 = kVars + 0x06E;
// The board's own limit of credits (a word): the boot puts 8 there (0003C2).
constexpr uint32_t kCreditLimit = kVars + 0x070;

// --- The board's settings, as the game keeps them ---
// The record's second byte is the index of the setting the board is set to: the game's table at
// 2D40E turns it into the chute type (FF8021) and the coin mode (FF8022), and the game shows it in
// its own operator menu (Configuration, System). The routine at 02D358 takes the record at A5 - 0x6272
// and reads the index from its second byte; the index the board is set to by default is 2 ("two
// players, two chutes, multi") and 0x0A is "four players, two chutes" (the table's rows 10 and 11 are
// the only ones that give a chute type of 3, which is four players).
constexpr uint32_t kSettings = kVars - 0x6272;
constexpr uint32_t kSettingsIndex = kSettings + 1;
constexpr uint8_t kSettingTwoPlayers = 2;
constexpr uint8_t kSettingFourPlayers = 0x0A;
// The chute type: how many players can play, and how many coin chutes (what the setting turned into).
constexpr uint32_t kChuteType = kVars + 0x021;

// --- The board's settings, named as the routine at 02D358 leaves them ---
// The record the board's settings are kept in, at A5 - 0x6272. Its bytes:
//   0      which kind of record it is (the switches of the board, or one of the two the test menu
//          makes: 0x11 and 0x12);
//   1      the index of the setting the board is set to (see kSettingsIndex);
//   2      the setting itself (what the record's kind says it is);
//   3..5   the three settings the game keeps as bytes;
//   6      another byte setting;
//   7      the setting the game keeps at FF8073;
//   8, 9   two more byte settings;
//   0x10, 0x11 two more;
//   0x20.. three longs the game keeps.
constexpr uint32_t kRecordKind = kSettings + 0;
constexpr uint32_t kRecordSetting = kSettings + 2;
constexpr uint32_t kRecordBytes1 = kSettings + 3;  // -> FF8026
constexpr uint32_t kRecordBytes2 = kSettings + 4;  // -> FF8027
constexpr uint32_t kRecordBytes3 = kSettings + 5;  // -> FF8028
constexpr uint32_t kRecordByte4 = kSettings + 6;   // -> FF8029
constexpr uint32_t kRecordByte5 = kSettings + 7;   // -> FF8073
constexpr uint32_t kRecordByte6 = kSettings + 8;   // -> FF8084
constexpr uint32_t kRecordByte7 = kSettings + 9;   // -> FF8085
constexpr uint32_t kRecordByte8 = kSettings + 0x10; // -> FF802A
constexpr uint32_t kRecordByte9 = kSettings + 0x11; // -> FF802B
constexpr uint32_t kRecordLong1 = kSettings + 0x60; // -> FF8076
constexpr uint32_t kRecordLong2 = kSettings + 0x64; // -> FF807A
constexpr uint32_t kRecordLong3 = kSettings + 0x68; // -> FF807E
constexpr uint32_t kRecordLong4 = kSettings + 0x6C; // -> FF7FD4

// The kinds of record: the board's own switches, and the two the test menu makes.
constexpr uint8_t kRecordSwitches = 0x00;
constexpr uint8_t kRecordMenuOne = 0x11;
constexpr uint8_t kRecordMenuTwo = 0x12;

// --- The game's own settings, as the routine at 02D358 leaves them (from A5) ---
constexpr uint32_t kSettingKind = kVars + 0x01C;   // the record's first byte
constexpr uint32_t kSettingIndex = kVars + 0x01D;  // the record's second byte (the index)
constexpr uint32_t kSettingWords = kVars + 0x01E;  // a word of the record's kind
constexpr uint32_t kSettingByte1 = kVars + 0x023;  // the record's third byte
constexpr uint32_t kSettingByte2 = kVars + 0x024;
constexpr uint32_t kSettingByte3 = kVars + 0x025;
constexpr uint32_t kSettingByte4 = kVars + 0x026;  // the record's fourth byte
constexpr uint32_t kSettingByte5 = kVars + 0x027;  // the record's fifth byte
constexpr uint32_t kSettingByte6 = kVars + 0x028;  // the record's sixth byte
constexpr uint32_t kSettingByte7 = kVars + 0x029;  // the record's seventh byte
constexpr uint32_t kSettingCoinMode = kVars + 0x022; // the coin mode (from the table)
constexpr uint32_t kSettingContinue = kVars + 0x020;  // the continue setting (from the table)
constexpr uint32_t kSettingLong1 = kVars + 0x076;
constexpr uint32_t kSettingLong2 = kVars + 0x07A;
constexpr uint32_t kSettingLong3 = kVars + 0x07E;
constexpr uint32_t kSettingLong4 = kVars + 0x7FD4;
constexpr uint32_t kSettingByte8 = kVars + 0x072;  // cleared by the routine
constexpr uint32_t kSettingByte9 = kVars + 0x073;  // the record's eighth byte
constexpr uint32_t kSettingByte10 = kVars + 0x084; // the record's ninth byte
constexpr uint32_t kSettingByte11 = kVars + 0x085; // the record's tenth byte
constexpr uint32_t kSettingByte12 = kVars + 0x02A; // the record's sixteenth byte
constexpr uint32_t kSettingByte13 = kVars + 0x02B; // the record's seventeenth byte

// --- The routine that applies the board's settings ---
constexpr uint32_t kApplySettings = 0x02D358;

// --- The players that joined ---
// The game counts the players that have joined in a long at FF8076 (the routine at 02DACA adds one to
// it: it is how many have joined since the game was last set up).
constexpr uint32_t kPlayersJoined = kVars + 0x076;
// How many had joined the last time (the routine at 02DACA compares the two).
constexpr uint32_t kPlayersJoinedBefore = kVars - 0x6212;
// The number of players in the game: the port's own way to know that four can play at once (the game's
// own count of the players that joined is in FF8076, but that is a count since the last setup, not the
// number playing; FF802C is the number playing, and the board's own setting decides how many there can
// be).
constexpr uint32_t kPlayersPlaying = kVars + 0x02C;
// Set while the game is in its test menu (the routine at 02DACA does nothing then).
constexpr uint32_t kInTestMenu = kVars + 0x082;
// The game's own timing: a word of a value and a word of a delay (the routine at 02DACA sets them when
// a player joins, and the game waits).
constexpr uint32_t kWaitValue = kVars - 0x616E;
constexpr uint32_t kWaitDelay = kVars - 0x616A;
// The routine that applies the board's settings, and the one that counts a player joining.
constexpr uint32_t kPlayerJoins = 0x02DACA;
// The blocks that reach the routine that applies the settings with the record ready: 02D344 (the
// board's start, after the routine that fills the record) and 02D33E (the other way there, the record
// already filled), and 02D53E (the test menu left). The port's own choice of how many can play is put
// into the record from there, before the routine reads it.
constexpr uint32_t kBoardStart = 0x02D344;
constexpr uint32_t kBoardStartToo = 0x02D33E;
constexpr uint32_t kTestMenuLeft = 0x02D53E;

// --- The game's cases, as a table of steps ---
// The game runs its play as a table of cases: a table of steps (each a case's data), and a structure
// in A0 that says which case is running. The routine at 001A32 takes a case (the number of it in D0)
// from the table of cases and fills the structure in; the routine at 001A56 goes on to the next step of
// the case that is running. Between them they are what the game's play is made of, and the routine at
// 001A32 is called from all over the game.
// The structure in A0:
//   +0x20 the pointer to the step the case is at;
//   +0x30 the number of the step left in the case (a word);
//   +0x32 what the step is (a byte): 0 is the end of the case's steps, 0xFE is the end of the case;
//   +0x34, +0x38, +0x3C three longs the step carries.
constexpr uint32_t kCaseStep = 0x20;   // the pointer to the step
constexpr uint32_t kCaseStepsLeft = 0x30; // the number of the step left in the case
constexpr uint32_t kCaseKind = 0x32;    // what the step is
constexpr uint32_t kCaseLong1 = 0x34;
constexpr uint32_t kCaseLong2 = 0x38;
constexpr uint32_t kCaseLong3 = 0x3C;
constexpr uint32_t kCaseEnd = 0xFE;     // the kind that ends the case
// The routines that load a case and go on to the next step.
constexpr uint32_t kLoadCase = 0x001A32;
constexpr uint32_t kNextStep = 0x001A56;

// --- The game's trigonometry ---
// The game works out the sine and the cosine of an angle by a table: the routine at 001A94 takes the
// kind of the angle (in D0) and the angle itself (a byte at A0 + 0x2D) and leaves the two words of
// the table for that angle at A0 + 0x14 and A0 + 0x1C. The kinds are the tables at 1AB4 (a long each,
// naming where the table of the kind is), and each table is 64 bytes: sixteen pairs of words, one pair
// for each of the sixteen angles, the first the sine and the second the cosine (each 256 is a full
// turn's worth of a length).
constexpr uint32_t kAngle = 0x2D;      // the angle, in the structure in A0
constexpr uint32_t kSine = 0x14;       // the sine of the angle
constexpr uint32_t kCosine = 0x1C;     // the cosine of the angle
constexpr uint32_t kAngleTable = 0x4C; // the table of the kind, kept in the structure in A0
constexpr uint32_t kAngleKinds = 0x1AB4; // the table of the kinds
constexpr uint32_t kAngleValues = 0x1C4836; // the tables of the angles
constexpr uint32_t kAngleTableSize = 64; // a table of the kind is 64 bytes
constexpr uint32_t kAngleSteps = 16;   // and it holds sixteen angles
constexpr uint32_t kTrigonometry = 0x001A94;

} // namespace cps2recomp::ddsom

namespace cps2recomp {
// The board's settings: the page of the board's own settings, and the routine that applies them, written
// again as C++ (board.cpp).
void RegisterBoard();
// The game's cases: the routines that load a case and go on to its next step, written again as C++
// (cases.cpp).
void RegisterCases();
} // namespace cps2recomp
