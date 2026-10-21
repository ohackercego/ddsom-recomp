// SPDX-License-Identifier: GPL-3.0-only
// Copyright (C) 2026 Tommy McLeroy
//
// Super Puzzle Fighter II Turbo: what the game's own sources add to the port (see PROGRESS.md).
#include "game.h"

namespace cps2recomp {

void RegisterCoins(); // coins.cpp
void CoinsFrame(Machine &machine);
void RegisterField();  // field.cpp
void RegisterAttack(); // attack.cpp
void RegisterTurn();   // turn.cpp
void RegisterDrop();   // drop.cpp
void RegisterPair();   // pair.cpp
void TestColoursFrame();
void RegisterFall();   // fall.cpp
void RegisterSteer();  // steer.cpp
void RegisterBig();    // big.cpp
void RegisterRound();  // round.cpp
void RegisterBanner(); // banner.cpp
void RegisterMatch();  // match.cpp
void RegisterComputer(); // computer.cpp
void RegisterDifficulty(); // difficulty.cpp
void RegisterSelect(); // select.cpp
void RegisterScreens(); // screens.cpp
void RegisterCommon(); // common.cpp
void RegisterTraining(); // training.cpp
void TrainingBeforeFrame(Machine &machine);
void TrainingFrame(Machine &machine);
void TrainingState(Archive &ar);
void RegisterModes(); // modes.cpp
void ModesBeforeFrame(Machine &machine);
void ModesFrame(Machine &machine);
void ModesState(Archive &ar);
void RegisterColours(); // colours.cpp
void ColoursBeforeFrame(Machine &machine);
void ColoursFrame(Machine &machine);
void RegisterBoard(); // board.cpp
void BoardBeforeFrame(Machine &machine);
void RegisterTitle(); // title.cpp
void TitleFrame(Machine &machine);

void RegisterMusic(); // music.cpp

bool InAMode(); // modes.cpp

void RegisterGame() {
    RegisterMusic();
    RegisterCoins();
    RegisterBoard(); // (its page right after the Game page's first features)
    RegisterTraining();
    RegisterField();
    RegisterAttack();
    RegisterTurn();
    RegisterDrop();
    RegisterPair();
    RegisterFall();
    RegisterSteer();
    RegisterBig();
    RegisterRound();
    RegisterBanner();
    RegisterMatch();
    RegisterComputer();
    RegisterDifficulty();
    RegisterSelect();
    RegisterScreens();
    RegisterCommon();
    RegisterModes();
    RegisterColours();
    RegisterTitle();
    // The training aids work only in a training session
    TrainingAids({"training_crash", "training_speed", "input_display", "attack_readout", "dummy", "dummy_attack"});
    TheGame().inTraining = [] { return FeatureOn("training_mode"); };
    TheGame().beforeFrame = [](Machine &machine) {
        BoardBeforeFrame(machine);
        ColoursBeforeFrame(machine);
        TrainingBeforeFrame(machine);
        ModesBeforeFrame(machine);
    };
    TheGame().frame = [](Machine &machine) {
        TrainingFrame(machine);
        ModesFrame(machine);
        ColoursFrame(machine);
        CoinsFrame(machine);
        TestColoursFrame();
        TitleFrame(machine);
    };
    TheGame().state = [](Archive &ar) {
        TrainingState(ar);
        ModesState(ar);
    };
    // With the arcade timing, the game's own code runs in place of the rewrites but where they carry what the
    // port adds (game.h, rewritesNeeded): the modes of the sources' own, training mode, and the features that change
    // how it plays
    TheGame().rewritesNeeded = [] {
        for (const char *name : {"training_mode", "training_crash", "training_speed", "input_display", "attack_readout",
                                 "landing_guide", "dummy", "dummy_attack", "no_attack", "no_attack_2", "challenger",
                                 "quick_win", "hard_drop", "hidden_characters", "watch_1", "watch_2"}) {
            if (FeatureOn(name)) {
                return true;
            }
        }
        return InAMode();
    };
}

} // namespace cps2recomp
