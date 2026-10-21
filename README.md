# Dungeons & Dragons: Shadow over Mystara — Native PC Recompilation

This repository publishes a **100% native PC port** of *Dungeons & Dragons: Shadow over Mystara*
(Arcade, Capcom CPS‑2, 1996), produced by **static recompilation** of the original 68000 + Z80
machine code into native x86‑64 C++. There is **no emulator** and **no interpreter** in the normal
play path: the game's original CPU code has been translated ahead‑of‑time into native functions that
the port calls directly. It runs on SDL3 (window, input, audio) and renders the CPS‑2 video/QRAM
and QSound hardware through a native backend.

The full toolkit it is built from — **cps2recomp** — lives in the [`cps2recomp/`](cps2recomp)
subdirectory of this repo, so you also get the complete source, the recompilation toolchain, every
game configuration, and the prebuilt executable.

---

## ⚠️ What is NOT included (and why)

- **No ROM.** You must supply the original arcade ROM dump yourself. It is copyrighted Capcom
  material and is intentionally **not** distributed here.
- **No encryption key (`ddsom.key`).** The CPS‑2 set is encrypted; the 20‑byte master key that
  decrypts it is **not** included and **cannot be derived**. You must provide it.
- For the legal/technical background, see [`cps2recomp/games/ddsom/README.md`](cps2recomp/games/ddsom/README.md).

### Do you really need the key?
**Yes — both at build time and at runtime.** The recompiler reads the encrypted ROM to statically
translate it, and the running executable re‑decrypts the data tables at load. Without a valid
`ddsom.key` the build fails and the game refuses to start. There is no "key‑free" mode.

### Exactly which ROM?
The set **`ddsom`, USA region, revision `950101`** (22 files) **plus your `ddsom.key`**. The full
file list and CRC32s are in [`cps2recomp/games/ddsom/game.cfg`](cps2recomp/games/ddsom/game.cfg).
Place all 22 ROM files and `ddsom.key` together in a folder (e.g. `rom/ddsom/`) and point the
executable at it with `--rom rom/ddsom`.

---

## Quick start (using the prebuilt executable)

1. Download [`cps2recomp/build/ddsom/ddsom.exe`](cps2recomp/build/ddsom/ddsom.exe).
2. Obtain the `ddsom` (USA 950101) ROM set and a `ddsom.key`, and put them in one folder.
3. Run:
   ```
   ddsom.exe --rom <path-to-folder-with-22-roms-and-ddsom.key>
   ```
4. Controls — 4 players, each with **Attack**, **Jump**, **Magic**:

   | Action | P1 (keyboard) | P2 | P3 | P4 |
   |--------|---------------|----|----|----|
   | Up/Down/Left/Right | Arrow keys | `6`/`4`/`3`/`5` | Numpad `8`/`2`/`4`/`6` | `T`/`G`/`F`/`H` |
   | Attack | `Z` (A) / `X` (B) / `C` (C) | `Z`/`X`/`C` | `O`/`P`/`;` | `V`/`B`/`N` |
   | Start | `Enter` | `1` | Numpad `0` | `7` |
   | Jump / Magic | `LShift` / `LAlt` | `LShift`/`LAlt` | `LShift`/`LAlt` | `LShift`/`LAlt` |

   Gamepads are auto‑detected (West = magic, South = attack, East = jump, etc.). A full control
   reference is in the ddsom README linked below.

---

## Repository layout

The whole **cps2recomp** project is nested under `cps2recomp/`. Top‑level files here are only the
wrapper `README.md`, `LICENSE`, and `.gitignore`.

```
ddsom-recomp/
├── README.md            ← this file (overview + how to run)
├── LICENSE              ← GPL‑3.0 (copied from the toolkit)
├── .gitignore           ← ignores ROM dumps, keys, runtime state, OS junk
└── cps2recomp/          ← the full cps2recomp toolkit (source + tools + exe)
    ├── README.md        ← upstream toolkit documentation (how the recompiler works)
    ├── LICENSE
    ├── Makefile, build.mk, CMakeLists.txt   ← build system
    ├── port/            ← SDL3 native port: window, input, audio, platform layer
    ├── machine/         ← CPS‑2 hardware model: memory map, devices, ROM loading
    ├── runtime/         ← recomp runtime: calls translated functions, handles hooks
    ├── recomp-common/   ← shared recompiler core (ELF/binary parsing, translation engine)
    │   └── launcher/    ← recomp bootstrap / entry point
    ├── video/           ← CPS‑2 video/QRAM rendering backend
    ├── sound/           ← QSound audio synthesis backend
    ├── tools/           ← recompilation toolchain (parser, recompiler, analyzer) + audit.sh
    ├── games/           ← per‑game integration: config + game‑specific C++ + tests
    │   └── ddsom/       ← Dungeons & Dragons: Shadow over Mystara
    │       ├── README.md   ← full ddsom docs: key, exact ROM, controls, FAQ
    │       ├── game.cfg     ← ROM file list, CRC32s, memory map, input mapping
    │       ├── src/         ← game‑specific hooks/overrides
    │       ├── tests/       ← binding/control tests
    │       └── movies/      ← attract‑mode script text
    ├── beat/  fight/  model/  example/   ← shared game‑type helper frameworks
    ├── third_party/     ← vendored dependencies
    ├── docs/  .github/  ← documentation and CI workflows
    ├── work/            ← GENERATED recompiled C++ (the 68000/Z80 translated to native)
    └── build/
        └── ddsom/
            └── ddsom.exe  ← the prebuilt native executable (the only build artifact kept)
```

### What each piece is for
- **`port/`** — the part that makes it a real PC program: creates the window, reads keyboard/gamepad,
  plays audio through SDL3, and drives the game loop. This is the "native port" layer.
- **`machine/`** — replaces the CPS‑2 circuit board in software: defines how the 68000/Z80 address
  space is laid out, maps the ROM, and loads it (requiring `ddsom.key` to decrypt).
- **`runtime/` + `recomp-common/`** — the recompilation engine. `recomp-common` parses the original
  binary and emits native C++; `runtime` wires the translated functions together and patches hooks.
- **`video/` + `sound/`** — native implementations of the CPS‑2 graphics (sprites, QRAM) and QSound
  DSP, so the original game code drives real PC rendering/audio instead of an emulator.
- **`tools/`** — the offline toolchain that actually performs the static recompilation
  (`make GAME=ddsom explore` regenerates `work/`).
- **`games/ddsom/`** — everything specific to this title: which ROM files, how buttons map, the
  control tests, and the gameplay hooks in `src/`.
- **`work/`** — the output of that toolchain: the 68000 and Z80 code translated into native C++
  source. It is included so the repository is self‑contained, but it is regenerated from the ROM.
- **`build/ddsom/ddsom.exe`** — the compiled result you actually run.

---

## Building from source

Prerequisites: a C++20 compiler, CMake, Make, Python 3, and the SDL3 / OpenGL / Vulkan dev libraries.

```bash
cd cps2recomp
make GAME=ddsom explore      # regenerates work/ from the ROM + key
make GAME=ddsom              # builds build/ddsom/ddsom.exe
```

See [`cps2recomp/README.md`](cps2recomp/README.md) and
[`cps2recomp/games/ddsom/README.md`](cps2recomp/games/ddsom/README.md) for the full, detailed
documentation (proof of native execution, key/ROM specifics, four‑player controls, and FAQ).

---

## License

GPL‑3.0. See [`LICENSE`](LICENSE) and [`cps2recomp/THIRD_PARTY_NOTICES.md`](cps2recomp/THIRD_PARTY_NOTICES.md).
