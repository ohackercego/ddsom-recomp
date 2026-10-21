// SPDX-License-Identifier: GPL-3.0-only
// Copyright (C) 2026 Tommy McLeroy
//
// Vampire Savior: what the game's own sources add to the port (see PROGRESS.md).
#include "colours.h"
#include "game.h"
#include "modes.h"

#include <cstdlib>
#include <cstring>

namespace cps2recomp {

void RegisterBoard();    // board.cpp
void BoardBeforeFrame(Machine &machine);
void RegisterCheats();   // cheats.cpp
void CheatsFrame(Machine &machine);
void RegisterCollision(); // collision.cpp
void RegisterFight();     // fight.cpp
void RegisterInput();     // input.cpp
void RegisterChoosing();  // choosing.cpp
void RegisterCamera();    // camera.cpp
void RegisterPlanes();    // planes.cpp
void RegisterRows();      // rows.cpp
void RegisterPicture();   // picture.cpp
void RegisterEdges();     // edges.cpp
void RegisterPieces();    // pieces.cpp
void RegisterFrames();    // frames.cpp
void RegisterTraining(); // training.cpp
void TrainingBeforeFrame(Machine &machine);
void TrainingFrame(Machine &machine);
void TrainingState(Archive &ar);
void RegisterModes(); // modes.cpp
void ModesBeforeFrame(Machine &machine);
void ModesFrame(Machine &machine);
void ModesState(Archive &ar);
void RegisterModel(); // model.cpp
void ModelBeforeFrame(Machine &machine);
void ModelFrame(Machine &machine);
void ModelState(Archive &ar);
bool ModelViewer();
void RegisterMachines(); // machines.cpp
void RegisterBattle();   // battle.cpp
void BattleBeforeFrame(Machine &machine);
void BattleFrame(Machine &machine);
void BattleState(Archive &ar);
void RegisterTitle(); // title.cpp
void TitleFrame(Machine &machine);
void RegisterWidescreen(); // widescreen.cpp
void WidescreenBeforeFrame(Machine &machine);
void RegisterColours(); // colours.cpp (the toolkit's fight/colours.h)

void RegisterMusic(); // music.cpp

bool TrainingMode(); // training.cpp
bool AttractMode();  // modes.cpp
bool StageViewer();
bool BattleMode();   // battle.cpp
bool WidePlay();     // widescreen.cpp

void RegisterGame() {
    RegisterMusic();
    RegisterModes();
    RegisterBattle();
    RegisterModel();
    RegisterMachines();
    RegisterBoard();
    RegisterCheats();
    RegisterTraining();
    RegisterCollision();
    RegisterFight();
    RegisterInput();
    RegisterChoosing();
    RegisterCamera();
    RegisterPlanes();
    RegisterRows();
    RegisterPicture();
    RegisterEdges();
    RegisterPieces();
    RegisterFrames();
    RegisterWidescreen();
    RegisterTitle();
    // The training aids work only in a training session (the dummy's are the toolkit's: fight::RegisterDummy)
    TrainingAids({"damage_readout", "frame_data", "frame_log", "frame_meter", "health_refill", "hit_boxes", "input_display"});
    TheGame().inTraining = [] { return FeatureOn("training_mode"); };
    RegisterColours();
    TheGame().beforeFrame = [](Machine &machine) {
        // (for tests: VSAV_CHARACTERS=13,-1 makes player 1 Donovan at the character choice outside the modes, whatever
        // is chosen: a character's number in hex for each player, -1 the one chosen; so that two games of the family
        // can be run with the same characters and controls)
        static const char *characters = getenv("VSAV_CHARACTERS");
        if (characters && !AttractMode() && !StageViewer() && !BattleMode() && !ModelViewer()) {
            const char *comma = strchr(characters, ',');
            Wanted().characters[0] = static_cast<int32_t>(strtol(characters, nullptr, 16));
            Wanted().characters[1] = comma ? static_cast<int32_t>(strtol(comma + 1, nullptr, 16)) : -1;
        }
        BoardBeforeFrame(machine);
        colours::BeforeFrame(machine);
        // (first: the modes below may run other machines' frames, which are never wide)
        WidescreenBeforeFrame(machine);
        TrainingBeforeFrame(machine);
        ModesBeforeFrame(machine);
        BattleBeforeFrame(machine);
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
        for (const char *name : {"widescreen", "infinite_time", "no_damage", "no_damage_2", "one_hit", "hidden_characters"}) {
            if (FeatureOn(name)) {
                return true;
            }
        }
        return TrainingMode() || AttractMode() || StageViewer() || BattleMode() || ModelViewer() || WidePlay();
    };
    // Its own, never set for the other games by the collection's launcher: the records, and the animations' numbers
    OwnFeatures({"survival_best", "ringout_best", "model_animation", "colours_animation"});
    // (a game made from this one's program with sources of its own: family.h)
    vsav::family::RegisterOwnSources();
}

} // namespace cps2recomp
