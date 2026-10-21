// SPDX-License-Identifier: GPL-3.0-only
// Copyright (C) 2026 Tommy McLeroy
//
// Alien vs. Predator: what the game's own sources add to the port (see PROGRESS.md): free play, the board's
// settings, cheats, and where a game starts.
#include "avsp.h"

#include <iterator>

namespace cps2recomp {

void RegisterTitle(); // title.cpp
void TitleFrame(Machine &machine);
void RegisterMusic(); // music.cpp
void RegisterModes(); // modes.cpp
int ViewerScene(); // viewer.cpp
int ViewerCharacter();
void ViewerFrame(Machine &machine);
bool ChoicesLeftOut();
void ModesBeforeFrame(Machine &machine);
void ModesFrame(Machine &machine);
void ModesState(Archive &ar);

namespace {
    using namespace avsp;

    // --- The board's settings (the Board page), as in the other games: each "As the board is set" unless chosen,
    // and a choice is what the test menu would have set. They go into the variables 003C5E fills, when it has
    // filled them; the board's settings memory is not changed, nor what the test menu shows and saves.
    constexpr const char *kBoardPage = "Board";
    struct Setting {
        const char *name;
        uint32_t variable;
        const uint8_t *values; // the variable's value for each choice after "As the board is set"
    };
    constexpr uint8_t kPlayerModes[] = {1, 0, 2}; // three on one chute, three with a chute each, two
    constexpr uint8_t kDifficulties[] = {0, 1, 2, 3, 4, 5, 6, 7};
    constexpr uint8_t kExtends[] = {0, 1, 2, 3, 4, 5, 6, 7};
    constexpr uint8_t kLiveses[] = {4, 5, 6, 7, 0, 1, 2, 3};
    constexpr uint8_t kOffOn[] = {0, 1};
    constexpr Setting kSettings[] = {
        {"board_player_mode", kSetPlayerMode, kPlayerModes}, {"board_difficulty", kSetDifficulty, kDifficulties},
        {"board_extend", kSetExtend, kExtends},              {"board_lives", kSetLives, kLiveses},
        {"board_continue", kSetContinue, kOffOn},            {"board_demo_sound", kSetDemoSound, kOffOn},
    };
    uint8_t g_board[std::size(kSettings)], g_boardThree[2]; // (the board's own, to put back for its test menu)
    bool g_chosen = false;

    void PutChoices() {
        g_chosen = false;
        for (size_t n = 0; n < std::size(kSettings); n++) {
            g_board[n] = Ram8(kSettings[n].variable);
        }
        g_boardThree[0] = Ram8(kThreePlayers);
        g_boardThree[1] = Ram8(kThreeChutes);
        for (const Setting &setting : kSettings) {
            const int choice = FeatureValue(setting.name);
            if (choice > 0) {
                SetRam8(setting.variable, setting.values[choice - 1]);
                g_chosen = true;
            }
        }
        // (as 003C5E works them out from the player mode)
        const uint8_t mode = Ram8(kSetPlayerMode);
        SetRam8(kThreePlayers, mode != 2 ? 1 : 0); // (traced: the factory's 2 is two players)
        SetRam8(kThreeChutes, mode == 0 ? 1 : 0);
    }
    bool SettingsRead(M68K &) {
        PutChoices();
        return false;
    }

    void RegisterBoard() {
        Game &game = TheGame();
        game.pages.push_back({kBoardPage, "", "The board's own settings, as its test menu has them (Configuration): each as the board is "
                                              "set unless chosen here. Taken when the game starts or is reset."});
        constexpr const char *kAsSet = "As the board is set";
        // (Enhanced chooses the first after "As the board is set": three players, so that a third can join)
        game.features.push_back({"board_player_mode", kBoardPage, "Players",
                                 "How many can play at once, and whether they share one coin chute or have one each. A new board is "
                                 "set for two; Enhanced lets a third join.",
                                 kFeatureEnhanced, {kAsSet, "Three, one chute", "Three, a chute each", "Two"}});
        game.features.push_back({"board_difficulty", kBoardPage, "Difficulty", "How hard the game is: 1 (easy) to 8 (hard). A new board: 4.", 0,
                                 {kAsSet, "1 (easy)", "2", "3", "4", "5", "6", "7", "8 (hard)"}});
        game.features.push_back({"board_extend", kBoardPage, "Extend", "The scores that give a life. A new board: only at 100,000.", 0,
                                 {kAsSet, "None", "Only at 100,000", "Only at 400,000", "Only at 1,000,000", "100,000, then every 400,000",
                                  "400,000, then every 500,000", "800,000, then every 600,000", "1,200,000, then every 1,000,000"}});
        game.features.push_back({"board_lives", kBoardPage, "Lives", "Lives at the start and after a continue. A new board: 1 and 2.", 0,
                                 {kAsSet, "1, continue 2", "2, continue 3", "3, continue 4", "5, continue 6", "1, continue 1", "2, continue 2",
                                  "3, continue 3", "5, continue 5"}});
        game.features.push_back({"board_continue", kBoardPage, "Continue", "Whether a game over can be continued.", 0, {kAsSet, "Off", "On"}});
        game.features.push_back({"board_demo_sound", kBoardPage, "Demo sound", "Whether the attract mode plays sound.", 0, {kAsSet, "Off", "On"}});
        AddHook(0x000D20, "SettingsRead", SettingsRead); // (the board's start)
        AddHook(0x0B3C6A, "SettingsReadAgain", SettingsRead); // (the test menu left)
    }

    // --- Where a game starts: 0082DC has put the players who joined in, and set the scene after round 1's story
    // (008352: its rts)
    bool GameBegins(M68K &) {
        if (ViewerScene() >= 0) {
            SetRam16(kScene, static_cast<uint16_t>(ViewerScene() * 2));
            if (ViewerCharacter() >= 0) {
                SetRam8(Player(0) + kCharacter, static_cast<uint8_t>(ViewerCharacter()));
            }
            return false;
        }
        const int round = ChoicesLeftOut() ? 0 : FeatureValue("start_round");
        if (round > 0 && round < kRounds) {
            SetRam16(kScene, static_cast<uint16_t>(kRoundFirstScene[round] * 2));
        }
        return false;
    }

    void Frame(Machine &machine) {
        // The test menu shows and saves the variables: the board's own go back while its switch is held, and the
        // choices come back when it is left (0B3C6A)
        if (g_chosen && machine.testSwitch) {
            for (size_t n = 0; n < std::size(kSettings); n++) {
                SetRam8(kSettings[n].variable, g_board[n]);
            }
            SetRam8(kThreePlayers, g_boardThree[0]);
            SetRam8(kThreeChutes, g_boardThree[1]);
            g_chosen = false;
        }
        ModesFrame(machine);
        ViewerFrame(machine);
        if (ChoicesLeftOut()) {
            TitleFrame(machine);
            return; // (free play and the cheats are not the board's own game)
        }
        if (FeatureOn("free_play") && Ram8(kCredits) < 3) {
            SetRam8(kCredits, 3); // (three: every player can join)
        }
        for (int n = 0; n < kPlayers; n++) {
            const uint32_t p = Player(n);
            if (Ram8(p + kInPlay) != 1) {
                continue;
            }
            const uint8_t health = Ram8(p + kHealth), most = Ram8(p + kHealthMax);
            if (FeatureOn("infinite_health") && health != 0xFF && health != 0 && health < most) {
                SetRam8(p + kHealth, most);
            }
            if (FeatureOn("infinite_lives") && Ram8(p + kLives) < 2) {
                SetRam8(p + kLives, 2);
            }
        }
        if (FeatureOn("infinite_time")) {
            SetRam8(kClockFrames, 0x3C); // (the frame count never runs out, so the clock stands still)
        }
        TitleFrame(machine);
    }
} // namespace

void RegisterGame() {
    Game &game = TheGame();
    RegisterMusic();
    RegisterBoard();
    RegisterTitle();
    RegisterModes();
    game.features.push_back({"free_play", "Game", "Free play", "Start works without a coin.", kFeatureEnhanced});
    game.features.push_back({"start_round", "Game", "Start at round",
                             "A new game starts at the beginning of the round chosen (the board starts at round 1).", 0,
                             {"Round 1", "Round 2", "Round 3", "Round 4", "Round 5", "Round 6", "Round 7"}});
    game.features.push_back({"infinite_health", "Cheats", "Infinite health", "No player's health goes down."});
    game.features.push_back({"infinite_lives", "Cheats", "Infinite lives", "A player who goes down always has another life."});
    game.features.push_back({"infinite_time", "Cheats", "Infinite time", "The round's clock stands still."});
    AddHook(0x008352, "GameBegins", GameBegins);
    game.beforeFrame = ModesBeforeFrame;
    game.frame = Frame;
    game.state = ModesState;
    OwnFeatures({"start_round"});
}

} // namespace cps2recomp
