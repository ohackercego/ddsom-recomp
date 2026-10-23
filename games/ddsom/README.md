# Dungeons & Dragons: Shadow over Mystara Recomp

> Part of **[CPS-2 Recomp](../../README.md)**: one download and one launcher for every Capcom arcade game this
> project has ported. This page is about Dungeons & Dragons: Shadow over Mystara; the [main page](../../README.md)
> says how to download and play.

An unofficial native PC port of the arcade game *Dungeons & Dragons: Shadow over Mystara* (Capcom, 1996, CPS-2 board)
for macOS, Linux (Steam Deck included) and Windows, made by static recompilation: a beat-'em-up for up to four
players, with free play, a stage to start at, a score attack, viewers for its stages and characters, the board's own
settings and cheats.

> **Beta.** The game plays through its first stages in made-up play and its code is checked instruction by instruction,
> but nobody has played it by hand yet. Expect rough edges, and please [report what you find](#feedback-and-contributing).

**No game files are included.** You need your own copy of the game: the arcade ROM set `ddsom` (USA, 950101) with its
key file, from a board you own. This project cannot help you get one.

This project is not affiliated with, endorsed by, or sponsored by Capcom. *Dungeons & Dragons* and *Shadow over
Mystara* are trademarks of their owners; "CPS-2" and "QSound" are names and trademarks of their owners, used here only
to say what the program works with.

## What it is

The game's 68000 program is translated ahead of time into C++ and compiled into the port, and so is its Z80 sound
program; what the program expects of the board is code written for this project — **there is no emulator**. The 68000
and Z80 machine code become native functions the host CPU runs directly. An interpreter exists in the tree, but only as
a fallback for any block not yet translated and for the `check` / `interp` / `blocks` verification modes. A `check`
run that compares the translated code against the interpreter reports **0.000% of instructions in the interpreter** —
every block is translated and runs natively.

- **Four players at once**, each with a keyboard layout or a gamepad of their own. The board as it left the factory
  takes up to four; the Board page's **Players** setting lets them join — four sharing one coin chute, or each with a
  chute of their own.
- Plays with a keyboard, a gamepad or an arcade stick; every key and button can be changed, for each player, in the
  launcher, where the buttons are called what the game calls them: **Attack**, **Jump**, **Magic** and **Select**. On a
  gamepad they start on West (X), South (A), East (B) and North (Y), as the collection's other games have their first
  four buttons.
- An in-game menu (Esc or `F1`; on a gamepad, the left stick's click or Start and Select together) with save states,
  speed, pause and frame advance, the cheats and every feature.
- **Pure** plays exactly as the board does. **Enhanced** (the default) adds free play and the extra players. Every
  other feature is a switch of its own.
- **Start at stage**: a new game starts at the beginning of any of the game's stages.
- **Alternative modes**, at the head of the in-game menu:
  - **Attract mode**: the game's own show on a loop, with its sound on.
  - **Score attack**: one player and one credit from stage 1, no continue and no cheats, for the best score, which is
    kept.
  - **Stage viewer**: each stage's scene with nobody on it, only the scenery and its music.
  - **Character viewer**: the game's heroes by themselves, from a quarter of their size to 25 times, moved with your
    controls or going through any of their animations, in slow motion if you like.
- **Custom soundtrack**: your own music files in place of the board's music (see the
  [main page](../../README.md#a-custom-soundtrack)).
- **Rapid fire**: a button held down is pressed again and again, for the first button (Attack) or for all of them.
- **Cheats**: infinite health, infinite lives and infinite time, for every player.
- The board's operator settings on the Board page, each as the board is set unless you choose; or the game's own test
  menu from the in-game menu.
- **Straight to the title**: at a start or a reset, the board's start runs unseen.
- The picture your way (Display page): sharp, whole pixels or smooth; 4:3, square pixels or the whole screen; and the
  window's size. Run-ahead takes a frame or two of input lag away.
- Your ROM set is only ever read. Settings, save states and the board's settings memory go in your own user folder.

## How it is built (native, not emulated)

The port is an ordinary C++ application that drives SDL3 directly; it loads and decrypts the CPS-2 ROM set itself.
There is no MAME or other emulator linked — a tree search finds no emulator sources, and the only "MAME" references are
comments that check each ROM file's CRC32 against MAME's `src/mame/capcom/cps2.cpp`.

- `tools/recomp.py` translates the 68000 program into `work/ddsom/gen/native_*.cpp` (the 68000, one file per block
  group); `tools/recompz80.py` translates the Z80 sound program into `work/ddsom/gen/z80_*.cpp`. These are compiled
  and linked into the executable (`CMakeLists.txt` globs `gen/native_*.cpp` and `gen/z80_*.cpp` into
  `add_executable`).
- At runtime the default mode is **native**: the translated functions execute directly. The interpreter is entered only
  for a block absent from the translated tables (none remain after `make explore`), or in the `check` / `interp` /
  `blocks` modes used to verify the translation.
- `runtime/state.h` serialises the whole machine (CPU, RAM, QSound, EEPROM) for save states — the same state a
  rollback netcode would rewind.

## What you need

- The ROM set `ddsom` (*Dungeons & Dragons: Shadow over Mystara*, USA 950101): `ddsom.zip` (or a folder with the set's
  files), **including `ddsom.key`** — the CPS-2 master key, which is *not* in the zip and must come from a board you
  own. The launcher checks every file by checksum and tells you which is missing or different.
- One of: macOS 13 or later (Apple silicon or Intel); Linux x86_64 or SteamOS; Windows 10 or 11, 64-bit.
- For four players: four gamepads, or keys chosen for the second, third and fourth players on the Controllers page.

## Playing

Dungeons & Dragons: Shadow over Mystara comes with [CPS-2 Recomp](../../README.md#playing): open its launcher and show
it your ROM set (drop `ddsom.zip` or its folder on the window, or choose the folder you keep your sets in), then press
**Play** beside Dungeons & Dragons: Shadow over Mystara. From a terminal, `cps2recomp-launcher --play ddsom` starts it
straight away; the game's own program (`ddsom`, beside the launcher) runs by itself too: `ddsom --help` lists what it
takes.

### Where to put the ROM and key

Keep your set out of the repository (it is copyrighted). The build and launcher look for it in `rom/ddsom/` by default
(Git-ignored), so a convenient layout is:

```
rom/ddsom/
  ddsom.zip          # the set's files (or extract them here)
  ddsom.key          # your CPS-2 master key, 20 bytes — not in the zip
```

Then either drop that folder (or the zip) on the launcher, or run the standalone program with:

```sh
ddsom --rom rom/ddsom        # a folder, or
ddsom --rom rom/ddsom.zip    # the zip
```

`--rom` accepts a folder of the set's files **or** the zip; `ddsom.key` is read from the same place. The set is only
ever read, never written. (You can also point the build elsewhere with `make GAME=ddsom ROM=<path>`.)

### Controls

Four players, each on their own keys or gamepad — no key is shared between two players. The defaults (all
changeable per player on the launcher's Controllers page):

| Player | Move | Attack | Jump | Magic | Select | Start | Coin |
| --- | --- | --- | --- | --- | --- | --- | --- |
| 1 | Arrow keys | `S` | `A` | `D` | `Z` | `Enter` | `Left Shift` |
| 2 | Numpad `8` `2` `4` `6` | `I` | `O` | `P` | `K` | `2` | `3` |
| 3 | `T` `G` `F` `H` (up/down/left/right) | `V` | `B` | `N` | `M` | Numpad `Enter` | Numpad `+` |
| 4 | `8` `4` `6` `9` (up/down/left/right) | `Q` | `W` | `E` | `R` | `7` | `0` |

The panel has six buttons; this game uses four of them. **Attack**, **Jump**, **Magic** and **Select** are the
four — Select is the one that changes character and item. The remaining two slots (`X` `C` for player 1,
`L` `;` for player 2, `,` `.` for player 3, `Y` `U` for player 4) are spare and unbound by the game.

Player 1 keeps the arrow keys and the letters beside them, so a game with more than one player still plays the
way it did when it had one. Players 2, 3 and 4 take what is left over, each with a set of keys no other player
uses.

On any SDL gamepad: the D-pad (or left stick) moves; **West (X) = Attack**, **South (A) = Jump**, **East (B) = Magic**;
**North (Y) = Select**; **Start** = Start, **Back** = Coin. Up to four gamepads, or any mix of gamepad and keyboard.

## Building from source

With your own ROM set in `rom/ddsom/` (ignored by Git), from the repository's top:

```sh
make GAME=ddsom            # twice the first time
make GAME=ddsom explore    # once: finds the game's code by made-up play and translates it
make GAME=ddsom run
make GAME=ddsom check
```

The first `make` configures the build; `explore` runs the toolkit that discovers and translates the 68000 / Z80 code
(writing `work/ddsom/gen/`); `run` plays it; `check` verifies the translation against the interpreter.
[PROGRESS.md](PROGRESS.md) has what was found of the game; [docs/BUILDING.md](../../docs/BUILDING.md) has every
platform.

## Feedback and contributing

Bug reports and pull requests are welcome: see [CONTRIBUTING.md](../../CONTRIBUTING.md) for the rules (above all:
nothing from the game, ever).

## License and credits

GPL-3.0-only (see [LICENSE](../../LICENSE)); [THIRD_PARTY_NOTICES.md](../../THIRD_PARTY_NOTICES.md) lists what comes
from other projects. The repository holds no game data: the names header's addresses are numbers.
