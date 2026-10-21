# Alien vs. Predator: progress

## How it is made (2026-10-18)

- **The ROM set** `avspu` (USA 940520), as MAME's current sets have it (with `avspu.key`). Older collections have
  four graphics ROMs (`avp.14m`, `16m`, `18m`, `20m`) from an earlier dump, and no key file; the CRCs in `game.cfg`
  are the redumped ones. The program is the one revision; the other regions differ only in their words.
- **Translated code**: the toolkit as for any CPS-2 game. `make explore` found the code (82,889 instructions; none
  left to the interpreter but 25 a guess took for code); the sound program, Capcom's QSound driver of November 1993,
  is translated too (2,153 instructions). `make check` passes.
- **Three players**: the toolkit's CPS-2 machine reads players 3 and 4 where the board has them (their sticks and
  buttons at 804010, Start and Coin on the system port), game.cfg says `players = 3`. The test menu's input test
  shows the third player's every line.

## What is known of the game (avsp.h has the names)

- A5 is FF8000. Credits: A5+18C (byte, at most 9).
- **Settings**: the settings memory's record at FF0EF0; 003C5E copies it into variables at the board's start
  (called at 000D1A) and when the test menu is left (0B3C64): coin 1CD (0-7, 8 two to start and one to continue,
  9 free play), continue 1BE, flip 1B2, demo sound 1AD, sound 1D2, player mode 1CE (0 three players with a chute
  each, 1 three on one chute, 2 two: the factory's), difficulty 1CF (0-7 for 1-8; 3), extend 1D0 (0 none, 1-3 only
  at 100,000/400,000/1,000,000, 4-7 first and every; 1), lives 1D1 (0-3 start and continue 1/2/3/5, 4-7 start
  1/2/3/5 and continue one more; 4). From the player mode it works out a third player (1C5) and a chute each (1C6),
  which the game reads. The Board page hooks the two returns (000D20, 0B3C6A).
- **The show** (A5+00, word; not A5+02, a countdown, which "Straight to the title" read at first): 2-C as the board
  starts, then 0, then the attract's parts, one every four steps from 4 to 68 (the step between is the next part's
  setting up; the table at 004706 has their routines): 4 and 38 the notices (486A), 8 and 3C QSound's (4C14), C and
  40 the story (48DE), 10 24 44 58 the title (4938), 14 28 48 5C a character's file (4C1A) and 18 2C 4C 60 how to
  play them (4C98), 1C 30 50 64 a demonstration (4CF6), 20 34 54 68 the best scores (48E4); 6C begins it again. The
  title is drawn at its own step (A5+04) 8. 2 for good while a game is played. The character the next file shows is
  A5-6FF6 (word, 0-3), moved on by each demonstration's end (004E9A). The choice of characters is on while A5+238 is 1.
- **Score**: a player's record +E0 (long, BCD).
- **Sound**: the sound program's table of songs is at 9006 after a word of their number (0x38D); music.cpp names the
  26 pieces of music found by playing (the 03 and 1B fanfares not placed yet).
- **Players**: three records of 0x100 from FF8380: +00 in play (1), +41 health (0x65 full; FF down), +45 the full
  health, +F1 lives after this one.
- **Rounds**: the scene, A5+1E8 (word, two a scene), cleared before a game and set to 2 by 0082DC as the players
  start after round 1's story (its rts, 008352, is hooked for "Start at round"); the round shown, A5+222, from the
  table at 00CCA0 (scenes 0-2 round 1, 3-4 round 2, 5 round 3, 6-7 round 4, 8 round 5, 9-13 round 6, 14 on round 7).
- **The clock**: minutes A5+224, BCD seconds A5+225, counted down at 0085A4 each time the frame count A5+221 (from
  0x3C) runs out.
- **The camera** (viewer.cpp): a record at FF8268, its place across at +20 (and +10, +28), up and down at +24; the
  scene's limits at +30/+32 (up and down) and +34/+36 (across), from the table at 0B0550 at the scene's start and
  moved on by each scene's own code as its waves are beaten (for scene 1, 0288xx). 0AF858 runs it each frame: the
  scene's camera routine (0AF8B4, from the table at 0AF8D4 by scene) calls 0B0680, which follows the players
  (0B0C22 finds them) and from 0B06B6 draws the scroll screens' new columns as it crosses each 16 pixels; 0B0786 does
  up and down. The maps come in pages (the table at A5-6142, by the camera's +03: 4 to 32 pages across); a scene has
  only its own, and past them the picture wraps or is black. How far each scene's camera goes was found by playing
  each to its end with a bot (viewer.cpp has them).
- **Objects**: the enemies' pool, 24 records of 0xC0 from FF9280 (01769E takes one: Z set when there is none, which
  not all the scenes' code looks at); 01B922 runs the first four, 01B980 the other twenty, each by its kind (+02). An
  object's frame is at +1C (16 bytes: a word of its time, 0FFF for good, a word whose top bit says the next long is
  where it goes on, the picture's address; 016518 starts an animation from a list and its number, 016536 steps it),
  its time left +0A, +24 the frame's second word. A hit that takes the last of an enemy's health writes 04000000 to
  +04 (going down), FFFF to +40 and +42, 0C to +49 (0229xx). 0A9B48 draws each object of the list A3 is at, with what
  goes with it (0A9B62 after it), its entries going out through A1 to the object list.
- **Characters**: a player's +D2 (0 Predator Warrior, 1 Maj. Schaefer, 2 Predator Hunter, 3 Lt. Kurosawa), which
  can be put in as a game begins. Each character's animations are four lists, one after another (15E240, 162F90,
  168564, 16D122 and on): three from the tables at 01B0E0, 01B104 and 021FFA by the character, the fourth in the
  character's own code (021E10). A list's last offsets are not always animations (Kurosawa's crash the game when
  forced on him): one is if its frames are (a time, and a picture in 100000-1FFFFF).

## Not done yet

- Two pieces of music not placed (03 and 1B, played once: an ending?), and the endings not reached.
- The stage viewer shows only what a scene has loaded when it starts: round 7's first scene stops before its last
  part, and round 4's second has a few black patches where its background loads later.
- Weapon cheats (the guns' heat, the Predators' cannon): not found yet.
- Nobody has played it by hand; the sound has not been listened to.
