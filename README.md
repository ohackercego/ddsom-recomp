# Dungeons & Dragons: Shadow over Mystara — Native PC Recompilation

A **fully native** PC port of *Dungeons & Dragons: Shadow over Mystara* (Capcom CPS‑2,
USA set `ddsom` / `ddsomu`, build 950101) built with the **cps2recomp** recompilation
toolkit. No emulator, no translation layer — the original 68000/Z80 program code is
recompiled into native x86‑64 C++ and linked against SDL3, so it runs as an ordinary
Windows executable.

> This repository purposefully does **not** include the game ROM or the 20‑byte decryption
> key (`ddsom.key`). You supply those yourself. See
> [Where to put the ROM](#where-to-put-the-rom) and
> [Do you need the decryption key?](#do-you-need-the-decryption-key).

## What's inside

This repo is the complete cps2recomp toolkit with the `ddsom` game preconfigured and a
ready‑to‑run native executable. Everything lives at the repository root (there is no
`cps2recomp/` subdirectory):

| Path | What it is |
|------|------------|
| `games/ddsom/` | Dungeons & Dragons: Shadow over Mystara game definition, `game.cfg`, `src/` (game‑specific recomp glue: `board.cpp`, `cases.cpp`, `ddsom.h`, `game.cpp`), and the full `README.md` with controls, move lists and FAQ. |
| `games/<other>/` | Other cps2recomp games that ship with the toolkit (avsp, dino, pocket_fighter, puzzle_fighter, sfa, sfa2, vampire_savior, vhunt2, vsav2, vsav_definitive). Included for completeness; only `ddsom` is built here. |
| `port/` | Native SDL3 frontend: `main.cpp`, menu, HUD, presentation, shader includes. |
| `machine/` | CPS‑2 machine glue recompiled to native: `cps2crypt`, `eeprom`, `kabuki`, `m68k`, `romset`, sound board. |
| `runtime/` | Recompilation runtime: `recomp.cpp`, `recomp_z80.cpp`, board/game state, `native.h`, `native_z80.h`. |
| `recomp-common/` | Shared recomp infrastructure + the optional game launcher (Steam integration, updates). |
| `video/` `sound/` | Native video and QSound/Z80 audio backends. |
| `model/` `fight/` `beat/` | Framework modules used across games. |
| `tools/` | Recomp tooling: `recomp.py`, `z80.py`, `audit.sh`, `graft.py`, release packaging. |
| `work/ddsom/` | **Recompiled native source** for ddsom — thousands of generated `native_*.cpp` / `z80_*.cpp` plus analysis logs. This is the actual native translation of the original program; you almost never need to touch it. |
| `build/ddsom/ddsom.exe` | The prebuilt native Windows executable (~18.5 MB). Just run it. |
| `docs/framework-README.md` | The upstream cps2recomp toolkit README (build instructions, toolkit design). |
| `Makefile`, `CMakeLists.txt`, `build.mk` | Build system for the toolkit. |
| `LICENSE` | GPL‑3.0 (this project is GPL‑3.0‑only). |

The full, game‑specific documentation (controls, move lists, FAQs, how recompilation works)
lives in [`games/ddsom/README.md`](games/ddsom/README.md).

## Is it really native?

Yes. There is no MAME/Arcade/MITU inside. The 68000 and Z80 program code of the original
CPS‑2 board is statically recompiled into native x86‑64 C++ (`work/ddsom/native_*.cpp`,
`work/ddsom/z80_*.cpp`), and the board hardware (video, sound, inputs, EEPROM) is
reimplemented in `port/`, `machine/`, `video/`, `sound/`. The only non‑game binary you need
is `ddsom.exe`. Because it is native you get: low latency, screen‑reader‑friendly menu/HUD
text (real UI widgets, not rasterized arcade text), and the ability to run at high
resolution / high refresh.

## Do you need the decryption key?

**Yes — both at build time and at runtime.** The recompiler reads the encrypted ROM to
statically translate it, and the running executable re‑decrypts the data tables at load.
Without a valid `ddsom.key` the build fails and the game refuses to start. There is no
"key‑free" mode. The key is a single 20‑byte file you add yourself; it is not shipped in
this repository for copyright reasons.

## Where to put the ROM

The set **`ddsom`, USA region, revision `950101`** (22 files) **plus your `ddsom.key`**. The
full file list and CRC32s are in [`games/ddsom/game.cfg`](games/ddsom/game.cfg). Place all
22 ROM files and `ddsom.key` together in a folder and point the executable at it:

```
ddsom/
├── (the 22 ROM files, e.g. ddsom.03, ddsom.04, … ddsomu.13, ddsomu.14 …)
└── ddsom.key
```

Run `build/ddsom/ddsom.exe --rom <path-to-ddsom-folder-or-zip>`. The key is read from the
same location automatically.

## Controls (summary)

See [`games/ddsom/README.md`](games/ddsom/README.md) for the complete mapping. Defaults:
move with the stick, act/use with the action buttons, and the launcher menu lets you remap
everything. The game supports up to 4 players.

## Building from source (optional)

You only need this if you want to rebuild the executable. Requires a C++20 toolchain,
CMake, Ninja and the SDL3 dev libraries.

```bash
cmake -B build -G Ninja
cmake --build build --target ddsom
```

Full toolkit build docs: [`docs/framework-README.md`](docs/framework-README.md).

## Legal

This project distributes **only** recompiled native code, build tooling and documentation.
It does **not** contain Capcom's copyrighted ROM images or the decryption key. You must
legally own the original game to obtain those. CPS‑2 recompilation and this toolkit are
released under GPL‑3.0; see `LICENSE`.
