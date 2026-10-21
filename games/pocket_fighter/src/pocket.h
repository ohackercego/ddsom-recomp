// SPDX-License-Identifier: GPL-3.0-only
// Copyright (C) 2026 Tommy McLeroy
//
// Super Gem Fighter Mini Mix (Pocket Fighter): names for what is known of the game's memory and routines, each found
// by experiment in this game (PROGRESS.md). Its variables are reached from A5 = FF8000, as in the Alpha games.
#pragma once

#include "game.h"
#include "lift.h"
#include "machine.h"

#include <cstdint>

namespace pocket {

using cps2recomp::M68K;

// --- where the game is ---

// Words from A5, each a step (in twos) of the one before, as in the Alpha games: the first is 2 once a game is being
// played (and the board's start and its show step through it before: title.cpp); the next 2 while the players choose
// their characters and 4 during the match; then 4 while a round is on, and under that 6 while it is being fought (2
// and 4 its introduction: the players walking in, "Round 1"). Traced with --watch.
constexpr uint32_t kMode = 0xFF8000, kGameStep = 0xFF8004, kMatchStep = 0xFF8008, kRoundStep = 0xFF800C;
enum : uint16_t { kGameChoosing = 2, kGameMatch = 4, kMatchRound = 4, kRoundFought = 6 };

// Who is in: a byte, bit 0 player 1, bit 1 player 2 (RAM dumps before and after each Start: 0, 1, 3)
constexpr uint32_t kPlayersIn = 0xFF810D;

// The stage a match is fought on: a word, the stage's number doubled (0 to 0E). Each character has a stage (the
// table at 009A3C, read pc-relative); the game takes one of the two players' at 009A0A.
constexpr uint32_t kStage = 0xFF8180;
constexpr int kStages = 8;
// The stages, by whose they are (each picture seen with kStage held at each number)
inline constexpr const char *kStageNames[kStages] = {
    "Felicia's (the shop)",  "Ibuki's (the big screen)", "Hsien-Ko's (the snowy village)", "Chun-Li's and Sakura's (the elephant)",
    "Morrigan's and Dan's (Chinatown)", "Ryu's (the forge)", "Tessa's and Akuma's (the castle)", "Ken's and Zangief's (the hall)"};
// Each stage's tune (stage n's is n + 1: found by the custom soundtrack's log over a fight on each), and the
// character choice's and the versus screen's
constexpr uint16_t kTuneChoose = 0x0B, kTuneVersus = 0x0F;

// The sound queue: a ring of sixteen-byte requests at A5+6230, the word at A5+7230 the place of the next (003264 puts
// one in; 0031A2 sends the oldest to the sound CPU), as Vampire Savior's
constexpr uint32_t kSoundQueue = 0xFFE230, kSoundQueueEnd = 0xFFF230;

// The camera: the picture's left edge in the stage (words; 0 to 100 across: the stages are 640 wide), and every place
// the game keeps it across, its own among them (all found moving together in RAM dumps as the players walked to one end)
constexpr uint32_t kCameraX = 0xFF8290, kCameraY = 0xFF8294;
inline constexpr uint32_t kCameraCopies[] = {0xFF8290, 0xFF802A, 0xFF82A0, 0xFF82A8};
constexpr int kCameraLeft = 0, kCameraRight = 0x100;
// Where the players stand at a round's start, and the walls (player 2 stops at 270 with the camera at its right end;
// the left the same distance from 0)
constexpr uint16_t kStartX[2] = {0xE8, 0x198};
constexpr int kStageLeft = 0x10, kStageRight = 0x270;

// The game's loop of notices, introduction, title, fights and rankings (kMode, traced with no coin in; the steps
// between are fades): 0C its start, 10 the QSound notice, 14 the maker's, 18 the introduction (to the title), 1C the
// advisory, 20 a fight, 24 the score ranking, 2E the recycling notice, 32 the maker's again, 36 the title, 3A the
// advisory, 3E another fight, 42 the versus ranking; then 0C again
enum : uint16_t { kLoop = 0x0C, kIntro = 0x18, kFirstFight = 0x20, kScores = 0x24, kSecondFight = 0x3E, kVersus = 0x42 };

// The round's clock: seconds left, a byte (99 at a round's start; a second off at 0083E6, subq.b #1,0x189(a5)), with
// its BCD copy for the picture in the byte before (FF8188) and the frames left of the second after (FF818A, counted
// down at 0083DC). Found with --watch through a fight.
constexpr uint32_t kRoundClock = 0xFF8189, kClockFrames = 0xFF818A;

// Credits paid for and not yet used, a byte (RAM dumps after two, four and six coins: 1, 2, 3; a new board's settings
// memory asks two coins a credit). The coins toward the next credit are at FF8143.
constexpr uint32_t kCredits = 0xFF80F6;

// The game's own free play, one of the operator's coin modes (the test menu's System page: 00-08 one coin for 1-9
// credits, 09-10 two to nine coins a credit, 11 two coins to start and one to continue, 12 free play; 09 on a new
// board). The board's start reads the mode (001796) into kCoinMode, and for free play sets kFreePlay and the two coin
// words to 0101 (001814); the other modes leave kFreePlay clear and take their coin words from a table (00228C). The
// game tests kFreePlay wherever it counts credits.
constexpr uint32_t kCoinMode = 0xFF8117, kFreePlay = 0xFF811C, kCoinsPerCredit1 = 0xFF80F2, kCoinsPerCredit2 = 0xFF80FA;
constexpr uint8_t kCoinFreePlay = 0x12;

// --- the players ---

// Each player's record, 0x400 long, the first at FF8400 (as in the Alpha games)
constexpr uint32_t kPlayer1 = 0xFF8400, kPlayerSize = 0x400;
inline uint32_t Player(int n) {
    return kPlayer1 + static_cast<uint32_t>(n) * kPlayerSize;
}
// Four bytes of steps, each a step of the one before (found by RAM dumps of player 1 standing, crouching, walking,
// jumping and attacking): in a fight the first is 2; the second says who is in charge of the player (0 the player,
// kAction then saying what it is doing; 2 a hit: reeling); while choosing, the second is 2 once the player has chosen
// (017598)
constexpr uint32_t kState = 0x04, kControl = 0x05, kAction = 0x06, kActionStep = 0x07;
enum : uint8_t { kControlOwn = 0x00, kControlReeling = 0x02, kChosen = 0x02 };
enum : uint8_t { kActionStand = 0x00, kActionCrouch = 0x02, kActionWalk = 0x04, kActionJump = 0x06, kActionAttack = 0x0A, kActionGuard = 0x0C, kActionSpecial = 0x16 };
constexpr uint32_t kFacesRight = 0x0B; // byte: 1 facing right (a box's offset across is kept as if facing left: 016088)
constexpr uint32_t kColourSet = 0x0D;  // byte: the first of the player's colour sets (player 1 10, player 2 15)
constexpr uint32_t kX = 0x10;          // word: where it stands across the stage
constexpr uint32_t kY = 0x14;          // word: height, upwards; kGround on the floor
constexpr uint16_t kGround = 0x30;
constexpr uint32_t kFrame = 0x1C;      // long: the animation frame being shown (in the ROM)
// Longs, 16 bits of fraction: the speeds across and up, and what is taken off them each step (a jump: up 0x68000 as it
// leaves the floor, pulled by 0x8000 a step; RAM dumps through a jump)
constexpr uint32_t kSpeedX = 0x20, kSpeedY = 0x24, kPullX = 0x28, kPullY = 0x2C;
constexpr uint32_t kHitStop = 0x4C;    // byte: frames the player stands frozen while a hit lands (both players)
constexpr uint32_t kAirborne = 0x4F;   // byte: FF while in the air (00E5D2: thrown into the air by a knock-down; a jump's too)
constexpr uint32_t kAttacking = 0x105; // byte: 1 while an attack of the player's is going on
constexpr uint32_t kCombo = 0x144;     // byte: hits in the combo being taken
// The super gauge: bytes, its stocks (9 at most: 013176) and the bar towards the next (013160 on); and the stun, a
// byte of what has built up and the most it can (a player is stunned there) (named in the FBNeo training script for
// this set; the stocks' most found here)
constexpr uint32_t kGaugeStocks = 0x194, kGaugeBar = 0x195;
constexpr uint8_t kGaugeStocksFull = 9;
constexpr uint32_t kStun = 0x17F, kStunMost = 0x19E;
// Words the camera's routine reads (01FAF0): half the player's width (the camera keeps X less and more than it in the
// picture), and a byte: not zero, the camera does not follow the player
constexpr uint32_t kHalfWidth = 0x130, kNotFollowed = 0xAD;
// Who the player is: a byte (the cursor writes it as it moves, 017586, and the fight reads it, 003806 on). kChoice
// keeps the last character chosen for the next game (0074EE copies it back).
constexpr uint32_t kCharacter = 0x381, kChoice = 0x3F0;
constexpr int kCharacters = 12;
// The characters by number, each seen in a fight with kCharacter held at it; Dan and Akuma are the hidden two
inline constexpr const char *kCharacterNames[kCharacters] = {"Ryu",      "Ken",    "Chun-Li", "Sakura",  "Morrigan", "Hsien-Ko",
                                                             "Felicia", "Tessa", "Ibuki",   "Zangief", "Dan",      "Akuma"};
// Boxes (the hit test 0154CA, the boxes' test 016088, the push 016188). A box is four words: its middle from where the
// player stands (across turned about when facing right; up from kY), its whole width and height. The player's record
// names two boxes it can be hit in (bytes kHurtA and kHurtB, numbers into the tables of eight-byte boxes at the longs
// kHurtTableA and kHurtTableB; 0 none) and its push box (kPush into kPushTable). The animation frame names the attack
// (its byte kFrameAttack, a number into the table of 32-byte attacks at the long kAttackTable: the box, then what the
// attack does).
constexpr uint32_t kHurtTableA = 0x80, kHurtTableB = 0x84, kAttackTable = 0x88, kPushTable = 0x8C; // longs
constexpr uint32_t kHurtA = 0x90, kHurtB = 0x91, kPush = 0x93;                                    // bytes
constexpr uint32_t kFrameAttack = 0x0A;
constexpr uint32_t kBoxSize = 8, kAttackSize = 0x20;
constexpr uint32_t kBoxX = 0, kBoxY = 2, kBoxWidth = 4, kBoxHeight = 6;
// The control panel as the game read it this frame, from A5: a word a player, buttons above (bits 0-2 the three) and
// the stick below (bit 0 right, 1 left, 2 down, 3 up). Found holding right, up and the first button for player 1 and
// down and the third for player 2.
constexpr uint32_t kControls[2] = {0xFF805C, 0xFF8060};

// --- the characters' colours ---

// A character's colours, in the ROM: from kColourSets, kColourSetsEach sets a character (by its number), each
// kPalettesInSet palettes of 16 colours (words, 4 bits each of red, green and blue; the game keeps the brightness the
// palette buffer had). When a round starts the game copies the player's set to its colour sets in the buffer (0274B0:
// kColourSets + (character * 4 + choice) * 0xA0). The player's byte kColourChoice picks the set: 0 Punch chose the
// character, 1 Kick, 2 Special, 3 two or more buttons at once (0175C8). Found from the palettes a fight showed, looked
// for in the ROM.
constexpr uint32_t kColourSets = 0x2099E8;
constexpr int kColourSetsEach = 4, kPalettesInSet = 5, kColoursInPalette = 16;
constexpr uint32_t kColourSetSize = kPalettesInSet * kColoursInPalette * 2;
constexpr uint32_t kColourChoice = 0x39F; // byte

// Objects by what they are (for Video::hideObject; found by hiding colour sets one at a time and in groups in the
// stage viewer's picture, and from the object list): sets 00-0F are the panel (the bars, names, gem counters) and the
// item chest (0A); 10-19 the players, five sets each from kColourSet (player 1's 10, player 2's 15: the set's five
// palettes, kPalettesInSet), with what they throw (Ryu's fireball 12) and their faces on the panel; 1A on the stage's
// own pieces, and in 1F the players' shadows (tiles 1D8E0-1D8FF, on
// the floor's line)
inline bool IsShadow(uint32_t tile, uint16_t attributes) {
    return (attributes & 0x1F) == 0x1F && tile >= 0x1D8E0 && tile < 0x1D900;
}
inline bool NotTheStage(uint32_t tile, uint16_t attributes) {
    return (attributes & 0x1F) < 0x1A || IsShadow(tile, attributes);
}
// The panel: its own sets, and the faces beside the bars, which are in the players' sets (tiles 14460 and 14Bxx; the
// fighters' own tiles were all below 3100 in the object lists looked at)
inline bool ThePanel(uint32_t tile, uint16_t attributes) {
    const unsigned set = attributes & 0x1F;
    return (set < 0x10 && set != 0x0A) || (set < 0x1A && (tile == 0x14460 || (tile >= 0x14B00 && tile < 0x14C00)));
}
inline bool NotTheFighters(uint32_t tile, uint16_t attributes) {
    return ThePanel(tile, attributes) || ((attributes & 0x1F) >= 0x1A && !IsShadow(tile, attributes));
}

// The round's result: bit 0 set when player 2 is knocked out (its health below nothing), bit 1 player 1 (0083B8,
// every step of a fight): 1 player 1 has won, 2 player 2, 3 neither
constexpr uint32_t kRoundResult = 0xFF818C;
// A byte of a player's: 1 while the game plays it (the fight step goes to the computer's routine, 0149D6, for it:
// 00C602; found 1 for player 2 in a one-player game, 0 in a two-player one)
constexpr uint32_t kComputer = 0x39E;
// The records of what the players throw and the like: a pool of 14 of 0x100 bytes from FF8C00 (01B30E sets it up;
// 01B52C takes one from it); a byte kActive not zero for one in use. Ryu's fireball was the last.
constexpr uint32_t kThrown = 0xFF8C00, kThrownSize = 0x100;
constexpr int kThrownRecords = 14;
constexpr uint32_t kActive = 0x00;

// --- routines ---
constexpr uint32_t kPlayerControl = 0x00C642; // a player's fight step, where it goes on by kControl (00C602; once a step each)
constexpr uint32_t kHitLands = 0x0155F2;      // an attack has hit (A6 the attacker, A1 the victim, A3 the attack)
constexpr uint32_t kChooseStage = 0x0099E0;   // the stage for a match is chosen (from one of the players' characters)
// A player's health, a word (0x90 at a round's start, 0082F4; below nothing a knockout, 015CE6); +42 the bar shown.
// Every hit and throw takes its damage off at 015D6E (sub.w d4,0x40(a1)), the only writer seen with --watch on both
// players' health through fights and made-up play.
constexpr uint32_t kHealth = 0x40;
constexpr uint16_t kHealthFull = 0x90;

// --- for hooks ---

inline void SetLow16(uint32_t &reg, uint32_t value) {
    reg = (reg & 0xFFFF0000) | (value & 0xFFFF);
}

} // namespace pocket

namespace cps2recomp {
// The wide picture (widescreen.cpp): the columns it adds on each side (0: none), how far right the stage is drawn in it,
// how far short of each end the camera stops (64, or 0: played as on the board), and a spare's frame drawn as wide as
// the port's (and the port's machine back)
int WideSide();
int WideStage();
int WideMargin();
void WideForSpare(Machine &machine);
void WideForPort(Machine &machine);
} // namespace cps2recomp
