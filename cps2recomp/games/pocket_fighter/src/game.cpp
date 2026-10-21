// SPDX-License-Identifier: GPL-3.0-only
// Copyright (C) 2026 Tommy McLeroy
//
// Super Gem Fighter Mini Mix: what the game's own sources add to the port (see PROGRESS.md).
#include "game.h"
#include "battle.h"
#include "colours.h"
#include "pocket.h"
#include "alternative.h"
#include "session.h"

namespace cps2recomp {

void RegisterCoins(); // coins.cpp
void CoinsFrame(Machine &machine);
void RegisterCheats(); // cheats.cpp
void RegisterTitle(); // title.cpp
void TitleFrame(Machine &machine);
void RegisterBoard(); // board.cpp
void BoardBeforeFrame(Machine &machine);
void BoardState(Archive &ar);
void RegisterMusic(); // music.cpp
void RegisterModes(); // modes.cpp
void RegisterTraining(); // training.cpp
void RegisterBattle(); // battle.cpp
void RegisterModel(); // model.cpp
void RegisterColours(); // colours.cpp
void RegisterChoosing(); // choosing.cpp
void RegisterWidescreen(); // widescreen.cpp
void WidescreenBeforeFrame(Machine &machine);
bool WidePlay();
void ChoosingBeforeFrame(Machine &machine);
void ModelBeforeFrame(Machine &machine);
void ModelFrame(Machine &machine);
void ModelState(Archive &ar);
bool ModelViewer();

void RegisterGame() {
    RegisterModes();
    RegisterTraining();
    RegisterBattle();
    RegisterModel();
    RegisterColours();
    RegisterChoosing();
    RegisterWidescreen();
    RegisterCoins();
    RegisterCheats();
    RegisterTitle();
    RegisterBoard();
    RegisterMusic();
    TheGame().beforeFrame = [](Machine &machine) {
        BoardBeforeFrame(machine);
        ChoosingBeforeFrame(machine);
        WidescreenBeforeFrame(machine);
        session::BeforeFrame(machine);
        modes::BeforeFrame(machine);
        BattleBeforeFrame(machine);
        colours::BeforeFrame(machine);
        ModelBeforeFrame(machine);
    };
    // (the training aids see what the frame did before the cheats put things back)
    TheGame().frame = [](Machine &machine) {
        session::Frame(machine);
        modes::Frame(machine);
        BattleFrame(machine);
        ModelFrame(machine);
        TitleFrame(machine);
        CoinsFrame(machine);
    };
    // (run-ahead: not in the modes, whose workings a save state does not wholly hold)
    TheGame().noRunAhead = [] { return modes::AttractMode() || modes::StageViewer() || BattleMode() || ModelViewer(); };
    // (what the sources keep between frames, with save states and run-ahead's frames taken back)
    TheGame().state = [](Archive &ar) {
        BoardState(ar);
        session::State(ar);
        modes::State(ar);
        BattleState(ar);
        ModelState(ar);
    };
    // With the arcade timing, the game's own code runs in place of the rewrites but where they carry what the port adds
    // (game.h, rewritesNeeded): the modes, widescreen
    TheGame().rewritesNeeded = [] {
        return session::InSession() || modes::AttractMode() || modes::StageViewer() || BattleMode() || ModelViewer() || WidePlay() ||
               FeatureOn("widescreen");
    };
    // Its own, never set for the other games by the collection's launcher: Damage level has Super Puzzle Fighter II
    // Turbo's choices (Low, Normal, High, Maximum), where it means the counter gems an attack sends
    OwnFeatures({"board_damage", "survival_best", "ringout_best", "model_animation", "colours_animation"});
}

} // namespace cps2recomp
