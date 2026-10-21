// SPDX-License-Identifier: GPL-3.0-only
// Copyright (C) 2026 Tommy McLeroy
//
// Super Puzzle Fighter II Turbo: small routines that the whole program uses, written again as C++
// (rewrites: lift.h): the tasks' three pools, random numbers, a number into decimal digits, colours copied
// and dimmed, an animation stepped, a fade asked for, a few things put on the screen.
//
// They are called from hundreds of places, by code that goes on to use what they leave in the registers
// and the flags, so each leaves exactly what the game's own code does.
#include "spf2.h"

namespace cps2recomp {

namespace {
    using namespace spf2;

    void SetFlags(M68K &c, bool negative, bool zero) {
        c.flagN = negative;
        c.flagZ = zero;
        c.flagV = c.flagC = false;
    }
    void SetFlags32(M68K &c, uint32_t value) {
        SetFlags(c, (value & 0x80000000) != 0, value == 0);
    }
    void SetFlags16(M68K &c, uint32_t value) {
        SetFlags(c, (value & 0x8000) != 0, (value & 0xFFFF) == 0);
    }
    uint32_t Extended(uint16_t word) {
        return static_cast<uint32_t>(static_cast<int32_t>(static_cast<int16_t>(word)));
    }

    // --- tasks ---
    // A task is one of the things that move or wait on the screen, each with a record. There are three
    // pools of records; the free ones of each are on a stack of the low halves of their addresses.
    struct Pool {
        int32_t count, top; // from A5: a byte, how many are free; a word, the stack's top
        uint8_t kind;       // what a freed record's byte 8 is set to
        bool longRecord;    // (the first pool's records have more to clear)
    };
    constexpr Pool kPools[3] = {{0x581C, 0x580A, 0x08, true}, {0x581D, 0x580C, 0x0C, false}, {0x581E, 0x580E, 0x10, false}};

    // 02DA46, 02DA8A, 02DAC6. A free record of the pool, in A4; none (and the Z flag) when there is none.
    // (What makes a task of it is the caller's adding one to its first byte.)
    void NewTask(M68K &c, const Pool &pool) {
        const uint8_t free = Ram8(Var(c, pool.count));
        if (free == 0) {
            SetFlags(c, false, true);
            return;
        }
        SetRam8(Var(c, pool.count), static_cast<uint8_t>(free - 1));
        c.a[0] = Extended(Ram16(Var(c, pool.top)));
        c.a[4] = Extended(Mem16(c, c.a[0]));
        c.a[0] += 2;
        SetRam16(Var(c, pool.top), static_cast<uint16_t>(c.a[0]));
        SetFlags16(c, c.a[0]);
        c.flagX = false;
    }
    // 02DA5C, 02DAA0, 02DADC. The task in A6 is over: its record is cleared and put back.
    void EndTask(M68K &c, const Pool &pool) {
        const uint32_t task = c.a[6];
        c.d[0] = 0;
        for (uint32_t at = 0; at < 0x10; at += 4) {
            c.Write(task + at, 2, 0);
        }
        if (pool.longRecord) {
            c.Write(task + 0x36, 2, 0);
            c.Write(task + 0x3A, 0, 0);
        }
        c.Write(task + 8, 0, pool.kind);
        c.a[0] = Extended(Ram16(Var(c, pool.top))) - 2;
        c.Write(c.a[0], 1, task & 0xFFFF);
        SetRam16(Var(c, pool.top), static_cast<uint16_t>(c.a[0]));
        SetRam8(Var(c, pool.count), static_cast<uint8_t>(Ram8(Var(c, pool.count)) + 1));
    }
    void NewTask1(M68K &c) { NewTask(c, kPools[0]); }
    void NewTask2(M68K &c) { NewTask(c, kPools[1]); }
    void NewTask3(M68K &c) { NewTask(c, kPools[2]); }
    void EndTask1(M68K &c) { EndTask(c, kPools[0]); }
    void EndTask2(M68K &c) { EndTask(c, kPools[1]); }
    void EndTask3(M68K &c) { EndTask(c, kPools[2]); }

    // 016A3A, 016B78, 016BCA. A frame of every task of a pool: each has a routine, by its kind (byte 2 of
    // its record), which is called with the record in A6.
    struct TaskList {
        uint32_t first, size; // the pool's records, from A5
        uint8_t count;
        uint32_t routines;    // a long for each kind
    };
    constexpr TaskList kTaskLists[3] = {{0xD00, 0x80, 0x20, 0x016A68}, {0x1D00, 0x40, 0x10, 0x016BA6}, {0x2100, 0x200, 4, 0x016BF8}};
    constexpr int32_t kTasksToGo = 0x89; // byte
    constexpr uint32_t kTaskSlotTime = 52, kTaskCallTime = 64; // (the time the game's loop takes, about)
    void RunTasks(M68K &c, const TaskList &list) {
        c.a[6] = c.a[5] + list.first;
        SetRam8(Var(c, kTasksToGo), list.count);
        do {
            Spend(c, kTaskSlotTime);
            if (Mem8(c, c.a[6])) {
                c.a[0] = c.ReadProgram(list.routines + Mem8(c, c.a[6] + 2) * 4u, 2);
                c.d[0] = 0;
                c.d[7] = 0;
                Spend(c, kTaskCallTime);
                Call(c, c.a[0]);
            }
            c.a[6] += list.size;
            SetRam8(Var(c, kTasksToGo), static_cast<uint8_t>(Ram8(Var(c, kTasksToGo)) - 1));
        } while (Ram8(Var(c, kTasksToGo)) != 0);
    }
    void RunTasks1(M68K &c) { RunTasks(c, kTaskLists[0]); }
    void RunTasks2(M68K &c) { RunTasks(c, kTaskLists[1]); }
    void RunTasks3(M68K &c) { RunTasks(c, kTaskLists[2]); }

    // 02D8C8. The third pool made anew: its four records cleared, and all free.
    void ClearThirdPool(M68K &c) {
        constexpr uint32_t kRecords = 0x2100, kRecordSize = 0x200, kStackBottom = 0x588E;
        c.d[1] = 0;
        c.a[0] = c.a[5] + kStackBottom;
        c.a[6] = c.a[5] + kRecords;
        SetRam8(Var(c, kPools[2].count), 4);
        for (int n = 0; n < 4; n++, c.a[6] += kRecordSize) {
            for (uint32_t at = 0; at < kRecordSize; at += 4) {
                c.Write(c.a[6] + at, 2, 0);
            }
            c.Write(c.a[6] + 8, 0, kPools[2].kind);
            c.a[0] -= 2;
            c.Write(c.a[0], 1, c.a[6] & 0xFFFF);
        }
        c.a[1] = c.a[6];
        SetLow16(c.d[0], 0xFFFF);
        SetLow16(c.d[7], 0xFFFF);
        SetRam16(Var(c, kPools[2].top), static_cast<uint16_t>(c.a[0]));
        SetFlags16(c, c.a[0]);
    }

    // --- numbers ---

    // 02DB02. The game's random number: a byte, in D0. (The players' own are another: pair.cpp.)
    constexpr int32_t kSeed = 0xAE; // two bytes
    void Random(M68K &c) {
        const uint16_t seed = Ram16(Var(c, kSeed));
        const uint8_t stirred = static_cast<uint8_t>(static_cast<uint16_t>(seed * 3) >> 8);
        const unsigned sum = Ram8(Var(c, kSeed + 1)) + stirred;
        SetRam8(Var(c, kSeed + 1), static_cast<uint8_t>(sum));
        SetRam8(Var(c, kSeed), stirred);
        SetLow16(c.d[1], seed);
        SetLow16(c.d[0], sum & 0xFF);
        SetFlags(c, (sum & 0x80) != 0, (sum & 0xFF) == 0);
        c.flagX = sum > 0xFF;
    }

    // 02D0AE. The number in D0 as decimal digits, a digit to each four bits: sixteen digits, the last eight
    // of them in the long at kDecimal. (The game adds up powers of two kept as decimal digits.)
    constexpr uint32_t kDecimalHigh = 0xFF1354, kDecimal = 0xFF1358, kPowersOfTwo = 0x02D0F4;
    void ToDecimal(M68K &c) {
        uint32_t number = c.d[0];
        uint64_t digits = 0;
        for (int place = 0; place < 16; place++, number /= 10) {
            digits |= static_cast<uint64_t>(number % 10) << (4 * place);
        }
        SetRam32(kDecimalHigh, static_cast<uint32_t>(digits >> 32));
        SetRam32(kDecimal, static_cast<uint32_t>(digits));
        c.d[0] = 0;
        c.d[2] = 0xFFFF;
        c.a[2] = kPowersOfTwo + 8 * 32;
        c.a[3] = c.a[5] + (kDecimal + 4 - kVars);
    }

    // --- colours ---

    constexpr uint32_t kPairTime = 250; // (the time the game's code takes for a pair of rows, about)
    // 020F2C. D7 + 1 pairs of rows of colours copied from A0 to A1, at full brightness.
    void CopyColours(M68K &c) {
        constexpr uint32_t kBright = 0xF000F000;
        c.d[6] = kBright;
        const int pairs = static_cast<int>(c.d[7] & 0xFFFF) + 1;
        for (int n = 0; n < pairs * 2; n++) {
            for (int colour = 0; colour < 4; colour++, c.a[0] += 4, c.a[1] += 4) {
                c.d[colour] = Mem32(c, c.a[0]) | kBright;
                c.Write(c.a[1], 2, c.d[colour]);
            }
        }
        SetLow16(c.d[7], 0xFFFF);
        SetFlags32(c, c.d[3]);
        Spend(c, kPairTime * static_cast<uint32_t>(pairs));
    }
    // 02107E, 02107C. D7 + 1 pairs of rows of colours at A1 (thirty-two pairs) dimmed: only D6's bits are kept.
    void DimColours(M68K &c) {
        const int pairs = static_cast<int>(c.d[7] & 0xFFFF) + 1;
        for (int n = 0; n < pairs * 2; n++) {
            for (int colour = 0; colour < 4; colour++, c.a[1] += 4) {
                c.d[colour] = Mem32(c, c.a[1]) & c.d[6];
                c.Write(c.a[1], 2, c.d[colour]);
            }
        }
        SetLow16(c.d[7], 0xFFFF);
        SetFlags32(c, c.d[3]);
        Spend(c, kPairTime * static_cast<uint32_t>(pairs));
    }
    void DimColours32(M68K &c) {
        c.d[7] = 0x1F;
        DimColours(c);
    }
    // 0210AC. All the colours dimmed to a quarter.
    void DimAllColours(M68K &c) {
        c.d[6] = 0x3FFF3FFF;
        c.a[1] = 0x90C000;
        for (int n = 0; n < 4; n++) {
            DimColours32(c);
        }
    }

    // --- animations ---
    // A thing that is drawn (a task, a character) has at +1C the address of its picture's present frame,
    // and at +18 a copy of the frame's first long: frames still to wait (a word), a mark, a number.
    constexpr uint32_t kFrameLeft = 0x18, kFrame = 0x1C, kFrameSize = 0x10;
    constexpr uint32_t kNextFrameTime = 84, kLastFrameTime = 20; // (the time the game's code takes)

    void SetFrame(M68K &c, uint32_t thing) {
        c.Write(thing + kFrame, 2, c.a[0]);
        const uint32_t first = Mem32(c, c.a[0]);
        c.Write(thing + kFrameLeft, 2, first);
        SetFlags32(c, first);
    }
    // 02D29C. A frame of time goes by for A6's animation: on to its next frame when this one's time is up
    // (a frame marked as the last holds the address to go on from).
    void StepAnimation(M68K &c) {
        const uint32_t thing = c.a[6];
        const uint16_t was = Mem16(c, thing + kFrameLeft), left = static_cast<uint16_t>(was - 1);
        c.Write(thing + kFrameLeft, 1, left);
        c.flagX = c.flagC = was == 0;
        c.flagV = was == 0x8000;
        c.flagN = (left & 0x8000) != 0;
        c.flagZ = left == 0;
        if (left != 0) {
            return;
        }
        Spend(c, kNextFrameTime);
        c.a[0] = Mem32(c, thing + kFrame) + kFrameSize;
        if (Mem8(c, thing + kFrameLeft + 2) & 0x80) {
            c.a[0] = Mem32(c, c.a[0]);
            Spend(c, kLastFrameTime);
        }
        SetFrame(c, thing);
    }
    // 02D2B4. A6 starts animation number D0 of the list at A0.
    void StartAnimation(M68K &c) {
        const uint16_t doubled = static_cast<uint16_t>((c.d[0] & 0xFF) * 2);
        const uint16_t offset = Mem16(c, c.a[0] + doubled);
        SetLow16(c.d[0], offset);
        c.a[0] += Extended(offset);
        c.flagX = false;
        SetFrame(c, c.a[6]);
    }
    // 014390. A6, a character, starts act number D0 of its own list.
    constexpr uint32_t kActLists = 0x0143F6; // a long for each character
    void SetAct(M68K &c) {
        c.d[1] = Mem8(c, c.a[6] + kCharacter) * 4u;
        c.a[0] = c.ReadProgram(kActLists + c.d[1], 2);
        const uint32_t doubled = (c.d[0] & 0xFFFF) * 2;
        const uint16_t offset = Mem16(c, c.a[0] + Extended(static_cast<uint16_t>(doubled)));
        SetLow16(c.d[0], offset);
        c.a[0] += Extended(offset);
        c.flagX = doubled > 0xFFFF;
        SetFrame(c, c.a[6]);
    }

    // --- the screen ---

    // 006634. A fade is asked for (D0: which). D0 and the flags say whether it has started: not while one
    // is on. (It also clears byte 0A of whatever A6 is.)
    constexpr int32_t kFadeOn = 0x2900, kFadeKind = 0x2902;
    void StartFade(M68K &c) {
        if (Ram8(Var(c, kFadeOn))) {
            c.d[0] = 0;
            SetFlags(c, false, true);
            return;
        }
        SetRam8(Var(c, kFadeOn), 1);
        SetRam8(Var(c, kFadeKind), static_cast<uint8_t>(c.d[0]));
        c.Write(c.a[6] + 0x0A, 0, 0);
        c.d[0] = 0xFFFFFFFF;
        SetFlags(c, true, false);
    }
    // 00CC14. A tall letter (tile D0 and the one sixteen on) with colours D1, in the text layer at A0.
    void PutTall(M68K &c) {
        const uint16_t tile = static_cast<uint16_t>(c.d[0]), colours = static_cast<uint16_t>(c.d[1]);
        c.Write(c.a[0], 1, tile);
        c.Write(c.a[0] + 2, 1, colours);
        SetLow16(c.d[0], tile + 0x10u);
        c.Write(c.a[0] + 4, 1, static_cast<uint16_t>(tile + 0x10));
        c.Write(c.a[0] + 6, 1, colours);
        c.flagX = tile > 0xFFEF;
        SetFlags16(c, colours);
    }
    // 00BB8C. Picture D0 of the stage numbers' list (a block of tiles) drawn at A1.
    constexpr uint32_t kStagePictures = 0x08DC3C;
    void ShowStageNumber(M68K &c) {
        c.a[0] = kStagePictures + Extended(Mem16(c, kStagePictures + 2));
        const uint16_t index = static_cast<uint16_t>(static_cast<int16_t>(static_cast<int8_t>(c.d[0])) << 4);
        c.a[0] = Mem32(c, c.a[0] + Extended(index) + 4);
        const uint16_t columns = Mem16(c, c.a[0] + 2), rows = Mem16(c, c.a[0] + 4);
        c.a[0] += 6;
        SetLow16(c.d[2], columns);
        uint16_t last = 0, tile = 0;
        for (unsigned row = 0; row <= rows; row++, c.a[1] += 0x40) {
            c.a[2] = c.a[1];
            for (unsigned column = 0; column <= columns; column++, c.a[0] += 4, c.a[2] += 4) {
                tile = Mem16(c, c.a[0]);
                last = Mem16(c, c.a[0] + 2);
                c.Write(c.a[2], 1, static_cast<uint16_t>(tile + 0x5800));
                c.Write(c.a[2] + 2, 1, last);
            }
        }
        SetLow16(c.d[0], tile + 0x5800u);
        SetLow16(c.d[1], 0xFFFF);
        SetLow16(c.d[3], 0xFFFF);
        c.flagX = tile >= 0xA800;
        SetFlags16(c, last);
    }
    // 00BBC8, 00BBE8. The two characters taken off the screen (their colours made white), and put back.
    constexpr int32_t kCharactersOff = 0x125;
    constexpr uint32_t kCharacters[2] = {kVars + 0xB00, kVars + 0xC00};
    void HideCharacters(M68K &c) {
        SetRam8(Var(c, kCharactersOff), 0xFF);
        SetRam8(kCharacters[0] + 1, 0);
        SetRam8(kCharacters[1] + 1, 0);
        c.d[0] = 0xFFFFFFFF;
        c.d[1] = 0x0000FFFF;
        for (c.a[0] = 0x90CFC0; c.a[0] < 0x90CFE0; c.a[0] += 4) {
            c.Write(c.a[0], 2, 0xFFFFFFFF);
        }
        SetFlags(c, true, false);
    }
    void ShowCharactersAgain(M68K &c) {
        if (Ram8(Var(c, kCharactersOff)) == 0xFF) {
            SetRam8(Var(c, kCharactersOff), 0);
        }
        SetRam8(kCharacters[0] + 1, 1);
        SetRam8(kCharacters[1] + 1, 1);
        SetFlags(c, false, false);
    }
} // namespace

void RegisterCommon() {
    // (the last number is the time a call is charged)
    AddRewrite(0x02DA46, "NewTask", NewTask1, 0, 68);
    AddRewrite(0x02DA8A, "NewTask2", NewTask2, 0, 68);
    AddRewrite(0x02DAC6, "NewTask3", NewTask3, 0, 68);
    AddRewrite(0x02DA5C, "EndTask", EndTask1, kFreeFlags, 162);
    AddRewrite(0x02DAA0, "EndTask2", EndTask2, kFreeFlags, 134);
    AddRewrite(0x02DADC, "EndTask3", EndTask3, kFreeFlags, 134);
    constexpr uint32_t kScratch = kFreeD0 | kFreeD1 | kFreeD2 | kFreeD3 | kFreeD4 | kFreeD5 | kFreeD6 | kFreeD7 | kFreeA0 | kFreeA1 |
                                  kFreeA2 | kFreeA3 | kFreeA4 | kFreeA6 | kFreeFlags;
    AddRewrite(0x016A3A, "RunTasks", RunTasks1, kScratch, 22);
    AddRewrite(0x016B78, "RunTasks2", RunTasks2, kScratch, 22);
    AddRewrite(0x016BCA, "RunTasks3", RunTasks3, kScratch, 22);
    AddRewrite(0x02D8C8, "ClearThirdPool", ClearThirdPool, 0, 11560);
    AddRewrite(0x02DB02, "Random", Random, 0, 86);
    AddRewrite(0x02D0AE, "ToDecimal", ToDecimal, kFreeFlags, 1640);
    AddRewrite(0x020F2C, "CopyColours", CopyColours, 0, 20);
    AddRewrite(0x02107E, "DimColours", DimColours, 0, 10);
    AddRewrite(0x02107C, "DimColours32", DimColours32, 0, 10);
    AddRewrite(0x0210AC, "DimAllColours", DimAllColours, 0, 30);
    AddRewrite(0x02D29C, "StepAnimation", StepAnimation, 0, 26);
    AddRewrite(0x02D2B4, "StartAnimation", StartAnimation, 0, 78);
    AddRewrite(0x014390, "SetAct", SetAct, 0, 112);
    AddRewrite(0x006634, "StartFade", StartFade, 0, 68);
    AddRewrite(0x00CC14, "PutTall", PutTall, 0, 52);
    AddRewrite(0x00BB8C, "ShowStageNumber", ShowStageNumber, 0, 1800);
    AddRewrite(0x00BBC8, "HideCharacters", HideCharacters, 0, 200);
    AddRewrite(0x00BBE8, "ShowCharactersAgain", ShowCharactersAgain, 0, 50);
}

} // namespace cps2recomp
