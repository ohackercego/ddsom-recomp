// SPDX-License-Identifier: GPL-3.0-only
// Copyright (C) 2026  Part of cps2recomp.
//
// Dungeons & Dragons: Shadow over Mystara: the game's cases, written again as C++.
//
// The game runs its play as a table of cases. A case is a list of steps; the game's own code for a
// case is a short list of things to do, each step carrying a number and three values. A structure in
// A0 says which case is running and which step of it the game is at.
//
// Two routines make it work, and they are written again here:
//
//   001A32  LoadCase. Takes the number of a case (in D0) and fills the structure in A0 with the case's
//           first step: the number of the step, what the step is, and the three values the step
//           carries. The steps of a case are in a table in the ROM that the case's number points at.
//   001A56  NextStep. Goes on to the next step of the case that is running: it takes one off the
//           number of the step left, and when there are none left it takes the next step from the
//           table. A step of the kind 0xFE ends the case.
//
// The routine at 001A32 is called from all over the game (479 call sites in the disassembly), and
// between them these two are what the game's play is made of.
//
// The table of cases is in the ROM as the code reads it (the deciphered program, not the work RAM), so
// the two are read differently: `Code16` and `Code32` for the ROM, `Mem16` and `Mem32` for the work
// RAM.
#include "ddsom.h"

#include "lift.h"

namespace cps2recomp {

namespace {
    using namespace ddsom;

    // The game's memory as the rewrite reads it.
    inline uint8_t Mem8(M68K &c, uint32_t address) {
        return static_cast<uint8_t>(c.Read(address, 0));
    }
    inline uint16_t Mem16(M68K &c, uint32_t address) {
        return static_cast<uint16_t>(c.Read(address, 1));
    }
    inline uint32_t Mem32(M68K &c, uint32_t address) {
        return c.Read(address, 2);
    }
    // The ROM as the program reads it where it names a table by its distance from the instruction
    // (deciphered: not what Mem16 sees at the same address).
    inline uint16_t Code16(M68K &c, uint32_t address) {
        return static_cast<uint16_t>(c.ReadProgram(address, 1));
    }
    inline uint32_t Code32(M68K &c, uint32_t address) {
        return c.ReadProgram(address, 2);
    }
    // The step of a case, copied into the structure in A0: the number of the step, what the step is,
    // and the three values the step carries. The pointer to the next step is left too, and the pointer
    // to it is what the routine leaves in A4.
    void CopyStep(M68K &c, uint32_t structure, uint32_t step) {
        SetRam16(structure + kCaseStepsLeft, Mem16(c, step));
        SetRam16(structure + kCaseKind, Mem16(c, step + 2));
        SetRam32(structure + kCaseLong1, Mem32(c, step + 4));
        SetRam32(structure + kCaseLong2, Mem32(c, step + 8));
        SetRam32(structure + kCaseLong3, Mem32(c, step + 12));
        SetRam32(structure + kCaseStep, step + 16);
        c.a[4] = step + 16;
    }

    // 001A32. A case is loaded: the structure in A0 is filled with the case's first step.
    void LoadCase(M68K &c) {
        // (the table of cases: the number of the case is a word's index into it, and the word there
        // says where the case's steps are, as an offset from A4)
        const uint32_t table = c.a[4] + static_cast<uint32_t>(c.d[0] & 0xFFFF) * 2;
        const uint16_t offset = Mem16(c, table);
        c.d[0] = offset;
        c.a[4] = c.a[4] + static_cast<uint32_t>(static_cast<int32_t>(static_cast<int16_t>(offset)));
        CopyStep(c, c.a[0], c.a[4]);
    }

    // 001A56. The case goes on to its next step.
    void NextStep(M68K &c) {
        const uint32_t structure = c.a[0];
        const uint16_t left = static_cast<uint16_t>(Mem16(c, structure + kCaseStepsLeft) - 1);
        SetRam16(structure + kCaseStepsLeft, left);
        if (left != 0) {
            // (the case is not at its end yet: the registers are left as they were)
            return;
        }
        c.a[4] = Mem32(c, structure + kCaseStep);
        const uint8_t kind = Mem8(c, structure + kCaseKind);
        // (the game's own code puts the kind in D0 with a byte's move: the rest of D0 is left as it was)
        c.d[0] = (c.d[0] & 0xFFFFFF00u) | kind;
        if (kind == 0) {
            // (the steps go on where the pointer says)
            CopyStep(c, structure, c.a[4]);
            return;
        }
        if (kind == kCaseEnd) {
            // (the end of the case: the game is told so, by the carry flag)
            SetRam16(structure + kCaseStepsLeft, 1);
            c.SetSR(static_cast<uint16_t>(c.SR() | 0x01));
            return;
        }
        // (the step points at the steps that come next)
        c.a[4] = Mem32(c, c.a[4]);
        CopyStep(c, structure, c.a[4]);
    }
    // 001A94. The sine and the cosine of an angle, by the game's own tables.
    void Trigonometry(M68K &c) {
        const uint32_t structure = c.a[0];
        // (the table of the kind of the angle: the kinds are in a table of their own, in the program)
        const uint32_t kind = static_cast<uint32_t>(c.d[0] & 0xFFFF) * 4;
        const uint32_t table = Code32(c, kAngleKinds + kind);
        SetRam32(structure + kAngleTable, table);
        // (the angle itself, and the pair of words the table has for it: the tables are in the program
        // too)
        const uint32_t angle = static_cast<uint32_t>(Mem8(c, structure + kAngle)) * 4;
        SetRam16(structure + kSine, Code16(c, table + angle));
        SetRam16(structure + kCosine, Code16(c, table + angle + 2));
        // (the routine leaves D0 clear)
        c.d[0] = 0;
    }
} // namespace

void RegisterCases() {
    // (the routine that loads a case, written again as C++. The original takes 150 cycles.)
    AddRewrite(kLoadCase, "LoadCase", LoadCase, kFreeFlags, 150);
    // (the routine that goes on to the next step, written again as C++. The original takes 78 cycles on
    // average, but the time depends on the step: the check reports the spread.)
    AddRewrite(kNextStep, "NextStep", NextStep, kFreeFlags, 78);
    // (the routine that works out the sine and the cosine, written again as C++)
    AddRewrite(kTrigonometry, "Trigonometry", Trigonometry, kFreeFlags, 60);
}

} // namespace cps2recomp
