// SPDX-License-Identifier: GPL-3.0-only
// Copyright (C) 2026 Tommy McLeroy
//
// Super Puzzle Fighter II Turbo: the counter gems that fall into a field, written again as C++ (rewrites:
// lift.h).
#include "spf2.h"

#include <algorithm>

namespace cps2recomp {

namespace {
    using namespace spf2;

    // Counter gems fall in a pattern of colours that belongs to the character of the player who sent them:
    // for each character twelve rows (the first to fall is the first row; after the twelfth the last is
    // used again) of eight words, a colour number for each of the six columns.
    constexpr uint32_t kDropPatterns = 0x010188, kDropPatternSize = 0xC0, kDropPatternRow = 16;
    constexpr int kDropPatternRows = 12;
    // What a colour number in a pattern makes of a cell: the colour in the high byte's high four bits
    constexpr uint32_t kDropColours = 0x010178;
    // ... and the rest of the cell by the sender's character: a counter gem (kind 7) with its count (5 for all)
    constexpr uint32_t kDropCounts = 0x010160;
    // When kShortCount is set the gems arrive with a count of 3 whoever sent them
    constexpr uint32_t kShortCount = 0x27F; // byte
    constexpr uint16_t kShortCounter = 0x0307;

    // 00FF3C. The counter gem for the column being filled (kDropColumn) of the row now falling, left in D2.
    void CounterGemFor(M68K &c) {
        const uint32_t player = c.a[6], opponent = Ram32(player + kOpponent);
        const uint8_t character = Ram8(opponent + kCharacter);
        const int16_t fallen = static_cast<int16_t>(Ram16(player + kRowsFallen));
        const uint32_t row = fallen < kDropPatternRows ? static_cast<uint16_t>(fallen) : kDropPatternRows - 1;
        const uint16_t colour = Mem16(c, kDropPatterns + character * kDropPatternSize + row * kDropPatternRow + Ram16(player + kDropColumn));
        uint16_t gem = Mem16(c, kDropColours + 2 * colour);
        if (Ram8(player + kShortCount)) {
            gem = static_cast<uint16_t>(gem + kShortCounter);
            c.d[0] = 2u * colour;
            c.a[3] = kDropColours;
        } else {
            const uint16_t rest = Mem16(c, kDropCounts + 2 * character);
            gem = static_cast<uint16_t>(gem + rest);
            c.d[0] = rest;
            c.a[3] = kDropCounts;
        }
        SetLow16(c.d[2], gem);
        c.a[2] = opponent;
    }

    // The order single counter gems fill a row in: eight orders of the six columns (bytes: the column
    // doubled), one chosen at random when a turn begins (kColumnOrder)
    constexpr uint32_t kColumnOrders = 0x00FF98;
    // The first attack of a round earns whoever made it kFirstAttackPoints (once: kFirstAttackDone, in both records)
    constexpr uint32_t kFirstAttackDone = 0x391; // byte
    constexpr uint32_t kFirstAttackPoints = 1000;
    constexpr uint32_t kShowFirstAttack = 0x00C8E0, kWarnOfAttack = 0x00C50C;

    // The time the game's own code takes over a drop, and over each gem of it (as near as the check measures)
    constexpr uint32_t kDropTime = 420, kGemTime = 370;

    // The counter gem for a column (doubled) of the row now falling
    uint16_t GemFor(M68K &c, uint32_t player, uint16_t column) {
        SetRam16(player + kDropColumn, column);
        CounterGemFor(c);
        Spend(c, kGemTime);
        return static_cast<uint16_t>(c.d[2]);
    }

    // 00CCC6. The opponent's attack falls into the player's field: into the row above the top, from where
    // the gems drop. A whole row of six at once when a row is to be started and six or more are waiting;
    // otherwise one at a time in the turn's column order, until none are waiting or the row is full.
    void DropCounterGems(M68K &c) {
        const uint32_t player = c.a[6], opponent = Ram32(player + kOpponent);
        c.a[2] = opponent;
        if (Ram8(opponent + kAttack) == 0) {
            return;
        }
        if (!Ram8(player + kFirstAttackDone) && !Ram8(Var(c, kNoAllClear))) {
            SetRam32(opponent + kScore, Ram32(opponent + kScore) + kFirstAttackPoints);
            SetRam8(Player(0) + kFirstAttackDone, 1);
            SetRam8(Player(1) + kFirstAttackDone, 1);
            Call(c, kShowFirstAttack);
        }
        Call(c, kWarnOfAttack);
        Spend(c, kDropTime);
        SetRam8(player + 0x322, 1);
        SetRam8(player + kAttacked, 1);
        // (a count to three that nothing reads)
        SetRam8(player + 0x261, static_cast<uint8_t>(Ram8(player + 0x261) + 1));
        if (static_cast<int8_t>(Ram8(player + 0x261)) >= 3) {
            SetRam8(player + 0x261, 0);
        }
        GemFor(c, player, 0); // (the game works one out here and does not use it)

        const uint32_t row = Ram32(player + kFieldBottom) - kRowSize + 2; // the row above the top one, its first cell
        for (;;) {
            if (Ram8(player + kRowFilled) == 0 && Ram8(opponent + kAttack) > 5) {
                for (uint16_t column = 0; column < 2 * kFieldColumns; column += 2) {
                    SetRam16(row + column, GemFor(c, player, column));
                }
                SetRam8(opponent + kAttack, static_cast<uint8_t>(Ram8(opponent + kAttack) - 6));
                SetRam16(player + kRowsFallen, static_cast<uint16_t>(Ram16(player + kRowsFallen) + 1));
                SetRam8(player + kFellThisTurn, static_cast<uint8_t>(Ram8(player + kFellThisTurn) + 6));
                break;
            }
            const uint16_t column = Mem8(c, kColumnOrders + ((Ram8(player + kColumnOrder) * 8u + Ram8(player + kRowFilled)) & 0xFF));
            if (Ram8(row + column) == 0) { // (no counter gem there already)
                SetRam16(row + column, GemFor(c, player, column));
            }
            SetRam8(player + kFellThisTurn, static_cast<uint8_t>(Ram8(player + kFellThisTurn) + 1));
            SetRam8(opponent + kAttack, static_cast<uint8_t>(Ram8(opponent + kAttack) - 1));
            SetRam8(player + kRowFilled, static_cast<uint8_t>(Ram8(player + kRowFilled) + 1));
            const bool rowFull = Ram8(player + kRowFilled) == kFieldColumns;
            if (rowFull) {
                SetRam16(player + kRowsFallen, static_cast<uint16_t>(Ram16(player + kRowsFallen) + 1));
                SetRam8(player + kRowFilled, 0);
            }
            if (rowFull || Ram8(opponent + kAttack) == 0) {
                SetRam8(player + 0x1BE, 0);
                SetRam16(player + 0x20A, 0);
                break;
            }
        }
        if (Ram8(opponent + kAttack) == 0) {
            SetRam8(player + kShortCount, 0);
        }
    }
} // namespace

void RegisterDrop() {
    AddRewrite(0x00CCC6, "DropCounterGems", DropCounterGems,
               kFreeD0 | kFreeD1 | kFreeD2 | kFreeD3 | kFreeA0 | kFreeA1 | kFreeA3 | kFreeFlags, 42);
    AddRewrite(0x00FF3C, "CounterGemFor", CounterGemFor, kFreeFlags, 258);
}

} // namespace cps2recomp
