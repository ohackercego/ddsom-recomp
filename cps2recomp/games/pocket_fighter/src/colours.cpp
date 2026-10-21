// SPDX-License-Identifier: GPL-3.0-only
// Copyright (C) 2026 Tommy McLeroy
//
// Super Gem Fighter Mini Mix: the players' own colours for the characters and the colour editor, which are the
// toolkit's (fight/colours.h). These are the game's facts for them: where its characters' colours are (pocket.h), the
// ROM file PalMod changes, and how the editor shows a set on player 1 in the model viewer (model.cpp).
#include "colours.h"
#include "machine.h"
#include "model.h"
#include "pocket.h"

#include <iterator>
#include <vector>

namespace cps2recomp {

void StartModelViewer(Machine &machine); // model.cpp
bool ModelViewer();

namespace {
    using namespace pocket;

    uint32_t SetAddress(int number, int set) {
        return kColourSets + static_cast<uint32_t>(number * kColourSetsEach + set) * kColourSetSize;
    }
    std::vector<colours::Range> Ranges() {
        return {{kColourSets, kColourSets + kCharacters * kColourSetsEach * kColourSetSize}};
    }

    // Player 1 in the set: its choice as the game keeps it; its palettes are in its own colour sets of the buffer the
    // board shows from (90C000, a palette of 0x20 bytes a set)
    uint32_t ShowSet(Machine &, int, int set, int &lines) {
        SetRam8(Player(0) + kColourChoice, static_cast<uint8_t>(set));
        lines = kPalettesInSet;
        return 0x90C000 + (Ram8(Player(0) + kColourSet) & 0x1F) * 0x20u;
    }

    // The stages as short as the panel needs them: whose they are
    std::vector<const char *> Backgrounds() {
        static const char *const names[kStages] = {"Felicia's", "Ibuki's", "Hsien-Ko's", "Chun-Li's", "Morrigan's", "Ryu's", "Tessa's", "Ken's"};
        std::vector<const char *> list(std::begin(names), std::end(names));
        list.push_back("White");
        list.push_back("Black");
        return list;
    }
} // namespace

void RegisterColours() {
    colours::Description d;
    for (int n = 0; n < kCharacters; n++) {
        // (the names the game has in Japan, for files called by them: Hsien-Ko is Lei-Lei there)
        d.characters.push_back({n, kCharacterNames[n], n == 5 ? "Lei-Lei" : nullptr});
    }
    d.setNames = {"P", "K", "S", "PK"};
    d.palettesInSet = kPalettesInSet;
    d.setAddress = SetAddress;
    d.ranges = Ranges;
    // The last of the set's program files has every character's colours (and the stages', the panel's); PalMod never
    // changes its first part, which is the game's data
    d.romFile = "pcf.07";
    d.romFileAt = 0x200000;
    d.romFileSize = 0x80000;
    d.same = {0x200000, 0x209000};
    d.romFileWhose = "Super Gem Fighter Mini Mix's, 970904";
    d.featureDescription = "Characters in the colours of the files in the \"palettes\" folder in the game's data folder: a "
                           "pcf.07 changed with PalMod, and text files, one a character (Sakura.txt), which go before it. "
                           "With no files the game's own colours. They are taken when a player is coloured: at the next round.";
    d.originalHelp = "# To change them, copy this file to the palettes folder and change the colours there. Each set is\n"
                     "# chosen by the button that chose the character ([P]: Punch, [K]: Kick, [S]: Special, [PK]: two\n"
                     "# buttons at once) and has five lines of 16 colours; the character is drawn in the first line's, the\n"
                     "# others are its effects. The 16th colour of a line is never seen. A colour is #RRGGBB or #RGB,\n"
                     "# rounded to the board's 16 levels of each; - keeps the game's. Sets and lines that are left out keep\n"
                     "# the game's colours.\n";
    d.showSet = ShowSet;
    d.startViewer = StartModelViewer;
    d.viewerOn = ModelViewer;
    d.viewerCharacter = model::CharacterNumber;
    d.characterNumber = model::Number;
    d.animationStep = model::AnimationStep;
    d.animationNames = model::AnimationNames;
    d.animationForLine = [](Machine &, int line) { return model::AnimationForLine(line); };
    d.backgrounds = Backgrounds();
    colours::Register(d);
}

} // namespace cps2recomp
