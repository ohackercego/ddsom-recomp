#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-only
# Copyright (C) 2026 Tommy McLeroy. Part of cps2recomp.
"""Characters of another game made from the same program, taken into this one: what the port needs to run them.

A game made from another's program (Vampire Savior 2 from Vampire Savior's) can have characters the first has
not. Their code and tables are in the other set's program, which the port maps whole at a base the board leaves
unused (game.cfg's extra_set and extra_base). There their code runs as it is, but for what it names outside itself:
this game's routines (the engine both games share) and this game's variables are elsewhere than in the other game.
This works out, from the two programs and a map of one in the other (tools/portmap.py, run the other way round:
from this game to the other), and writes as C++ tables for the game's sources (games/<id>/src/graft.cpp):

  - the characters' own code: what is reached from their entries of the character tables (game.cfg's graft
    spec names the tables) and is not a routine this game has too;
  - in that code, each place named absolutely and each variable (A5's distances, FFxxxx) moved to where it is here
    (the routines this game has: its own; the other game's code and tables: at the base; variables: this game's);
  - branches from that code to this game's routines (short, so not movable): to be made by hooks;
  - the other game's code places this game has, for addresses loaded from its tables as the characters run
    (M68K::PointerLoad);
  - each table's entries for the characters, from the other game's same table.

  graft.py --spec games/<id>/graft.cfg --work work/<id> --base A00000 --out work/<id>/gen/graft_data.h

What it writes is made from the two programs and stays in work/ (never in the repository).
"""
import argparse
import bisect
import collections
import os
import re
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import m68k  # noqa: E402
import recomp  # noqa: E402

HEX = re.compile(r"\$([0-9A-F]+)")
A5 = re.compile(r"(-?0x[0-9A-F]+)\(a5")


def read_spec(path):
    spec = {"characters": [], "tables": [], "keep": set()}
    for raw_line in open(path):
        line = raw_line.split("#")[0].split()
        if not line:
            continue
        if line[0] == "character":
            spec["characters"].append((int(line[1], 16), " ".join(line[2:])))
        elif line[0] == "extend":
            # extend <table> <stride> <data|pc> <entries here> <entries in the other game> [other=...] (read at ...)
            at = re.search(r"read at ([0-9A-F]{6})", raw_line)
            other = [int(x[6:], 16) for x in line[6:] if x.startswith("other=")]
            spec.setdefault("extend", []).append((int(line[1], 16), int(line[2]), line[3], int(line[4], 16), int(line[5], 16),
                                                  other[0] if other else None, int(at.group(1), 16), "words" in line))
        elif line[0] == "divert":
            # divert <this game's place> <the other game's same place> <the address register with the player, or
            # a5+N: the byte N from A5 is the character>
            who = int(line[3][3:], 16) | 0x10000 if line[3].startswith("a5+") else int(line[3][1])
            spec.setdefault("divert", []).append((int(line[1], 16), int(line[2], 16), who))
        elif line[0] == "ram":
            # ram <the other game's variables> <bytes> <where they are here>
            spec.setdefault("ram", []).append((int(line[1], 16), int(line[2], 16), int(line[3], 16)))
        elif line[0] == "mask":
            # mask <this game's move.l #mask,dN> <the other game's>
            spec.setdefault("mask", []).append((int(line[1], 16), int(line[2], 16)))
        elif line[0] == "patch":
            # patch <the other game's address> <bytes> <as they are> <as they are to be>
            spec.setdefault("patch", []).append((int(line[1], 16), int(line[2]), int(line[3], 16), int(line[4], 16)))
        elif line[0] == "table":
            # (where this game reads it: "read at <address>" in the line's remark, for the tables made by hooks)
            at = re.search(r"read at ([0-9A-F]{6})", raw_line)
            # table <this game's address> <stride> <entry bytes> <data|pc> <kind> [other=<the other game's address>]
            other = [int(x[6:], 16) for x in line[6:] if x.startswith("other=")]
            # (index=+N: where it is read, the index is the character's number and N more)
            shift = [int(x[7:], 16) for x in line[6:] if x.startswith("index=+")]
            spec["tables"].append((int(line[1], 16), int(line[2]), int(line[3]), line[4], line[5], other[0] if other else None,
                                   int(at.group(1), 16) if at else None))
            spec.setdefault("index", {})[(int(line[1], 16), int(at.group(1), 16) if at else None)] = shift[0] if shift else 0
            # (swap=AA:BB: in the entries' bytes, AA becomes BB, a number the other game has and this one has not)
            for x in line[6:]:
                if x.startswith("swap="):
                    was, now = (int(v, 16) for v in x[5:].split(":"))
                    spec.setdefault("swap", {}).setdefault(int(line[1], 16), []).append((was, now))
    return spec


def read_entries(path):
    out = []
    if os.path.exists(path):
        for line in open(path):
            line = line.strip()
            if line and not line.startswith("#"):
                try:
                    out.append(int(line, 16))
                except ValueError:
                    pass
    return out


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--spec", required=True)
    ap.add_argument("--work", required=True, help="the game's work folder: program.bin(.data), program.bin.extra(.data), "
                                                  "entries.txt, extra/portmap.txt")
    ap.add_argument("--base", default="A00000")
    ap.add_argument("--limit", default="100000", help="where the code ends (hex), in both programs")
    ap.add_argument("--out", required=True)
    args = ap.parse_args()
    base = int(args.base, 16)
    limit = int(args.limit, 16)
    spec = read_spec(args.spec)
    w = args.work
    here = open(os.path.join(w, "program.bin"), "rb").read()
    here_data = open(os.path.join(w, "program.bin.data"), "rb").read()
    other = open(os.path.join(w, "program.bin.extra"), "rb").read()
    other_data = open(os.path.join(w, "program.bin.extra.data"), "rb").read()

    # The map, this game -> the other: instructions, and the places matched instructions name
    imap, named = {}, {}
    for line in open(os.path.join(w, "extra", "portmap.txt")):
        t = line.split()
        if t and t[0] in ("I", "T"):
            imap.setdefault(int(t[1], 16), int(t[2], 16))
        elif t and t[0] == "P":
            n, total = (int(v) for v in t[3].split("/"))
            if 2 * n > total:
                named[int(t[1], 16)] = int(t[2], 16)
    # (variables of the other game's this one has no place for: where graft.cfg puts them, byte by byte)
    for b, size, a in spec.get("ram", []):
        for n in range(size):
            named[a + n] = b + n
    # ... and the other way round: the other game's code places this game has. A routine's start as the code
    # calls it (named) before an instruction's place (a routine may begin with something new there).
    back = {}
    for a, b in named.items():
        if a < limit and b < limit:
            back.setdefault(b, a)
    for a, b in imap.items():
        back.setdefault(b, a)
    # Variables: the pairs named, and between two that moved alike
    ram = sorted((b, a) for a, b in named.items() if a >= 0xFF0000 and b >= 0xFF0000)
    ram_keys = [b for b, _ in ram]
    ram_map = dict(ram)

    def ram_here(b):
        if b in ram_map:
            return ram_map[b]
        i = bisect.bisect_right(ram_keys, b)
        if 0 < i < len(ram_keys):
            lo, hi = ram_keys[i - 1], ram_keys[i]
            if ram_map[lo] - lo == ram_map[hi] - hi and hi - lo <= 0x40:
                return b + ram_map[lo] - lo
        return None

    # The other game's code (as its own discovery finds it from what portmap placed)
    other_code, _, _ = recomp.discover(other, sorted(set(imap.values())) + [0x400], limit, True)
    # The characters' entries of the tables, and their own code from there
    here_tables = []
    for addr, stride, size, view, kind, given, read in spec["tables"]:
        if given is not None:
            here_tables.append((addr, given, stride, size, view, kind, read))
            continue
        if addr not in named:
            print("graft: table %06X: not found in the other game" % addr, file=sys.stderr)
            continue
        here_tables.append((addr, named[addr], stride, size, view, kind, read))
    roots = set()
    # (where a character taken in goes on in the other game's code: that code is theirs too)
    for _, there, _ in spec.get("divert", []):
        roots.add(there)
    for addr, oaddr, stride, size, view, kind, _ in here_tables:
        if kind not in ("code", "jump"):
            continue
        mem = other if view == "pc" else other_data
        for number, _ in spec["characters"]:
            if kind == "code":
                v = int.from_bytes(mem[oaddr + number * stride:oaddr + number * stride + 4], "big")
            else:
                v = (oaddr + int.from_bytes(mem[oaddr + number * stride:oaddr + number * stride + 2], "big", signed=True)) & 0xFFFFFF
            if v and v < limit and v not in back:
                roots.add(v)
    # (followed no further than this game's routines: calls and jumps to one stop there; a routine of the other
    # game's that begins differently is still this game's, by the code's calls to it: `back`)
    # Tables of routines by a number that the other game has more of (the things thrown and their kinds): the
    # other game's routines for the numbers this one has not, theirs too
    extended = []
    for addr, stride, view, count, other_count, given, at, words in spec.get("extend", []):
        oaddr = given if given is not None else named.get(addr)
        if oaddr is None:
            print("graft: table %06X to extend: not found in the other game" % addr, file=sys.stderr)
            continue
        mem = other if view == "pc" else other_data
        for i in range(count, other_count):
            if words:  # (offsets from the table)
                v = (oaddr + int.from_bytes(mem[oaddr + i * stride:oaddr + i * stride + 2], "big", signed=True)) & 0xFFFFFF
            else:
                v = int.from_bytes(mem[oaddr + i * stride:oaddr + i * stride + 4], "big")
            extended.append((addr, i, v, at, words))
            if v and v < limit and v not in back:
                roots.add(v)
    # (followed no further than this game's routines: calls and jumps to a routine this game's code calls too stop
    # there. Everything else reached is theirs, even where it looks like some of this game's code)
    routines_back = {b for a, b in named.items() if a < limit and b < limit}
    reach, _, _ = recomp.discover(other, sorted(roots), limit, True, stop=routines_back)
    own = set(reach)
    # (an instruction of the characters' code that portmap matched to this game's: still theirs if it is among
    # theirs; code of this game's is reached only by calls out)
    own_ranges = []
    for x in sorted(own):
        end = x + reach[x].length
        if own_ranges and x <= own_ranges[-1][1] + 0x10:
            own_ranges[-1][1] = max(own_ranges[-1][1], end)
        else:
            own_ranges.append([x, end])

    def is_own(v):
        i = bisect.bisect_right([r[0] for r in own_ranges], v) - 1
        return i >= 0 and own_ranges[i][0] <= v < own_ranges[i][1]

    unmapped_code = collections.Counter()

    def place(v):
        """Where something the other game's code names is here: (value, how)"""
        if v >= 0xFF0000:
            h = ram_here(v)
            return (h, "ram") if h is not None else (v, "ram?")
        if v >= 0x400000:
            return v, "io"
        if is_own(v):
            return base + v, "own"
        if v < limit and v in other_code:
            if v in back:
                return back[v], "engine"
            unmapped_code[v] += 1
            return base + v, "engine?"
        return base + v, "data"

    # A record's fields that hold addresses: those some code of the other game's loads into an address register
    # (movea.l 0x1C(a6),a0). A number of theirs that is not a place of code or of the work memory is an address only
    # where it goes into an address register or such a field; elsewhere (move.l #$50000,d1: a speed) it is a number.
    # Such a field holds numbers in other records (+44: a player's speed up), so a number among their code's places
    # is a number there too (move.l #$60000,0x44(a6)).
    pointer_fields = set()
    for x in list(other_code) + list(reach):
        insn = other_code.get(x) or reach.get(x)
        m = re.match(r"movea\.l (-?0x[0-9A-F]+)\(a[0-6]\),a[0-7]$", insn.text)
        if m:
            pointer_fields.add(int(m.group(1), 16))

    code_end = max(other_code)

    def into_address(text):
        dest = text.rsplit(",", 1)[-1]
        if re.fullmatch(r"a[0-7]", dest) or text.startswith(("lea ", "pea ")):
            return True
        m = re.fullmatch(r"(-?0x[0-9A-F]+)\(a[0-6]\)", dest)
        return bool(m) and int(m.group(1), 16) in pointer_fields and text.startswith("move.l ")
    numbers = []

    # Each instruction of their code: what it names, moved
    patches, far, odd = [], [], []
    for x in sorted(own):
        insn = reach[x]
        text = insn.text
        raw = other[x:x + insn.length]
        if insn.flow in ("call", "branch", "cond", "dbcc") and insn.target is not None:
            op = text.split()[0]
            if op.startswith(("jsr", "jmp")):
                pass  # (an absolute place: below)
            elif not is_own(insn.target):
                h, how = place(insn.target)
                far.append((x, insn.length, op, h, how))
                continue
            else:
                continue
        for m in HEX.finditer(text):
            v = int(m.group(1), 16)
            immediate = m.start() > 0 and text[m.start() - 1] == "#"
            pcrel = text[m.end():m.end() + 3] == "(pc"
            if pcrel:
                continue  # (the other game's own place, at the base like the rest of it)
            if immediate and not (0x10000 <= v < 0x400000 or v >= 0xFF0000):
                continue
            h, how = place(v)
            if h == v:
                continue
            # (and one among their code's places that is not an instruction's, or a whole number in the game's fixed
            # point, 16.16, is a speed or the like wherever it goes but an address register: their tables are after
            # their code, and none starts at a multiple of 0x10000)
            number = v < code_end or v & 0xFFFF == 0
            if immediate and how in ("data", "engine?") and (not into_address(text) or (number and not re.search(r",a[0-7]$", text))):
                numbers.append((x, text))
                continue
            for size in (4, 2):
                enc = (v & 0xFFFFFFFF if size == 4 else v & 0xFFFF).to_bytes(size, "big")
                at = raw.find(enc, 2)
                if at >= 0:
                    patches.append((x + at, size, h if size == 4 else h & 0xFFFF, how))
                    break
            else:
                odd.append((x, text))
        for m in A5.finditer(text):
            d = int(m.group(1), 16)
            h = ram_here((0xFF8000 + d) & 0xFFFFFF)
            if h is None:
                odd.append((x, text + " (A5)"))
                continue
            nd = h - 0xFF8000
            if nd == d:
                continue
            enc = (d & 0xFFFF).to_bytes(2, "big")
            at = raw.find(enc, 2)
            if at < 0 or not -0x8000 <= nd < 0x8000:
                odd.append((x, text))
                continue
            patches.append((x + at, 2, nd & 0xFFFF, "a5"))

    # Changes to their code by hand (graft.cfg), each where it is as expected
    for x, size, was, now in spec.get("patch", []):
        if int.from_bytes(other[x:x + size], "big") != was:
            sys.exit("graft: patch at %06X: the other game's program is not as expected" % x)
        patches.append((x, size, now, "by hand"))

    # The tables' entries: bytes to put at a place of this game's program; and for the tables of offsets from
    # themselves, where the character's entry leads
    entries, targets = [], []
    for addr, oaddr, stride, size, view, kind, read in here_tables:
        mem = other if view == "pc" else other_data
        for number, _ in spec["characters"]:
            raw = mem[oaddr + number * stride:oaddr + number * stride + size]
            if kind == "entry":
                targets.append((addr, number + spec.get("index", {}).get((addr, read), 0), base + oaddr + number * stride, kind, "data", read))
                continue
            if kind in ("jump", "selfrel"):
                to = (oaddr + int.from_bytes(raw, "big", signed=True)) & 0xFFFFFF
                h, how = place(to) if kind == "jump" else (base + to, "data")
                targets.append((addr, number, h, kind, how, read))
                continue
            if kind in ("code", "data") and size == 4:
                v = int.from_bytes(raw, "big")
                if v:
                    v, _how = place(v)
                raw = v.to_bytes(4, "big")
            if kind == "value" and addr in spec.get("swap", {}):
                raw = bytes(dict(spec["swap"][addr]).get(b, b) for b in raw)
            entries.append((addr + number * stride, raw, view))

    # The other game's code places this game has (for addresses loaded from its tables), sorted
    code_back = sorted((b, a) for b, a in back.items() if b < limit and a < limit)

    out = []
    out.append("// Made by tools/graft.py from the two programs: stays in work/ (never in the repository)")
    out.append("#pragma once")
    out.append("#include <cstdint>")
    out.append("namespace graft {")
    out.append("constexpr uint32_t kBase = 0x%X;" % base)
    out.append("struct Range { uint32_t from, to; };")
    out.append("inline constexpr Range kOwnCode[] = {")
    out += ["    {0x%06X, 0x%06X}," % (a, b) for a, b in own_ranges]
    out.append("};")
    out.append("struct Patch { uint32_t at; uint8_t size; uint32_t value; };")
    out.append("inline constexpr Patch kPatches[] = {")
    out += ["    {0x%06X, %d, 0x%X}, // %s" % (a, s, v, how) for a, s, v, how in patches]
    out.append("};")
    out.append("struct Far { uint32_t at; uint8_t length; const char *op; uint32_t to; };")
    out.append("inline constexpr Far kFar[] = {")
    out += ['    {0x%06X, %d, "%s", 0x%X}, // %s' % (a, n, op, h, how) for a, n, op, h, how in far]
    out.append("};")
    out.append("struct Pair { uint32_t other, here; };")
    out.append("inline constexpr Pair kCodeHere[] = {")
    out += ["    {0x%06X, 0x%06X}," % (b, a) for b, a in code_back]
    out.append("};")
    out.append("inline constexpr Pair kRamHere[] = {")
    out += ["    {0x%06X, 0x%06X}," % (b, a) for b, a in ram]
    out.append("};")
    out.append("struct Entry { uint32_t at; uint8_t size; uint8_t bytes[128]; bool program; };")
    out.append("inline constexpr Entry kEntries[] = {")
    out += ["    {0x%06X, %d, {%s}, %s}," % (a, len(raw), ", ".join("0x%02X" % b for b in raw), "true" if view == "pc" else "false")
            for a, raw, view in entries]
    out.append("};")
    out.append("// A table of offsets from itself (jumped to, or added to its address): the character's entry's place")
    out.append("enum class Kind { Jump, Call, SelfRelative, Entry };")
    out.append("// (made at `at`, the instruction that reads the table, `length` bytes; its index is d`index`, the number times")
    out.append("// `stride`; what it loads goes to a`to_register`. Entry: that register given `to`)")
    out.append("struct Target { uint32_t table; uint8_t character; uint32_t to; Kind kind; uint32_t at; uint8_t length, index, stride, to_register; };")
    out.append("inline constexpr Target kTargets[] = {")
    kinds = {"jump": "Kind::Jump", "call": "Kind::Call", "selfrel": "Kind::SelfRelative", "entry": "Kind::Entry"}
    for addr, i, v, at, words in extended:
        h, how = place(v) if v else (0, "none")
        targets.append((addr, i, h, "jump" if words else "entry", how, at))  # (a table extended where each line says)
    for a, n, h, kind, how, read in targets:
        # (the instruction that uses the table: the first from where the number is read that is indexed by a register)
        at = read
        for _ in range(8):
            insn = m68k.decode(here, at)
            if re.search(r",d\d\.w\)", insn.text):
                break
            at += insn.length
        m = re.search(r",d(\d)\.w\)", insn.text)
        stride = ([t[2] for t in here_tables if t[0] == a] + [e[1] for e in spec.get("extend", []) if e[0] == a])[0]
        ra = re.search(r",a(\d)$", insn.text)
        if kind == "jump" and m68k.decode(here, at + insn.length).text.startswith("jsr"):
            kind = "call"  # (a call by the table: what follows the jsr is where it comes back to)
        out.append("    {0x%06X, 0x%02X, 0x%X, %s, 0x%06X, %d, %s, %d, %s}, // %s: %s" % (
            a, n, h, kinds[kind], at, insn.length, m.group(1) if m else "0", stride, ra.group(1) if ra else "0", how, insn.text))
    out.append("};")
    # Where a character taken in goes on in the other game's code (its engine has more for them there)
    out.append("struct Divert { uint32_t at, to; uint32_t who; }; // (who: an address register with the player; 0x10000 + N: the byte N from A5 is the character)")
    out.append("inline constexpr Divert kDiverts[] = {")
    for a, b, r in spec.get("divert", []):
        if not is_own(b):
            sys.exit("graft: divert %06X: %06X is not theirs" % (a, b))
        out.append("    {0x%06X, 0x%X, %d}, // %s / %s" % (a, base + b, r, m68k.decode(here, a).text, m68k.decode(other, b).text))
    out.append("};")
    # This game's masks of bits by character number, and the other's: the characters taken in are as the other has them
    out.append("struct Mask { uint32_t at; uint8_t length, reg; uint32_t here, there; };")
    out.append("inline constexpr Mask kMasks[] = {")
    for a, b in spec.get("mask", []):
        mine, theirs = m68k.decode(here, a), m68k.decode(other, b)
        mm = re.match(r"move\.l #\$([0-9A-F]+),d(\d)$", mine.text)
        mt = re.match(r"move\.l #\$([0-9A-F]+),d(\d)$", theirs.text)
        if not mm or not mt:
            sys.exit("graft: mask %06X / %06X: not a move.l #mask,dN (%s / %s)" % (a, b, mine.text, theirs.text))
        out.append("    {0x%06X, %d, %s, 0x%s, 0x%s}, // %s / %06X" % (a, mine.length, mm.group(2), mm.group(1), mt.group(1), mine.text, b))
    out.append("};")
    out.append("} // namespace graft")
    os.makedirs(os.path.dirname(args.out) or ".", exist_ok=True)
    # (the places of this game's code the sources hook, for tools/recomp.py: --hooks)
    hooks = sorted({int(re.search(r"0x([0-9A-F]{6}), \d+, \d, \d+, \d\}", line).group(1), 16)
                    for line in out if "Kind::" in line and "}," in line} | {a for a, _ in spec.get("mask", [])} | {a for a, _, _ in spec.get("divert", [])})
    with open(os.path.splitext(args.out)[0] + "_hooks.txt", "w") as f:
        f.write("".join("%06X\n" % h for h in hooks))
    content = "\n".join(out) + "\n"
    if not os.path.exists(args.out) or open(args.out).read() != content:
        open(args.out, "w").write(content)
    print("graft: %d characters; their code %d instructions in %d ranges; %d places moved, %d branches out, %d table "
          "entries; %d numbers left as numbers; %d places not understood, %d calls to the other game's code this one has not"
          % (len(spec["characters"]), len(own), len(own_ranges), len(patches), len(far), len(entries) + len(targets), len(numbers), len(odd),
             len(unmapped_code)))
    for x, text in odd[:20]:
        print("graft: not understood: %06X %s" % (x, text), file=sys.stderr)
    if os.environ.get("GRAFT_NUMBERS"):
        for x, text in numbers:
            print("graft: a number, left: %06X %s" % (x, text), file=sys.stderr)
    for v, n in unmapped_code.most_common(20):
        print("graft: the other game's code %06X, called %d times, is not this one's" % (v, n), file=sys.stderr)


if __name__ == "__main__":
    main()
