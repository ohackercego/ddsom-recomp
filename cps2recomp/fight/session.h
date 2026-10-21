// SPDX-License-Identifier: GPL-3.0-only
// Copyright (C) 2026 Tommy McLeroy. Part of cps2recomp.
//
// A fighting game's training session, for every game on the board: the menu's "Start" (the board switched off and
// on, and the way to the character choice with two players worked unseen), player 1's controls choosing for both,
// the round kept as its fighting begins for "Start over" (the players put in the middle or a corner, sides swapped
// or not), the stage chosen, and the aids of fight.h put together: the combo readout, the input display, frame data,
// its meter and its log, hit boxes, health that comes back, and the dummy.
//
// A game's sources describe where the game keeps what this needs (Description), register it, and tell it from their
// hooks what the game does: each player's step of a fight (Step), a hit landing or being guarded (Contact), a hit's
// damage being taken (Hit). The game's own hook where it chooses the stage asks WantedStage, and its clock and its
// knockouts ask NoClock and NoKnockOut.
#pragma once

#include "fight.h"
#include "game.h"
#include "machine.h"

#include <cstdint>
#include <string>
#include <vector>

namespace cps2recomp {
class Archive;
}

namespace cps2recomp::session {

struct Description {
    // --- the game's memory ---
    uint32_t player1 = 0xFF8400, playerSize = 0x400, x = 0x10;
    bool xLong = false; // (a long with 16 bits of fraction, not a word)
    // Health: words, what the player has and (0: none) the most it can have back; the most in all; and (0: none) what
    // its bar shows, which a refill sets too
    uint32_t health = 0, healthCeiling = 0;
    uint16_t healthFull = 0;
    uint32_t healthShown = 0;
    // The round's clock (a byte: seconds left) and its most: a round is being fought while it is in 1..most
    uint32_t clock = 0;
    uint8_t clockFull = 99;
    // The camera (words: the picture's left edge and bottom in the stage) and every place the game keeps the camera
    // across, its own word too (each set by "Start over")
    uint32_t cameraX = 0, cameraY = 0;
    std::vector<uint32_t> cameraCopies;
    // Where the players stand: at a round's start, and the ends they can go to; the camera's ends; how far apart
    // the players are put in a corner
    uint16_t startX[2] = {0, 0};
    int stageLeft = 0, stageRight = 0, cameraLeft = 0, cameraRight = 0;
    int cornerApart = 120;
    // The picture's line that height 0 in the stage is on, with the camera at the bottom (for the boxes)
    int pictureFloor = 239;

    // --- the game ---
    // A round is being fought, for the aids (none: the clock in 1..clockFull and nobody's health over its most)
    bool (*fighting)() = nullptr;
    // A player's health may come back now (none: whenever it has not been hit for a moment)
    bool (*mayRefill)(int player) = nullptr;
    // A player has chosen its character (then player 1's controls choose for player 2)
    bool (*chosen)(int player) = nullptr;
    // What the game read off the panel for a player this frame, as Button bits, and whether the player faces left
    uint16_t (*panelHeld)(int player) = nullptr;
    bool (*facingLeft)(int player) = nullptr;
    // A move's start, for the frame data log (none: no log)
    fight::MoveLog::StartFn moveStart = nullptr;
    // The boxes the game's hit tests test (none: no hit boxes)
    std::vector<fight::StageBox> (*boxes)(Machine &machine) = nullptr;
    // For the dummy's guard: whether something of player 1's is near enough to guard, and whether it must be guarded
    // standing (none: the dummy guards whenever player 1 attacks)
    void (*guardView)(Machine &machine, bool &inRange, bool &standing) = nullptr;
    // Whether the dummy is knocked down (none: taken from how long it reels)
    bool (*knockedDown)() = nullptr;
    // The game's own readouts, beside the toolkit's (lines and boxes of the HUD); and a player's readout under its bar
    // in place of the toolkit's combo readout (with the game's own lines about the player), or none
    void (*hud)(Machine &machine, std::vector<HudLine> &lines, std::vector<HudBox> &boxes) = nullptr;
    void (*playerReadout)(int player, const fight::Combos &combos, std::vector<HudLine> &lines) = nullptr;
    // Where the HUD's parts are: the input display's first line and its lines' spacing, the frame data's line and the
    // meter's top (parts of the picture, and its line)
    float inputY = 0.30f, inputSpacing = 0.062f, framesY = 0.72f;
    int framesTop = 193;
    // The dummy: what the game adds to what it is told of the fight (none: nothing), what the game does after the dummy
    // has worked the controls, and whether it acts only once a round's aids have started afresh
    void (*dummyView)(Machine &machine, fight::DummyView &view) = nullptr;
    void (*afterDummy)(Machine &machine, const fight::DummyView &view) = nullptr;
    bool dummyAfterAfresh = false;
    // The session's state in a save state with the frame data first (the order a game's earlier save states have)
    bool framesFirstInState = false;
    // The game's own training features: after the combo readout's, and after the hit boxes'
    std::vector<Feature> readoutFeatures, boxFeatures;
    // The input display's words (none: the toolkit's)
    const char *inputWords = nullptr;
    // The wide picture: how far right the stage is drawn in it, and the columns it adds on each side (none: 0)
    int (*wideStage)() = nullptr;
    int (*wideSide)() = nullptr;
    int (*wideMargin)() = nullptr;
    // What the dummy can do of the game's own
    fight::DummyOptions dummy;
    // The aids of the game's own, as training aids (they work only in a session): their names
    std::vector<const char *> ownAids;
    // The stage choices (first "The game's choice"), and the words for them
    std::vector<const char *> stageChoices;
    const char *stageWords = "Where a training session's fight is: the game's own choice or a stage named here. Takes effect when the "
                             "characters are next chosen.";
    // A variable that prints every move's frame data to stderr too (besides CPS2RECOMP_FRAMES), or none
    const char *framesPrintVariable = nullptr;
};

// The game's description, and the menu's training features and commands (on modes.h's training page)
void Register(const Description &description);

// The menu's "Start": the board switched off and on and taken to the character choice with two players
void StartSession(Machine &machine);
// A session is on
bool InSession();
// The stage the machine that is running is to fight on, for the game's choice of stage (0: the game's choice):
// what a mode wants (alternative.h), else the session's "Stage"
int WantedStage();
// No clock, and nobody knocked out: a session, the stage viewer, or a fighting mode that wants it
bool NoClock();
bool NoKnockOut();

// From the game's hooks: a player's step of a fight; a hit landing on a player or guarded; a hit's damage taken (a chip
// off a guarded hit is never part of a combo)
void Step(int player, const fight::StepView &view);
void Contact(int victim, bool guarded);
void Hit(int player, unsigned damage, unsigned comesBack = 0, bool chip = false);
// What a player is doing (the frame data's view of it)
fight::Doing Now(int player);

// Around the port's machine's frame (the game's beforeFrame and frame), and its save states
void BeforeFrame(Machine &machine);
void Frame(Machine &machine);
void State(Archive &ar);

} // namespace cps2recomp::session
