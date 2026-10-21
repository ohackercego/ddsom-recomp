// SPDX-License-Identifier: GPL-3.0-only
// Copyright (C) 2026 Tommy McLeroy
//
// Super Gem Fighter Mini Mix: widescreen, as Street Fighter Alpha 2 and Vampire Savior have it. A fight is shown 512
// columns wide where the board shows 384 (16:9 for 4:3, the height and the size of everything as they were): more of
// the stage on each side. The fight is played in that picture too: the players can go to its edges, and the camera
// stops where its edges come to the stage's ends (64 nearer the middle at each end). With the feature off none of that
// is there and the game is the board's. It is off in every preset: it changes how the game plays.
//
// With "Invisible walls" (widescreen_walls) as well, the fight is shown in the wide picture but played as on the
// board: nothing here touches the rules (the margin is 0), so the players are stopped where the board's picture ends
// and the camera goes as far as the board's does. Only the picture is wider, and near a stage's end the view is kept
// inside the stage, so the board's picture is then off the middle of the wide one.
//
// The game's scroll screens are drawn whole when a stage begins (nothing is written to them while the camera moves:
// watched), so the picture needs nothing from here but its width and, near a stage's end, the view kept where it was 64
// from it; and the panel kept in the middle.
//
// The rules the wide picture changes are written again here as readable C++ (rewrites: lift.h), each with the margin
// as a line in it: the camera's place across and up (its ends 64 nearer the middle; a mode's wanted place: battle.h),
// and keeping the players inside the picture (64 further out on each side; the ring out lets one over an end).
#include "battle.h"
#include "alternative.h"
#include "pocket.h"

#include <algorithm>

namespace cps2recomp {

namespace {
    using namespace pocket;

    constexpr int kSide = Video::kMaxSide;
    // The camera's record (the stage's own scroll screen, FF8280), and in it
    constexpr uint32_t kCamera = 0xFF8280;
    constexpr uint32_t kCameraPlaceX = 0x20, kCameraShake = 0x50, kCameraBefore = 0x28, kCameraGoing = 0x3B, kCameraStepMost = 0x5C;
    constexpr uint32_t kCameraPlaceY = 0x24, kCameraShakeY = 0x54, kCameraBeforeY = 0x2C, kCameraGoingY = 0x3A, kCameraStepMostY = 0x5E;
    constexpr uint32_t kCameraHeld = 0x60; // byte: not zero, the camera stays where it is
    constexpr uint32_t kCameraEndTop = 0x40, kCameraEndBottom = 0x42, kCameraEndLeft = 0x44, kCameraEndRight = 0x46;
    constexpr uint32_t kPlaneShownX = 0x10; // where a scroll screen is shown across (what a player is kept inside)
    constexpr uint32_t kAtEdge = 0x114;     // a player's: 1 at the picture's left edge, 2 at its right
    constexpr uint32_t kOpponent = 0x34;    // a player's: the other's record (a word, FFxxxx)
    // Routines the rewrites call
    constexpr uint32_t kOuterEdges = 0x01FAF0; // D0: the players' left edge, D1 their right (the camera follows these)
    constexpr uint32_t kHeights = 0x01FB66;    // D1: the higher player's height (D0 the lower)
    constexpr uint32_t kPlaneOf = 0x011122;    // A3: the scroll screen the player A6 stands in

    bool g_play = false;  // fights are played in the wide picture
    bool g_shown = false; // fights are shown in the wide picture (played in it, or played as on the board)
    int g_side = 0, g_shift = 0, g_cameraBefore = 0;

    int Margin() {
        return g_play ? kSide : 0;
    }

    // --- the camera ---

    // The end of the camera's place across: its shake added, which way it went (0C right or staying, 08 left), kept
    // (01FAB8)
    void AcrossDone(M68K &c, uint32_t camera) {
        const uint16_t shown = static_cast<uint16_t>(c.d[0] + Ram16(camera + kCameraShake));
        SetLow16(c.d[0], shown);
        SetRam8(camera + kCameraGoing, static_cast<int16_t>(shown) < static_cast<int16_t>(Ram16(camera + kCameraBefore)) ? 0x08 : 0x0C);
        SetRam16(camera + kCameraPlaceX, shown);
    }
    // The camera stays where it is across (01F9FA): its place, then as above
    void AcrossStays(M68K &c, uint32_t camera) {
        SetLow16(c.d[0], Ram16(camera + kCameraPlaceX));
        AcrossDone(c, camera);
    }
    // The end of the camera's place up and down: its shake added, which way it went, kept (01FAD4)
    void UpDone(M68K &c, uint32_t camera) {
        const uint16_t shown = static_cast<uint16_t>(c.d[0] + Ram16(camera + kCameraShakeY));
        SetLow16(c.d[0], shown);
        SetRam8(camera + kCameraGoingY, static_cast<int16_t>(shown) < static_cast<int16_t>(Ram16(camera + kCameraBeforeY)) ? 0x04 : 0x00);
        SetRam16(camera + kCameraPlaceY, shown);
    }
    // The step's most (a word of the camera's): what is beyond it is cut to it, in D2
    uint16_t Step(M68K &c, uint16_t by, uint32_t most) {
        if (by > Ram16(most)) {
            SetLow16(c.d[2], Ram16(most));
        } else {
            SetLow16(c.d[2], by);
        }
        return static_cast<uint16_t>(c.d[2]);
    }
    // The camera held (the two players' bytes +133, the camera's own): D0 the three together
    bool Held(M68K &c, uint32_t camera) {
        const uint8_t held = Ram8(Player(0) + 0x133) | Ram8(Player(1) + 0x133) | Ram8(camera + kCameraHeld);
        c.d[0] = (c.d[0] & 0xFFFFFF00) | held;
        return held != 0;
    }

    void CameraUp(M68K &c);

    // 01F9C6. The camera's place across: it follows the players' middle when they are far apart (C0 or more), and
    // otherwise moves only when one of them comes near the picture's edge; at most the camera's +5C a frame, and not
    // past its ends (in the wide picture, 64 nearer the middle). Then its shake and which way it went. (Going left
    // within its ends, the game's code goes on into the place up and down, 01FA52, and leaves the shake out.)
    void CameraAcross(M68K &c) {
        const uint32_t camera = c.a[6] & 0xFFFFFF;
        if (Held(c, camera)) {
            AcrossStays(c, camera);
            return;
        }
        const int margin = Margin();
        if (modes::CameraAt().x >= 0) {
            // (a mode wants the camera at a place of its own: it goes there, as far as its ends let it)
            const int end0 = Ram16(camera + kCameraEndLeft) + margin, end1 = Ram16(camera + kCameraEndRight) - margin;
            SetLow16(c.d[0], static_cast<uint16_t>(std::clamp(modes::CameraAt().x, end0, end1)));
            SetLow16(c.d[2], 0);
            AcrossDone(c, camera);
            return;
        }
        Call(c, kOuterEdges);
        const uint16_t left = static_cast<uint16_t>(c.d[0]), right = static_cast<uint16_t>(c.d[1]);
        const uint16_t place = Ram16(camera + kCameraPlaceX);
        int16_t by = 0;
        int way = 0; // 1 right, -1 left, 0 stays
        SetLow16(c.d[2], static_cast<uint16_t>(right - left));
        if (static_cast<uint16_t>(right - left) >= 0xC0) {
            // (far apart: their middle in the middle)
            by = static_cast<int16_t>(static_cast<uint16_t>(((right + left) & 0xFFFF) >> 1) - place - 0xC0);
            way = by < 0 ? -1 : 1;
        } else if ((by = static_cast<int16_t>(right - place - 0x120)) >= 0) {
            way = 1;
        } else if ((by = static_cast<int16_t>(left - place - 0x60)) <= 0) {
            way = -1;
        }
        SetLow16(c.d[2], static_cast<uint16_t>(by));
        if (way == 0) {
            AcrossStays(c, camera);
            return;
        }
        if (way > 0) {
            const uint16_t moved = static_cast<uint16_t>(place + Step(c, static_cast<uint16_t>(by), camera + kCameraStepMost));
            SetRam16(camera + kCameraPlaceX, moved);
            // (the right end: in the wide picture 64 nearer the middle)
            const uint16_t end = static_cast<uint16_t>(Ram16(camera + kCameraEndRight) - margin);
            SetLow16(c.d[0], static_cast<int16_t>(end) <= static_cast<int16_t>(moved) ? end : moved);
            AcrossDone(c, camera);
            return;
        }
        const uint16_t moved = static_cast<uint16_t>(place - Step(c, static_cast<uint16_t>(-by), camera + kCameraStepMost));
        SetRam16(camera + kCameraPlaceX, moved);
        const uint16_t end = static_cast<uint16_t>(Ram16(camera + kCameraEndLeft) + margin);
        if (static_cast<int16_t>(end) >= static_cast<int16_t>(moved)) {
            SetLow16(c.d[0], end);
            AcrossDone(c, camera);
            return;
        }
        SetLow16(c.d[0], moved);
        CameraUp(c); // (the game's own code falls through into the place up and down here)
    }

    // 01FA52. The camera's place up and down: it follows the higher player when that one is more than 96 above its
    // bottom (or below), at most the camera's +5E a frame, and not past its ends; then its shake and which way it
    // went. (A mode can want it at a place of its own.)
    void CameraUp(M68K &c) {
        const uint32_t camera = c.a[6] & 0xFFFFFF;
        if (Held(c, camera)) {
            AcrossStays(c, camera); // (the game's code goes to the place across's end here: 01F9FA)
            return;
        }
        if (modes::CameraAt().y >= 0) {
            const CameraWanted &wanted = modes::CameraAt();
            SetLow16(c.d[0], static_cast<uint16_t>(wanted.beyondEnds ? wanted.y :
                                                   std::clamp<int>(wanted.y, Ram16(camera + kCameraEndBottom), Ram16(camera + kCameraEndTop))));
            SetLow16(c.d[2], 0);
            UpDone(c, camera);
            return;
        }
        Call(c, kHeights);
        const uint16_t high = static_cast<uint16_t>(c.d[1]);
        const uint16_t place = Ram16(camera + kCameraPlaceY);
        const int16_t by = static_cast<int16_t>(high - place - 0x60);
        SetLow16(c.d[2], static_cast<uint16_t>(by));
        if (by > 0) {
            const uint16_t moved = static_cast<uint16_t>(place + Step(c, static_cast<uint16_t>(by), camera + kCameraStepMostY));
            SetRam16(camera + kCameraPlaceY, moved);
            const uint16_t end = Ram16(camera + kCameraEndTop);
            SetLow16(c.d[0], static_cast<int16_t>(end) <= static_cast<int16_t>(moved) ? end : moved);
        } else if (by < 0) {
            const uint16_t moved = static_cast<uint16_t>(place - Step(c, static_cast<uint16_t>(-by), camera + kCameraStepMostY));
            SetRam16(camera + kCameraPlaceY, moved);
            const uint16_t end = Ram16(camera + kCameraEndBottom);
            SetLow16(c.d[0], static_cast<int16_t>(end) >= static_cast<int16_t>(moved) ? end : moved);
        } else {
            SetLow16(c.d[0], place);
        }
        UpDone(c, camera);
    }

    // --- the players inside the picture ---

    // 010EDA. The player at A6 kept inside the picture (in the wide picture, its edges); a player pressed against an
    // edge while reeling passes the push back to the other (if they are near enough).
    void KeepIn(M68K &c) {
        const uint32_t player = c.a[6] & 0xFFFFFF;
        if (Ram8(player + 0x132)) {
            return;
        }
        SetRam8(player + kAtEdge, 0);
        if (Ram8(player + 0x150)) {
            return;
        }
        // (the ring out: a player knocked about at an end of the stage goes over it)
        if (RingOutLetsOut(player)) {
            return;
        }
        Call(c, kPlaneOf);
        const uint32_t plane = c.a[3] & 0xFFFFFF;
        if (!Ram8(plane)) {
            return;
        }
        c.a[4] = static_cast<uint32_t>(static_cast<int16_t>(Ram16(player + kOpponent)));
        const uint32_t other = c.a[4] & 0xFFFFFF;
        SetLow16(c.d[6], 0x100);
        if (!(Ram16(player + kHealth) & 0x8000) && !(Ram16(other + kHealth) & 0x8000)) {
            SetLow16(c.d[6], 0x150);
        }
        uint16_t apart = static_cast<uint16_t>(Ram16(other + kX) - Ram16(player + kX));
        if (apart & 0x8000) {
            apart = static_cast<uint16_t>(0u - apart);
        }
        SetLow16(c.d[1], apart);
        const uint16_t half = Ram16(player + kHalfWidth);
        SetLow16(c.d[2], half);
        const uint16_t shown = static_cast<uint16_t>(Ram16(plane + kPlaneShownX) - Margin());
        const uint16_t x = Ram16(player + kX);
        int16_t over = static_cast<int16_t>(static_cast<uint16_t>(x - half - shown));
        if (over <= 0) {
            // (left: over is how far past, nothing or below)
            SetLow16(c.d[0], static_cast<uint16_t>(over));
            SetRam16(player + kX, static_cast<uint16_t>(x - over));
            SetRam8(player + kAtEdge, 1);
        } else {
            SetLow16(c.d[2], static_cast<uint16_t>(2 * half));
            over = static_cast<int16_t>(static_cast<uint16_t>(over + 2 * half - (0x180 + 2 * Margin())));
            SetLow16(c.d[0], static_cast<uint16_t>(over));
            if (over < 0) {
                return;
            }
            SetRam16(player + kX, static_cast<uint16_t>(x - over));
            SetRam8(player + kAtEdge, 2);
        }
        if (Ram16(player + kState) != 0x0202 || apart >= static_cast<uint16_t>(c.d[6]) || Ram8(player + 0x4F) || Ram8(player + 0x166) ||
            Ram8(other + 0x166)) {
            return;
        }
        if (!Ram8(other + 0x16C) && (Ram8(other + 0x4F) || (Ram8(player + 0xA0) & 0x80))) {
            return;
        }
        SetRam16(other + kX, static_cast<uint16_t>(Ram16(other + kX) - over));
    }

    // 010F88. Both players (one holding the other: a throw) kept inside the picture together
    void KeepBothIn(M68K &c) {
        const uint32_t player = c.a[6] & 0xFFFFFF;
        c.a[4] = static_cast<uint32_t>(static_cast<int16_t>(Ram16(player + kOpponent)));
        const uint32_t other = c.a[4] & 0xFFFFFF;
        SetRam8(other + kAtEdge, 0);
        if (Ram8(other + 0x150)) {
            return;
        }
        // (A0 the one on the left, A1 the one on the right)
        c.a[0] = c.a[4];
        c.a[1] = c.a[6];
        SetLow16(c.d[1], Ram16(player + kX));
        if (Ram16(player + kX) < Ram16(other + kX)) {
            c.a[0] = c.a[6];
            c.a[1] = c.a[4];
        }
        Call(c, kPlaneOf);
        const uint32_t leftOne = c.a[0] & 0xFFFFFF, rightOne = c.a[1] & 0xFFFFFF;
        const uint16_t shown = static_cast<uint16_t>(Ram16((c.a[3] & 0xFFFFFF) + kPlaneShownX) - Margin());
        uint16_t half = Ram16(leftOne + kHalfWidth);
        SetLow16(c.d[2], half);
        int16_t over = static_cast<int16_t>(static_cast<uint16_t>(Ram16(leftOne + kX) - half - shown));
        SetLow16(c.d[0], static_cast<uint16_t>(over));
        if (over <= 0) {
            SetRam16(player + kX, static_cast<uint16_t>(Ram16(player + kX) - over));
            SetRam16(other + kX, static_cast<uint16_t>(Ram16(other + kX) - over));
            SetRam8(other + kAtEdge, Ram8(other + kAtEdge) | 1);
        }
        half = Ram16(rightOne + kHalfWidth);
        SetLow16(c.d[2], half);
        over = static_cast<int16_t>(static_cast<uint16_t>(Ram16(rightOne + kX) - shown + half - (0x180 + 2 * Margin())));
        SetLow16(c.d[0], static_cast<uint16_t>(over));
        if (over < 0) {
            return;
        }
        SetRam16(player + kX, static_cast<uint16_t>(Ram16(player + kX) - over));
        SetRam16(other + kX, static_cast<uint16_t>(Ram16(other + kX) - over));
        SetRam8(other + kAtEdge, Ram8(other + kAtEdge) | 2);
    }

    // --- the picture ---

    int CameraLeftEnd() {
        return Ram16(kCamera + kCameraEndLeft);
    }
    int CameraRightEnd() {
        return Ram16(kCamera + kCameraEndRight);
    }
    bool InFight() {
        return Ram16(kMode) == 2 && Ram16(kGameStep) == kGameMatch && Ram16(kMatchStep) == kMatchRound;
    }
    bool OnStage(int camera) {
        return CameraRightEnd() >= CameraLeftEnd() + 2 * kSide && camera >= CameraLeftEnd() && camera <= CameraRightEnd();
    }

    // An object's place: the panel's stay in the middle, the stage's and the fighters' go with the view
    int MoveObject(uint32_t tile, uint16_t attributes, int) {
        return ThePanel(tile, attributes) ? 0 : g_shift;
    }

    // The wide picture's switches for the frame about to run (the port's, or a spare's): the camera's place now and
    // where it was for the picture being made (a frame behind)
    void SetWide(Machine &machine, int camera, int shown) {
        g_side = g_shift = 0;
        if (g_shown && OnStage(camera) && OnStage(shown)) {
            g_side = kSide;
            // (the view's left edge is 64 left of the camera, and no further left than the camera's own end: nearer
            // the end than that, the stage is drawn to the side by what is missing. Played in the wide picture the
            // camera never goes there; with invisible walls it does, and the board's picture is then to one side)
            g_shift = shown - std::clamp<int>(shown, CameraLeftEnd() + kSide, CameraRightEnd() - kSide);
        }
        Video::Wide &wide = machine.video.wide;
        wide.side = g_side;
        wide.scroll[0] = wide.scroll[1] = wide.scroll[2] = g_shift;
        wide.object = g_side ? MoveObject : nullptr;
        wide.tile = nullptr;
        wide.fillHoles = g_side != 0;
    }

    struct Kept {
        int side, shift;
    } g_kept;
} // namespace

bool WidePlay() {
    return g_play;
}
int WideMargin() {
    return Margin();
}
int WideSide() {
    return g_side;
}
int WideStage() {
    return g_side + g_shift;
}

void WidescreenBeforeFrame(Machine &machine) {
    const int camera = static_cast<int16_t>(Ram16(kCameraX)), shown = g_cameraBefore;
    g_cameraBefore = camera;
    // (fights only; the stage viewer keeps the board's picture. With invisible walls the rules stay the board's)
    g_shown = FeatureOn("widescreen") && InFight() && !modes::StageViewer();
    g_play = g_shown && !FeatureOn("widescreen_walls");
    SetWide(machine, camera, shown);
}

// A spare machine's frame (the modes' other machines) is drawn as wide as the port's picture is, so that the two can
// be put together: from the spare's own camera (where it is now, for want of where it was)
void WideForSpare(Machine &machine) {
    g_kept = {g_side, g_shift};
    const int camera = static_cast<int16_t>(Ram16(kCameraX));
    SetWide(machine, camera, camera);
    // (exactly as wide as the port's picture, even where the spare is not on its stage yet)
    if (g_side != g_kept.side) {
        Video::Wide &wide = machine.video.wide;
        g_side = wide.side = g_kept.side;
        g_shift = 0;
        wide.scroll[0] = wide.scroll[1] = wide.scroll[2] = 0;
        wide.object = g_side ? MoveObject : nullptr;
    }
}
void WideForPort(Machine &) {
    g_side = g_kept.side;
    g_shift = g_kept.shift;
}

void RegisterWidescreen() {
    TheGame().widestPicture = [] { return Video::kWidth + (FeatureOn("widescreen") ? 2 * kSide : 0); };
    AddRewrite(0x01F9C6, "CameraAcross", CameraAcross, kFreeFlags, 283);
    AddRewrite(0x01FA52, "CameraUp", CameraUp, kFreeFlags, 251);
    AddRewrite(0x010EDA, "KeepIn", KeepIn, kFreeFlags, 291);
    AddRewrite(0x010F88, "KeepBothIn", KeepBothIn, kFreeFlags, 229);
    // (not in the Enhanced preset: it shows more of the stage than the board does, and a player who wants the game as
    // it is played in competition must not get that without asking)
    TheGame().features.push_back({"widescreen", "Display", "Widescreen",
                                  "Fights are shown and played in 16:9, with more of the stage on each side: the players can go to "
                                  "the wide picture's edges, so they can stand further apart than on the board. Off unless switched "
                                  "on here, in every preset: it changes how the game plays.",
                                  kFeatureFirstPage});
    TheGame().features.push_back({"widescreen_walls", "Display", "Invisible walls",
                                  "With widescreen: fights are still shown in 16:9, but played exactly as on the board. The players "
                                  "are stopped where the board's picture ends, by walls that are not seen, and the camera goes as far "
                                  "as the board's; near a stage's end the view stays on the stage, so the play area is then to one "
                                  "side of the picture. Nothing without widescreen.",
                                  0});
}

} // namespace cps2recomp
