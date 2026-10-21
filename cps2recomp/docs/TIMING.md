# The board's timing

A CPS-2 board does not give its 68000 every clock. The port's **arcade timing** (the `arcade_timing` feature,
off unless chosen; `machine/machine.h`, `SetBoardTiming`) takes from it what the board takes, so a game runs short
of time in a frame, and slows down, where and as much as it does on the board. With it off the 68000 has every
clock, as the port has always run it: the games then do about a third more work in a frame than the board lets
them, and seldom slow down.

## What the board takes

- **The acknowledge.** The board answers the 68000 a clock late for every bus access outside 500000-7FFFFF: the
  ROM (instruction fetches too), work RAM, video RAM and the I/O ports; the QSound's shared memory, the extra RAM
  and the object RAM are not waited for (`M68K::busWait`, `WaitedFor`). The port counts a clock for each word of an
  instruction (as the 68000 fetches them), each byte or word of data it reads or writes, a word more where a one-word
  instruction goes elsewhere (a taken short branch, `rts`, `jmp (an)`, `rte`, `trap`: the 68000 fetches two words
  there in place of the one it had fetched ahead), and three for an interrupt (its acknowledge and the handler's
  first two words). The interpreter counts them as it goes; translated code adds them up over each run of
  instructions and charges them before one that may go elsewhere (`tools/recomp.py`), so the two stay alike cycle
  for cycle (`make check-blocks` with `ARGS="--features arcade_timing"`).
- **The video chip's DMA.** The CPS-A takes the bus to copy what it draws from into its own memory, a word every
  four clocks (`Machine::LineDma`): at every line, the row scroll's word (if row scroll is on); when the line reaches
  a new row of a scroll layer's tiles (every 8 lines for the first layer, 16 for the others, by their vertical
  scroll), that row (100, 98 and 30 words); and at the line after the game sets the palette's base, every enabled
  palette page (512 words each: about 12,300 clocks for all six, most of the vertical blank). That is 7 to 13% of a
  frame.

What the port's own code does in place of the game's (the readable rewrites, `runtime/lift.h`) takes the time its
original takes without the board's waits; and a rewrite that calls into the game's code runs it to the end at once.
So with arcade timing the game's own code runs in place of the rewrites, unless the game's sources need them
for something the port adds (`Game::rewritesNeeded`: training mode, the sources' own modes, widescreen, the cheats
that work inside a rewrite). There the timing is close, not exact. The port's own code takes none of the board's
waits: a hook only looks at the game (it reads memory without the 68000), and a rewrite charges what it says
(`runtime/recomp.cpp`, `RunHook`).

## How it was found and checked

The reference is [jtcps2](https://github.com/jotego/jtcores) (jotego's CPS-2 core for MiSTer and other FPGA
systems, GPL-3.0-or-later), simulated with Verilator: a model of the board's chips clock by clock, its acknowledge
from Loic Petit's study of the DL-1827 ([petitl.fr/cps2](https://petitl.fr/cps2/DL-1827.pdf)) and its DMA matched by
jotego to measurements of a board. No code of it is in this repository; the rules above are what it does. The
simulation was made to log, at the start of every vertical blank, the 68000's work RAM and how many of the frame's
clocks went to the DMA and to the acknowledge, and was played the same input as the port (Street Fighter Alpha 2:
coins, Start, the character select and a whole fight against the CPU, to the knockout and after: 5,671 frames).

- **Without the board's timing**, the port's 68000 does about 1.35 times the board's work in a frame: the board's
  memory test, CPU-bound, takes 14 frames a step on the reference and 10 or 11 on the port, and the two part ways in
  the attract mode's introduction of the fighters, the busiest scene the run reaches.
- **With it**, work RAM is the same as the reference's at every one of the 5,671 frames but 32 (5,639 the same): a
  few in the board's
  first frames and single frames in the fight where the stack area or a routine's place to keep registers is caught
  at a different point of its work as the vertical blank begins (the game's state is the same). The DMA comes to the
  reference's within 0.3% a frame (33,554 clocks against 33,650).

The run found more on the way, now part of the rules: the refetch after a taken short branch and the interrupt's
waits (the settings memory's routine, all short loops, was 10% fast without them); a rewrite that runs game code to
the end at once (it put two frames of work into one at a knockout); and the settings memory's shortcut
(`runtime/board.h`), timed without the board's waits, which the game's own routine now replaces. And
`make check-blocks` with the arcade timing on (every game's ladder movie, 6,000 frames: 33 to 38 million blocks)
found the interpreter's other ways of running an instruction counting the refetch differently, and hooks charged for
the memory they read: both fixed. With different timing a game can also reach code its exploration never did (Super
Puzzle Fighter II Turbo jumped into what the translator had taken for data): the interpreter runs it, and
`--record-entries` adds it for the next translation.

## What it does not do

- **Within a frame** the comparison is of the state at each vertical blank; the 68000 can be a few instructions
  further on or behind within a frame (the cases above). A difference that lands right at a frame's end can move a
  frame.
- **The settings memory** answers at once in the port, as it does in jtcps2. A real 93C46 is busy for a few
  milliseconds after a write, by an amount that differs from chip to chip, so a board's own timing differs a little
  there (the game writes it at a coin, at the end of a game and in the test menu).
- **The sound side**: the Z80 and the QSound are the port's as always. jtcps2 cannot run its QSound without the
  chip's own program (not part of a ROM set), so for the comparison its QSound said it was always ready; the port's
  kept its own timing (the game sees the sound side only in how soon the Z80 takes its requests).
- Only Street Fighter Alpha 2 was compared with the reference so far; Vampire Savior and Super Puzzle Fighter II
  Turbo have the same board and chips, and the same rules.

## How close it is

What this was compared with, what that shows, and the path to arcade perfect: [ARCADE_ACCURACY.md](ARCADE_ACCURACY.md).

## Logs

`CPS2RECOMP_BUS_LOG=FILE` writes, for each frame, the acknowledge waits and the DMA's clocks the port counted.
