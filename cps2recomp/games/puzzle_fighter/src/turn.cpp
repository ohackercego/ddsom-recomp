// SPDX-License-Identifier: GPL-3.0-only
// Copyright (C) 2026 Tommy McLeroy
//
// Super Puzzle Fighter II Turbo: a player's turn, step by step, and a frame of the two players' turns,
// written again as C++ (rewrites: lift.h). In the steps A6 is the player's record.
#include "spf2.h"

namespace cps2recomp {

bool DummyStandsBy(); // training.cpp
bool ViewerStandsBy(); // modes.cpp

namespace {
    using namespace spf2;

    // The game's routines used here that are not written again
    constexpr uint32_t kRandom = 0x02DB02, kShowDigits = 0x011208, kShowDigitsPlayer2 = 0x01120C, kShowWarning = 0x0142D2,
                       kAttackSound = 0x02BB0A, kLostToAttack = 0x025A48;
    // ... and those written again in the other files
    constexpr uint32_t kSteerPair = 0x01166C, kNextPair = 0x011A1A, kFallGems = 0x00CEC4, kTidyField = 0x00ECAA, kCountField = 0x00D76E,
                       kCountDownCounterGems = 0x00D8BC, kTurnCounterGems = 0x00D824, kMarkBreaks = 0x00D462, kStartBreaks = 0x00D3D6,
                       kDiamondBreaks = 0x00D26A, kBigGemBonusRoutine = 0x00F8E0, kScoreBreak = 0x00FAB0, kDropCounterGems = 0x00CCC6,
                       kDropLooseGems = 0x00D058, kMakePictureCodes = 0x00D9AE, kClearBanner = 0x00C948, kShowBanner = 0x00C8F8,
                       kShowChainBanner = 0x00C89E;
    // ... and in this one
    constexpr uint32_t kStartPlayer = 0x00BF88, kAfterTurns = 0x00C05E, kShowScore = 0x00C106, kSetWarningSize = 0x00C13E,
                       kCheckLost = 0x00C79E;
    constexpr uint32_t kMakeBigGems[3] = {0x00DCD6, 0x00E0EE, 0x00E8BC}, kJoinBigGems[3] = {0x00F060, 0x00EEB0, 0x00ECEC};
    // The computer player's routines that choose where its next pair goes
    constexpr uint32_t kComputerPlans[3] = {0x013620, 0x012424, 0x012314};

    constexpr uint32_t kBannerTimer = 0x381;     // byte
    constexpr uint32_t kFlash = 0x32A;           // word: counts 1 to 3 and stops
    constexpr uint32_t kSecondPart = 0x342;      // word: frames of the round's current second
    constexpr uint16_t kFramesASecond = 60, kRoundSecondsMost = 539;
    constexpr uint32_t kPushScore = 0x376;       // long: points earned while the pair fell, added when it has landed
    constexpr uint32_t kCounted = 0x252;         // byte: the landing's count of the field has been made
    constexpr uint32_t kWarned = 0x396;          // byte: the turn's warning of an attack has been given
    constexpr uint32_t kDroppedAlready = 0x18D;  // byte: the opponent's counter gems are not to come in this turn
    constexpr uint32_t kDoomed = 0x260;          // byte: the attack that is coming is more than the field has room for
    // The attack waiting to fall on the opponent, as the opponent is shown it: the number (99 at most) and
    // how big the warning is (0 none, 1, 2 over kWarnBig, 3 over kWarnHuge)
    constexpr uint32_t kAttackShown = 0x27A, kWarningSize = 0x16E; // word (in the opponent's record), byte
    constexpr uint8_t kAttackShownMost = 99, kWarnBig = 10, kWarnHuge = 30;
    constexpr uint32_t kAttackNoted = 0x244;     // byte: the player's attack as it was when the opponent's turn noted it
    // How the attacker's character on the screen acts as its attack comes in (bigger numbers for bigger attacks)
    constexpr uint32_t kCharacterAct = 0xD0;     // byte
    constexpr uint32_t kComputerPressure = 0x1B0, kComputerPressureLater = 0x1B2; // words (see ShowBreak)
    // A round lost: the field's rows go out one by one from the top, kRowsLeft counting them
    constexpr uint32_t kRowsLeft = 0x354, kRowOut = 0x360, kLostFrames = 0x1A0; // byte, a byte a row, byte
    constexpr int32_t kFields[2] = {0x2B00, 0x2D00}; // from A5: the two players' fields, from their top rows
    constexpr uint32_t kFloorsTime = 380; // (what the game's code takes to put the floors back)
    constexpr uint16_t kStickRight = 0x0001, kStickLeft = 0x0002;
    constexpr uint16_t kHeldLong = 15;
    constexpr uint32_t kScorePlace = 0x9007E4, kScorePlacePlayer2 = 0x900D64; // where on the screen the scores go

    void NextStep(uint32_t player) {
        SetRam8(player + kStep, static_cast<uint8_t>(Ram8(player + kStep) + 1));
    }

    // --- the steps of a turn (kStep; each is a frame's work, and moves the step on when it is done) ---

    // 00C176. kStepNewPair: the turn's working is cleared, and the pair falls as the player steers it (every
    // frame until it lands: SteerPair moves the step on).
    void StepNewPair(M68K &c) {
        const uint32_t player = c.a[6];
        SetRam8(Var(c, kFieldsBusy), 0);
        SetRam8(player + kCounted, 0);
        SetRam8(player + kAllClearGiven, 0);
        SetRam8(player + 0x261, 0);
        SetRam16(player + kBreakListAt, 0);
        SetRam16(player + 0x256, 0);
        SetRam16(player + kBreakKindBefore, 0);
        SetRam16(player + kBreakKind, 0);
        SetRam16(player + kRowsFallen, 0);
        SetRam16(player + kDropColumn, 0);
        SetRam8(player + kChainStarted, 0);
        SetRam8(player + kRowFilled, 0);
        SetRam8(player + kWarned, 0);
        Call(c, kRandom);
        SetRam8(player + kColumnOrder, c.d[0] & 7);
        if ((Ram16(Var(c, kFrameCount)) & 0x3F) == 0) {
            SetRam8(player + kRedrawBigGems, 1);
        }
        Call(c, kSteerPair);
    }

    // 00C2C6. kStepPairFalls, kStepFall, kStepFallAgain: a frame of gems falling (FallGems moves the step on
    // when all have landed).
    void StepFall(M68K &c) {
        SetRam8(Var(c, kFieldsBusy), 1);
        Call(c, kFallGems);
        Call(c, kTidyField);
    }

    // 00C2D8. kStepCount: the points for pushing the pair down; the field is counted and the counter gems
    // count down, once; and those that have run out turn into gems, which takes some frames.
    void StepCount(M68K &c) {
        const uint32_t player = c.a[6];
        SetRam32(player + kScore, Ram32(player + kScore) + Ram32(player + kPushScore));
        SetRam32(player + kPushScore, 0);
        if (!Ram8(player + kCounted)) {
            if (!Ram8(player + kTurnTimer)) {
                SetRam8(player + kMadeCount, 0);
                Call(c, kCountField);
                Call(c, kCountDownCounterGems);
            }
            Call(c, kTurnCounterGems);
        }
        if (!Ram8(player + kTurnTimer)) {
            NextStep(player);
            SetRam8(player + kCounted, 1);
        }
    }

    // A pass of making big gems (not while a break is under way), and the pictures made to match
    void MakeBigGems(M68K &c, int pass) {
        const uint32_t player = c.a[6];
        NextStep(player);
        if (!Ram8(player + kBreaking)) {
            Call(c, kMakeBigGems[pass]);
        }
        Call(c, kMakePictureCodes);
        Call(c, kTidyField);
    }
    // 00C1FA. kStepMakeBig, kStepMakeBigLast
    void StepMakeBig(M68K &c) {
        MakeBigGems(c, 0);
        SetRam8(c.a[6] + kRedrawBigGems, 1);
    }
    // 00C218. kStepMakeBig2
    void StepMakeBig2(M68K &c) {
        MakeBigGems(c, 1);
        SetRam8(c.a[6] + kRedrawBigGems, 1);
    }
    // 00C236. kStepMakeBig3
    void StepMakeBig3(M68K &c) {
        MakeBigGems(c, 2);
    }
    // 00C1E8. (step 5: a frame in which nothing is done)
    void StepWait(M68K &c) {
        NextStep(c.a[6]);
    }

    // 00C28E. kStepFindBreaks: gems marked the frame before start to break; the diamond breaks its colour;
    // and the marks spread a step further (MarkBreaks, which sends the step back to kStepFall while it
    // finds anything).
    void StepFindBreaks(M68K &c) {
        const uint32_t player = c.a[6];
        NextStep(player);
        Call(c, kStartBreaks);
        Call(c, kDiamondBreaks);
        SetRam8(player + 0x395, 0);
        Call(c, kMarkBreaks);
    }

    // 00C2B0. kStepBigBonus
    void StepBigBonus(M68K &c) {
        const uint32_t player = c.a[6];
        NextStep(player);
        if (Ram16(player + kBreakKind)) {
            SetRam16(player + kBreakKindBefore, Ram16(player + kBreakKind));
        }
        Call(c, kBigGemBonusRoutine);
    }

    // 00F72A. kStepShowBreak. While a break is being shown: counts its frames, and at the last takes the
    // gems away and scores them (ScoreBreak), and goes back to kStepFall for what was resting on them: the
    // next break of a chain is found from there. With no break to show, the turn goes on.
    void ShowBreak(M68K &c) {
        const uint32_t player = c.a[6];
        if (Ram8(player + kBreakTimer)) {
            SetRam8(player + kNoteFalling, 1);
            SetRam8(player + kBreakTimer, static_cast<uint8_t>(Ram8(player + kBreakTimer) - 1));
            if (Ram8(player + kBreakTimer)) {
                return;
            }
            SetRam8(player + kStep, kStepFall);
            SetRam16(player + kBrokenCount, 0);
            SetRam16(player + kGemPoints, 0);
            SetRam16(player + kCrashPoints, 0);
            SetRam16(player + kCounterPoints, 0);
            for (uint32_t kind = 0; kind < 6; kind++) {
                SetRam8(player + kCrashKindsBroken + kind, 0);
            }
            SetRam8(player + kNoteBusy, 0);
            SetRam8(player + kNoteChanged, 1);
            SetRam8(player + kChainLength, static_cast<uint8_t>(Ram8(player + kChainLength) + 1));
            SetRam8(player + kNoCounterBreaks, 0);
            Call(c, kShowChainBanner);
            Call(c, kScoreBreak);
            SetRam8(player + kHalfAttack, 0);
            return;
        }

        // The turn's breaks are over. A number the computer player goes by (what for is not known yet) is
        // kept in the opponent's record: the chain's length times one less is added, then what ScoreBreak
        // put by if no counter gems are to come in this turn, and the sum is quartered (not to nothing).
        const uint32_t opponent = Ram32(player + kOpponent);
        const uint16_t chain = Ram8(player + kChainLength);
        const uint16_t forChain = static_cast<uint16_t>(chain * (chain ? chain - 1 : 0));
        if (!(forChain & 0x8000)) {
            SetRam16(opponent + kComputerPressure, static_cast<uint16_t>(Ram16(opponent + kComputerPressure) + forChain));
        }
        NextStep(player);
        SetRam8(player + 0x19F, 0);
        if (Ram8(player + kDroppedAlready)) {
            SetRam16(opponent + kComputerPressure,
                     static_cast<uint16_t>(Ram16(opponent + kComputerPressure) + Ram16(opponent + kComputerPressureLater)));
            SetRam16(opponent + kComputerPressureLater, 0);
        }
        const int16_t pressure = static_cast<int16_t>(Ram16(opponent + kComputerPressure));
        if (pressure != 0) {
            SetRam16(opponent + kComputerPressure, (pressure >> 2) > 0 ? static_cast<uint16_t>(pressure >> 2) : 1);
        }
    }

    // A pass of joining big gems
    void JoinBigGems(M68K &c, int pass) {
        NextStep(c.a[6]);
        Call(c, kJoinBigGems[pass]);
        Call(c, kMakePictureCodes);
        Call(c, kTidyField);
    }
    // 00C24C, 00C262, 00C278. kStepJoinBig and the two steps after it
    void StepJoinBig(M68K &c) {
        JoinBigGems(c, 0);
    }
    void StepJoinBig2(M68K &c) {
        JoinBigGems(c, 1);
    }
    void StepJoinBig3(M68K &c) {
        JoinBigGems(c, 2);
    }

    // 00C4F0. kStepNoteAttack
    void StepNoteAttack(M68K &c) {
        const uint32_t player = c.a[6], opponent = Ram32(player + kOpponent);
        NextStep(player);
        SetRam8(opponent + kAttackNoted, Ram8(opponent + kAttack));
    }

    // 00C500. kStepCountAgain
    void StepCountAgain(M68K &c) {
        NextStep(c.a[6]);
        Call(c, kCountField);
    }

    // 00C5E2. kStepAttacked: the opponent's counter gems come in over the field, unless the opponent has
    // lost, and once a turn only (kStepEnd comes back here while they fall)
    void StepAttacked(M68K &c) {
        const uint32_t player = c.a[6];
        NextStep(player);
        if (Ram8(Ram32(player + kOpponent) + kStep) == kStepLost) {
            return;
        }
        if (!Ram8(player + kDroppedAlready)) {
            Call(c, kDropCounterGems);
        }
        SetRam8(player + kDroppedAlready, 0);
    }

    // 00C50C. Called as an attack starts to come in (A6: the player it falls on), once a turn: how the
    // attacker's character acts, by the attack's size; and whether the attack is the end of the player,
    // which it is taken to be when its rows and the gems in the fourth column would reach the top.
    void WarnOfAttack(M68K &c) {
        const uint32_t player = c.a[6];
        if (Ram8(player + kWarned)) {
            return;
        }
        SetRam8(player + kWarned, 1);
        const uint32_t opponent = Ram32(player + kOpponent);
        c.a[2] = opponent;
        const uint16_t attack = Ram8(opponent + kAttack);
        if (attack == 0) {
            return;
        }
        SetRam8(opponent + kAttackNoted, 0);
        uint8_t act = attack < 12 ? 0 : attack < 18 ? 2 : attack < 24 ? 4 : attack < 30 ? 6 : 8;
        if (attack >= 30) {
            int empty = 0;
            while (Ram16(Ram32(player + kFieldBottom) + 2 + 2 * kStartColumn + static_cast<uint32_t>(empty) * kRowSize) == 0) {
                empty++;
            }
            if ((kFieldRows - 1 - empty) + attack / kFieldColumns >= kFieldRows - 1) {
                act = 10;
                SetRam8(player + kDoomed, 1);
            }
        }
        SetRam8(opponent + kCharacterAct, act);
        // (a mark of how full the field will be, which nothing reads)
        const uint16_t all = static_cast<uint16_t>(attack + Ram16(player + kGemCount));
        SetRam8(player + 0x253, all < 12 ? 0 : all < 30 ? 2 : 4);
        Call(c, kShowWarning);
    }

    // 00C79E. The round is lost when a turn ends with a gem in the fourth column's top row (where pairs
    // come in)
    void CheckLost(M68K &c) {
        const uint32_t player = c.a[6];
        if (Ram8(player + kResting) || Ram16(Cell(player, kStartColumn, kFieldRows - 2)) == 0) {
            return;
        }
        SetRam8(player + kStep, kStepLost);
        SetRam8(player + kLostFrames, 0);
        SetRam8(player + kRowsLeft, kFieldRows);
        SetRam8(player + kPairStarted, 0);
        SetRam16(player + kPairRow, 0);
        SetRam16(player + kPairNow, 0);
        SetRam16(player + kPairNow + 2, 0);
        Call(c, kClearBanner);
    }

    // 00C31E. kStepEnd. Counter gems that came in fall a row a frame (back through kStepCountAgain each
    // time). When nothing falls any more the turn ends: the round is lost, or everything is made ready for
    // the next pair.
    void StepEnd(M68K &c) {
        const uint32_t player = c.a[6];
        Call(c, kDropLooseGems);
        if (Ram8(player + kStep) != kStepNewPair) {
            Call(c, kTidyField);
            return;
        }
        if (Ram8(player + kAttacked)) {
            // The sound of the attack landing, by its size
            SetRam16(player + kFlash, 1);
            const int8_t fell = static_cast<int8_t>(Ram8(player + kFellThisTurn));
            c.d[0] = fell <= 6 ? 0 : fell <= 12 ? 1 : fell <= 18 ? 2 : fell <= 24 ? 3 : 4;
            Call(c, kAttackSound);
        }
        // What was left breaking in the three rows at the top goes
        for (int row = kFieldRows - 3; row < kFieldRows; row++) {
            for (int column = 0; column < kFieldColumns; column++) {
                const uint32_t cell = Cell(player, column, row);
                if (Ram16(cell) & kCellBreaking) {
                    SetRam16(cell, 0);
                    if (row != kFieldRows - 3) {
                        SetRam16(cell + kFieldExtra, 0);
                    }
                }
            }
        }
        Call(c, kCheckLost);
        if (Ram8(player + kStep) == kStepLost) {
            if (Ram8(player + kDoomed)) {
                SetLow16(c.d[1], 0xFFFF); // (the routine keeps what D1 holds: the game's loop counter, run out)
                Call(c, kLostToAttack);
            }
            Call(c, kTidyField);
            return;
        }

        // Where the next pair comes in: the fourth column, in the field's top row (the row above it when pairs
        // fall at their fastest, unless the cell under the top one is taken)
        SetRam16(player + kPairColumn, kStartColumn);
        SetRam16(player + kPairRow, Ram16(player + kPushSpeed) == 0xFFFE ? 1 : 2);
        if (Ram16(Cell(player, kStartColumn, kFieldRows - 3)) != 0) {
            SetRam16(player + kPairRow, 2);
            SetRam16(player + kSlide, 0xFFF0);
        }
        if (Ram8(player + kComputer)) {
            for (const uint32_t plan : kComputerPlans) {
                Call(c, plan);
            }
        }
        Call(c, kNextPair);
        SetRam8(player + 0x18E, 1);
        // (a stick held over from the turn before moves the new pair at once)
        if (Ram16(player + kPad) & kStickRight) {
            SetRam16(player + kHeldRight, kHeldLong);
        }
        if (Ram16(player + kPad) & kStickLeft) {
            SetRam16(player + kHeldLeft, kHeldLong);
        }
        Call(c, kCountField);
        SetRam8(player + kPushHeld, 0);
        SetRam8(player + kLanding, 0);
        SetRam8(player + kAttacked, 0);
        SetRam8(player + 0x322, 0);
        SetRam16(player + 0x20A, 0);
        SetRam8(player + kDoomed, 0);
        SetRam16(player + kChainBonus, 0);
        SetRam8(player + kChainLength, 0);
        SetRam16(player + kColourBonus, 0);
        SetRam16(player + kBrokenCount, 0);
        SetRam16(player + kBigGemBonus, 0);
        SetRam16(player + kGemPoints, 0);
        SetRam16(player + kCrashPoints, 0);
        SetRam16(player + kCounterPoints, 0);
        SetRam8(player + 0x349, 0);
        SetRam8(player + 0x370, 0);
        SetRam8(player + kNoCounterBreaks, 0);
        SetRam8(Ram32(player + kOpponent) + 0x1EE, 0);
        Call(c, kTidyField);
    }

    // 00C60A. kStepLost: the field's rows go out one a frame; then the opponent has won the round.
    void StepLost(M68K &c) {
        const uint32_t player = c.a[6];
        if (Ram8(player + kRowsLeft)) {
            const uint8_t left = static_cast<uint8_t>(Ram8(player + kRowsLeft) - 1);
            SetRam8(player + kRowsLeft, left);
            if (!(left & 0x80)) {
                SetRam8(player + kRowOut + left, 1);
                return;
            }
        }
        SetRam8(Var(c, kRoundWinners), Ram8(Ram32(player + kOpponent) + kPlayerBit));
    }

    constexpr uint32_t kTurnStepTable = 0x00C034;
    constexpr uint32_t kTurnSteps[] = {
        0x00C176, // kStepNewPair
        0x00C2C6, // kStepPairFalls
        0x00C2D8, // kStepCount
        0x00C2C6, // kStepFall
        0x00C1FA, // kStepMakeBig
        0x00C1E8, //   (a frame's wait)
        0x00C218, // kStepMakeBig2
        0x00C236, // kStepMakeBig3
        0x00C28E, // kStepFindBreaks
        0x00C2B0, // kStepBigBonus
        0x00C2C6, // kStepFallAgain
        0x00C4EA, // kStepShowBreak (ShowBreak)
        0x00C24C, // kStepJoinBig
        0x00C262, //   its second pass
        0x00C278, //   and third
        0x00C1FA, // kStepMakeBigLast
        0x00C4F0, // kStepNoteAttack
        0x00C500, // kStepCountAgain
        0x00C5E2, // kStepAttacked
        0x00C31E, // kStepEnd
        0x00C60A, // kStepLost
    };

    // --- what goes on around the steps ---

    // 00BF88. The round's first pair
    void StartPlayer(M68K &c) {
        const uint32_t player = c.a[6];
        if (Ram8(player + kPairStarted) || Ram8(player + kStep) == kStepLost) {
            return;
        }
        SetRam8(player + kPairStarted, 1);
        Call(c, kNextPair);
        SetRam16(player + kPairRow, 3);
    }

    // 00C13E. How big the warning of the player's waiting attack is
    void SetWarningSize(M68K &c) {
        const uint32_t player = c.a[6];
        const uint8_t attack = Ram8(player + kAttack) > kAttackShownMost ? kAttackShownMost : Ram8(player + kAttack);
        SetRam8(player + kWarningSize, attack == 0 ? 0 : attack <= kWarnBig ? 1 : attack <= kWarnHuge ? 2 : 3);
        c.d[0] = attack;
    }

    // 00C106. The player's score goes to the screen, and the attack the player has waiting is put where
    // the opponent's side shows it.
    void ShowScore(M68K &c) {
        const uint32_t player = c.a[6];
        const bool second = Ram8(player + kPlayerBit) == 2;
        c.d[0] = Ram32(player + kScore);
        c.a[1] = second ? kScorePlacePlayer2 : kScorePlace;
        Call(c, second ? kShowDigitsPlayer2 : kShowDigits);
        const uint32_t opponent = Ram32(player + kOpponent);
        c.a[2] = opponent;
        SetRam16(opponent + kAttackShown, Ram8(player + kAttack) > kAttackShownMost ? kAttackShownMost : Ram8(player + kAttack));
        Call(c, kSetWarningSize);
    }

    // 00C05E. After both turns: with both players out the banner goes; and every fourth frame the row of
    // walls under each field (and under each field's second) is put back.
    void AfterTurns(M68K &c) {
        if (Ram8(Player(0) + kStep) == kStepLost && Ram8(Player(1) + kStep) == kStepLost) {
            SetRam8(Var(c, kFieldsBusy), 0);
            Call(c, kClearBanner);
        }
        if (Ram16(Var(c, kFrameCount)) & 3) {
            return;
        }
        Spend(c, kFloorsTime);
        for (const int32_t field : kFields) {
            const uint32_t floor = Var(c, field) + kRowSize * kFieldRows;
            for (uint32_t cell = 0; cell < kRowSize; cell += 2) {
                SetRam16(floor + cell, kWall);
                SetRam16(floor + kFieldExtra + cell, kWall);
            }
        }
    }

    // 00BFAE. A frame of the player's turn: the step it is at; then what goes on whatever the step (a banner
    // over the field, the score, the round's time).
    void PlayerTurn(M68K &c) {
        const uint32_t player = c.a[6];
        SetRam8(player + kNoteChanged, 0);
        const uint8_t step = Ram8(player + kStep);
        if (step != kStepNewPair) {
            SetRam8(player + kRedrawBigGems, 1);
        }
        // (the registers as the game's own jump through its table leaves them)
        c.d[0] = step * 2u;
        SetLow16(c.d[1], kTurnSteps[step] - kTurnStepTable);
        Call(c, kTurnSteps[step]);

        if (Ram8(player + kBannerTimer)) {
            SetRam8(player + kBannerTimer, static_cast<uint8_t>(Ram8(player + kBannerTimer) - 1));
            Call(c, Ram8(player + kBannerTimer) ? kShowBanner : kClearBanner); // (the words over the field: banner.cpp)
        }
        Call(c, kShowScore);
        if (Ram16(player + kFlash)) {
            const uint16_t flash = static_cast<uint16_t>(Ram16(player + kFlash) + 1);
            SetRam16(player + kFlash, static_cast<int16_t>(flash) > 3 ? 0 : flash);
        }

        // The round's time goes on, a second every sixty frames, until the opponent has lost
        const uint32_t opponent = Ram32(player + kOpponent);
        c.a[2] = opponent;
        if (Ram8(opponent + kStep) == kStepLost) {
            return;
        }
        SetRam16(player + kSecondPart, static_cast<uint16_t>(Ram16(player + kSecondPart) + 1));
        if (static_cast<int16_t>(Ram16(player + kSecondPart)) < kFramesASecond) {
            return;
        }
        SetRam16(player + kSecondPart, 0);
        SetRam16(player + kRoundSeconds, static_cast<uint16_t>(Ram16(player + kRoundSeconds) + 1));
        if (static_cast<int16_t>(Ram16(player + kRoundSeconds)) > kRoundSecondsMost) {
            SetRam16(player + kRoundSeconds, kRoundSecondsMost);
        }
    }

    // 00BF48. A frame of a round being played: each player's turn, player 1's first.
    void BothPlayersTurns(M68K &c) {
        for (int n = 0; n < 2; n++) {
            const uint32_t player = (c.a[5] + 0x300 + static_cast<uint32_t>(n) * kPlayerSize) & 0xFFFFFF;
            c.a[6] = player;
            if ((n == 1 && DummyStandsBy()) || ViewerStandsBy()) {
                continue; // (training: player 2 does not play; the colour editor's viewer: nobody does)
            }
            if (!Ram8(player + kComputer)) {
                SetRam16(player + kPad, Ram16(player + kPadRead));
                SetRam16(player + kPadBefore, Ram16(player + kPadReadBefore));
            }
            Call(c, kStartPlayer);
            PlayerTurn(c);
        }
        Call(c, kAfterTurns);
    }
} // namespace

void RegisterTurn() {
    constexpr uint32_t kScratch = kFreeD0 | kFreeD1 | kFreeD2 | kFreeD3 | kFreeD4 | kFreeD5 | kFreeD6 | kFreeD7 | kFreeA0 | kFreeA1 |
                                  kFreeA2 | kFreeA3 | kFreeA4 | kFreeFlags;
    // (what the steps' routines leave in the registers is not the same from the rewritten ones; nothing after
    // a turn reads it. The last number is the time a call is charged, over and above the routines it calls:
    // the average the check measured for the game's own code)
    AddRewrite(0x00C176, "StepNewPair", StepNewPair, kScratch, 280);
    AddRewrite(0x00C2C6, "StepFall", StepFall, kScratch, 50);
    AddRewrite(0x00C2D8, "StepCount", StepCount, kScratch, 0);
    AddRewrite(0x00C1FA, "StepMakeBig", StepMakeBig, kScratch, 0);
    AddRewrite(0x00C1E8, "StepWait", StepWait, kScratch, 20);
    AddRewrite(0x00C218, "StepMakeBig2", StepMakeBig2, kScratch, 0);
    AddRewrite(0x00C236, "StepMakeBig3", StepMakeBig3, kScratch, 0);
    AddRewrite(0x00C28E, "StepFindBreaks", StepFindBreaks, kScratch, 0);
    AddRewrite(0x00C2B0, "StepBigBonus", StepBigBonus, kScratch, 55);
    AddRewrite(0x00F72A, "ShowBreak", ShowBreak, kScratch, 84);
    AddRewrite(0x00C24C, "StepJoinBig", StepJoinBig, kScratch, 0);
    AddRewrite(0x00C262, "StepJoinBig2", StepJoinBig2, kScratch, 0);
    AddRewrite(0x00C278, "StepJoinBig3", StepJoinBig3, kScratch, 0);
    AddRewrite(0x00C4F0, "StepNoteAttack", StepNoteAttack, kScratch, 56);
    AddRewrite(0x00C500, "StepCountAgain", StepCountAgain, kScratch, 0);
    AddRewrite(0x00C5E2, "StepAttacked", StepAttacked, kScratch, 140);
    AddRewrite(0x00C31E, "StepEnd", StepEnd, kScratch, 610);
    AddRewrite(0x00C60A, "StepLost", StepLost, kScratch, 90);
    AddRewrite(0x00C50C, "WarnOfAttack", WarnOfAttack, kScratch, 186);
    AddRewrite(0x00C79E, "CheckLost", CheckLost, kScratch, 60);
    AddRewrite(0x00BF88, "StartPlayer", StartPlayer, kScratch, 23);
    AddRewrite(0x00C13E, "SetWarningSize", SetWarningSize, kFreeFlags, 51);
    AddRewrite(0x00C106, "ShowScore", ShowScore, kScratch, 105);
    AddRewrite(0x00C05E, "AfterTurns", AfterTurns, kScratch, 58);
    AddRewrite(0x00BFAE, "PlayerTurn", PlayerTurn, kScratch & ~kFreeA2, 300);
    AddRewrite(0x00BF48, "BothPlayersTurns", BothPlayersTurns, kScratch, 700);
}

} // namespace cps2recomp
