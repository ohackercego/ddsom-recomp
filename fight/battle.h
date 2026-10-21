// SPDX-License-Identifier: GPL-3.0-only
// Copyright (C) 2026 Tommy McLeroy. Part of cps2recomp.
//
// The fighting modes of a fighting game's "Alternative modes" page, for every game on the board that has them.
// Each is a two-player match that is set up (who fights, where, and which of the two the game itself plays),
// watched, and followed with another: the next match is made ready on a spare machine (runtime/spare.h) while
// the last one's ending plays, and then takes its place.
//
//   Survival        the player against one opponent after another, with the health left from the fight
//                   before; over at the first knock-down, with the count of opponents beaten.
//   Endless battle  two fighters the game plays, with no panel; the one knocked down is replaced. A person
//                   takes a fighter over by touching the controls, and gives it back by leaving them alone.
//   Ring out        the player against one opponent after another, with no health: a stage's ends are open,
//                   and whoever is knocked over one is thrown out and falls through the floor. The winner jumps
//                   off, and the camera follows it down onto the next stage.
//
// A game's sources describe where the game keeps what these need (Description) and register it; they also
// make the ring out's own movements of a player at its step of a fight (RingOutMove) and let a player knocked
// about go over a stage's end (RingOutLetsOut), where the game keeps the players in.
#pragma once

#include "game.h"
#include "machine.h"

#include <cstdint>
#include <vector>

namespace cps2recomp {

class Archive;

// What the machine that is running is to fight when the game next makes a match (the game's sources hook where
// the game chooses them): the stage (1 the first; 0: the game's choice), the two characters and their colours
// (-1: the ones chosen at the character choice). Whoever runs a machine sets this first.
struct MatchWanted {
    int stage = 0;
    int characters[2] = {-1, -1};
    int colours[2] = {-1, -1}; // which of a character's sets of colours (-1: as chosen)
};

// Where the machine that is running is to have its camera (the picture's left edge and bottom, in the stage),
// whatever the players do: -1 leaves that to the game (its camera routines ask). Kept within the stage's ends,
// unless `beyondEnds` (the ring out's way down: the camera above a stage, for the fighters alone).
struct CameraWanted {
    int x = -1, y = -1;
    bool beyondEnds = false;
};

// The ring out's own movements of a player, made by the game's sources at the player's step of a fight: flung out
// over the end of the stage (at the first step; `side` -1 the left end, 1 the right), falling from there (the
// game's routine for the player not run), or (at one step) put in the next match in the air as it was in its
// jump over the old stage: at `x`, `y`, the rest of it as `record` has it (the player's record on the old stage,
// the description's playerSize bytes; which of it to take is the game's)
struct RingOut {
    enum Move { kNone, kFlungOut, kFalling, kDropsIn } move = kNone;
    int side = 0;
    int x = 0, y = 0;
    const uint8_t *record = nullptr;
};

namespace battle {

constexpr uint32_t kMostRecord = 0x400; // the most bytes of a player's record (kept for the way down)

struct Description {
    // --- the game's memory ---
    uint32_t result = 0;            // byte: the round's result: 0 still going, 1 player 1 has won, 2 player 2, 3 neither
    uint32_t cameraX = 0, cameraY = 0; // words: where the camera is (the picture's left edge and bottom)
    int cameraLeft = 256, cameraRight = 640; // the camera's ends across on every stage (the board's picture)
    // The players' records, and the records of what they throw (a byte `active` not zero for one in use); the
    // players' first, their objects' places across (longs, 16 bits of fraction) at `x`
    uint32_t player1 = 0, playerSize = 0;
    int playerRecords = 0;
    uint32_t thrown = 0, thrownSize = 0;
    int thrownRecords = 0;
    uint32_t active = 0, x = 0, y = 0;
    // A player's: who it is (byte), a byte set to 1 while the game plays it, who is in charge of it (byte;
    // `controlOwn` the player itself), half its width (word)
    uint32_t character = 0, computer = 0, control = 0, halfWidth = 0;
    uint8_t controlOwn = 0;
    // Bytes set to 2 for both players while a mode runs (the game counts down for one not set), or 0: none
    uint32_t plays = 0;
    // A byte: which of its sets of colours the player chose (kept from match to match), or 0: none
    uint32_t colours = 0;
    // A word: the game's random number (a match begun from the character choice is given its own, or every one
    // would play out the same way from there), or 0: none
    uint32_t random = 0;
    // Health (words): what the player has, and the most it can have back (the bar's other end), and in all;
    // what survival gives back for an opponent beaten. With `ceilingShown` the second is what the bar shows,
    // put at what is left.
    uint32_t health = 0, healthCeiling = 0;
    uint16_t healthFull = 0, healthBack = 0;
    bool ceilingShown = false;

    // --- the game ---
    int stages = 0;                 // the stages (1 to this)
    int stageToRound = 2000;        // frames a stage's machine takes to its round, at most, and a few
    int characterSpan = 0;          // characters at random: 0 to this, those choosable
    bool (*choosable)(int number) = nullptr;
    // A choice of the menu's "Fighter" rows (0: at random) as a character's number (-1: none)
    int (*rosterNumber)(int choice) = nullptr;
    // The menu's choices: who (first "Random"), and where (first "Shuffle", "Scroll", then the stages)
    std::vector<const char *> fighterChoices, stageChoices;
    uint16_t tuneOff = 0xFF05;      // a sound asked for before another tune

    // A game is being played (no: the board was reset)
    bool (*inGame)() = nullptr;
    // Player 1 is choosing a character (survival and the ring out: at the game's own pace, seen)
    bool (*choosing)() = nullptr;
    // The controls of a machine on its way from switching on to a round of a two-player game; true once the
    // round is on (player 1 left to its own controls when `choose`)
    bool (*toRound)(Machine &machine, int32_t &frames, bool choose) = nullptr;
    void (*playSound)(uint16_t code) = nullptr;
    MatchWanted &(*wanted)() = nullptr;
    CameraWanted &(*cameraAt)() = nullptr;
    // The game's other modes ended (one mode at a time), and whether the stage viewer draws the picture itself
    void (*endModes)() = nullptr;
    bool (*stageViewer)() = nullptr;
    // The wide picture (0: none): columns it adds on each side, and how far short of each end the camera stops
    int (*wideSide)() = nullptr;
    int (*wideMargin)() = nullptr;
    // Objects by what they are (Video::hideObject): all but the stage's pieces, the panel only, the panel and
    // the stage's pieces; a player's own colour sets, as a mask of bits
    bool (*notTheStage)(uint32_t tile, uint16_t attributes) = nullptr;
    bool (*thePanel)(uint32_t tile, uint16_t attributes) = nullptr;
    bool (*notTheFighters)(uint32_t tile, uint16_t attributes) = nullptr;
    uint32_t (*playerSets)(int player) = nullptr;
    // A player's shadow not drawn from the next frame on (-1: both drawn), or nullptr
    void (*noShadow)(Machine &machine, int player) = nullptr;
    // The ring out's way down: the player is in the air, and at the top of a jump (going up no more)
    bool (*inAir)(uint32_t player) = nullptr;
    bool (*atJumpTop)(uint32_t player) = nullptr;
};

// The game's description, and the menu's pages, commands and features of the modes
void Register(const Description &description);

} // namespace battle

// Whether one of these modes is on, and ending it
bool BattleMode();
void EndBattle();
bool BattleNoClock();    // the round's clock stands still
bool BattleNoKnockOut(); // nobody's health goes below nothing
// Around the port's machine's frame (the game's beforeFrame and frame), and its save states
void BattleBeforeFrame(Machine &machine);
void BattleFrame(Machine &machine);
void BattleState(Archive &ar);
// The ring out: what the player whose record is at `player` is to do at its step (above), and whether one knocked
// about at an end of the stage is not to be kept in there (it goes over the end)
RingOut RingOutMove(uint32_t player);
bool RingOutLetsOut(uint32_t player);

} // namespace cps2recomp
