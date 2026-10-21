# Vampire Savior 2: progress

Capcom made Vampire Savior 2 and Vampire Hunter 2 (both Japan, 1997) from Vampire Savior's program: the same
engine, with Donovan, Phobos (Huitzil) and Pyron back and other changes. This port is made the way Vampire Hunter
2's is (`games/vhunt2`), and both build Vampire Savior's sources (`games/vampire_savior/src`) rather than having their
own: every feature Vampire Savior has, they have, from the same code.

## How it is made (2026-10-16)

- **The ROM set** `vsav2` (Japan 970913): its CRCs and SHA-1s are MAME's. The upper half of its graphics
  (`vs2.14m`-`vs2.20m`) is the same files as Vampire Hunter 2's.
- **Translated code**: the toolkit as for any game. A fresh tree's first entries were Vampire Savior's moved
  over by `tools/portmap.py` (76% of its instructions were placed at once), and `make explore` found the rest.
- **The shared sources** name Vampire Savior's places as `At(0x...)` (ROM and work RAM) and `AtA5(...)` (a variable
  by its distance from A5). This game's `family.h` lists where each is here, between the lines `places: begin` and
  `places: end`, made by `tools/places.py` from portmap's map of the two programs and checked: every rewrite runs
  beside this game's own code (`check_rewrites.sh`). A line marked "by hand" was placed or checked another way.
- **What is this game's own** is in `family.h` too: the roster, the stages, the music, the hidden characters, the
  tiles the port tells apart, and the rules of the fight that differ from Vampire Savior's (below).

## What differs from Vampire Savior (found in its code)

- **Places.** Most routines and tables moved by a few bytes to a few thousand; the stages' pieces' scripts by
  +0xE700 or so, and some code to other places altogether (portmap walks the two programs side by side from the
  anchors and the jump tables). Work RAM: FF8000-FF8FFF is as it was but for a few variables (the super move's
  backdrop flag 133 is 165, 162 is 164); FF1E36 on, FF2EAA-FF3502, FF4302 on (the stage's plan, now FF4382),
  FF5D92 on and FFF966 on moved by 0x0E to 0x80; the hit test's variables (-4BCA on from A5) are 0x52 further.
- **Characters** (a player's record +382): Bulleta 00, Demitri 01, Victor 03, Zabel 04, Morrigan 05, Anakaris 06,
  Felicia 07, Bishamon 08, Q-Bee 0C, Lei-Lei 0D, Lilith 0E, Jedah 0F, Phobos 10, Pyron 11, Donovan 13 (Gallon,
  Aulbath and Sasquatch are not here). Hidden: Oboro Bishamon as a player is 19, as the last opponent 18; Dark
  Gallon (12) is in the tables, with no Gallon to give him.
- **The character choice** (01F744 on) is its own: no "codes stopped" byte (Vampire Savior's BF(A5)), Bishamon with
  Start held and a button is 19 (01F84C), Start held as Phobos or Donovan is chosen sets their +3C2 (Donovan's the
  other way round), Shadow is Start five times on the "?" box (01F8C4), Marionette seven times (01F930, +3C3). It is
  not rewritten; the "Hidden characters" feature hooks it where it stores the colours (01F7EC) and where it counts
  Shadow's code.
- **Stages**: thirteen. The thirteenth is Donovan's (his row of the table of stages by character, 00AD2A in the plain
  ROM, gives it ten times in sixteen): Feast of the Damned's hall, empty and darker, with its own tune (15) and
  Feast of the Damned's row scroll (the table of the stages' code, 01EA28, points both at 01EA42). The camera's ends
  are Vampire Savior's on every stage (256-640; Fetus of God 160-736).
- **The fight's rules** (each a `family.h` fact the shared rewrites follow):
  - TakeDamage (017422): an attacker whose byte +1C3 is set has all of a hit's damage taken for good. That byte
    is set by a new action (0261A6: also +1C8, +143 = 14, +1C4 = 70, two gauges spent, the health timer 1/3).
  - HitLands (016AEC): kind 2D cannot be guarded unless the victim's +143 is set; kind 50 is an overhead too.
  - The hit test goes on, when it has hit, to a test of the victim's guarding pose (016AA8, which calls 02694A)
    before HitLands.
  - GuardRangeAttack (02687C): near enough to guard is 0x100 either way, not 0x80.
  - KeepIn (0273E6): a player kept in at the picture's edge also has +148 set to FF.
  - The routine that sets the health's timer after a hit (0172B0) first tests the attacker's +1C3.
- **Drawing**: the objects of kind 12 have eight draw lists (one for each depth: the table at 013D0C), one more
  than Vampire Savior's, so the stage's pieces' lists come one later (list counts at 7A49 from A5, lists at 7A02).
- **Tables the rewrites had copied from Vampire Savior's ROM** are read from the game: the first five stage
  scripts' tables of steps, the lady's poses (her colours: 1260, 1240, 1280, 12A0 where Vampire Savior's are 1120,
  1100, 1140, 1160), the stages' row scroll code.
- **Graphics**: the same pictures as Vampire Savior's mostly at other tile numbers. Shadows 30F88-30FC7 (Vampire
  Savior's F8E0-F91F, +216A8), the panel 14200-14DFF (+A00; 2,823 of its 3,072 tiles the same), the curtain between
  a match's screens 0444F-503E (its objects at the versus screen). Found by comparing the graphics as the board
  addresses them (the ROMs' address lines unshuffled).
- **Colours**: the tables at 396B94 (sets) and 396C14 (effects), all in `vs2j.10` (PalMod's file); the characters
  whose other palettes go elsewhere: EF9AEF96 (02A108).
- **Music** (the sound program's table at 9000, A70 songs; 48 pieces of music): the stages 09-15, the winners 17
  and the character's number, the rival 30, Jedah E2, after Jedah is beaten 38 then 39 or 3A, game over 07. The
  others are named by number in `songs.txt` until found.

## Checked

- `games/vsav2/check_rewrites.sh` (`games/vampire_savior/check_family.sh`): every rewrite beside the game's own code,
  over made-up play (one player and two) and a training session on each of the thirteen stages, with the outer
  rewrites left out in turn: no call differs.
- `games/vsav2/check_widescreen.sh`: the stage's plan against the map the game drew (none differ), invisible walls
  the board's in memory at nine places, no stage's ends unpainted.
- Seen in pictures (no window): the character choice, every character in the model viewer, a training session on
  the thirteenth stage with every aid, survival, the endless battle, ring out, the infinite stage viewer, the
  title, attract mode, a save state saved and loaded.

## Not checked by a person

Nobody has played it with a gamepad or a keyboard, listened to it, or looked at it in a window. The music's names
beyond those above are not known; Marionette and the other ways of Phobos and Donovan have not been looked at.
