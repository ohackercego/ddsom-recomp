# Super Gem Fighter Mini Mix: progress

The USA arcade release of 1997-09-04 (ROM set `sgemf`, the parent set), *Super Gem Fighter: Mini Mix*, called
*Pocket Fighter* in Japan, on Capcom's CPS-2 board. Started 2026-10-19. Its sources are its own (`src/`).

## The ROM set

`sgemf` was chosen because it is the set competitive play uses: FBNeo's (and so Fightcade's) set of that name, the
one the FBNeo training-mode scripts are written for. Every region's release (`pfghtj` Japan, `sgemfa` Asia,
`sgemfh` Hispanic) is the same revision, 970904; they differ in the words on the screen and the warning. All 18
files, the key included, match MAME's CRC32s (`src/mame/capcom/cps2.cpp`, `ROM_START( sgemf )`).

The graphics are two rows of sockets: four 4 MB files at 0, then four 1 MB files at 0x1000000 (as Alien vs.
Predator's). Three buttons a player (Punch, Kick, Special: MAME's `cps2_2p3b`).

## Where it stands (2026-10-19)

- Boots and plays on the toolkit's own machine: the warning, the QSound logo, the introduction, the title, the
  attract mode's stories and demonstrations, the character choice, versus fights, the whole one-player ladder to
  the ending, the staff roll, the score ranking and the attract mode again (the ladder movie, by frame 38,000).
  Seen in pictures only: nobody has played it in a window or listened to it yet.
- 68000 program: 80,465 instructions translated to C++ (4,302 entries, `make explore` twice: about 4.5 and 7
  minutes). No instruction falls back to the interpreter over runs `explore` never made: the ladder movie, two
  made-up ladders of 60,000 frames, versus, 30,000 frames of two-player play, 20,000 of attract, the test menu
  (checked 2026-10-19). The interpreter stays only as the fallback for a place no run has reached.
- Sound: driver 1.30, translated: 392 entries, 5,180 instructions (the ladder, versus and the sound test,
  `--sound-test`, seeds 1-12, recorded into the entries). `--stats` still counts one instruction of the sound
  driver with no translation in every run, as Vampire Savior's build does (the same driver): the toolkit's, not
  this game's.
- The features every game has: free play (Enhanced; the game's own), straight to the title, the board's settings
  (the Board page), cheats (no damage for each player, one-hit knockouts, infinite time), save states, run-ahead,
  arcade timing, a custom soundtrack (its music named), the in-game menu and the launcher.
- Everything the other fighting games have (2026-10-19, the same day): a training mode on the toolkit's session
  (fight/session.h: hit boxes, frame data, its meter and log, input display, combo readout, health refills, the dummy;
  and the game's own stun readout and gauge refill), the Alternative modes page on the toolkit's (fight/alternative.h:
  attract mode, the stage viewer), the fighting modes (fight/battle.h: survival, endless battle, ring out), the model
  viewer (model/), custom colours and the colour editor (fight/colours.h), widescreen and invisible walls, the hidden
  characters with no code. The session and the Alternative modes page were made toolkit modules from Vampire Savior's
  and Alpha 2's copies of them for this game, and those two games moved onto them.
- Readable rewrites: four, for widescreen (`widescreen.cpp`): the camera's place across (01F9C6) and up (01FA52),
  keeping a player inside the picture (010EDA) and both together (010F88); `check_rewrites.sh` checks every call of
  each against the game's own code over four movies (0 differed, about 15,000 to 31,000 calls each), and each is
  charged the time its original takes on average (within a few cycles).

## The game's memory and routines (found by experiment)

Variables are reached from A5 = FF8000, as in the Alpha games.

- **Where the game is** (`pocket.h`): words at FF8000, FF8004, FF8008, FF800C, each a step of the one before (2 in
  a game; under it 2 choosing, 4 the match; then the match's and the round's steps). Traced with `--watch`: the
  board's start goes 2, 4 (the warning, frames 95-276), 6, 8 (frame 459); the show then 2, 0C, 0E, 10, ..., 18 the
  introduction (frame 1210), whose step at FF800C counts up in twos from 2 to 5A; at **4C** (frame 4887) the title
  builds up from white. Then 1A and on (the parental advisory, the demonstrations). With a credit in, from that 2
  to 2 and 4 (frames 470-471): the title asking for Start. A game begins at **0079D8** (FF8000 to 2), which copies
  the difficulty to A5+13E (007A64). FF8002 counts down each frame.
- **Credits**: a byte at **FF80F6** (RAM dumps after 2, 4 and 6 coins: 1, 2, 3; a new board asks two coins a
  credit). The coins toward the next credit are at FF8143. Held at 2 with `--poke`, Start goes to the character
  choice.
- **Free play**: the game's own, one of the coin modes (A5+117): 00-08 one coin for one to nine credits, 09-10 two to
  nine coins a credit, 11 two coins to start and one to continue, **12 free play** (09 on a new board). The board's
  start reads the mode (001796) and for free play sets **A5+11C** and the coin words **A5+F2** and **A5+FA** to 0101
  (001814); the other modes take their coin words from a table (00228C). The game tests A5+11C wherever it counts
  credits. The feature (`coins.cpp`) sets what that start sets, as Super Puzzle Fighter II Turbo's does. As on the
  board, the attract mode runs its whole show in free play; Start lets a player in at any point.
- **The players**: records at FF8400 and FF8800 (0x400 long). Health a word at **+40**, full 0x90 (0082F4 at a
  round's start); below nothing a knockout (015CE6 puts FFFF in it and in the bar, +42).
- **Damage**: every hit and throw takes its damage off at **015D6E** (`sub.w d4,0x40(a1)`, the victim at A1; the
  routine 015D64 keeps the health before at A5-5780 and adds the damage to +1B0). It is the only writer of either
  player's health in a fight that `--watch` found over fights and made-up play, but for the round's start and the
  knockout. The cheats are a hook there (`cheats.cpp`): 117 hits on player 1 over a run with no damage, the health
  0x90 after each.
- **The round's clock**: a byte at **FF8189** (A5+189), seconds left, 99 at a round's start; a second off at
  **0083E6** (`subq.b #1,0x189(a5)`), with the picture's BCD copy at FF8188 taken down by `sbcd` beside it; the
  frames of the second at **FF818A**, counted down at **0083DC** (at nothing the second goes, and 008FBE starts
  them again). Infinite time gives the frame back before it is taken (`cheats.cpp`), so both copies stand still.
- **The board's settings**: read once as the board starts, at **001796** (called at 001088, returning to
  **00108E**), from the settings record at A5-7398 (FF0C68) into variables from A5+113 on. The test menu edits the
  variables themselves. The Game page's and two of the System page's, each stepped through in the test menu with
  the variable read after each step (the board's own, shown in blue, in brackets):

  | | record | A5+ | values |
  | --- | --- | --- | --- |
  | Coin mode | +00 | 117 | as above (09, two coins a credit) |
  | Continue | +01 | 118 | 0 off, 1 on (on) |
  | Monitor | +02 | 116 | (not on the Board page) |
  | Demo sound | +03 | 119 | 0 off, 1 on (on) |
  | Sound mode | +04 | 11A | (not on the Board page) |
  | Chute type | +05 | 11B | (not on the Board page) |
  | Difficulty | +10 | 120 | 0 easy, 1 normal, 2-6 hard 1-5, 7 expert (**hard 3**) |
  | Damage level | +11 | 121 | 0 low, 1 normal, 2 high, 3 maximum (normal) |
  | Timer speed | +12 | 160 | 0 slow, 1 normal, 2 fast, 3 fastest (slow) |
  | Game speed | +13 | 15B | 0-3 turbo 1-4, 4 free select (free select) |
  | Max round | +14 | 122 | 1, 2, 3: one, three, five rounds (three) |
  | Event | +15 | 125 | 0 off, 1 one match, which clears continue (0018B6) (off) |
  | Join-in | +16 | 124 | 0 off, 1 on (on) |

  The collection's launcher shares a feature between games by name and choices: Difficulty, Timer speed, Max round
  and Event mean what Street Fighter Alpha's do, Join-in what Super Puzzle Fighter II Turbo's does. Damage level has
  Puzzle Fighter's choices where it means something else, so it is this game's own (`OwnFeatures`, game.cpp); Game
  speed's choices are its own.
- **The settings memory**: its routines are at 001F22 (read), 001F86, 001FEA and 002108 (the line driver, 804040).
  The toolkit's shortcut (`runtime/board.h`, `AddSettingsMemory`) is not used yet: it only saves time at the start.
- **The sound program**: Capcom's sound driver **1.30** (`1.30 /CPS2 1996 /OCT.`, at the start of `pcf.01`), as
  Vampire Savior's; not the toolkit's 2.01b, so the Z80 runs translated. Its table of songs is at 0x9006, four bytes
  a song, the word at 0x9000 the count (0x7ED), as Vampire Savior's.

## What the modes and the training mode use (found by experiment)

- **The steps of a game**: a round is fought with FF8000 2, FF8004 4, FF8008 4, FF800C **6** (2 and 4 its
  introduction). **Who is in**: FF810D, bit 0 player 1, bit 1 player 2.
- **The characters**: a player's byte **+381** (the cursor writes it, 017586; the fight reads it, 003806 on); +3F0
  keeps the last one chosen. 0 Ryu, 1 Ken, 2 Chun-Li, 3 Sakura, 4 Morrigan, 5 Hsien-Ko, 6 Felicia, 7 Tessa, 8 Ibuki,
  9 Zangief, 10 Dan, 11 Akuma (each seen in a fight with +381 held). A player has chosen when its +05 is 2 (017598);
  the button that chose sets the colours, +39F (0175C8: Punch 0, Kick 1, Special 2, two or more at once 3).
- **The stages**: a word at **FF8180**, the stage doubled (0 to 0E), from a table of one per character at 009A3C
  (009A0A). The match is made at 009970 (called at 01778A): the hook where it returns (017790) puts in the characters
  and the stage a mode wants. Stage n's tune is n + 1 (the soundtrack log over a fight on each); player select 0B,
  versus 0F (two players) and 0C (one).
- **The sound queue**: a ring of sixteen-byte requests at A5+6230 with the end word at A5+7230 (003264 puts one in),
  Vampire Savior's layout.
- **The camera**: its record at FF8280, the place at FF8290 (copies FF802A, FF82A0, FF82A8); 0 to 100 across on every
  stage (the table at 01FBF0: ends top +40, bottom +42, left +44, right +46); the step's most at +5C and +5E; held
  while A5+533, A5+933 or its +60 is set. The players start at E8 and 198; the walls are 10 and 270.
- **A player's record**: +04-07 the steps (in a fight 2; +05 0 the player in charge, 2 reeling; +06 00 standing, 02
  crouching, 04 walking, 06 jumping, 0A attacking, 0C guarding, 16 a special move), +0B facing right, +0D its first
  colour set (player 1 10, player 2 15; five each), +10 X, +14 height (30 the floor), +18 the frame's time, +1C the
  animation frame, +20/+24 the speeds and +28/+2C their pulls (longs), +4C the hit stop, +4F in the air (FF), +105
  attacking, +130 half its width and +AD not followed by the camera (the camera's 01FAF0), +144 the combo's hits,
  +17F/+19E the stun and its most, +194/+195 the gauge's stocks (9 at most: 013176) and bar, +39E played by the game
  (the fight step goes to 0149D6 for it).
- **Boxes** (the hit test 0154CA, the boxes' test 016088, the push 016188): four words, the middle's offset across
  (turned about when facing right) and up, the whole width and height. The record names two boxes it can be hit in
  (+90 into the table at +80, +91 into +84) and its push box (+93 into +8C); the animation frame names the attack
  (+0A into the 32-byte table at +88). A frame's +08 to +0B are copied to the record's +90 to +93 when it starts
  (010E24). Drawn in the right places on the picture (seen).
- **Animations**: three tables of a long a character (072B10, 072B44, 072B78; started by 010E04, 010DF4, 010DFC with
  the number in D0), each a list of word offsets; a frame is 0x18 bytes, as in Alpha 2 (a word of time, 0FFF for
  good; a byte whose top bit says the next is a jump to the long after the frame; the picture; the boxes' numbers);
  stepped at 010EAC. A knock-down's reelings are in the third list: 12 thrown into the air, 8 falling.
- **A player's step**: 00C51C, by its state; a fight's at 00C602, which goes on by +05 at **00C642** (the frame data's
  hook, and the ring out's). A hit reaches the other at **0155F2** (A6 the attacker, A1 the other); guarded when the
  other is in its guarding pose (a guarded hit takes nothing: 015D6E with D4 0). The fly routines: 011AA6 (across),
  011AB8 (up).
- **The round's result**: FF818C, bit 0 player 2 knocked out, bit 1 player 1 (0083B8).
- **Thrown things**: a pool of 14 records of 0x100 from FF8C00 (01B30E; Ryu's fireball the last), of a player's layout:
  the frame at +1C names the attack (Ryu's fireball's 36, a box 14 wide and 12 high, 7 ahead) in the thrower's
  attack table, which the record keeps at +88. Drawn among the hit boxes in the thrown things' colour.
- **Objects**: sets 00-0F the panel and the item chest (0A); 10-19 the players (five each), what they throw and their
  faces on the panel (tiles 14460 and 14Bxx); 1A on the stage; the shadows in 1F, tiles 1D8E0-1D8FF (found by hiding
  sets one at a time in the stage viewer).
- **Colours**: the characters' sets at 2099E8, four of five palettes each (0274B0); in pcf.07, PalMod's file.
- **The scroll screens** are drawn whole when a stage begins (nothing written to them while the camera moves: watched),
  so the wide picture needs only its width.

## Checked

All on 2026-10-19, on the branch that brought the game in:

- `make GAME=pocket_fighter check` (translated code against the interpreter, frame by frame; two runs alike): the
  ladder movie (40,000 frames, `free_play,no_damage,one_hit`), the versus movie (14,000, `free_play`), made-up play
  with every cheat and straight to the title on (40,000). All ok.
- `make GAME=pocket_fighter check-blocks`: the ladder movie (346,717,987 68000 blocks, 139,442,384 of the sound
  program's), versus (113,258,704 and 48,420,936), 20,000 frames of the test menu with made-up input (198,334,937 and
  79,271,831). No difference.
- `make GAME=pocket_fighter check-release` (40,000 frames): two runs of the build with no translated code alike;
  its pictures the translated build's but for 10 frames, ending on the same.
- **With every feature off the game runs as with no sources**: the interpreter (`--cpu interp`) gives the same
  per-frame hashes with `src/` and with an empty one, 6,000 frames of made-up play.
- Straight to the title: the per-frame hashes are the same with it on and off (4,900 frames).
- Free play: Start goes to the character choice with no coin, and the screens say FREE PLAY.
- Cheats: no damage, 117 hits on player 1 over a run, the health 0x90 after each; one-hit knockouts put Morrigan
  down and the next match came; infinite time held the clock at 99.
- The board's settings: each of the 32 choices puts the right value in its variable as the board starts; with
  Expert, the one-match event and continue chosen, continue is cleared, and the test switch puts the board's own
  values back (Hard 3, off, on); the settings memory file is the same with and without choices.
- Save states: a state saved at frame 3,000 of made-up play, loaded at 3,000 into a run that had played otherwise,
  with the same input after: frames 3,001-6,000 alike. Run-ahead 1 and 2: the state the same as without it, all
  6,000 frames of the versus movie (2 frames took three times the processor, as it should).
- `make check` (the whole repository: audit of 422 files, the build with no game, the launchers).
- The game's launcher, drawn with no window: the Game page (the set's name), the Features page (Enhanced, free play
  on, the cheats, the Board page's nine settings), the Controllers page (Punch, Kick, Special).

The modes and the training mode (2026-10-19), all headless, by pictures and RAM:

- Training: the session reached from the menu, player 1 choosing for both, the aids drawn (hit boxes over the
  fighters, frame data: a jab startup 5, active 4, recovery 17; the meter; the input display; the combo and stun
  readouts); the dummy guarding (player 2's health unchanged over 21 hits with "Always", 0x90 after each); start over.
- Attract mode (all three ways), the stage viewer and its infinite stage, survival, endless battle, the ring out (the
  player knocked over the left end and falling out; the opponent knocked over the right end and the winner jumping down
  onto the next stage), the model viewer (the character alone on white, black or a stage), the colour editor, custom
  colours from a file (a fight in them), the hidden characters (Start on Ryu: Akuma).
- `make GAME=pocket_fighter check-rewrites`: every call of the four rewrites the game's own (above).
- `make GAME=pocket_fighter check-widescreen`: no unpainted columns at either end of any of the eight stages; with
  invisible walls the game's memory the same as with widescreen off every 4,000 frames of the ladder; nothing to the
  interpreter.
- `make GAME=pocket_fighter check` (ladder, versus), `check-blocks` (the ladder: 346,229,240 blocks and 139,442,309 of
  the sound program's, no difference) and `check-release` (11 frames early, the same last picture), again with all of
  it in.

**Not checked by a person yet**: nobody has played it in a window, heard its sound or music, or tried it with a
gamepad; the custom soundtrack has not been tried with files; arcade timing has not been run over the ladder; the
dummy's recording and replay and its answers have not been tried by hand.

## The movies

Scripted (the generator is not kept): `ladder.txt` (with `free_play,no_damage,one_hit`: Start at 1200, then
forward and Punch every 40 frames) takes Ryu up the ladder; `versus.txt` (with `free_play`) is both players in,
player 1 forward and Punch, player 2 Kick now and then.

## Next

- Hand-play: the training mode (the dummy's recording and replay, its answers), the modes, widescreen, the colour
  editor.
- More readable rewrites where features want them: the hit test and damage (fixed damage, a throw range readout), the
  players' state machine.
- Throws: where the game tests a throw's reach, for a throw-range box as Alpha 2's.
- The settings memory's shortcut (`AddSettingsMemory`), if its routines are Capcom's usual ones.
