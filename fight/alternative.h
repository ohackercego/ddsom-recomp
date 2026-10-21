// SPDX-License-Identifier: GPL-3.0-only
// Copyright (C) 2026 Tommy McLeroy. Part of cps2recomp.
//
// The "Alternative modes" page at the head of a fighting game's in-game menu, for every game on the board that has
// one: the page itself and its training page (the session is fight/session.h), attract mode on a loop, and a viewer
// for the stages; and what the modes share: getting a machine to a round, sounds asked for, what is wanted of the
// machine that is running. Each mode is started by a command and ends when the board is reset, when another is
// started, or (attract mode) with Start.
//
// Attract mode is the game's own loop of notices, introduction, title, fights and rankings, with no coin in. "The
// intro only" and "the fights only" leave parts of the loop out by moving the game's own step on where a wanted part
// ends (the game's sources say where its parts are: Description::attractStep).
//
// The stage viewer is a round of a two-player game on the stage chosen, in which nobody is drawn but the stage: the
// players stand a fixed distance apart, are moved together, and the game's own camera follows them; the objects of
// the players and of the panel are left out of the picture (video.h, hideObject). The stage's pieces move and its
// music plays as in a fight. "Infinite" is every stage in turn, each scrolled from end to end and then out of the
// picture as the next comes in: two machines, the port's and a spare run beside it every frame (runtime/spare.h),
// the picture made of the two and the sound mixed.
#pragma once

#include "battle.h"
#include "game.h"
#include "machine.h"

#include <cstdint>
#include <vector>

namespace cps2recomp {
class Archive;
}

namespace cps2recomp::modes {

// The menu's pages (the session's page is the training page; its dummy's page under it)
constexpr const char *kModes = "Alternative modes", *kTraining = "Training mode", *kDummy = "Dummy";
constexpr const char *kAttract = "Attract mode", *kViewer = "Stage viewer";

struct Description {
    // --- the game's memory ---
    // The game's four step words (each a step of the one before, 4 bytes apart from `steps`): 2 in a game, then
    // these for the players choosing, a match, a round of it, and the round being fought
    uint32_t steps = 0xFF8000;
    uint16_t gameChoosing = 2, gameMatch = 4, matchRound = 4, roundFought = 2;
    // Bytes: credits, and who is in (bit 0 player 1, bit 1 player 2)
    uint32_t credits = 0, playersIn = 0;
    // The sound queue: a ring of sixteen-byte requests from `soundQueue`, the word at `soundQueueEnd` the place of
    // the next (bytes into the ring); a tune asked for off before another
    uint32_t soundQueue = 0, soundQueueEnd = 0;
    uint16_t tuneOff = 0xFF05;
    // The players' records, and where each stands across (a word, or a long with 16 bits of fraction)
    uint32_t player1 = 0xFF8400, playerSize = 0x400, x = 0x10;
    bool xLong = false;
    // The button that chooses a character, and whether both Starts are pressed at once on the way to a round (else
    // player 1's until it is in, then player 2's)
    uint16_t choose = kPunch1;
    bool startsTogether = false;

    // --- the stages ---
    // The stages by name (1 the first), each one's tune (0: none), and the other tunes offered after the stages'
    std::vector<const char *> stageNames;
    std::vector<uint16_t> stageTunes;
    std::vector<const char *> otherTuneNames;
    std::vector<uint16_t> otherTunes;
    // The camera's ends across (the picture's left edge in the stage), and how far apart the viewer's players stand
    // (the camera then keeps their middle in the picture's)
    int cameraLeft = 256, cameraRight = 640;
    int apart = 0xC0;

    // --- the picture ---
    // Objects that are not the stage's own (hidden in the viewer), and the scroll screens the viewer shows
    bool (*notTheStage)(uint32_t tile, uint16_t attributes) = nullptr;
    unsigned viewerLayers = 0xF;
    // The wide picture (none: no widescreen): columns it adds on each side, how far short of each end the camera
    // stops, and a spare's frame about to run drawn as wide as the port's (and the port's machine back)
    int (*wideSide)() = nullptr;
    int (*wideMargin)() = nullptr;
    void (*wideForSpare)(Machine &machine) = nullptr;
    void (*wideForPort)(Machine &machine) = nullptr;

    // --- attract mode ---
    // The game's loop has begun (the board's start-up is over: `frames` since the board was switched on), and it is
    // over (the board was reset, a coin put in or a game begun)
    bool (*attractBegun)(int frames) = nullptr;
    bool (*attractOver)() = nullptr;
    // Each frame of attract mode: the loop's parts left out for `show` (kShowIntro, kShowFights), with GoTo; true to
    // run unseen meanwhile
    bool (*attractStep)(int show) = nullptr;
    const char *attractWords = "The game's own show on a loop, with no coin in: the introduction, the title, fights and rankings.";
    const char *attractShowWords = "What attract mode plays: everything the board does, the introduction over and over, or one fight "
                                   "after another.";

    // --- around a spare's frame (runtime/spare.h) ---
    // What the game's sources keep of their own while a spare's frame runs, fresh for it, and put back (besides what is
    // wanted, the camera and the wide picture, which this keeps)
    void (*keep)() = nullptr;
    void (*fresh)(Machine &machine) = nullptr;
    void (*putBack)(Machine &machine) = nullptr;

    // --- training ---
    // The menu's "Start" on the training page (fight/session.h, StartSession)
    void (*startTraining)(Machine &machine) = nullptr;
    const char *modesWords = "Other ways to run the game: training, attract mode, viewers for the stages and the characters, "
                             "survival, endless battle, ring out.";
};
enum { kShowAll, kShowIntro, kShowFights };

// The game's description; the menu's pages, commands and features of the page, attract mode and the viewer; the
// spares' keeping of what is wanted
void Register(const Description &description);
const Description &Described();

// What the machine that is running is to fight, and where its camera is to be (fight/battle.h): the game's hooks
// where it chooses the stage and the characters, and its camera routines, ask
MatchWanted &Wanted();
CameraWanted &CameraAt();

// A round of a match is being fought
bool RoundFought();
// Players are choosing their characters, both in
bool Choosing();
// The controls of a machine on its way from switching on to a round of a two-player game: credits kept, both Starts
// until both players are in, then the characters taken as they come (player 1's left to its own controls when
// `choose`). True once the round is on.
bool ToRound(Machine &machine, int32_t &frames, bool choose = false);
// A sound asked for as the game asks: a tune's number, or the description's tuneOff before another tune
void PlaySound(uint16_t code);
// The game's step moved to another part of its loop, as the game does it: the first step word, the ones under it
// and its timers cleared
void GoTo(uint16_t part);
// The stage choices for a menu row: the stages' names, between `before` and `after`
std::vector<const char *> StageChoices(std::initializer_list<const char *> before, std::initializer_list<const char *> after = {});

// Whether a mode is on, the stage the viewer's machine that is running is to fight on (0: none), and ending them
bool AttractMode();
bool StageViewer();
int StageViewerStage();
void End();

// Around the port's machine's frame (the game's beforeFrame and frame), and its save states
void BeforeFrame(Machine &machine);
void Frame(Machine &machine);
void State(Archive &ar);

} // namespace cps2recomp::modes
