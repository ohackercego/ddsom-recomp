// SPDX-License-Identifier: GPL-3.0-only
// Copyright (C) 2026 Tommy McLeroy
//
// Vampire Savior: the model viewer (the toolkit's model/model.h), a mode of the menu's "Alternative modes" page:
// one character alone, on a stage or on plain white or black, at any size, standing as it does in a fight (and moved
// with player 1's controls), or going through any one of its animations.
//
// It is a round of a two-player match, reached unseen as the stage viewer's is (modes.h), with the second
// player put out of the way: not followed by the camera, out of the picture, and never drawn. The character's
// picture is its objects and its shadow; at its own size on a stage, what the stage has in front of the fighters
// (a table, a fence, a fire) is put back in front of it.
//
// A character's animations are in three tables in the ROM, by the character's number (the game starts one
// at 027EB8, 027EC0 or 027EC8): each a list of how far on from the table an animation is. An animation is
// frames of 0x18 bytes: how many frames of the game it is shown for, what follows it (the next, a jump to
// the frame a long names, or the end), and where its picture is. Showing one here is putting its frame in
// the player's record each frame, with the time left kept high so that the game's own stepping never
// comes round; the game goes on believing the player stands.
#include "colours.h"
#include "model.h"
#include "modes.h"
#include "state.h"
#include "widescreen.h"

#include <iterator>

namespace cps2recomp {

namespace {
    using namespace vsav;

    constexpr const char *kModel = "Model viewer";
    constexpr int kStartMost = 9000; // frames after which getting to a round is given up

    // The characters in the menu's order (family.h), by their numbers in the game
    consteval std::array<int, std::size(kRoster)> CharacterNumbers() {
        std::array<int, std::size(kRoster)> numbers{};
        for (size_t n = 0; n < std::size(kRoster); n++) {
            numbers[n] = kRoster[n].number;
        }
        return numbers;
    }
    constexpr std::array<int, std::size(kRoster)> kCharacterNumbers = CharacterNumbers();
    // The "Animation" choice: standing as in a fight, or one of the three tables
    constexpr uint32_t kAnimationTables[3] = {At(0x0BCEFA), At(0x0BCF7A), At(0x0BCE7A)};
    constexpr int kMostAnimations = 256;
    constexpr uint32_t kFrameBytes = 0x18, kFramePicture = 0x04, kFrameJump = 0x18;
    constexpr uint32_t kTimer = 0x20;     // a player's record: frames the animation's frame is still shown for
    constexpr uint32_t kActionStep = 0x06; //   the action and its own step (a word): nothing, from its start

    struct State {
        bool on = false;
        bool starting = false;
        int32_t frames = 0;
    } g;
    bool g_fast = false;
    bool g_noShadow = false; // the second player's shadow is not drawn (NoShadow)

    void Fast(bool on) {
        if (on != g_fast) {
            g_fast = on;
            SetFastForward(on);
        }
    }
    void End() {
        if (g.on) {
            g = State();
            Wanted() = MatchWanted();
            Fast(false);
            model::End();
        }
    }

    uint32_t Read32(Machine &machine, uint32_t address) {
        return (static_cast<uint32_t>(machine.Read16(address)) << 16) | machine.Read16(address + 2);
    }
    // An address the game keeps in its tables, as its code loads it (in a game that runs another set's code, an
    // address in that set's tables is moved: M68K::PointerLoad)
    uint32_t ReadAddress(Machine &machine, uint32_t address) {
        return machine.cpu.PointerLoad(address, Read32(machine, address));
    }
    // (the tables are in the ROM's second half on; or in the same part of another set's program, where a game has
    // one: game.cfg's extra_set)
    bool InExtra(uint32_t address) {
        return ExtraBase() && address >= ExtraBase() && InProgram(address);
    }
    bool InRom(uint32_t address) {
        const uint32_t own = InExtra(address) ? address - ExtraBase() : address;
        return own >= At(0x100000) && own < 0x400000 && !(address & 1);
    }
    // Is this an animation's frame? (Its picture is somewhere in the ROM, and it is shown for a while.) A frame of
    // another set's names its picture as that set does (where it is there)
    bool IsFrame(Machine &machine, uint32_t frame) {
        if (!InRom(frame) || machine.Read8(frame) == 0) {
            return false;
        }
        const uint32_t picture = Read32(machine, frame + kFramePicture);
        return InExtra(frame) ? InRom(ExtraBase() + picture) : InRom(picture);
    }
    // Where an animation of the character's starts (0: there is none of that number)
    uint32_t AnimationAt(Machine &machine, int character, int table, int number) {
        const uint32_t list = ReadAddress(machine, kAnimationTables[table] + static_cast<uint32_t>(character) * 4);
        if (!InRom(list)) {
            return 0;
        }
        const uint32_t frame = list + machine.Read16(list + static_cast<uint32_t>(number) * 2);
        return IsFrame(machine, frame) ? frame : 0;
    }
    // The frame after a frame, by what follows it: the next, a jump to the frame a long names, or the end (0)
    uint32_t NextFrame(Machine &machine, uint32_t frame) {
        const uint8_t follows = machine.Read8(frame + kFrameEnd);
        return follows & 0x80 ? ReadAddress(machine, frame + kFrameJump) : follows & 0x40 ? 0 : frame + kFrameBytes;
    }
    void ShowFrame(Machine &machine, uint32_t frame) {
        if (!frame) {
            // (back to standing: the game's own "nothing" action from its start sets the animation again)
            SetRam16(Player(0) + kActionStep, 0);
            SetRam8(Player(0) + kTimer, 1);
            return;
        }
        SetRam32(Player(0) + kFrame, frame);
        SetRam32(Player(0) + kTimer, Read32(machine, frame));
        SetRam8(Player(0) + kTimer, 0x7F);
    }

    // Objects by what they are: for the frame's own picture, the stage's alone (modes.h, NotTheStage); for the
    // character's, player 1's own and what it throws, and its shadow; and for the picture as the board would show
    // it, everything but the panel and the second player
    bool IsModel(uint32_t tile, uint16_t attributes) {
        if (IsShadow(tile, attributes)) {
            return true;
        }
        const unsigned set = attributes & 0x1F;
        return !(set < 5 || set > 15 || ThePanel(tile, attributes));
    }
    bool InAFight(uint32_t tile, uint16_t attributes) {
        const unsigned set = attributes & 0x1F;
        return !(ThePanel(tile, attributes) || ((kPlayerSets[1] >> set) & 1));
    }

    void StartModel(Machine &machine) {
        End();
        EndModes();
        EndBattle();
        SetFeature("training_mode", false);
        g.on = true;
        g.starting = true;
        model::Begin();
        Wanted().stage = model::Plain(model::Background()) ? 1 : model::Background() + 1;
        Wanted().characters[0] = kCharacterNumbers[model::Character()];
        Wanted().characters[1] = kCharacterNumbers[(model::Character() + 1) % std::size(kCharacterNumbers)];
        machine.Reset();
    }

    // The second player out of the way: the camera does not follow it (the game's own flag, which also
    // keeps it out of the hit tests and lets it leave the picture), and it stands far off to the right
    void PutAway() {
        SetRam8(Player(1) + kNotFollowed, 0x80);
        SetRam32(Player(1) + kX, (static_cast<uint32_t>(Ram16(Player(0) + kX)) + 0x300) << 16);
    }
} // namespace

void StartModelViewer(Machine &machine) {
    StartModel(machine);
}
// The character being shown, by its number in the game; and the number of a choice of "Character" (-1: none)
int ModelCharacter() {
    return model::CharacterNumber();
}
int ModelCharacterNumber(int choice) {
    return model::Number(choice);
}
int ModelAnimationStep(Machine &machine, int choice, int by) {
    return model::AnimationStep(machine, choice, by);
}
int ModelAnimationForLine(Machine &, int line) {
    return model::AnimationForLine(line);
}
const std::vector<const char *> &ModelAnimationNames() {
    return model::AnimationNames();
}

bool ModelViewer() {
    return g.on;
}
void EndModel() {
    End();
}

void ModelBeforeFrame(Machine &machine) {
    // (the second player stands 768 to the right, and the object list counts across in 1024: its shadow
    // would come round into the picture, on the floor, when the character walks to the right)
    const bool noShadow = g.on && !g.starting;
    if (noShadow != g_noShadow) {
        g_noShadow = noShadow;
        NoShadow(machine, noShadow ? 1 : -1);
    }
    if (!g.on) {
        return;
    }
    Wanted().stage = model::Plain(model::Background()) ? 1 : model::Background() + 1;
    if (g.starting) {
        Fast(true);
        if (ToRound(machine, g.frames)) {
            g.starting = false;
            Fast(false);
            // (the character in the middle of the stage)
            SetRam32(Player(0) + kX, static_cast<uint32_t>((kStageLeft + kStageRight) / 2) << 16);
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
    SetRam8(kClock, kClockFull);
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

// (Nothing of the viewer is kept in a save state, so that the states made before there was one still load:
// a state loaded while viewing ends the viewer, and what is loaded is the fight it holds.)
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
    d.foregroundWords = "What the stage has in front of the fighters (a table, a fence, a fire): in front of the character too, as in "
                        "a fight, or behind it so that all of it is seen. In front only at the character's own size, and not in the "
                        "colour editor.";
    d.characters = RosterChoices();
    d.numbers = {kCharacterNumbers.begin(), kCharacterNumbers.end()};
    d.stages = StageChoices({});
    d.start = [](Machine &machine) {
        colours::EndEditor();
        StartModel(machine);
    };
    d.lists = 3;
    d.perList = kMostAnimations;
    d.animationAt = AnimationAt;
    d.isFrame = IsFrame;
    d.frameTime = [](Machine &machine, uint32_t frame) { return static_cast<int>(machine.Read8(frame)); };
    d.nextFrame = NextFrame;
    d.showFrame = ShowFrame;
    d.animationChoice = colours::EditorAnimation;
    d.isModel = IsModel;
    d.inAFight = InAFight;
    d.anchor = [](Machine &, int &x, int &y) {
        x = static_cast<int16_t>(Ram16(Player(0) + kX)) - static_cast<int16_t>(Ram16(kCameraX)) + WideStage();
        y = 239 - (static_cast<int16_t>(Ram16(Player(0) + kY)) - static_cast<int16_t>(Ram16(kCameraY)));
    };
    d.shift = colours::EditorShift;
    d.linesInSet = kPalettesInSet;
    d.firstLine = [](Machine &) { return static_cast<int>(Ram8(Player(0) + kPaletteSlot)); };
    d.noScan = [](int character) { return ((kFirstLineOnly >> (character & 31)) & 1) != 0; };
    d.spareFrame = [](Machine &) { SetRam8(kClock, kClockFull); };
    model::Register(d);
}

} // namespace cps2recomp
