// SPDX-License-Identifier: GPL-3.0-only
// Copyright (C) 2026 Tommy McLeroy. Part of cps2recomp.
//
// The character viewer of a beat-'em-up's "Alternative modes" page (beat.h): the toolkit's model viewer
// (model/model.h) on a game begun unseen at the stage chosen (modes.cpp, as the stage viewer's are) with the character
// chosen, the enemies standing still and unseen. What the viewer needs of the game is in beat::Description.
#include "beat.h"

#include "model.h"

#include <iterator>

namespace cps2recomp::beat {

void BeginModel(Machine &machine, int stage); // modes.cpp

namespace {
    constexpr int kListsMost = 8, kAList = 128; // (the "Animation" choice: up to 8 lists of up to 128, named A0...)
    constexpr int kForGoodShown = 30;           // frames a frame shown for good is shown before the animation starts again

    Description g_d;
    std::vector<std::vector<uint32_t>> g_lists; // (the character's, as the game's sources found them)
    int g_listsOf = -1;

    uint32_t AnimationAt(Machine &, int character, int list, int index) {
        if (g_listsOf != character) {
            g_lists.clear();
            g_d.animations(character, g_lists);
            g_listsOf = character;
        }
        return static_cast<size_t>(list) < g_lists.size() && static_cast<size_t>(index) < g_lists[static_cast<size_t>(list)].size()
                   ? g_lists[static_cast<size_t>(list)][static_cast<size_t>(index)]
                   : 0;
    }
} // namespace

int ViewedCharacter() {
    return model::Character();
}

void StartModel(Machine &machine) {
    model::Begin();
    BeginModel(machine, model::Plain(model::Background()) ? 0 : model::Background());
}

void EndModel() {
    model::End();
    SetHud({});
}

void ModelBeforeFrame(Machine &machine) {
    for (int n = 1; n < Machine::kMaxPlayers; n++) {
        machine.players[n] = 0; // (the character alone: nobody joins)
    }
    model::BeforeFrame(machine);
}

void ModelFrame(Machine &machine) {
    // (another character or background chosen in the menu: the viewer begins again with it)
    if (model::Changed()) {
        StartModel(machine);
        return;
    }
    model::Frame(machine);
}

void RegisterModel(const Description &description) {
    g_d = description;
    if (description.characters.empty()) {
        return;
    }
    model::Description d;
    d.page = "Character viewer";
    d.pageWords = "One character by itself, on a stage or on white or black, at any size, moved with your controls or going "
                  "through any of its animations.";
    d.resetWords = "Puts every choice of this page back: the first character and stage, its own size, standing, full speed, "
                   "repeating at once, one animation only.";
    d.animationWords = "What the character does: stands, moved with player 1's controls, or goes through one of its animations over "
                       "and over. They are named by the game's lists of them and a number (A3); the ones a character has not got "
                       "are passed over. While one is showing, left and right go to the one before and the one after, and its "
                       "name is shown.";
    d.characters = description.characters;
    d.stages = description.stages;
    d.start = StartModel;
    d.lists = kListsMost;
    d.perList = kAList;
    d.animationAt = AnimationAt;
    d.frameTime = [](Machine &, uint32_t frame) {
        const int time = g_d.frameTime(frame);
        return time > 0 ? time : kForGoodShown;
    };
    // (a frame shown for good, or one going on to itself, ends the animation)
    d.nextFrame = [](Machine &, uint32_t frame) {
        const uint32_t next = g_d.frameTime(frame) == 0 ? 0 : g_d.nextFrame(frame);
        return next == frame ? 0 : next;
    };
    d.showFrame = [](Machine &, uint32_t frame) { g_d.showFrame(frame); };
    if (description.isFrame) {
        d.isFrame = [](Machine &, uint32_t frame) { return g_d.isFrame(frame); };
    }
    d.isModel = description.characterObject;
    d.anchor = [](Machine &, int &x, int &y) { g_d.standing(x, y); };
    d.anchorNow = true;
    d.largeLift = 48;
    model::Register(d);
}

} // namespace cps2recomp::beat
