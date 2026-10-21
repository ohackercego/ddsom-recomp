// SPDX-License-Identifier: GPL-3.0-only
// Copyright (C) 2026 Tommy McLeroy
//
// Street Fighter Alpha 2: the "Alternative modes" page at the head of the in-game menu (as Vampire Savior has it):
// training (training.cpp), attract mode on a loop, and a viewer for the stages. Each is started by a command and
// ends when the board is reset, when another is started, or (attract mode) with Start.
//
// Attract mode is the game's own loop of notices, introduction, rankings, title and two demonstration fights,
// with no coin in. "The intro only" and "the fights only" leave parts of the loop out by moving the game's own
// step on (kMode) where a wanted part ends.
//
// The stage viewer is a round of a two-player game on the stage chosen, in which nothing is drawn but the stage:
// the players stand a fixed distance apart and are moved together, so that the game's own camera follows them;
// their objects and the panel's are left out of the picture (Video::hideObject), and so is the first scroll
// screen, which holds the bars. The stage's pieces move and its music plays as in a fight. "Infinite" is every
// stage in turn, each scrolled from end to end and then out of the picture as the next comes in: two machines,
// the port's and a spare run beside it every frame, the picture made of the two and the sound mixed.
#include "modes.h"
#include "state.h"

#include <iterator>
#include <vector>

namespace cps2recomp {

void StartTraining(Machine &machine); // training.cpp

namespace {
    using namespace sfa2;

    // The parts of the game's loop, by kMode (each step is two; the steps between are fades). The loop starts again
    // from 0 after the second fight.
    constexpr uint16_t kIntro = 0x10;       // the introduction
    constexpr uint16_t kFirstFight = 0x16;  // a demonstration fight (from the step that sets it up)
    constexpr uint16_t kSecondFight = 0x2C; // another fight
    constexpr uint16_t kLoopEnd = 0x32;
    constexpr uint32_t kAttractFight = 0xFF0C68; // counts the demonstration fights, two for each: picks who fights
    constexpr uint32_t kPlayersIn = 0xFF80A8;

    // The parts of the loop left out for what is to be shown
    bool AttractStep(int show) {
        const uint16_t part = Ram16(kMode);
        if (show == modes::kShowIntro) {
            if (part < kIntro || part > kIntro + 2) {
                modes::GoTo(kIntro);
            }
        } else {
            // (the game counts its fights on, two a fight, in a part after each that is left out here: the count picks
            // who fights next)
            if (part < kFirstFight) {
                modes::GoTo(kFirstFight);
            } else if (part > kFirstFight + 4 && part < kSecondFight) {
                SetRam8(kAttractFight, static_cast<uint8_t>(Ram8(kAttractFight) + 2));
                modes::GoTo(kSecondFight);
            } else if (part >= kLoopEnd) {
                SetRam8(kAttractFight, static_cast<uint8_t>(Ram8(kAttractFight) + 2));
                modes::GoTo(kFirstFight);
            }
        }
        return false;
    }
} // namespace

bool AttractMode() {
    return modes::AttractMode();
}
bool StageViewer() {
    return modes::StageViewer();
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
    d.soundQueue = 0xFFCF60;
    d.soundQueueEnd = 0xFFDF60;
    d.tuneOff = kTuneOff;
    d.player1 = kPlayer1;
    d.playerSize = kPlayerSize;
    d.x = kX;
    d.stageNames.assign(std::begin(kStageNames), std::end(kStageNames));
    d.stageTunes.assign(std::begin(kStageTunes), std::end(kStageTunes));
    d.otherTuneNames = {"Character select", "Versus"};
    d.otherTunes = {kTuneChoose, kTuneVersus};
    d.cameraLeft = 256;
    d.cameraRight = 640;
    d.notTheStage = NotTheStage;
    d.viewerLayers = kLayersWithoutPanel;
    d.wideSide = WideSide;
    d.wideMargin = WideMargin;
    d.wideForSpare = WideForSpare;
    d.wideForPort = WideForPort;
    // (the board starts up through steps 2 to 8 and then begins its loop at 0; a game begun is at 2 for good)
    d.attractBegun = [](int frames) { return frames >= 470; };
    d.attractOver = [] { return Ram8(kPlayersIn) != 0 || Ram8(kCredits) != 0; }; // (a coin in, or a game begun)
    d.attractStep = AttractStep;
    d.attractWords = "The game's own show on a loop, with no coin in: the introduction, the rankings, the title, fights.";
    d.attractShowWords = "What attract mode plays: everything the board does, the introduction over and over, or one "
                         "demonstration fight after another.";
    d.startTraining = StartTraining;
    d.modesWords = "Other ways to run the game: training, attract mode, viewers for the stages and the characters, the colour editor, "
                   "survival, endless battle, ring out.";
    modes::Register(d);
}

} // namespace cps2recomp
