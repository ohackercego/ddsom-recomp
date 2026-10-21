// SPDX-License-Identifier: GPL-3.0-only
// Copyright (C) 2026 Tommy McLeroy
//
// Super Puzzle Fighter II Turbo: names for what is known of the game's memory (found by experiment:
// PROGRESS.md says how).
#pragma once

#include "game.h"
#include "lift.h"

#include <cstdint>

namespace spf2 {

// The game's variables are reached from A5, which stays at kVars
constexpr uint32_t kVars = 0xFF8000;

// --- where the game is ---
// Words from A5, each a step (in twos) of the one before. The first is 2 once a game is being played; the
// next 2 before a match (choosing the mode, the characters) and 4 during it.
constexpr uint32_t kGameOn = 0xFF8000, kGameStep = 0xFF8004, kMatchStep = 0xFF8008, kRoundStep = 0xFF800C, kSubStep = 0xFF8010;
enum : uint16_t { kGameChoosing = 2, kGameMatch = 4 };
// Before a match (kMatchStep 2): kRoundStep 0 is the choice of mode (against the computer, or two players)
// and kChoosingCharacters the choice of characters
enum : uint16_t { kChoosingMode = 0, kChoosingCharacters = 6 };
// The choice of mode: where the cursor is (0: against the computer, 1: two players)
constexpr uint32_t kModeCursor = 0xFF0C8B;
// The choice of characters: a bit for each player who has chosen (bit 0 player 1, bit 1 player 2)
constexpr uint32_t kCharactersChosen = 0xFF0C8D;
// In a match: kMatchStep kMatchRound while a round is on, and under it kRoundStep kRoundPlayed while the
// players are playing it
enum : uint16_t { kMatchRound = 4, kRoundPlayed = 2 };

// --- coins ---
// Coins paid for and not yet used
constexpr uint32_t kCredits = 0xFF8076;
// Not zero when the board is set to free play: Start works without a coin. The game sets it from the
// operator's settings when it starts (kApplySettings: the coin setting's value kCoinFreePlay), together with
// the two words beside the counts of coins.
constexpr uint32_t kFreePlay = 0xFF809C, kCoinsPerCredit1 = 0xFF8072, kCoinsPerCredit2 = 0xFF807A;
constexpr uint32_t kApplySettings = 0x01DE52;
// The operator's settings as kept in the settings memory: a record of bytes; the first is the coin setting
constexpr uint32_t kSettings = 0xFF0F90;
enum : uint8_t { kCoinFreePlay = 0x12 };
// Who is playing: bit 0 player 1, bit 1 player 2
constexpr uint32_t kPlayersIn = 0xFF80A8;

// --- the control panel, as the game reads it once a frame (in its frame interrupt, about 0013EC) ---
// A player's word: buttons in the high byte (bit 0 turns the pair one way, bit 1 the other), the stick in the
// low byte (bit 0 right, 1 left, 2 down, 3 up); and the word of the frame before
constexpr uint32_t kControls[2] = {0xFF8058, 0xFF805C}, kControlsBefore[2] = {0xFF805A, 0xFF805E};
// Starts and coins, a byte a frame, the last four frames (bit 0 player 1's Start, bit 1 player 2's)
constexpr uint32_t kStartsAndCoins = 0xFF8060;

// --- players ---
// A player's record (A6 in the game's code); the second player's follows the first's
constexpr uint32_t kPlayer1 = 0xFF8300, kPlayerSize = 0x400;
constexpr uint32_t kScore = 0xAA;        // long
constexpr uint32_t kOpponent = 0x160;    // long: the other player's record
constexpr uint32_t kFieldBottom = 0x164; // long: the player's field (see below)
// Where the player's turn is: a step, the number of one of kTurnSteps (turn.cpp names them)
constexpr uint32_t kStep = 0x17E;        // byte
enum : uint8_t {
    kStepNewPair = 0,     // a new pair comes in, and falls as the player steers it
    kStepPairFalls = 1,   // the pair has landed: what has nothing under it falls
    kStepCount = 2,       // the field is counted; counter gems count down, and turn into gems
    kStepFall = 3,        // gems fall (after a break as well: the chain comes back here)
    kStepMakeBig = 4,     // big gems are made, in three passes
    kStepMakeBig2 = 6,
    kStepMakeBig3 = 7,
    kStepFindBreaks = 8,  // what breaks is found
    kStepBigBonus = 9,
    kStepFallAgain = 0x0A,
    kStepShowBreak = 0x0B, // the break is shown, then scored; with nothing breaking the turn goes on
    kStepJoinBig = 0x0C,  // big gems that make a rectangle together become one, in three passes
    kStepMakeBigLast = 0x0F,
    kStepNoteAttack = 0x10,
    kStepCountAgain = 0x11,
    kStepAttacked = 0x12, // the opponent's counter gems come into the field
    kStepEnd = 0x13,      // they fall; then the turn ends, or the round is lost
    kStepLost = 0x14,
};
// What the player holds, and held the frame before (words: buttons high, stick low), as the frame interrupt
// keeps them in the record; a person's are copied each frame to where the turn reads them
constexpr uint32_t kPadRead = 0x9A, kPadReadBefore = 0x9C, kPad = 0x140, kPadBefore = 0x142;
// The player's number (0, 1) and the same as a bit (1, 2)
constexpr uint32_t kPlayerNumber = 0x81, kPlayerBit = 0x180; // bytes
// From A5: the bits of the players who have won the round
constexpr int32_t kRoundWinners = 0x10C;
// From A5, byte: not zero while a player's gems are falling or breaking
constexpr int32_t kFieldsBusy = 0x149;
// From A5: frames counted
constexpr int32_t kFrameCount = 0x80; // word
// The falling pair: where its first gem is (kPairRow counts from the row above the field's top), and the two
// gems (the one that stays, and the one that turns about it)
constexpr uint32_t kPairColumn = 0x10, kPairRow = 0x14, kPairNow = 0x152; // words
constexpr uint32_t kPairStarted = 0x328; // byte: the round's first pair has been made
constexpr uint16_t kStartColumn = 3;
// Resting: when the pair has something under it, it can still be steered for a while (the count is here)
constexpr uint32_t kResting = 0x176;     // byte
// Words: the pair's place between two rows, for the picture; and how it moves when pushed down
constexpr uint32_t kSlide = 0x242, kPushSpeed = 0x388;
constexpr uint32_t kPushHeld = 0x329;    // byte: the push's sound has been made
constexpr uint32_t kHeldRight = 0x188, kHeldLeft = 0x34E; // words: frames the stick has been held that way
constexpr uint32_t kUnderDiamond = 0x1F2; // word: what the diamond has landed on (kDiamondDone once it has broken it)
constexpr uint16_t kDiamond = 5, kDiamondDone = 0xFE;
// Falling
constexpr uint32_t kLanding = 0x371;     // byte: 1 while something falls, 2 when it has just stopped
constexpr uint32_t kFell = 0x178;        // byte: something has fallen since this was last looked at
// Marks the game leaves as the field changes. Nothing in this program reads them.
constexpr uint32_t kNoteChanged = 0x171, kNoteBusy = 0x174, kNoteFalling = 0x177, kNoteMoved = 0x18C; // bytes
// Not zero when the pictures of the field's big gems are to be made again
constexpr uint32_t kRedrawBigGems = 0x266; // byte
// A break is shown for kBreakFrames frames before the gems go
constexpr uint32_t kBreakTimer = 0x175;  // byte
constexpr uint8_t kBreakFrames = 0x28;
// The opponent's counter gems coming into the field (drop.cpp)
constexpr uint32_t kRowsFallen = 0x306;  // word: rows of counter gems that have fallen in this round
constexpr uint32_t kDropColumn = 0x308;  // word: the column being filled, doubled
constexpr uint32_t kColumnOrder = 0x37B; // byte: which order of columns single counter gems fall in this turn
constexpr uint32_t kRowFilled = 0x37A;   // byte: gems of the row now being filled that have fallen (0-5)
constexpr uint32_t kFellThisTurn = 0x37D; // byte
constexpr uint32_t kAttacked = 0x318;    // byte: counter gems have come in this turn
constexpr uint32_t kMadeCount = 0x194;   // byte: counts the big gems made, 1 to 6 and round again
// Counter gems this player has earned and not yet sent: they fall on the opponent (who is shown the number)
constexpr uint32_t kAttack = 0x19E;      // byte
constexpr uint32_t kGemCount = 0x374;    // word: gems in the field, counted by CountField
constexpr uint32_t kDanger = 0xD1;       // byte: 0, 1 (kDangerSome gems or more), 2 (kDangerMany or more)
constexpr uint16_t kDangerSome = 36, kDangerMany = 54;
// Clearing the whole field ("all clear"): given once (kAllClearGiven) until the round's next
constexpr uint32_t kAllClearGiven = 0x3A6;  // byte
constexpr uint32_t kAllClearScore = 0x25C;  // long: grows by kAllClearPoints each time, and is added to the score
constexpr uint32_t kAllClearAttack = 0x392; // byte: grows by kAllClearGems each time, and is added to kAttack
constexpr uint32_t kAllClearPoints = 600;
constexpr uint8_t kAllClearGems = 6;
constexpr uint32_t kShowAllClear = 0x00C8CC; // the game's routine that puts the words on the screen
// Counter gems whose count has run out turn into ordinary gems, flashing for kTurnTime frames first
constexpr uint32_t kTurnTimer = 0x246; // byte
constexpr uint8_t kTurnTime = 0x20;
constexpr uint32_t kTurnSound = 0x02BA68;
// Breaking: what MarkBreaks leaves for the steps that follow
constexpr uint32_t kBreaking = 0x31A;       // byte: 1 when something in the field is breaking
constexpr uint32_t kBreakKind = 0x25A;      // word: the cell that last started a break, as found
constexpr uint32_t kBreakKindBefore = 0x258; // word
constexpr uint32_t kColoursBroken = 0x394;  // byte: a bit for each colour broken this turn (bit 0 blue ... 3 red)
constexpr uint32_t kChainLength = 0x19C;    // byte: breaks so far in this turn's chain
constexpr uint32_t kChainShown = 0xD8;      // byte: the chain number to show
constexpr uint32_t kChainStarted = 0x37C;   // byte
constexpr uint32_t kShowChain = 0x014314;
// A list of the turn's breaks: for each, the colour and how many gems (words; kBreakListAt: bytes used)
constexpr uint32_t kBreakListAt = 0x254, kBreakColours = 0x280, kBreakCounts = 0x2C0;
constexpr uint32_t kNoCounterBreaks = 0x38C; // byte: not zero when breaks leave the counter gems beside them alone
// From A5: not zero when clearing the field earns nothing
constexpr int32_t kNoAllClear = 0xD8;

// --- a break's score, and the attack it makes (attack.cpp) ---
// The parts of a break's score, words in the player's record. They are added up when the broken gems are
// taken away (ScoreBreak), and the total is what the attack is worked out from.
constexpr uint32_t kChainBonus = 0x1FC;   // for the second and later breaks of a chain
constexpr uint32_t kColourBonus = 0x1FE;  // for breaking more than one colour at once
constexpr uint32_t kBrokenCount = 0x200;  // gems taken away so far this break; then the bonus for many
constexpr uint32_t kBigGemBonus = 0x202;  // big gems' cells broken; then the bonus for them
constexpr uint32_t kGemPoints = 0x204, kCrashPoints = 0x206, kCounterPoints = 0x208;
constexpr uint16_t kPointsGem = 100, kPointsCounter = 10, kPointsExtraGem = 10, kPointsColour = 200;
constexpr uint32_t kChainBonuses = 0x0100C8;   // table of words: by the break's place in the chain, less two
constexpr uint32_t kBigGemBonuses = 0x00FFF8;  // table of words: points a cell, by the number of cells less one
constexpr uint16_t kBigGemBonusLast = 50, kBigGemBonusCapped = 150;
constexpr uint32_t kBigGemBonusCap = 0x3A4;    // byte: not zero when a big gem's cells earn kBigGemBonusCapped at most
constexpr uint32_t kLongestChain = 0x32E;      // byte
constexpr uint32_t kColoursBrokenCount = 0x393; // byte
constexpr uint32_t kBigGemCellsBroken = 0x32D; // byte; and a byte for each colour from kBigGemCellsByColour
constexpr uint32_t kBigGemCellsByColour = 0x3A0;
constexpr uint32_t kCrashKindsBroken = 0x1E8;  // six bytes: set when a crash gem of that kind (8-13) broke
constexpr uint32_t kLastBrokenColumn = 0x35A, kHighestBrokenRow = 0x35C; // words, as the loops count them
// The round's time, in seconds, as far as kRoundSecondsMost. Attacks grow with it (kAttackTimes: the
// seconds at which they step up; kAttackTenths: tenths added at each step), and from kNoOffsetAfter on
// attacks are no longer set against each other.
constexpr uint32_t kRoundSeconds = 0x340; // word
constexpr uint16_t kNoOffsetAfter = 180;
constexpr uint32_t kAttackTimes = 0x010110, kAttackTenths = 0x010130;
// The operator's damage setting (from A5; 0-7) and what it does to an attack: tenths, with ten added
constexpr int32_t kDamageLevel = 0xA1;
constexpr uint32_t kDamageTenths = 0x010150;
constexpr uint32_t kHalfAttack = 0x1FA;     // byte: not zero when the player's attacks count half
constexpr uint32_t kComputer = 0x9E;        // byte: not zero when the computer plays this side
constexpr uint32_t kCharacter = 0x82;       // byte: 8 and 9 are the hidden characters whose attacks are weaker
// With handicaps (the game's mode kModeHandicap, from A5), each player has a level and the weaker is helped
constexpr int32_t kMode = 0xAC;
constexpr uint8_t kModeHandicap = 3;
constexpr uint32_t kHandicap = 0xA9;        // byte
constexpr uint8_t kAttackMost = 250;
constexpr uint32_t kAttackBeforeOffset = 0x358; // byte
constexpr uint32_t kShowChainScore = 0x01575E;

// Where the sound routines put aside the registers they are called with (d0-d3, a3, a4)
constexpr uint32_t kSoundSavedRegisters = 0xFF1336, kSoundSavedRegistersEnd = 0xFF134E;

// --- a player's field of gems ---
// Rows of eight words: a wall (kWall), six cells, a wall. kFieldBottom is the address of the row that is
// second from the top; rows go down from there to the bottom row (kBottomRow from it), and under that is a
// row of walls. Fourteen rows hold gems; the top ones are above the picture.
// A second field of the same shape, kFieldExtra after the first, has a word for each cell that says which
// big gem the cell is part of.
constexpr int kFieldColumns = 6, kFieldRows = 14;
constexpr uint32_t kRowSize = 16, kBottomRow = 0xC0, kFieldExtra = 0x400;
constexpr uint16_t kWall = 0x00FF;
// A cell's low byte: what is there (kKind) and marks
constexpr uint16_t kKind = 0x0F, kColour = 0x07;
enum : uint16_t {
    kEmpty = 0, kBlue = 1, kYellow = 2, kGreen = 3, kRed = 4,
    kCounterFlash = 6, // a counter gem turning into a gem, in the frames it is shown as one
    kCounter = 7,      // a counter gem: the high byte has its colour (high four bits) and its count (low four)
    kCrash = 8,        // a crash gem: 8 and its colour
};
enum : uint16_t {
    kCellBreaking = 0x10, // on its way out
    kCellMarked = 0x20,   // part of a group that a crash gem has reached
    kCellSeen = 0x40,     // has spread its mark to its neighbours
    kCellBig = 0x80,      // part of a big gem
};
// A counter gem caught by a break beside it, and the same as it is left by a break that has spent itself
constexpr uint8_t kCounterBroken = 0x37, kCounterGone = 0x57;

// --- for rewrites of the game's routines: its memory and registers by name ---

// Memory anywhere the 68000 reads data from (the ROM's tables as well as RAM)
inline uint8_t Mem8(cps2recomp::M68K &c, uint32_t address) {
    return static_cast<uint8_t>(c.Read(address, 0));
}
inline uint16_t Mem16(cps2recomp::M68K &c, uint32_t address) {
    return static_cast<uint16_t>(c.Read(address, 1));
}
inline uint32_t Mem32(cps2recomp::M68K &c, uint32_t address) {
    return c.Read(address, 2);
}
// The ROM as the program reads it where it names a table by its distance from the instruction (the board
// deciphers what is read as program: such a table is not what Mem8 and Mem16 see at the same address)
inline uint8_t Code8(cps2recomp::M68K &c, uint32_t address) {
    return static_cast<uint8_t>(c.ReadProgram(address, 0));
}
inline uint16_t Code16(cps2recomp::M68K &c, uint32_t address) {
    return static_cast<uint16_t>(c.ReadProgram(address, 1));
}
// The game's variables, by their distance from A5
inline uint32_t Var(cps2recomp::M68K &c, int32_t offset) {
    return (c.a[5] + static_cast<uint32_t>(offset)) & 0xFFFFFF;
}
// A register's low word or byte set, the rest of it kept (as move.w and move.b do)
inline void SetLow16(uint32_t &reg, uint32_t value) {
    reg = (reg & 0xFFFF0000) | (value & 0xFFFF);
}
inline void SetLow8(uint32_t &reg, uint32_t value) {
    reg = (reg & 0xFFFFFF00) | (value & 0xFF);
}

inline uint32_t Player(int n) {
    return kPlayer1 + static_cast<uint32_t>(n) * kPlayerSize;
}
// The cell in a player's field: column 0-5 from the left, row 0-13 from the bottom
inline uint32_t Cell(uint32_t player, int column, int row) {
    return cps2recomp::Ram32(player + kFieldBottom) + kBottomRow - static_cast<uint32_t>(row) * kRowSize +
           static_cast<uint32_t>(column + 1) * 2;
}

} // namespace spf2

namespace cps2recomp {
// What a player's last break came to (attack.cpp keeps it, for the training readout)
struct BreakNote {
    unsigned serial = 0; // goes up with every break
    int chain = 0, gems = 0, score = 0;
    int attack = 0;    // counter gems the break made
    int cancelled = 0; // of the opponent's waiting attack, how many it took away
    int waiting = 0;   // what the player then has waiting to fall on the opponent
};
const BreakNote &LastBreak(int player);
} // namespace cps2recomp
