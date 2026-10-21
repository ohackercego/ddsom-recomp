# Super Puzzle Fighter II Turbo: progress

The European arcade release of 1996-05-29 (ROM set `spf2t`), on Capcom's CPS-2 board. `spf2t` is
the parent set. The other regions (`spf2tu` USA 960620, `spf2xj` Japan 960531, `spf2ta`,
`spf2th`) differ in one program ROM and the key; graphics and sound are the same files.

## Where it stands (2026-10-12)

- The second game on cps2recomp, which now has a repository of its own (`tmcleroy/cps2recomp`); this
  repository has a copy (`cps2recomp/`, `make check-toolkit`). The toolkit needed one change for it: a graphics list in `game.cfg` can
  start at an offset (`@800000`), because this board has only its last row of graphics sockets
  filled and the game's tile numbers count from the first.
- The game boots and plays on cps2recomp's own machine: QSound and Capcom logos, attract mode
  (character introductions, demonstration match, the how-to-play dojo), coin, player select with
  the difficulty choice, two-player matches over several rounds. Seen in pictures from headless
  runs of made-up play; nobody has played it by hand yet, and the sound has not been listened to.
- 68000 program: 31,752 instructions (122 KB) translated to C++ in 7,549 blocks and 2,271
  routines, from 1,461 entries found by made-up play (`make explore`) and 430 guessed from the
  program. Sound driver: 4,653 Z80 instructions. In a 24,000-frame run of made-up play no 68000
  instruction falls back to the interpreter (the translator leaves 55 instructions to it: status
  register and exception instructions, as in Vampire Savior).
- Checks, over 12,000 frames of made-up play (`ARGS="--auto 7:400"`): `make check` (translated
  code equals the interpreter run block by block; two runs alike) and `make check-blocks`
  (108 million 68000 blocks and 54 million of the sound driver's beside the interpreter).
- About 2,300 frames a second headless.
- The port's own parts work as for any cps2recomp game: in-game menu, save states, pause, frame
  step, speed, launcher, control bindings. None of them tried with this game yet.

## How native it is (2026-10-12)

- No emulator is linked. The 68000 program runs as translated C++; of the 36 instructions (of 32,210) the
  translator still marks for the interpreter, 34 are data a guess took for code (never run) and 2 are
  a `divs #0` and a `chk` in what also looks like data. Traps, returns from exceptions and writes to
  the status register are translated (toolkit).
- The one-player game to its end (2026-10-12): with the cheat `quick_win` (player 1 sends forty counter
  gems with every pair and its field is emptied before it can fill) made-up play beats all nine opponents,
  the last of them Akuma, after which the game shows a short scene and GAME OVER; a game on EASY ends
  after three. `make explore` does twelve such runs (game.cfg `explore_ladder_features`): they found 63
  more entries (32,804 instructions translated now), and eighteen further runs found none. The stage
  counted from 0 is the word at FF8112. `movies/ladder.txt` is one such run (five times up the ladder,
  66,000 frames; features `free_play,quick_win`); the rewrites are checked over it too, which puts every
  computer opponent under the check. Not reached: the hidden challenger (the rule is under "The rules,
  as found"; made-up play with `quick_win` does not earn it), and losing on purpose at each stage.
- The new modes swept with `--stats` (2026-10-15, headless): attract mode, each of its four shows for 30,000
  frames; CPU battle, each strength for 80,000 frames and with each hidden character for 60,000;
  the colour editor on each of the twelve; straight to the title; a training session of made-up play; eight
  runs of made-up play with the hidden characters chosen by kicks (six one-player ladders of 120,000 frames
  with `quick_win`, two two-player runs). The attract mode's second demonstration match reached two entries
  nothing had found (002580, 002586: added to the entries, translated); after that no 68000 instruction falls
  back to the interpreter in any of them. The sound side reports 1 to 4 Z80 instructions interpreted a run,
  growing with the board's resets and with no new entry recorded: the Z80's start, not the driver's work.
- The interpreter is still in the binary: it runs any code that made-up play has not found yet (none
  in the checks' runs), and the checks use it.
- The picture goes to the window through SDL's GPU API (Metal on this Mac), `--renderer sdl` for SDL's
  renderer. Checked by drawing off screen and reading back (`--present-test`) and by runs with the
  window hidden; nobody has compared the two in a real window yet.
- The settings memory is read and written directly (toolkit `runtime/board.h`; this game's two
  routines are at 01E62C and 01E6F4): no talking to the EEPROM bit by bit. The routines' time is kept
  exactly, by a wait during which interrupts are taken as before. The file is the same as it was.
- Sound: the game's sound program (Capcom's QSound driver, version 2.01b) runs as ordinary code
  (toolkit `sound/driver.cpp`), not on the Z80: it takes the game's commands from the memory the two
  processors shared and works the toolkit's QSound. `--sound z80` runs the program itself (translated),
  `--sound check` both. Checked by `make check-sound`: every write to the chip the same as the Z80's
  (11.9 million over eight sound tests and a run of play; the tests play every one of the ROM's 907
  songs, and made-up single samples and ADPCM samples, which this game has none of), and the game's
  side of a run the same frame by frame. The sound is not the same sample for sample: the driver is
  never behind as the Z80 sometimes is, and its writes are paced as the Z80's by measured averages
  (in a 200-second run every second is within 3% of the Z80's in loudness). Nobody has listened to it.
- Still at the level of the board: the game writes CPS-2 video registers and the toolkit's own
  renderer makes the picture from them; the QSound chip is the toolkit's model of it;
  the controls come to the game as the bytes of the board's ports (a few shifts of what the player
  holds: there is no part of the board in between to leave out).

## Features (`src/`)

- **Cheats**: player 1 or 2 cannot attack; `quick_win` (player 1 wins quickly); `challenger` (the hidden
  challenger comes at the seventh stage, as for a player who has earned it).
- **Free play** (`coins.cpp`; on in the launcher's Enhanced preset): the game's own. The operator's coin
  setting 0x12 makes the game set a flag (FF809C) when it starts; the feature sets what the game sets and
  leaves the settings memory alone. The screen says FREE PLAY, Start goes to the game's choice of mode
  (against the computer, or two players).
- **Training mode** (`training.cpp`; in the menu: Alternative modes, Training mode, Start; the Dummy page is under it): resets the board and works Start and
  the choice of the two-player mode with the game running unseen, to the choice of characters. Player
  1's controls choose for player 2 once player 1 has chosen. In the match player 2 **stands by**: its
  turns are left out (in `BothPlayersTurns`), so its side does nothing. The round starts over when
  player 1's field fills up, and on Backspace / left shoulder ("Start over": the machine as it was when
  the round's play began).
  - **Attack readout**: over a player's field after each break: gems and chain, score, counter gems
    made, how many of the opponent's were cancelled and what is sent, or the turn's total. From the
    numbers `ScoreBreak` works with. Also works outside training.
  - **Input display** (`input_display`): player 1's controls as the turn reads them, newest first, with
    the frames each was held; numpad notation for the stick, A and B for the two turn buttons. Over
    player 2's field.
  - **Dummy attacks**: 3 to 48 counter gems sent at the start of every fourth turn, to practise
    cancelling. They fall in the pattern of player 2's character, as in a real match.
  - Dummy "Plays": player 2 with its own controls.
  - **Crash gems** in every pair, or none; **A diamond next**; **Pairs fall** as at the round's start
    throughout, or slowly; **Empty my field**. The first three are lines in `MakeNextPair` and
    `SetFallSpeed`.
- **Up drops the pair** (`hard_drop`, Game page; on in Enhanced; `steer.cpp`, HardDrop in SteerPair): a
  person's push of the stick up takes the pair down to what is under it and gives it a rest in its last
  frame, so the game's own landing follows in the same frame. Checked headless with a movie that only
  presses up (the pairs stack in the entry column); not played by hand.
  The computer uses it too (2026-10-15): where ComputerSteers would push its pair down (after its wait,
  from the second stage on, or always on HARD) it pushes up instead, and HardDrop lets go of that push as
  it drops the pair (else the next pair, coming in with the stick still up, drops before it is steered).
  The menu's CPU battle is a two-player match, so always at the first stage: there the computer keeps the
  game's rule and never pushes down (nor drops) unless the course is HARD.
- **Landing guide** (Game page): outlines where the two gems of the pair being steered would come to
  rest. Drawn from the pair's place and the field (`PairPlace` in `steer.cpp`); the field's place on
  the screen (16 pixels a cell, fields at x 16 and 272, floor at y 208) was set from headless pictures.
- **Cheats**: either player's breaks make no counter gems (one line in `AddAttack`).
- **Straight to the title** (`skip_intro`, Game page; `title.cpp`): from the board's own start (Machine::frame 0)
  the game runs unseen until the introduction's title (FF8000 0x14 with FF800C 0x18, about frame 2050), a
  step past it, or a Start.
- **Hidden characters** (`hidden_characters`, Game page, player 1, 2 or both; in the ChooseCharacter rewrite):
  a kick chooses one outright, LK Devilot (8), MK Akuma (9), HK Dan (10), as the game's leftover at 0076EC
  does with FF8085 set. Seen in pictures of the match screen for each; not played by hand.
- **Board settings** (`board.cpp`, the launcher's and the menu's Board page): the operator's settings of the test
  menu's Configuration (difficulty, damage, game speed, game mode, rounds against the computer and between two
  players, join-in, handicap, continue, demo sound), each "As the board is set" unless chosen. A choice goes into
  the variable the game reads, at 001076 (after the board's start has read the settings memory, 01DE52); the
  settings memory is not changed. The board's test switch puts the board's own values back for the test menu
  (it shows and saves the variables), and the choices go back in when a game next begins (00A046, before it
  copies difficulty to FF80BE, speed and continue), since leaving the test menu does not read the settings
  again. Checked headless: the variables with and without choices, the test menu showing the board's, and
  Hardest back in FF80BE in the game after it.
- **Alternative modes** (`modes.cpp`): attract mode (the whole loop, or only the introduction, the demonstration
  matches or the dojo), CPU battle (both records' +9E set and +A8, the level, from the
  Strength choice, every frame of a match; the board started again unseen after each match), and the colour
  editor's viewer (below). Checked headless with pictures; nobody has watched them in a window.
- **Custom colours and the colour editor** (`colours.cpp`, toolkit `fight/colours.h`): see "Colours" under Game
  facts. The viewer is a two-player match with both players' turns left out (`ViewerStandsBy` in turn.cpp) and
  player 1's field filled with a gem, a crash gem and a counter gem of each colour and the diamond. A colour
  file was checked to recolour a match (the red gems and Ryu); the editor's panel was seen in pictures only.
  The editor's Animation row makes player 1's character attack over and over: its record at FF8B00 (the
  player's +32 points at it) gets the attack's size at +D0 (0 to 10 in twos, as WarnOfAttack gives the
  player's) and its state +4 = 020A0000 (attacking); +5 is 0A while it attacks, and 40 frames after it stops
  the attack is started again. Background: Black and White fill the scroll layers' palettes (90C400-90CFFF)
  before and after each frame; the objects (gems, characters, attacks) keep theirs. Size has one choice.

## Readable code (`src/`, checked by `make check-rewrites`)

| Routine | Address | File |
| --- | --- | --- |
| How well the computer plays: StartSkill, HoldToStage, LevelForMatch, GiveLevel, SkillAfterMatch, EaseSkill, SkillForChain, SkillAfterContinue, AddSkill, ClampSkill, LevelFromSkill | 01565A ... 0157F0 | `difficulty.cpp` |
| The computer player's plans: PlanPair (each character has one of three ways), MatcherLooks, MatcherPlans, StackerPlans, SeekerPlans, KeepFromTheTop, KeepFromHighSide | 013620, 01367E, 013B3C, 013846, 015984, 013AA8, 013AFC | `computer.cpp` |
| The computer player's hands and pace: ComputerSteers (its planned presses played on the controls), SetGap, SetThinkTime, DrawSlips | 011F34, 012082, 012424, 012314 | `computer.cpp` |
| What comes before a match: BeforeMatch, Choose, BeforeLeave; the screens ChooseMode, ChooseLevel, ChooseCharacter (one player), ChooseCharacters (two, with handicap levels), ChooseOnJoining, ShowOpponentScreen (the ladder); DrawBackdrop; and what the screens after a match use of them (ShowCharacters, ShowLadderOnOtherPage, SpinLadder, SpinStage, ShowAfterLadder, ShowStageOpponentOnOtherPage, BringBackAfterMatch) | 0068B4, 0069E0, 007452; 0069F8, 006CD2, 006FCC, 00718A, 007314, 007AC6; 008470; 0077BE, 007C7E, 007D0C, 007D94, 007DF2, 007F46, 0082EA | `select.cpp` |
| MatchSetUp, ClearForMatch: a match made ready | 00A262, 00A378 | `match.cpp` |
| After the game: ContinueScreen, GameOver (the step of each that goes back to the title waits: the game's own code) | 00B636, 00B846 | `screens.cpp` |
| What the whole program calls: a frame of every task (RunTasks and its likes: 016A3A, 016B78, 016BCA), the tasks' three pools (NewTask, EndTask and their likes, ClearThirdPool), Random, ToDecimal, CopyColours, DimColours, DimAllColours, StepAnimation, StartAnimation, SetAct, StartFade, PutTall, ShowStageNumber, HideCharacters, ShowCharactersAgain | 02DA46 ... 02DADC, 02D8C8, 02DB02, 02D0AE, 020F2C, 02107C, 0210AC, 02D29C, 02D2B4, 014390, 006634, 00CC14, 00BB8C, 00BBC8, 00BBE8 | `common.cpp` |
| The screens around a match: MatchIntro (the characters face each other before the first round), AfterMatch (the winner speaks; a person's score, best chain, biggest break and time), StageInterlude (a scene after the fourth stage), LadderAfterMatch (the next opponent picked out) | 00A3A2, 00ABCC, 00ADD2, 00AF50 | `screens.cpp` |
| SetOpponentOrder, SetOpponent, NextOpponent: the ladder's opponents (eight orders of the eight characters; those beaten are not met again) | 00BAB2, 00BA44, 00BB16 | `match.cpp` |
| MatchOver (who goes on, whose game is over), ChooseWinQuote (the winner's words: nineteen a character), LadderWon (on to the ending) | 00AAF6, 00BC02, 00B576 | `match.cpp` |
| KeepGameBests, KeepScores: what is carried from match to match | 00B2FC, 00B32A | `match.cpp` |
| MeetChallenger, MeetLastOpponent: the opponents that come unasked | 00BD38, 00BDAC | `round.cpp` |
| RoundFrame: a frame of a round: its step, then the turns and the picture's work | 00A660 | `round.cpp` |
| RoundBegin and its four steps (BeginSetUp, BeginShow, BeginWaitForCharacters, BeginPlay) | 00A6CA, 00A6DE, 00A78C, 00A7CE, 00A7EA | `round.cpp` |
| RoundPlay: play until someone has won; the round and the match given to the winner; CountPlayTime | 00A810, 00B284 | `round.cpp` |
| RoundWon and its four steps (WonShow, WonSettle, WonSpeak, WonEnd); KeepBests | 00A908, 00A928, 00A95C, 00A972, 00A996, 00A8DA | `round.cpp` |
| RoundAfter and its four steps (AfterClear, AfterWait, AfterBonuses, AfterEnd: the next round, or the match is over) | 00A9E0, 00A9F4, 00AA2E, 00AA40, 00AA60 | `round.cpp` |
| PlayTurns, RoundBonus: the winner's bonuses (time, chain, big gems, stage) | 00BEFA, 00C7DE | `round.cpp` |
| BothPlayersTurns: a frame of a round, each player's turn; StartPlayer, AfterTurns | 00BF48, 00BF88, 00C05E | `turn.cpp` |
| PlayerTurn: the turn's step, banner, score, the round's seconds; ShowScore, SetWarningSize | 00BFAE, 00C106, 00C13E | `turn.cpp` |
| The turn's steps, all of them (StepNewPair, StepFall, StepCount, StepMakeBig and its two more passes, StepWait, StepFindBreaks, StepBigBonus, ShowBreak, StepJoinBig and its two more passes, StepNoteAttack, StepCountAgain, StepAttacked, StepEnd, StepLost) | 00C176 ... 00C60A, 00F72A | `turn.cpp` |
| WarnOfAttack (how the attacker's character acts; whether the attack ends the round), CheckLost | 00C50C, 00C79E | `turn.cpp` |
| StartBreaks, DiamondBreaks, DropLooseGems | 00D3D6, 00D26A, 00D058 | `field.cpp` |
| MakePictureCodes, ShapeCode: the codes the big gems' pictures are drawn from, and their corners | 00D9AE, 00DAFA | `big.cpp` |
| The words over a field: ShowBanner, ClearBanner, ClearOpponentBanner, ShowChainBanner, ShowTechBonus, ShowAllClear, ShowFirstAttack | 00C8F8, 00C948, 00C9BE, 00C89E, 00C8B8, 00C8CC, 00C8E0 | `banner.cpp` |
| TidyField | 00ECAA | `field.cpp` |
| CountField: gems in the field, danger, the all-clear reward | 00D76E | `field.cpp` |
| CountDownCounterGems, TurnCounterGems | 00D8BC, 00D824 | `field.cpp` |
| MarkBreaks, BreakCountersBeside: what a crash gem breaks | 00D462, 00D6D6 | `field.cpp` |
| BigGemBonus | 00F8E0 | `attack.cpp` |
| ScoreBreak: takes the broken gems away, score, attack, cancelling | 00FAB0 | `attack.cpp` |
| ManyGemsBonus, ChainBonus, ColourBonus | 00FF14, 00FEE4, 00FEA8 | `attack.cpp` |
| AddAttack: the attack a score makes (time, damage setting, handicap) | 00FD64 | `attack.cpp` |
| CounterStrength: what an attack is worth against a bigger one | 00FCC6 | `attack.cpp` |
| NextPair, MakeNextPair: the pair that comes next (tables by the round's colour order, the diamond) | 011A1A, 011A26 | `pair.cpp` |
| PlayerRandom, NextDiamond, CountColours | 0122EC, 011B3A, 011AB8 | `pair.cpp` |
| SetFallSpeed: how fast pairs fall, by pairs landed, level and the round's time | 01182A | `pair.cpp` |
| FallGems: a frame of falling, big gems as wholes; FindDiamond | 00CEC4, 00D01C | `fall.cpp` |
| DropCounterGems: the opponent's attack into the row above the field (a row of six, or singles in the turn's column order); the first attack's 1000 points | 00CCC6 | `drop.cpp` |
| SteerPair: a frame of the falling pair: falling, resting (16 frames, eight rests at most), landing | 01166C | `steer.cpp` |
| TurnLeft, TurnRight, MakeRoomToTurn (the push away from a wall or a gem; stuck after being pushed up three times), SwapPair | 011D06, 011DB8, 011E6A, 011C80 | `steer.cpp` |
| PushDown, LandPair (the diamond), UnderPair, PairCell | 011BBE, 011974, 011948, 011916 | `steer.cpp` |
| TurnPicture: where the turning gem is drawn for the eight frames after a turn (two tables) | 0123BE | `steer.cpp` |
| MovePair, MoveRight, MoveLeft: the stick to one side (at once, then repeating after 12 frames) | 0120F8, 012108, 0121FA | `steer.cpp` |
| JoinUpwards, JoinUpwardsFromRight, JoinSideways: two big gems of one colour that make a rectangle become one | 00F060, 00EEB0, 00ECEC | `big.cpp` |
| MakeBigGemsRightwards, MakeBigGemsLeftwardsStrictly, MakeBigGemsLeftwards: the biggest rectangle of one colour from a cell becomes a big gem (three passes a turn, one procedure) | 00DCD6, 00E0EE, 00E8BC | `big.cpp` |
| CounterGemFor: the colour and count of a falling counter gem, from the sender's drop pattern | 00FF3C | `drop.cpp` |

Every call of each matches the game's own code over the movies (those made by hand for the screens before a
match among them: `select_*.txt`) and over eight long runs of made-up
play (`check_cover.sh`; registers, all of work RAM), layer by layer (`layers.sh`: a rewrite is only checked
when the game's own code calls it, so each run leaves out the layers outside the one it checks). Whole runs
with and without the rewrites give the same pictures (at `--cpu-speed 4`); a frame or two may differ in
passing where the game changes the screen just as the picture is taken, since a rewrite's time is an
average (the check allows three such frames and the same end). The banners write to video memory, which
the check of a call does not look at: the whole runs' pictures are their check.

Time: a rewrite is charged the average number of 68000 cycles the check measured for the code it
replaces (the last argument of `AddRewrite`), not the exact number for each call. The game is short of
time in some frames (it slows down, as the board does), and with the charge only an average the port
can come to slow down in a slightly different frame than the board would. `--no-rewrites` runs the
game's own code, exact.

Not reached by the movies, so not checked: (big gems are covered by `check_cover.sh`, half of whose runs deal only two colours:
`SPF2_TEST_COLOURS=2`, a test aid in `pair.cpp`); handicap mode, the two hidden characters' weaker attacks,
attacks past 30 gems in `CounterStrength`, attacks capped at 250, rounds longer than 180 seconds.

## The rules, as found

- Score of a break: 100 a gem (crash gems too), 10 a counter gem; 10 for each gem past the second;
  big gems: each cell 100 to 350 more by how many cells broke (table at 00FFF8); 200 a colour when two
  or more colours break at once; chains 200, 400, 1000, then 600 more each, to 4000 (table at 0100C8).
- Attack: a tenth of the score added for each step of the round's time (steps at 75, 105, 135 ... 405
  seconds, as the game counts them); times the operator's damage setting (0.8, 1.0, 1.3, 1.5); a
  counter gem for every 100. Hidden characters played by a person: three quarters.
- Cancelling: an attack bigger than what is waiting for the player cancels it all and the rest is
  sent. A smaller or equal one takes off only half its size (up to 12 gems; bigger ones (n/8+1) times
  5 to 8), and nothing is sent. After 180 seconds nothing is cancelled.
- Drop patterns: a table for each character at 010188 (twelve rows of six colours; the twelfth row
  repeats). Counter gems arrive with a count of 5 for every character, or 3 when the record's byte
  +27F is set.
- Pairs: drawn with the player's own random numbers from a table of 64 for the round's colour order
  (one of four); the second gem from the table's first 32. A player given more than twelve of a colour
  is owed a crash gem. Diamonds come at pair numbers from a table (the first at the 25th pair).
- Fall speed: frames a step from a table by stage (a stage every 20 pairs) and level, plus 3, 2, 1 or 0
  by the level chosen; after two minutes that addition goes, after four there is no wait at all.
- All clear: 600 points more each time and 6 more counter gems each time.
- The pair: it rests 16 frames on whatever is under it and can be steered meanwhile; after eight
  rests in a turn it lands at once. Turning into a wall or a gem pushes the pair the other way; pushed
  up off the stack more than twice it is stuck and lands. With gems on both sides a turn button swaps
  the pair's two gems. Pushing down scores 10 if the stick is straight down when the pair lands.
- A diamond as the pair's second gem halves the attack; as either gem it makes the counter gems the
  opponent receives arrive with a count of 3.
- The first attack of a round earns 1000 points.
- Big gems: three passes a turn look from every gem upwards and sideways for the biggest rectangle of
  its colour (11 rows up, 6 columns across at most). A rectangle may take in one big gem already
  there, only if it starts at that gem's corner and covers it whole. The second pass (00E0EE) marks
  the cell it starts from as a top right corner though it is the bottom right one, and has a branch
  that lands in the first pass's code (for a cell with a corner mark and no gem number, which the
  game never makes): the rewrite does not follow that branch.
- A quirk: the count of gems the game notes for a turn's first break is wrong whenever the routine
  that shows the chain number has used the register it is kept in.
- A turn, step by step (`turn.cpp`, `kStep`): the pair is steered; what has nothing under it falls; the
  field is counted and counter gems count down (those at nothing flash for 32 frames and become gems);
  gems fall; big gems are made (three passes); gems marked the frame before start to break, the diamond
  marks its colour, marks spread; the big gems' bonus; gems fall; the break is shown for 40 frames and
  then scored, and the turn goes back to falling (that is a chain); with nothing breaking, big gems join
  (three passes) and are made once more; the attack waiting is noted; the field is counted; the
  opponent's counter gems come in and fall a row a frame; the turn ends.
- The round is lost when a turn ends with a gem in the fourth column's top row.
- A diamond that lands on the floor breaks nothing and earns 10000 ("TECH BONUS").
- An attack of 30 or more is taken to be the end of the player when its rows and the height of the
  fourth column together reach the top: the attacker's character then does its biggest act, and the
  round ends with a sound of its own.
- The winner's bonuses: 100 for every second under 180, 500 for each break of the longest chain, 100 for
  each cell of the most big-gem cells broken at once, and 2000 plus 1000 a stage.
- A quirk: when a round is over the game means to clear each player's pair (the mark that the first
  was made, its row, its two gems), but it gives the places as distances from the second player's
  record where it means the places themselves, and clears eight places some way below the records
  (FF0D28, FF1128, FF0A14, FF0E14, FF0B52, FF0F52, FF0B54, FF0F54) instead.
- The hidden challenger (the game's test at 00BD38, called before each match; seen with the cheat
  `challenger`, which says a person has done the four things): against the computer, not with handicaps, at stage 6, a person who has not continued
  (FF80AD) and has, in this game, won a round in under 60 seconds (+B3), made a chain of 4 or more
  (+AE), broken 20 or more cells of big gems at once (+B0) and ended a round with an attack too big
  for the field (+B2) is met by character 8 in place of the stage's opponent (FF813E becomes 4, and
  what comes between the rounds, 00BDDA, brings it in). It is not a matter of the difficulty setting.
- The ladder: the game has one of eight orders of the eight characters (table at 00BC78, sixteen bytes
  an order; FF8122 says which), and the stage's opponent is the first in it not yet beaten (FF8110 has a
  bit a character). The ninth is character 10, which becomes character 9 as it is shown coming in.
- What comes between two rounds (00BDDA: the challenger or the last opponent coming in) waits for the
  frame's interrupt part of the way through; the rewrites it is reached through stand aside while it is
  on (`RewriteWhen` in `round.cpp`).
- Before a match (`select.cpp`; FF8008 says where: set up, choosing, leaving; FF800C the screen, FF8010
  its step). The choice of mode (cursor FF0C8B), of level (FF0C8C: easy, normal, hard; it sets the
  ladder's last stage from the table at 006FBC: 2, 8, 8), of characters, then the ladder with the next
  opponent. Each choice has 19 seconds (9 for a handicap level) and takes what the cursor is on when
  they run out. The screens' moving parts are tasks (02DA46 starts one), not rewritten; they count
  FF0C8A up as they finish. The set-up (0068D8) waits for the frame's interrupt, so it stays the game's
  own code (`RewriteWhen`), but for the backdrop it draws.
- Choosing a character: eight in two rows of four (table at 007906: the character at each place); the
  stick does not go from the end of one row to the other. Either button chooses; the second gives the
  character's other colours, and a second person choosing the same character gets whichever the first
  has not. Two players then each choose a handicap level, 0 to 4, starting at 2 (+A9 of the record).
- Choosing a hidden character (tried: `select_codes.txt`, `select_timed.txt`): while Start is held with
  the cursor on character 0 (player 1) or character 6 (player 2: the bottom corners, where left and down,
  or right and down, do not move the cursor), the game follows the stick against three sequences (table
  at 0086C2), starting again at any wrong move, and undoing a finished one at any move at all: left down
  left down left down left (player 2: right for left), and thirteen downs (twice over: two of the three
  are the same). Choosing with a button then gives character 9 after the first sequence, character 10
  after the thirteen downs, and character 8 after the thirteen downs if the choice is made while the
  timer shows 10. (FF8085 set lets the three kick buttons, which this game does not otherwise use,
  choose characters 8, 9 and 10 outright (0076EC), but the game only ever clears it, at 00154A: left
  over from its making. That path of the rewrite has not been run.)
- A match's screens are steps of FF8008 (table at 00A256): set up, the intro, the round, the winner, the
  interlude, the ladder. After the fourth stage, once a game and not on the short ladder, one of six
  scenes is shown, chosen by chance (table at 00AE7A); a button ends it after its first half second.
  The winner's screen shows a person's totals for 3.5 seconds or until a button; when the computer has
  won, or the operator's setting ends the game with the match, the game step moves on from there.
- The ladder's screen: against the first opponents the stage's place runs through the eight characters
  for two seconds and stops at the opponent (it is already decided: `SetOpponent`); a button cuts it
  short. At the last stage (but on the easy level) the whole ladder runs.
- How well the computer plays (`difficulty.cpp`): one number, the skill (FF0CC6, 0 to 255); a computer
  player's level is an eighth of it. It starts from the operator's difficulty (levels 0, 3, 5, 8, 12,
  16, 20, 24). Before a match it is held to the stage's level for the setting (table at 015844): not above
  it at the four easier settings (and not below at stages 2, 5, 7, 8), not below it at the harder four.
  After a match it goes up 16 (less after a long match, more after a very long one: 8, 4, 16, 32 by
  256-frame steps from 60) and comes down by the setting (24 at the easiest to 0) and a little by
  chance. A person's chain adds 1 for three, 2, 4, and 8 for six or more. A continue takes off 16 to 64
  by how many have been used, and the next match is not held to its stage.
- The computer's plans (`computer.cpp`): each character has one of three ways. The matcher (characters
  1, 3, 4, 7, 10, 11) stands the pair upright in every column, either way up, and scores what the two
  gems under the landing cell are (their colours; more for crash gems); failing that it looks for a
  column where a gem lands on or beside its colour, and failing that takes a place by chance. The
  stacker (character 6) does not look at the field: pairs with the one colour it keeps go to the left,
  the rest to the right. The seeker (characters 0, 2, 5, 8, 9; 015984) scores every way of putting the
  pair down: upright in each column either way up (the column's own score, four for each row down,
  weights for what each of the landing cell's seven neighbours is to the gem, sixteen for a gem of the
  other's colour beside where the other comes), and lying across two columns (half the two upright
  scores, 32 more when both are 64 or over); the best is taken, of equals the last looked at. The
  weights are tables by character and kind of gem, with a second, sharper set used by chance (more
  often for some characters and as the two fields fill: "crowding", 0 to 24). Against the first six
  opponents, while crowding is 0, the plan is thrown away and the place goes by the frame count. The
  matcher and the stacker keep away from the top and from a high side. (A quirk: the matcher means to take a place by chance one time in
  sixteen at the first stage, but asks for the stage at the wrong place, FF0412 or FF0812.) Others in the
  seeker: its sharper way's bonus for landing on a pile that rests on the other gem's colour never
  comes about (it tests the mark of a breaking cell, bit 4, where a crash gem, bit 3, seems meant);
  its look down a column leaves out the bottom row, so an empty column counts as holding one gem; and
  with FF8085 set (nothing sets it) it shows its crowding, sharpness and level as numbers by its field.
- The computer player: where a pair is to go comes from its planning as counts of presses, which it
  plays one of each kind at a time with a gap between (table by course and stage; longer as the first
  stage's round goes on, shortest when it is in danger), not before the pair is three rows in. Then it
  waits (a table by its level, the course and the round's progress, less a little by chance) and pushes
  the pair down every other frame; by chance it first pushes on for some frames and then lets the pair
  fall by itself for some. At the first stage it never pushes down except on HARD. (The course, FF0C8C:
  the player's choice of EASY, NORMAL or HARD when the game starts, 0 to 2; the choice is made at 006DC6.)
- The banners are text: a character is two tiles of the text layer, one over the other (digits A0+n,
  letters C1 on for A to O and E0 on for P to Z, the lower halves 10 further).

## Game facts

- A5 = FF8000. Step words FF8000/8004/8008/800C/8010 (`spf2.h`): match 4 / round 4 / played 2.
- Players' records FF8300 and FF8700 (A6), 0x400 each: score +AA, opponent +160, field +164, step
  +17E, counter gems to send +19E, round's seconds +340, computer-played +9E, character +82.
- Field: rows of 8 words (wall, 6 cells, wall), 14 rows, FFAB00 (player 1) and FFAD00; a second field
  0x400 further for big gems. Cell: low byte kind (1 blue, 2 yellow, 3 green, 4 red, 7 counter, 8+colour
  crash) and marks (10 breaking, 20 marked, 40 seen, 80 big gem); high byte of a counter gem: colour and
  count.
- Controls: 58(A5) and 5C(A5), read about 0013EC in the frame interrupt (not a routine of its own).
- At the choice of characters FF808D has a bit for each player choosing (3 in the two-player mode); the
  players-in byte FF80A8 stays 1 there. (Training's way in waited for FF80A8 to be 3 and so ran to its limit
  of 4000 frames, past the choice's timer: fixed to wait for FF808D.)
- The show's loop, by FF8000 (two steps a part: a set-up, then the part): 2-8 the board's start (version,
  warning), 0, then 2-12 the notices (QSound, Capcom), 14 the introduction (FF800C: the logo, a character's
  introduction, a demonstration match at 10, the title at 18), 16/18 the how-to-play dojo (about 9,400 frames),
  1A/1C the score ranking, 1E/20 a demonstration match, 22-2C the notices, the title and the ranking again,
  2E/30 back to the start. FF8000 is 2 in the notices as well as in a game: a game has players in (FF80A8).
- The demonstration matches: the introduction's is chosen by FF80E1 (stepped by 2 at 002E78, masked to 0-E,
  table at 002ED8: FF8100); the one after the dojo by FF0C68 (002CF4: both characters and FF8100 from a table
  at 002D12; stepped by 2 at 002610, in the part the attract mode's "matches only" leaves out, so it steps it).
- Colours: the characters' at 0A0000, two sets a character (0x80 bytes each: four palettes), character 0-10;
  copied to the object palettes 12-15 (player 1, 90C180) and 16-19 (player 2) when a match is set up (020DC4),
  and taken by the character screens from the same table. The gems' in a match at 0A0B00 (twelve palettes to
  90C000, 020E18): 0 red, 1 blue, 2 yellow, 3 green, for the gems, crash gems, counter gems and big gems of
  the colour alike; 5 the marks under the scores. The introduction and the dojo copy a whole page from
  0A12C0 + FF8128 * 0x200 (020C9A; FF8128 is 4 there), whose gem palettes are at 0A1AC0. The scroll layer's
  page (from 0A1EC0) has gem-like palettes 12-15 at 0A2040 that nothing on the screens tried was drawn with.
  The two program ROMs: pzfe.03 (this region's) and pzf.04 (0x80000 on, every region's), which holds them all.
- The settings record (FF0F90; 01DE52 copies it to the variables, 01DFA2 the variables back for saving): +00
  coin mode (97), +01 continue (98), +02 (96), +03 demo sound (99), +04 sound mode (9A), +05 chute (9B), +06
  language (BC), +07 (93), +08 (CE), +09 (D2), +0A (D3); +10 difficulty (A0: 0 easiest ... 3 medium ... 7
  hardest), +11 damage (A1: low, normal, high, maximum), +12 game speed (DE: slow ... fastest), +13 game mode
  (A5: types A-D, B the standard; D, one match, clears continue), +14 1P rounds (A2: 1, 2, 3 for one, three,
  five rounds), +15 2P rounds (A3), +16 join-in (A4), +17 handicap (BB). A new settings memory: medium, normal,
  normal, B, one round, three rounds, on, on. A game's start (00A046 on) copies A0 to BE (the computer's skill
  starts from BE), DE to DF and 98 to 11F.
- Characters: 0 Morrigan, 1 Chun-Li, 2 Ryu, 3 Ken, 4 Hsien-Ko, 5 Donovan, 6 Felicia, 7 Sakura, 8 Devilot,
  9 Akuma, 10 Dan (the ladder's last opponent is 10 and becomes 9 as it is shown coming in).
- The sound routines put d0-d3/a3/a4 aside at FF1336; the rewrite check does not compare it.
- Tables reached with `lea table(pc)` are read as data: they are in `program.bin.data`, not
  `program.bin` (which is the deciphered view, right for code and for `move.w table(pc,d0)`).

## Custom soundtrack (2026-10-16)

The toolkit's custom soundtrack (machine/soundtrack.h) with the game's music named in `src/music.cpp`. The sound
program (driver 2.01b) keeps its table of songs at 6000 (a word, 0x500 songs; song 0's entry at 6005). Music is
01-17 and 1E (24 pieces, all named): the characters' themes 01-0B in the game's order (Devilot's 09 and Dan's 0B
by it), the title 0C, the dojo 0D, mode and character select 0E, the logo before each match 0F, winner 10,
intermission 11, continue 12, challenger 13, game over 14, the rankings 15 and 16, the ending and staff roll 17, the
QSound logo 1E. The game sends FF08 (the music faster) when a player is in danger; a file is not made faster.

## Not done

- Not rewritten: the frame interrupt's reading of
  the controls, what comes between rounds (00BDDA: it waits), the set-up of the screens before a match
  (0068D8: it waits), the tasks of the screens around a match (the pictures, the words, the fades), the
  two dispatchers above them (00A1DC, 00A24A: they jump into code that can wait), the ending (00875E),
  the title and demonstrations.
- Training: no preset fields; the drop patterns are not shown.
- Made-up play has not been seen to reach: a one-player game through to the ending, the hidden
  characters, much of the operator's test menu. Code only those reach runs in the interpreter.
- Nobody has played with the training mode by hand; the readout's place on the screen was chosen from
  headless pictures.

## What is left of the game's own code, and why

The rules, the computer player, and every screen of a game from the choice of mode to game over are
readable C++ now. What still runs as translated code:

- The tasks themselves (about seventy kinds at 016A68, ten at 016BA6, five at 016BF8: cursors, words that
  slide in, the winner's picture and words, the count down to continue) and the two characters' acts
  above the fields (013C60, 0140B8 on). They are the game's presentation: long, table-driven, and no
  feature so far reaches into them. Worth doing only for a feature that needs one.
- The pictures of the fields (0151F0, 01495A, 0178A8) and of whole screens (0179E4, 017BA8).
- Sound: the command routines (02B1F4 on) and the hundred small routines that each start one sound.
- What waits for the frame's interrupt, which a rewrite cannot do: the scheduler (001288, 0012CC,
  02CE7C), the dispatchers at 00A1DC and 00A24A, the set-up at 0068D8, between rounds 00BDDA, the returns
  to the title.
- The frame interrupt itself (the controls at 0013EC), the start-up, the operator's test menu, the title,
  the demonstrations, the ending (00875E) and the list of best players.

## Next

1. Play by hand (`make run`); listen to the sound; try training mode from the menu.
2. The ending (00875E); the tasks of the screens around a match
   (02DA46 on: the cursors, the words that slide in, the winner's picture and words).
3. The drop patterns shown in training; preset fields.

## Other work on this game

None found (2026-10-12): no disassembly, decompilation, symbol or RAM map of the arcade game.
FinalBurn Neo lists hacks of the set (colour-blind gems `spf2tcb`, a balance patch `spf2xpri`, a
boss hack `spf2tb`), which shows where others have changed the program but not as sources.
