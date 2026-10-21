// SPDX-License-Identifier: GPL-3.0-only
// Copyright (C) 2026 Tommy McLeroy
//
// Super Gem Fighter Mini Mix: the training session is the toolkit's (fight/session.h); this says where the game keeps
// what it needs, and tells it from the game's own code what the players do: each player's step of a fight (frame
// data), a hit landing, and the hit boxes as the game's hit tests read them.
#include "fight.h"
#include "alternative.h"
#include "pocket.h"
#include "session.h"

#include <cstdio>
#include <iterator>

namespace cps2recomp {

bool RingOutStep(M68K &c); // battle.cpp

namespace {
    using namespace pocket;

    int PlayerOf(uint32_t record) {
        record &= 0xFFFFFF;
        return record == Player(0) ? 0 : record == Player(1) ? 1 : -1;
    }

    // The player is free to do anything: in charge of itself, and standing, crouching, walking or in its guarding pose
    bool CanAct(uint32_t player) {
        const uint8_t action = Ram8(player + kAction);
        return Ram8(player + kControl) == kControlOwn &&
               (action == kActionStand || action == kActionCrouch || action == kActionWalk || action == kActionGuard);
    }

    // 00C642: a player's fight step, where it goes on to whatever is in charge of the player; once a step for each
    // player (seen as the player's own routine starts on this step: its record is as the last step left it, which is
    // what the picture showed and the hit tests tested)
    bool PlayerStep(M68K &c) {
        if (RingOutStep(c)) { // (the ring out's own movements: battle.cpp)
            Return(c);
            return true;
        }
        const uint32_t player = c.a[6] & 0xFFFFFF;
        const int n = PlayerOf(player);
        if (n < 0 || Ram8(player + kState) != 2) {
            return false;
        }
        fight::StepView view;
        const uint8_t control = Ram8(player + kControl);
        view.control = control == kControlOwn       ? fight::StepView::Own :
                       control == kControlReeling ? fight::StepView::Reeling :
                                                    fight::StepView::Held;
        view.frozen = Ram8(player + kHitStop) != 0;
        view.canAct = CanAct(player);
        view.attacking = Ram8(player + kAttacking) != 0;
        view.jumping = Ram8(player + kAction) == kActionJump;
        view.attackOut = c.Read(Ram32(player + kFrame) + kFrameAttack, 0) != 0;
        session::Step(n, view);
        return false;
    }

    // 0155F2: an attack has reached the other player (A6 the attacker, A1 the one reached), hit or guarded: guarded when
    // the one reached was in its guarding pose (a guarded hit takes nothing off its health: 015D6E)
    bool HitLands(M68K &c) {
        const int victim = PlayerOf(c.a[1]);
        if (victim >= 0) {
            session::Contact(victim, Ram8((c.a[1] & 0xFFFFFF) + kAction) == kActionGuard);
        }
        return false;
    }

    // What the game read off the panel for a player this frame, as Button bits
    uint16_t PanelHeld(int n) {
        const uint16_t word = Ram16(kControls[n]);
        return static_cast<uint16_t>((word & 0x0F) | (((word >> 8) & 0x07) << 4));
    }

    fight::MoveLog::Start MoveStart(int n) {
        const uint32_t player = Player(n);
        const uint8_t who = Ram8(player + kCharacter);
        fight::MoveLog::Start start;
        start.character = who < kCharacters ? kCharacterNames[who] : "?";
        start.input = fight::Notation(PanelHeld(n), Ram8(player + kFacesRight) == 0);
        start.air = Ram8(player + kAction) == kActionJump || Ram16(player + kY) != kGround;
        start.id = Ram32(player + kFrame);
        return start;
    }

    // A box of a player's, from its table (eight bytes a box, or 32 an attack, the box first), placed in the stage
    void AddBox(M68K &c, std::vector<fight::StageBox> &boxes, fight::BoxKind kind, uint32_t player, uint32_t box, bool thrown = false) {
        const int16_t offset = static_cast<int16_t>(c.Read(box + kBoxX, 1));
        const int x = static_cast<int16_t>(Ram16(player + kX)) + (Ram8(player + kFacesRight) ? -offset : offset);
        const int y = static_cast<int16_t>(Ram16(player + kY)) + static_cast<int16_t>(c.Read(box + kBoxY, 1));
        const int width = static_cast<int>(c.Read(box + kBoxWidth, 1)), height = static_cast<int>(c.Read(box + kBoxHeight, 1));
        if (width == 0 || height == 0) {
            return;
        }
        boxes.push_back({kind, x, y, width / 2, height / 2, thrown});
    }

    std::vector<fight::StageBox> Boxes(Machine &machine) {
        M68K &c = machine.cpu;
        std::vector<fight::StageBox> boxes;
        for (int n = 0; n < 2; n++) {
            const uint32_t player = Player(n);
            if (Ram8(player + kState) != 2) {
                continue;
            }
            if (const uint8_t push = Ram8(player + kPush)) {
                AddBox(c, boxes, fight::BoxKind::Push, player, Ram32(player + kPushTable) + push * kBoxSize);
            }
            if (const uint8_t hurt = Ram8(player + kHurtA)) {
                AddBox(c, boxes, fight::BoxKind::Hurt, player, Ram32(player + kHurtTableA) + hurt * kBoxSize);
            }
            if (const uint8_t hurt = Ram8(player + kHurtB)) {
                AddBox(c, boxes, fight::BoxKind::Hurt, player, Ram32(player + kHurtTableB) + hurt * kBoxSize);
            }
            if (const uint32_t attack = c.Read(Ram32(player + kFrame) + kFrameAttack, 0)) {
                AddBox(c, boxes, fight::BoxKind::Attack, player, Ram32(player + kAttackTable) + attack * kAttackSize);
            }
        }
        // What the players throw (the pool from kThrown, records of a player's layout: Ryu's fireball's frame names its
        // attack, 36, in its thrower's table, which the record keeps too)
        for (int n = 0; n < kThrownRecords; n++) {
            const uint32_t thrown = kThrown + static_cast<uint32_t>(n) * kThrownSize;
            const uint32_t frame = Ram32(thrown + kFrame);
            if (!Ram8(thrown + kActive) || frame < 0x100000 || frame >= 0x280000) {
                continue;
            }
            if (const uint32_t attack = c.Read(frame + kFrameAttack, 0)) {
                AddBox(c, boxes, fight::BoxKind::Attack, thrown, Ram32(thrown + kAttackTable) + attack * kAttackSize, true);
            }
        }
        return boxes;
    }
    // The game's own readouts and aids: the gauge kept full, and each player's stun against how much stuns it
    void Hud(Machine &, std::vector<HudLine> &lines, std::vector<HudBox> &) {
        for (int n = 0; n < 2; n++) {
            const uint32_t player = Player(n);
            if (FeatureOn("gauge_refill") && Ram8(player + kGaugeStocks) < kGaugeStocksFull) {
                SetRam8(player + kGaugeStocks, kGaugeStocksFull);
            }
            if (FeatureOn("stun_readout")) {
                char text[32];
                snprintf(text, sizeof text, "stun %d/%d", Ram8(player + kStun), Ram8(player + kStunMost));
                lines.push_back({n == 0 ? 0.02f : 0.98f, 0.12f, n == 0 ? -1 : 1, text});
            }
        }
    }
} // namespace

void RegisterTraining() {
    session::Description d;
    d.player1 = kPlayer1;
    d.playerSize = kPlayerSize;
    d.x = kX;
    d.health = kHealth;
    d.healthFull = kHealthFull;
    d.clock = kRoundClock;
    d.clockFull = 99;
    d.cameraX = kCameraX;
    d.cameraY = kCameraY;
    d.cameraCopies.assign(std::begin(kCameraCopies), std::end(kCameraCopies));
    d.startX[0] = kStartX[0];
    d.startX[1] = kStartX[1];
    d.stageLeft = kStageLeft;
    d.stageRight = kStageRight;
    d.cameraLeft = kCameraLeft;
    d.cameraRight = kCameraRight;
    d.chosen = [](int n) { return Ram8(Player(n) + kControl) >= kChosen; };
    d.panelHeld = PanelHeld;
    d.facingLeft = [](int n) { return Ram8(Player(n) + kFacesRight) == 0; };
    d.moveStart = MoveStart;
    d.boxes = Boxes;
    d.wideStage = WideStage;
    d.wideSide = WideSide;
    d.wideMargin = WideMargin;
    d.stageChoices = modes::StageChoices({"The game's choice"});
    d.hud = Hud;
    d.ownAids = {"gauge_refill", "stun_readout"};
    // Knocked down (reeling for longer than this): a button tapped gets up sooner where the game allows it
    d.dummy.downAfter = 34;
    Game &game = TheGame();
    game.features.push_back({"gauge_refill", modes::kTraining, "Gauge refills", "Both players' super gauge kept full (nine stocks)."});
    game.features.push_back({"stun_readout", modes::kTraining, "Stun readout",
                             "Under each player's bar: the stun built up, against how much stuns the player."});
    session::Register(d);
    AddHook(HookAt(kPlayerControl), "PlayerStep", PlayerStep);
    AddHook(HookAt(kHitLands), "HitLands", HitLands);
}

} // namespace cps2recomp
