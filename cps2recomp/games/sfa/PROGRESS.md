# Street Fighter Alpha: progress

The European arcade release of 1995-07-27 (ROM set `sfa`, the parent set), *Street Fighter Alpha: Warriors' Dreams*,
on Capcom's CPS-2 board. Started 2026-10-17. Its sources are its own (`src/`), not Alpha 2's: see "Alpha 2's
sources" below.

## Where it stands (2026-10-18)

- Boots and plays on the toolkit's own machine: the warning, the QSound logo, the introduction, the title, the
  attract mode's fights and rankings, the character choice, versus fights, the whole one-player ladder to the last
  opponent, an ending, the score ranking and the title again; seen in a window, played and listened to.
- 68000 program: 64,023 instructions translated to C++ (3,826 entries from made-up play, `make explore`). No
  instruction falls back to the interpreter over runs `explore` never made: four ladders to the ending, two
  versus runs, 20,000 frames of attract, the test menu and plain two-player play (checked 2026-10-17); the
  interpreter stays only as the fallback for a place no run has reached. The code ends at 0x054AC6 (`code_end`
  stays 100000).
- Sound: Capcom's sound driver **1.05A** (`version 1.05A /CPS2 1993 / JUNE`, at 0x0F in `sfz.01`): not the
  toolkit's 2.01b, so the Z80 program runs translated, as Alpha 2's 1.05C does: 411 entries, 2,326 instructions,
  all of it (the sound test, `--sound-test`, every song in turn, seeds 1-8). Listened to: right.
- The features every game has: free play (Enhanced), straight to the title, the board's settings (the Board
  page), cheats (no damage for each player, one-hit knockouts, infinite time), save states, run-ahead, arcade
  timing, a custom soundtrack (its songs named by number until found), the in-game menu and the launcher.
- Not yet: a training mode, the alternative modes, widescreen, custom colours (below, "Next").

## Alpha 2's sources

Capcom made Vampire Savior 2 and Vampire Hunter 2 from Vampire Savior's program, and their ports build its sources
(docs/TOOLKIT.md). Alpha 1 and Alpha 2 were compared for the same before anything was written (2026-10-17):

- `tools/portmap.py` from Alpha 2 to Alpha 1: 254 anchors, 27.9% of the instructions it reaches placed (4,838 of
  17,311), 5% of all Alpha 2 has translated. Vampire Savior to Vampire Savior 2 placed 76%.
- What the instructions do, the places they name left out: 38% of Alpha 2's code and 50% of Alpha 1's is in a run of
  eight found somewhere in the other. The two share much of Capcom's common code, in other places.
- The routines Alpha 2's sources hook: of the 200 code addresses they name, the run of 12 instructions that starts
  there is **nowhere in Alpha 1 for 161** (126 even for a run of 6), every one of hit.cpp's, damage.cpp's,
  machines.cpp's and training.cpp's among them. Alpha 2 rewrote the fight's code.
- And Alpha 2's sources name their places as plain addresses: to be shared, about 309 would become `At()`, with
  Alpha 2 proved unchanged.

So this game has its own sources. Where a routine has the same shape in both (the settings memory's, the settings
read, a game's beginning, the damage taken off health), Alpha 2's sources are a guide to reading it.

## The game's memory and routines (found by experiment)

Variables are reached from A5 = FF8000, as in Alpha 2; many of Alpha 2's are here 0x100 further on.

- **Where the game is** (`sfa.h`): words at FF8000, FF8004, FF8008, FF800C, each a step of the one before (2 in a
  game; 2 choosing, 4 the match; 4 a round on; 2 fought, 0 its introduction), as in Alpha 2. Traced with `--watch`:
  the board's start goes 2, 4 (the warning, frames 108-289), 6, 8, 0 (frame 475); the show then 2, 4, ..., 10 the
  introduction (frame 948), whose step 0C is the title fading in from white (frame 2122); 12 on the attract fights.
  With a coin in, from that 0 to 2 and 4 (frame 486): the title asking for Start. FF8002 counts down each frame.
- **Credits**: a byte at FF8170 (RAM dumps after 0, 1, 2 and 3 coins; one coin a credit as a new board has it).
  Alpha 2's is FF8076.
- **The players**: records at FF8400 and FF8800 (0x400 long, as Alpha 2's). Health a word at **+40** (Alpha 2's
  +50), full 0x90; +42 the bar shown, which follows it; +14A a copy. Found by RAM dumps through a fight.
- **Damage**: a hit's comes off at **00FA06** (`sub.w d3,0x40(a1)`, the victim at A1), a throw's at **021148**
  (`sub.w d3,0x40(a4)`); the hit routine also takes one point off without D3 (00FA20-00FA26, from 00F8E2 and
  00F906), its two ways meeting again at 00FA2A. Found with `--watch` on the health. The cheats are hooks there
  (`cheats.cpp`).
- **The round's clock**: a byte at **FFAE09** (A5+2E09), seconds left: 99 (0x63) at a round's start (004378), a
  second off at **004B3A** (`subq.b #1,0x2E09(a5)`; below nothing, time is up: A5+2E0D set, 005736); A5+2E0A counts
  a second's frames (0058F6 sets them). FF365D is the picture's copy, in BCD. Found by RAM dumps
  against the clock shown (91, 87, 83, 79) and `--watch`.
- **The board's settings**: read once as the board starts, at **00B2BA** (called at 001102, returning to 001108),
  from the settings record at A5-5E7E into variables at A5+198 on: Alpha 2's routine at 00ACF0, each variable 0x100
  further. A game begins at **004076** (Alpha 2's 004B6C), which copies the difficulty to A5+1BE. The values and
  their names, from the test menu's Configuration pages, each stepped through with the variable read after each
  step (the board's own in brackets):

  | | A5+ | values |
  | --- | --- | --- |
  | Continue | 198 | 0 off, 1 on (on) |
  | Demo sound | 199 | 0 off, 1 on (on) |
  | Difficulty | 1A0 | 0 easy, 1 normal, 2-6 hard 1-5, 7 expert (normal) |
  | Damage level | 1A1 | 0 low, 1 normal, 2 high, 3 maximum (normal) |
  | Timer speed | 1A2 | 0 slow, 1 normal, 2 fast, 3 fastest (normal) |
  | Game speed | 1A3 | 0 normal, 1-2 turbo 1-2, 3 free select (free select) |
  | Max round | 1A4 | 1, 2, 3: one, three, five rounds (three) |
  | Event | 1A5 | 0 off, 1 one match, which clears continue (off) |
  | Automatic | 1BB | 0 off, 1 free select (off) |

  The test menu edits these variables themselves. The collection's launcher shares a feature between games by name
  and choices: Difficulty, Timer speed, Event and Automatic are shared with Alpha 2's, which mean the same. Game
  speed has choices of its own (four; Alpha 2 has five). Damage level has Super Puzzle Fighter II Turbo's choices
  (Low, Normal, High, Maximum), where it means the counter gems an attack sends, so it is marked as this game's own
  (`OwnFeatures`, game.cpp).
- **The settings memory**: read and written at **00B942** and **00BA0A**, variables A5-5D7E (buffer), -5D78
  (count), -5D74 (start), +1CC (busy). Instruction for instruction Alpha 2's 00B444 and 00B50C (28 and 49 of them).
  The toolkit's shortcut (`runtime/board.h`) is **not used**: the routine they call to drive the chip's lines
  (00BB28) also sets bit 0x10 of the board's output port 804040 from A5+63 bit 2, where Alpha 2's (00B62A) puts in a
  copy kept at A5+88, and the shortcut never writes the port. It only saves time, so the game's own routines run.
- **The music**: the sound program's table of songs at 0x9006, four bytes a song, the word at 0x9000 the count
  (0x300; 498 entries used), read the same way as Alpha 2's (Z80 code at 0206-021D).

## Checked

- `make GAME=sfa check` (translated code against the interpreter, block by block; two runs alike): the ladder movie
  (40,000 frames, with and without its features), the versus movie (12,000), made-up play with the features on
  (40,000), a 70,000-frame ladder with one-hit knockouts. All ok.
- `make GAME=sfa check-blocks`: the ladder movie (361,817,107 68000 blocks, 349,459,882 of the sound program's),
  versus (105,755,806 and 82,756,919), a whole made-up ladder of 130,000 frames (1,171,002,117 and 837,912,573),
  40,000 frames with the features on, 30,000 of the test menu with a board choice. No difference.
- `make GAME=sfa check-release` (40,000 frames): two runs of the build with no translated code alike; its pictures
  the translated build's but for 3 frames, ending on the same.
- **With every feature off the game runs as with no sources**: the interpreter (`--cpu interp`, which takes no
  notice of where blocks start) gives the same per-frame hashes with `src/` and without it (6,000 frames of
  made-up play). The translated build's hashes move when entries are added (docs/BUILDING.md), which is why the
  interpreter is the measure here.
- Straight to the title: the per-frame hashes are the same with it on and off; the picture comes back at frame
  2124 (the title's white), 485 with free play.
- The board's settings: each of the 31 choices puts the right value in its variable as the board starts; Expert's
  copy (A5+1BE) is 7 when a game begins; the test switch puts the board's own values back; the settings memory file
  is the same with and without a choice.
- Save states: a state saved at frame 3,000 of made-up play, loaded at 3,000 into a run that had played otherwise:
  the 3,000 frames after it the same as the first run's. Run-ahead 1 and 2: the state the same as without it, all
  6,000 frames; the picture shown at N the plain game's at N+1 (N+2) but where the input changes.
- The custom soundtrack (a test tone as a playlist, 29 pieces of music over the ladder): the pictures the same as
  without it, all 27,000 frames, and the 68000's work RAM at the end. (`--hash-log` and `--hash-game` differ from
  the first piece played from a file, in Alpha 2 too: the sound side is told "the music stops" instead.)
- Arcade timing: the ladder plays to its end, nothing to the interpreter.
- In a window (the contributor, macOS, 2026-10-18): the picture, the sound and music, the launcher, every feature
  above, a remapped control, save states, the test menu.

**On macOS, SDL 3.4.14 leaves the window black** (found here; asking the build for 3.4.16 or later is a toolkit
change of its own). With Homebrew's 3.4.14 every game's window stays black while the sound plays: its Metal GPU
device takes a frame's fence still being worked on for one done, so after the first frames every swapchain picture
is skipped (SDL 3.4.16: "Fixed the SDL_QueryGPUFence() return value on Apple platforms"). A release build compiles
SDL 3.4.16 itself.

## The movies

Scripted, as Alpha 2's: `ladder.txt` (with `free_play,no_damage,one_hit`: Start, choose, then forward and a hard
punch over and over) takes Ryu to Sagat, his ending, the score ranking and the title by frame 26,400; `versus.txt`
(with `free_play`) is Ryu against Ken, then the winner on against the computer.

## Tech debt

From the review of the pull request that brought this game in (tmcleroy/cps2recomp#34), left for a change of its own
because it touches other games' sources:

- **`board.cpp` and `title.cpp` are near copies of Alpha 2's** (and `board.cpp` of Vampire Savior's): `PutChoices`,
  the two hooks, `BoardBeforeFrame`, the flags; `TitleFrame` with its steps. Only the table of settings, the hook
  addresses, the mode/step/credit addresses and the labels differ. Toolkit helpers taking those (a settings table and
  two hook addresses; the mode, step and credit addresses and the title's steps) would replace the copies, so a fix is
  made once.
- **Alpha 2's copies have the two bugs fixed here**: its Board flags are not in the game's state (run-ahead can lose a
  choice put back after the test menu; a loaded state can let the test menu save the choices), and its skip to the
  title does not start again at a reset within it. The helpers would fix them there too; Vampire Savior's `board.cpp`
  is to be looked at for the first.
- **`SetLow16`** is a third copy (`sfa2.h`, `vsav.h`), and `PlayerOf` a copy of Alpha 2's: `SetLow16` belongs in
  `runtime/lift.h`.
- **The settings memory's shortcut** (`runtime/board.h`) could learn the line routine's other output bit (here bit
  0x10 of 804040, from A5+63), so that this game can use it too.

The helpers can be checked here with Alpha 1's and Alpha 2's sets (each game's hashes the same before and after,
the Board page's and the title's tests above); Vampire Savior's and the games built from its sources need their sets.

## Next

- A training mode, the alternative modes, widescreen and custom colours, as Alpha 2's port has them: written for
  this game's own fight code (its hit tests, damage, the players' state machine), found as above.
- The music's names (`CPS2RECOMP_SOUNDTRACK_LOG` over every stage and screen; `--music-test` for what the board
  plays once).
- The board's hidden characters (Akuma, Dan, M. Bison) and the Dramatic Battle need codes made-up play never
  enters: movies of them, played by hand, so that their code is translated and checked too.
