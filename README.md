# Dungeons & Dragons: Shadow over Mystara — Recomp (native PC port)

A **100% native PC port** of *Dungeons & Dragons: Shadow over Mystara* (Capcom, 1996,
CPS‑2 arcade), produced by **static recompilation** of the original 68000 + Z80 arcade
machine code into x86‑64 native code. There is **no emulator** in this build: the game's
own program ROM is translated ahead‑of‑time into native functions that the launcher calls
directly, and the original Z80 sound program is recompiled the same way. What you run is a
normal Windows executable (`ddsom.exe`) that talks to your GPU and audio through SDL3.

> Not affiliated with or endorsed by Capcom.

This repository contains **only the Dungeons & Dragons: Shadow over Mystara recomp** —
not the rest of the cps2recomp toolkit or the other games it supports.

---

## What "recomp" means here (and why there is no emulation)

- **Native recompilation, not emulation.** The CPS‑2 68000 CPU ROM and the Z80 sound ROM
  are translated into native C++ (`native_*.cpp` / `z80_*.cpp`) by the cps2recomp toolkit
  *before* you ever run the game. At runtime the launcher jumps straight into that translated
  code — no 68000 interpreter, no Z80 interpreter, no cycle‑accurate bus. (The translator also
  keeps a fallback interpreter for any instruction it could not prove, but for this game the
  translated coverage is complete, so the interpreter is never reached in normal play.)
- **Native PC port.** Rendering, input, audio and windowing are handled by a small SDL3
  launcher, not by an arcade‑board simulation. You get a window, a keyboard/gamepad, and sound
  — like any other native game.

---

## Files in this repository

| File / folder | Purpose |
|---|---|
| `ddsom.exe` | The native Windows executable — the actual recompiled game. **Run this.** |
| `games/ddsom/game.cfg` | Per‑game config: ROM set name, the exact 22 ROM filenames + CRC32s, the CPS‑2 master key filename, player count (4) and button names (Attack / Jump / Magic). Read by the launcher at startup. |
| `games/ddsom/README.md` | Game‑specific notes: how the recomp was produced, build status, known limits. |
| `games/ddsom/src/` | The Dungeons & Dragons‑specific recomp source — the hand‑written routines and board mapping for this title (the parts the auto‑translator could not derive automatically). |
| `games/ddsom/PROGRESS.md` | Build / translation progress and how the native code was verified. |
| `games/ddsom/tests/` | A small test that checks the control bindings are consistent. |
| `games/ddsom/check_bindings.sh`, `check_rewrites.sh` | Developer helpers (verify bindings / verify rewritten routines). |
| `games/ddsom/family.h`, `Makefile` | Game‑family definition and the per‑game make stub used by the toolkit. |
| `LICENSE` | GPL‑3.0 (the project this recomp is derived from is GPL‑3.0). |
| `.gitignore` | Keeps build output, extracted ROMs and save states out of the repo. |

> **No ROM image and no encryption key are included here** (see *The encryption key* below).
> You supply those yourself.

---

## The encryption key — yes, you need it

CPS‑2 arcade boards store their program ROMs **encrypted**. To turn the original board dumps
into something the recomp can run, the CPS‑2 master key for this title is used to decrypt them.
Concretely:

- **The file is `ddsom.key`** — a 20‑byte CPS‑2 master key (region `key`). It is referenced in
  `games/ddsom/game.cfg` as `key = ddsom.key:541e425d`.
- **It is required at runtime.** The launcher loads the ROM set *and* `ddsom.key` from the same
  place; if the key file is missing it stops with an error. This is not optional for the standard,
  encrypted ROM set.
- **It is NOT in this repository.** The key is arcade‑board data we do not distribute. You must
  obtain `ddsom.key` yourself and drop it next to the ROM files.
- **Which versions need it?** The recomp is built from the **USA release, revision 950101**
  (`ddsom`). That set is CPS‑2‑encrypted, so it needs `ddsom.key`. Other regional sets (whose
  words differ, nothing else) need their own equivalent key. A *pre‑decrypted* ROM set would not
  need the key — but the launcher expects the standard encrypted set plus `ddsom.key`, so use that.

---

## The exact ROM you need

- **ROM set name:** `ddsom`
- **Title:** *Dungeons & Dragons: Shadow over Mystara (USA 950101)*
- **22 ROM files** (filenames and their CRC32, verified against MAME's `src/mame/capcom/cps2.cpp`):

```
Program (68000), 8 files:
  dd2e.03e  dd2e.04e  dd2e.05e  dd2e.06e
  dd2e.07   dd2e.08   dd2e.09   dd2e.10

Graphics (4 planes per row), 8 files:
  dd2.13m  dd2.15m  dd2.17m  dd2.19m
  dd2.14m  dd2.16m  dd2.18m  dd2.20m

Sound program (Z80), 2 files:
  dd2.01   dd2.02

QSound samples, 2 files:
  dd2.11m  dd2.12m

Key (supplied by you, NOT in the zip), 1 file:
  ddsom.key
```

Get these from a MAME / `ddsom.zip` collection that matches the USA 950101 set, then verify the
filenames and CRC32s against the list above.

---

## Where to put the ROM and key

Put all 22 ROM files **and** `ddsom.key` together in a folder named `rom/ddsom/` next to
`ddsom.exe`:

```
ddsom-recomp/
├── ddsom.exe
├── rom/
│   └── ddsom/
│       ├── dd2e.03e  …  dd2.20m     (the 22 ROM files)
│       └── ddsom.key              (the CPS-2 master key, you supply)
└── games/ddsom/game.cfg
```

Or point the launcher at the folder / zip explicitly:

```
ddsom.exe --rom <path-to-folder-or-zip-containing-the-22-files-and-ddsom.key>
```

The launcher reads the 22 ROM files and `ddsom.key` from that same place.

---

## Controls

**Four players.** Each player has **Attack**, **Jump** and **Magic** (the launcher shows them as
*Attack | Jump | Magic*) plus **Coin** and **Start**.

**Keyboard** — one keyboard is enough for all four; each player has its own block of keys:

| Action | P1 | P2 | P3 | P4 |
|---|---|---|---|---|
| Up    | Arrow Up | 6 | Numpad 8 | T |
| Down  | Arrow Down | 4 | Numpad 2 | G |
| Left  | Arrow Left | 3 | Numpad 4 | F |
| Right | Arrow Right | 5 | Numpad 6 | H |
| Attack | A | Z | O | V |
| Jump   | S | X | P | B |
| Magic  | D | C | ; (semicolon) | N |
| Start  | Enter* | 2 | 8 | 0 |
| Coin   | Left Shift* | 1 | 7 | 9 |

\* P1 uses **Enter** for Start and **Left Shift** for Coin so the number row stays free for the
other players.

**Gamepad** — the first gamepad is P1, the second P2, and so on. Laid out like a six‑button panel:
- Buttons Attack / Jump / Magic → **West / South / East** (then North, Right Shoulder, Right Trigger).
- **Start** → Start button. **Coin** → Back button.
- The left stick (or d‑pad) is always the directions.

You can rebind everything in the launcher's **Controllers** page; your choices are saved to
`settings.ini`.

---

## How to play

1. Get the `ddsom` (USA 950101) ROM set (22 files) and `ddsom.key`.
2. Place them in `rom/ddsom/` (or pass `--rom <folder>`).
3. Run `ddsom.exe`.
4. Insert coins (per‑player Coin key / Back on gamepad), press Start, and play.

---

## Building from source (optional)

This repository ships the **finished native executable**, so you do not need to build to play.
If you want to rebuild or modify the recomp, it is produced by the
[cps2recomp](https://github.com/tmcleroy/cps2recomp) toolkit: that project provides the
runtime / launcher and the translator; this folder's `games/ddsom/` is the per‑game input to it.

---

## License

GPL‑3.0. See `LICENSE`.
