// SPDX-License-Identifier: GPL-3.0-only
// Copyright (C) 2026 Tommy McLeroy
//
// Super Gem Fighter Mini Mix: the fighting modes of the menu's "Alternative modes" page (survival, the endless battle,
// the ring out) are the toolkit's (fight/battle.h); this says where the game keeps what they need.
#include "battle.h"
#include "alternative.h"
#include "pocket.h"

#include <iterator>

namespace cps2recomp {

namespace {
    using namespace pocket;

    constexpr uint32_t kSetReelAnimation = 0x010DFC; // D0: the character's animation of that number in its third list (the reelings)
    constexpr uint32_t kAnimate = 0x010EAC;          // the animation on by a step
    constexpr uint32_t kFlyAcross = 0x011AA6;        // a step through the air across: the speed by its pull, the place by the speed
    constexpr uint32_t kFlyUp = 0x011AB8;            //   and up
    constexpr uint8_t kReelLaunched = 12;            // thrown into the air by a knock-down (rising: found in Ken's, 1177DA)
    constexpr uint8_t kReelFalling = 8;              //   and coming down (1175D2)
    constexpr int32_t kPull = -0x8000;               // what is taken off the speed up each step (a jump's)
    constexpr int32_t kFlungAcross = 0xC000;         // knocked out: away over the end, three quarters of a pixel a step
    constexpr int32_t kFlungUp = 0x60000;            //   and up, six pixels a step at first
    constexpr int kGoneBelow = 400;                  // how far under the floor one falling stops (long out of the picture)
    // The part of a player's record that is how it moves and is drawn (its steps, its frame, its speeds, its boxes):
    // taken from its record on the old stage when it comes down onto the next, but for its place and its opponent
    constexpr uint32_t kMovingFrom = 0x04, kMovingTo = 0x100;
    constexpr uint32_t kOpponent = 0x34;
} // namespace

// The ring out's own movements (fight/battle.h, RingOutMove) at a player's step (A6): true when it has made the step,
// the game's own routine for the player not run (training.cpp's PlayerStep, at 00C642)
bool RingOutStep(M68K &c) {
    const uint32_t player = c.a[6] & 0xFFFFFF;
    const RingOut ringOut = RingOutMove(player);
    if (ringOut.move == RingOut::kFlungOut) {
        // Knocked over the end of the stage: a knock-down's throw into the air, up and away (no longer kept in)
        SetRam32(player + kState, 0x02020202); // (reeling, thrown into the air, rising: as the game's knock-down, 00E1A4 and 00E5C8)
        SetRam8(player + kAirborne, 0xFF);
        SetRam8(player + kHitStop, 0);
        SetRam8(player + kAttacking, 0);
        SetRam32(player + kSpeedX, static_cast<uint32_t>(ringOut.side * kFlungAcross));
        SetRam32(player + kSpeedY, static_cast<uint32_t>(kFlungUp));
        SetRam32(player + kPullX, 0);
        SetRam32(player + kPullY, static_cast<uint32_t>(kPull));
        c.d[0] = kReelLaunched;
        Call(c, kSetReelAnimation);
        return true;
    }
    if (ringOut.move == RingOut::kFalling) {
        // Through the air and the floor; coming down from the top, the knock-down's falling
        if (static_cast<int16_t>(Ram16(player + kY)) > static_cast<int16_t>(kGround) - kGoneBelow) {
            const bool rising = static_cast<int32_t>(Ram32(player + kSpeedY)) > 0;
            Call(c, kFlyAcross);
            Call(c, kFlyUp);
            if (rising && static_cast<int32_t>(Ram32(player + kSpeedY)) <= 0) {
                c.d[0] = kReelFalling;
                Call(c, kSetReelAnimation);
                return true;
            }
        }
        Call(c, kAnimate);
        return true;
    }
    if (ringOut.move == RingOut::kDropsIn) {
        // Coming down onto the next stage: as it was in its jump over the old stage, at its place here; the game's jump
        // goes on from there and lands it
        for (uint32_t at = kMovingFrom; at < kMovingTo; at++) {
            if ((at < kX || at >= kY + 4) && (at < kOpponent || at >= kOpponent + 2)) {
                SetRam8(player + at, ringOut.record[at]);
            }
        }
        SetRam32(player + kX, static_cast<uint32_t>(ringOut.x) << 16);
        SetRam32(player + kY, static_cast<uint32_t>(ringOut.y) << 16);
    }
    return false;
}

void RegisterBattle() {
    battle::Description d;
    d.result = kRoundResult;
    d.cameraX = kCameraX;
    d.cameraY = kCameraY;
    d.cameraLeft = kCameraLeft;
    d.cameraRight = kCameraRight;
    d.player1 = kPlayer1;
    d.playerSize = kPlayerSize;
    d.playerRecords = 2;
    d.thrown = kThrown;
    d.thrownSize = kThrownSize;
    d.thrownRecords = kThrownRecords;
    d.active = kActive;
    d.x = kX;
    d.y = kY;
    d.character = kCharacter;
    d.computer = kComputer;
    d.control = kControl;
    d.controlOwn = kControlOwn;
    d.halfWidth = kHalfWidth;
    d.health = kHealth;
    d.healthFull = kHealthFull;
    d.healthBack = 0x18; // (a sixth of it all)
    d.stages = kStages;
    d.stageToRound = 2300; // (about 2,100 from switching on, and a few)
    d.characterSpan = kCharacters;
    d.choosable = [](int) { return true; };
    d.rosterNumber = [](int choice) { return choice >= 1 && choice <= kCharacters ? choice - 1 : -1; };
    d.fighterChoices = {"Random"};
    d.fighterChoices.insert(d.fighterChoices.end(), std::begin(kCharacterNames), std::end(kCharacterNames));
    d.stageChoices = modes::StageChoices({"Shuffle", "Scroll"});
    d.inGame = [] { return Ram16(kMode) == 2; };
    d.choosing = modes::Choosing;
    d.toRound = modes::ToRound;
    d.playSound = modes::PlaySound;
    d.wanted = modes::Wanted;
    d.cameraAt = modes::CameraAt;
    d.endModes = modes::End;
    d.stageViewer = modes::StageViewer;
    d.wideSide = WideSide;
    d.wideMargin = WideMargin;
    d.notTheStage = NotTheStage;
    d.thePanel = ThePanel;
    d.notTheFighters = NotTheFighters;
    d.playerSets = [](int n) { return 0x1Fu << (Ram8(Player(n) + kColourSet) & 0x1F); };
    d.inAir = [](uint32_t player) { return Ram8(player + kAirborne) != 0; };
    d.atJumpTop = [](uint32_t player) { return Ram8(player + kAirborne) && static_cast<int32_t>(Ram32(player + kSpeedY)) <= 0; };
    battle::Register(d);
}

} // namespace cps2recomp
