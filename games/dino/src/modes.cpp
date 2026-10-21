// SPDX-License-Identifier: GPL-3.0-only
// Copyright (C) 2026 Tommy McLeroy
//
// Cadillacs and Dinosaurs: the menu's "Alternative modes" (the toolkit's beat/beat.h): what they need of the game.
//
// The show is a task of its own that goes through four lists of parts (by the byte at FF0FD0, from the table at
// 097C88), each part a routine it calls: the title (09A3CE, and 098330 the title's second screen), the characters'
// files (09A7E6), a demonstration (099E88, from 097D6C and 097D78, which count them: the count picks the stage),
// the story (0988C0), the best scores (0983D8), and the opening (09BF50, "About 400 years ago"). Attract mode leaves
// a part out by returning from its routine as it is called; the show goes on to the next. The show's sound is on
// whatever the board's demo sound says (board.cpp).
#include "beat.h"
#include "dino.h"

namespace cps2recomp {

namespace {
    using namespace dino;

    enum { kShowAll, kShowStory, kShowDemonstrations, kShowCharacters };

    // A part's routine is called: left out if it is not wanted
    template <int kShow> bool Part(M68K &c) {
        const int show = beat::AttractShow();
        if (show == kShowAll || show == kShow || !beat::AttractMode()) {
            return false;
        }
        Return(c);
        return true;
    }

    uint32_t FromBcd(uint32_t bcd) {
        uint32_t value = 0;
        for (int shift = 28; shift >= 0; shift -= 4) {
            value = value * 10 + ((bcd >> shift) & 0xF);
        }
        return value;
    }
} // namespace

// Whether the player's own choices (free play, the cheats, where a game starts, the board's settings for the show's
// sound and continuing) are left out now
bool ChoicesLeftOut() {
    return beat::AttractMode() || beat::ScoreAttack() || beat::Viewing();
}

void ModesBeforeFrame(Machine &machine) {
    beat::BeforeFrame(machine);
}

void ModesFrame(Machine &machine) {
    beat::Frame(machine);
}

void ModesState(Archive &ar) {
    beat::State(ar);
}

void DescribeViewer(beat::Description &d); // viewer.cpp

void RegisterModes() {
    beat::Description d;
    DescribeViewer(d);
    d.shows = {"The story only", "The demonstrations only", "The characters only"};
    d.showsWords = "What attract mode plays: everything the board does, or only the story (the opening and the dinosaurs' "
                   "attack), only the demonstrations, or only the characters' files, over and over.";
    d.showBegun = [](const Machine &) { return Ram8(kShow) >= kShowTitle; };
    d.canStart = [](const Machine &) { return Ram8(kShow) >= kShowTitle; };
    d.credit = [] { SetRam8(kChute + kChuteCredits, 1); };
    d.credits = [] { return static_cast<int>(Ram8(kChute + kChuteCredits)); };
    // (watched only once the credit is taken: the show's demonstrations, which have the step too, are over by then)
    d.inGame = [] {
        const uint8_t step = Ram8(kStep);
        return step != 0 && (step < kStepGameOver || step > kStepGameOver + 8);
    };
    d.score = [] { return FromBcd(Ram32(Player(0) + kScore)); };
    beat::Register(d);
    AddHook(0x09A3CE, "ShowTitle", Part<-1>);
    AddHook(0x098330, "ShowTitleSecond", Part<-1>);
    AddHook(0x09A7E6, "ShowCharacters", Part<kShowCharacters>);
    AddHook(0x099E88, "ShowDemonstration", Part<kShowDemonstrations>);
    AddHook(0x0988C0, "ShowStory", Part<kShowStory>);
    AddHook(0x0983D8, "ShowScores", Part<-1>);
    AddHook(0x09BF50, "ShowOpening", Part<kShowStory>);
}

} // namespace cps2recomp
