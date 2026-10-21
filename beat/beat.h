// SPDX-License-Identifier: GPL-3.0-only
// Copyright (C) 2026 Tommy McLeroy. Part of cps2recomp.
//
// The modes of a beat-'em-up's "Alternative modes" page, for every game on the board that has them (fight/battle.h
// has a fighting game's). Each is started by a command of the in-game menu and ends when the board is reset, when
// another is started, or as said below.
//
//   Attract mode   the game's own show on a loop, with no coin in and its sound on, or only some of its parts (the
//                  story, the demonstrations, the characters...): the game's sources leave the others out. Start
//                  (or a coin) ends it.
//   Score attack   one player, one credit, no continue and no cheats, from the first round: the board's own game,
//                  for the best score, which is kept (score_attack.txt beside settings.ini). It ends with the game.
//   Stage viewer   a stage (a part of a round) with nobody on it: no players, no enemies, no panel, only the stage
//                  and its music. The stick scrolls it, or it scrolls by itself from one end to the other and back;
//                  "Infinite" is every stage in turn. It is a game begun unseen and taken to that stage, whose camera
//                  is then the viewer's: the game's sources say how to get there, and how far the camera goes.
//   Character viewer  one of the players' characters by itself, on one of the stages or on white or black, at any
//                  size, moved with player 1's controls or going through any of its animations (beat/model.cpp).
//                  It is a game begun unseen on the stage with that character, the enemies standing still and unseen;
//                  an animation is shown by putting its frames into the character's record, as the game would.
//
// A game's sources describe where the game keeps what these need (Description), register it, and ask while the
// game runs whether a mode is on: in attract mode they leave out the parts not wanted and put the board's demo
// sound on; in score attack they put free play, the cheats, "Start at round" and the board's continue out of the way.
#pragma once

#include "game.h"
#include "machine.h"

#include <cstdint>
#include <vector>

namespace cps2recomp {

class Archive;

namespace beat {

struct Description {
    // --- attract mode ---
    // The "Shows" choices after "Everything" (the parts the game's sources can show alone), and the words of the row
    std::vector<const char *> shows;
    const char *showsWords = "";
    // The board's start is over and its show has begun (the parts can be told apart from here on)
    bool (*showBegun)(const Machine &machine) = nullptr;
    // --- score attack ---
    // The board's start is over: a credit and Start begin a game
    bool (*canStart)(const Machine &machine) = nullptr;
    // Puts a credit in for player 1, and player 1's credits (the game has taken it when they are 0 again)
    void (*credit)() = nullptr;
    int (*credits)() = nullptr;
    // A game is on, up to its game over (not the show's demonstrations)
    bool (*inGame)() = nullptr;
    // Player 1's score, as the game shows it
    uint32_t (*score)() = nullptr;
    // --- stage viewer ---
    // The "Stage" choices (before "Infinite"), and the words of the row
    std::vector<const char *> stages;
    // The controls of a machine on its way from switching on to stage `stage` (from 0); true once it is there and
    // the camera is the viewer's (it is run unseen meanwhile)
    bool (*toStage)(Machine &machine, int stage, int32_t &frames) = nullptr;
    // The way the camera goes through the stage, in the game's own units: the places it passes, in order, across and
    // up and down (y -1: as the game has it, which moves it up and down itself); one place, or none: the camera is the
    // game's own, which moves it by itself. The game draws a stage's new columns and rows as its camera crosses them,
    // so the viewer starts where the game's camera is (stageCamera) and moves it a few pixels a frame at most.
    struct Place {
        int x, y;
    };
    void (*stagePath)(int stage, std::vector<Place> &path) = nullptr;
    void (*stageCamera)(int &x, int &y) = nullptr;
    // Each frame of viewing, before the game runs it: where the camera is to be (-1: as the game has it). The game's
    // sources keep the players out of harm and the enemies away.
    void (*view)(Machine &machine, int stage, int x, int y) = nullptr;
    // The scroll screens shown while viewing (Video::layerMask: the panel's left out; objects are never shown)
    unsigned viewLayers = 0xF;
    // --- character viewer ---
    // The characters a player can be (the game's sources put the one chosen in as a game the viewer begins begins)
    std::vector<const char *> characters;
    // A character's animations: its lists (named A, B, C... in this order), each the frames its animations begin at
    void (*animations)(int character, std::vector<std::vector<uint32_t>> &lists) = nullptr;
    // A frame of an animation: how many frames of the game it is shown for (0: for good), and the frame after it (0:
    // the animation ends there; the same frame: it is held)
    int (*frameTime)(uint32_t frame) = nullptr;
    uint32_t (*nextFrame)(uint32_t frame) = nullptr;
    // Whether an address is a frame (none: every one the game's lists and frames give is): one that is not is not shown
    bool (*isFrame)(uint32_t frame) = nullptr;
    // Each frame, before the game runs it: the frame to show on the character (0: none, the game's own: player 1's
    // controls work it), as the game would put it in its record
    void (*showFrame)(uint32_t frame) = nullptr;
    // The character's own objects (for its picture alone), and where it stands in the picture (across, and its feet;
    // -1: not known, found from its picture)
    bool (*characterObject)(uint32_t tile, uint16_t attributes) = nullptr;
    void (*standing)(int &x, int &y) = nullptr;
};

// The game's description, and the menu's pages, commands and features of the modes
void Register(const Description &description);

bool AttractMode();
int AttractShow(); // the "Shows" choice: 0 everything, then the description's shows from 1
bool ScoreAttack();
bool StageViewer();
int ViewedStage(); // (the stage the viewer is showing or getting to, from 0)
bool CharacterViewer();
int ViewedCharacter(); // (the character the viewer is showing, from 0)
// The stage or the character viewer: a game begun unseen at a stage of the viewer's (ViewedStage), with the players
// out of harm and the enemies standing still and unseen
inline bool Viewing() {
    return StageViewer() || CharacterViewer();
}
// A mode is getting the board to where it begins (the game's sources leave the board alone meanwhile: no running
// unseen of their own)
bool Starting();
// Ends the modes (another mode of the game's own starts)
void EndModes();

// The game's sources call these from their own (Game::beforeFrame, frame, state)
void BeforeFrame(Machine &machine);
void Frame(Machine &machine);
void State(Archive &ar);

// (model.cpp: the character viewer, run by modes.cpp)
void RegisterModel(const Description &description);
void StartModel(Machine &machine);
void ModelBeforeFrame(Machine &machine);
void ModelFrame(Machine &machine);
void EndModel();

} // namespace beat
} // namespace cps2recomp
