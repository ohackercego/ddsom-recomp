// SPDX-License-Identifier: GPL-3.0-only
// Copyright (C) 2026 Tommy McLeroy
//
// Cadillacs and Dinosaurs: the board's settings (the Board page), as in the other games: each "As the board is set"
// unless chosen, and a choice is what the test menu would have set. The game reads its settings from three bytes laid
// out as the older CPS-1 boards' DIP switches (dino.h, kSwitches); the choices are put into them, on the board's own
// (kBoardSwitches), before every frame but while the test menu is shown, which shows and saves the board's own.
#include "beat.h"
#include "dino.h"

namespace cps2recomp {

namespace {
    using namespace dino;

    constexpr const char *kPage = "Board";
    struct Field {
        const char *name;
        int byte;  // 0-2: A, B, C
        int shift;
        uint8_t mask;
    };
    constexpr Field kFields[] = {
        {"board_difficulty", 1, 0, 0x07},  {"board_difficulty_2", 1, 3, 0x03}, {"board_extend", 1, 5, 0x03},
        {"board_lives", 2, 0, 0x03},       {"board_demo_sound", 2, 5, 0x01},   {"board_continue", 2, 6, 0x01},
    };
} // namespace

void BoardFrame(Machine &machine) {
    if (machine.testSwitch) {
        return;
    }
    uint8_t switches[3] = {Ram8(kBoardSwitches), Ram8(kBoardSwitches + 1), Ram8(kBoardSwitches + 2)};
    bool chosen = false;
    // (attract mode with its sound, and score attack with no continue: beat/beat.h)
    if (beat::AttractMode()) {
        switches[2] |= 1 << 5;
        chosen = true;
    } else if (beat::ScoreAttack()) {
        switches[2] &= static_cast<uint8_t>(~(1 << 6));
        chosen = true;
    }
    for (const Field &field : kFields) {
        const int choice = FeatureValue(field.name);
        if (choice > 0) {
            uint8_t &b = switches[field.byte];
            b = static_cast<uint8_t>((b & ~(field.mask << field.shift)) | (((choice - 1) & field.mask) << field.shift));
            chosen = true;
        }
    }
    if (chosen) {
        for (int n = 0; n < 3; n++) {
            SetRam8(kSwitches + static_cast<uint32_t>(n), switches[n]);
        }
    }
}

void RegisterBoard() {
    Game &game = TheGame();
    game.pages.push_back({kPage, "", "The board's own settings, as its test menu has them (System Configuration): each as the board is set "
                                     "unless chosen here."});
    constexpr const char *kAsSet = "As the board is set";
    game.features.push_back({"board_difficulty", kPage, "Difficulty", "How hard the game is. A new board: level 3, normal.", 0,
                             {kAsSet, "Level 0, extra easy", "Level 1, very easy", "Level 2, easy", "Level 3, normal", "Level 4, hard",
                              "Level 5, very hard", "Level 6, extra hard", "Level 7, hardest"}});
    game.features.push_back({"board_difficulty_2", kPage, "Difficulty 2", "The test menu's second difficulty. A new board: level 1, normal.", 0,
                             {kAsSet, "Level 0, easy", "Level 1, normal", "Level 2, hard", "Level 3, very hard"}});
    game.features.push_back({"board_extend", kPage, "Extend", "The scores that give a life. A new board: 500,000, then every 1,000,000.", 0,
                             {kAsSet, "None", "300,000, then every 700,000", "500,000, then every 1,000,000", "Only at 1,000,000"}});
    game.features.push_back({"board_lives", kPage, "Lives", "Lives for each credit (the test menu's number of players). A new board: 2.", 0,
                             {kAsSet, "1", "2", "3", "4"}});
    game.features.push_back({"board_continue", kPage, "Continue", "Whether a game over can be continued.", 0, {kAsSet, "Off", "On"}});
    game.features.push_back({"board_demo_sound", kPage, "Demo sound", "Whether the attract mode plays sound.", 0, {kAsSet, "Off", "On"}});
}

} // namespace cps2recomp
