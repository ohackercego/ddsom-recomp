// SPDX-License-Identifier: GPL-3.0-only
// Copyright (C) 2026 Tommy McLeroy
//
// Vampire Savior: what the modes of the menu's "Alternative modes" page share (modes.cpp: attract mode and
// the stage viewer; battle.cpp: survival, the endless battle and the ring out). Most of it is for running more
// than one machine: the port's own, which is what is seen, and others kept as save states ("spares"), which
// are loaded into the port's machine, run and put away again within a frame.
#pragma once

#include "alternative.h"
#include "battle.h"
#include "machine.h"
#include "spare.h"
#include "vsav.h"

#include <cstdint>
#include <functional>
#include <vector>

namespace cps2recomp {

// What the machine that is running is to fight (fight/battle.h, MatchWanted), for the game's own choice of stage
// (training.cpp's hook)
MatchWanted &Wanted();
// The character a mode's fighter on side n (0, 1) is given for `character`: its hidden form when the "Hidden
// characters" feature asks for that side (choosing.cpp), or itself
int HiddenForm(int n, int character);

// Where the machine that is running is to have its camera (fight/battle.h, CameraWanted): camera.cpp's routines ask
CameraWanted &CameraAt();

// A round of a match is being fought
bool RoundFought();
// The controls of a machine on its way from switching on to a round of a two-player game: coins in, both
// Starts until both players are in, then the characters and their choices taken as they come (player 1's
// left to `one`, the controls as the port has them, when `choose` is set). True once the round is on.
bool ToRound(Machine &machine, int32_t &frames, bool choose = false);
// A sound asked for as the game asks: a tune's number, or kTuneOff before another tune
void PlaySound(uint16_t code);
// A stage's own tune (1 the first stage; family.h: 0, none of its own)
inline uint16_t StageTune(int stage) {
    return stage >= 1 && stage <= vsav::kStages ? vsav::family::kStageTunes[stage - 1] : 0;
}
constexpr uint16_t kTuneOff = 0xFF05;

// Objects by what they are (for Video::hideObject). The players, what they throw and sparks use colour sets
// 5-21, the panel (bars, names, the clock, the gauges) 0-4 and, for the faces beside the bars, the players'
// own with tiles 13800-143FF; a stage's own pieces 22-31; the players' shadows are in set 22 with some stages'
// pieces, and are told by their tiles.
bool IsShadow(uint32_t tile, uint16_t attributes);
bool NotTheStage(uint32_t tile, uint16_t attributes); // everything but the stage's pieces
bool ThePanel(uint32_t tile, uint16_t attributes);    // the panel only
bool NotTheFighters(uint32_t tile, uint16_t attributes); // the panel and the stage's pieces
// A player's shadow not drawn from the next frame on (-1: both drawn), for a player put out of the way or
// fallen off the stage: its shadow would stay on the floor (machines.cpp; the spares' frames draw both)
void NoShadow(Machine &machine, int player);

// Machines kept as save states and run beside the port's are the toolkit's (runtime/spare.h: Spare, RunSpare,
// Promote, Take, Give, Made); what is wanted (above), the camera's place and the wide picture are kept around a
// spare's frame by machines.cpp.

// Modes by the files that have them: whether one is on, and ending it
bool AttractMode();  // modes.cpp
bool StageViewer();
void EndModes();
// The colour sets of a player's own objects (the first player's 10-15, the second's 16-21), as a mask of bits
constexpr uint32_t kPlayerSets[2] = {0x0000FC00, 0x003F0000};
// The fighting modes (battle.cpp: the toolkit's, fight/battle.h), and the ring out's own movements at a player's
// step (A6): true when it has made the step itself
void RegisterBattle();
bool RingOutStep(M68K &c);

} // namespace cps2recomp
