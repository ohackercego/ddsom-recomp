# The infinite stage: two games at once on one machine

The stage viewer's **Infinite** choice (in-game menu → Alternative modes → Stage viewer → Stage) shows every stage in
turn. Each scrolls from end to end, then slides out to the left while the next slides in from the right. As it
comes in, the first stage's tune fades into the next one's. The two stages are seen and heard at the same time,
each running as the game itself runs it: its pieces moving, its row scroll, its music.

The game can't do this by itself: it holds one stage at a time. So the port runs **two games**, one for the stage
being shown and one for the stage coming next. This page says how. The code is
[`src/modes.cpp`](../src/modes.cpp) (`RunNext`); the toolkit's part is [`runtime/spare.h`](../../../runtime/spare.h).

## One machine, two games

There is only one emulated board: the port's `Machine`. The second game is a **save state** of that machine,
kept in memory (`g_other`, about 330 KB). Every frame:

1. The port's machine runs a frame as usual: the stage being shown, with the viewer's camera.
2. `RunNext` takes that frame's picture and sound out of the machine.
3. It saves the machine's whole state (`g_mine`), then loads the next stage's state (`g_other`) into it.
4. It runs one frame of the next stage. While that game is still on its way to its round, it runs a few frames,
   to get there sooner (`kOnTheWay`).
5. It takes that frame's picture and sound, saves its state back into `g_other`, and loads `g_mine` again.

The port's machine ends the frame exactly as the shown stage left it. The next stage's game is put away until the
next frame. A frame's machine state is the board's whole memory, registers and chips, so the two games never see
each other.

The toolkit offers this as **spares** (`runtime/spare.h`: `RunSpare`, `Take`, `Give`, `Promote`): a machine kept as
a save state and run a frame inside the port's frame. The endless battle and survival use spares too, to make the
next match ready while the last one ends. The infinite stage does the swapping itself (with the same `Archive`
serialization) because it also mixes the two games' pictures and sound every frame.

## Getting the next stage to a round

The next stage's game starts from a reset. `ToRound` drives it through the attract mode and character select to a
two-player round. The stage it wants is set through `Wanted().stage`: the game's own stage choice (`00AE7C`,
`training.cpp`'s `ChooseStage` hook) puts it in place of the game's pick. Until it is in its round, the next stage
isn't shown, and the shown stage waits at its right end. That takes about 320 frames, a few at a time per port
frame, so it is ready long before the shown stage has scrolled to its end. While it runs, only the stage is drawn:
the fighters' and the panel's objects are hidden (`video.hideObject = NotTheStage`).

## The picture

The picture is the two frames side by side. The shown stage is moved left by the slide (how far the next stage has
come in, in pixels) and the next stage's frame fills the columns on the right. In widescreen both games draw at the
port's width (`WideForSpare` and `WideForPort`), so their columns line up.

## The sound

Each game makes its own sound: its Z80 runs its sound program and its QSound chip plays its samples. The two
streams are mixed sample by sample, each weighted by how much of its stage is in view:
`next = slide / width`, `shown = 1 − next`. That makes one tune fade into the other as the next stage slides in,
with no silence and no jump. The two games' frames don't make exactly the same number of samples, so the next
stage's sound waits in a short queue (`g_otherSound`) and is taken a frame's worth at a time.

## When the next stage has come all the way in

The two games trade places: the next stage's game becomes the port's own (the shown one is dropped). Whatever it
had made that wasn't heard yet is played first (`g_carry`). Then the stage after it is started as the new spare.
That repeats through every stage and around again.

## Cost

Running two games costs about twice the CPU, plus saving and loading a 330 KB state twice a frame. On the Mac it
runs at about 1,000 frames a second headless with both games running, far above the board's 60. Run-ahead is off in
the modes, because a spare's game is not in the port's save states.

## Checked

Over 4,200 frames at the fast speed: five stages in order, no silence, and no jump in the sound larger than a plain
viewer run has (`PROGRESS.md`).
