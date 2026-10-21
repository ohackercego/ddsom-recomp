# How close the arcade timing is to the board

The port's **arcade timing** (the `arcade_timing` feature; the rules are in [TIMING.md](TIMING.md)) takes from the
68000 the clocks a CPS-2 board takes, so the games slow down where and as much as the board does. This page says
what it was compared with, what that comparison shows and leaves open, and what is still to do before the port can
be called arcade perfect. The work still to do is tracked in [#52](https://github.com/tmcleroy/cps2recomp/issues/52).

## What "arcade perfect" would mean here

For the same input from power-on, the port and a board would show:

1. the same game state at every frame: the 68000's work RAM, and the video and object RAM it writes;
2. the same picture at every frame, slowdown included;
3. the same sound, at the same moments;
4. the same frame rate and the same input-to-picture delay.

This needs to hold for every game the port carries, and with the port's own additions switched on where they claim
to keep the board's timing.

## The comparison that was done

**The reference.** [jtcps2](https://github.com/jotego/jtcores) (jotego's CPS-2 core for MiSTer), simulated
clock by clock with Verilator. Its 68000 wait states follow Loic Petit's study of the DL-1827
([petitl.fr/cps2](https://petitl.fr/cps2/DL-1827.pdf)), and jotego matched its video DMA to measurements of a real
board. It is a model of the board, not the board.

**The harness.** This is kept outside the repository, in a local checkout of jtcores at `bfced64` with four
small changes:

- `jtframe/verilator/test.cpp` dumps the 68000's 64 KB of work RAM at the start of every vertical blank, and logs
  that frame's DMA and wait clocks. `sdram.h` gets the `peek` this needs.
- `jtframe_68kdtack_cen.v`: the wait counter's width goes from 6 bits to 16. It overflowed during a long SDRAM hold
  in the simulation, which is something a board never does.
- `jtcps15_sound.v` (`DSP_ALWAYS_READY`): the QSound DSP always reports ready. The simulation can't run the DSP
  without the chip's own internal program, which isn't in any ROM set, and the sound program would otherwise wait
  for it forever.

The port was run on the same input script with `--features arcade_timing`, dumping its work RAM at the same point
of every frame. A small script (`track.py`) pairs each reference frame with an identical frame of the port's.
Pairs have to sit within 30 frames of the previous match. The stack area (`FF7C00-FF7FFF`) and two bytes
(`FF043C`, `FFDF63`) are left out of the comparison. `diffat.py` lists the bytes that differ at a given frame.

**The run.** Street Fighter Alpha 2 (`sfa2u`), 5,671 frames from power-on: the board's tests, the attract mode, two
real coins (no free play), Start, character select, a whole fight against the CPU to the knockout, and after.

### What it showed

| | Reference frames matched | First lasting divergence |
|---|---|---|
| Arcade timing off | 904 of 5,671 | frame 913 (attract mode, the fighters' introduction) |
| Arcade timing on | **5,639 of 5,671** | none |

- **Off:** the port's 68000 gets through about 1.35 times the board's work in a frame. The board's memory test
  takes 14 frames a step on the reference and 10 or 11 on the port.
- **On:**
  - From frame 80 to the end, the port's state follows the reference at a fixed offset of 11 frames (the port reaches each
    state 11 frames sooner), and it never drifts. The offset builds up during the boot sequence (frames 70 to 80), and its cause was not looked into.
  - The 32 frames that don't match are isolated frames: eight during boot, and 24 during the fight. In
    each one, a routine's saved registers or the stack area was caught at a slightly different point when the
    vertical blank began. The next frame matches again.
  - The DMA per frame comes within 0.3% of the reference's (33,554 clocks against 33,650).

The run also turned up several bugs, which are now fixed and part of the rules:

- the 68000's refetch after a taken short branch;
- the wait clocks of an interrupt;
- a readable rewrite that ran two frames of game work in one at a knockout;
- the settings-memory shortcut, which was timed without the board's waits.

### What it does not show

- **Only SFA2.** Vampire Savior, Super Puzzle Fighter II Turbo, Alien vs. Predator and the Vampire Savior variants
  (`vsav2`, `vhunt2`, `vsav_definitive`) were never compared and passed. They use the same board and get the same
  rules.
  - A first Vampire Savior run (`vsavj`, 6,000 frames, coins at 1,100 and 1,150) was made after the merge. It
    follows the reference at the same fixed 11-frame offset up to reference frame 1,958. It then diverges for good:
    one byte differs at 1,965 (`FF0861`), eight at 1,970, and 139 by 2,100. That is shortly after the coins, in
    the menus or the start of the fight. The cause hasn't been found, and the run itself hasn't been checked for an
    input mismatch.
- **Only work RAM, only at frame boundaries.** Pictures, video RAM, object RAM and the 68000's position inside a
  frame weren't compared. A raster effect or a mid-frame write can be early or late within the frame and still
  leave the same work RAM at the vertical blank.
- **No sound.** The reference's QSound was stubbed to always ready, so the Z80, the QSound and the timing of the
  68000-to-Z80 handshake are the port's own and were never checked. Only the port's previous behaviour was checked.
- **No real board.** Everything was compared with jtcps2. Where jtcps2 differs from a board, so may the port.
  - One known case: the 93C46 settings memory, which both answer at once. A real chip is busy for a few
    milliseconds after a write, by an amount that varies from chip to chip.
- **Not with the port's additions.** Training mode, widescreen, the sources' own modes, and cheats that work
  inside a rewrite keep the readable rewrites, and those don't take the board's waits. The timing there is close,
  not exact.
- **Not reproducible from this repository.** The harness, its patches, the input scripts and the comparison
  scripts live on one machine.

## Where it stands

For Street Fighter Alpha 2's game logic, the arcade timing matches a cycle-level model of the board frame for
frame. That covers a whole credit, with no drift beyond a fixed 11-frame offset from boot. Nothing has been measured
for the other games, the picture, the sound, or against real hardware, and the one other game tried (Vampire
Savior) diverges after about 2,000 frames.

So the accurate claim is **"matches jtcps2's 68000 timing on SFA2"**, not "arcade perfect".

## The path to arcade perfect

In rough order. Each step makes the next one's results mean more.

1. **Make the comparison reproducible.** ([#42](https://github.com/tmcleroy/cps2recomp/issues/42)) Put the harness patches, input scripts and comparison scripts under
   `tools/reference/`, with a `make compare-reference GAME=…` that builds the simulation, runs both sides, and
   prints the match table above. Every later step depends on this.
2. **Find the Vampire Savior divergence.** ([#43](https://github.com/tmcleroy/cps2recomp/issues/43)) Re-run it with a checked input script, find the first instruction where
   the two runs part using the bus logs and a per-line trace, and fix the rule that's missing. This is the clearest
   sign that the rules aren't complete yet.
3. **Compare every game.** ([#44](https://github.com/tmcleroy/cps2recomp/issues/44)) Run Super Puzzle Fighter II Turbo, Alien vs. Predator, and the Vampire Savior variants
   over a full credit each, and keep the results as checks that fail when the match count drops.
4. **Explain the 11-frame boot offset.** ([#45](https://github.com/tmcleroy/cps2recomp/issues/45)) The game runs in step after boot, but power-on should match too. Find what
   the port does differently during the tests: the settings memory's first read, the QSound's ready answer, or how
   the simulation's ROM download is counted.
5. **Compare inside the frame.** ([#46](https://github.com/tmcleroy/cps2recomp/issues/46)) Log the 68000's cycle count at every raster interrupt and every write to the
   video registers on both sides, and compare video and object RAM at each vertical blank. This catches timing
   that lands in the same frame but at the wrong line.
6. **Compare the picture.** ([#46](https://github.com/tmcleroy/cps2recomp/issues/46)) jtcps2's simulation writes frames (`frames/`, `test.mp4`), and the port has
   `--shot-every`. Compare them pixel for pixel, slowdown frames included, to cover the CPS-B priority, object
   latency and raster effects as drawn.
7. **The sound side.** ([#47](https://github.com/tmcleroy/cps2recomp/issues/47)) Compare the Z80 and the 68000-to-Z80 handshake timing with the QSound stub, then check the
   QSound itself against a capture from a board or a decapped-DSP emulation. jtcps2 can't do this without the
   DSP's program.
8. **The port's additions.** ([#48](https://github.com/tmcleroy/cps2recomp/issues/48)) Charge the board's waits inside readable rewrites, either by counting the original's
   bus accesses or by recording each rewrite's cost from the interpreter. Then training mode, widescreen and cheats
   keep the timing as well, and `rewritesNeeded` no longer trades accuracy away.
9. **Ground truth from a real board.** ([#49](https://github.com/tmcleroy/cps2recomp/issues/49)) Use a board, or published logic-analyser captures, for the cases a model
   can't settle: slowdown lengths in known scenes (a frame-counted video of a board, for example), the 93C46's busy
   time, and the DL-1827's wait in corner cases.
10. **The CPS-1 with QSound (Cadillacs and Dinosaurs).** ([#50](https://github.com/tmcleroy/cps2recomp/issues/50)) Its timing isn't modelled at all, and `SetBoardTiming`
    leaves it off. It needs its own rules and its own reference (jtcps15).
11. **Frame rate and latency on the player's screen.** ([#51](https://github.com/tmcleroy/cps2recomp/issues/51)) The game runs at the board's 59.637 Hz, paced by a timer.
    On a 60 Hz display that means a repeated frame about every 2.75 seconds. Use variable refresh where the display
    has it, and measure input-to-picture delay against a board's.
