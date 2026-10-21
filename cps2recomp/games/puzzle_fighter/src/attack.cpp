// SPDX-License-Identifier: GPL-3.0-only
// Copyright (C) 2026 Tommy McLeroy
//
// Super Puzzle Fighter II Turbo: what a break is worth, and the attack it makes, written again as C++
// (rewrites: lift.h). In all of these A6 is the record of the player whose gems broke.
//
// In short: a break scores 100 a gem, 10 a counter gem, with bonuses for many gems, for big gems, for
// several colours at once and for chains. The attack is the score over 100 in counter gems, more the longer
// the round has lasted, scaled by the operator's damage setting. It is first set against what the opponent
// has waiting for this player: cancelling all of it leaves the rest as the attack; failing to cancel all of
// it takes off only a part of the attack's worth (CounterStrength).
#include "spf2.h"

#include <algorithm>

namespace cps2recomp {

BreakNote g_breakNotes[2];
const BreakNote &LastBreak(int player) {
    return g_breakNotes[player];
}

namespace {
    using namespace spf2;

    // The game divides by taking away until what is left goes below nothing
    uint16_t DivideByTakingAway(uint16_t value, uint16_t by) {
        uint16_t times = 0;
        for (;;) {
            value = static_cast<uint16_t>(value - by);
            if (value & 0x8000) {
                return times;
            }
            times++;
        }
    }
    // ... and multiplies by adding
    uint16_t Times(uint16_t value, uint16_t times) {
        return static_cast<uint16_t>(value * times);
    }

    // 00F97A. A big gem's cell at `cell` is breaking: the spent counter gems (nothing but the kind left of
    // them) in its row and its column go with it.
    void BreakSpentCounters(uint32_t cell) {
        auto along = [&](int32_t step, uint16_t end) {
            for (uint32_t at = cell + step; Ram16(at) != end; at += step) {
                if (Ram16(at) == kCounter) {
                    SetRam16(at, Ram16(at) | kCellBreaking);
                }
            }
        };
        along(2, kWall);
        along(-2, kWall);
        along(static_cast<int32_t>(kRowSize), kWall);
        along(-static_cast<int32_t>(kRowSize), kEmpty); // (upwards as far as the first empty cell)
    }

    // 00F8E0. The bonus for the big gems in a break: every breaking cell of one counts, and each is worth
    // more the more of them there are.
    void BigGemBonus(M68K &c) {
        const uint32_t player = c.a[6];
        uint16_t cells = 0;
        for (int column = 0; column < kFieldColumns; column++) {
            for (int row = 0; row < kFieldRows - 1; row++) {
                const uint32_t cell = Cell(player, column, row);
                const uint16_t value = Ram16(cell), kind = value & kKind;
                if ((value >> 8) == 0 || !(value & kCellBreaking) || kind == 0x0F || kind == kCounter) {
                    continue;
                }
                BreakSpentCounters(cell);
                cells++;
            }
        }
        uint16_t bonus = 0;
        if (cells) {
            uint16_t each = Mem16(c, kBigGemBonuses + 2 * std::min<uint16_t>(cells - 1, kBigGemBonusLast));
            if (static_cast<int16_t>(each) > kBigGemBonusCapped && Ram8(player + kBigGemBonusCap)) {
                each = kBigGemBonusCapped;
            }
            bonus = Times(each, cells);
        }
        SetRam16(player + kBigGemBonus, bonus);
    }

    // 00FF14. The bonus for a break of many gems: 10 for each gem past the second.
    void ManyGemsBonus(M68K &c) {
        const uint32_t player = c.a[6];
        const int16_t gems = static_cast<int16_t>(Ram16(player + kBrokenCount));
        SetRam16(player + kBrokenCount, gems > 2 ? Times(kPointsExtraGem, static_cast<uint16_t>(gems - 2)) : 0);
    }

    // 00FEE4. The bonus for a chain: nothing for a turn's first break, then by the table. Keeps the longest
    // chain the player has made.
    void ChainBonus(M68K &c) {
        const uint32_t player = c.a[6];
        const uint8_t chain = Ram8(player + kChainLength);
        if (static_cast<int8_t>(chain) > static_cast<int8_t>(Ram8(player + kLongestChain))) {
            SetRam8(player + kLongestChain, chain);
        }
        uint16_t bonus = 0;
        if (chain > 1) {
            bonus = Mem16(c, kChainBonuses + 2 * std::min<uint32_t>(chain - 1u, 10));
        }
        SetRam16(player + kChainBonus, bonus);
    }

    // 00FEA8. The bonus for breaking gems of two or more colours in one break: 200 a colour.
    void ColourBonus(M68K &c) {
        const uint32_t player = c.a[6];
        uint8_t colours = 0;
        for (int bit = 0; bit < 4; bit++) {
            colours += (Ram8(player + kColoursBroken) >> bit) & 1;
        }
        SetRam8(player + kColoursBrokenCount, colours);
        SetRam16(player + kColourBonus, colours >= 2 ? Times(kPointsColour, colours) : 0);
    }

    // The attack a score makes, in counter gems
    uint16_t AttackFor(M68K &c, uint32_t player, uint16_t score) {
        // More as the round goes on: a tenth of the score added for each step of time passed
        int step = 0;
        while (static_cast<int16_t>(Ram16(player + kRoundSeconds) + 15) > static_cast<int16_t>(Mem16(c, kAttackTimes + 2 * step))) {
            step++;
        }
        uint16_t worth = static_cast<uint16_t>(score + Times(DivideByTakingAway(score, 10), Mem16(c, kAttackTenths + 2 * step)));
        // The operator's damage setting: so many tenths of it
        const uint16_t tenths = static_cast<uint16_t>(Mem16(c, kDamageTenths + 2 * (Ram8(Var(c, kDamageLevel)) & 7)) + 10);
        worth = Times(DivideByTakingAway(worth, 10), tenths);
        // A counter gem for every hundred
        int16_t attack = static_cast<int16_t>(DivideByTakingAway(worth, 100));
        if (Ram8(player + kHalfAttack)) {
            attack >>= 1;
        }
        // A person playing one of the two hidden characters attacks at three quarters' strength
        if (!Ram8(player + kComputer) && (Ram8(player + kCharacter) == 8 || Ram8(player + kCharacter) == 9)) {
            attack = static_cast<int16_t>((attack >> 1) + (attack >> 2));
        }
        // Handicaps: a tenth more for each level the opponent is above the player, and one more
        if (Ram8(Var(c, kMode)) == kModeHandicap) {
            const int16_t levels = static_cast<int16_t>(Ram8(Ram32(player + kOpponent) + kHandicap) - Ram8(player + kHandicap));
            if (levels > 0) {
                attack = static_cast<int16_t>(attack + Times(DivideByTakingAway(static_cast<uint16_t>(attack), 10), static_cast<uint16_t>(levels + 1)));
            }
        }
        return static_cast<uint16_t>(attack);
    }

    // 00FD64. Adds the attack for a break's score (D3) to what the player has to send, kAttackMost at most.
    // (The game tests for the most with the sum's low byte only, so a sum past 255 is not caught.)
    void AddAttack(M68K &c) {
        const uint32_t player = c.a[6];
        uint16_t attack = AttackFor(c, player, static_cast<uint16_t>(c.d[3]));
        if (FeatureOn(player == Player(0) ? "no_attack" : "no_attack_2")) {
            attack = 0;
        }
        const uint16_t sum = static_cast<uint16_t>((attack & 0xFF00) | ((attack + Ram8(player + kAttack)) & 0xFF));
        if (static_cast<int16_t>(sum) > kAttackMost) {
            SetRam8(player + 0x1EE, 0);
            SetRam8(player + kAttack, kAttackMost);
        } else {
            SetRam8(player + kAttack, static_cast<uint8_t>(Ram8(player + kAttack) + attack));
        }
    }

    // What an attack is worth against a bigger one waiting for the player: half for a small attack, and for a
    // bigger one about five to eight tenths
    uint16_t CounterStrengthOf(uint16_t attack) {
        if (static_cast<int16_t>(attack) <= 12) {
            return attack >> 1;
        }
        const uint16_t times = attack <= 18 ? 5 : attack <= 24 ? 6 : attack <= 30 ? 7 : 8;
        return Times(static_cast<uint16_t>(DivideByTakingAway(attack, 8) + 1), times);
    }
    // 00FCC6. (the attack in D0, and its worth left there)
    void CounterStrength(M68K &c) {
        SetLow16(c.d[0], CounterStrengthOf(static_cast<uint16_t>(c.d[0])));
    }

    // 00FAB0. Takes the broken gems out of the field, adds up the break's score and bonuses, makes the
    // attack and sets it against the opponent's.
    void ScoreBreak(M68K &c) {
        const uint32_t player = c.a[6], opponent = Ram32(player + kOpponent);
        auto add16 = [&](uint32_t field, uint16_t by) { SetRam16(player + field, static_cast<uint16_t>(Ram16(player + field) + by)); };
        auto add8 = [&](uint32_t field) { SetRam8(player + field, static_cast<uint8_t>(Ram8(player + field) + 1)); };

        SetRam16(player + kHighestBrokenRow, 0);
        uint16_t taken = 0;
        for (int column = 0; column < kFieldColumns; column++) {
            for (int row = 0; row < kFieldRows; row++) {
                const uint32_t cell = Cell(player, column, row);
                uint16_t value = Ram16(cell);
                if (!(value & kCellBreaking)) {
                    continue;
                }
                SetRam16(cell, 0);
                if (value & kCellBig) {
                    add8(kBigGemCellsBroken);
                    add8(kBigGemCellsByColour + std::max((value & kColour) - 1, 0));
                    value = 0; // (and so a big gem's cells score as ordinary gems)
                }
                // (the game's loop counters: columns from 5 down to 0, rows from 13 at the bottom down to 0)
                SetRam16(player + kLastBrokenColumn, static_cast<uint16_t>(kFieldColumns - 1 - column));
                const int16_t rowCount = static_cast<int16_t>(kFieldRows - 1 - row);
                if (rowCount > static_cast<int16_t>(Ram16(player + kHighestBrokenRow))) {
                    SetRam16(player + kHighestBrokenRow, static_cast<uint16_t>(rowCount));
                }
                const uint16_t kind = value & kKind;
                if (kind == kCounter) {
                    add16(kCounterPoints, kPointsCounter);
                } else if (kind < kCounter) {
                    add16(kGemPoints, kPointsGem);
                } else {
                    add16(kCrashPoints, kPointsGem);
                    if (kind >= 8 && kind <= 13) {
                        SetRam8(player + kCrashKindsBroken + (kind - 8), 1);
                    }
                }
                add16(kBrokenCount, 1);
                taken++;
            }
        }
        ManyGemsBonus(c);

        bool scored = false;
        if (taken) {
            ChainBonus(c);
            ColourBonus(c);
            const uint16_t score = static_cast<uint16_t>(Ram16(player + kChainBonus) + Ram16(player + kColourBonus) + Ram16(player + kBrokenCount) +
                                                         Ram16(player + kBigGemBonus) + Ram16(player + kGemPoints) + Ram16(player + kCrashPoints) +
                                                         Ram16(player + kCounterPoints));
            SetRam32(player + kScore, Ram32(player + kScore) + score);
            scored = score != 0;
            if (scored) {
                BreakNote &note = g_breakNotes[player == Player(0) ? 0 : 1];
                const uint8_t attackBefore = Ram8(player + kAttack), theirsBefore = Ram8(opponent + kAttack);
                c.d[3] = score;
                AddAttack(c);
                note.serial++;
                note.chain = Ram8(player + kChainLength);
                note.gems = taken;
                note.score = score;
                note.attack = Ram8(player + kAttack) - attackBefore;
                SetRam16(player + kBrokenCount, 0);
                SetRam16(player + kBigGemBonus, 0);
                SetRam16(player + kColourBonus, 0);
                SetRam8(player + kColoursBrokenCount, 0);
                SetRam8(player + kColoursBroken, 0);
                SetRam8(player + kBigGemBonusCap, 0);

                if (static_cast<int16_t>(Ram16(player + kRoundSeconds)) >= kNoOffsetAfter) {
                    SetRam8(player + 0x1EE, 0);
                } else {
                    // The attack against what the opponent has waiting for this player
                    const uint8_t mine = Ram8(player + kAttack), theirs = Ram8(opponent + kAttack);
                    SetRam8(player + kAttackBeforeOffset, mine);
                    if (theirs < mine) {
                        SetRam8(player + kAttack, static_cast<uint8_t>(mine - theirs));
                        SetRam8(player + 0x1EE, 0);
                        SetRam8(opponent + 0x1EE, 0);
                        SetRam8(opponent + kAttack, 0);
                    } else {
                        const int16_t left = static_cast<int16_t>(theirs - CounterStrengthOf(mine));
                        SetRam8(opponent + kAttack, static_cast<uint8_t>(std::max<int16_t>(left, 0)));
                        SetRam8(opponent + 0x1EE, 0);
                        SetRam8(player + 0x1EE, 0);
                        SetRam8(player + kAttack, 0);
                    }
                }
                SetRam8(player + 0x349, 1);
                note.cancelled = theirsBefore - Ram8(opponent + kAttack);
                note.waiting = Ram8(player + kAttack);
            }
        }
        if (Ram8(player + kChainLength)) {
            c.d[0] = Ram8(player + kChainLength);
            Call(c, kShowChainScore);
        }
    }
} // namespace

void RegisterAttack() {
    // (the last number is the time a call is charged: the average the check measured for the game's own code)
    constexpr uint32_t kScratch = kFreeD0 | kFreeD1 | kFreeD2 | kFreeD3 | kFreeD4 | kFreeA0 | kFreeA1 | kFreeA2 | kFreeA3 | kFreeFlags;
    AddRewrite(0x00F8E0, "BigGemBonus", BigGemBonus, kScratch, 3313);
    AddRewrite(0x00FF14, "ManyGemsBonus", ManyGemsBonus, kScratch, 125);
    AddRewrite(0x00FEE4, "ChainBonus", ChainBonus, kScratch, 75);
    AddRewrite(0x00FEA8, "ColourBonus", ColourBonus, kScratch, 268);
    AddRewrite(0x00FD64, "AddAttack", AddAttack, kScratch, 5050);
    AddRewrite(0x00FCC6, "CounterStrength", CounterStrength, kFreeD2 | kFreeD3 | kFreeD4 | kFreeFlags, 40);
    AddRewrite(0x00FAB0, "ScoreBreak", ScoreBreak, kScratch, 11300);
}

} // namespace cps2recomp
