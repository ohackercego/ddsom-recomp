// SPDX-License-Identifier: GPL-3.0-only
// Copyright (C) 2026 Tommy McLeroy
//
// Super Gem Fighter Mini Mix: the "Alternative modes" page, attract mode and the stage viewer are the toolkit's
// (fight/alternative.h); this says where the game keeps what they need, which parts of its loop are which, and makes the
// match a mode wants (its characters and stage) where the game makes one.
#include "alternative.h"
#include "pocket.h"
#include "session.h"

#include <iterator>

namespace cps2recomp {

namespace {
    using namespace pocket;

    // The parts of the loop left out for what is to be shown
    bool AttractStep(int show) {
        const uint16_t part = Ram16(kMode);
        if (part < kLoop + 4) {
            return true; // (the loop's own start)
        }
        if (show == modes::kShowIntro) {
            if (part < kIntro || part > kIntro + 2) {
                modes::GoTo(kIntro);
            }
        } else if (part < kFirstFight || (part >= kScores && part < kSecondFight)) {
            modes::GoTo(part < kFirstFight ? kFirstFight : kSecondFight);
        } else if (part >= kVersus) {
            modes::GoTo(kFirstFight);
        }
        return false;
    }

    // 017790: the match has been made (009970: the computer's next opponent and the stage, or for two players the
    // stage of one of them). What a mode wants is put in its place: the characters, and the stage.
    bool MatchMade(M68K &) {
        const MatchWanted &wanted = modes::Wanted();
        for (int n = 0; n < 2; n++) {
            if (wanted.characters[n] >= 0 && wanted.characters[n] < kCharacters) {
                SetRam8(Player(n) + kCharacter, static_cast<uint8_t>(wanted.characters[n]));
            }
        }
        const int stage = session::WantedStage();
        if (stage >= 1 && stage <= kStages) {
            SetRam16(kStage, static_cast<uint16_t>((stage - 1) * 2));
        }
        return false;
    }
} // namespace

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
    d.player1 = kPlayer1;
    d.playerSize = kPlayerSize;
    d.x = kX;
    d.stageNames.assign(std::begin(kStageNames), std::end(kStageNames));
    for (int stage = 1; stage <= kStages; stage++) {
        d.stageTunes.push_back(static_cast<uint16_t>(stage));
    }
    d.otherTuneNames = {"Character select", "Versus"};
    d.otherTunes = {kTuneChoose, kTuneVersus};
    d.cameraLeft = kCameraLeft;
    d.cameraRight = kCameraRight;
    d.notTheStage = NotTheStage; // (pocket.h)
    d.wideSide = WideSide;
    d.wideMargin = WideMargin;
    d.wideForSpare = WideForSpare;
    d.wideForPort = WideForPort;
    d.attractBegun = [](int) { return Ram16(kMode) >= kLoop; };
    d.attractOver = [] { return Ram16(kMode) < kLoop; };
    d.attractStep = AttractStep;
    d.attractWords = "The game's own show on a loop, with no coin in: the introduction, the title, fights and rankings.";
    d.startTraining = session::StartSession;
    d.modesWords = "Other ways to run the game: training, attract mode, viewers for the stages and the characters, survival, "
                   "endless battle, ring out.";
    modes::Register(d);
    AddHook(HookAt(0x017790), "MatchMade", MatchMade);
}

} // namespace cps2recomp
