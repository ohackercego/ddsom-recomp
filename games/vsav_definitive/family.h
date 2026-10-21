// SPDX-License-Identifier: GPL-3.0-only
// Copyright (C) 2026 Tommy McLeroy
//
// Vampire Savior Definitive: Vampire Savior's program, so the places its sources name are its own (no list of them:
// games/vampire_savior/src/places.h), and its facts are Vampire Savior's (games/vampire_savior/family.h: keep the two
// in step) but for the characters taken from Vampire Savior 2 (src/graft.cpp).
#pragma once

#include <array>
#include <cstdint>

#include "game.h"

namespace vsav::family {

struct Place {
    uint32_t vsav, here;
};
inline constexpr std::array<Place, 0> kPlaces = {};

// The characters a player chooses, by the number the game keeps for each (a player's record, +382), with
// what this set calls them and, if another, what the American and European sets do
struct Character {
    uint8_t number;
    const char *name, *otherName;
};
inline constexpr Character kRoster[] = {
    {0x00, "Bulleta", "BB Hood"}, {0x01, "Demitri", nullptr},    {0x02, "Gallon", "Talbain"},     {0x03, "Victor", nullptr},
    {0x04, "Zabel", "Lord Raptor"}, {0x05, "Morrigan", nullptr}, {0x06, "Anakaris", nullptr},     {0x07, "Felicia", nullptr},
    {0x08, "Bishamon", nullptr},  {0x09, "Aulbath", "Rikuo"},   {0x0A, "Sasquatch", nullptr},    {0x0C, "Q-Bee", nullptr},
    {0x0D, "Lei-Lei", "Hsien-Ko"}, {0x0E, "Lilith", nullptr},   {0x0F, "Jedah", nullptr},
    // (from Vampire Savior 2, at the numbers they have there)
    {0x10, "Phobos", "Huitzil"},  {0x11, "Pyron", nullptr},     {0x13, "Donovan", nullptr},
};
// The hidden ones (src/choosing.cpp): Gallon's and Bishamon's numbers with bit 4; and what the "Hidden characters"
// feature gives (its words for the menu). The feature works in the rewrites of the choosing code (no hooks: 0)
inline constexpr Character kHidden[] = {{0x12, "Dark Gallon", "Dark Talbain"}, {0x18, "Oboro Bishamon", nullptr}};
inline constexpr uint8_t kDarkGallon = 0x12, kOboroBishamon = 0x18;
inline constexpr uint32_t kHiddenAtColours = 0, kHiddenShadow = 0;
inline constexpr const char *kHiddenWords =
    "Choosing Gallon gives Dark Gallon and choosing Bishamon gives Oboro Bishamon (the last boss, not chosen by any code "
    "on the board); a punch chooses their first colours, a kick their second. The \"?\" box gives Shadow, who becomes "
    "each opponent beaten. For player 1, player 2 or both; the alternative modes' fighters too.";

// The stages, in the order of their numbers (the word at FF8100 is the number doubled)
inline constexpr const char *kStageNames[] = {
    "Feast of the Damned", "Concrete Cave", "Tower of Arrogance", "Red Thirst", "Deserted Chateau", "Abaraya",
    "Vanity Paradise", "War Agony", "Forever Torment", "Green Scream", "Iron Horse, Iron Terror", "Fetus of God",
};

// The frame's lists of objects to draw that the objects of kind 12 have, one for each depth (src/picture.cpp)
inline constexpr int kTwelveLists = 7;

// Each stage's own tune (src/modes.cpp: the stage viewer's music)
inline constexpr uint16_t kStageTunes[] = {0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x10, 0x11, 0x12, 0x13, 0x14};

// Its music, for a custom soundtrack (src/music.cpp): the sound program (driver 1.30) keeps a table of its songs (a
// word, how many; song 0's entry four bytes on), and each piece of music named. Found by playing: each stage, the
// arcade ladder to the end with every character (player 1's character byte held), a lost game and attract mode
// (CPS2RECOMP_SOUNDTRACK_LOG), and --music-test for the ones the board plays once (`true`: quiet before 90 s)
struct Song {
    uint16_t number;
    const char *name, *title;
    bool once = false;
};
inline constexpr uint32_t kSongTable = 0x9006, kSongCount = 0x6D8;
inline constexpr Song kSongs[] = {
    // Each stage's own (kStageTunes)
    {0x09, "feast_of_the_damned", "Feast of the Damned"},
    {0x0A, "concrete_cave", "Concrete Cave"},
    {0x0B, "tower_of_arrogance", "Tower of Arrogance"},
    {0x0C, "red_thirst", "Red Thirst"},
    {0x0D, "deserted_chateau", "Deserted Chateau"},
    {0x0E, "abaraya", "Abaraya"},
    {0x0F, "vanity_paradise", "Vanity Paradise"},
    {0x10, "war_agony", "War Agony"},
    {0x11, "forever_torment", "Forever Torment"},
    {0x12, "green_scream", "Green Scream"},
    {0x13, "iron_horse_iron_terror", "Iron Horse, Iron Terror"},
    {0x14, "fetus_of_god", "Fetus of God"},
    // Around the fights
    {0x55, "qsound", "The QSound logo", true},
    {0x57, "capcom", "The Capcom logo", true},
    {0x01, "opening", "The opening", true},
    {0x02, "character_select", "Character select"},
    {0x03, "next", "Between a win and the next versus screen"},
    {0x05, "versus", "Versus screen", true},
    {0x47, "rival", "Before the last fight (the rival)"},
    {0xE2, "jedah_appears", "Jedah appears", true},
    {0x06, "continue", "Continue?"},
    {0x07, "game_over", "Game over", true},
    {0x08, "ranking", "Score ranking", true},
    {0x45, "staff_roll", "Staff roll"},
    // The winner's, on the screen after a win
    {0x15, "bulleta_win", "Bulleta wins"},
    {0x16, "demitri_win", "Demitri wins"},
    {0x17, "gallon_win", "Gallon wins"},
    {0x18, "victor_win", "Victor wins"},
    {0x19, "zabel_win", "Zabel wins"},
    {0x1A, "morrigan_win", "Morrigan wins"},
    {0x1B, "anakaris_win", "Anakaris wins"},
    {0x1C, "felicia_win", "Felicia wins"},
    {0x1D, "bishamon_win", "Bishamon wins"},
    {0x1E, "aulbath_win", "Aulbath wins"},
    {0x1F, "sasquatch_win", "Sasquatch wins"},
    {0x21, "qbee_win", "Q-Bee wins"},
    {0x22, "leilei_win", "Lei-Lei wins"},
    {0x23, "lilith_win", "Lilith wins"},
    {0x24, "jedah_win", "Jedah wins"},
    // The endings (Q-Bee's and Lei-Lei's by the order of the others: not seen yet)
    {0x25, "bulleta_ending", "Bulleta's ending"},
    {0x26, "demitri_ending", "Demitri's ending", true},
    {0x27, "gallon_ending", "Gallon's ending", true},
    {0x28, "victor_ending", "Victor's ending"},
    {0x29, "zabel_ending", "Zabel's ending"},
    {0x2A, "morrigan_ending", "Morrigan's ending"},
    {0x2B, "anakaris_ending", "Anakaris's ending"},
    {0x2C, "felicia_ending", "Felicia's ending"},
    {0x2D, "bishamon_ending", "Bishamon's ending"},
    {0x2E, "aulbath_ending", "Aulbath's ending"},
    {0x2F, "sasquatch_ending", "Sasquatch's ending"},
    {0x31, "qbee_ending", "Q-Bee's ending"},
    {0x32, "leilei_ending", "Lei-Lei's ending"},
    {0x33, "lilith_ending", "Lilith's ending"},
    {0x34, "jedah_ending", "Jedah's ending"},
    // Music the game has whose place is not known yet
    {0x04, "music_04", "Where it plays is not known yet", true},
    {0x35, "music_35", "Where it plays is not known yet"},
    {0x36, "music_36", "Where it plays is not known yet"},
    {0x37, "music_37", "Where it plays is not known yet"},
    {0x38, "music_38", "Where it plays is not known yet"},
    {0x39, "music_39", "Where it plays is not known yet"},
    {0x3A, "music_3a", "Where it plays is not known yet"},
    {0x3B, "music_3b", "Where it plays is not known yet"},
    {0x3C, "music_3c", "Where it plays is not known yet"},
    {0x3D, "music_3d", "Where it plays is not known yet"},
    {0x3E, "music_3e", "Where it plays is not known yet"},
    {0x3F, "music_3f", "Where it plays is not known yet"},
    {0x42, "music_42", "Where it plays is not known yet"},
    {0x43, "music_43", "Where it plays is not known yet"},
    {0x44, "music_44", "Where it plays is not known yet"},
    {0x46, "music_46", "Where it plays is not known yet"},
    {0x4E, "music_4e", "Where it plays is not known yet"},
    {0x4F, "music_4f", "Where it plays is not known yet"},
    {0x50, "music_50", "Where it plays is not known yet"},
    {0x51, "music_51", "Where it plays is not known yet"},
    {0x52, "music_52", "Where it plays is not known yet"},
    {0x53, "music_53", "Where it plays is not known yet"},
    {0x54, "music_54", "Where it plays is not known yet", true},
    {0x478, "music_478", "Where it plays is not known yet", true},
};

// Where the hit test goes on to when an attack has hit (src/collision.cpp): HitLands (018230)
inline constexpr uint32_t kHitTestHit = 0x018230;

// How the fight's rules differ from game to game (src/fight.cpp, src/edges.cpp; 0: as in Vampire Savior):
// - a byte of the attacker's record that, set, has all of a hit's damage taken for good, and where (from A5) the game
//   keeps A0 while it looks
// - one more kind of attack that is an overhead
// - a byte of the victim's record that, set, lets it guard the attacks that cannot be guarded
// - how near (across) the other player has to be for a player to guard by being near
// - a byte of a player's record set (FF) when the player is kept in at the picture's edge
inline constexpr uint32_t kAllForGood = 0;
inline constexpr int32_t kAllForGoodKeep = 0;
inline constexpr uint8_t kOverheadToo = 0;
inline constexpr uint32_t kGuardsAnything = 0;
inline constexpr uint16_t kGuardNear = 0x80;
inline constexpr uint32_t kAtEdgeToo = 0;

// Tiles of the objects the port tells apart (src/machines.cpp, src/widescreen.cpp), from one to one less than the
// other: the shadows (colour set 22), the panel's faces beside the bars (drawn in the players' own colours), the red
// curtain between a match's screens (set 3)
inline constexpr uint32_t kShadowTiles[2] = {0xF8E0, 0xF920};
inline constexpr uint32_t kPanelTiles[2] = {0x13800, 0x14400};
inline constexpr uint32_t kCurtainTiles[2] = {0x2C600, 0x2FE00};

// The characters whose three other palettes go elsewhere when their effects need them, a bit by number (02ADB2)
inline constexpr uint32_t kFirstLineOnly = 0xEF9AEF96; // (Vampire Savior's, and Donovan's bit, 13, from Vampire Savior 2's)

// The ROM file whose colours PalMod changes, and which set it is
inline constexpr const char *kColoursFile = "vm3j.10b", *kColoursFileWhose = "the Japanese set's, 970519";

// What this game's own sources add: the characters taken in (src/graft.cpp)
void RegisterOwnSources();

// What a character put in by a mode in place of the one chosen needs besides its number (src/training.cpp): Phobos's
// and Donovan's form, +3C2, which the character choice sets from Start held as they are chosen (01F832 in Vampire Savior 2; here none: src/rules.cpp); the form
// chosen without it
inline void CharacterPut(uint32_t player, int character) {
    if (character == 0x10 || character == 0x13) {
        cps2recomp::SetRam8(player + 0x3C2, character == 0x13 ? 1 : 0);
    }
}


} // namespace vsav::family
