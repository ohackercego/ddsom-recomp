// SPDX-License-Identifier: GPL-3.0-only
// Copyright (C) 2026 Tommy McLeroy. Part of cps2recomp.
//
// A fighting game's training session (session.h), made from Vampire Savior's, which every fighting game's was a copy
// of but for where the game keeps things.
#include "session.h"

#include "battle.h"
#include "alternative.h"
#include "state.h"

#include <algorithm>
#include <cstdlib>

namespace cps2recomp::session {

namespace {
    Description d;

    constexpr int kRefillAfter = 90; // frames without a hit after which health comes back
    constexpr uint16_t kSticks = kLeft | kRight | kUp | kDown;
    constexpr uint16_t kAllButtons = kPunch1 | kPunch2 | kPunch3 | kKick1 | kKick2 | kKick3;

    fight::Frames g_frames;
    fight::MoveLog g_log;
    fight::InputHistory g_inputs;
    fight::Combos g_combos;
    fight::Dummy g_dummy;
    bool g_fighting = false;
    // Getting to a session from switching on: credits, then both Start buttons, until both players are choosing their
    // characters (the game runs unseen meanwhile)
    bool g_boot = false;
    int g_bootFrames = 0;
    constexpr int kBootMost = 4000; // frames after which it gives up
    // The machine as it was when the round's fighting began, for starting over
    std::vector<uint8_t> g_roundStart;
    bool g_roundFought = false;
    std::vector<HudBox> g_nextBoxes; // the hit boxes for the next picture
    bool g_choosingTwo = false;      // player 1's controls are choosing player 2's character

    uint32_t Player(int n) {
        return d.player1 + static_cast<uint32_t>(n) * d.playerSize;
    }
    int X(int n) {
        return d.xLong ? static_cast<int>(Ram32(Player(n) + d.x) >> 16) : static_cast<int16_t>(Ram16(Player(n) + d.x));
    }
    void SetX(int n, int x) {
        if (d.xLong) {
            SetRam32(Player(n) + d.x, static_cast<uint32_t>(x) << 16);
        } else {
            SetRam16(Player(n) + d.x, static_cast<uint16_t>(x));
        }
    }

    // Starts the aids afresh (a round starting, or started over), but for what the dummy was told to do
    void Afresh(bool fighting) {
        g_fighting = fighting;
        g_inputs.Clear();
        g_combos.Clear();
        g_dummy.Afresh();
        g_frames.Reset();
    }

    bool Fighting() {
        if (d.fighting) {
            return d.fighting();
        }
        for (int n = 0; n < 2; n++) {
            const uint32_t health = d.healthCeiling ? d.healthCeiling : d.health;
            if (static_cast<int16_t>(Ram16(Player(n) + health)) > static_cast<int16_t>(d.healthFull)) {
                return false;
            }
        }
        const uint8_t clock = Ram8(d.clock);
        return clock > 0 && clock <= d.clockFull;
    }

    // What the controls do on the way there: coins while there are fewer than two credits (and no free play), then
    // both Start buttons
    void Boot(Machine &machine) {
        const modes::Description &m = modes::Described();
        uint16_t &one = machine.players[0], &two = machine.players[1];
        const bool press = g_bootFrames % 16 < 6;
        one = two = 0;
        if (Ram8(m.playersIn) == 3 || ++g_bootFrames > kBootMost) {
            g_boot = false;
            SetFastForward(false);
        } else if (Ram8(m.credits) < 2 && !FeatureOn("free_play")) {
            one = press ? kCoin : 0;
        } else if (m.startsTogether) {
            one = two = press ? kStart : 0;
        } else if (!(Ram8(m.playersIn) & 1)) {
            one = press ? kStart : 0;
        } else {
            two = press ? kStart : 0;
        }
    }

    // "Start over": the round as it began, with the players put where the player wants them. The stick held as it is
    // asked for chooses the place (and it is remembered): left or right for a corner, down for the sides swapped, up
    // for the middle again.
    void StartOver(Machine &machine) {
        if (g_roundStart.empty() || !FeatureOn("training_mode")) {
            return;
        }
        int place = FeatureValue("restart_at"); // 0 middle, 1 left corner, 2 right corner; +3: sides swapped
        const uint16_t stick = machine.players[0] & kSticks;
        if (stick) {
            int where = place % 3;
            bool swapped = place >= 3;
            if (stick & kUp) {
                where = 0;
                swapped = false;
            }
            if (stick & kLeft) {
                where = 1;
            } else if (stick & kRight) {
                where = 2;
            }
            if (stick & kDown) {
                swapped = !swapped;
            }
            place = where + (swapped ? 3 : 0);
            SetFeatureValue("restart_at", place);
        }
        Archive ar(g_roundStart, false);
        machine.Serialize(ar);
        const int margin = d.wideMargin ? d.wideMargin() : 0; // (the camera's end in the picture played in)
        int x[2] = {d.startX[0], d.startX[1]};
        int camera = static_cast<int16_t>(Ram16(d.cameraX)); // (the middle: where it is)
        if (place % 3 == 1) {
            x[0] = d.stageLeft;
            x[1] = d.stageLeft + d.cornerApart;
            camera = d.cameraLeft + margin;
        } else if (place % 3 == 2) {
            x[0] = d.stageRight - d.cornerApart;
            x[1] = d.stageRight;
            camera = d.cameraRight - margin;
        }
        if (place >= 3) {
            std::swap(x[0], x[1]);
        }
        for (int n = 0; n < 2; n++) {
            SetX(n, x[n]);
        }
        for (const uint32_t copy : d.cameraCopies) {
            SetRam16(copy, static_cast<uint16_t>(camera));
        }
        // (and the training aids start afresh; a recording being replayed from its beginning)
        Afresh(true);
    }

    void Dummy(Machine &machine) {
        fight::DummyView view;
        view.fighting = Fighting() && (!d.dummyAfterAfresh || g_fighting);
        view.doing = g_frames.Now(1);
        view.onRight = X(1) >= X(0);
        view.sinceHit = g_combos.Since(1);
        if (view.fighting && FeatureValue("dummy_guard")) {
            if (d.guardView) {
                d.guardView(machine, view.attackInRange, view.guardStanding);
            } else {
                const fight::Doing one = g_frames.Now(0);
                view.attackInRange = one == fight::Doing::Startup || one == fight::Doing::Active;
            }
        }
        if (d.knockedDown) {
            view.knowsDown = true;
            view.down = d.knockedDown();
        }
        if (d.dummyView) {
            d.dummyView(machine, view);
        }
        g_dummy.BeforeFrame(machine.players[0], machine.players[1], view);
        if (d.afterDummy) {
            d.afterDummy(machine, view);
        }
    }
} // namespace

void StartSession(Machine &machine) {
    modes::End();
    EndBattle();
    SetFeature("training_mode", true);
    machine.Reset();
    g_boot = true;
    g_bootFrames = 0;
    SetFastForward(true);
}

bool InSession() {
    return FeatureOn("training_mode");
}

int WantedStage() {
    const int wanted = modes::Wanted().stage;
    if (wanted) {
        return wanted;
    }
    return InSession() ? FeatureValue("training_stage") : 0;
}

bool NoClock() {
    return InSession() || modes::StageViewer() || BattleNoClock();
}
bool NoKnockOut() {
    return InSession() || modes::StageViewer() || BattleNoKnockOut();
}

void Step(int player, const fight::StepView &view) {
    g_frames.Step(player, view);
    if (d.moveStart && FeatureOn("frame_log")) {
        g_log.Step(player, g_frames, d.moveStart);
    }
}
void Contact(int victim, bool guarded) {
    g_frames.Contact(victim, guarded);
}
void Hit(int player, unsigned damage, unsigned comesBack, bool chip) {
    // A combo goes on while the player has not got free since the last hit
    const fight::Doing doing = g_frames.Now(player);
    g_combos.Hit(player, damage, comesBack, !chip && (doing == fight::Doing::HitStun || doing == fight::Doing::Held));
}
fight::Doing Now(int player) {
    return g_frames.Now(player);
}

void BeforeFrame(Machine &machine) {
    if (g_boot) {
        Boot(machine);
        return;
    }
    if (InSession() && modes::Choosing() && d.chosen && d.chosen(0) && !d.chosen(1)) {
        // Player 1 has chosen: its controls choose for player 2 (training with one set of controls). What was held as
        // it chose is not passed on until let go, or the button that chose for player 1 would choose for player 2 at
        // once.
        static uint16_t heldOver = 0;
        const uint16_t held = machine.players[0] & (kSticks | kAllButtons);
        heldOver = g_choosingTwo ? heldOver & held : held;
        g_choosingTwo = true;
        machine.players[1] |= held & ~heldOver;
        machine.players[0] &= kStart | kCoin;
        return;
    }
    g_choosingTwo = false;
    Dummy(machine);
}

void Frame(Machine &machine) {
    if (modes::StageViewer() || BattleMode()) {
        // (nothing of training's is shown, or kept)
        g_fighting = false;
        g_roundFought = false;
        return;
    }
    // The round's fighting begins: kept, for starting over
    const bool fought = modes::RoundFought();
    if (fought && !g_roundFought && InSession()) {
        g_roundStart.clear();
        Archive ar(g_roundStart, true);
        machine.Serialize(ar);
    }
    g_roundFought = fought;
    const bool fighting = Fighting();
    if (!fighting || !g_fighting) {
        // (a round starting: start afresh)
        Afresh(fighting);
        SetHud({});
        SetBoxes({});
        g_nextBoxes.clear();
        return;
    }
    g_combos.Frame();
    std::vector<HudLine> lines;
    std::vector<HudBox> boxes;
    const bool readout = FeatureOn("damage_readout"), refill = FeatureOn("health_refill"), inputs = FeatureOn("input_display");
    for (int n = 0; n < 2; n++) {
        const uint16_t health = Ram16(Player(n) + d.health);
        if (refill && g_combos.Since(n) > kRefillAfter && health < d.healthFull && (!d.mayRefill || d.mayRefill(n))) {
            SetRam16(Player(n) + d.health, d.healthFull);
            if (d.healthCeiling) {
                SetRam16(Player(n) + d.healthCeiling, d.healthFull);
            }
            if (d.healthShown) {
                SetRam16(Player(n) + d.healthShown, d.healthFull);
            }
        }
        if (d.playerReadout) {
            d.playerReadout(n, g_combos, lines);
        } else if (readout) {
            g_combos.Hud(n, lines, 0.17f, true, {}, d.healthCeiling != 0);
        }
        // What the player holds, as the game read it off the panel this frame, newest on top, with how long
        if (d.panelHeld) {
            g_inputs.Push(n, d.panelHeld(n), d.facingLeft && d.facingLeft(n));
            if (inputs) {
                g_inputs.Hud(n, lines, d.inputY, d.inputSpacing);
            }
        }
    }
    const int side = d.wideSide ? d.wideSide() : 0;
    g_frames.Hud(FeatureOn("frame_data"), FeatureOn("frame_meter"), lines, boxes, d.framesY, d.framesTop, Video::kWidth, side);
    if (d.boxes && FeatureOn("hit_boxes")) {
        // What the game's memory says now is what the next picture shows, so the boxes are drawn a frame after they
        // are worked out (in a wide picture the stage is further right than on the board's)
        const int stage = d.wideStage ? d.wideStage() : 0;
        for (HudBox &box : g_nextBoxes) {
            box.x += stage;
        }
        boxes.insert(boxes.begin(), g_nextBoxes.begin(), g_nextBoxes.end());
        g_nextBoxes.clear();
        const int cameraX = static_cast<int16_t>(Ram16(d.cameraX)), cameraY = static_cast<int16_t>(Ram16(d.cameraY));
        for (const fight::StageBox &box : d.boxes(machine)) {
            g_nextBoxes.push_back(fight::ToPicture(box, cameraX, cameraY, d.pictureFloor));
        }
    } else {
        g_nextBoxes.clear();
    }
    if (d.hud) {
        d.hud(machine, lines, boxes);
    }
    SetBoxes(std::move(boxes));
    g_dummy.Hud(lines, 0.17f);
    SetHud(std::move(lines));
}

void State(Archive &ar) {
    ar.Mark(0x5452414E); // TRAN
    if (d.framesFirstInState) {
        g_frames.Serialize(ar);
        g_dummy.Serialize(ar);
        ar.Vec(g_roundStart);
        ar.Pod(g_roundFought);
        ar.Pod(g_fighting);
        ar.Pod(g_combos);
        ar.Pod(g_inputs);
        return;
    }
    ar.Pod(g_fighting);
    ar.Pod(g_combos);
    ar.Pod(g_inputs);
    g_dummy.Serialize(ar);
    ar.Vec(g_roundStart);
    ar.Pod(g_roundFought);
    g_frames.Serialize(ar);
}

void Register(const Description &description) {
    d = description;
    g_frames.print = getenv("CPS2RECOMP_FRAMES") != nullptr || (d.framesPrintVariable && getenv(d.framesPrintVariable));
    Game &game = TheGame();
    const char *page = modes::kTraining;
    game.commands.push_back({"training_restart", page, "Start over",
                             "In a training session: the round as it began, with the players where \"Start over at\" says. Backspace "
                             "or a gamepad's left shoulder does this while playing; the stick held as it is pressed chooses the place "
                             "(left, right: a corner; down: sides swapped; up: the middle).",
                             StartOver, kKeyRestart});
    game.features.push_back({"restart_at", page, "Start over at",
                             "Where \"Start over\" puts the players: the middle or a corner, player 1 on the left or (swapped) on the right.",
                             0, {"Middle", "Left corner", "Right corner", "Middle, swapped", "Left corner, swapped", "Right corner, swapped"}});
    game.features.push_back({"training_stage", page, "Stage", d.stageWords, 0, d.stageChoices});
    game.features.push_back({"training_mode", page, "Training session",
                             "The fight has no clock and nobody is knocked out. (\"Start\" above switches this on and takes the game to "
                             "the character choice; Reset ends it.)",
                             kFeatureSession});
    game.features.push_back({"damage_readout", page, "Combo readout",
                             d.healthCeiling ? "Under the bar of the player hit: what the hit took off; for a combo (hits landing before the "
                                               "player is free again) the hits and the damage; how much of it is lost for good and how much "
                                               "comes back; the round's best." :
                                               "Under the bar of the player hit: what the hit took off; for a combo (hits landing before the "
                                               "player is free again) the hits and the damage; the round's best."});
    game.features.insert(game.features.end(), d.readoutFeatures.begin(), d.readoutFeatures.end());
    if (d.panelHeld) {
        game.features.push_back({"input_display", page, "Input display",
                                 d.inputWords ? d.inputWords : d.facingLeft ? "Shows what each player presses and for how many frames: the stick as a keypad number (5 "
                                                "is the middle, 6 forward, 2 down), then the buttons." :
                                                "Shows what each player presses and for how many frames: the stick as a keypad number (5 "
                                                "is the middle, 6 right, 2 down), then the buttons."});
    }
    if (d.boxes) {
        game.features.push_back({"hit_boxes", page, "Hit boxes",
                                 "Draws the boxes the game tests: where a player can be hit (blue), attacks (red; orange for things "
                                 "thrown), and the boxes that keep the players apart (green)."});
        game.features.insert(game.features.end(), d.boxFeatures.begin(), d.boxFeatures.end());
    }
    game.features.push_back({"frame_data", page, "Frame data",
                             "After a move: the step it can first hit on (startup), how many it can hit for (active) and how many follow "
                             "before the player can act (recovery). After a hit or a guarded one: who can act first, by how many steps "
                             "(+: the attacker)."});
    if (d.moveStart) {
        game.features.push_back({"frame_log", page, "Frame data log",
                                 "Writes every move's frame data to frame_data.csv, beside the settings file: the character, what was "
                                 "held as it began, startup, active, recovery and the advantage after a hit or a guarded one."});
    }
    game.features.push_back({"frame_meter", page, "Frame meter",
                             "A row of cells for each player, one for every step of the game: green before a move can hit, red while it "
                             "can, blue after; yellow reeling from a hit, light blue from a guarded one; grey in the air."});
    game.features.push_back({"health_refill", page, "Health refills", "A player's health comes back when they have not been hit for a moment."});
    fight::RegisterDummy(d.dummy);
    for (const char *name : {"damage_readout", "input_display", "hit_boxes", "frame_data", "frame_log", "frame_meter", "health_refill"}) {
        TrainingAids({name});
    }
    for (const char *name : d.ownAids) {
        TrainingAids({name});
    }
    game.inTraining = [] { return InSession(); };
}

} // namespace cps2recomp::session
