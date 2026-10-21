// SPDX-License-Identifier: GPL-3.0-only
// Copyright (C) 2026 Tommy McLeroy
//
// Super Puzzle Fighter II Turbo: where the game keeps its characters' and its gems' colours, for the toolkit's
// custom colours and colour editor (fight/colours.h), and the viewer the editor works in (modes.cpp).
//
// The characters' colours are a table at 0A0000: for each character (0-10) two sets, the first chosen with the
// first button and the second with the other, each four palettes of 16 colours (0x80 bytes). The game copies
// a player's set to the board's object palettes 12-15 (player 1) or 16-19 (player 2) when a match is set up
// (020DC4), and the character screens take theirs from the same table.
//
// The gems are not a character, but the editor takes them as one ("Gems"): four palettes, red, blue, yellow
// and green (the board's object palettes 0-3, which the gems, the crash gems, the counter gems and the big
// gems of that colour are all drawn in). The game has them twice: at 0A0B00 for a match (020E18), and at
// 0A1AC0, in the whole page of palettes it copies for the introduction's demonstration match and the
// how-to-play dojo (020C9A, with the page number 4 at FF8128). The Gems' first set is the first, its second
// set the other.
#include "colours.h"
#include "machine.h"
#include "spf2.h"

#include <vector>

namespace cps2recomp {

void StartColourViewer(Machine &machine, int character); // modes.cpp
bool ColourViewer();
bool ColourViewerReady();
int ColourViewerCharacter();

namespace {
    using namespace spf2;

    constexpr uint32_t kCharacterColours = 0x0A0000, kSetSize = 0x80;
    constexpr int kCharacters = 11, kSets = 2;
    constexpr uint32_t kGemColours[kSets] = {0x0A0B00, 0x0A1AC0};
    constexpr int kGems = 0x20; // (the editor's number for the gems: not a character's)
    // Where the board shows from: the object palettes, 32 bytes each (player 1's character from the 12th)
    constexpr uint32_t kObjectPalettes = 0x90C000, kPlayer1Palettes = 0x90C180;

    // The editor's "Character" choice: the characters in the order the character screen has them, the hidden
    // three, and the gems
    constexpr int kChoices[] = {2, 1, 7, 3, 0, 4, 5, 6, 8, 9, 10, kGems};
    constexpr int kChoiceCount = static_cast<int>(sizeof(kChoices) / sizeof(kChoices[0]));
    int g_editing = -1; // what the viewer was started for (a number of kChoices)

    int ChosenNumber() {
        const int choice = FeatureValue("model_character");
        return kChoices[choice >= 0 && choice < kChoiceCount ? choice : 0];
    }

    uint32_t SetAddress(int number, int set) {
        if (number == kGems) {
            return kGemColours[set & 1];
        }
        return kCharacterColours + static_cast<uint32_t>(number * kSets + (set & 1)) * kSetSize;
    }
    std::vector<colours::Range> Ranges() {
        return {{kCharacterColours, kCharacterColours + kCharacters * kSets * kSetSize},
                {kGemColours[0], kGemColours[0] + kSetSize},
                {kGemColours[1], kGemColours[1] + kSetSize}};
    }
    // The set shown on player 1's character (or on the gems), and where its palettes are in what the board shows
    uint32_t ShowSet(Machine &, int number, int set, int &lines) {
        lines = 4;
        if (!ColourViewerReady()) {
            return 0;
        }
        if (number == kGems) {
            return kObjectPalettes;
        }
        SetRam8(Player(0) + 0x9F, static_cast<uint8_t>(set)); // (the player's other colours: select.cpp, kOtherColours)
        return kPlayer1Palettes;
    }

    void StartViewer(Machine &machine) {
        g_editing = ChosenNumber();
        // (for the gems, Ryu stands by them)
        StartColourViewer(machine, g_editing == kGems ? 2 : g_editing);
    }
    int ViewerCharacter() {
        return ColourViewer() ? g_editing : -1;
    }
    int CharacterNumber(int choice) {
        return choice >= 0 && choice < kChoiceCount ? kChoices[choice] : -1;
    }
    // The editor's "Animation": player 1's character standing, or attacking over and over, as it does when its
    // player sends counter gems (turn.cpp, WarnOfAttack: the act by the attack's size, in the player's record
    // and then the character's +D0; the character's state +5 is 0A while it attacks)
    const std::vector<const char *> &AnimationNames() {
        // (short enough for the panel beside the row's own name)
        static const std::vector<const char *> names = {"Standing", "Small attack", "Attack", "Big attack", "Bigger attack",
                                                        "Huge attack", "Finisher"};
        return names;
    }
    int AnimationStep(Machine &, int choice, int by) {
        const int count = static_cast<int>(AnimationNames().size());
        return ((choice + by) % count + count) % count;
    }
    constexpr uint32_t kCharacter1 = 0xFF8B00;   // player 1's character on the screen (the record +32 points at it)
    constexpr uint32_t kAct = 0xD0, kState = 0x04; // byte; long: the steps of its state machine
    constexpr uint32_t kAttacking = 0x020A0000;
    constexpr int kAttackAgainAfter = 40;        // frames it stands between two attacks
    int g_standing = 0;                          // frames since the last attack ended

    // The editor's "Background": the match's stage, or plain black or white. The stage, the fields' frames and
    // the words are drawn in the scroll layers, whose palettes (the pages after the objects') are made all one
    // colour; the stage's own objects (object palettes 26-31, which the stage sets: 0-11 are the gems', 12-19
    // the characters', 20-25 their attacks') are not drawn. The stage's palettes are kept to put back for
    // "The stage" and when the editor ends (the game sets them once, when the match is set up).
    constexpr uint32_t kScrollPalettes = 0x90C400, kScrollPalettesEnd = 0x90D000;
    constexpr uint16_t kFirstStagePalette = 26;
    std::vector<uint16_t> g_stagePalettes; // (empty: the stage's are in place)
    bool HideStageObject(uint32_t, uint16_t attributes) {
        return (attributes & 0x1F) >= kFirstStagePalette;
    }
    void PlainBackground(Machine &machine) {
        const int choice = ColourViewerReady() ? FeatureValue("model_background") : 0;
        if (choice <= 0) {
            if (!g_stagePalettes.empty()) {
                for (uint32_t at = kScrollPalettes, n = 0; at < kScrollPalettesEnd; at += 2, n++) {
                    machine.Write16(at, g_stagePalettes[n]);
                }
                g_stagePalettes.clear();
            }
            if (machine.video.hideObject == HideStageObject) {
                machine.video.hideObject = nullptr;
            }
            return;
        }
        if (g_stagePalettes.empty()) {
            for (uint32_t at = kScrollPalettes; at < kScrollPalettesEnd; at += 2) {
                g_stagePalettes.push_back(machine.Read16(at));
            }
        }
        const uint16_t colour = choice == 1 ? 0xF000 : 0xFFFF;
        for (uint32_t at = kScrollPalettes; at < kScrollPalettesEnd; at += 2) {
            machine.Write16(at, colour);
        }
        machine.video.hideObject = HideStageObject;
    }
    void Animate() {
        const int choice = FeatureValue("colours_animation");
        if (choice <= 0 || g_editing == kGems) {
            return;
        }
        if (Ram8(kCharacter1 + kState + 1) == (kAttacking >> 16 & 0xFF)) {
            g_standing = 0;
            return;
        }
        if (++g_standing >= kAttackAgainAfter) {
            SetRam8(kCharacter1 + kAct, static_cast<uint8_t>(2 * (choice - 1)));
            SetRam32(kCharacter1 + kState, kAttacking);
            g_standing = 0;
        }
    }
} // namespace

// Before every frame: the files' colours put in or taken out as the feature is switched; and the editor's
// panel worked, starting the viewer again when another character is chosen
void ColoursBeforeFrame(Machine &machine) {
    colours::BeforeFrame(machine);
    if (!ColourViewer()) {
        if (machine.video.hideObject == HideStageObject) {
            machine.video.hideObject = nullptr; // (the board was reset: its palettes are the next game's own)
        }
        g_stagePalettes.clear();
        return;
    }
    if (ChosenNumber() != g_editing) {
        StartViewer(machine);
        return;
    }
    if (ColourViewerReady()) {
        colours::EditorBeforeFrame(machine);
        Animate();
        PlainBackground(machine);
    }
}

// After the frame: the panel over the picture
void ColoursFrame(Machine &machine) {
    if (ColourViewerReady()) {
        PlainBackground(machine); // (and again for the next frame's palette, should the game have written one)
        colours::EditorFrame(machine);
    }
}

void RegisterColours() {
    colours::Description d;
    d.characters = {
        {0, "Morrigan", nullptr}, {1, "Chun-Li", nullptr}, {2, "Ryu", nullptr},      {3, "Ken", nullptr},
        {4, "Hsien-Ko", "Lei-Lei"}, {5, "Donovan", nullptr}, {6, "Felicia", nullptr}, {7, "Sakura", nullptr},
        {8, "Devilot", nullptr},  {9, "Akuma", "Gouki"},   {10, "Dan", nullptr},     {kGems, "Gems", nullptr},
    };
    // (a character's: by the button that chooses it; the gems': in a match, and in the attract's demonstration
    // and dojo)
    d.setNames = {"A", "B"};
    d.palettesInSet = 4;
    d.lineNames = {"Line 1 / red", "Line 2 / blue", "Line 3 / yellow", "Line 4 / green"};
    d.setAddress = SetAddress;
    d.ranges = Ranges;
    d.romFile = "pzf.04";
    d.romFileAt = 0x080000;
    d.romFileSize = 0x080000;
    d.same = {0x080000, kCharacterColours}; // (the graphics' tables before the colours; every region has this file)
    d.romFileWhose = "Super Puzzle Fighter II Turbo's";
    d.featureDescription = "Characters and gems in the colours of the files in the \"palettes\" folder in the game's data "
                           "folder: a pzf.04 changed with PalMod, and text files, one a character (Ryu.txt) and one for "
                           "the gems (Gems.txt), which go before it. With no files the game's own colours. They are taken "
                           "when the game next sets its colours: at the next match.";
    d.originalHelp = "# To change them, copy this file to the palettes folder and change the colours there. A character\n"
                     "# has two sets: [A], chosen with the first button, and [B], with the other. Each has four lines\n"
                     "# of 16 colours: the character is drawn in the first line's, the others are for its attacks.\n"
                     "# Gems.txt is the gems': [A] in a match and [B] in the attract's demonstration and dojo, each\n"
                     "# with four lines, red, blue, yellow and green (the gems, crash gems, counter gems and big gems\n"
                     "# of that colour). The 16th colour of a line is never seen. A colour is #RRGGBB or #RGB, rounded\n"
                     "# to the board's 16 levels of each; - keeps the game's. Sets and lines left out keep the game's.\n";
    d.showSet = ShowSet;
    d.startViewer = StartViewer;
    d.viewerOn = ColourViewer;
    d.viewerCharacter = ViewerCharacter;
    d.characterNumber = CharacterNumber;
    d.animationStep = AnimationStep;
    d.animationNames = AnimationNames;
    d.backgrounds = {"Stage", "Black", "White"};
    colours::Register(d);
    // The viewer's choices the editor has rows for (in Vampire Savior's they are the model viewer's)
    Game &game = TheGame();
    game.features.push_back({"model_character", "Colour editor", "Character",
                             "Whose colours the editor changes: a character (shown as player 1, in a match nobody plays), or "
                             "the gems (in player 1's field: a row of each colour with a crash gem and a counter gem, and the "
                             "diamond). Changing it while editing goes to the new one.",
                             0, {"Ryu", "Chun-Li", "Sakura", "Ken", "Morrigan", "Hsien-Ko", "Donovan", "Felicia", "Devilot", "Akuma",
                                 "Dan", "Gems"}});
    game.features.push_back({"model_background", "Colour editor", "Background",
                             "What is behind the gems and the characters: the match's stage, or plain black or white (the "
                             "fields' frames and the words go too).",
                             0, {"The stage", "Black", "White"}});
    game.features.push_back({"model_size", "Colour editor", "Size", "As large as in a match.", 0, {"As in a match"}});
}

} // namespace cps2recomp
