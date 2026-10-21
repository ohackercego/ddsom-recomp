# The toolkit

cps2recomp is the toolkit the games in this repository are made with: it makes native PC ports of Capcom CPS-2
arcade games (and CPS-1 games with QSound: [below](#a-cps-1-game)) by static recompilation. A game's 68000 program is translated to C++ ahead of time and compiled
into the port. There it runs on a small machine written for this toolkit: the video, the QSound chip, the sound
CPU (with its driver translated too, or rewritten as ordinary code), the board's registers and its settings
memory. No emulator is part of a port, and no game data is in this repository.

This page is for people making or working on ports. To play, see the [main page](../README.md).

## What a port gets from it

- **Translation ahead of time.** `tools/recomp.py` turns the game's 68000 program into C++, and
  `tools/recompz80.py` does the same for the sound CPU's. The code is found by reading the program
  and by made-up play (`tools/explore.sh`). Code it has not found yet runs in an interpreter, and is
  counted, so that the count can be driven to zero.
- **A machine of its own, not an emulator.** The 68000, the bus, interrupts and timing, the settings
  EEPROM, the ROM set and its cipher (`machine/`), the picture (`video/`), and the Z80 and QSound
  (`sound/`). Capcom's sound driver, version 2.01b, is also there as ordinary C++ (`sound/driver.cpp`),
  used in place of the Z80 for games that have that version.
- **A native picture.** SDL's GPU API (Metal, Vulkan or Direct3D 12), with SDL's renderer where
  there is no GPU device, and a wider picture (up to 16:9) for games whose sources ask for it. The
  Display page lets players choose how it fits the window: sharp, whole pixels or smooth; 4:3, square
  pixels or the whole screen; and the window's size.
- **Readable rewrites, with proof.** A game's routines can be rewritten by hand as ordinary C++. A
  checker runs the original and the rewrite on every call and compares memory, registers, timing and,
  when asked, video RAM ([docs/READABLE.md](READABLE.md)).
- **A training mode's parts.** `fight/` has what fighting games share: frame data and a frame meter,
  a log of every move, the input display in numpad notation, the combo readout, a dummy that stands,
  crouches, jumps, guards, records and replays, and hit boxes drawn through the game's camera. A
  game's sources only say, from its own memory, what the players are doing
  ([docs/FIGHT.md](FIGHT.md)).
- **The fighting modes.** Survival, the endless battle (on one stage, stages that fade one into another, or a floor
  that scrolls through them) and the ring out are `fight/battle.h`: each match made ready on a spare machine and
  put in the place of the last, the fighters drawn over a stage another machine shows. A game describes where it
  keeps the players, the camera and the round's result, and makes the ring out's two movements at its players'
  step: one knocked out thrown back and falling through the floor, the winner put in the next match in the air
  as it was ([docs/FIGHT.md](FIGHT.md)).
- **The beat-'em-up modes.** Attract mode (all of a game's show, or only some of its parts), a score attack (one
  credit, no continue or cheats, the best score kept), a stage viewer (a game taken unseen to a stage and its camera
  moved along the stage's way, a few pixels a frame so that the game draws as it goes) and a character viewer (a
  character alone at any size, with its animations put into its record frame by frame) are `beat/beat.h`. A game
  says how it gets to a stage, where its camera, its characters' animations and their objects are, and leaves out
  the parts of its show not wanted.
- **The model viewer.** One character alone, on a stage or on plain white or black, at a quarter of its size to 25
  times, moved with player 1's controls or going through any of its animations (one, or all in turn, repeated, in slow
  motion) is `model/model.h`, for the fighting games' "Model viewer" and the beat-'em-ups' "Character viewer" alike: the
  menu's page, the animations shown and stepped, and the picture made at the size wanted. A game gets to a character
  alone its own way and says where its animations and frames are, how a frame is put in a character's record, which
  objects are the character's and where it stands.
- **Players' own colours.** Text files a character, or a ROM file changed with PalMod, recolour the
  characters, and a colour editor in the model viewer makes them; a game only says where its colours
  are (`fight/colours.h`).
- **A custom soundtrack.** The player's own music files in place of the board's music, caught where the game
  asks the sound CPU for it, played in the game's time; a game only says where its sound program's table of songs
  is, and names its music (`machine/soundtrack.h`, `port/soundtrack.h`; [docs/FEATURES.md](FEATURES.md)).
- **The board's timing, as an option.** The 68000's waits on its memory and the video chip's DMA counted as the
  board has them, so the games slow down as on the board; checked against a cycle-exact model of the board
  ([docs/TIMING.md](TIMING.md)).
- **Features players expect.** Save states, run-ahead (a press seen a frame or two sooner, with the
  game running exactly as without it), speed, pause and frame advance, slow motion, cheats and
  options a game declares once, and the in-game menu and launcher the recomp ports share
  ([recomp-common](https://github.com/tmcleroy/recomp-common); [docs/FEATURES.md](FEATURES.md)).
  Every change to how a game plays is opt-in: with all features off, the game runs as the board.
- **Checks at every step.** Translated code is checked against the interpreter, the sound driver's
  every chip write against the Z80's, and runs of the same input movie must give the same per-frame
  hashes ([docs/VERIFYING.md](VERIFYING.md)).
- **macOS, Linux (Steam Deck included) and Windows** through SDL 3. CI builds and starts the toolkit
  on all three.

## Trying it with no game

`make check` needs no game. It audits the files (`tools/audit.sh`: nothing from a game, no keys, no
personal paths), checks the copy of recomp-common, builds the program with no translated code and
the launcher for the stand-in game in `example/`, and starts both.

```sh
git clone https://github.com/tmcleroy/cps2recomp
cd cps2recomp
make check
```

You need a C++20 compiler (Clang, GCC or MinGW-w64), CMake, Ninja, Python 3, zlib and SDL 3. On
macOS that is `brew install cmake ninja sdl3`. [CONTRIBUTING.md](../CONTRIBUTING.md) has the rest, and
`.github/workflows/ci.yml` shows exactly what each system installs.

## How a game uses it

A game is a folder in `games/`:

```
games/<id>/     game.cfg (the ROM set's files and checksums, and what the build, the launcher and the checks
                need to know), src/ (names, features, rewrites), movies/, check_rewrites.sh, PROGRESS.md,
                README.md and docs/ (its page for players, what it has, how it works, what is next)
```

Everything is built from the top with `GAME=<id>` (`make GAME=<id>`, `make GAME=<id> explore`,
`make GAME=<id> check`, `check-blocks`, `check-rewrites`, `check-release`, `launcher`, `app`: `build.mk` has
them), and `make games` and `make collection` build every game and the collection's launcher
([BUILDING.md](BUILDING.md)). `example/` is a game with no game in it, which CI builds; any game in `games/` is a
complete one. Start a new game from either: a `game.cfg`, an empty `src/`, `make GAME=<id>` twice and
`make GAME=<id> explore`, then the game's own names and features. The collection's launcher picks up every game
in `games/` (`tools/gamecfg.py --collection` writes its table from their `game.cfg` files).

The player's ROM set is only read. The program is deciphered with the set's own key file, as the
board does when it starts; no key is in this repository.


## A game made from another's program

Capcom made several games from one program, changed here and there (Vampire Savior 2 and Vampire Hunter 2 from
Vampire Savior's). Such a game builds the first game's sources rather than having its own: its `game.cfg` says
`sources = vampire_savior`, CMake compiles that game's `src/` too, and the game's own `family.h` (in its folder, on
the include path) says what is its own.

- The shared sources name places as `At(0x...)` (the ROM or work RAM, as the first game has them) and `AtA5(...)` (a
  variable by its distance from A5), worked out when compiling from `family.h`'s list (`games/vampire_savior/src/places.h`).
  The first game's list is empty: a place is where it is said to be. A place the list lacks stops the build.
- `tools/portmap.py` maps the two programs: anchors (runs alike but for the places they name), then both programs'
  code walked side by side from the anchors and from jump tables, then what is left between two placed
  instructions matched by what the instructions do. It also pairs the places matched instructions name.
- `tools/places.py` writes `family.h`'s list from that map, choosing for a code place the candidate where most
  instructions on do the same; a line marked "by hand" is kept.
- What differs in the other game's code is a fact in `family.h` that the shared code follows (a byte the rules test,
  a range of tiles, the roster), or, where a routine is too different to share, the place 0 (`kNoPlace`, `lift.h`):
  no rewrite is made there and the game's own code runs. `tools/recomp.py` takes hooks written `At(0x...)` and
  places a `family.h` names.
- The shared rewrites are checked against the other game's own code like any rewrite
  (`games/vampire_savior/check_family.sh`).

## A CPS-1 game

The CPS-2 grew out of the CPS-1, and the toolkit runs CPS-1 games with QSound (the "CPS-1.5": Cadillacs and Dinosaurs
and its kind) too. A game says so in its `game.cfg`:

```
board = cps1
cps_b = 0A 0C 0E 00 02 04 16 16 16
```

`cps_b` is where the game's CPS-B chip has its registers (byte offsets, as MAME's table of the chips gives them for the
game): layer control, the four priority masks, palette control, and the layer control's bits that switch scroll
screens 1-3 on. With it, the machine (`machine/machine.*`) has the CPS-1's memory map and inputs (players 1 and 2 at
800000, the switches at 800018, players 3 and 4 at F1C000), a 12 MHz 68000, no raster interrupts, the settings memory
as 128 bytes, and the sound CPU's two memories where the 68000 sees them (F18000, F1E000); the video
(`video/video.*`) takes the object list from video RAM and draws the objects as one of the four layers, where the
layer control puts them, with the scroll screen just below them keeping its high pens (each tile's group picks one
of the four priority masks) in front of them; the ROM set has no program cipher, no graphics shuffle and its samples
as bytes. The board timing (`arcade_timing`) is the CPS-2's and stays off.

The sound CPU of these boards deciphers its program as it reads it (Kabuki: opcodes one way, other reads another,
both by address), and no ROM set carries the key. `machine/kabuki.*` works it out from the program itself when the
set is loaded: Capcom's QSound programs all start the same way, and every Kabuki key's swap keys are orders of 0-7,
so a search that splits the cipher's two halves leaves a few thousand candidates, of which one deciphers into code
that can be followed from its start without a fault. The Z80 reads opcodes through `Z80Bus::ReadOpcode`, and
`tools/recompz80.py` takes the opcode image (`--opcodes`). A CPS-1 board's graphics PAL maps tile numbers to ROMs; the
games here have them in order, and a game whose PAL does not will need its map in `cps_b`'s company.

## Working on the toolkit

A toolkit change is checked with every game: build each (`make GAME=<id>`) and run its checks
(`make GAME=<id> check`, `check-rewrites`; [VERIFYING.md](VERIFYING.md)). Anything about one game belongs in
that game's `game.cfg` or `src/`, not in the toolkit. A feature more than one game has should have the same
name and choices in each: the collection's launcher then shows it once, for all of them.

## Repository layout

| Path | What |
| --- | --- |
| `tools/` | The translators (`recomp.py`, `m68k.py`; `recompz80.py`, `z80.py`), `gamecfg.py`, the checks (`check.sh`, `check_sound.sh`, `check_release.sh`, `at-once.sh`), finding a game's code by made-up play (`explore.sh`), reading the listing (`lst.py`, `tracechanges.py`, `ramfind.py`), `gen_shaders.py`, `sync-common.sh`, `audit.sh`, `collection.sh`, and the release builds (`release/`) |
| `runtime/` | What translated code is written in terms of; hooks and readable rewrites (`lift.h`); features and settings (`game.h`); save states (`state.h`); spare machines a game's modes run beside the port's (`spare.h`) |
| `machine/` | The 68000, bus, interrupts, timing, settings EEPROM, ROM set and cipher; the custom soundtrack's catching of the game's music (`soundtrack.h`) |
| `video/`, `sound/` | The picture; the Z80 and QSound; Capcom's sound driver (version 2.01b) as ordinary code (`driver.cpp`) |
| `third_party/` | The sound file decoders the custom soundtrack reads with (stb_vorbis, dr_flac, dr_mp3, dr_wav), as their projects publish them |
| `port/` | The frontend (SDL 3), the custom soundtrack's player (`soundtrack.*`, `decoders.*`), putting the picture on the screen (`present.*`; shaders in `shaders/`, compiled into `shaders.inc` by `tools/gen_shaders.py`), the in-game menu adapter, the HUD, control bindings |
| `fight/` | What a fighting game's training mode has in common, its fighting modes, and the players' own colours ([docs/FIGHT.md](FIGHT.md)) |
| `model/` | The model viewer every game can have: the fighting games' "Model viewer" and the beat-'em-ups' "Character viewer" |
| `beat/` | A beat-'em-up's alternative modes: attract mode, score attack, the stage viewer (`modes.cpp`) and the character viewer (`model.cpp`, on `model/`) |
| `recomp-common/` | A copy of [recomp-common](https://github.com/tmcleroy/recomp-common) (the in-game menu, the settings rules, the launcher), made by `tools/sync-common.sh`; never changed here |
| `launcher/` | What this toolkit tells the shared launcher (`recomp-common/launcher`; SDL 3 + Dear ImGui): `src/description.cpp` has the words for a ROM set, the check of the player's, the game's arguments and the buttons that can be chosen, for one game's launcher or the collection's |
| `games/` | The games ([the main page](../README.md) has the list) |
| `example/` | A stand-in game with no game: what `make check` and CI build |
| `docs/` | `BUILDING.md`, `RELEASING.md`, `VERIFYING.md`, `READABLE.md`, `FEATURES.md`, `FIGHT.md`, this page; `screenshots/` (the READMEs' pictures) |
