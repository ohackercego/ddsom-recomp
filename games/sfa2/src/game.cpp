// SPDX-License-Identifier: GPL-3.0-only
// Copyright (C) 2026 Tommy McLeroy
//
// Street Fighter Alpha 2: what the game's own sources add to the port (see PROGRESS.md).
#include "colours.h"
#include "game.h"
#include "modes.h"

namespace cps2recomp {

void RegisterBoard();  // board.cpp
void BoardBeforeFrame(Machine &machine);
void RegisterCheats(); // cheats.cpp
void CheatsFrame(Machine &machine);
void RegisterGuard();    // guard.cpp
void RegisterFight();    // fight.cpp
void RegisterTraining(); // training.cpp
void RegisterInput();    // input.cpp
void RegisterCollision(); // collision.cpp
void RegisterDamage();    // damage.cpp
void RegisterHit();       // hit.cpp
void RegisterThrows();    // throws.cpp
void RegisterPlayer();    // player.cpp
void TrainingBeforeFrame(Machine &machine);
void TrainingFrame(Machine &machine);
void TrainingState(Archive &ar);

void RegisterModes(); // modes.cpp
void ModesBeforeFrame(Machine &machine);
void ModesFrame(Machine &machine);
void ModesState(Archive &ar);
void RegisterBattle(); // battle.cpp
void BattleBeforeFrame(Machine &machine);
void BattleFrame(Machine &machine);
void BattleState(Archive &ar);

void RegisterModel(); // model.cpp
void ModelBeforeFrame(Machine &machine);
void ModelFrame(Machine &machine);
void ModelState(Archive &ar);
bool ModelViewer();
void RegisterColours(); // colours.cpp

void RegisterWidescreen(); // widescreen.cpp
void WidescreenBeforeFrame(Machine &machine);

void RegisterMachines(); // machines.cpp
void RegisterTitle();    // title.cpp
void TitleFrame(Machine &machine);

void RegisterMusic(); // music.cpp

bool TrainingMode(); // training.cpp
bool AttractMode();  // modes.cpp
bool StageViewer();
bool BattleMode();   // battle.cpp
bool WidePlay();     // widescreen.cpp

void RegisterGame() {
    RegisterMusic();
    RegisterMachines();
    RegisterModes();
    RegisterBattle();
    RegisterModel();
    RegisterBoard();
    RegisterColours();
    RegisterTraining();
    RegisterCheats();
    RegisterGuard();
    RegisterFight();
    RegisterInput();
    RegisterCollision();
    RegisterDamage();
    RegisterHit();
    RegisterThrows();
    RegisterPlayer();
    RegisterWidescreen();
    RegisterTitle();
    // The training aids work only in a training session (the dummy's are the toolkit's: fight::RegisterDummy)
    TrainingAids({"damage_readout", "stun_readout", "custom_combo_readout", "input_display", "hit_boxes", "throw_range", "frame_data",
                  "frame_log", "frame_meter", "health_refill", "fixed_damage", "dummy_stun_mash", "dummy_computer"});
    TheGame().inTraining = [] { return FeatureOn("training_mode"); };
    TheGame().beforeFrame = [](Machine &machine) {
        BoardBeforeFrame(machine);
        WidescreenBeforeFrame(machine);
        TrainingBeforeFrame(machine);
        ModesBeforeFrame(machine);
        BattleBeforeFrame(machine);
        colours::BeforeFrame(machine);
        ModelBeforeFrame(machine);
    };
    // (the training aids see what the frame did before the cheats put things back)
    TheGame().frame = [](Machine &machine) {
        TrainingFrame(machine);
        ModesFrame(machine);
        BattleFrame(machine);
        ModelFrame(machine);
        TitleFrame(machine);
        CheatsFrame(machine);
    };
    // (run-ahead: not in the modes, whose workings a save state does not wholly hold)
    TheGame().noRunAhead = [] { return AttractMode() || StageViewer() || BattleMode() || ModelViewer(); };
    TheGame().state = [](Archive &ar) {
        TrainingState(ar);
        ModesState(ar);
        BattleState(ar);
        ModelState(ar);
    };
    // With the arcade timing, the game's own code runs in place of the rewrites but where they carry what the
    // port adds (game.h, rewritesNeeded): training mode, the modes of the sources' own, widescreen, the cheats
    // that work inside them (the damage, the clock)
    TheGame().rewritesNeeded = [] {
        for (const char *name : {"widescreen", "infinite_time", "no_damage", "no_damage_2", "one_hit", "fixed_damage", "no_stun", "no_stun_2"}) {
            if (FeatureOn(name)) {
                return true;
            }
        }
        return TrainingMode() || AttractMode() || StageViewer() || BattleMode() || ModelViewer() || WidePlay();
    };
    // Its own, never set for the other games by the collection's launcher: the records, and the animations' numbers
    OwnFeatures({"survival_best", "ringout_best", "model_animation", "colours_animation"});
}

} // namespace cps2recomp
