// SPDX-License-Identifier: GPL-3.0-only
// Copyright (C) 2026 Tommy McLeroy
//
// Vampire Savior: the "Alternative modes" page at the head of the in-game menu, attract mode on a loop and a viewer for
// the stages are the toolkit's (fight/modes.h); this says where the game keeps what they need and which parts of its
// loop are which (training is training.cpp's: the toolkit's fight/session.h).
#include "modes.h"
#include "session.h"
#include "state.h"
#include "widescreen.h"

#include <iterator>
#include <vector>

namespace cps2recomp {

namespace {
    using namespace vsav;

    // The parts of the game's loop, by kMode (each step is two; the odd steps between are fades)
    constexpr uint16_t kLoop = 0x0C;        // where the loop starts
    constexpr uint16_t kNotices = 0x10;     // the sound notice, then the maker's
    constexpr uint16_t kIntro = 0x16;       // the introduction, ending on the title
    constexpr uint16_t kFirstFight = 0x1A;  // a fight
    constexpr uint16_t kScores = 0x22;      // the score ranking, the maker's notice, the title
    constexpr uint16_t kSecondFight = 0x38; // another fight
    constexpr uint16_t kVersus = 0x40;      // the versus ranking; then the loop starts again from kNotices

    // The parts of the loop left out for what is to be shown (true: the loop's own start, run unseen)
    bool AttractStep(int show) {
        const uint16_t part = Ram16(kMode);
        if (part < kNotices) {
            return true;
        }
        if (show == modes::kShowIntro) {
            if (part < kIntro || part >= kFirstFight) {
                modes::GoTo(kIntro);
            }
        } else {
            // (the game counts its fights on as it comes to a ranking, which is left out here: the count picks who
            // fights next, and where)
            if (part < kFirstFight) {
                modes::GoTo(kFirstFight);
            } else if (part >= kVersus) {
                SetRam8(kAttractFight, static_cast<uint8_t>(Ram8(kAttractFight) + 2));
                modes::GoTo(kFirstFight);
            } else if (part >= kScores && part < kSecondFight) {
                SetRam8(kAttractFight, static_cast<uint8_t>(Ram8(kAttractFight) + 2));
                modes::GoTo(kSecondFight);
            }
        }
        return false;
    }

    int g_keptShadow = -1; // (machines.cpp: the shadow not drawn, kept around a spare's frame)
} // namespace

int NoShadowNow(); // machines.cpp

bool AttractMode() {
    return modes::AttractMode();
}
bool StageViewer() {
    return modes::StageViewer();
}
// The stage the machine that is running is to fight on (the game's choice of stage: training.cpp)
int StageViewerStage() {
    return modes::StageViewerStage();
}
void EndModes() {
    modes::End();
}
void ModesBeforeFrame(Machine &machine) {
    modes::BeforeFrame(machine);
}
void ModesFrame(Machine &machine) {
    modes::Frame(machine);
}
void ModesState(Archive &ar) {
    modes::State(ar);
}

void RegisterModes() {
    modes::Description d;
    d.steps = kMode;
    d.gameChoosing = kGameChoosing;
    d.gameMatch = kGameMatch;
    d.matchRound = kMatchRound;
    d.roundFought = kRoundFought;
    d.credits = kCredits;
    d.playersIn = kPlayersIn;
    d.soundQueue = kSoundQueue;
    d.soundQueueEnd = kSoundQueueEnd;
    d.tuneOff = kTuneOff;
    d.player1 = kPlayer1;
    d.playerSize = kPlayerSize;
    d.x = kX;
    d.xLong = true;
    d.startsTogether = true;
    d.stageNames = StageChoices({});
    for (int stage = 1; stage <= kStages; stage++) {
        d.stageTunes.push_back(StageTune(stage));
    }
    d.otherTuneNames = {"Character select"};
    d.otherTunes = {0x02};
    d.cameraLeft = kCameraLeft;
    d.cameraRight = kCameraRight;
    d.notTheStage = NotTheStage;
    d.wideSide = WideSide;
    d.wideMargin = WideMargin;
    d.wideForSpare = WideForSpare;
    d.wideForPort = WideForPort;
    d.attractBegun = [](int) { return Ram16(kMode) >= kLoop; };
    d.attractOver = [] { return Ram16(kMode) < kLoop; };
    d.attractStep = AttractStep;
    // (the shadow a mode keeps from being drawn: machines.cpp)
    d.keep = [] { g_keptShadow = NoShadowNow(); };
    d.fresh = [](Machine &machine) { NoShadow(machine, -1); };
    d.putBack = [](Machine &machine) { NoShadow(machine, g_keptShadow); };
    d.startTraining = session::StartSession;
    modes::Register(d);
}

} // namespace cps2recomp
