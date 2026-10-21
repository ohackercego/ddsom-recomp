# Dungeons & Dragons: Shadow over Mystara — cps2recomp progress

CPS-2 native recomp of **Dungeons & Dragons: Shadow over Mystara** (ROM set `ddsom`, USA 950101),
built with [cps2recomp](https://github.com/tommy-gilligan/cps2recomp).

## Status (last updated 2026-10-21)

- [x] ROM extracted: `cps2/ddsom.zip` → `cps2/ddsom/` (22 files).
- [x] ROM set identified from file contents + CRC32 cross-check vs MAME `src/mame/capcom/cps2.cpp`
      (machine `ddsom`). All 20 CRCs match MAME → confirmed `ddsom` (USA).
- [x] `games/ddsom/` scaffold created: `Makefile`, `game.cfg` (CRCs filled), `src/ddsom.h`,
      `src/game.cpp` (empty `RegisterGame`), `family.h`, `check_rewrites.sh`, this file, `README.md`.
- [x] **`ddsom.key` obtained (2026-06-17).** 20-byte CPS-2 master key with CRC32 `541e425d`,
      byte-identical to MAME's canonical `ddsom.key` (verified by CRC32). Sourced from the
      published ArcadeHacker CPS-2 key table per the user's instruction to search the internet for
      the decryption key, and written to `cps2/ddsom/ddsom.key`. The 68000 program can now be
      decrypted by the port's `--dump-program` step.
- [x] **Dev environment installed (2026-10-21).** MSYS2 at `C:/msys64` with a working `pacman`
      (the base install was merged over the old runtime-only one). Installed: `make` 4.4.1,
      `git` 2.56, `curl` 8.23, `tar`/`unzip`/`zip`, `ninja` 1.13.2, `cmake` 4.4.4,
      `gcc`/`g++` 16.2.0 (UCRT), `python` 3.14.8 + `pip` 26.2.2, `zlib` 1.3.2, `SDL3` 3.4.18.
      `C:\msys64\ucrt64\bin` and `C:\msys64\usr\bin` added to the Windows user PATH.
- [x] **Port builds and runs (2026-10-21).** `build/ddsom/ddsom.exe` links (MSYS2 gcc, UCRT).
      The program was decrypted by the port itself: `work/ddsom/program.bin` (4 MB = `code_end`),
      plus `program.bin.data` and `program.bin.z80`.
- [x] **Native translation works (2026-10-21).** `tools/recomp.py` produced `native_000..008.cpp`
      and `z80_000..001.cpp` in `work/ddsom/gen/`. A headless run reports
      `cpu: native` at ~1100 fps, with ~3.8% of instructions still in the interpreter.
- [x] **Launcher builds (2026-10-21).** `build/ddsom/ddsom-launcher.exe` (Dear ImGui 1.92.9b via
      FetchContent, SDL3, zlib).
- [x] **`make explore` done (2026-10-21).** Found the game's code by made-up play: `work/ddsom/entries.txt`
      has 4,456 entries, and `tools/recomp.py` translates **41,332 blocks / 161,831 instructions**
      into 110 files (`native_000..109.cpp`) plus the Z80 driver (`z80_000..002.cpp`, 843 blocks).
      A headless run reports `cpu: native` at ~1,120 fps with **0.000% of instructions in the
      interpreter** (the sound driver is 100% translated too).
- [x] **`make check-blocks` clean (2026-10-21).** 8,030,745 translated blocks checked against the
      interpreter, plus 3,133,236 of the sound driver's: no divergence. (The `0x000984` divergence
      seen before `explore` was an artefact of the guessed entries and is gone.)
- [x] **`make check` clean (2026-10-21).** "ok: translated code does what the interpreter does
      (600 frames)" and "ok: two runs alike (600 frames)".
- [x] **`tools/audit.sh` passes** (456 files): no keys, no personal paths, licences present.
- [x] **`make check-sound`**: "skipped: there is no driver for this game's sound program (it runs on
      the Z80)" — expected for D&D SoM; the Z80 program is fully translated.
- [x] **Repository line endings normalised to LF** (2026-10-21). The Windows checkout had CRLF in
      every tracked file, which made `tools/sync-common.sh --check` fail (160,114 line diffs, all
      CRLF-only, no content change). `git config core.autocrlf false`, `core.eol lf`, then the
      working tree was rewritten to LF. `git status` is now clean except the new `games/ddsom/`.
- [x] **Ghidra located** at `C:/tools/Ghidra/Ghidra_12.1.4_build/ghidra_12.1.4_PUBLIC/`
      (Ghidra 12.1.4). The decrypted program is imported (`cps2/ghidra_project`, `ddsom_program.bin`,
      68000 BE, base 0) and 978 functions exported (`cps2/ghidra_out/functions.txt`). The scripts
      `cps2/ghidra_scripts/DecompileOne.java` (decompile the function at an address) and
      `cps2/ghidra_export.bat` / `cps2/ghidra_at.bat` (the wrappers) work headless.
- [x] REA available as CLI `rea.exe` (`rea@6.7.0`,
      `C:\Users\azure\AppData\Local\Author Software\nvm\.nodejs\rea.exe`). Not run yet.
- [x] **Four players and free play (2026-10-21).** The board's own setting, the game's "Chute type"
      (Configuration → System), is now the port's own setting: `board_players` (as the board is set /
      Two players / Four players) and `free_play`. How it was found and how it works:
      - The game keeps the board's settings in a record at `A5 - 0x6272` (`FF1D8E`); the routine at
        `02D358` takes the record and fills the game's own settings from it. The record's second byte is
        the index of the setting the board is set to, which the game's table at `2D40E` turns into the
        chute type (`FF8021`) and the coin mode (`FF8022`).
        - The table's rows 10 and 11 are the only ones that give a chute type of 3, which is four
          players. So the record's second byte is 10 for four players (the board's own is 2, for two
          players, two chutes, multi).
        - The index 14 that the game's own menu shows as "four players, two chutes, single" is a menu
          index, not one of the table's rows: the game translates it to 9 before it reads the table.
      - The port's hook is at `02D358`, the block that applies the settings, and it puts the chosen
        index in the record before the block reads it.
      - Measured, with the four players each putting a coin in and pressing start
        (`cps2/fourp.movie`): the game counts the players that are playing at `FF802C` — 4 with the
        board set for four, 0 with the board as it is (the game counts only the players that joined
        when the board allows more than two), and 0 when only player one's coin goes in.
      - The credits are a word per chute: `FF806C` (the first, `0005E2` adds one when the coin of
        player 1 is in, `01F82A` and `0E0562` take one away) and `FF806E` (the second, `01C468`).
        Free play keeps both up, so any player can start.
      - The choices are kept in the `[features]` section of the `settings.ini` the launcher writes
        (`board_players=2`, `free_play=1`); the port reads them when `--settings` names the file, which
        is what the launcher does.
- [x] **Verification of the four players (2026-10-21).** `make check` clean ("ok: translated code does
      what the interpreter does", "ok: two runs alike"). `make check-blocks`: 25,009,711 translated
      blocks checked against the interpreter, no divergence — the hook changes nothing the code does. A
      headless run reports `cpu: native`, 0.000% of the instructions in the interpreter, ~680 fps.
- [x] **The first readable rewrites (2026-10-21).** Two of the game's routines are written again as
      C++ in `games/ddsom/src/board.cpp`, and `--check-rewrites` runs each against the game's own code
      and compares what each leaves behind:
        - `ApplyBoardSettings` (the routine at 02D358, the one that applies the board's settings):
          **0 differences** in memory, registers and flags, and the same time (622 cycles).
        - `PlayerJoins` (the routine at 02DACA, the one that counts a player joining): **0 differences**,
          the same time (128 cycles), and 4 calls with the four players joining.
      The check is real: a deliberate one-byte change to a rewrite is reported at once.
      What it took to get right, for the next one:
        - The tables are named by their distance from the instruction, and the 68000's rule is that the
          base is the address of the extension word: the chute type and the coin mode are read from
          `2D40E` and `2D40F` (interleaved, a word apart per index), the continue setting from `2E426`
          (not `2D426`), and the board's switches from `2D38C`.
        - The tables are read with the index *doubled*, so the array is indexed by the index itself, and
          the routine leaves D0 holding the index doubled and D1 the index.
        - The routine leaves A4 pointing at the record, and the check compares it.
        - **A rewrite and a hook cannot be at the same address**: `AddHook` replaces whatever is there.
          The port's own choice of how many can play is therefore a hook at the blocks that reach the
          routine with the record ready (`02D344`, `02D33E` and `02D53E`), and not part of the rewrite,
          which has to do exactly what the game's own routine does. A hook at the routine's own start
          (`02D358`) is too early as well: the record is written after it, by the routine at `02D7AA`.
- [x] **`check_rewrites.sh` and `movies/menu.txt`** (2026-10-21): the game's own check script, with the
      movie that takes the game into its own operator menu (where the settings are read too).
- [x] **The toolkit's own check, made to work on Windows (2026-10-21).** Two of the toolkit's helpers
      were passing here and doing nothing, and that is worth knowing about:
        - `tools/at-once.sh` put the checks' own words in a temporary folder, and the folder was not
          always where the shell that writes there could see it: the checks never ran, and the script
          said they had. The folder is now made under the repository's own work folder, and the script
          leaves nothing behind (`at_once_clean`).
        - `tools/audit.sh` kept its list of the files in a temporary folder too (the same problem), and
          the `grep` of a Windows shell does not know the `{n,}` repetition: the pattern was taken for
          the name of a file, and the checks that use it found nothing. The searches that need the
          repetition now go through `tools/find-hex.py` (a Python regular expression, with the pattern
          and the list of the files given in files, as a shell's `xargs` changes a pattern that has the
          characters it treats as its own).
      Both were found by making the checks fail on purpose: a rewrite broken by one byte, and a key
      written into a file that is in the tree. The audit now says so (and does not say so when the
      tree is clean), and the check of the rewrites exits non-zero when a rewrite differs.
- [x] **The gameplay's own machine, as rewrites (2026-10-21).** The game runs its play as a table of
      cases: a case is a list of steps, and two routines make it work. Both are written again as C++ in
      `games/ddsom/src/cases.cpp`:
        - `LoadCase` (the routine at 001A32, called from 479 places in the game): **40 calls, 0
          differences**, the same time (150 cycles).
        - `NextStep` (the routine at 001A56): **1196 calls, 0 differences** over the menu's movie, and
          **19,226 calls, 0 differences** over the play's movie (`movies/play.txt`: the four players in,
          and playing). Its time varies with the step (-52 to 132 cycles about the average, which is
          right).
      What the game's own code for them says:
        - The table of cases is in the work RAM at A4: the number of the case is a word's index into
          it, and the word there says where the case's steps are, as an offset from A4 (signed). The
          routine leaves D0 holding that offset and A4 pointing at the case's first step.
        - A step is a word of the number of the step, a word of what the step is, and three longs. The
          structure in A0 keeps the number of the step left, what the step is, the three values and a
          pointer to the step.
        - `NextStep` takes one off the number of the step left; when there are none left, a step of the
          kind 0 ends the case's steps there, a step of the kind 0xFE ends the case (the game is told
          so by the carry flag), and any other kind points at the steps that come next.
        - The kind goes into D0 with a *byte's* move, so the rest of D0 is left as it was: the check
          catches that only over a long run (306 of 16,709 calls).
- [x] **The game's trigonometry, as a rewrite (2026-10-21).** The routine at 001A94 works out the sine
      and the cosine of an angle by the game's own tables (the tables at 1AB4, one for each of the
      kinds of angle; each table is 64 bytes, sixteen pairs of words, one pair for each of the sixteen
      angles, the sine and the cosine, 256 being a full turn's worth of a length). It is written again
      as C++ in `games/ddsom/src/cases.cpp` as the rewrite `Trigonometry`. The play's movie does not
      reach it yet, so it was checked against the game's own code for every kind and every angle
      (24 kinds x 16 angles): the same answer in every case, and the values are indeed
      sin and cos of the angle (each 22.5 degrees) times 256.
      The tables are in the program, not in the work RAM: a rewrite reads them with `Code16` and
      `Code32`, not with `Mem16` and `Mem32`.

## ROM mapping (authoritative — from MAME `ddsom`)

| Region            | Files (board order)                                              | Size   |
|-------------------|------------------------------------------------------------------|--------|
| 68000 program     | `dd2e.03e dd2e.04e dd2e.05e dd2e.06e dd2e.07 dd2e.08 dd2e.09 dd2e.10` | 4 MB |
| Graphics (obj)    | `dd2.13m dd2.15m dd2.17m dd2.19m dd2.14m dd2.16m dd2.18m dd2.20m`     | 8 ×    |
| Z80 sound program | `dd2.01 dd2.02`                                                 | 256 KB |
| QSound samples    | `dd2.11m dd2.12m`                                               | 4 MB   |
| CPS-2 key         | `ddsom.key` (OBTAINED, CRC `541e425d` — matches MAME)             | 20 B   |

(Extra files in the zip, `dd2ex.03e`/`dd2ex.04e`, are not referenced by the `ddsom` machine and
are ignored by the port.)

## Plan (tasks)

### Phase 0 — inventory & decryption (DONE)
- [x] Extract ROM; identify set; fill `game.cfg` CRCs.
- [x] **Obtain `ddsom.key`** → sourced from the internet (ArcadeHacker CPS-2 key table), CRC-verified, placed in `cps2/ddsom/`.
- [x] **Install the dev environment** → MSYS2 + pacman at `C:/msys64`; make/ninja/cmake/gcc/python/zlib/SDL3 installed; PATH persisted.
- [x] **First build** → `make GAME=ddsom ROM=<the rom> port` builds `build/ddsom/ddsom.exe`;
      the port self-decrypts the program on the first run (`--dump-program`) and re-runs.
- [x] **Translation** → `recomp.py` wrote `native_000..008.cpp` + `z80_000..001.cpp`; the run is `cpu: native`.

### Phase 1 — bootable port (DONE)
- [x] `make explore` to find the game's code by made-up play (writes `work/ddsom/entries.txt`).
- [x] `make check` (translated code vs interpreter, frame by frame) — no divergences.
- [x] `make check-blocks` (every translated block beside the interpreter) — no divergences.
- [x] `make check-sound` — the game runs on the Z80, which is fully translated.
- [x] Verify it reaches the title and is playable (4 players) — done with a movie
      (`cps2/fourp.movie`): the game counts the players that joined at `FF8079` (4 with the board set
      for four, 2 with the board as it is).

### Phase 2 — reverse engineering (REA + Ghidra)
- [x] Load decrypted `work/ddsom/program.bin` in Ghidra (68000, 4 MB) — done, 978 functions exported.
- [x] Map RAM: the board's settings record (`FF1D8E`), the setting index (`FF801D`), the chute type
      (`FF8021`), the coin mode (`FF8022`), the players that joined (`FF8079`), the credits of the two
      chutes (`FF806C`, `FF806E`), the game's state (`FF8072`).
- [ ] Identify readable routines worth rewriting (see docs/READABLE.md).

### Phase 3 — readable rewrites & beat-'em-up modes
- [x] `src/ddsom.h`: the game's memory named, with how each was found; `src/game.cpp`:
      `RegisterGame` with free play; `src/board.cpp`: the board's settings, and the first rewrite
      (`ApplyBoardSettings`, the routine at 02D358) — checked against the game's own code, 0 differences.
- [ ] Use `beat/modes.cpp`, `beat/model.cpp` (shared beat-'em-up support) for 4-player modes.
- [x] `make check-rewrites` per routine (`games/ddsom/check_rewrites.sh` + `games/ddsom/movies/`).
- [ ] More rewrites: the routines the game's code spends its time in (the ones in `entries.txt` that
      the exploration found), each checked the same way.

### Phase 4 — verification
- [ ] Full playthrough movies; `check`, `check-blocks`, `check-sound`, `check-rewrites` clean.

### Phase 5 — build & release
- [ ] `make launcher` / `make app`; `check_release.sh` within `release_allowed = 150`.

## Open questions
- `players = 4`, `buttons = Attack | Jump | Magic` assumed (D&D SoM is 4P, 3 buttons). Verify.
- `code_end = 0x400000` matches the 4 MB program region; may shrink once real code extent is known.
- `rom_parent` set to `ddsom` (self-contained set); adjust if it is a clone of `ddon2`.

## Controls (player 1, keyboard)

Written in `work/ddsom/settings.ini` (the port reads it once at start; the launcher's Controllers
page writes the same format):

| Key | Action |
|---|---|
| Arrow keys | walk (up, down, left, right) |
| `A` | Attack (button 1) |
| `S` | Jump (button 2) |
| `D` | Magic (button 3) |
| `F` | a 4th button (the board has four; the game uses three) |
| `Left Shift` | Coin (insert a coin) |
| `Return` | Start (player 1's start button) |

Also: `Esc`/`F1` open the in-game menu, `F5` saves a state, `F9` loads it back, `F6`/`F7` change
the slot, `P` pauses, `O` steps one frame, `-`/`=` change the speed, `F11` full screen.

To check the file is read as intended (the port's own code, `port/bindings.h`):
```bash
g++ -std=gnu++20 -O1 -I port -I recomp-common/launcher/src -I "C:/msys64/ucrt64/include" \
    work/ddsom/check_bindings.cpp -L "C:/msys64/ucrt64/lib" -lSDL3 -o check_bindings.exe
```

## Save states

- `F5` saves, `F9` loads, `F6`/`F7` pick the slot. The file is `state<n>.bin` beside the EEPROM
  (`work/ddsom/state1.bin`), ~320 KB: the whole machine's state (68000, Z80, QSound, video chip,
  EEPROM, RAM, video/object RAM, timers).
- The board's own settings memory (the EEPROM the game writes its high scores and settings to) is
  kept in `work/ddsom/eeprom.bin`, passed with `--eeprom`.
- Verified: `--state-save 300:FILE` then `--state-load 300:FILE` both run clean.

## How to build here (Windows / MSYS2)

```bash
cd <the project>
export PATH="/c/msys64/usr/bin:/c/msys64/ucrt64/bin:$PATH"
export CMAKE_PREFIX_PATH="C:/msys64/ucrt64" ZLIB_ROOT="C:/msys64/ucrt64"

# CURDIR must be a NATIVE path: the CMake/Ninja on Windows do not read MSYS paths (/c/...).
make GAME=ddsom CURDIR="<the project>" ROM="<the rom>" all
```

Outputs:
- `build/ddsom/ddsom.exe` — the port (needs `SDL3.dll` and `zlib1.dll` beside it).
- `build/ddsom/ddsom-launcher.exe` — the launcher (same DLLs).
- `work/ddsom/program.bin` — the decrypted 68000 program (4 MB).
- `work/ddsom/entries.txt` — the 4,456 entries `explore` found.
- `work/ddsom/gen/native_*.cpp` — the translated code.

Run it headless: `./build/ddsom/ddsom.exe --rom "<the rom>" --headless --auto 7:400 --frames 3000 --stats`

## Phase 2 — reverse engineering (REA + Ghidra) — next

The decrypted program is ready: `work/ddsom/program.bin` (4 MB, 68000, base 0x000000).
- Ghidra 12.1.4: `C:/tools/Ghidra/Ghidra_12.1.4_build/ghidra_12.1.4_PUBLIC/`
  (`analyzeHeadless.bat` for a scripted run; the `m68k` language is in `Ghidra/Processors/68000`).
- REA 6.7.0 CLI: `C:\Users\azure\AppData\Local\Author Software\nvm\.nodejs\rea.exe`
  (`rea open`, `rea analyze`, `rea decompile`).
- `work/ddsom/program.lst` is the disassembly of everything the translator decoded — a ready-made
  map to drive either tool, with `>` marking each block's start.
- What to map first: the RAM fields the beat-'em-up support expects (player records, health,
  round/scene, credits, board settings, character select) and the game's own entry points, so
  `src/game.cpp`'s `RegisterGame` can offer free play, cheats and 4-player modes.
