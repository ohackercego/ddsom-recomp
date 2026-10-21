// SPDX-License-Identifier: GPL-3.0-only
// Copyright (C) 2026 Tommy McLeroy
//
// Alien vs. Predator: the stage viewer (the toolkit's beat/beat.h). A stage is a scene of a round (avsp.h, kScene):
// the viewer begins a game at it, unseen, and then has the camera.
//
// The camera is a record at FF8268 (kCamera): its place across at +20, which 0B0680 moves after the players each
// frame (between the scene's limits at +34 and +36, which the scene's own code moves on as each wave of enemies is
// beaten) before it draws the scroll screens' new columns from 0B06B6 on. While viewing, the place is the viewer's
// and the drawing goes on from there. How far each scene's camera goes was found by playing it to its end (its
// right end is also the last right limit the scene's code sets); a scene whose camera moves by itself (round 3's
// ride) is left to the game. The enemies stand still while viewing (EnemyRuns), and nobody is drawn.
#include "avsp.h"
#include "beat.h"

#include <iterator>
#include <set>
#include <utility>

namespace cps2recomp {

namespace {
    using namespace avsp;

    constexpr uint32_t kCamera = kVars + 0x268, kCameraX = kCamera + 0x20;
    constexpr uint32_t kCameraFollows = kVars + 0x1AF; // byte: the camera may move (0: it stays, as in a scene's talk)

    struct Stage {
        const char *name;
        uint16_t scene;
        int left, right; // the camera's place across at the scene's ends (equal: the game moves it)
        bool ownCamera = true; // the scene's own camera routine runs (false: SceneCamera)
    };
    constexpr Stage kStages[] = {
        {"Round 1: City of Despair (1)", 1, 0x280, 0x480},
        {"Round 1: City of Despair (2)", 2, 0x300, 0x480},
        {"Round 2: War in the Underpass (1)", 3, 0x300, 0xB00},
        {"Round 2: War in the Underpass (2)", 4, 0x280, 0x400},
        {"Round 3: Forced Assault", 5, 0, 0}, // (a ride: the background scrolls behind the vehicle by itself)
        {"Round 4: Assault on a Queen (1)", 6, 0x300, 0xAD0},
        {"Round 4: Assault on a Queen (2)", 7, 0x100, 0xE30},
        {"Round 5: Secrets", 8, 0x100, 0xD80, false},
        {"Round 6: Nightmare in the Lab (1)", 9, 0x200, 0x580},
        {"Round 6: Nightmare in the Lab (2)", 10, 0x100, 0x580},
        {"Round 6: Nightmare in the Lab (3)", 11, 0x220, 0x8D0},
        {"Round 6: Nightmare in the Lab (4)", 12, 0x3F0, 0x880},
        {"Round 6: Nightmare in the Lab (5)", 13, 0, 0},
        {"Round 7: Hunt's End (1)", 14, 0x100, 0xE80},
        {"Round 7: Hunt's End (2)", 15, 0x500, 0xA80},
        {"Round 7: Hunt's End (3)", 16, 0x200, 0xB80},
        {"Round 7: Hunt's End (4)", 17, 0x200, 0x580},
        {"Round 7: Hunt's End (5)", 18, 0x200, 0x480},
    };
    constexpr int kBootFrames = 420;
    constexpr int kMiddle = 0x80; // (the camera's place is the picture's left edge and 0x40: its middle is 0x80 on)

    int32_t g_x = -1;      // where the viewer has the camera (-1: not viewing)
    int32_t g_inScene = 0; // frames the game has been at the wanted scene

    // 0B0680: the camera after the players, while viewing: put where the viewer has it, and drawn from there
    bool CameraFollows(M68K &c) {
        if (g_x < 0 || !beat::StageViewer()) {
            return false;
        }
        SetRam16(c.a[6] + 0x20, static_cast<uint16_t>(g_x));
        c.pc = 0x0B06B6;
        return true;
    }

    // The enemies come as the scene has them, but stand still where they come, unseen: the two loops that run their
    // records (01B922: the four from FF9280; 01B980: the twenty from FF9580, 0xC0 bytes each) call each one's routine
    // by its kind (+2), and while viewing go on to the next record instead. (Not taking them from their pool at all
    // is not safe: some of the scenes' code does not look whether it got one.)
    bool EnemyRuns(M68K &c) {
        if (!beat::Viewing() || beat::Starting()) {
            return false;
        }
        c.pc = c.pc == 0x01B93C ? 0x01B94E : 0x01B9AC;
        return true;
    }

    // 0AF8B4: the scene's own camera routine (by scene, from the table at 0AF8D4), called by the camera's task
    // (0AF858) each frame. For a scene that waits on something before its camera follows (round 5's start waits on
    // its first enemies, who never come), once the routine has set the camera up (+5, its step, past 0): the one
    // every scene uses while it follows the players instead, across (0B0680, the viewer's place) and up and down
    // (0B0786).
    bool SceneCamera(M68K &c) {
        if (g_x < 0 || !beat::StageViewer() || kStages[beat::ViewedStage()].ownCamera || Ram8(kCamera + 5) == 0) {
            return false;
        }
        c.pc = 0x0B046A;
        return true;
    }

    bool ToStage(Machine &machine, int stage, int32_t &frames) {
        const uint16_t scene = kStages[stage].scene;
        if (Ram16(kMode) != kModeGame) {
            g_inScene = 0;
            if (machine.frame > kBootFrames && Ram8(kCredits) == 0 && frames < 2000) {
                SetRam8(kCredits, 1);
            }
            machine.players[0] = (frames & 4) ? kStart : 0;
            return false;
        }
        if (Ram8(Player(0) + kInPlay) != 1) {
            machine.players[0] = (frames & 4) ? kPunch1 : 0; // (the character choice: the first offered)
            return false;
        }
        if (Ram16(kScene) != scene * 2) {
            g_inScene = 0;
            return false;
        }
        // (there, once the camera may move: or a while after, for a scene whose camera is the game's)
        return (Ram8(kCameraFollows) && Ram16(kCameraX) != 0) || ++g_inScene > 300;
    }

    void View(Machine &, int, int x, int) {
        g_x = x;
        if (x >= 0) {
            SetRam8(kCameraFollows, 1);
        }
        // (the players out of harm, and in the middle of the picture, where the game's camera, which follows them
        // up and down, looks for them; and the clock still)
        for (int n = 0; n < kPlayers; n++) {
            const uint32_t p = Player(n);
            if (Ram8(p + kInPlay) == 1 && Ram8(p + kHealth) != 0xFF) {
                SetRam8(p + kHealth, Ram8(p + kHealthMax));
                if (x >= 0) {
                    SetRam16(p + kX, static_cast<uint16_t>(x + kMiddle));
                }
            }
        }
        SetRam8(kClockFrames, 0x3C);
    }
} // namespace

// --- the character viewer ---
namespace {
    // The characters, by their numbers in the game (a player's record's +D2), and their animations: each character's
    // are four lists of where each begins (word offsets from the list), one after another, each as long as what
    // begins there is an animation (IsAnimation; the last no further than where the first's first offset points). Three lists are taken from tables
    // by the character's number (01B0E0, 01B104, 021FFA), the fourth in each character's own code (as 021E10 does).
    constexpr const char *kCharacters[] = {"Predator Warrior", "Maj. Dutch Schaefer", "Predator Hunter", "Lt. Linn Kurosawa"};
    constexpr uint32_t kLists[4][4] = {
        {0x15E240, 0x15E2EA, 0x15E32C, 0x15E36E},
        {0x162F90, 0x163032, 0x163074, 0x1630F6},
        {0x168564, 0x16860E, 0x168650, 0x168692},
        {0x16D122, 0x16D1C4, 0x16D206, 0x16D23A},
    };
    // A frame of an animation (as 016518 and 016536 step them): 16 bytes, a word of how many frames it is shown for
    // (0FFF: for good), a word whose top bit says the next sixteen bytes are where it goes on (a long), and the rest
    constexpr uint32_t kFrameBytes = 0x10;
    constexpr uint16_t kForGood = 0x0FFF;
    // A player's record: the frame shown (+1C), its time left (+0A, a word) and its second word (+24)
    constexpr uint32_t kFrame = 0x1C, kFrameTime = 0x0A, kFrameFlags = 0x24;

    uint16_t Word(uint32_t address) {
        const uint8_t *p = Program(address);
        return static_cast<uint16_t>((p[0] << 8) | p[1]);
    }
    uint32_t Long(uint32_t address) {
        return (static_cast<uint32_t>(Word(address)) << 16) | Word(address + 2);
    }
    // A frame is shown for a while and its picture is in the program's second half; an animation is one whose frames
    // all are (a list's last offsets are not always animations: the Predators' last list runs on into other data, and
    // forcing a frame that is not one on a player crashes the game)
    bool IsFrame(uint32_t frame) {
        if ((frame & 1) || frame < 0x100000 || frame >= 0x200000 - kFrameBytes) {
            return false;
        }
        const uint32_t picture = Long(frame + 4);
        return Word(frame) != 0 && !(picture & 1) && picture >= 0x100000 && picture < 0x200000;
    }
    uint32_t NextFrame(uint32_t frame) {
        return Word(frame + 2) & 0x8000 ? Long(frame + kFrameBytes) : frame + kFrameBytes;
    }
    bool IsAnimation(uint32_t frame) {
        for (int n = 0; n < 64; n++) {
            if (!IsFrame(frame)) {
                return false;
            }
            const uint32_t next = NextFrame(frame);
            if (Word(frame) == kForGood || next <= frame) {
                return true;
            }
            frame = next;
        }
        return true;
    }

    // The character's own objects: the entries the drawing of its record writes into the object list (0A9B48 takes the
    // next record to draw, from the list A3 is at; 0A9B62 is after it and what goes with it are drawn; A1 is where the
    // entries go), told by their tiles and attributes, of this frame and the one before (the list is shown a frame
    // after it is written)
    std::set<std::pair<uint16_t, uint16_t>> g_drawn[2];
    uint32_t g_drawFrom = 0;
    bool DrawNext(M68K &c) {
        g_drawFrom = beat::CharacterViewer() && (Ram16(c.a[3]) | 0xFF0000u) == Player(0) ? c.a[1] : 0;
        return false;
    }
    bool DrawDone(M68K &c) {
        if (g_drawFrom && (c.a[6] & 0xFFFFFF) == Player(0)) {
            for (uint32_t at = g_drawFrom; at < c.a[1] && at < g_drawFrom + 0x200; at += 8) {
                g_drawn[0].insert({c.bus.Read16(at + 4), c.bus.Read16(at + 6)});
            }
        }
        g_drawFrom = 0;
        return false;
    }

    void ShowFrame(uint32_t frame) {
        const uint32_t p = Player(0);
        const int character = beat::ViewedCharacter();
        if (!frame) {
            // (back to standing: the first animation of its first list, which the game goes on from)
            frame = kLists[character][0] + Word(kLists[character][0]);
            SetRam32(p + kFrame, frame);
            SetRam16(p + kFrameTime, Word(frame) == kForGood ? 1 : Word(frame));
            SetRam16(p + kFrameFlags, Word(frame + 2));
            return;
        }
        // (the time left kept high, so that the game's own stepping never comes round)
        SetRam32(p + kFrame, frame);
        SetRam16(p + kFrameTime, 0x7FFF);
        SetRam16(p + kFrameFlags, Word(frame + 2));
    }
} // namespace

// The scene a game the viewers begin starts at (-1: not the viewers'), and the character it is played with
int ViewerScene() {
    return beat::Viewing() ? kStages[beat::ViewedStage()].scene : -1;
}
int ViewerCharacter() {
    return beat::CharacterViewer() ? beat::ViewedCharacter() : -1;
}

void ViewerFrame(Machine &) {
    if (!beat::StageViewer() || beat::Starting()) {
        g_x = -1;
    }
    g_drawn[1] = std::move(g_drawn[0]);
    g_drawn[0].clear();
}

void DescribeViewer(beat::Description &d) {
    for (const Stage &stage : kStages) {
        d.stages.push_back(stage.name);
    }
    d.toStage = ToStage;
    d.stagePath = [](int stage, std::vector<beat::Description::Place> &path) {
        if (kStages[stage].left != kStages[stage].right) {
            path = {{kStages[stage].left, -1}, {kStages[stage].right, -1}};
        }
    };
    d.stageCamera = [](int &x, int &y) {
        x = Ram16(kCameraX);
        y = -1;
    };
    d.view = View;
    d.viewLayers = 0xD; // (not the first scroll screen: the panel)
    for (const char *character : kCharacters) {
        d.characters.push_back(character);
    }
    d.animations = [](int character, std::vector<std::vector<uint32_t>> &lists) {
        const uint32_t end = kLists[character][0] + Word(kLists[character][0]);
        for (int n = 0; n < 4; n++) {
            const uint32_t base = kLists[character][n], next = n < 3 ? kLists[character][n + 1] : end;
            std::vector<uint32_t> list;
            for (uint32_t at = base; at < next && IsAnimation(base + Word(at)); at += 2) {
                list.push_back(base + Word(at));
            }
            lists.push_back(std::move(list));
        }
    };
    d.frameTime = [](uint32_t frame) { return Word(frame) == kForGood ? 0 : std::max<int>(Word(frame), 1); };
    d.nextFrame = NextFrame;
    d.isFrame = IsFrame;
    d.showFrame = ShowFrame;
    d.characterObject = [](uint32_t tile, uint16_t attributes) {
        const std::pair<uint16_t, uint16_t> entry = {static_cast<uint16_t>(tile), attributes};
        return g_drawn[0].count(entry) + g_drawn[1].count(entry) > 0;
    };
    d.standing = [](int &x, int &y) {
        x = static_cast<int16_t>(Ram16(Player(0) + kX) - Ram16(kCameraX) + 0x40);
        y = -1;
    };
    AddHook(0x0A9B48, "DrawNext", DrawNext);
    AddHook(0x0A9B62, "DrawDone", DrawDone);
    AddHook(0x0AF8B4, "SceneCamera", SceneCamera);
    AddHook(0x0B0680, "CameraFollows", CameraFollows);
    AddHook(0x01B93C, "EnemyRuns", EnemyRuns);
    AddHook(0x01B99A, "EnemyRunsToo", EnemyRuns);
}

} // namespace cps2recomp
