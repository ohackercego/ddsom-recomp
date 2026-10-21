// SPDX-License-Identifier: GPL-3.0-only
// Copyright (C) 2026 Tommy McLeroy
//
// Super Puzzle Fighter II Turbo: the pairs of gems a player is given, written again as C++ (rewrites: lift.h).
// In all of these A6 is the player's record.
#include "spf2.h"

#include <cstdlib>

namespace cps2recomp {

namespace {
    using namespace spf2;

    // A gem of a pair is a word: its colour (1-4), 8 more for a crash gem, or kDiamond
    constexpr uint32_t kPairNext = 0x156; // two words, as kPairNow: the pair that comes next
    constexpr uint32_t kRandom = 0x144;                     // word: the player's own random numbers
    constexpr uint16_t kRandomStart = 0x1C3;
    constexpr uint32_t kPairsGiven = 0x1C8;                 // word: pairs the player has been given this round (99 at most)
    // Tables of gems to draw from, one set for each of the round's four colour orders (kColourOrder): for the
    // round's first pairs, which have no crash gems apart from the set's own few; and for the rest, where
    // about three in eight are crash gems. The second gem is drawn from a table's first 32 only.
    constexpr uint32_t kColourOrder = 0x380;                // byte
    constexpr uint32_t kFirstPairsFirst = 0x012578, kFirstPairsSecond = 0x012778, kPairs = 0x0127B8;
    // The diamond: it comes as the second gem of the pair whose number (kPairsMade) is kDiamondAt, and then
    // the next entry of kDiamondTimes says when the one after comes
    constexpr uint32_t kPairsMade = 0x1F4, kDiamondAt = 0x1F6, kDiamondsGiven = 0x1F8; // words
    constexpr uint32_t kDiamondTimes = 0x0124B8;
    // How many gems of each colour the player has been given (bytes, by colour): past kCrashOwed of one, the
    // next pair's first gem is made a crash gem
    constexpr uint32_t kColoursGiven = 0x350;
    constexpr int8_t kCrashOwed = 12;
    // From A5: not zero while the game plays itself to show how it is played (every first gem a crash gem)
    constexpr int32_t kShowingHow = 0x83;

    // 0122EC. The player's next random number, left in D0's low word.
    uint16_t PlayerRandom(uint32_t player) {
        uint16_t seed = Ram16(player + kRandom);
        if (seed == 0) {
            seed = kRandomStart;
        }
        const uint16_t tripled = static_cast<uint16_t>(seed * 3);
        const uint16_t next = static_cast<uint16_t>((tripled & 0xFF00) | ((seed + (tripled >> 8)) & 0xFF));
        SetRam16(player + kRandom, next);
        return next;
    }
    void PlayerRandomRoutine(M68K &c) {
        SetLow16(c.d[0], PlayerRandom(c.a[6]));
    }

    // 011B3A. A diamond has been given: when the next one comes.
    void NextDiamond(M68K &c) {
        const uint32_t player = c.a[6];
        const uint16_t given = static_cast<uint16_t>(Ram16(player + kDiamondsGiven) + 1);
        SetRam16(player + kDiamondsGiven, given);
        SetRam16(player + kDiamondAt, Mem16(c, kDiamondTimes + 2u * given));
        SetLow16(c.d[0], 2u * given);
        c.a[4] = kDiamondTimes;
    }

    // 011AB8. Counts the colours of the pair just made; a colour the player has had more than twelve of
    // makes the pair's first gem a crash gem. (The counts are kept by colour, 1 to 4, but the game looks
    // at the four from 0: so it is the count of the colour before that decides, and the crash gem given is
    // of the colour after.)
    void CountColours(M68K &c) {
        const uint32_t player = c.a[6];
        for (const uint32_t gem : {kPairNext, kPairNext + 2}) {
            const uint32_t count = player + kColoursGiven + (Ram16(player + gem) & kColour);
            SetRam8(count, static_cast<uint8_t>(Ram8(count) + 1));
        }
        for (uint16_t n = 0; n < 4; n++) {
            const uint32_t count = player + kColoursGiven + n;
            if (static_cast<int8_t>(Ram8(count)) > kCrashOwed) {
                SetRam8(count, 0);
                SetRam16(player + kPairNext, static_cast<uint16_t>(kCrash + 1 + n));
            }
        }
    }

    // Training: what the player asked the pairs to be
    void TrainingPairs(uint32_t player) {
        if (!FeatureOn("training_mode") || player != Player(0)) {
            return;
        }
        const int crash = FeatureValue("training_crash"); // 0 the game's, 1 one in every pair, 2 none
        uint16_t first = Ram16(player + kPairNext);
        const uint16_t second = Ram16(player + kPairNext + 2);
        if (crash == 1 && first < kCrash && (second < kCrash || second == kDiamond) && first != kDiamond) {
            first = static_cast<uint16_t>(first + kCrash);
        } else if (crash == 2) {
            if (first > kCrash) {
                first = static_cast<uint16_t>(first - kCrash);
            }
            if (second > kCrash) {
                SetRam16(player + kPairNext + 2, static_cast<uint16_t>(second - kCrash));
            }
        }
        SetRam16(player + kPairNext, first);
    }

    // 011A26. Makes the pair that will come after the one now coming.
    void MakeNextPair(M68K &c) {
        const uint32_t player = c.a[6];
        const uint32_t order = Ram8(player + kColourOrder);
        if (static_cast<int16_t>(Ram16(player + kPairsGiven)) <= 1) {
            // The round's first pairs. Two of one colour are two plain gems, never a crash gem with its own colour.
            uint16_t first = Mem16(c, kFirstPairsFirst + order * 64 + 2u * (PlayerRandom(player) & 0x1F));
            uint16_t second = Mem16(c, kFirstPairsSecond + 2u * (PlayerRandom(player) & 0x1F));
            const uint16_t firstColour = static_cast<int16_t>(first) > 7 ? first - 8 : first;
            const uint16_t secondColour = static_cast<int16_t>(second) > 7 ? second - 8 : second;
            if (firstColour == secondColour) {
                first = second = firstColour;
            }
            SetRam16(player + kPairNext, first);
            SetRam16(player + kPairNext + 2, second);
        } else {
            uint16_t first = Mem16(c, kPairs + order * 128 + 2u * (PlayerRandom(player) & 0x3F));
            if (Ram8(Var(c, kShowingHow)) && static_cast<int16_t>(first) < 7) {
                first = static_cast<uint16_t>(first + kCrash);
            }
            SetRam16(player + kPairNext, first);
            SetRam16(player + kPairNext + 2, Mem16(c, kPairs + order * 128 + 2u * (PlayerRandom(player) & 0x1F)));
        }

        const uint16_t made = static_cast<uint16_t>(Ram16(player + kPairsMade) + 1);
        SetRam16(player + kPairsMade, made);
        if (made == Ram16(player + kDiamondAt)) {
            SetRam16(player + kPairNext + 2, kDiamond);
            NextDiamond(c);
        }
        CountColours(c);
        // A crash gem does not come with another of its own kind
        const uint16_t first = Ram16(player + kPairNext);
        if (Ram16(player + kPairNext + 2) == first && static_cast<int16_t>(first) > kCrash) {
            SetRam16(player + kPairNext, static_cast<uint16_t>(first - kCrash));
        }
        TrainingPairs(player);
    }

    // 011A1A. The pair that was next is the one now coming, and a new next pair is made.
    void NextPair(M68K &c) {
        const uint32_t player = c.a[6];
        SetRam16(player + kPairNow, Ram16(player + kPairNext));
        SetRam16(player + kPairNow + 2, Ram16(player + kPairNext + 2));
        MakeNextPair(c);
    }
} // namespace

namespace {
    // --- how fast pairs fall ---
    // A pair goes down a step every so many frames (kFallFrames; kFallTimer counts them). The number comes
    // from tables: by how far the round has got (kFallStage, one more every twenty pairs, fifteen at most)
    // and the player's level; less a little for the level chosen at the start, until the round is two
    // minutes old. From four minutes on there is no wait at all.
    constexpr uint32_t kFallFrames = 0x18A, kFallTimer = 0x15E;          // bytes
    constexpr uint32_t kPairsInTen = 0x1A1;                              // byte: pairs landed, counted to ten
    constexpr uint32_t kPairsInStage = 0x184, kFallStage = 0x186;        // words
    constexpr uint16_t kPairsAStage = 20, kFallStageMost = 15, kPairsGivenMost = 99;
    constexpr uint32_t kSlideStep = 0x27C;                               // word, by the stage: table kSlideSteps
    constexpr uint32_t kSlideSteps = 0x012BD8, kFallByStage = 0x012C20, kFallByChoice = 0x012BF8;
    constexpr int32_t kSpeedChosen = 0xDF;                               // from A5: the level chosen (0 easy ...)
    constexpr uint8_t kLevelUsual = 2, kLevelFastest = 4;
    constexpr int16_t kFasterAfter = 120, kFastestAfter = 240;           // the round's seconds

    // 01182A. A pair has landed: counts it, and sets how fast the next falls.
    void SetFallSpeed(M68K &c) {
        const uint32_t player = c.a[6];
        const bool handicaps = Ram8(Var(c, kMode)) == kModeHandicap;
        // Training can hold player 1's speed where the round started, or slow it right down
        const int asked = FeatureOn("training_mode") && player == Player(0) ? FeatureValue("training_speed") : 0;

        SetRam8(player + kPairsInTen, static_cast<uint8_t>(Ram8(player + kPairsInTen) + 1));
        if (Ram8(player + kPairsInTen) == 10) {
            SetRam8(player + kPairsInTen, 0);
            const uint16_t given = static_cast<uint16_t>(Ram16(player + kPairsGiven) + 1);
            SetRam16(player + kPairsGiven, static_cast<int16_t>(given) > kPairsGivenMost ? kPairsGivenMost : given);
        }
        if (asked == 0) {
            SetRam16(player + kPairsInStage, static_cast<uint16_t>(Ram16(player + kPairsInStage) + 1));
            if (Ram16(player + kPairsInStage) == kPairsAStage) {
                SetRam16(player + kPairsInStage, 0);
                const uint16_t stage = static_cast<uint16_t>(Ram16(player + kFallStage) + 1);
                SetRam16(player + kFallStage, static_cast<int16_t>(stage) >= 16 ? kFallStageMost : stage);
            }
        }
        const uint16_t stage = Ram16(player + kFallStage);
        SetRam16(player + kSlideStep, Mem16(c, kSlideSteps + 2u * stage));

        const uint8_t level = handicaps && !Ram8(player + kComputer) ? Ram8(player + kHandicap) : kLevelUsual;
        uint8_t frames = Mem8(c, kFallByStage + ((stage + (level << 4)) & 0xFF));
        SetRam16(player + kPushSpeed, 0xFFF0);
        if (handicaps && Ram8(player + kHandicap) == kLevelFastest) {
            SetRam16(player + kPushSpeed, 0xFFFE);
        }
        const int16_t seconds = asked ? 0 : static_cast<int16_t>(Ram16(player + kRoundSeconds));
        uint8_t choice;
        if (seconds <= kFasterAfter) {
            choice = handicaps ? Ram8(player + kHandicap) : kLevelUsual;
        } else if (seconds <= kFastestAfter) {
            choice = 3;
        } else {
            choice = 4;
            SetRam16(player + kPushSpeed, 0xFFFE);
            frames = 0;
        }
        frames = static_cast<uint8_t>(frames + Mem8(c, kFallByChoice + ((choice * 8 + Ram8(Var(c, kSpeedChosen))) & 0xFF)));
        if (asked == 2) {
            frames = 40;
        }
        SetRam8(player + kFallFrames, frames);
        SetRam8(player + kFallTimer, frames);
    }
} // namespace

// For the checks (check_cover.sh): with SPF2_TEST_COLOURS=2 in the environment every gem dealt is blue or
// yellow, so that made-up play builds big gems and long chains, which it otherwise hardly ever does, and the
// rules for them get run. Done after each frame, outside the rewrites, so that the check of the routines
// that deal the pairs still holds. Never set in play.
void TestColoursFrame() {
    static const bool two = getenv("SPF2_TEST_COLOURS") != nullptr;
    if (!two) {
        return;
    }
    for (int n = 0; n < 2; n++) {
        for (const uint32_t gem : {kPairNext, kPairNext + 2}) {
            const uint16_t value = Ram16(Player(n) + gem);
            if (value != kDiamond && value != 0) {
                SetRam16(Player(n) + gem, static_cast<uint16_t>((value & kCrash) | (1 + (value & 1))));
            }
        }
    }
}

// (for steer.cpp)
void SetFallSpeedFor(M68K &c) {
    SetFallSpeed(c);
}

// Training's "A diamond next": the pair made next has the diamond
void DiamondNext(uint32_t player) {
    SetRam16(player + kDiamondAt, static_cast<uint16_t>(Ram16(player + kPairsMade) + 1));
}

void RegisterPair() {
    // (the last number is the time a call is charged: the average the check measured for the game's own code)
    constexpr uint32_t kScratch = kFreeD0 | kFreeD1 | kFreeD3 | kFreeD4 | kFreeA0 | kFreeA4 | kFreeFlags;
    AddRewrite(0x0122EC, "PlayerRandom", PlayerRandomRoutine, kFreeFlags, 132);
    AddRewrite(0x011B3A, "NextDiamond", NextDiamond, kFreeFlags, 66);
    AddRewrite(0x011AB8, "CountColours", CountColours, kFreeD0 | kFreeA0 | kFreeFlags, 242);
    AddRewrite(0x011A26, "MakeNextPair", MakeNextPair, kScratch, 957);
    AddRewrite(0x011A1A, "NextPair", NextPair, kScratch, 985);
    AddRewrite(0x01182A, "SetFallSpeed", SetFallSpeed, kFreeD0 | kFreeD1 | kFreeA0 | kFreeFlags, 424);
}

} // namespace cps2recomp
