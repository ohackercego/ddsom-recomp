# Features

Every change to how the game plays, looks or is controlled is a switch, set in the launcher's
Features page or in the in-game menu (Esc, `F1`, a gamepad's left stick click) and kept in
`settings.ini`. With every one off the game runs exactly as the arcade board does. The launcher's
presets: **Pure** (all off), **Enhanced** (the default: free play, up drops the pair and custom
colours on), **Custom**.

The tables are what `puzzle_fighter --list-features` prints. How features are written is in
[cps2recomp/docs/FEATURES.md](../cps2recomp/docs/FEATURES.md).

## Board settings

The Board page holds the board's own operator settings, as the game's test menu has them
(Configuration): each is "As the board is set" unless chosen, and a choice is taken when the game
starts or is reset. The board's settings memory is not changed. The in-game menu's "Test menu" row
(or F2) opens the game's own test menu instead.

## Alternative modes

"Alternative modes", the first row of the in-game menu, has ways of running the game that the
board does not have. Each mode has a page there with a Start row and its choices, and ends when
another is started or with the menu's Reset.

- **Training mode**: straight to the character choice with two players (player 1's controls choose
  both); player 2 stands by, and the round starts over when your field fills up. The Training mode
  and Dummy features below are its aids.
- **Attract mode**: the game's own show on a loop with no coin in, even with free play on: all of it,
  or only the introduction, the demonstration matches (a different one each time) or the
  how-to-play dojo. Start or a coin ends it.
- **CPU battle**: a two-player match that the computer plays on both sides, at the
  strength chosen, with the characters chosen or at random; when it is over the next begins.
- **Colour editor**: a match that nobody plays, with a panel worked by player 1's controls: the
  character (any of the eleven, the hidden three too) or the gems, the set of colours (A or B: the
  button that chooses it; for the gems, those of a match or those of the attract's demonstration
  and dojo), its line (for the gems: red, blue, yellow, green; player 1's field holds a gem, a
  crash gem and a counter gem of each), the colour, and its red, green and blue (the board's 16
  levels each), with the character standing or attacking over and over (the attacks show its other
  lines' colours) and the stage or plain black or white behind. LP copies a colour, MP pastes it, HP puts back the game's, LK twice on the Random
  colours row gives the set random colours and HK takes them back. Changes are saved to the
  character's file (or `Gems.txt`) in the `palettes` folder and used in play with Custom colours on;
  "Save set" keeps a set in the library and "Load" puts one back.

## Game

| Feature | Name in `settings.ini` | What it does |
| --- | --- | --- |
| Free play | `free_play` | Start works without a coin. On in Enhanced. |
| Landing guide | `landing_guide` | Outlines where the two gems of the pair you are steering would come to rest if you let it fall now. |
| Up drops the pair | `hard_drop` | Pushing the stick up puts the pair you are steering straight down where it would come to rest, and it lands at once. The computer does it too, where it would push the pair down. On in Enhanced. |
| Hidden characters | `hidden_characters` | At the choice of characters a kick chooses one of the three hidden characters outright, with no code: light kick Devilot, medium kick Akuma, hard kick Dan. (The board's codes still work.) A person playing one of them sends three quarters of the usual attack, as on the board. For player 1, player 2 or both. Choices: Off, Player 1, Player 2, Both. |
| Straight to the title | `skip_intro` | When the game starts, and when it is reset, goes straight to the title: the board's memory test, the notices, the introduction and its demonstration match are run unseen, in a second or so. |
| Run-ahead | `run_ahead` | Shows the game a frame or two further on than it is, so that what you press is seen sooner than on the board (input lag taken away). The game runs as it does without it; it takes two or three times the processor. Off by itself in the game's own modes beyond fights and training. Choices: Off, 1 frame, 2 frames. |

## Board

| Feature | Name in `settings.ini` | What it does |
| --- | --- | --- |
| Difficulty | `board_difficulty` | How well the computer plays against you. Choices: As the board is set, Easiest ... Hardest (9). |
| Damage level | `board_damage` | How many counter gems an attack sends. Choices: As the board is set, Low, Normal, High, Maximum. |
| Game speed | `board_speed` | How fast the pairs fall. Choices: As the board is set, Slow, Normal, Fast, Fastest. |
| Game mode | `board_mode` | The board's game types; B is the standard one, D one match only. Choices: As the board is set, Type A, Type B (standard), Type C, Type D (one match). |
| Rounds against the computer | `board_rounds_1p` | Rounds a match against the computer lasts at most. Choices: As the board is set, 1 round, 3 rounds, 5 rounds. |
| Rounds, two players | `board_rounds_2p` | Rounds a match between two players lasts at most. Choices: As the board is set, 1 round, 3 rounds, 5 rounds. |
| Join-in | `board_join_in` | Whether a second player can join a game against the computer. Choices: As the board is set, Off, On. |
| Handicap | `board_handicap` | Whether two players choose handicap levels. Choices: As the board is set, Off, On. |
| Continue | `board_continue` | Whether a game over can be continued. Choices: As the board is set, Off, On. |
| Demo sound | `board_demo_sound` | Whether the attract mode plays sound. Choices: As the board is set, Off, On. |

## Display

| Feature | Name in `settings.ini` | What it does |
| --- | --- | --- |
| Custom colours | `custom_colours` | Characters and gems in the colours of the files in the "palettes" folder in the game's data folder: a pzf.04 changed with PalMod, and text files, one a character (Ryu.txt) and one for the gems (Gems.txt), which go before it. With no files the game's own colours. They are taken when the game next sets its colours: at the next match. On in Enhanced. |
| Scaling | `display_scaling` | Sharp: as large as fits, each pixel a crisp block. Whole pixels: every pixel the same whole number of the screen's pixels tall (a border round it). Smooth: softened all over. Choices: Sharp, Whole pixels, Smooth. |
| Picture shape | `display_shape` | 4:3: as the arcade's monitor showed it. Square pixels: each pixel as wide as it is tall, a little wider than 4:3. Fill: the whole window or screen, stretched to it. Choices: 4:3, Square pixels, Fill. |
| Window size | `window_size` | How large the window opens, in times the board's 224 lines (it can be made any size by dragging its edges; full screen is on the launcher's first page and F11). Choices: 3x, 2x, 4x, 5x, 6x. |

## Training mode

| Feature | Name in `settings.ini` | What it does |
| --- | --- | --- |
| Crash gems | `training_crash` | In a training session, what player 1's pairs bring: as the game deals them, a crash gem in every pair, or none at all (to build without setting anything off). Choices: As the game deals, In every pair, None. |
| Pairs fall | `training_speed` | In a training session, how fast player 1's pairs fall: faster as the round goes on, as the game has it; as at the start of a round throughout; or slowly, for time to think. Takes effect with the next pair. Choices: As the game has it, As at the start, Slowly. |
| Input display | `input_display` | In a training session, player 1's controls as the game reads them, the newest at the top, each with the frames it was held: the stick in numpad notation (5 is the middle, 4 left, 6 right, 2 down), A and B the two buttons that turn the pair. Shown over player 2's field. |
| Attack readout | `attack_readout` | Over a player's field after each break: the gems broken and the chain, the score, the counter gems the break made, and how many of the opponent's it cancelled or the turn's total. |

## Dummy

| Feature | Name in `settings.ini` | What it does |
| --- | --- | --- |
| Player 2 | `dummy` | In a training session: player 2 stands by (its side does nothing and takes no counter gems), or plays with its own controls. Choices: Stands by, Plays. |
| Attacks with | `dummy_attack` | While it stands by, player 2 sends this many counter gems at the start of every fourth turn of yours, to practise cancelling them. They fall in the pattern of player 2's character. Choices: Nothing, 3 counter gems, 6, 12, 24, 48. |

## Attract mode

| Feature | Name in `settings.ini` | What it does |
| --- | --- | --- |
| Shows | `attract_show` | What attract mode plays: everything the board does, the introduction over and over (with its demonstration match and the title), one demonstration match after another, or the how-to-play dojo. Choices: Everything, The intro only, The matches only, The dojo only. |

## CPU battle

| Feature | Name in `settings.ini` | What it does |
| --- | --- | --- |
| Player 1 | `watch_1` | Player 1's character, or Random: one of the eight, a different one each match. Choices: Random, Morrigan ... Dan (12). |
| Player 2 | `watch_2` | Player 2's character, or Random: one of the eight, a different one each match. Choices: Random, Morrigan ... Dan (12). |
| Strength | `watch_strength` | How well the computer plays both sides: its level, which the game counts from 0 to 31 (here 4, 12, 20 or 31). The game's own opponents grow stronger along the ladder in the same way. Choices: Weak, Middling, Strong, Strongest. |

## Colour editor

| Feature | Name in `settings.ini` | What it does |
| --- | --- | --- |
| Animation | `colours_animation` | What the character does in the editor: stands in the game's idle, or goes through one of its animations (as in the model viewer). The panel has this row too. Choices: Standing, Small attack, Attack, Big attack, Bigger attack, Huge attack, Finisher. |
| Character | `model_character` | Whose colours the editor changes: a character (shown as player 1, in a match nobody plays), or the gems (in player 1's field: a row of each colour with a crash gem and a counter gem, and the diamond). Changing it while editing goes to the new one. Choices: Ryu, Chun-Li ... Gems (12). |
| Background | `model_background` | What is behind the gems and the characters: the match's stage, or plain black or white (the fields' frames and the words go too). Choices: The stage, Black, White. |
| Size | `model_size` | As large as in a match. Choices: As in a match. |

## Cheats

| Feature | Name in `settings.ini` | What it does |
| --- | --- | --- |
| Player 2 cannot attack | `no_attack_2` | Player 2's breaks (the opponent's, in a one-player game) make no counter gems. |
| Player 1 cannot attack | `no_attack` | Player 1's breaks make no counter gems. |
| Hidden challenger appears | `challenger` | Against the computer, the hidden challenger comes in place of the seventh opponent, as for a player who has earned it (if no continue has been used). |
| Player 1 wins quickly | `quick_win` | Player 1 sends forty counter gems with every pair, and player 1's field is emptied before it can fill. |

