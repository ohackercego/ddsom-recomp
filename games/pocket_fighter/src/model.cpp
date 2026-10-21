// SPDX-License-Identifier: GPL-3.0-only
// Copyright (C) 2026 Tommy McLeroy
//
// Super Gem Fighter Mini Mix: the model viewer (the toolkit's model/model.h), a mode of the menu's "Alternative modes"
// page: one character alone, on a stage or on plain white or black, at any size, standing as it does in a fight (and
// moved with player 1's controls), or going through any one of its animations.
//
// It is a round of a two-player match, reached unseen as the stage viewer's is (fight/alternative.h), with the second player
// put out of the way: not followed by the camera, out of the picture, and never drawn.
//
// A character's animations are in three tables in the ROM, by the character's number (the game starts one at 010E04,
// 010DF4 or 010DFC): each a list of how far on from the list an animation is. An animation is frames of 0x18 bytes,
// as in Street Fighter Alpha 2: a word of how many frames of the game it is shown for (0FFF: for good, the end of a
// move), a byte whose top bit says the next is a jump (to the frame the long after this one names), where its picture
// is, and the numbers of its boxes. Showing one is putting its frame in the player's record each frame, as the game's
// own 010E24 does, with the time left kept high so that the game's own stepping (010EAC) never comes round; the game
// goes on believing the player stands.
#include "colours.h"
#include "model.h"
#include "alternative.h"
#include "pocket.h"
#include "session.h"
#include "state.h"

#include <iterator>

namespace cps2recomp {

namespace {
    using namespace pocket;

    constexpr const char *kModel = "Model viewer";
    constexpr int kStartMost = 9000; // frames after which getting to a round is given up

    // The characters in the menu's order, by their numbers in the game
    constexpr int kCharacterNumbers[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11};
    // The "Animation" choice: standing as in a fight, or one of the three tables (in the order they are in the ROM)
    constexpr uint32_t kAnimationTables[3] = {0x072B10, 0x072B44, 0x072B78};
    constexpr int kMostAnimations = 256;
    constexpr uint32_t kFrameBytes = 0x18, kFramePicture = 0x04, kFrameJump = 0x18, kFrameFollows = 0x02;
    constexpr uint32_t kFrameBoxes = 0x08, kRecordBoxes = 0x90; // (010E2C: the frame's box numbers to the record's)
    constexpr uint16_t kForGood = 0x0FFF;
    constexpr int kForGoodShown = 30; // frames a picture the game shows for good is shown before the animation starts again
    constexpr uint32_t kTimer = 0x18; // a player's record: frames the animation's frame is still shown for (a word)

    struct State {
        bool on = false;
        bool starting = false;
        int32_t frames = 0;
    } g;
    bool g_fast = false;

    void Fast(bool on) {
        if (on != g_fast) {
            g_fast = on;
            SetFastForward(on);
        }
    }
    void End() {
        if (g.on) {
            g = State();
            modes::Wanted() = MatchWanted();
            Fast(false);
            model::End();
        }
    }

    uint32_t Read32(Machine &machine, uint32_t address) {
        return (static_cast<uint32_t>(machine.Read16(address)) << 16) | machine.Read16(address + 2);
    }
    bool InRom(uint32_t address) {
        return address >= 0x100000 && address < 0x280000 && !(address & 1);
    }
    // Is this an animation's frame? (Its picture is somewhere in the ROM, and it is shown for a while.)
    bool IsFrame(Machine &machine, uint32_t frame) {
        return InRom(frame) && machine.Read16(frame) != 0 && InRom(Read32(machine, frame + kFramePicture));
    }
    // Where an animation of the character's starts (0: there is none of that number)
    uint32_t AnimationAt(Machine &machine, int character, int table, int number) {
        const uint32_t list = Read32(machine, kAnimationTables[table] + static_cast<uint32_t>(character) * 4);
        if (!InRom(list)) {
            return 0;
        }
        const uint32_t frame = list + machine.Read16(list + static_cast<uint32_t>(number) * 2);
        return IsFrame(machine, frame) ? frame : 0;
    }
    // Frames an animation's frame is shown for (the game's time for it, but not for good); and the frame after it
    // (0: the end, or the frame going on to itself, how the game holds an animation's last picture)
    int FrameTime(Machine &machine, uint32_t frame) {
        const uint16_t time = machine.Read16(frame);
        return time == kForGood ? kForGoodShown : time;
    }
    uint32_t NextFrame(Machine &machine, uint32_t frame) {
        if (machine.Read16(frame) == kForGood) {
            return 0;
        }
        const uint32_t next = machine.Read16(frame + kFrameFollows) & 0x8000 ? Read32(machine, frame + kFrameJump) : frame + kFrameBytes;
        return next == frame ? 0 : next;
    }
    void ShowFrame(Machine &machine, uint32_t frame) {
        if (!frame) {
            // (back to standing: the game's own "nothing" action from its start sets the animation again)
            SetRam16(Player(0) + kAction, 0);
            SetRam16(Player(0) + kTimer, 1);
            return;
        }
        SetRam32(Player(0) + kFrame, frame);
        SetRam32(Player(0) + kRecordBoxes, Read32(machine, frame + kFrameBoxes));
        SetRam16(Player(0) + kTimer, 0x7FFF);
    }

    // Objects by what they are: for the frame's own picture, the stage's alone (NotTheStage); for the character's,
    // player 1's own and what it throws; and for the picture as the board would show it, everything
    // but the panel and the second player
    unsigned g_firstSet = 0x10, g_secondSet = 0x15;
    bool IsModel(uint32_t tile, uint16_t attributes) {
        const unsigned set = attributes & 0x1F;
        // (not the shadows: both players' are in one set, and only the object list's places would tell them apart)
        return set >= g_firstSet && set < g_firstSet + kPalettesInSet && !ThePanel(tile, attributes);
    }
    bool InAFight(uint32_t tile, uint16_t attributes) {
        const unsigned set = attributes & 0x1F;
        return !(ThePanel(tile, attributes) || (set >= g_secondSet && set < g_secondSet + kPalettesInSet));
    }

    void StartModel(Machine &machine) {
        End();
        modes::End();
        EndBattle();
        SetFeature("training_mode", false);
        g.on = true;
        g.starting = true;
        model::Begin();
        modes::Wanted().stage = model::Plain(model::Background()) ? 1 : model::Background() + 1;
        modes::Wanted().characters[0] = kCharacterNumbers[model::Character()];
        modes::Wanted().characters[1] = kCharacterNumbers[(model::Character() + 1) % std::size(kCharacterNumbers)];
        machine.Reset();
    }

    // The second player out of the way: the camera does not follow it (the game's own flag), and it stands far off
    // to the right
    void PutAway() {
        SetRam8(Player(1) + kNotFollowed, 1);
        SetRam16(Player(1) + kX, static_cast<uint16_t>(Ram16(Player(0) + kX) + 0x300));
    }
} // namespace

bool ModelViewer() {
    return g.on;
}
void StartModelViewer(Machine &machine) {
    StartModel(machine);
}

void ModelBeforeFrame(Machine &machine) {
    if (!g.on) {
        return;
    }
    modes::Wanted().stage = model::Plain(model::Background()) ? 1 : model::Background() + 1;
    if (g.starting) {
        Fast(true);
        if (modes::ToRound(machine, g.frames)) {
            g.starting = false;
            Fast(false);
            // (the character in the middle of the stage)
            SetRam16(Player(0) + kX, static_cast<uint16_t>((kStageLeft + kStageRight) / 2));
        } else if (g.frames > kStartMost) {
            End();
        }
        return;
    }
    machine.players[1] = 0;
    PutAway();
    if (colours::EditorAnimation()) {
        model::Scan(machine);
    }
    colours::EditorBeforeFrame(machine); // (takes player 1's controls, when it is on)
    SetRam8(kRoundClock, 99);
    model::BeforeFrame(machine);
}

void ModelFrame(Machine &machine) {
    if (!g.on) {
        return;
    }
    // (the board reset, by the menu or by another mode starting: the viewer is over)
    if (!g.starting && Ram16(kMode) == 0) {
        End();
        return;
    }
    if (g.starting) {
        return;
    }
    // Another character or background chosen in the menu: the viewer starts again with it
    if (model::Changed()) {
        StartModel(machine);
        return;
    }
    machine.video.hideObject = NotTheStage;
    model::Frame(machine);
    colours::EditorFrame(machine);
}

// (Nothing of the viewer is kept in a save state: a state loaded while viewing ends the viewer, and what is loaded is
// the fight it holds.)
void ModelState(Archive &ar) {
    if (!ar.Saving() && !RunningAhead()) { // (not when run-ahead puts the frame back: runtime/game.h)
        End();
    }
}

void RegisterModel() {
    model::Description d;
    d.page = kModel;
    d.pageWords = "One character by itself, on a stage or on white or black, at any size, standing or going through any of its "
                  "animations.";
    d.resetWords = "Puts every choice of this page back: the first character and stage, its own size, standing, full speed, "
                   "repeating at once, one animation only. (While viewing, the viewer goes to that character and stage.)";
    d.animationWords = "What the character does: stands as in a fight, moved with player 1's controls, or goes through one of its "
                       "animations over and over. They are named by the game's three lists of them and a number (A34); the ones a "
                       "character has not got are passed over. While one is showing, left and right go to the one before and the "
                       "one after, and its name is shown.";
    d.foregroundWords = "What the stage has in front of the fighters: in front of the character too, as in a fight, or behind it so "
                        "that all of it is seen. In front only at the character's own size, and not in the colour editor.";
    d.characters = {std::begin(kCharacterNames), std::end(kCharacterNames)};
    d.numbers = {std::begin(kCharacterNumbers), std::end(kCharacterNumbers)};
    d.stages = {std::begin(kStageNames), std::end(kStageNames)};
    d.start = [](Machine &machine) {
        colours::EndEditor();
        StartModel(machine);
    };
    d.lists = 3;
    d.perList = kMostAnimations;
    d.animationAt = AnimationAt;
    d.isFrame = IsFrame;
    d.frameTime = FrameTime;
    d.nextFrame = NextFrame;
    d.showFrame = ShowFrame;
    d.animationChoice = colours::EditorAnimation;
    d.isModel = IsModel;
    d.inAFight = InAFight;
    d.beforeCompose = [](Machine &) {
        g_firstSet = Ram8(Player(0) + kColourSet) & 0x1F;
        g_secondSet = Ram8(Player(1) + kColourSet) & 0x1F;
    };
    d.anchor = [](Machine &, int &x, int &y) {
        x = static_cast<int16_t>(Ram16(Player(0) + kX)) - static_cast<int16_t>(Ram16(kCameraX));
        y = 239 - (static_cast<int16_t>(Ram16(Player(0) + kY)) - static_cast<int16_t>(Ram16(kCameraY)));
    };
    d.shift = colours::EditorShift;
    d.linesInSet = kPalettesInSet;
    d.firstLine = [](Machine &) { return static_cast<int>(Ram8(Player(0) + kColourSet) & 0x1F); };
    d.spareFrame = [](Machine &) { SetRam8(kRoundClock, 99); };
    model::Register(d);
}

} // namespace cps2recomp
