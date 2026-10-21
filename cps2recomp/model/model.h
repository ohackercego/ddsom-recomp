// SPDX-License-Identifier: GPL-3.0-only
// Copyright (C) 2026 Tommy McLeroy. Part of cps2recomp.
//
// The model viewer every game's sources can have (the fighting games' "Model viewer", the beat-'em-ups' "Character
// viewer"): one character alone, on one of the game's stages or on plain white or black, at a size from a quarter
// to twenty-five times its own, moved with player 1's controls or going through any one of its animations.
//
// A game gets there its own way (a round of a match reached unseen, a game begun at a stage) and calls these each
// frame once it is; what is the same in every game is here: the menu's page and choices, the animation shown (a
// frame of it put in the character's record each frame by the game's sources, with the game's own stepping kept off),
// stepped with left and right, repeated or played all in turn, and its name shown; and the picture, made in two
// parts: the frame's own is the stage with nothing on it (the game's sources hide the rest), and the character is
// drawn again by itself (the board's objects alone, and of them only its own) and laid over the stage or the plain
// colour at the size wanted, about the place where it stands. At its own size on a stage, what the stage has in
// front of the fighters can be put back in front of it.
//
// For the colour editor (fight/colours.h) it also finds which animation shows each of a character's palettes, on a
// copy of the viewer's machine (runtime/spare.h).
#pragma once

#include "game.h"
#include "machine.h"

#include <cstdint>
#include <vector>

namespace cps2recomp::model {

struct Description {
    // --- the menu ---
    const char *page = "Model viewer", *parent = "Alternative modes";
    const char *pageWords = "";
    const char *startWords = "Shows the character chosen below by itself. Player 1's controls move it while it stands. Reset ends it.";
    const char *resetWords = "";
    const char *animationWords = "";
    const char *foregroundWords = "";
    // The characters by name, in the menu's order, and their numbers in the game (none: the order)
    std::vector<const char *> characters;
    std::vector<int> numbers;
    // The backgrounds before "White" and "Black": the game's stages
    std::vector<const char *> stages;
    // Run by the page's "Start"
    void (*start)(Machine &machine) = nullptr;

    // --- the animations ---
    // The "Animation" choice: standing, then `lists` lists of up to `perList`, named by a letter and a number (A34)
    int lists = 3, perList = 256;
    // Where animation `index` of list `list` of a character (by its number) begins (0: it has none of that number)
    uint32_t (*animationAt)(Machine &machine, int character, int list, int index) = nullptr;
    // Whether an address is a frame (none: every address the game gives is)
    bool (*isFrame)(Machine &machine, uint32_t frame) = nullptr;
    // How many frames of the game a frame is shown for, and the frame after it (0: the animation ends there)
    int (*frameTime)(Machine &machine, uint32_t frame) = nullptr;
    uint32_t (*nextFrame)(Machine &machine, uint32_t frame) = nullptr;
    // A frame put in the character's record, as the game would, with the game's own stepping kept off (0: back to
    // standing, as the character was before an animation was shown)
    void (*showFrame)(Machine &machine, uint32_t frame) = nullptr;
    // The name of the animation choice the viewer follows (none: model_animation; the colour editor has its own)
    const char *(*animationChoice)() = nullptr;

    // --- the picture ---
    // The character's own objects; and the picture as the board would draw it in a fight (none: no stage in front)
    bool (*isModel)(uint32_t tile, uint16_t attributes) = nullptr;
    bool (*inAFight)(uint32_t tile, uint16_t attributes) = nullptr;
    // Before the character's picture is drawn (to look at what the filters need)
    void (*beforeCompose)(Machine &machine) = nullptr;
    // Where the character stands in the picture, as the frame after this one will show it (y -1: its feet found from
    // its picture as it first stands)
    void (*anchor)(Machine &machine, int &x, int &y) = nullptr;
    // The anchor is where it stands in this frame's picture, not the next (asked as the picture is made)
    bool anchorNow = false;
    // How far up from the feet the larger sizes are centred
    int largeLift = 56;
    // How far left the character is moved for a panel at the picture's right (the colour editor's; none: 0)
    int (*shift)() = nullptr;

    // --- the colour editor's lines (none: no scan) ---
    // How many palettes a character's set has; the first one's slot, from its record; characters the scan is not
    // for; each frame of the copy
    int linesInSet = 5;
    int (*firstLine)(Machine &machine) = nullptr;
    bool (*noScan)(int character) = nullptr;
    void (*spareFrame)(Machine &machine) = nullptr;
};

// The game's description, and the menu's page, its commands and choices
void Register(const Description &description);

// The choices as they are now (clamped), and the numbers in the game of the character chosen
int ChosenCharacter();
int ChosenBackground();
int Number(int choice);
bool Plain(int background); // white or black, not a stage
int White();
int Black();

// The viewer begins with the choices made now; it is on until End
void Begin();
void End();
bool On();
// The character and background it began with (choices), and the character's number in the game
int Character();
int Background();
int CharacterNumber();
// The choices made since differ from those it began with: the game begins it again
bool Changed();

// Each frame while viewing, before the game runs it: the animation shown (player 1's controls taken while one is),
// and the speed chosen
void BeforeFrame(Machine &machine);
// The colour editor's scan of which animation shows each line, a few frames of it on the copy (while it is on)
void Scan(Machine &machine);
// Each frame while viewing, after the game ran it: the picture made, the animation's name shown, and where the
// character stands for the next
void Frame(Machine &machine);

// For the colour editor: the animation after `choice` (by 1) or before it (by -1) that the character has, with
// standing (0) before the first and after the last; the animation that shows a line best (0: standing; -1: none;
// -2: not known yet); and the choices' names
int AnimationStep(Machine &machine, int choice, int by);
int AnimationForLine(int line);
const std::vector<const char *> &AnimationNames();

} // namespace cps2recomp::model
