// SPDX-License-Identifier: GPL-3.0-only
// Copyright (C) 2026 Tommy McLeroy
//
// Vampire Savior: the training session is the toolkit's (fight/session.h): getting there, player 1 choosing for both,
// start over, the stage, and the aids (a readout of the damage hits and combos do, a display of what the players press,
// frame data, hit boxes, health that comes back, and a second player that stands, crouches, jumps, guards, or does
// again what was recorded). This says where the game keeps what it needs; hits are told it where the game takes their
// damage off (fight.cpp), the players' steps where their routine dispatches (frames.cpp), and the stage and the
// characters a mode wants are put in where the game chooses them (below).
#include "boxes.h"
#include "fight.h"
#include "frames.h"
#include "machine.h"
#include "modes.h"
#include "session.h"
#include "state.h"
#include "widescreen.h"
#include "vsav.h"

#include <iterator>

namespace cps2recomp {

bool AttackInRange(M68K &c, uint32_t player); // fight.cpp
bool GuardStanding(M68K &c, uint32_t player);

namespace {
    using namespace vsav;

    constexpr int kPictureFloor = 239; // the picture's line that height 0 in the stage is on, with the camera at the bottom

    // The game is choosing the stage for a match (vsav.h, kChooseStage). In a training session with a stage
    // asked for, that one is used and the game's routine is left out.
    bool ChooseStage(M68K &c) {
        // The characters a mode wants, in place of the ones chosen (two alike: the second in its other colours),
        // in their hidden forms if asked for (which have two sets of colours)
        const MatchWanted &wanted = Wanted();
        for (int n = 0; n < 2; n++) {
            if (wanted.characters[n] >= 0) {
                const int character = HiddenForm(n, wanted.characters[n]);
                SetRam8(Player(n) + kCharacter, static_cast<uint8_t>(character));
                SetRam8(Player(n) + kCharacterAgain, static_cast<uint8_t>(character));
                family::CharacterPut(Player(n), character);
            }
        }
        for (int n = 0; n < 2; n++) {
            if (wanted.colours[n] >= 0) {
                const bool hidden = wanted.characters[n] >= 0 && HiddenForm(n, wanted.characters[n]) != wanted.characters[n];
                SetRam8(Player(n) + kColours, static_cast<uint8_t>(hidden ? wanted.colours[n] & 1 : wanted.colours[n]));
            }
        }
        if (wanted.characters[0] >= 0 && Ram8(Player(0) + kCharacter) == Ram8(Player(1) + kCharacter) &&
            Ram8(Player(0) + kColours) == Ram8(Player(1) + kColours)) {
            SetRam8(Player(1) + kColours, Ram8(Player(1) + kColours) ^ 1);
        }
        // (0: the game's choice)
        const int stage = session::WantedStage();
        if (stage < 1 || stage > kStages || Ram8(kPlayersThen) != 3) {
            return false;
        }
        SetRam16(kStage, static_cast<uint16_t>((stage - 1) * 2));
        Return(c);
        return true;
    }

    // What the game read off the panel for a player this frame (vsav.h, kControls), as Button bits
    uint16_t PanelHeld(int player) {
        const uint16_t word = Ram16(0xFF8000 + static_cast<uint32_t>(kControls[player]));
        uint16_t held = word & 0x0F; // (the stick's four bits are in the same order)
        held |= static_cast<uint16_t>(((word >> 8) & 0x07) << 4); // punches
        held |= static_cast<uint16_t>(((word >> 12) & 0x07) << 7); // kicks
        return held;
    }

    // The boxes the game tests (boxes.h)
    std::vector<fight::StageBox> Boxes(Machine &machine) {
        std::vector<fight::StageBox> boxes;
        for (const PlacedBox &box : AllBoxes(machine)) {
            const fight::BoxKind kind = box.kind == BoxKind::Attack ? fight::BoxKind::Attack :
                                        box.kind == BoxKind::Push   ? fight::BoxKind::Push :
                                                                      fight::BoxKind::Hurt;
            boxes.push_back({kind, box.x, box.y, box.halfWidth, box.halfHeight, box.thrown});
        }
        return boxes;
    }
} // namespace

// A hit's damage is being taken off a player's health (from the hook in fight.cpp)
void TrainingHit(int player, unsigned forGood, unsigned besides) {
    session::Hit(player, forGood + besides, besides);
}

// A hit has landed on a player, or been guarded (fight.cpp, HitLands)
void TrainingContact(int victim, bool guarded, uint8_t kind) {
    (void)kind;
    session::Contact(victim, guarded);
}

void TrainingBeforeFrame(Machine &machine) {
    session::BeforeFrame(machine);
}
void TrainingFrame(Machine &machine) {
    session::Frame(machine);
}
void TrainingState(Archive &ar) {
    session::State(ar);
}

// A training session is on, or the stage viewer: no clock and no knockouts (fight.cpp)
bool TrainingMode() {
    return session::NoKnockOut();
}

void RegisterTraining() {
    AddHook(At(0x00AE7C), "ChooseStage", ChooseStage);
    session::Description d;
    d.player1 = kPlayer1;
    d.playerSize = kPlayerSize;
    d.x = kX;
    d.xLong = true;
    d.health = kHealth;
    d.healthCeiling = kHealthCeiling;
    d.healthFull = kHealthFull;
    d.clock = kClock;
    d.clockFull = kClockFull;
    d.cameraX = kCameraX;
    d.cameraY = kCameraY;
    d.cameraCopies.assign(std::begin(kCameraCopies), std::end(kCameraCopies));
    d.startX[0] = kStartX[0];
    d.startX[1] = kStartX[1];
    d.stageLeft = kStageLeft;
    d.stageRight = kStageRight;
    d.cameraLeft = kCameraLeft;
    d.cameraRight = kCameraRight;
    d.cornerApart = 120;
    d.pictureFloor = kPictureFloor;
    d.chosen = [](int n) { return Ram8(Player(n) + kChoosingStep) >= kChosen; };
    d.panelHeld = PanelHeld;
    d.moveStart = FramesMoveStart;
    d.boxes = Boxes;
    d.guardView = [](Machine &machine, bool &inRange, bool &standing) {
        inRange = AttackInRange(machine.cpu, Player(1));
        standing = inRange && GuardStanding(machine.cpu, Player(1));
    };
    d.wideStage = WideStage;
    d.wideSide = WideSide;
    d.wideMargin = WideMargin;
    d.stageChoices = StageChoices({"The game's choice"});
    d.stageWords = "Where a training session's fight is: the game's own choice (at random, from those that go with player 2's "
                   "character) or a stage named here. Takes effect when the characters are next chosen.";
    d.framesPrintVariable = "VSAV_FRAMES";
    fight::DummyOptions &dummy = d.dummy;
    // While guarding: buttons pressed one after another, guarding still (pushing the attacker away); or a guard
    // cancel: towards, down, down and towards with a punch or a kick, again and again
    dummy.guardActions.push_back({"Pushes away", [](int frame, uint16_t, uint16_t) -> uint16_t {
                                      static constexpr uint16_t kMash[] = {kPunch1, 0, kPunch2, 0, kPunch3, 0, kKick1, 0, kKick2, 0, kKick3, 0};
                                      return kMash[frame % 12];
                                  },
                                  true});
    dummy.guardActions.push_back({"Guard cancel, punch", [](int frame, uint16_t towards, uint16_t) -> uint16_t {
                                      const int at = (frame / 2) % 4;
                                      return at == 0 ? towards : at == 1 ? kDown : at == 2 ? static_cast<uint16_t>(kDown | towards | kPunch2) : 0;
                                  }});
    dummy.guardActions.push_back({"Guard cancel, kick", [](int frame, uint16_t towards, uint16_t) -> uint16_t {
                                      const int at = (frame / 2) % 4;
                                      return at == 0 ? towards : at == 1 ? kDown : at == 2 ? static_cast<uint16_t>(kDown | towards | kKick2) : 0;
                                  }});
    dummy.guardActionsWords = "What player 2 does while guarding a hit: presses buttons to push the attacker away, or does a guard "
                              "cancel (towards, down, down and towards with a punch or a kick: most characters have one or the other).";
    // Thrown: the throw's own command softens it ("tech hit")
    dummy.heldAction = [](int frame, uint16_t, uint16_t away) -> uint16_t {
        return static_cast<uint16_t>(away | ((frame & 1) ? (kPunch2 | kPunch3) : 0));
    };
    dummy.heldActionTitle = "Softens throws";
    dummy.heldActionWords = "Player 2 does a throw's own command while being thrown (a \"tech hit\").";
    // Knocked down (reeling for more than 34 frames): the stick held with a button tapped rolls it along the floor
    dummy.getUp.push_back({"Towards", [](int frame, uint16_t towards, uint16_t) -> uint16_t {
                               return static_cast<uint16_t>(towards | ((frame & 2) ? kPunch1 : 0));
                           }});
    dummy.getUp.push_back({"Away", [](int frame, uint16_t, uint16_t away) -> uint16_t {
                               return static_cast<uint16_t>(away | ((frame & 2) ? kPunch1 : 0));
                           }});
    dummy.getUpWords = "Where player 2 gets up after being knocked down: where it fell, or rolled towards player 1 or away.";
    dummy.downAfter = 34;
    session::Register(d);
}

} // namespace cps2recomp
