// SPDX-License-Identifier: GPL-3.0-only
// Copyright (C) 2026 Tommy McLeroy
//
// Vampire Savior Definitive: the characters taken in at the character choice. The choice's grid is Vampire
// Savior's, a place for each of its characters and one (0B) for a random choice; the three taken in have places of
// their own, a row under it: Donovan's, Pyron's and Phobos's. A place's number is its character's (the game's code
// makes the place chosen the character: +03 in the player's record, the place, to +382), so theirs are 13, 11 and
// 10, and what is read by character (the picture, the figure and the name beside the grid) needs nothing more.
// What reads the place is here: the cursor's moves, its ring, and the grid's picture, which gets their faces.
#include "definitive.h"
#include "vsav.h"

namespace cps2recomp {

namespace {
    using namespace vsav;
    using namespace definitive;

    constexpr uint32_t kPlace = 0x03, kNumber = 0x381; // (the player's record)
    constexpr uint8_t kRandom = 0x0B, kBishamon = 0x08, kAulbath = 0x09;

    // Their places, left to right under the random choice's, 44 pixels apart and 34 lower; their faces (Vampire
    // Savior 2's, from its graphics: a tile, its size and a palette the screen leaves free) and where each is put in
    // its place, and its colours in Vampire Savior 2's screen
    struct Place {
        uint8_t character;
        int16_t x;                 // its middle, from the random choice's
        int16_t ringX;             // the ring's, from the random choice's (on the face as drawn)
        uint16_t tile, attributes; // the face
        int16_t faceX, faceY;      // ... its object, from the place's middle
        uint8_t theirPalette;
    };
    constexpr Place kPlaces[] = {
        {kDonovan, -44, -44, 0xB10B, 0x1216, -0x12, -0x11, 0x05},
        {kPyron, 0, -1, 0xB0F5, 0x1217, -0x18, -0x11, 0x11},
        {kPhobos, 44, 40, 0xB108, 0x1219, -0x20, -0x10, 0x13},
    };
    constexpr int kBelow = 34, kRingBelow = 36;
    int PlaceIndex(uint8_t place) {
        for (int i = 0; i < 3; i++) {
            if (kPlaces[i].character == place) {
                return i;
            }
        }
        return -1;
    }

    // The cursor's moves (020A68: the directions newly pressed made a number, D1, by the table at 211D4: right, left,
    // down, up, down right, down left, up right, up left; a place's eight next are at 211E4): from theirs, and down
    // from the three above them
    enum { kRight, kLeft, kDown, kUp, kDownRight, kDownLeft, kUpRight, kUpLeft };
    int NextPlace(uint8_t place, int direction) {
        const int i = PlaceIndex(place);
        if (i >= 0) {
            static constexpr uint8_t kAbove[3][3] = {
                // (up left, up, up right)
                {kBishamon, kBishamon, kRandom},
                {kBishamon, kRandom, kAulbath},
                {kRandom, kAulbath, kAulbath},
            };
            switch (direction) {
            case kRight:
            case kDownRight: return kPlaces[(i + 1) % 3].character;
            case kLeft:
            case kDownLeft: return kPlaces[(i + 2) % 3].character;
            case kUpLeft: return kAbove[i][0];
            case kUp: return kAbove[i][1];
            case kUpRight: return kAbove[i][2];
            default: return place;
            }
        }
        const bool down = direction == kDown || direction == kDownLeft || direction == kDownRight;
        if (!down || (place != kBishamon && place != kRandom && place != kAulbath)) {
            return -1; // (the game's own table)
        }
        const int column = place == kBishamon ? 0 : place == kRandom ? 1 : 2;
        const int to = column + (direction == kDownLeft ? -1 : direction == kDownRight ? 1 : 0);
        return kPlaces[to < 0 ? 0 : to > 2 ? 2 : to].character;
    }
    bool CursorMoves(M68K &c) {
        const int next = NextPlace(Ram8((c.a[6] & 0xFFFFFF) + kPlace), c.d[1] & 7);
        if (next < 0) {
            return false;
        }
        SetLow8(c.d[0], static_cast<uint8_t>(next));
        c.pc = At(0x020A7C); // (on to the place and the character set from D0)
        return true;
    }

    // The cursor's ring (an object, A6; its player's record, A3): where it is by the place (05FAD0, D6, in a table of
    // its own, at 05FAE2, whose y is up) and its picture by the place (05FA96, from 2671C6). At theirs it is the
    // random choice's ring, moved
    constexpr uint32_t kRingPlaces = 0x05FAE2, kRingPictures = 0x2671C6;
    bool RingPlace(M68K &c) {
        const int i = PlaceIndex(c.d[6] & 0xFF);
        if (i < 0) {
            return false;
        }
        const uint8_t *random = ProgramOpcodes(kRingPlaces + kRandom * 4);
        const uint32_t me = c.a[6] & 0xFFFFFF;
        SetRam16(me + 0x10, static_cast<uint16_t>(((random[0] << 8) | random[1]) + kPlaces[i].ringX));
        SetRam16(me + 0x14, static_cast<uint16_t>(((random[2] << 8) | random[3]) - kRingBelow));
        c.cycles += 40;
        c.pc = c.Pop32();
        return true;
    }
    bool RingPicture(M68K &c) {
        if (PlaceIndex(c.d[6] & 0xFF) < 0) {
            return false;
        }
        uint32_t number = kRandom;
        if (Ram8((c.a[3] & 0xFFFFFF) + kNumber)) {
            number += 0x20;
        } else if (Ram8(0xFF8403) == Ram8(0xFF8803) && (Ram8(0xFF808D) & 2)) { // (both players at one place)
            number += 0x40;
        }
        const uint8_t *offset = Program(kRingPictures + 2);
        SetRam32((c.a[6] & 0xFFFFFF) + 0x1C, kRingPictures + ((offset[0] << 8) | offset[1]) + number * 4 - 4);
        c.cycles += 60;
        c.pc = c.Pop32();
        return true;
    }

    // Their faces' colours, to palettes the screen leaves free (the figure beside the grid may change its own
    // palettes and the next with them, every frame: these are put after, with the faces), once the screen has
    // faded in (a face's palette, Demitri's, as the screen's own are: at 3A3800; theirs at 3BAF3C there)
    constexpr uint32_t kOurGridPalettes = 0x3A3800, kTheirGridPalettes = 0x3BAF3C, kPalettes = 0x90C000;
    void TheirColours(M68K &c) {
        const uint8_t *demitri = Program(kOurGridPalettes + 0x06 * 32 + 2);
        if (c.bus.Read16(kPalettes + 0x06 * 32 + 2) != (0xF000 | (demitri[0] << 8) | demitri[1])) {
            return;
        }
        for (const Place &place : kPlaces) {
            const uint8_t *from = ExtraProgram(kTheirGridPalettes + place.theirPalette * 32);
            const uint32_t to = kPalettes + (place.attributes & 0x1F) * 32;
            for (uint32_t i = 0; i < 16; i++) {
                c.bus.Write16(to + i * 2, static_cast<uint16_t>(0xF000 | (from[i * 2] << 8) | from[i * 2 + 1]));
            }
        }
    }

    // The grid (the object at FFB800, its picture 2689FA's, all the faces), and their faces after it: put at the end
    // of its drawing (01B2CA, the loop's rts: A2 where the next object goes, D7 how many more the frame has room for),
    // each drawn from Vampire Savior 2's graphics (an object is, when that program was read just before it:
    // Machine::ObjectWritten). The places below are where the grid has its faces with its object at C0, 50.
    constexpr uint32_t kGrid = 0xFFB800, kGridPicture = 0x2689FA;
    constexpr int kRandomMiddleX = 0xFF, kRandomMiddleY = 0xA6, kGridX = 0xC0, kGridY = 0x50;
    bool TheirFaces(M68K &c) {
        const uint32_t grid = c.a[6] & 0xFFFFFF;
        if (grid != kGrid || Ram32(grid + 0x1C) != kGridPicture || !ExtraBase() || (c.d[7] & 0xFFFF) < 3) {
            return false;
        }
        const int dx = static_cast<int16_t>(Ram16(grid + 0x10)) - kGridX;
        const int dy = static_cast<int16_t>(Ram16(grid + 0x14)) - kGridY;
        for (const Place &place : kPlaces) {
            const int x = kRandomMiddleX + place.x + place.faceX + dx;
            const int y = kRandomMiddleY + kBelow + place.faceY + dy;
            c.bus.Read16(ExtraBase());
            c.bus.Write16(c.a[2], static_cast<uint16_t>((x & 0x3FF) | Ram16(grid + 0x1A)));
            c.bus.Write16(c.a[2] + 2, static_cast<uint16_t>((y & 0x3FF) | Ram16(grid + 0x18)));
            c.bus.Write16(c.a[2] + 4, place.tile);
            c.bus.Write16(c.a[2] + 6, place.attributes);
            c.a[2] += 8;
        }
        SetLow16(c.d[7], (c.d[7] - 3) & 0xFFFF);
        TheirColours(c);
        return false;
    }

} // namespace

void definitive::RegisterChoosing() {
    AddHook(At(0x020A68), "CursorMoves", CursorMoves);
    AddHook(At(0x05FAD0), "RingPlace", RingPlace);
    AddHook(At(0x05FA96), "RingPicture", RingPicture);
    AddHook(At(0x01B2CA), "TheirFaces", TheirFaces);
}

} // namespace cps2recomp
