// SPDX-License-Identifier: GPL-3.0-only
// Copyright (C) 2026 Tommy McLeroy
//
// Cadillacs and Dinosaurs: what the game's own sources add to the port (see PROGRESS.md): free play, cheats, the
// stage a game starts at, and straight to the title.
#include "dino.h"

namespace cps2recomp {

void RegisterBoard(); // board.cpp
void RegisterMusic(); // music.cpp
int ViewerEpisode(); // viewer.cpp
int ViewerCharacter();
void ViewerFrame(Machine &machine);
void RegisterModes(); // modes.cpp
bool ChoicesLeftOut();
void ModesBeforeFrame(Machine &machine);
void ModesFrame(Machine &machine);
void ModesState(Archive &ar);
void BoardFrame(Machine &machine);

namespace {
    using namespace dino;

    // 00A9F2: a game begins, its variables just cleared (the stage among them)
    bool GameBegins(M68K &) {
        if (ViewerEpisode() >= 0) {
            SetRam8(kStage, static_cast<uint8_t>(ViewerEpisode()));
            if (ViewerCharacter() >= 0) {
                SetRam8(Player(0) + kCharacter, static_cast<uint8_t>(ViewerCharacter()));
            }
            return false;
        }
        const int stage = ChoicesLeftOut() ? 0 : FeatureValue("start_stage");
        if (stage > 0 && stage < kStages) {
            SetRam8(kStage, static_cast<uint8_t>(stage));
        }
        return false;
    }

    // "Straight to the title": the board's start, its version and its warning, run unseen
    bool g_skipping = false, g_fast = false;
    int g_frames = 0;
    void TitleFrame(Machine &machine) {
        // (a mode of the menu started: its start is its own, and so is running unseen)
        if (ChoicesLeftOut()) {
            g_skipping = g_fast = false;
            return;
        }
        if (machine.frame <= 1 && !g_skipping) {
            g_skipping = FeatureOn("skip_intro");
            g_frames = 0;
        }
        if (g_skipping && (Ram8(kShow) >= kShowTitle || ++g_frames > 900)) {
            g_skipping = false;
        }
        if (g_skipping != g_fast) {
            g_fast = g_skipping;
            SetFastForward(g_fast);
        }
    }

    void Frame(Machine &machine) {
        BoardFrame(machine);
        ModesFrame(machine);
        ViewerFrame(machine);
        if (ChoicesLeftOut()) {
            TitleFrame(machine);
            return; // (free play and the cheats are not the board's own game)
        }
        if (FeatureOn("free_play")) {
            for (int n = 0; n < 3; n++) {
                const uint32_t credits = kChute + kChuteSize * static_cast<uint32_t>(n) + kChuteCredits;
                if (Ram8(credits) < 2) {
                    SetRam8(credits, 2);
                }
            }
        }
        for (int n = 0; n < kPlayers; n++) {
            const uint32_t p = Player(n);
            if (Ram8(p + kInPlay) != 1) {
                continue;
            }
            const uint8_t health = Ram8(p + kHealth);
            if (FeatureOn("infinite_health") && health != 0 && health < kHealthFull) {
                SetRam8(p + kHealth, kHealthFull);
            }
            if (FeatureOn("infinite_lives") && Ram8(p + kLives) < 2) {
                SetRam8(p + kLives, 2);
            }
        }
        if (FeatureOn("infinite_time")) {
            SetRam8(kClockFrames, 0x32); // (the frame count never runs out, so the clock stands still)
        }
        TitleFrame(machine);
    }
} // namespace

void RegisterGame() {
    Game &game = TheGame();
    RegisterMusic();
    RegisterBoard();
    RegisterModes();
    game.features.push_back({"free_play", "Game", "Free play", "Start works without a coin.", kFeatureEnhanced});
    game.features.push_back({"skip_intro", "Game", "Straight to the title",
                             "When the game starts, and when it is reset, goes straight to the title: the board's start, its version "
                             "and its warning are run unseen.",
                             0});
    game.features.push_back({"start_stage", "Game", "Start at stage",
                             "A new game starts at the beginning of the stage chosen (the board starts at stage 1).", 0,
                             {"Stage 1", "Stage 2", "Stage 3", "Stage 4", "Stage 5", "Stage 6", "Stage 7", "Stage 8"}});
    game.features.push_back({"infinite_health", "Cheats", "Infinite health", "No player's health goes down."});
    game.features.push_back({"infinite_lives", "Cheats", "Infinite lives", "A player who goes down always has another life."});
    game.features.push_back({"infinite_time", "Cheats", "Infinite time", "The stage's clock stands still."});
    AddHook(0x00A9F2, "GameBegins", GameBegins);
    game.beforeFrame = ModesBeforeFrame;
    game.frame = Frame;
    game.state = ModesState;
    OwnFeatures({"start_stage"});
}

} // namespace cps2recomp
