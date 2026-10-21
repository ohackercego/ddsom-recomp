// SPDX-License-Identifier: GPL-3.0-only
// Copyright (C) 2026 Tommy McLeroy
//
// Vampire Savior Definitive: the characters taken from Vampire Savior 2. Its program is mapped whole from
// graft::kBase on (game.cfg's extra_set and extra_base); the characters' own code runs there. What
// tools/graft.py worked out from the two programs (work/<id>/gen/graft_data.h, from the player's sets: never in the
// repository) is put in place before the first frame:
//   - in their code, each place it names moved to where it is here (this game's routines, its variables, and the
//     other set's own code and tables at the base);
//   - Vampire Savior's tables by character given their entries (graft.cfg);
//   - hooks for what cannot be moved: their code's short branches to this game's routines, and this game's tables
//     of offsets from themselves;
//   - an address loaded from the other set's tables (movea.l) moved the same way (M68K::PointerLoad).
// Their sounds go to the other set's sound side (game.cfg's extra_sound_program and extra_samples), and, for now, which
// character each player is comes from the menu (the character choice has no place for them yet).
#include "definitive.h"
#include "modes.h"
#include "sound_board.h"
#include "vsav.h"

#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <iterator>

#if __has_include("graft_data.h")
#include "graft_data.h"
#define VSAV_GRAFT 1
#endif

namespace cps2recomp {

bool ModelViewer(); // (model.cpp)

namespace {
    using namespace vsav;

    void (*g_before)(Machine &) = nullptr; // (the game's own, run after)

    using namespace definitive;
    // Which character a player is, from the menu (0: as chosen; then the characters taken in)
    constexpr const char *kPlayerFeatures[2] = {"definitive_p1", "definitive_p2"};

#ifdef VSAV_GRAFT
    template <typename T>
    void Put(uint8_t *at, T value, int size) {
        for (int n = size - 1; n >= 0; n--) {
            at[n] = static_cast<uint8_t>(value);
            value >>= 8;
        }
    }

    bool InOwnCode(uint32_t place) {
        for (const graft::Range &r : graft::kOwnCode) {
            if (place >= r.from && place < r.to) {
                return true;
            }
        }
        return false;
    }
    // A place of the other game's code that this game has: where it is here (0: none)
    uint32_t CodeHere(uint32_t place) {
        const auto *end = std::end(graft::kCodeHere);
        const auto *at = std::lower_bound(std::begin(graft::kCodeHere), end, place,
                                          [](const graft::Pair &p, uint32_t v) { return p.other < v; });
        return at != end && at->other == place ? at->here : 0;
    }
    // A variable of the other game's: where it is here (as it is, if the map has no word of it)
    uint32_t RamHere(uint32_t place) {
        const auto *end = std::end(graft::kRamHere);
        const auto *at = std::lower_bound(std::begin(graft::kRamHere), end, place,
                                          [](const graft::Pair &p, uint32_t v) { return p.other < v; });
        if (at != end && at->other == place) {
            return at->here;
        }
        // (between two that moved alike)
        if (at != end && at != std::begin(graft::kRamHere)) {
            const graft::Pair &lo = at[-1], &hi = *at;
            if (lo.here - lo.other == hi.here - hi.other && hi.other - lo.other <= 0x40) {
                return place + (lo.here - lo.other);
            }
        }
        return place;
    }

    // An address loaded from the other set (its tables, its code's): the other game's places as they are here
    uint32_t PointerFrom(uint32_t, uint32_t value) {
        const uint32_t v = value & 0xFFFFFF;
        if (v >= 0xFF0000) {
            return (value & 0xFF000000) | RamHere(v);
        }
        if (v < 0x400 || v >= 0x400000) {
            return value;
        }
        if (InOwnCode(v)) {
            return graft::kBase + v;
        }
        if (const uint32_t here = CodeHere(v)) {
            return here;
        }
        return graft::kBase + v; // (a table of the other set's)
    }

    // Their code's short branches to this game's routines
    bool FarBranch(M68K &c) {
        const uint32_t at = (c.pc & 0xFFFFFF) - graft::kBase;
        for (const graft::Far &f : graft::kFar) {
            if (f.at != at) {
                continue;
            }
            const std::string op = f.op;
            static const char *kConditions[16] = {"bra", "", "bhi", "bls", "bcc", "bcs", "bne", "beq",
                                                  "bvc", "bvs", "bpl", "bmi", "bge", "blt", "bgt", "ble"};
            c.cycles += 10;
            if (op == "bsr") {
                c.Push32(c.pc + f.length);
                c.pc = f.to;
                return true;
            }
            for (unsigned cc = 0; cc < 16; cc++) {
                if (op == kConditions[cc]) {
                    c.pc = (cc == 0 || c.Condition(cc)) ? f.to : c.pc + f.length;
                    return true;
                }
            }
            fprintf(stderr, "graft: a branch out (%s at %06X) not made\n", f.op, at);
            return false;
        }
        return false;
    }

} // namespace

// This game's tables of offsets from themselves, read for a character taken in
bool definitive::OffsetTable(M68K &c) {
    const uint32_t at = c.pc & 0xFFFFFF;
    if (getenv("GRAFT_DEBUG")) {
        fprintf(stderr, "graft: table read at %06X, d0 %08X d1 %08X a6 %08X:", at, c.d[0], c.d[1], c.a[6]);
        for (int n = 0; n < 16; n++) {
            fprintf(stderr, " %02X", Ram8((c.a[6] & 0xFFFFFF) + n));
        }
        fprintf(stderr, "\n");
    }
    for (const graft::Target &t : graft::kTargets) {
        if (t.at != at || (c.d[t.index] & 0xFFFF) / t.stride != t.character) {
            continue;
        }
        c.cycles += 14;
        switch (t.kind) {
        case graft::Kind::Jump:
            c.pc = t.to;
            return true;
        case graft::Kind::Call:
            c.Push32(at + t.length + 4); // (after the jsr that follows: 4 bytes, by the table)
            c.pc = t.to;
            return true;
        case graft::Kind::SelfRelative:
        case graft::Kind::Entry:
            c.a[t.to_register] = t.to;
            c.pc = at + t.length;
            return true;
        }
    }
    return false;
}

namespace {

    // A sound asked for by a character taken in (the player itself, or a thing of its: A4 its player) is the other
    // set's: its number is that set's (its voices are where the characters it left out had theirs). It goes to the
    // other set's sound side (machine/sound_board.h), and not to the game's queue.
    bool InPlayers(uint32_t record) {
        return (record & 0xFFFFFF) == Player(0) || (record & 0xFFFFFF) == Player(1);
    }
    bool TakenSound(M68K &c) {
        SoundBoard *board = ExtraSound();
        if (!board || !(TakenPlayer(c.a[6]) || (!InPlayers(c.a[6]) && TakenPlayer(c.a[4])))) {
            return false;
        }
        board->Request(c.d[1], c.d[2], c.d[3]);
        Return(c);
        return true;
    }
    // 0031DA: ... as the game's, none while the sound is off (A5+83: the attract mode's, by the operator's
    // setting) unless it is wanted all the same (A5+99)
    bool TakenSoundIfOn(M68K &c) {
        if (!Ram8(Var(c, AtA5(0x99))) && Ram8(Var(c, AtA5(0x83)))) {
            return false; // (the game's own code: nothing)
        }
        return TakenSound(c);
    }

    // Where Vampire Savior 2's engine has more for the characters taken in: they go on in its code (graft.cfg: divert)
    bool Divert(M68K &c) {
        const uint32_t at = c.pc & 0xFFFFFF;
        for (const graft::Divert &d : graft::kDiverts) {
            const bool taken = d.who & 0x10000 ? IsTaken(Ram8(Var(c, static_cast<int32_t>(d.who & 0xFFFF)))) : TakenPlayer(c.a[d.who]);
            if (d.at == at && taken) {
                c.pc = d.to;
                return true;
            }
        }
        return OffsetTable(c); // (a table of this game's read there, extended: graft.cfg)
    }

    // This game's masks of bits by character number: the characters taken in are as the other game has them
    bool Mask(M68K &c) {
        const uint32_t at = c.pc & 0xFFFFFF;
        for (const graft::Mask &m : graft::kMasks) {
            if (m.at != at) {
                continue;
            }
            uint32_t mask = m.here;
            for (const uint8_t taken : kTaken) {
                const uint32_t bit = 1u << (taken & 31);
                mask = (mask & ~bit) | (m.there & bit);
            }
            c.d[m.reg] = mask;
            c.flagN = mask & 0x80000000u;
            c.flagZ = mask == 0;
            c.flagV = c.flagC = false;
            c.cycles += 12;
            c.pc = at + m.length;
            return true;
        }
        return false;
    }

    void PutInPlace() {
        static bool done = false;
        if (done || !ExtraOpcodes(0)) {
            return;
        }
        done = true;
        for (const graft::Patch &p : graft::kPatches) {
            Put(ExtraOpcodes(p.at), p.value, p.size);
        }
        for (const graft::Entry &e : graft::kEntries) {
            uint8_t *data = Program(e.at), *code = ProgramOpcodes(e.at);
            std::copy(e.bytes, e.bytes + e.size, e.program ? code : data);
        }
    }
#endif

} // namespace

#ifndef VSAV_GRAFT
// (built without the data made from the two programs: nothing is taken in, no table is read for them)
bool definitive::OffsetTable(M68K &) {
    return false;
}
#endif

bool definitive::IsTaken(uint8_t character) {
    return std::find(std::begin(kTaken), std::end(kTaken), character) != std::end(kTaken);
}
bool definitive::TakenPlayer(uint32_t record) {
    record &= 0xFFFFFF;
    return (record == vsav::Player(0) || record == vsav::Player(1)) && IsTaken(Ram8(record + vsav::kCharacter));
}

} // namespace cps2recomp

namespace vsav::family {

void RegisterOwnSources() {
    using namespace cps2recomp;
    Game &game = TheGame();
#ifdef VSAV_GRAFT
    for (const graft::Far &f : graft::kFar) {
        const uint32_t at = graft::kBase + f.at; // (in the other set's code, which is not translated: tools/recomp.py need not know)
        AddHook(at, "GraftBranch", FarBranch);
    }
    definitive::RegisterRules(); // (first: where a rule's hook is, it reads the table itself)
    definitive::RegisterChoosing();
    for (const graft::Divert &d : graft::kDiverts) {
        const uint32_t at = d.at; // (tools/recomp.py has these from graft_data_hooks.txt)
        if (!HasHook(at)) {
            AddHook(at, "GraftDivert", Divert);
        } else {
            fprintf(stderr, "graft: divert at %06X: a rule's hook is there already\n", at);
        }
    }
    for (const graft::Target &t : graft::kTargets) {
        const uint32_t at = t.at; // (tools/recomp.py has these from graft_data_hooks.txt)
        if (!HasHook(at)) {
            AddHook(at, "GraftTable", definitive::OffsetTable);
        }
    }
    SetPointerLoads(PointerFrom, graft::kBase, graft::kBase + ExtraProgramSize());
    for (const graft::Mask &m : graft::kMasks) {
        const uint32_t at = m.at; // (tools/recomp.py has these from graft_data_hooks.txt)
        AddHook(at, "GraftMask", Mask);
    }
    AddHook(At(0x0031DA), "TakenSoundIfOn", TakenSoundIfOn);
    AddHook(At(0x0031E6), "TakenSound", TakenSound);
#endif
    game.pages.push_back({"Definitive", "", "The characters taken from Vampire Savior 2. Until the character choice has them, "
                                            "a player is one from here: chosen at the next character choice."});
    for (int n = 0; n < 2; n++) {
        game.features.push_back({kPlayerFeatures[n], "Definitive", n == 0 ? "Player 1 is" : "Player 2 is",
                                 "Whom the player fights as, whatever is chosen at the character choice.", 0,
                                 {"As chosen", kTakenNames[0], kTakenNames[1], kTakenNames[2]}});
    }
    g_before = game.beforeFrame;
    game.beforeFrame = [](Machine &machine) {
#ifdef VSAV_GRAFT
        PutInPlace();
#endif
        if (!AttractMode() && !StageViewer() && !BattleMode() && !ModelViewer()) {
            for (int n = 0; n < 2; n++) {
                const int choice = FeatureValue(kPlayerFeatures[n]);
                Wanted().characters[n] = choice > 0 && choice <= static_cast<int>(std::size(kTaken)) ? kTaken[choice - 1] : -1;
            }
        }
        if (g_before) {
            g_before(machine);
        }
    };
}

} // namespace vsav::family
