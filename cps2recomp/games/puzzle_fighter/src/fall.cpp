// SPDX-License-Identifier: GPL-3.0-only
// Copyright (C) 2026 Tommy McLeroy
//
// Super Puzzle Fighter II Turbo: gems falling, written again as C++ (rewrites: lift.h). A6 is the player's
// record.
#include "spf2.h"

#include <algorithm>

namespace cps2recomp {

namespace {
    using namespace spf2;

    constexpr uint32_t kBelow = kRowSize;
    // A big gem's cells have the gem's number in the high byte of their words in the second field
    uint8_t BigGemAt(uint32_t cell) {
        return Ram8(cell + kFieldExtra);
    }
    constexpr uint32_t kBiggestBroken = 0x32F; // byte: the most cells of one colour's big gems broken at once
    constexpr uint32_t kLandSound = 0x02BA3A;

    void MoveDown(uint32_t cell) {
        SetRam16(cell + kBelow, Ram16(cell));
        SetRam16(cell, 0);
        SetRam16(cell + kBelow + kFieldExtra, Ram16(cell + kFieldExtra));
        SetRam16(cell + kFieldExtra, 0);
    }
    // What the game sets when anything in the field has moved
    void NoteMoved(uint32_t player) {
        SetRam8(player + kNoteBusy, 1);
        SetRam8(player + kNoteChanged, 1);
        SetRam8(player + kNoteFalling, 1);
        SetRam8(player + kFell, 1);
        SetRam8(player + kNoteMoved, 1);
    }

    // 00D01C. Looks for the diamond in the field and notes what is under it (the diamond itself if it is on
    // the floor): that is the colour it will break.
    void FindDiamond(M68K &c) {
        const uint32_t player = c.a[6];
        for (int column = 0; column < kFieldColumns; column++) {
            for (int row = 0; row <= kFieldRows; row++) {
                const uint32_t cell = Cell(player, column, row);
                if (Ram16(cell) == kDiamond) {
                    const uint16_t under = Ram16(cell + kBelow);
                    SetRam16(player + kUnderDiamond, under == kWall ? kDiamond : under);
                    return;
                }
            }
        }
    }

    // A big gem's bottom left cell has nothing under it. If there is nothing under any of its bottom row,
    // the whole gem goes down a row. True if it did.
    bool BigGemFalls(uint32_t player, uint32_t cell) {
        const uint8_t gem = BigGemAt(cell);
        SetRam8(player + 0x240, BigGemAt(cell - 2));
        if (BigGemAt(cell - 2) == gem) {
            return false; // (not the gem's left edge)
        }
        int width = 0;
        do {
            width++;
            SetRam8(player + 0x240, BigGemAt(cell + 2u * width));
        } while (BigGemAt(cell + 2u * width) == gem);
        if (width == 1) {
            return false;
        }
        for (int n = 0; n < width; n++) {
            if (Ram16(cell + kBelow + 2u * n) != 0) {
                return false;
            }
        }
        for (int n = 0; n < width; n++) {
            uint32_t at = cell + 2u * n;
            do {
                MoveDown(at);
                at -= kRowSize;
                SetRam8(player + 0x240, BigGemAt(at));
            } while (BigGemAt(at) == gem);
        }
        NoteMoved(player);
        return true;
    }

    // 00CEC4. A frame of falling: in each column the lowest gem with nothing under it goes down a row (a big
    // gem only when all of it can). When nothing falls any more the turn's step moves on.
    void FallGems(M68K &c) {
        const uint32_t player = c.a[6];
        SetRam8(player + 0x1E4, 0);
        // The big gems broken just now: the most cells of one colour, if it is the player's most yet
        int8_t most = static_cast<int8_t>(Ram8(player + kBigGemCellsByColour));
        for (int colour = 1; colour < 4; colour++) {
            most = std::max(most, static_cast<int8_t>(Ram8(player + kBigGemCellsByColour + colour)));
        }
        if (most > static_cast<int8_t>(Ram8(player + kBiggestBroken))) {
            SetRam8(player + kBiggestBroken, static_cast<uint8_t>(most));
        }
        SetRam8(player + kBigGemCellsBroken, 0);
        SetRam32(player + kBigGemCellsByColour, 0);

        bool fell = false;
        for (int column = 0; column < kFieldColumns; column++) {
            bool solidBelow = true;
            for (int row = 0; row < kFieldRows; row++) {
                const uint32_t cell = Cell(player, column, row);
                const uint16_t value = Ram16(cell);
                if (value == 0) {
                    solidBelow = false;
                    continue;
                }
                bool moved = value == kWall;
                if (!moved) {
                    if (solidBelow) {
                        continue;
                    }
                    if ((value & kKind) != kCounter && (value >> 8) != 0) {
                        fell = BigGemFalls(player, cell) || fell;
                    }
                    if (Ram16(cell) & kCellBig) {
                        solidBelow = true; // (a big gem that stays is something to rest on)
                        continue;
                    }
                    // (after a big gem has gone down this is its empty corner: the game moves that too)
                    MoveDown(cell);
                }
                NoteMoved(player);
                SetRam8(player + kLanding, 1);
                SetRam8(player + kChainStarted, 0);
                fell = true;
                break;
            }
        }

        if (!fell) {
            if (Ram8(player + kLanding)) {
                SetRam8(player + kLanding, 2);
            }
            SetRam8(player + kStep, static_cast<uint8_t>(Ram8(player + kStep) + 1));
            if (Ram8(player + kLanding) == 2) {
                Call(c, kLandSound);
                SetRam8(player + kLanding, 0);
            }
            if (Ram8(player + kFell)) {
                // Something fell on the way here: fall again from the start, to be sure
                SetRam8(player + kFell, 0);
                SetRam8(player + kNoteBusy, 0);
                SetRam8(player + kNoteChanged, 1);
                SetRam8(player + kStep, kStepPairFalls);
                SetRam8(player + 0x275, 1);
                return;
            }
        }
        FindDiamond(c);
        if (Ram8(player + 0x275)) {
            SetRam8(player + 0x275, 0);
            SetRam8(player + kRedrawBigGems, 1);
        }
    }
} // namespace

void RegisterFall() {
    // (the last number is the time a call is charged: the average the check measured for the game's own code)
    constexpr uint32_t kScratch = kFreeD0 | kFreeD1 | kFreeD2 | kFreeD3 | kFreeD4 | kFreeD5 | kFreeD6 | kFreeA0 | kFreeA1 | kFreeA2 |
                                  kFreeA3 | kFreeFlags;
    AddRewrite(0x00D01C, "FindDiamond", FindDiamond, kScratch, 4160);
    AddRewrite(0x00CEC4, "FallGems", FallGems, kScratch, 8850);
}

} // namespace cps2recomp
