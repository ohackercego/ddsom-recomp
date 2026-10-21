// SPDX-License-Identifier: GPL-3.0-only
// Copyright (C) 2026 Tommy McLeroy
//
// Super Puzzle Fighter II Turbo: the computer player's pace and its hands on the controls, written again as C++ (rewrites: lift.h). A6 is
// the computer's record.
#include "spf2.h"

namespace cps2recomp {


namespace {
    using namespace spf2;

    constexpr uint32_t kRandom = 0x02DB02;
    constexpr uint32_t kLevel = 0xA8;            // byte: how well this computer player plays (0 to 31: difficulty.cpp)
    constexpr uint32_t kCourse = kVars - 0x7374; // byte: the player's choice at the start: 0 EASY, 1 NORMAL, 2 HARD
    constexpr uint32_t kStage = kVars + 0x112;   // word: which opponent of the ladder
    // Its pace: it waits kThinkFrames before it steers a pair. The wait is from a table by its level, the
    // course and how far the round has gone (kPaceStage: a step every kPairsAStep pairs, fifteen at most), with
    // one of sixteen small differences added by chance
    constexpr uint32_t kThinkFrames = 0x31C;     // byte
    constexpr uint32_t kPaceStage = 0x31D, kPacePairs = 0x31E; // bytes
    constexpr uint8_t kPairsAStep = 10, kPaceStageMost = 15;
    constexpr uint32_t kThinkTable = 0x012E20, kThinkChanges = 0x012498;
    // Slips: two counts of frames drawn by chance for each pair from tables by its level. Once the pair is
    // where it is to go, the computer pushes it down for the first count and then leaves it to fall for the
    // second (ComputerSteers); the second, less 10 or 20 on NORMAL and HARD, is also taken off the wait
    constexpr uint32_t kBeforeLetGo = 0x37E, kLetGo = 0x37F; // bytes
    constexpr uint32_t kBeforeLetGoTable = 0x013220, kLetGoTable = 0x013420;
    constexpr uint32_t kPairsWaiting = 0x1C8;    // word: (more than one, and no slips are drawn)

    // --- steering ---
    // Where the pair is to go is worked out (PlanPair) as counts of presses: so many to
    // the right, to the left, and of each turn button. This plays them on the computer's controls.
    constexpr uint32_t kPressRight = 0x15A, kPressLeft = 0x15B, kPressTurnA = 0x15C, kPressTurnB = 0x31B; // bytes
    constexpr uint32_t kStartWait = 0x300, kGap = 0x301;   // bytes: frames before the first press, and to the next
    constexpr uint32_t kGapTable = 0x012C70;               // bytes: the gap, by course and stage
    // At the first stage the gap grows as the round goes on: kGapStep goes up one in every four presses, to
    // nine, and kGapMore is what it adds (table kGapMoreTable); later stages add the most whenever the
    // computer is in danger
    constexpr uint32_t kGapPresses = 0x30E, kGapStep = 0x310, kGapMore = 0x312; // words
    constexpr uint32_t kGapMoreTable = 0x012CA0;
    constexpr uint16_t kGapStepMost = 9;
    constexpr uint32_t kPushSound = 0x02BA0C;
    constexpr uint16_t kStickRightBit = 0x0001, kStickLeftBit = 0x0002, kStickDownBit = 0x0004, kStickUpBit = 0x0008, kTurnA = 0x0100, kTurnB = 0x0200;
    constexpr uint32_t kNoWaiting = kVars + 0x83;          // byte

    uint8_t PressesLeft(uint32_t player) {
        return static_cast<uint8_t>(Ram8(player + kPressRight) + Ram8(player + kPressLeft) + Ram8(player + kPressTurnA) +
                                    Ram8(player + kPressTurnB));
    }

    // 012082. The frames to the next press.
    void SetGap(M68K &c) {
        const uint32_t player = c.a[6];
        const uint16_t byCourse = Ram8(kCourse) * 16;
        SetRam8(player + kGap, Mem8(c, kGapTable + byCourse + Ram16(kStage)));
        bool step = false;
        if (Ram16(kStage) != 0) {
            if (Ram8(player + kDanger)) {
                SetRam16(player + kGapStep, kGapStepMost);
                SetRam8(player + kGap, 1);
                step = true;
            }
        } else {
            SetRam16(player + kGapPresses, static_cast<uint16_t>(Ram16(player + kGapPresses) + 1));
            if (Ram16(player + kGapPresses) == 4) {
                SetRam16(player + kGapStep, static_cast<uint16_t>(Ram16(player + kGapStep) + 1));
                if (Ram16(player + kGapStep) == kGapStepMost + 1) {
                    SetRam16(player + kGapStep, kGapStepMost);
                }
                step = true;
            }
        }
        if (step) {
            SetRam16(player + kGapMore, Mem16(c, kGapMoreTable + static_cast<uint16_t>(Ram16(player + kGapStep) * 2 + byCourse)));
            SetRam16(player + kGapPresses, 0);
        }
        SetRam8(player + kGap, static_cast<uint8_t>(Ram8(player + kGap) + Ram16(player + kGapMore)));
    }

    // 011F34. A frame of the computer's controls while its pair falls. With presses still to make: one of
    // each kind that is left, every so many frames (not until the pair is well into the field). With none
    // left: after its wait (kThinkFrames) it pushes the pair down, every other frame; but for a number of
    // frames drawn by chance (DrawSlips) it first pushes on (kBeforeLetGo) and then lets the pair fall by
    // itself (kLetGo).
    void ComputerSteers(M68K &c) {
        const uint32_t player = c.a[6];
        SetRam16(player + kPadBefore, Ram16(player + kPadBefore) & kStickDownBit);
        SetRam16(player + kPad, Ram16(player + kPad) & kStickDownBit);
        if (PressesLeft(player) != 0) {
            if (Ram8(player + kStartWait)) {
                SetRam8(player + kStartWait, static_cast<uint8_t>(Ram8(player + kStartWait) - 1));
                return;
            }
            SetRam8(player + kGap, static_cast<uint8_t>(Ram8(player + kGap) - 1));
            if (Ram8(player + kGap) != 0) {
                SetRam16(player + kPadBefore, 0);
                SetRam16(player + kPad, 0);
                return;
            }
            SetGap(c);
            if (static_cast<int16_t>(Ram16(player + kPairRow)) <= 2) {
                return;
            }
            if (Ram8(player + kPressTurnA)) {
                SetRam16(player + kPad, kTurnA);
                SetRam8(player + kPressTurnA, static_cast<uint8_t>(Ram8(player + kPressTurnA) - 1));
            }
            if (Ram8(player + kPressTurnB)) {
                SetRam16(player + kPad, kTurnB);
                SetRam8(player + kPressTurnB, static_cast<uint8_t>(Ram8(player + kPressTurnB) - 1));
            }
            if (Ram8(player + kPressRight)) {
                SetRam16(player + kPad, Ram16(player + kPad) | kStickRightBit);
                SetRam8(player + kPressRight, static_cast<uint8_t>(Ram8(player + kPressRight) - 1));
            }
            if (Ram8(player + kPressLeft)) {
                SetRam16(player + kPad, Ram16(player + kPad) | kStickLeftBit);
                SetRam8(player + kPressLeft, static_cast<uint8_t>(Ram8(player + kPressLeft) - 1));
            }
            // (the last press made, and no wait: the sound of pushing down)
            if (PressesLeft(player) == 0 && !Ram8(player + kThinkFrames)) {
                Call(c, kPushSound);
            }
            return;
        }

        if (!Ram8(kNoWaiting)) {
            if (Ram8(kCourse) != 2 && Ram16(kStage) == 0) {
                return; // (at the first stage it never pushes down, except on HARD)
            }
            if (Ram8(player + kThinkFrames)) {
                SetRam8(player + kThinkFrames, static_cast<uint8_t>(Ram8(player + kThinkFrames) - 1));
                return;
            }
        }
        if (FeatureOn("hard_drop")) {
            // ("Up drops the pair", not the game's: where it would push the pair down it drops it at once, as a
            // person can; SteerPair's HardDrop takes the push of the stick up at the next frame)
            SetRam16(player + kPad, kStickUpBit);
            return;
        }
        if (Ram8(player + kBeforeLetGo)) {
            SetRam8(player + kBeforeLetGo, static_cast<uint8_t>(Ram8(player + kBeforeLetGo) - 1));
        } else if (Ram8(player + kLetGo)) {
            SetRam16(player + kPad, 0);
            SetRam8(player + kLetGo, static_cast<uint8_t>(Ram8(player + kLetGo) - 1));
            if (Ram8(player + kLetGo) == 0) {
                Call(c, kPushSound);
            }
            return;
        }
        if ((Ram16(Var(c, kFrameCount)) & 1) == 0) {
            SetRam16(player + kPad, kStickDownBit);
        }
    }

    // --- planning ---
    // Before each of its pairs the computer works out the presses that take the pair where it wants it.
    // There are three ways of doing it, and each character has one of them.
    enum class Plan { Seeker, Matcher, Stacker };
    constexpr Plan kPlans[12] = {Plan::Seeker, Plan::Matcher, Plan::Seeker, Plan::Matcher, Plan::Matcher, Plan::Seeker,
                                 Plan::Stacker, Plan::Matcher, Plan::Seeker, Plan::Seeker, Plan::Matcher, Plan::Matcher};
    constexpr uint32_t kPairNext = 0x156;          // two words: the pair that comes next (pair.cpp)
    constexpr uint32_t kFound = 0x265;             // byte: a place has been found
    constexpr uint32_t kTurns = 0x262;             // byte: presses of the turn button the place needs
    constexpr uint32_t kPlace = 0x268;             // word: the place, a number into kPresses
    constexpr uint32_t kScore = 0x210;             // byte: how good the column being looked at is
    constexpr uint32_t kBestColumn = 0x26C, kBestScore = 0x26D, kFirstColumn = 0x26E, kFirstScore = 0x26F; // bytes
    constexpr uint32_t kTried = 0x270;             // two words: the pair as it is being tried (which gem under)
    constexpr uint32_t kPressesRight = 0x263, kPressesLeft = 0x264; // bytes
    // The presses for a place: to the right and to the left (bytes). Places 0 to 5 are the columns, counted
    // from the right (2 is the column pairs come in at); 8 to 15 and 16 to 23 are what is taken by chance
    constexpr uint32_t kPresses = 0x013C30;
    constexpr uint32_t kStackerKept = 0x356, kStackerTurn = 0x355; // word, byte
    constexpr int32_t kBelow = 16;
    constexpr uint16_t kDiamond = 5;
    // (the time the game's code takes over a look at every column, a search along them, and the rest of a plan)
    constexpr uint32_t kLookTime = 5000, kFindTime = 3700, kPlanTime = 330;

    uint32_t BottomLeft(uint32_t player) {
        return Ram32(player + kFieldBottom) + kBottomRow + 2;
    }
    void SetPresses(uint32_t player, uint8_t right, uint8_t left, uint8_t turns) {
        SetRam8(player + kPressRight, right);
        SetRam8(player + kPressLeft, left);
        SetRam8(player + kPressTurnA, turns);
    }

    // 013AA8. With gems up to the fifth row from the top of the middle, the pair goes to a side that is not
    // as high: right if the third column is, else left if the fifth is.
    void KeepFromTheTop(M68K &, uint32_t player) {
        const uint32_t top = Ram32(player + kFieldBottom);
        if (Ram16(top + 0x48) == 0) {
            return;
        }
        if (Ram16(top + 0x36) != 0) {
            SetPresses(player, 5, 0, 0);
        } else if (Ram16(top + 0x3A) != 0) {
            SetPresses(player, 0, 5, 0);
        }
    }
    // 013AFC. ... and does not go towards a side whose column next to the middle is high
    void KeepFromHighSide(M68K &, uint32_t player) {
        const uint32_t top = Ram32(player + kFieldBottom);
        if (Ram8(player + kPressRight)) {
            if (Ram16(top + 0x3A) != 0) {
                SetRam8(player + kPressRight, 0);
                SetRam8(player + kPressLeft, 3);
            }
        } else if (Ram8(player + kPressLeft) && Ram16(top + 0x36) != 0) {
            SetRam8(player + kPressRight, 3);
            SetRam8(player + kPressLeft, 0);
        }
    }

    // 013742. Scores every column for the pair standing upright in it, `lower` under `upper`: on what the
    // two gems under the first empty cell are. 1: the second one down is of the upper gem's colour (or the
    // column is full); 2: the one just under is of the lower gem's colour; 3 to 6: both, and more by whether
    // either is a crash gem's meeting. A column that is neither keeps the score of the column before it.
    // The best column (the first of the best) is noted.
    void ScoreColumns(uint32_t player, uint16_t upper, uint16_t lower) {
        SetRam8(player + kBestColumn, 0);
        SetRam8(player + kBestScore, 0);
        SetRam8(player + kScore, 0);
        for (int n = 5; n >= 0; n--) {
            const uint32_t bottom = BottomLeft(player) + 2u * static_cast<uint32_t>(5 - n);
            int row = 0;
            while (row < 10 && Ram16(bottom - static_cast<uint32_t>(row) * kBelow) != 0) {
                row++;
            }
            if (row == 10) {
                SetRam8(player + kScore, 1);
            } else {
                const uint32_t cell = bottom - static_cast<uint32_t>(row) * kBelow;
                const uint16_t under = Ram16(cell + kBelow), underThat = Ram16(cell + 2 * kBelow);
                const bool far = (underThat & kColour) == (upper & kColour), near = (under & kColour) == (lower & kColour);
                if (far && near) {
                    const bool farBig = (underThat & kKind) + (upper & kKind) > 8, nearBig = (under & kKind) + (lower & kKind) > 8;
                    SetRam8(player + kScore, farBig ? (nearBig ? 6 : 5) : (nearBig ? 4 : 3));
                } else if (far) {
                    SetRam8(player + kScore, 1);
                } else if (near) {
                    SetRam8(player + kScore, 2);
                }
            }
            if (static_cast<int8_t>(Ram8(player + kScore)) > static_cast<int8_t>(Ram8(player + kBestScore))) {
                SetRam8(player + kBestScore, Ram8(player + kScore));
                SetRam8(player + kBestColumn, static_cast<uint8_t>(n));
                SetRam8(player + kFound, 1);
            }
        }
    }

    // 013826. A place by chance
    void PlaceByChance(M68K &c, uint32_t player) {
        SetRam8(player + kFound, 1);
        SetRam8(player + kTurns, 1);
        Call(c, kRandom);
        SetRam16(player + kPlace, static_cast<uint16_t>((c.d[0] & 7) + 0x10));
    }

    // 01367E. The matcher's first look: the pair upright in each column, either way up, for gems of its
    // colours to land on. What it finds is thrown away for some columns (the entry column and the one at
    // the right-hand end; all the right half when the computer is in danger) and when the danger is great.
    // With nothing found it takes a place by chance, unless the pair has a diamond or pairs are waiting;
    // and one time in sixteen it takes a place by chance whatever it found (at the first stage only, as the
    // game means it: see below).
    void MatcherLooks(M68K &c) {
        const uint32_t player = c.a[6];
        Spend(c, kLookTime);
        SetRam8(player + kFound, 0);
        SetRam8(player + 0x26A, 0);
        SetRam8(player + kTurns, 0);
        const uint16_t first = Ram16(player + kPairNext), second = Ram16(player + kPairNext + 2);
        SetRam16(player + kTried, first);
        SetRam16(player + kTried + 2, second);
        ScoreColumns(player, first, second);
        SetRam8(player + kFirstColumn, Ram8(player + kBestColumn));
        SetRam8(player + kFirstScore, Ram8(player + kBestScore));
        SetRam16(player + kTried, second);
        SetRam16(player + kTried + 2, first);
        ScoreColumns(player, second, first);
        if (static_cast<int8_t>(Ram8(player + kBestScore)) < static_cast<int8_t>(Ram8(player + kFirstScore))) {
            SetRam8(player + kBestColumn, Ram8(player + kFirstColumn));
            SetRam8(player + kTurns, 2);
        }
        const uint16_t place = Ram8(player + kBestColumn);
        SetRam16(player + kPlace, place);
        if (place == 0 || place == 2 || (Ram8(player + kDanger) && place <= 3)) {
            SetRam8(player + kFound, 0);
        }
        // (the game asks whether the stage is the first, but gives the stage's place as a distance from A6,
        // the record, where it means the place itself: it looks at a word some way below the records)
        const uint32_t misplaced = (player + static_cast<uint32_t>(static_cast<int16_t>(kStage & 0xFFFF))) & 0xFFFFFF;
        bool byChance = false;
        if (Ram16(misplaced) == 0) {
            Call(c, kRandom);
            byChance = (c.d[0] & 0xF) == 0;
        }
        if (!byChance) {
            if (Ram8(player + kDanger) == 2) {
                SetRam8(player + kFound, 0);
            }
            if (Ram8(player + kFound) || Ram16(player + kPairNext) == kDiamond || Ram16(player + kPairsWaiting) != 0) {
                return;
            }
        }
        PlaceByChance(c, player);
    }

    // 013A0A. Looks along the columns, from the left, for the first whose landing cell has a gem of the
    // colour under it, or a crash gem of it under and to one side. The column is noted as the place, and
    // kScore says whether one was found. (The column pairs come in at only counts over a crash gem;
    // otherwise the cells above its landing cell are looked at in the same way.)
    void FindBeside(M68K &c, uint32_t player, uint16_t colour) {
        Spend(c, kFindTime);
        colour &= kColour;
        SetRam8(player + kScore, 0);
        for (int n = 5; n >= 0; n--) {
            const uint32_t bottom = BottomLeft(player) + 2u * static_cast<uint32_t>(5 - n);
            for (int row = 0; row < kFieldRows; row++) {
                const uint32_t cell = bottom - static_cast<uint32_t>(row) * kBelow;
                if (Ram16(cell) != 0) {
                    if (row == kFieldRows - 1) {
                        SetRam8(player + kScore, 0);
                    }
                    continue;
                }
                bool hit = (Ram16(cell + kBelow) & kColour) == colour;
                if (!hit) {
                    const uint16_t right = Ram16(cell + kBelow + 2), left = Ram16(cell + kBelow - 2);
                    if ((right & kColour) == colour) {
                        hit = (right & kKind) > 7;
                    } else if ((left & kColour) == colour) {
                        hit = (left & kKind) > 7;
                    }
                    if (!hit) {
                        break;
                    }
                }
                if (n == 2 && static_cast<int16_t>(Ram16(cell + kBelow)) <= 7) {
                    if (row == kFieldRows - 1) {
                        SetRam8(player + kScore, 0);
                    }
                    continue;
                }
                SetRam8(player + kScore, 1);
                SetRam16(player + kPlace, static_cast<uint16_t>(n));
                return;
            }
        }
        SetRam16(player + kPlace, 0);
    }

    // 013B3C. The matcher's plan. With nothing found by its first look: a column where one of the pair's
    // gems lands beside its colour (the second gem's by turning the pair over), else a place by chance. Then
    // the presses for the place, kept from the top and from a high side.
    void MatcherPlans(M68K &c) {
        const uint32_t player = c.a[6];
        Spend(c, kPlanTime);
        if (!Ram8(player + kFound)) {
            const uint16_t first = Ram16(player + kPairNext), second = Ram16(player + kPairNext + 2);
            bool found;
            if (static_cast<int16_t>(second) < 7) {
                FindBeside(c, player, first);
                found = Ram8(player + kScore) != 0;
                if (!found) {
                    FindBeside(c, player, second);
                    found = Ram8(player + kScore) != 0;
                    if (found) {
                        SetRam8(player + kTurns, 2);
                    }
                }
            } else {
                FindBeside(c, player, second);
                found = Ram8(player + kScore) != 0;
                if (found) {
                    SetRam8(player + kTurns, 2);
                } else {
                    FindBeside(c, player, second);
                    found = Ram8(player + kScore) != 0;
                }
            }
            if (!found) {
                Call(c, kRandom);
                SetRam16(player + kPlace, static_cast<uint16_t>((c.d[0] & 7) + 8));
            }
            if (second == kDiamond) {
                SetRam8(player + kTurns, 0);
            }
        }
        const uint32_t presses = kPresses + static_cast<uint16_t>(Ram16(player + kPlace) * 2);
        SetRam8(player + kPressesRight, Code8(c, presses));
        SetRam8(player + kPressesLeft, Code8(c, presses + 1));
        SetPresses(player, Ram8(player + kPressesRight), Ram8(player + kPressesLeft), Ram8(player + kTurns));
        SetRam8(player + kFound, 0);
        KeepFromHighSide(c, player);
        KeepFromTheTop(c, player);
        SetRam8(player + kStartWait, 1);
    }

    // 013846. The stacker's plan: it goes by the pair's colours against one colour it keeps (kStackerKept),
    // not by the field: pairs with that colour to the left, turned so that it comes where the others are,
    // and the rest to the right (to the left when the right is high).
    void StackerPlans(M68K &c) {
        const uint32_t player = c.a[6];
        Spend(c, kPlanTime);
        const uint16_t first = Ram16(player + kPairNext), second = Ram16(player + kPairNext + 2), kept = Ram16(player + kStackerKept);
        const auto otherTurn = [&]() {
            SetRam8(player + kStackerTurn, (Ram8(player + kStackerTurn) + 1) & 1);
            return Ram8(player + kStackerTurn) != 0;
        };
        if (first == second && first == kept) {
            SetPresses(player, 0, 1, 1);
        } else if (second == kept) {
            if (otherTurn()) {
                SetPresses(player, 1, 1, 1);
            } else {
                SetPresses(player, 0, 3, 3);
            }
        } else if (first == kept) {
            if (otherTurn()) {
                SetPresses(player, 0, 1, 3);
            } else {
                SetPresses(player, 0, 2, 1);
            }
        } else {
            SetPresses(player, 3, 0, 0);
            if (Ram16(Ram32(player + kFieldBottom) + 0x4A) != 0) {
                SetPresses(player, 0, 3, 0);
            }
        }
        KeepFromTheTop(c, player);
    }

    // --- the seeker ---
    // 015984. The seeker's plan: it scores every way the pair can be put down (upright in each column either
    // way up, and lying across each two columns either way round), by what each gem would land among, and
    // takes the best. Its sums are kept in the game's variables:
    constexpr uint32_t kLanding = 0xFF0CCC;      // six longs: in each column, the cell a gem would come to rest in
    constexpr uint32_t kDepth = 0xFF0CE4;        // six bytes: how many rows down that is
    constexpr uint32_t kOpponentLanding = 0xFF0D1C, kOpponentDepth = 0xFF0D34; // the same of the opponent's field
    constexpr uint32_t kGem = 0xFF0CEA, kOtherGem = 0xFF0D14; // words: the gem being scored, and the pair's other
    constexpr uint32_t kScores = 0xFF0D10;       // long: where the scores being worked out go
    constexpr uint32_t kWeights = 0xFF0D0C;      // long: the table that scores the gem's neighbours
    // The scores, a byte a column (and two bytes more): upright with the first gem under; with the second
    // under; lying with the first gem in the column and the second to its left; and the second to its right
    constexpr uint32_t kFirstUnder = 0xFF0CEC, kSecondUnder = 0xFF0CF4, kSecondLeft = 0xFF0CFC, kSecondRight = 0xFF0D04;
    constexpr uint32_t kBestSeekerColumn = 0xFF0CF2; // word (it lies over the end of kFirstUnder)
    constexpr uint32_t kCrowding = 0xFF0D16;     // word: how full the two fields are: 0, 4, 8 or 12 for each, added
    constexpr uint32_t kSharp = 0xFF0D1B;        // byte: the computer plays this pair its sharper way
    constexpr uint32_t kChosen = 0xFF0D18;       // three bytes: the presses chosen (left, right, turns)
    constexpr int32_t kShowThinking = 0x85;      // from A5, byte: a leftover of the game's making (see ShowThinking)
    // Tables. Some are read as the program is (Code8, Code16) and some as data (Mem8...): see spf2.h
    constexpr uint32_t kCrowdingTable = 0x01604C;   // bytes: by depth (twelve) and column (six, from the right)
    constexpr uint32_t kSharpChances = 0x0166C4;    // by character: words to rows of longs, by crowding; a bit in 32
    constexpr uint32_t kWeightSets = 0x015AAE;      // by character: words to rows of longs, the tables by kind of gem
    constexpr uint32_t kWeightSetOfGem = 0x015AC2;  // bytes: which of the row, by the gem (gems, crash gems, the diamond)
    constexpr uint32_t kSharpWeights = 0x40;        // (the sharper way's table follows the usual one)
    constexpr uint32_t kColumnScore = 0x015B2E;     // bytes: what a column scores to begin with, from the right
    constexpr uint32_t kNeighbours = 0x015B34;      // words: where the seven neighbours of a cell are, from it
    constexpr int kNeighbourCount = 7;
    constexpr uint32_t kColumnPresses = 0x015DA0;   // bytes: presses to the left and to the right, by column
    constexpr uint32_t kChanceTurns = 0x015DC2, kChancePresses = 0x015E34;
    // What a neighbour is to the gem, for the weights (a row of eight for each)
    enum : uint32_t { kIsEmpty = 0, kIsSame = 1, kIsSameColour = 2, kIsCounterOfColour = 4, kIsCounter = 5, kIsWall = 6, kIsBreaking = 7 };
    constexpr int kRowsLooked = 12;
    constexpr uint16_t kStageSeekerSure = 6;
    // (the time the game's code takes: the plan, and more for the sharper way)
    constexpr uint32_t kSeekerTime = 34500, kSharpTime = 2000;

    // 015F18. Where a gem would land in each column of a field, and how far down. (It looks at twelve rows
    // from the second from the top, which leaves out the bottom row: an empty column counts as if it had
    // one gem in it.)
    void FindLandings(uint32_t field, uint32_t landing, uint32_t depth) {
        for (int column = 0; column < kFieldColumns; column++) {
            const uint32_t top = Ram32(field + kFieldBottom) + 2 + 2u * static_cast<uint32_t>(column);
            int row = 0;
            while (row < kRowsLooked && Ram16(top + static_cast<uint32_t>(row) * kRowSize) == 0) {
                row++;
            }
            const int down = row == kRowsLooked ? kRowsLooked - 1 : row;
            const uint32_t cell = top + static_cast<uint32_t>(row) * kRowSize - kRowSize;
            SetRam32(landing + 4u * static_cast<uint32_t>(column), cell);
            SetRam8(depth + static_cast<uint32_t>(column), static_cast<uint8_t>(down));
        }
    }
    // 015F60. How full a field is, from its columns' depths: 0, 4, 8 or 12
    uint16_t Crowding(M68K &c, uint32_t depth) {
        uint16_t sum = 0;
        for (int column = 0; column < kFieldColumns; column++) {
            sum = static_cast<uint16_t>(sum + Mem8(c, kCrowdingTable + Ram8(depth + static_cast<uint32_t>(column)) * 6u +
                                                          static_cast<uint32_t>(5 - column)));
        }
        return sum < 0x10 ? 0 : sum < 0x20 ? 4 : sum < 0x30 ? 8 : 12;
    }
    // 015FA0. Whether this pair is played the sharper way: by chance, more often for some characters and
    // as the fields fill
    void DrawSharp(M68K &c, uint32_t player) {
        SetRam8(kSharp, 0);
        Call(c, kRandom);
        const uint32_t chance = c.d[0] & 0x1F;
        const uint32_t rows = kSharpChances + Mem16(c, kSharpChances + Ram8(player + kCharacter) * 2u);
        if (Mem32(c, rows + Ram16(kCrowding)) & (1u << chance)) {
            SetRam8(kSharp, 1);
        }
    }
    // 015FD6. With a switch that nothing in the game sets, the computer's thinking is shown as numbers in
    // the text layer by its field: the crowding, the sharper way, and its level
    void ShowThinking(M68K &c, uint32_t player) {
        if (!Ram8(Var(c, kShowThinking))) {
            return;
        }
        const uint32_t at = Ram8(player + kPlayerNumber) ? 0x900E28 : 0x9007A8;
        const auto put = [&](uint32_t where, uint16_t number) {
            c.Write(where, 1, number);
            c.Write(where + 2, 1, 0);
        };
        put(at, (Ram16(kCrowding) >> 2) & 0xF);
        put(at + 0x100, Ram8(kSharp) & 1);
        put(at + 0x200, (Ram8(player + kLevel) >> 4) & 1);
        put(at + 0x280, Ram8(player + kLevel) & 0xF);
    }

    // 015B42. What a neighbour of the landing cell adds to the score: by what it is to the gem, from the
    // weights. A gem of another colour adds nothing.
    uint8_t NeighbourScore(M68K &c, uint16_t cell, int neighbour) {
        const uint16_t gem = Ram16(kGem);
        uint32_t is;
        if ((cell & 0xFF) == kWall) {
            is = kIsWall;
        } else if ((cell & kCellBreaking) && (cell & kColour) == gem) {
            is = kIsBreaking;
        } else {
            const uint16_t kind = (cell & kCellBreaking) ? (cell & kColour) : (cell & kKind);
            if (kind == kEmpty) {
                is = kIsEmpty;
            } else if (kind == kCounter) {
                is = ((cell >> 12) & 0xF) == (gem & 0xFF) ? kIsCounterOfColour : kIsCounter;
            } else if ((cell & kKind) == gem) {
                is = kIsSame;
            } else if ((cell & kKind) == (gem & kColour)) {
                is = kIsSameColour;
            } else {
                return 0;
            }
        }
        return Mem8(c, Ram32(kWeights) + is * 8 + static_cast<uint32_t>(neighbour));
    }
    // 015BF4. Whether a gem beside where the pair's other gem will rest (on top of the first) is of its
    // colour. Crash gems only count the sharper way.
    bool BesideMatches(uint16_t beside, uint16_t other) {
        if (((other | beside) & kCrash) && !Ram8(kSharp)) {
            return false;
        }
        const uint8_t colour = (beside & kCrash) || (other & kCrash) ? (beside & kColour) : (beside & 0xFF);
        return colour == (other & kColour);
    }
    // 015A8C. The scores of the pair upright in each column, with kGem under kOtherGem: the column's own
    // score, four for each row down, what the neighbours of the landing cell add, and sixteen for each gem
    // of the other's colour beside where the other comes
    void ScoreUpright(M68K &c, uint32_t player, uint16_t gem, uint16_t other, uint32_t scores) {
        SetRam16(kGem, gem);
        SetRam16(kOtherGem, other);
        SetRam32(kScores, scores);
        const uint32_t sets = kWeightSets + Code16(c, kWeightSets + Ram8(player + kCharacter) * 2u);
        SetRam32(kWeights, Mem32(c, sets + Code8(c, kWeightSetOfGem + gem)) + (Ram8(kSharp) ? kSharpWeights : 0));
        for (int column = 0; column < kFieldColumns; column++) {
            const uint32_t cell = Ram32(kLanding + 4u * static_cast<uint32_t>(column));
            uint8_t score = Code8(c, kColumnScore + static_cast<uint32_t>(5 - column));
            score = static_cast<uint8_t>(score + (Ram8(kDepth + static_cast<uint32_t>(column)) << 2));
            for (int n = kNeighbourCount - 1; n >= 0; n--) {
                const uint32_t there = cell + static_cast<uint32_t>(static_cast<int16_t>(Code16(c, kNeighbours + 2u * static_cast<uint32_t>(n))));
                score = static_cast<uint8_t>(score + NeighbourScore(c, Ram16(there), n));
            }
            const uint32_t above = cell - kRowSize;
            score = static_cast<uint8_t>(score + (BesideMatches(Ram16(above - 2), other) ? 0x10 : 0) +
                                         (BesideMatches(Ram16(above + 2), other) ? 0x10 : 0));
            SetRam8(scores + static_cast<uint32_t>(column), score);
        }
    }
    // 015C6A. The sharper way's bonus: thirty-two where the gem would land on a pile of its colour that
    // rests on the other gem's colour, with a breaking gem there. (As the game has it this never comes
    // about: it asks for the mark of a cell that is breaking where a crash gem seems to be meant, and
    // nothing is breaking while the computer plans.)
    void ScoreSharp(uint16_t gem, uint16_t other, uint32_t scores) {
        SetRam16(kGem, gem);
        SetRam16(kOtherGem, other);
        SetRam32(kScores, scores);
        for (int column = 0; column < kFieldColumns; column++) {
            uint32_t cell = Ram32(kLanding + 4u * static_cast<uint32_t>(column)) + kRowSize;
            const uint16_t under = Ram16(cell);
            if (!(gem & kCellBreaking) && !(under & kCellBreaking)) {
                continue;
            }
            if ((under & kColour) != (gem & kColour)) {
                continue;
            }
            // (down the pile to the first cell of another colour, twelve rows at most)
            uint16_t colour = gem & kColour;
            for (int n = 0; n < kRowsLooked; n++) {
                cell += kRowSize;
                colour = Ram16(cell) & kColour;
                if (colour != (gem & kColour)) {
                    break;
                }
            }
            if (colour == (gem & kColour) && (gem & kCellBreaking)) {
                continue; // (the pile goes all the way)
            }
            if (colour != (other & kColour)) {
                continue;
            }
            const auto breaking = [&](uint16_t there) { return !(there & 0x8000) && ((other | there) & kCellBreaking); };
            const uint16_t right = Ram16(cell - kRowSize + 2);
            if (breaking(Ram16(cell)) || breaking(Ram16(cell - kRowSize - 2)) || breaking(right)) {
                SetRam8(scores + static_cast<uint32_t>(column), static_cast<uint8_t>(Ram8(scores + static_cast<uint32_t>(column)) + 0x20));
            }
        }
    }
    // 015A74. The score of the pair lying across two columns, from the two gems' upright scores there:
    // half their sum, and thirty-two more when both are good
    uint8_t ScoreLying(uint8_t one, uint8_t two) {
        const uint8_t half = static_cast<uint8_t>(static_cast<uint8_t>(one + two) >> 1);
        return one >= 0x40 && two >= 0x40 ? static_cast<uint8_t>(half + 0x20) : half;
    }

    void SeekerPlans(M68K &c) {
        const uint32_t player = c.a[6];
        Spend(c, kSeekerTime);
        FindLandings(player, kLanding, kDepth);
        FindLandings(0xFF0000 | Ram16(player + 0x34), kOpponentLanding, kOpponentDepth);
        SetRam16(kCrowding, static_cast<uint16_t>(Crowding(c, kDepth) + Crowding(c, kOpponentDepth)));
        DrawSharp(c, player);
        ShowThinking(c, player);

        const uint16_t first = Ram16(player + kPairNext), second = Ram16(player + kPairNext + 2);
        ScoreUpright(c, player, first, second, kFirstUnder);
        ScoreUpright(c, player, second, first, kSecondUnder);
        SetRam8(kSecondLeft, 0);
        SetRam8(kSecondRight + 5, 0);
        for (uint32_t column = 1; column < kFieldColumns; column++) {
            SetRam8(kSecondLeft + column, ScoreLying(Ram8(kSecondUnder + column - 1), Ram8(kFirstUnder + column)));
            SetRam8(kSecondRight + column - 1, ScoreLying(Ram8(kFirstUnder + column - 1), Ram8(kSecondUnder + column)));
        }
        if (Ram8(kSharp)) {
            Spend(c, kSharpTime);
            ScoreSharp(first, second, kFirstUnder);
            ScoreSharp(second, first, kSecondUnder);
        }

        // The best: of equals, the one looked at last (the ways in this order, each from the right)
        const struct { uint32_t scores; uint8_t turns; } ways[4] = {{kSecondUnder, 2}, {kSecondLeft, 1}, {kSecondRight, 3}, {kFirstUnder, 0}};
        uint8_t best = 0, turns = 0;
        uint16_t bestColumn = 0;
        for (const auto &way : ways) {
            for (int column = kFieldColumns - 1; column >= 0; column--) {
                const uint8_t score = Ram8(way.scores + static_cast<uint32_t>(column));
                if (score >= best) {
                    best = score;
                    bestColumn = static_cast<uint16_t>(column);
                    turns = way.turns;
                }
            }
        }
        SetRam16(kBestSeekerColumn, bestColumn);

        if (Ram16(kStage) < kStageSeekerSure && Ram16(kCrowding) == 0) {
            // Against the first opponents, while the fields are still empty enough, it throws the plan away:
            // the place goes by the count of frames
            const uint8_t frames = Ram8(Var(c, kFrameCount) + 1);
            SetRam8(player + kPressTurnA, Code8(c, kChanceTurns + (frames & 0xE)));
            SetRam8(player + kPressTurnB, Code8(c, kChanceTurns + (frames & 0xE) + 1));
            SetRam8(player + kPressLeft, Code8(c, kChancePresses + (frames & 0x1C)));
            SetRam8(player + kPressRight, Code8(c, kChancePresses + (frames & 0x1C) + 1));
            return;
        }
        SetRam8(player + kPressLeft, Code8(c, kColumnPresses + bestColumn * 2u));
        SetRam8(player + kPressRight, Code8(c, kColumnPresses + bestColumn * 2u + 1));
        SetRam8(player + kPressTurnA, turns);
        SetRam8(kChosen, Ram8(player + kPressLeft));
        SetRam8(kChosen + 1, Ram8(player + kPressRight));
        SetRam8(kChosen + 2, turns);
    }

    // 013620. The plan for the pair that comes next, the character's way.
    void PlanPair(M68K &c) {
        const uint32_t player = c.a[6];
        SetRam8(player + kStartWait, 1);
        switch (kPlans[Ram8(player + kCharacter) % 12]) {
        case Plan::Seeker:
            SeekerPlans(c);
            break;
        case Plan::Matcher:
            MatcherLooks(c);
            MatcherPlans(c);
            break;
        case Plan::Stacker:
            StackerPlans(c);
            break;
        }
    }
    void KeepFromTheTopRoutine(M68K &c) {
        KeepFromTheTop(c, c.a[6]);
    }
    void KeepFromHighSideRoutine(M68K &c) {
        KeepFromHighSide(c, c.a[6]);
    }

    // 012424. The wait before the next pair is steered.
    void SetThinkTime(M68K &c) {
        const uint32_t player = c.a[6];
        SetRam8(player + kPacePairs, static_cast<uint8_t>(Ram8(player + kPacePairs) + 1));
        if (static_cast<int8_t>(Ram8(player + kPacePairs)) >= kPairsAStep) {
            SetRam8(player + kPacePairs, 0);
            SetRam8(player + kPaceStage, static_cast<uint8_t>(Ram8(player + kPaceStage) + 1));
            if (static_cast<int8_t>(Ram8(player + kPaceStage)) >= kPaceStageMost) {
                SetRam8(player + kPaceStage, kPaceStageMost);
            }
        }
        const uint16_t entry = static_cast<uint16_t>(Ram8(player + kLevel) * 32 + Ram8(kCourse) * 8 + Ram8(player + kPaceStage));
        SetRam8(player + kThinkFrames, Mem8(c, kThinkTable + entry));
        Call(c, kRandom);
        const uint16_t change = Mem16(c, kThinkChanges + 2 * (c.d[0] & 0xF));
        const int16_t wait = static_cast<int16_t>(Ram8(player + kThinkFrames) + change);
        SetRam8(player + kThinkFrames, wait < 0 ? 0 : static_cast<uint8_t>(wait));
    }

    // 012314. The pair's slips.
    void DrawSlips(M68K &c) {
        const uint32_t player = c.a[6];
        SetRam8(player + kBeforeLetGo, 0);
        SetRam8(player + kLetGo, 0);
        if ((Ram8(kCourse) == 2 && static_cast<int16_t>(Ram16(kStage)) > 2) || static_cast<int16_t>(Ram16(player + kPairsWaiting)) > 1) {
            return;
        }
        Spend(c, 330);
        Call(c, kRandom);
        SetRam8(player + kBeforeLetGo, Mem8(c, kBeforeLetGoTable + (c.d[0] & 0xF) + (Ram8(player + kLevel) >> 4)));
        Call(c, kRandom);
        int hurry = Mem8(c, kLetGoTable + (c.d[0] & 0xF) + (Ram8(player + kLevel) >> 4));
        hurry -= Ram8(kCourse) == 2 ? 20 : Ram8(kCourse) == 1 ? 10 : 0;
        if (hurry < 0) {
            hurry = 0;
        }
        SetRam8(player + kLetGo, static_cast<uint8_t>(hurry));
        const int wait = Ram8(player + kThinkFrames) - hurry;
        SetRam8(player + kThinkFrames, wait < 0 ? 0 : static_cast<uint8_t>(wait));
    }
} // namespace

void RegisterComputer() {
    // (the last number is the time a call is charged; the plans count their own time)
    constexpr uint32_t kScratch = kFreeD0 | kFreeD1 | kFreeD2 | kFreeD3 | kFreeD4 | kFreeD5 | kFreeA0 | kFreeA1 | kFreeFlags;
    AddRewrite(0x013620, "PlanPair", PlanPair, kScratch | kFreeD6 | kFreeA2 | kFreeA3 | kFreeA4, 60);
    AddRewrite(0x01367E, "MatcherLooks", MatcherLooks, kScratch, 1);
    AddRewrite(0x013B3C, "MatcherPlans", MatcherPlans, kScratch, 1);
    AddRewrite(0x013846, "StackerPlans", StackerPlans, kScratch, 1);
    AddRewrite(0x015984, "SeekerPlans", SeekerPlans, kScratch | kFreeD6 | kFreeA2 | kFreeA3 | kFreeA4, 1);
    AddRewrite(0x013AA8, "KeepFromTheTop", KeepFromTheTopRoutine, kScratch, 50);
    AddRewrite(0x013AFC, "KeepFromHighSide", KeepFromHighSideRoutine, kScratch, 90);
    AddRewrite(0x012082, "SetGap", SetGap, kFreeD0 | kFreeD1 | kFreeA0 | kFreeFlags, 160);
    AddRewrite(0x011F34, "ComputerSteers", ComputerSteers, kFreeD0 | kFreeD1 | kFreeA0 | kFreeFlags, 210);
    AddRewrite(0x012424, "SetThinkTime", SetThinkTime, kFreeD0 | kFreeD1 | kFreeA0 | kFreeFlags, 262);
    AddRewrite(0x012314, "DrawSlips", DrawSlips, kFreeD0 | kFreeD1 | kFreeA0 | kFreeFlags, 90);
}

} // namespace cps2recomp
