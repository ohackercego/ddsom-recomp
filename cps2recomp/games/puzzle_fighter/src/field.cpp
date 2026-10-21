// SPDX-License-Identifier: GPL-3.0-only
// Copyright (C) 2026 Tommy McLeroy
//
// Super Puzzle Fighter II Turbo: the rules of a player's field of gems, written again as C++ (rewrites:
// lift.h). In all of these A6 is the player's record.
#include "spf2.h"

namespace cps2recomp {

namespace {
    using namespace spf2;

    // The cell's two bytes: what it is and its marks (low), and a counter gem's colour and count (high)
    constexpr uint32_t kLow = 1, kHigh = 0;
    // From a cell to the ones beside it
    constexpr int32_t kLeft = -2, kRight = 2, kBelow = 16, kAbove = -16;

    // The game's loops over the field leave their counters and addresses in registers, and some callers go on
    // to use them: a word counter that has run out, and where the two addresses end up.
    constexpr uint32_t kRanOut = 0x0000FFFF;
    void LeaveLoopAddresses(M68K &c, uint32_t player, int rows) {
        const uint32_t bottom = Ram32(player + kFieldBottom) + kBottomRow + 2;
        c.a[0] = bottom + 2 * kFieldColumns;
        c.a[1] = bottom + 2 * (kFieldColumns - 1) - static_cast<uint32_t>(rows) * kRowSize;
    }

    // 00ECAA. Tidies the field: a cell with nothing in it has no marks left and belongs to no big gem, and
    // neither does a counter gem.
    void TidyField(M68K &c) {
        const uint32_t player = c.a[6];
        for (int column = 0; column < kFieldColumns; column++) {
            for (int row = 0; row < kFieldRows - 1; row++) {
                const uint32_t cell = Cell(player, column, row);
                if (Ram8(cell + kLow) == 0) {
                    SetRam16(cell, 0);
                    SetRam16(cell + kFieldExtra, 0);
                }
                if (Ram8(cell + kLow) == kCounter) {
                    SetRam16(cell + kFieldExtra, 0);
                }
            }
        }
        c.d[0] = c.d[1] = kRanOut;
        LeaveLoopAddresses(c, player, kFieldRows - 1);
    }

    // 00D76E. Counts the gems in the field, for how much danger the player is in; clears away what the
    // last break left; and, if the field is empty, gives the reward for clearing it.
    void CountField(M68K &c) {
        const uint32_t player = c.a[6];
        uint16_t gems = 0;
        bool any = false;
        for (int column = 0; column < kFieldColumns; column++) {
            for (int row = 0; row < kFieldRows - 1; row++) {
                const uint32_t cell = Cell(player, column, row);
                const uint16_t value = Ram16(cell);
                if (value == 0) {
                    continue;
                }
                if (Ram8(cell + kLow) == kCounterGone) {
                    SetRam16(cell, 0);
                    continue;
                }
                any = true;
                if (value == kWall) {
                    continue;
                }
                gems++;
                if (!(value & kCellBig)) {
                    SetRam16(cell + kFieldExtra, 0);
                }
            }
        }
        SetRam16(player + kGemCount, gems);
        SetRam8(player + kDanger, gems < kDangerSome ? 0 : gems < kDangerMany ? 1 : 2);

        if (!any && !Ram8(Var(c, kNoAllClear)) && !Ram8(player + kAllClearGiven)) {
            SetRam8(player + kAllClearGiven, 1);
            SetRam32(player + kAllClearScore, Ram32(player + kAllClearScore) + kAllClearPoints);
            SetRam8(player + kAllClearAttack, static_cast<uint8_t>(Ram8(player + kAllClearAttack) + kAllClearGems));
            SetRam32(player + kScore, Ram32(player + kScore) + Ram32(player + kAllClearScore));
            SetRam8(player + kAttack, static_cast<uint8_t>(Ram8(player + kAttack) + Ram8(player + kAllClearAttack)));
            Call(c, kShowAllClear);
        }
    }

    // 00D8BC. A turn has passed: every counter gem's count goes down by one. Those that reach nothing start
    // to turn into ordinary gems (TurnCounterGems).
    void CountDownCounterGems(M68K &c) {
        const uint32_t player = c.a[6];
        bool turning = false;
        for (int column = 0; column < kFieldColumns; column++) {
            for (int row = 0; row < kFieldRows; row++) {
                const uint32_t cell = Cell(player, column, row);
                const uint8_t high = Ram8(cell + kHigh);
                if ((Ram16(cell) & kKind) != kCounter || high == 0) {
                    continue;
                }
                uint8_t count = high & 0x0F;
                if (count != 0) {
                    count--;
                }
                if (count == 0) {
                    SetRam8(player + kTurnTimer, kTurnTime);
                    turning = true;
                }
                SetRam8(cell + kHigh, static_cast<uint8_t>((high & 0xF0) | count));
            }
        }
        if (turning) {
            Call(c, kTurnSound);
        }
    }

    // 00D824. Counter gems whose count has run out: while the timer runs they flash (shown as a counter
    // gem three frames in four), and when it has run out they are gems of their colour.
    void TurnCounterGems(M68K &c) {
        const uint32_t player = c.a[6];
        if (Ram8(player + kTurnTimer)) {
            SetRam8(player + kTurnTimer, static_cast<uint8_t>(Ram8(player + kTurnTimer) - 1));
        }
        const uint8_t timer = Ram8(player + kTurnTimer);
        for (int column = 0; column < kFieldColumns; column++) {
            for (int row = 0; row < kFieldRows; row++) {
                const uint32_t cell = Cell(player, column, row);
                const uint16_t value = Ram16(cell);
                const uint16_t kind = value & kKind;
                const uint8_t high = Ram8(cell + kHigh);
                if ((kind != kCounterFlash && kind != kCounter) || high == 0 || (high & 0x0F) != 0) {
                    continue;
                }
                if (timer) {
                    SetRam16(cell, static_cast<uint16_t>((value & 0xF0F0) + ((timer & 3) ? kCounter : kCounterFlash)));
                } else {
                    // (the colour, as the game's arithmetic leaves it: a signed shift of a byte)
                    SetRam16(cell, static_cast<uint8_t>(static_cast<int8_t>(high & 0xF0) >> 4));
                }
            }
        }
    }

    // 00D6D6. A gem at `cell` is breaking: the counter gems beside it break with it. (Not the one above when
    // the cell is in one of the two top rows: `rowsAbove` is how many rows there are over the cell's.)
    void BreakCountersBeside(uint32_t player, uint32_t cell, int rowsAbove) {
        if (Ram8(player + kNoCounterBreaks)) {
            return;
        }
        for (const int32_t beside : {kRight, kLeft, kBelow, kAbove}) {
            if (beside == kAbove && rowsAbove <= 1) {
                continue;
            }
            if (Ram8(cell + beside + kLow) == kCounter) {
                SetRam8(cell + beside + kLow, kCounterBroken);
            }
        }
    }
    void BreakCountersBesideRoutine(M68K &c) {
        BreakCountersBeside(c.a[6], c.a[3], static_cast<int16_t>(c.d[0]));
    }

    // 00D462. Finds what breaks. A crash gem, and any gem already breaking that is marked, marks the gems of
    // its colour beside it; this is done over the whole field once each time, so a group is marked a step
    // further every frame until nothing new is found. Then notes the break for the chain and the score.
    void MarkBreaks(M68K &c) {
        const uint32_t player = c.a[6];
        bool found = false;
        uint16_t broken = 0;
        for (int column = 0; column < kFieldColumns; column++) {
            for (int row = 0; row < kFieldRows; row++) {
                const uint32_t cell = Cell(player, column, row);
                const int rowsAbove = kFieldRows - 1 - row;
                uint16_t value = Ram16(cell);
                if (value == 0 || (value & kKind) == kCounter) {
                    continue;
                }
                uint16_t starter;
                if (value & kCellMarked) {
                    if (!(value & kCellBreaking)) {
                        continue;
                    }
                    BreakCountersBeside(player, cell, rowsAbove);
                    starter = value;
                } else {
                    if ((value & kCellSeen) || (value & kKind) <= kCounter) {
                        continue;
                    }
                    starter = value & kKind; // a crash gem
                }
                SetRam16(player + kBreakKind, starter);
                SetRam8(cell + kLow, static_cast<uint8_t>((Ram8(cell + kLow) & ~kCellMarked) | kCellSeen));
                const uint16_t kind = Ram16(cell) & kKind;
                if (kind >= kBlue && kind <= kRed) {
                    SetRam8(player + kColoursBroken, static_cast<uint8_t>(Ram8(player + kColoursBroken) | (1 << (kind - 1))));
                }
                broken++;

                const uint16_t colour = starter & kColour;
                bool spread = false;
                for (const int32_t beside : {kLeft, kRight, kBelow, kAbove}) {
                    const uint16_t other = Ram16(cell + beside);
                    if ((other & kColour) == kColour || (other & (kCellBreaking | kCellSeen)) || (other & kColour) != colour) {
                        continue;
                    }
                    SetRam16(cell + beside, other | kCellMarked);
                    SetRam16(cell, Ram16(cell) | kCellMarked);
                    found = true;
                    spread = true;
                    BreakCountersBeside(player, cell, rowsAbove);
                    BreakCountersBeside(player, cell + beside, rowsAbove);
                }
                if (!spread) {
                    SetRam8(cell + kLow, static_cast<uint8_t>(Ram8(cell + kLow) & ~(kCellMarked | kCellSeen)));
                }
            }
        }

        if (!found) {
            SetRam8(player + kBreaking, 0);
            return;
        }
        SetRam8(player + kStep, kStepFall);
        SetRam8(player + kBreaking, 1);
        SetRam8(player + kNoteBusy, 1);
        SetRam8(player + kBreakTimer, kBreakFrames);
        SetRam8(player + kNoteMoved, 1);
        if (!Ram8(player + kChainStarted)) {
            SetRam8(player + kChainShown, static_cast<uint8_t>(Ram8(player + kChainLength) + 1));
            // (the game keeps its count of gems in D6, which the routine that shows the chain sometimes uses
            // for something else: the count noted for a turn's first break is whatever that leaves)
            c.d[6] = broken;
            Call(c, kShowChain);
            SetRam8(player + kChainStarted, 1);
            broken = static_cast<uint16_t>(c.d[6]);
        }
        const uint16_t colour = Ram16(player + kBreakKind) & kColour;
        if (colour != Ram16(player + kBreakKindBefore)) {
            const uint16_t at = Ram16(player + kBreakListAt);
            SetRam16(player + kBreakColours + at, colour);
            SetRam16(player + kBreakCounts + at, broken);
            SetRam16(player + kBreakListAt, static_cast<uint16_t>(at + 2));
        }
    }
    // 00D3D6. The gems marked the frame before start to break (a counter gem's mark only becomes the mark
    // of one that has been seen).
    constexpr uint32_t kBreakSound = 0x02BA96, kBreakSoundOther = 0x02BAD0;
    constexpr uint32_t kLoopColumn = 0x276, kLoopRow = 0x278; // words: where the game keeps this loop's counters
    void StartBreaks(M68K &c) {
        const uint32_t player = c.a[6];
        for (int column = 0; column < kFieldColumns; column++) {
            for (int row = 0; row < kFieldRows; row++) {
                const uint32_t cell = Cell(player, column, row);
                const uint16_t value = Ram16(cell);
                if (value == 0 || !(value & kCellMarked)) {
                    continue;
                }
                if ((value & kKind) == kCounter) {
                    SetRam8(cell + kLow, static_cast<uint8_t>((Ram8(cell + kLow) & ~kCellMarked) | kCellSeen));
                    continue;
                }
                SetRam16(cell, value | kCellBreaking);
                // (one sound or the other, by the cell's high byte: 3 in its high half is a counter gem that
                // was turning into a green gem)
                Call(c, (Ram8(cell + kHigh) & 0xF0) == 0x30 ? kBreakSoundOther : kBreakSound);
            }
        }
        SetRam16(player + kLoopColumn, kRanOut);
        SetRam16(player + kLoopRow, kRanOut);
    }

    // 00D26A. The diamond: every gem of the colour it landed on is marked to break, crash gems and counter
    // gems too (a counter gem of that colour becomes a gem first), and the diamond with them. On the floor
    // it breaks nothing and earns kPointsDiamondOnFloor.
    constexpr uint32_t kDiamondOnFloor = 0x390; // byte
    constexpr uint32_t kPointsDiamondOnFloor = 10000;
    constexpr uint32_t kShowDiamondBonus = 0x00C8B8;
    constexpr uint32_t kDiamondTime = 9000; // (what the game's code takes to go through the field, about)
    void DiamondBreaks(M68K &c) {
        const uint32_t player = c.a[6];
        SetRam8(player + kDiamondOnFloor, 0);
        const uint16_t under = Ram16(player + kUnderDiamond);
        if (under != kDiamondDone && (under & kColour) != 0) {
            // (what a counter gem holds in its high byte: its colour)
            const uint16_t colour = (under & kColour) == kCounter ? Ram8(player + kUnderDiamond) >> 4 : under & kColour;
            Spend(c, kDiamondTime);
            bool any = false;
            for (int column = 0; column < kFieldColumns; column++) {
                for (int row = 0; row < kFieldRows; row++) {
                    const uint32_t cell = Cell(player, column, row);
                    uint16_t kind = Ram16(cell) & kColour;
                    if (kind == 0) {
                        continue;
                    }
                    if (kind == kCounter) {
                        kind = Ram8(cell + kHigh) >> 4;
                        if (kind == colour) {
                            SetRam16(cell, kind);
                        }
                    }
                    if (kind == kCounter) {
                        continue;
                    }
                    if (kind == kDiamond) {
                        any = true;
                        SetRam16(cell, Ram16(cell) | kCellMarked);
                        const uint16_t below = Ram16(cell + kBelow);
                        if (below == kWall) {
                            SetRam8(player + kDiamondOnFloor, 1);
                        } else if (below == kCounter) {
                            SetRam16(cell + kBelow, below | kCellMarked);
                        } else if (!(below & kCellBig)) {
                            SetRam16(cell + kBelow, kCellMarked | 6); // (whatever it was: a marked gem of kind 6)
                        }
                        SetRam16(player + kUnderDiamond, kDiamondDone);
                    } else if (kind == colour) {
                        any = true;
                        SetRam16(cell, Ram16(cell) | kCellMarked);
                    }
                }
            }
            if (any) {
                SetRam8(player + kStep, kStepFall);
                SetRam8(player + kNoteBusy, 1);
                SetRam8(player + kBreakTimer, kBreakFrames);
                SetRam8(player + kNoteMoved, 1);
            }
        }
        if (Ram8(player + kDiamondOnFloor)) {
            SetRam32(player + kScore, Ram32(player + kScore) + kPointsDiamondOnFloor);
            Call(c, kShowDiamondBonus);
        }
    }

    // 00D058. As a turn ends: every gem with nothing under it goes down a row (the counter gems that have
    // just come in; a big gem stays, and holds up what is on it). While anything moved the turn goes back
    // to count the field, and comes here again; otherwise it is over.
    void DropLooseGems(M68K &c) {
        const uint32_t player = c.a[6];
        SetRam8(player + 0x1E4, 0);
        for (int column = 0; column < kFieldColumns; column++) {
            bool held = true;
            for (int row = 0; row <= kFieldRows; row++) {
                const uint32_t cell = Cell(player, column, row);
                const uint16_t value = Ram16(cell);
                if (value == 0) {
                    held = false;
                } else if (!held) {
                    if (value & kCellBig) {
                        held = true;
                        continue;
                    }
                    SetRam16(cell + kBelow, value);
                    SetRam16(cell, 0);
                    SetRam8(player + kNoteBusy, 1);
                    SetRam8(player + kNoteChanged, 1);
                    SetRam8(player + kNoteFalling, 1);
                    SetRam8(player + kFell, 1);
                    SetRam8(player + kNoteMoved, 1);
                }
            }
        }
        SetRam8(player + kStep, kStepNewPair);
        if (Ram8(player + kFell)) {
            SetRam8(player + kFell, 0);
            SetRam8(player + kNoteBusy, 0);
            SetRam8(player + kNoteChanged, 1);
            SetRam8(player + kStep, kStepCountAgain);
        }
    }
} // namespace

void RegisterField() {
    AddScratchMemory(kSoundSavedRegisters, kSoundSavedRegistersEnd);
    // (the last number is the time a call is charged: the average the check measured for the game's own code)
    constexpr uint32_t kScratch = kFreeD0 | kFreeD1 | kFreeD2 | kFreeD3 | kFreeD4 | kFreeA0 | kFreeA1 | kFreeFlags;
    AddRewrite(0x00ECAA, "TidyField", TidyField, kFreeFlags, 6886);
    AddRewrite(0x00D76E, "CountField", CountField, kScratch, 6289);
    AddRewrite(0x00D8BC, "CountDownCounterGems", CountDownCounterGems, kScratch, 5005);
    AddRewrite(0x00D824, "TurnCounterGems", TurnCounterGems, kScratch | kFreeD5, 7706);
    AddRewrite(0x00D6D6, "BreakCountersBeside", BreakCountersBesideRoutine, kFreeD1 | kFreeFlags, 161);
    constexpr uint32_t kAll = kScratch | kFreeD5 | kFreeD6 | kFreeD7 | kFreeA2 | kFreeA3 | kFreeA4;
    AddRewrite(0x00D3D6, "StartBreaks", StartBreaks, kAll, 6600);
    AddRewrite(0x00D26A, "DiamondBreaks", DiamondBreaks, kAll, 0);
    AddRewrite(0x00D058, "DropLooseGems", DropLooseGems, kAll, 4760);
    AddRewrite(0x00D462, "MarkBreaks", MarkBreaks, kScratch | kFreeD5 | kFreeD6 | kFreeA3, 8925);
}

} // namespace cps2recomp
