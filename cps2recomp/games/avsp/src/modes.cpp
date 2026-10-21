// SPDX-License-Identifier: GPL-3.0-only
// Copyright (C) 2026 Tommy McLeroy
//
// Alien vs. Predator: the menu's "Alternative modes" (the toolkit's beat/beat.h): what they need of the game.
//
// Attract mode leaves out the parts of the show not wanted by moving the show's step (avsp.h, kMode) on to the
// next part that is, at the step that sets it up; the game then goes on from there as it would have. The show's
// sound is on whatever the board's demo sound says.
#include "avsp.h"
#include "beat.h"

namespace cps2recomp {

namespace {
    using namespace avsp;

    enum { kShowAll, kShowStory, kShowDemonstrations, kShowCharacters };
    constexpr int kBootFrames = 420; // (the board's start: its steps back to 0 at about frame 405)

    bool Wanted(ShowPart part, int show) {
        return (show == kShowStory && part == ShowPart::Story) || (show == kShowDemonstrations && part == ShowPart::Demonstration) ||
               (show == kShowCharacters && part == ShowPart::Characters);
    }

    void GoTo(uint16_t step) {
        SetRam16(kMode, step);
        for (uint32_t at = kMode + 2; at < kMode + 0x10; at += 2) {
            SetRam16(at, 0);
        }
    }

    // A part not wanted: on to the next that is, round the show's loop. Its demonstrations move on the character the
    // next "characters" part shows (004E9A): so does passing one by.
    void AttractFrame() {
        const int show = beat::AttractShow();
        const uint16_t step = Ram16(kMode);
        const ShowPart part = Part(step);
        if (show == kShowAll || part == ShowPart::Other || Wanted(part, show)) {
            return;
        }
        int demonstrations = part == ShowPart::Demonstration ? 1 : 0;
        for (uint16_t next = static_cast<uint16_t>(step + 4); next != step; next = static_cast<uint16_t>((next + 4) % kModeShowEnd)) {
            if (Wanted(Part(next), show)) {
                SetRam16(kShowCharacter, static_cast<uint16_t>((Ram16(kShowCharacter) + demonstrations) & 3));
                GoTo(static_cast<uint16_t>(next - 2));
                return;
            }
            demonstrations += Part(next) == ShowPart::Demonstration ? 1 : 0;
        }
    }

    uint32_t FromBcd(uint32_t bcd) {
        uint32_t value = 0;
        for (int shift = 28; shift >= 0; shift -= 4) {
            value = value * 10 + ((bcd >> shift) & 0xF);
        }
        return value;
    }

    uint8_t g_demoSound = 0;
    bool g_demoSoundSet = false;
} // namespace

// Whether the player's own choices (free play, the cheats, where a game starts) are left out now
bool ChoicesLeftOut() {
    return beat::AttractMode() || beat::ScoreAttack() || beat::Viewing();
}

void ModesBeforeFrame(Machine &machine) {
    beat::BeforeFrame(machine);
}

void ModesFrame(Machine &machine) {
    beat::Frame(machine);
    // (the show's sound on while attract mode is, and the board's own setting back after it)
    if (beat::AttractMode() && !g_demoSoundSet) {
        g_demoSound = Ram8(kSetDemoSound);
        g_demoSoundSet = true;
    } else if (!beat::AttractMode() && g_demoSoundSet) {
        SetRam8(kSetDemoSound, g_demoSound);
        g_demoSoundSet = false;
    }
    if (beat::AttractMode()) {
        SetRam8(kSetDemoSound, 1);
        if (!beat::Starting()) {
            AttractFrame();
        }
    }
    if (beat::ScoreAttack()) {
        SetRam8(kSetContinue, 0);
    }
}

void ModesState(Archive &ar) {
    beat::State(ar);
}

void DescribeViewer(beat::Description &d); // viewer.cpp

void RegisterModes() {
    beat::Description d;
    DescribeViewer(d);
    d.shows = {"The story only", "The demonstrations only", "The characters only"};
    d.showsWords = "What attract mode plays: everything the board does, or only the story, only the demonstration fights, or only "
                   "the characters' files and how to play them, over and over.";
    d.showBegun = [](const Machine &machine) { return machine.frame > kBootFrames; };
    d.canStart = [](const Machine &machine) { return machine.frame > kBootFrames; };
    d.credit = [] { SetRam8(kCredits, 1); };
    d.credits = [] { return static_cast<int>(Ram8(kCredits)); };
    d.inGame = [] { return Ram16(kMode) == kModeGame; };
    d.score = [] { return FromBcd(Ram32(Player(0) + kScore)); };
    beat::Register(d);
}

} // namespace cps2recomp
