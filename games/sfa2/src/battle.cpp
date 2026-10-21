// SPDX-License-Identifier: GPL-3.0-only
// Copyright (C) 2026 Tommy McLeroy
//
// Street Fighter Alpha 2: the fighting modes of the menu's "Alternative modes" page (survival, the endless battle,
// the ring out) are the toolkit's (fight/battle.h); this says where the game keeps what they need.
#include "battle.h"
#include "modes.h"

#include <iterator>

namespace cps2recomp {

namespace {
    using namespace sfa2;

    // The characters fought with and against: the eighteen (the other forms of six of them are left to the
    // menu's own choices)
    constexpr int kFighters = kStages;
    constexpr uint32_t kCameraX = 0xFF8290, kCameraY = 0xFF8294;
    constexpr uint32_t kHalfWidth = 0x12A, kPlayersIn = 0xFF80A8, kSpeedUp = 0x44;
    constexpr int kPlayerRecords = 4;

    // The panel and the stage's pieces: everything but the fighters
    bool NotTheFighters(uint32_t tile, uint16_t attributes) {
        const unsigned set = attributes & 0x1F;
        return ThePanel(tile, attributes) || set >= 27 || set < 11;
    }
} // namespace

void RegisterBattle() {
    battle::Description d;
    d.result = 0xFF810C;
    d.cameraX = kCameraX;
    d.cameraY = kCameraY;
    d.player1 = kPlayer1;
    d.playerSize = kPlayerSize;
    d.playerRecords = kPlayerRecords;
    d.thrown = kThrown;
    d.thrownSize = kThrownSize;
    d.thrownRecords = kThrownRecords;
    d.active = kActive;
    d.x = kX;
    d.y = kY;
    d.character = kCharacter;
    d.colours = kColourChoice;
    d.random = 0xFF80AE; // (01975A)
    d.computer = kComputer;
    d.control = kControl;
    d.controlOwn = kControlOwn;
    d.halfWidth = kHalfWidth;
    d.health = kHealth;
    d.healthCeiling = kHealthShown;
    d.healthFull = kHealthFull;
    d.healthBack = 0x18; // (a sixth of it all)
    d.ceilingShown = true;
    d.stages = kStages;
    d.stageToRound = 2000; // (about 1,900, and a few)
    d.characterSpan = kFighters;
    d.choosable = [](int) { return true; };
    d.rosterNumber = [](int choice) { return choice >= 1 && choice <= kFighters ? choice - 1 : -1; };
    d.fighterChoices = {"Random"};
    d.fighterChoices.insert(d.fighterChoices.end(), std::begin(kCharacterNames), std::begin(kCharacterNames) + kFighters);
    d.stageChoices = {"Shuffle", "Scroll"};
    d.stageChoices.insert(d.stageChoices.end(), std::begin(kStageNames), std::end(kStageNames));
    d.tuneOff = kTuneOff;
    d.inGame = [] { return Ram16(kMode) == 2; };
    d.choosing = [] { return Ram16(kMode) == 2 && Ram16(kGameStep) == kGameChoosing && Ram8(kPlayersIn) == 3; };
    d.toRound = [](Machine &machine, int32_t &frames, bool choose) { return ToRound(machine, frames, choose); };
    d.playSound = PlaySound;
    d.wanted = Wanted;
    d.cameraAt = CameraAt;
    d.endModes = EndModes;
    d.stageViewer = StageViewer;
    d.wideSide = WideSide;
    d.wideMargin = WideMargin;
    d.notTheStage = NotTheStage;
    d.thePanel = ThePanel;
    d.notTheFighters = NotTheFighters;
    // (the players' own objects are in their colour sets: +0D, player 1's from 16, player 2's from 11, five each)
    d.playerSets = [](int n) { return 0x1Fu << (Ram8(Player(n) + kColourSet) & 0x1F); };
    d.inAir = [](uint32_t player) { return Ram8(player + kInAir) != 0; };
    d.atJumpTop = [](uint32_t player) { return Ram8(player + kInAir) && static_cast<int32_t>(Ram32(player + kSpeedUp)) <= 0; };
    battle::Register(d);
}

} // namespace cps2recomp
