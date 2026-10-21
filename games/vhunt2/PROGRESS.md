# Vampire Hunter 2: progress

Vampire Hunter 2 is Night Warriors' roster on Vampire Savior's engine, made by Capcom from Vampire Savior's program
at the same time as Vampire Savior 2 (both Japan, 1997). This port is made the way Vampire Savior 2's is: it builds
Vampire Savior's sources (`games/vampire_savior/src`) with its own `family.h` saying where their places are in this
game and what is its own. [games/vsav2/PROGRESS.md](../vsav2/PROGRESS.md) says how, and what the two games share;
this page has what is Vampire Hunter 2's.

## How it is made (2026-10-16)

- **The ROM set** `vhunt2` (Japan 970929): its CRCs and SHA-1s are MAME's (`vhunt2r1`, 970913, is not taken).
- Translated code, places and checks as Vampire Savior 2's; most places differ from Vampire Savior 2's by a few
  bytes.

## What differs from Vampire Savior (found in its code)

- **Characters** (a player's record +382): Demitri 01, Gallon 02, Victor 03, Zabel 04, Morrigan 05, Anakaris 06,
  Felicia 07, Bishamon 08, Aulbath 09, Sasquatch 0A, Lei-Lei 0D, Phobos 10, Pyron 11, Donovan 13 (no Bulleta,
  Q-Bee, Lilith or Jedah). Hidden: Dark Gallon 12 (in the tables; no code chooses him), Oboro Bishamon 19 as a
  player and 18 as the last opponent.
- **The character choice** (01F730 on) as Vampire Savior 2's: Bishamon with Start held and a button is 19
  (01F838), Phobos (the other way round from Vampire Savior 2's) and Donovan another way with Start held (+3C2),
  Shadow with Start five times on the "?" box (01F8BC), Marionette seven times (01F928). The "Hidden characters"
  feature hooks it where the colours are stored (01F7D8) and where Shadow's code is counted; it gives Dark Gallon
  for Gallon too.
- **Music** follows the characters, not the stages: a fight plays one fighter's theme (09 and the character's
  number: Demitri 0A ... Donovan 1C): in the one-player game the opponent's; in a two-player round set up by the
  modes, player 1's (which of the two the game takes in other cases is not known yet). The stage viewer's "stage's own" tune is the theme of the character a stage
  belongs to (whose row of the table of stages by character, 00ACC2 in the plain ROM, has it most): Feast of the
  Damned Demitri's, Concrete Cave Gallon's, Tower of Arrogance Pyron's, Red Thirst Anakaris's, Deserted Chateau
  Morrigan's, Abaraya Bishamon's, Vanity Paradise Lei-Lei's, War Agony Phobos's, Forever Torment Zabel's, Green
  Scream Aulbath's, the thirteenth stage Donovan's; Iron Horse, Iron Terror and Fetus of God are nobody's (the
  game's own music plays). The winners' tunes are 22 and the number; the rival 3B, Pyron EA, after Pyron is beaten
  3D then 3E or 3F, game over 07 (the sound program's table: A5B songs, 61 pieces of music).
- **Stages**, the fight's rules, the draw lists, the shadows', panel's and curtain's tiles: as Vampire Savior 2's
  (the guarding pose's test before HitLands at 016AAE; the kind-12 lists' table at 013D12; the stages' row scroll
  code at 01EA14).
- **Colours**: the tables at 376B68 (sets) and 376BE8 (effects), from `vh2j.09` over into `vh2j.10` (376C70-38FB30),
  so PalMod's one file cannot hold them: the custom colours take the text files and the colour editor only.
  Characters whose other palettes go elsewhere: EF9AEF96 (02A136).

## Checked

- `games/vhunt2/check_rewrites.sh`: every rewrite beside the game's own code (made-up play, a training session on
  each of the thirteen stages, outer rewrites left out in turn): no call differs.
- `games/vhunt2/check_widescreen.sh`: as Vampire Savior 2's.
- Seen in pictures (no window): every character in the model viewer, training with every aid, survival, the
  endless battle, ring out, attract mode, the title, a save state saved and loaded; Dark Gallon and Oboro Bishamon
  through the feature.

## Not checked by a person

Nobody has played it with a gamepad or a keyboard, listened to it, or looked at it in a window. The music's names
beyond those above are not known.
