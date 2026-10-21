// SPDX-License-Identifier: GPL-3.0-only
// Copyright (C) 2026 Tommy McLeroy
//
// Super Puzzle Fighter II Turbo: how well the computer plays, written again as C++ (rewrites: lift.h).
//
// The game keeps one number, the skill (0 to 255), and the computer player's level is an eighth of it (0 to
// 31: computer.cpp). It starts from the operator's difficulty setting, is held to what the setting allows
// at each stage, goes up when a match is over (more after a long one) and with a person's chains, and comes
// down a little after each match and a good deal with a continue.
#include "spf2.h"

namespace cps2recomp {

namespace {
    using namespace spf2;

    constexpr uint32_t kSkill = kVars - 0x733A;      // word
    constexpr uint16_t kSkillMost = 255;
    constexpr uint32_t kHoldOff = kVars - 0x7338;    // word: matches for which the skill is not held to the stage's
    constexpr uint32_t kMatchFrames = kVars - 0x7336; // word: frames of play in this match (RoundPlay counts them)
    constexpr uint32_t kLevelNow = kVars + 0x12E;    // word: the level the computer players are given
    constexpr uint32_t kSetting = kVars + 0xBE;      // byte: the operator's difficulty (0 to 7)
    constexpr uint32_t kStage = kVars + 0x112;       // word
    constexpr uint32_t kComputerLevel = 0xA8;        // byte, in a player's record
    constexpr uint32_t kContinues = 0x98;            // byte, in a player's record
    constexpr uint32_t kRandom = 0x02DB02;
    // Tables of words
    constexpr uint32_t kStartLevels = 0x015834;      // the level a game starts at, by the setting
    constexpr uint32_t kStageLevels = 0x015844;      // the stage's level, by the setting (eight stages each)
    constexpr uint32_t kLastStageLevels = 0x0158C4;  // ... and the last stage's
    constexpr uint32_t kEase = 0x015934;             // what comes off after a match, by the setting
    constexpr uint32_t kEaseByChance = 0x015944;     // ... and one of 32 by chance
    constexpr uint32_t kContinueDrops = 0x0158D4;    // what a continue takes off, by how many have been used
    constexpr uint32_t kContinueByChance = 0x0158F4; // ... and one of 32 added by chance
    constexpr uint32_t kChainRises = 0x015784;       // what a person's chain adds, by its length (read as program)
    constexpr uint16_t kContinueDropMost = 0x48;
    constexpr uint16_t kLastStageNumber = 8;

    // 015798. The skill kept between nothing and kSkillMost
    void ClampSkill(M68K &c) {
        const int16_t skill = static_cast<int16_t>(Ram16(kSkill));
        if (skill < 0) {
            SetRam16(kSkill, 0);
        } else if (skill >= 0x100) {
            SetRam16(kSkill, kSkillMost);
        }
        c.d[0] = 0;
    }
    // 015794. (D0: what to add)
    void AddSkill(M68K &c) {
        SetRam16(kSkill, static_cast<uint16_t>(Ram16(kSkill) + c.d[0]));
        ClampSkill(c);
    }
    // 015690.
    void LevelFromSkill(M68K &c) {
        SetLow16(c.d[0], Ram16(kSkill) >> 3);
        SetRam16(kLevelNow, Ram16(kSkill) >> 3);
    }
    // 01565A. A game begins
    void StartSkill(M68K &c) {
        const uint16_t level = Mem16(c, kStartLevels + 2u * Ram8(kSetting));
        SetRam16(kLevelNow, level);
        SetRam16(kSkill, static_cast<uint16_t>(level << 3));
        c.d[0] = static_cast<uint16_t>(level << 3);
        c.a[0] = kStartLevels;
    }

    // 0156C8. The skill held to the stage's level for the setting. At the four easier settings it may not be
    // above it, and at stages 2, 5, 7 and 8 not below it either; at the four harder ones it may not be below.
    // (Not for the match after a continue.)
    void HoldToStage(M68K &c) {
        if (Ram16(kHoldOff)) {
            SetRam16(kHoldOff, static_cast<uint16_t>(Ram16(kHoldOff) - 1));
            return;
        }
        const uint16_t setting = Ram8(kSetting), stage = Ram16(kStage);
        const uint16_t held = static_cast<uint16_t>(
            (stage >= kLastStageNumber ? Mem16(c, kLastStageLevels + 2u * setting) : Mem16(c, kStageLevels + ((16u * setting + 2u * stage) & 0xFFFF)))
            << 3);
        c.d[6] = (c.d[6] & 0xFFFF0000) | held;
        bool atLeast = setting >= 4;
        if (!atLeast) {
            if (held < Ram16(kSkill)) {
                SetRam16(kSkill, held);
            }
            atLeast = stage == 8 || stage == 7 || stage == 2 || stage == 5;
        }
        if (atLeast && held >= Ram16(kSkill)) {
            SetRam16(kSkill, held);
        }
    }

    // 015740. The computer players are given the level
    void GiveLevel(M68K &) {
        for (int n = 0; n < 2; n++) {
            if (Ram8(Player(n) + kComputer)) {
                SetRam8(Player(n) + kComputerLevel, Ram8(kLevelNow + 1));
            }
        }
    }

    // 015676. Before a match
    void LevelForMatch(M68K &c) {
        SetRam16(kMatchFrames, 0);
        HoldToStage(c);
        ClampSkill(c);
        LevelFromSkill(c);
        GiveLevel(c);
    }

    // 01569C. After a match the skill comes down: by the setting (most at the easiest), and by chance
    void EaseSkill(M68K &c) {
        SetRam16(kSkill, static_cast<uint16_t>(Ram16(kSkill) - Mem16(c, kEase + 2u * Ram8(kSetting))));
        Call(c, kRandom);
        SetRam16(kSkill, static_cast<uint16_t>(Ram16(kSkill) - Mem16(c, kEaseByChance + (c.d[0] & 0x3E))));
    }

    // 0157F0. A match is over (not with handicaps): the skill goes up, by how long the match's play took
    // (in steps of 256 frames: 16 under 60 of them, then 8, 4, 16, and 32 from 150 on), and then eases.
    void SkillAfterMatch(M68K &c) {
        if (Ram8(Var(c, kMode)) == kModeHandicap) {
            return;
        }
        const uint8_t time = Ram8(kMatchFrames);
        c.d[0] = time >= 150 ? 32 : time >= 120 ? 16 : time >= 90 ? 4 : time >= 60 ? 8 : 16;
        AddSkill(c);
        EaseSkill(c);
        ClampSkill(c);
        LevelFromSkill(c);
    }

    // 01575E. A person's chain (D0: its length; A6: the player) raises the skill: nothing under three, then
    // 1, 2, 4, and 8 for six or more.
    void SkillForChain(M68K &c) {
        if (Ram8(Var(c, kMode)) == kModeHandicap || Ram8(c.a[6] + kComputer)) {
            return;
        }
        const uint16_t length = (c.d[0] & 0xFFFF) >= 8 ? 7 : (c.d[0] & 0xFF);
        c.d[0] = Code16(c, kChainRises + 2u * length);
        AddSkill(c);
    }

    // 0157B4. A continue (A6: the player): the level is set as before a match, and then the skill comes
    // down, by more the more continues have been used, give or take a little by chance; and the next match
    // does not hold it to the stage's.
    void SkillAfterContinue(M68K &c) {
        LevelForMatch(c);
        SetRam16(kHoldOff, 1);
        const uint8_t continues = Ram8(c.a[6] + kContinues);
        const uint16_t drop = continues < 16 ? Mem16(c, kContinueDrops + 2u * continues) : kContinueDropMost;
        SetRam16(kSkill, static_cast<uint16_t>(Ram16(kSkill) - drop));
        Call(c, kRandom);
        c.d[0] = Mem16(c, kContinueByChance + (c.d[0] & 0x3E));
        AddSkill(c);
        ClampSkill(c);
    }
} // namespace

void RegisterDifficulty() {
    // (the last number is the time a call is charged)
    constexpr uint32_t kScratch = kFreeD0 | kFreeD1 | kFreeD6 | kFreeA0 | kFreeFlags;
    AddRewrite(0x015798, "ClampSkill", ClampSkill, kFreeFlags, 44);
    AddRewrite(0x015794, "AddSkill", AddSkill, kFreeFlags, 60);
    AddRewrite(0x015690, "LevelFromSkill", LevelFromSkill, kFreeFlags, 36);
    AddRewrite(0x01565A, "StartSkill", StartSkill, kFreeFlags, 78);
    AddRewrite(0x0156C8, "HoldToStage", HoldToStage, kScratch, 282);
    AddRewrite(0x015740, "GiveLevel", GiveLevel, kScratch, 66);
    AddRewrite(0x015676, "LevelForMatch", LevelForMatch, kScratch, 460);
    AddRewrite(0x01569C, "EaseSkill", EaseSkill, kScratch, 124);
    AddRewrite(0x0157F0, "SkillAfterMatch", SkillAfterMatch, kScratch, 400);
    AddRewrite(0x01575E, "SkillForChain", SkillForChain, kScratch, 150);
    AddRewrite(0x0157B4, "SkillAfterContinue", SkillAfterContinue, kScratch, 920);
}

} // namespace cps2recomp
