// SPDX-License-Identifier: GPL-3.0-only
// Copyright (C) 2026 Tommy McLeroy
//
// Super Puzzle Fighter II Turbo: the falling pair and the player's steering of it, written again as C++
// (rewrites: lift.h). This is the first step of a player's turn. A6 is the player's record.
//
// The pair is two gems: one at a column and row of the field (kPairColumn, kPairRow), which the other turns
// about; kPairTurn says where the other is (0 above, 1 to the right, 2 below, 3 to the left). The pair is
// not in the field's cells until it lands.
#include "spf2.h"

namespace cps2recomp {

void SetFallSpeedFor(M68K &c); // pair.cpp

namespace {
    using namespace spf2;

    constexpr uint32_t kPairTurn = 0x170;                   // byte
    constexpr uint16_t kStartRow = 0;
    // From the first gem's cell to the other's, by kPairTurn; and what a pair that cannot turn where it is
    // does instead: it is pushed the other way (columns and rows)
    constexpr int32_t kOther[4] = {-16, 2, 16, -2};
    constexpr int16_t kPushColumn[4] = {0, -1, 0, 1}, kPushRow[4] = {1, 0, -1, 0};
    constexpr int32_t kBelow = 16;
    // The controls, in the player's pad word
    constexpr uint16_t kStick = 0x000F, kStickDown = 0x0004, kStickUp = 0x0008, kTurnLeft = 0x0100, kTurnRight = 0x0200;

    // Resting: when the pair has something under it, it can still be steered for kRestFrames frames (the
    // count is in kResting); eight such rests in a turn and it stays where it is
    constexpr uint8_t kRestFrames = 0x10, kRestsMost = 8;
    constexpr uint32_t kRests = 0x1A8;     // byte
    constexpr uint32_t kFallTimer = 0x15E; // byte: frames until the pair goes down a row
    constexpr uint32_t kFallFrames = 0x18A;
    constexpr uint32_t kWasResting = 0x27E; // byte
    constexpr uint32_t kSlideStep = 0x27C; // word: how far the pair's picture moves a frame
    constexpr uint32_t kTurnFrames = 0x1A6; // word: frames of this turn
    constexpr uint32_t kPushCount = 0x1A9, kPushRows = 0x320; // byte, word: pushing down
    constexpr uint32_t kKicksUp = 0x314, kStuck = 0x316; // word, byte: pushed up by turning too often, the pair stays
    constexpr uint32_t kTurnWay = 0x1A4;    // byte: 1 while a turn to the right is being tried
    constexpr uint32_t kTurnPictureFrames = 0x24C;                 // word
    constexpr uint32_t kTurningAcross = 0x248, kTurningDown = 0x24A; // words: where the turning gem is drawn, from its cell
    constexpr uint32_t kTurnShown = 0x247;  // byte: for the picture (0 right, 1 left, 2 none)
    constexpr uint32_t kShortCount = 0x27F;
    constexpr uint32_t kComputerSteers = 0x011F34, kPushSound = 0x02BA0C, kTurnSound = 0x02B9B0, kLandSound = 0x02BA3A,
                       kDrawPair = 0x0123BE;

    constexpr uint32_t kLandingTime = 3300; // (what the game's code takes over a landing, more than over other frames)

    uint16_t Pressed(uint32_t player) {
        return static_cast<uint16_t>(~Ram16(player + kPadBefore) & Ram16(player + kPad));
    }
    // The first gem's cell
    uint32_t PairCell(uint32_t player) {
        const int16_t row = static_cast<int16_t>(Ram16(player + kPairRow)), column = static_cast<int16_t>(Ram16(player + kPairColumn));
        return Ram32(player + kFieldBottom) + 2 - 2 * kRowSize + (row > 0 ? row * kRowSize : 0) + (column > 0 ? column * 2u : 0);
    }
    // 011916. (the game's routine: two addresses are moved to the pair's place, in A0 by cells of the field
    // and in A1 by the cells of something else laid out in columns)
    void PairCellRoutine(M68K &c) {
        const uint32_t player = c.a[6];
        const int16_t row = static_cast<int16_t>(Ram16(player + kPairRow)), column = static_cast<int16_t>(Ram16(player + kPairColumn));
        c.a[0] += -2 * static_cast<int32_t>(kRowSize) + (row > 0 ? row * kRowSize : 0) + (column > 0 ? column * 2u : 0);
        c.a[1] += -0xC + (row > 0 ? row * 4u : 0) + (column > 0 ? column * 0x40u : 0);
        SetLow16(c.d[1], row > 0 ? 0xFFFF : static_cast<uint16_t>(row - 1));
        SetLow16(c.d[0], column > 0 ? 0xFFFF : static_cast<uint16_t>(column - 1));
    }

    // What is under the pair's two gems, added together: nothing if it can go down a row
    uint16_t UnderPair(uint32_t player) {
        const uint32_t cell = PairCell(player);
        return static_cast<uint16_t>(Ram16(cell + kBelow) + Ram16(cell + kOther[Ram8(player + kPairTurn) & 3] + kBelow));
    }
    // 011948. (left in D3)
    void UnderPairRoutine(M68K &c) {
        const uint32_t player = c.a[6];
        c.d[3] = UnderPair(player);
        c.d[0] = Ram8(player + kPairTurn) * 2u;
        c.a[0] = PairCell(player) + kOther[Ram8(player + kPairTurn) & 3];
        c.a[1] = 0x012BC8;
    }

    // 011974. The pair lands: its gems go into the field. A diamond notes what it landed on (that colour
    // will break), and the counter gems the break sends will have short counts.
    void LandPair(M68K &c) {
        const uint32_t player = c.a[6];
        const uint32_t first = PairCell(player), other = first + kOther[Ram8(player + kPairTurn) & 3];
        SetRam16(player + kUnderDiamond, 0);
        SetRam16(first, Ram16(player + kPairNow));
        SetRam16(other, Ram16(player + kPairNow + 2));
        uint16_t under = 0;
        if (Ram16(player + kPairNow) == kDiamond) {
            SetRam8(player + kNoCounterBreaks, 1);
            under = Ram16(first + kBelow);
            if (under == 0 || under == kWall) {
                under = kDiamond;
            }
        } else if (Ram16(player + kPairNow + 2) == kDiamond) {
            SetRam8(player + kHalfAttack, 1);
            SetRam8(player + kNoCounterBreaks, 1);
            under = Ram16(other + kBelow);
        }
        if (under == 0) {
            return;
        }
        SetRam8(Ram32(player + kOpponent) + kShortCount, 1);
        SetRam16(player + kUnderDiamond, under);
        SetRam8(player + 0x31F, 0);
        if (Ram16(first + 2 * kBelow) & kCellBig) {
            SetRam8(player + 0x31F, 1);
            SetRam16(player + 0x372, Ram16(first + 2 * kBelow + kFieldExtra));
        }
    }

    // 011BBE. The stick held straight down pushes the pair: a row every few frames, sooner the longer it is held.
    void PushDown(M68K &c) {
        const uint32_t player = c.a[6];
        if (Ram8(player + 0x19A) || (Ram16(player + kPad) & kStick) != kStickDown || static_cast<int16_t>(Ram16(player + kTurnFrames)) <= 16) {
            return;
        }
        if ((!Ram8(player + kPushHeld) && Ram16(player + kPairRow) == 3) || ((Pressed(player) & kStickDown) && !Ram8(player + kComputer))) {
            Call(c, kPushSound);
            SetRam8(player + kPushHeld, 1);
        }
        SetRam8(player + kPushCount, static_cast<uint8_t>(Ram8(player + kPushCount) + 1));
        SetRam16(player + kPushRows, static_cast<uint16_t>(Ram16(player + kPushRows) + 1));
        const int16_t rows = static_cast<int16_t>(Ram16(player + kPushRows));
        const int8_t frames = rows < 6 ? 4 : rows < 12 ? 2 : 1;
        if (frames > static_cast<int8_t>(Ram8(player + kPushCount))) {
            return;
        }
        SetRam8(player + kFallTimer, 0);
        SetRam8(player + kPushCount, 0);
        SetRam16(player + kSlide, 0);
    }

    // The pair is at the very top and still coming into the field
    bool ComingIn(uint32_t player) {
        return Ram16(player + kPairRow) == 2 && static_cast<int16_t>(Ram16(player + kSlide)) < 0;
    }

    // 011C80. A turn button with the pair shut in on both sides (or still coming in): its two gems change places.
    void SwapPair(M68K &c) {
        const uint32_t player = c.a[6];
        if (!(Pressed(player) & (kTurnLeft | kTurnRight)) || Ram8(player + kResting) == kRestFrames) {
            return;
        }
        if (!ComingIn(player)) {
            const uint32_t cell = PairCell(player);
            if (Ram16(cell + 2) == 0 || Ram16(cell - 2) == 0) {
                return;
            }
        }
        const uint16_t first = Ram16(player + kPairNow);
        SetRam16(player + kPairNow, Ram16(player + kPairNow + 2));
        SetRam16(player + kPairNow + 2, first);
        SetRam16(player + kTurningAcross, 0);
        SetRam16(player + kTurningDown, 0);
        SetRam16(player + kTurnPictureFrames, 8);
        SetRam8(player + kTurnShown, 2);
    }

    // 011E6A. The pair is to turn so that its other gem is at `turn` (D0), with its first gem at `cell` (A0).
    // If something is in the way there and the opposite side is free, the pair is pushed that way to make
    // room. Pushed up off what it rests on more than twice, it stays where it is for good.
    void MakeRoomToTurn(M68K &c, uint32_t &cell, uint16_t &turn) {
        const uint32_t player = c.a[6];
        if (Ram16(cell + kOther[turn & 3]) == 0 || Ram16(cell - kOther[turn & 3]) != 0) {
            SetRam8(player + 0x1A3, 0);
            SetRam8(player + kTurnShown, 2);
            return;
        }
        SetRam16(player + kPairColumn, static_cast<uint16_t>(Ram16(player + kPairColumn) + kPushColumn[turn & 3]));
        SetRam16(player + kPairRow, static_cast<uint16_t>(Ram16(player + kPairRow) + kPushRow[turn & 3]));
        SetRam8(player + 0x397, 1);
        if (kPushRow[turn & 3] < 0) {
            cell = PairCell(player);
            if (Ram16(cell + 2 * kBelow) != 0) {
                SetRam8(player + kTurnWay, 0);
                SetRam16(player + kKicksUp, static_cast<uint16_t>(Ram16(player + kKicksUp) + 1));
                if (static_cast<int16_t>(Ram16(player + kKicksUp)) > 2) {
                    SetRam16(player + kKicksUp, 5);
                    SetRam8(player + kStuck, 1);
                    SetRam8(player + kResting, 1);
                }
            } else {
                SetRam16(player + kKicksUp, 0);
                SetRam8(player + kStuck, 0);
            }
        }
        SetRam8(player + 0x1A3, 1);
        turn = (Ram8(player + kPairTurn) + 1) & 3;
        SetRam8(player + kPairTurn, static_cast<uint8_t>((Ram8(player + kPairTurn) + Ram8(player + kTurnWay)) & 3));
    }
    void MakeRoomToTurnRoutine(M68K &c) {
        uint32_t cell = c.a[0];
        uint16_t turn = static_cast<uint16_t>(c.d[0]);
        const uint16_t asked = turn;
        MakeRoomToTurn(c, cell, turn);
        c.a[0] = cell;
        if (Ram8(c.a[6] + 0x1A3)) {
            c.d[0] = turn;
        } else {
            SetLow16(c.d[0], asked);
        }
    }

    // 011D06 and 011DB8. A turn button: the pair's other gem goes a quarter of the way round, if the place is free.
    void Turn(M68K &c, bool right) {
        const uint32_t player = c.a[6];
        if (ComingIn(player) || Ram8(player + kStuck) || !(Pressed(player) & (right ? kTurnRight : kTurnLeft)) ||
            Ram8(player + kResting) == kRestFrames) {
            return;
        }
        Call(c, kTurnSound);
        uint32_t cell = PairCell(player);
        SetRam16(player + kTurningAcross, 0);
        SetRam16(player + kTurningDown, 0);
        SetRam16(player + kTurnPictureFrames, 8);
        const uint16_t here = Ram16(cell);
        uint16_t turn = (Ram8(player + kPairTurn) + (right ? 1 : 3)) & 3;
        SetRam8(player + kTurnWay, right ? 1 : 0);
        MakeRoomToTurn(c, cell, turn);
        const uint8_t shown = right ? 0 : 1;
        if (Ram8(player + kTurnShown) != 2) {
            SetRam8(player + kTurnShown, shown);
        }
        if (Ram8(player + kStuck)) {
            return;
        }
        if (static_cast<uint16_t>(here + Ram16(cell + kOther[turn & 3])) != 0) {
            return;
        }
        SetRam8(player + kPairTurn, static_cast<uint8_t>((Ram8(player + kPairTurn) + (right ? 1 : 3)) & 3));
        SetRam8(player + kTurnShown, shown);
    }
    void TurnLeft(M68K &c) {
        Turn(c, false);
    }
    void TurnRight(M68K &c) {
        Turn(c, true);
    }

    // The stick to one side moves the pair a column, if both its gems have room: at once when the stick is
    // first pressed, and again and again once it has been held for kRepeatAfter frames.
    constexpr uint16_t kStickRight = 0x0001, kStickLeft = 0x0002;
    constexpr int16_t kRepeatAfter = 12;
    constexpr uint32_t kMoveSound = 0x02B9DE;
    void MoveSideways(M68K &c, bool right) {
        const uint32_t player = c.a[6];
        const uint16_t way = right ? kStickRight : kStickLeft;
        const uint32_t held = player + (right ? kHeldRight : kHeldLeft);
        if (Pressed(player) & way) {
            SetRam16(held, 0);
        } else if (Ram16(player + kPad) & way) {
            SetRam8(player + 0x19A, 1);
            SetRam16(held, static_cast<uint16_t>(Ram16(held) + 1));
            if (static_cast<int16_t>(Ram16(held)) < kRepeatAfter) {
                return;
            }
        } else {
            return;
        }
        const int32_t side = right ? 2 : -2;
        const int32_t other = kOther[Ram8(player + kPairTurn) & 3];
        const uint16_t column = static_cast<uint16_t>(Ram16(player + kPairColumn) + (right ? 1 : -1));
        // A pair that has only just come in, with a gem beside the place it is coming down to, can still
        // slip across over that gem: it is put back up a row
        const bool justIn = Ram16(player + kPairColumn) == kStartColumn && Ram16(player + kPairRow) == 3 &&
                            static_cast<int16_t>(Ram16(player + kSlide)) > -12 &&
                            Ram16(Ram32(player + kFieldBottom) + kRowSize + 2 + 2 * kStartColumn + side) != 0;
        if (justIn) {
            const uint32_t cell = PairCell(player) - kRowSize;
            if (static_cast<uint16_t>(Ram16(cell + side) + Ram16(cell + other + side)) != 0) {
                return;
            }
            SetRam16(player + kPairRow, 2);
            SetRam16(player + kSlide, 0xFFFF);
        } else {
            const uint32_t cell = PairCell(player);
            if (static_cast<uint16_t>(Ram16(cell + side) + Ram16(cell + other + side)) != 0) {
                return;
            }
            if (Ram16(player + kPairRow) == 2) {
                SetRam16(player + kSlide, 0xFFFF);
            }
        }
        SetRam16(player + kPairColumn, column);
        Call(c, kMoveSound);
    }
    // 0120F8. (not in a frame in which turning has pushed the pair)
    void MovePair(M68K &c) {
        if (Ram8(c.a[6] + 0x397)) {
            return;
        }
        MoveSideways(c, true);
        MoveSideways(c, false);
    }
    void MoveRight(M68K &c) {
        MoveSideways(c, true);
    }
    void MoveLeft(M68K &c) {
        MoveSideways(c, false);
    }

    // The pair goes down a row
    void GoDown(uint32_t player) {
        SetRam16(player + kPairRow, static_cast<uint16_t>(Ram16(player + kPairRow) + 1));
        if (Ram8(player + kWasResting)) {
            SetRam8(player + kFallTimer, 10);
        }
        SetRam16(player + kSlide, Ram16(player + kPushSpeed));
        SetRam8(player + kWasResting, 0);
    }

    // The feature "hard_drop" (not the game's): a push of the stick up puts the pair straight down on whatever
    // is under it, and it lands this frame, with no rest in which to steer it further. A person's push, or the
    // computer's (ComputerSteers pushes up, with the feature on, where it would push the pair down).
    void HardDrop(uint32_t player) {
        if (!(Pressed(player) & kStickUp) || ComingIn(player)) {
            return;
        }
        if (Ram8(player + kComputer)) {
            // (the computer's push is let go at once: nothing else of its lets go before its next pair comes in)
            SetRam16(player + kPad, Ram16(player + kPad) & ~kStickUp);
        }
        while (static_cast<uint8_t>(UnderPair(player)) == 0) {
            SetRam16(player + kPairRow, static_cast<uint16_t>(Ram16(player + kPairRow) + 1));
        }
        SetRam16(player + kSlide, 0);
        SetRam8(player + kResting, 1); // (a rest in its last frame: SteerPair lands the pair)
    }

    // 01166C. A frame of the pair: it falls, rests, lands; and the player steers it.
    void SteerPair(M68K &c) {
        const uint32_t player = c.a[6];
        if (FeatureOn("hard_drop")) {
            HardDrop(player);
        }
        bool steer = true; // (false once the pair has landed)
        bool fall = true;  // the frame's fall is still to be worked out
        if (Ram8(player + kResting)) {
            if (static_cast<uint8_t>(UnderPair(player)) == 0) {
                // What it rested on has gone (or it was moved off it): it falls again
                SetRam8(player + kResting, 0);
                SetRam8(player + kWasResting, 1);
                SetRam16(player + 0x188, 0);
                SetRam16(player + 0x34E, 0);
                SetRam16(player + kPad, Ram16(player + kPad) & 0xFFF0);
                GoDown(player);
                fall = false;
            } else {
                SetRam8(player + kResting, static_cast<uint8_t>(Ram8(player + kResting) - 1));
                if (Ram8(player + kResting) == 0) {
                    if (static_cast<uint8_t>(UnderPair(player)) == 0) {
                        GoDown(player);
                        fall = false;
                    } else {
                        // The rest is over: the pair lands, and the turn goes on to its next step
                        LandPair(c);
                        Spend(c, kLandingTime);
                        Call(c, kLandSound);
                        if (Ram16(player + kPad) == kStickDown) {
                            SetRam32(player + kScore, Ram32(player + kScore) + 10);
                        }
                        SetRam16(player + kPushRows, 1);
                        SetRam16(player + kKicksUp, 0);
                        SetRam8(player + kStuck, 0);
                        SetRam8(player + 0x1A3, 0);
                        SetRam16(player + kTurnFrames, 0);
                        SetRam16(player + kSlide, 0);
                        SetRam8(player + kNoteChanged, 1);
                        SetRam8(player + kNoteBusy, 1);
                        SetRam8(player + kPairTurn, 0);
                        SetRam8(player + kRests, 0);
                        SetRam8(player + kStep, static_cast<uint8_t>(Ram8(player + kStep) + 1));
                        SetFallSpeedFor(c);
                        SetRam16(player + kPairRow, kStartRow);
                        SetRam16(player + kPairColumn, kStartColumn);
                        steer = fall = false;
                    }
                }
            }
        }
        if (fall) {
            SetRam8(player + kNoteFalling, 0);
            SetRam16(player + kTurnFrames, static_cast<uint16_t>(Ram16(player + kTurnFrames) + 1));
            SetRam8(player + 0x19A, 0);
            if (Ram8(player + kComputer)) {
                Call(c, kComputerSteers);
            }
            PushDown(c);
            // The fall's timer (it stands still while the pair rests)
            SetRam8(player + kFallTimer, static_cast<uint8_t>(Ram8(player + kFallTimer) + Ram8(player + kResting) - 1));
            if (static_cast<int8_t>(Ram8(player + kFallTimer)) < 0) {
                if (Ram8(player + kWasResting)) {
                    SetRam8(player + kFallTimer, 10);
                    SetRam8(player + kWasResting, 0);
                } else {
                    SetRam8(player + kFallTimer, Ram8(player + kFallFrames));
                }
                bool between = false; // the pair is shown part of the way between two rows
                if (Ram16(player + kSlide) != 0) {
                    SetRam16(player + kSlide, static_cast<uint16_t>(Ram16(player + kSlide) + Ram16(player + kSlideStep)));
                    const int16_t slide = static_cast<int16_t>(Ram16(player + kSlide));
                    if (slide != 0) {
                        between = true;
                        if (slide > 0) {
                            SetRam16(player + kSlide, 0xFFFC);
                            SetRam16(player + kSlideStep, 4);
                        }
                    }
                }
                if (!between && !Ram8(player + kResting)) {
                    if (static_cast<uint8_t>(UnderPair(player)) == 0) {
                        GoDown(player);
                    } else {
                        // Something is under it: it rests
                        SetRam8(player + kWasResting, 1);
                        SetRam8(player + kResting, kRestFrames);
                        SetRam8(player + kRests, static_cast<uint8_t>(Ram8(player + kRests) + 1));
                        if (Ram8(player + kRests) == kRestsMost) {
                            SetRam8(player + kResting, 1);
                        }
                    }
                }
            }
        }
        if (steer) {
            SetRam8(player + 0x397, 0);
            if (Ram8(player + kResting) != 1) {
                SwapPair(c);
                TurnRight(c);
                TurnLeft(c);
                MovePair(c);
            }
        }
        Call(c, kDrawPair);
    }
} // namespace

// For the landing guide (training.cpp): where the player's pair is, as columns and rows of the field (rows
// from the bottom), and its two gems. False when no pair is being steered.
bool PairPlace(uint32_t player, int columns[2], int rows[2], uint16_t gems[2]) {
    if (Ram8(player + kStep) != kStepNewPair) {
        return false;
    }
    const int turn = Ram8(player + kPairTurn) & 3;
    columns[0] = static_cast<int16_t>(Ram16(player + kPairColumn));
    rows[0] = kFieldRows - static_cast<int16_t>(Ram16(player + kPairRow));
    columns[1] = columns[0] + (turn == 1 ? 1 : turn == 3 ? -1 : 0);
    rows[1] = rows[0] + (turn == 0 ? 1 : turn == 2 ? -1 : 0);
    gems[0] = Ram16(player + kPairNow);
    gems[1] = Ram16(player + kPairNow + 2);
    return true;
}

namespace {
    // 0123BE. The picture of a pair turning: for the eight frames after a turn (kTurnPictureFrames counts
    // them down) the gem that turns is drawn on its way round, at a place taken from a table: one table for
    // turns to the right and one for turns to the left, a row for each place the gem can be going to, and in
    // a row eight distances across and eight down. (The left table's fifth row is for a pair whose gems
    // change places instead: kShownNone.)
    constexpr uint32_t kTurnPlacesRight = 0x012DA0, kTurnPlacesLeft = 0x012D00;
    constexpr uint32_t kPlacesRow = 32, kPlacesDown = 16;
    constexpr uint8_t kShownNone = 2;
    void TurnPicture(M68K &c) {
        const uint32_t player = c.a[6];
        if (Ram16(player + kTurnPictureFrames) == 0) {
            return;
        }
        Spend(c, 180);
        const uint16_t frames = static_cast<uint16_t>(Ram16(player + kTurnPictureFrames) - 1);
        SetRam16(player + kTurnPictureFrames, frames);
        const uint8_t shown = Ram8(player + kTurnShown);
        const uint32_t row = shown == kShownNone ? 4 : Ram8(player + kPairTurn);
        const uint32_t place = (shown == 0 ? kTurnPlacesRight : kTurnPlacesLeft) + ((row * kPlacesRow + frames * 2u) & 0xFFFF);
        SetRam16(player + kTurningAcross, Mem16(c, place));
        SetRam16(player + kTurningDown, Mem16(c, place + kPlacesDown));
    }
} // namespace

void RegisterSteer() {
    TheGame().features.push_back({"hard_drop", "Game", "Up drops the pair",
                                  "Pushing the stick up puts the pair you are steering straight down where it would come to rest, "
                                  "and it lands at once. The computer does it too, where it would push the pair down.",
                                  kFeatureEnhanced});
    AddRewrite(0x0123BE, "TurnPicture", TurnPicture, kFreeD0 | kFreeD1 | kFreeA0 | kFreeFlags, 22);
    // (the last number is the time a call is charged: the average the check measured for the game's own code)
    constexpr uint32_t kScratch = kFreeD0 | kFreeD1 | kFreeD2 | kFreeD3 | kFreeD4 | kFreeD5 | kFreeD6 | kFreeD7 | kFreeA0 | kFreeA1 |
                                  kFreeA2 | kFreeA3 | kFreeA4 | kFreeFlags;
    AddRewrite(0x011916, "PairCell", PairCellRoutine, kFreeFlags, 290);
    AddRewrite(0x011948, "UnderPair", UnderPairRoutine, kFreeD1 | kFreeFlags, 500);
    AddRewrite(0x011974, "LandPair", LandPair, kScratch, 620);
    AddRewrite(0x011BBE, "PushDown", PushDown, kScratch, 100);
    AddRewrite(0x011C80, "SwapPair", SwapPair, kScratch, 100);
    AddRewrite(0x011E6A, "MakeRoomToTurn", MakeRoomToTurnRoutine, kFreeD1 | kFreeD3 | kFreeD4 | kFreeA1 | kFreeA4 | kFreeFlags, 150);
    AddRewrite(0x011D06, "TurnLeft", TurnLeft, kScratch, 105);
    AddRewrite(0x011DB8, "TurnRight", TurnRight, kScratch, 105);
    AddRewrite(0x012108, "MoveRight", MoveRight, kScratch, 105);
    AddRewrite(0x0121FA, "MoveLeft", MoveLeft, kScratch, 105);
    AddRewrite(0x0120F8, "MovePair", MovePair, kScratch, 277);
    AddRewrite(0x01166C, "SteerPair", SteerPair, kScratch, 977);
}

} // namespace cps2recomp
