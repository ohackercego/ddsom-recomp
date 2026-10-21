# CPS-2 Recomp

[![CI](https://github.com/tmcleroy/cps2recomp/actions/workflows/ci.yml/badge.svg)](https://github.com/tmcleroy/cps2recomp/actions/workflows/ci.yml)
[![Release](https://img.shields.io/github/v/release/tmcleroy/cps2recomp?include_prereleases)](https://github.com/tmcleroy/cps2recomp/releases)
[![License: GPL-3.0](https://img.shields.io/badge/license-GPL--3.0-blue)](LICENSE)

**Unofficial native PC ports of Capcom's CPS-2 arcade games (and CPS-1 ones), for macOS, Linux (Steam Deck
included) and Windows, in one download with one launcher.** Each game's program is translated ahead of time into C++
and runs on a machine written for this project, not an emulator. Each fighting game but Street Fighter Alpha (so far)
has a training mode, which no release of these games has ever had; the beat-'em-ups take three players at once, with
free play, a stage to start at, a score attack and viewers for their stages and characters.

**[Get started: download it and play →](#playing)**

> **Beta.** Every game's code is checked instruction by instruction, and the fighting games play from start to
> ending (the beat-'em-ups, new, through their first stages in made-up play), but they have been played on few
> machines and controllers yet. Expect rough edges, and please
> [report what you find](#feedback-and-contributing).

**No game files are included.** You need your own copy of each game: its arcade ROM set (a CPS-2 game's with its key
file), from a board you own. **Only the version named below works for each game.** This project cannot help you get
one.

This project is not affiliated with, endorsed by, or sponsored by Capcom. *Street Fighter*, *Super Puzzle
Fighter*, *Super Gem Fighter*, *Pocket Fighter*, *Vampire Savior*, *Vampire Hunter* and *Darkstalkers* are trademarks of Capcom Co., Ltd.; *Alien vs. Predator*
and *Cadillacs and Dinosaurs* are trademarks of their owners; "CPS-1", "CPS-2" and "QSound" are names and trademarks of
their owners, used here only to say what the programs work with.

![The launcher: every game whose ROM set you have, ready to play](docs/screenshots/launcher-games.png)

## The games

| Game | ROM set | What it adds |
| --- | --- | --- |
| [**Street Fighter Alpha**](games/sfa/README.md) (1995) | `sfa` (Europe 950727) | The game as the board plays it, with the features every game has (free play, straight to the title, the board's settings, cheats, a custom soundtrack); a training mode is next |
| [**Street Fighter Alpha 2**](games/sfa2/README.md) (1996) | `sfa2u` (USA 960430) | A full training mode (hit and throw boxes, frame data and a frame meter, combo, stun and Custom Combo readouts, input display, a dummy that records, replays, guards and answers), widescreen, survival, ring out, endless battle, the stage and model viewers, the hidden characters, custom colours |
| [**Super Gem Fighter Mini Mix**](games/pocket_fighter/README.md) (1997, *Pocket Fighter*) | `sgemf` (USA 970904) | A full training mode (hit boxes, frame data and a frame meter, input display, combo and stun readouts, a dummy that records, replays and guards), widescreen, survival, ring out, endless battle, the stage and model viewers, the hidden characters with no code, custom colours |
| [**Super Puzzle Fighter II Turbo**](games/puzzle_fighter/README.md) (1996) | `spf2t` (Europe 960529) | A training mode (attack readout, landing guide, input display, a dummy that sends counter gems to cancel, crash gems as you choose), CPU battle, the hidden characters at a kick, custom colours for the characters and the gems |
| [**Vampire Savior: The Lord of Vampire**](games/vampire_savior/README.md) (1997) | `vsavj` (Japan 970519) | A full training mode (hit boxes, frame data and a frame meter, input display, a dummy that records, replays and guards), widescreen, survival, ring out, endless battle, the stage and model viewers, the hidden characters, custom colours |
| [**Vampire Savior 2: The Lord of Vampire**](games/vsav2/README.md) (1997) | `vsav2` (Japan 970913) | Donovan, Phobos and Pyron back, and Donovan's stage; everything Vampire Savior's port has (its sources, checked against this game's code) |
| [**Vampire Savior Definitive**](games/vsav_definitive/README.md) (beta) | `vsavj` and `vsav2` | Vampire Savior with Donovan, Phobos and Pyron taken from Vampire Savior 2, all eighteen by Vampire Savior's rules, the three with places of their own at the character choice; everything Vampire Savior's port has |
| [**Vampire Hunter 2: Darkstalkers Revenge**](games/vhunt2/README.md) (1997) | `vhunt2` (Japan 970929) | Night Warriors' cast on Vampire Savior's engine, each fighter's own theme; everything Vampire Savior's port has, and Dark Gallon among the hidden characters |
| [**Alien vs. Predator**](games/avsp/README.md) (1994) | `avspu` (USA 940520) | Three players at once (the board's own setting, on in Enhanced), free play, a round to start at, score attack, attract mode, the stage and character viewers, rapid fire, the board's settings, infinite health, lives and time |
| [**Dungeons & Dragons: Shadow over Mystara**](games/ddsom/README.md) (1996) | `ddsom` (USA 950101) | Four players at once (the board's own setting, on in Enhanced), free play, a stage to start at, score attack, attract mode, the stage and character viewers, rapid fire, the board's settings and cheats, infinite health, lives and time |
| [**Cadillacs and Dinosaurs**](games/dino/README.md) (1993, CPS-1) | `dino` (World 930201) | The first CPS-1 game here, on a CPS-1 board written for it; three players at once, free play, a stage to start at, score attack, attract mode, the stage and character viewers, rapid fire, the board's settings, infinite health, lives and time |

| | | |
| --- | --- | --- |
| [![Street Fighter Alpha 2: training mode](docs/screenshots/sfa2/training.png)](games/sfa2/README.md) | [![Super Puzzle Fighter II Turbo: training mode](docs/screenshots/puzzle_fighter/training.png)](games/puzzle_fighter/README.md) | [![Vampire Savior: training mode](docs/screenshots/vampire_savior/training.png)](games/vampire_savior/README.md) |
| [![Vampire Savior 2: Phobos, Pyron and Donovan](docs/screenshots/vsav2/returning.png)](games/vsav2/README.md) | [![Vampire Hunter 2: Gallon, Aulbath and Sasquatch](docs/screenshots/vhunt2/night-warriors.png)](games/vhunt2/README.md) | [![Vampire Savior 2: training mode on Donovan's stage](docs/screenshots/vsav2/training.png)](games/vsav2/README.md) |
| [![Vampire Savior Definitive: Phobos against Morrigan](docs/screenshots/vsav_definitive/phobos.png)](games/vsav_definitive/README.md) | [![Vampire Savior Definitive: the character choice, the three's places under the grid](docs/screenshots/vsav_definitive/choosing.png)](games/vsav_definitive/README.md) | [![Vampire Savior Definitive: the versus screen](docs/screenshots/vsav_definitive/versus.png)](games/vsav_definitive/README.md) |
| [![Alien vs. Predator: three players](docs/screenshots/avsp/three-players.png)](games/avsp/README.md) | [![Cadillacs and Dinosaurs: three players](docs/screenshots/dino/three-players.png)](games/dino/README.md) | [![Cadillacs and Dinosaurs: the mansion](docs/screenshots/dino/mansion.png)](games/dino/README.md) |

Each game's page has all it does, with pictures.

## Every game has

- **Keyboard, gamepad or arcade stick**, with every key and button changeable, for each player, once for all
  the games (the launcher's Controllers page).
- **An in-game menu** (Esc or `F1`; on a gamepad, the left stick's click or Start and Select together): the
  game's modes, save states, speed, pause and frame advance, cheats, and every feature.
- **Pure or Enhanced.** Pure plays exactly as the board does. Enhanced (the default) adds free play and your
  own colours. Every other feature is a switch of its own, off unless you choose it.
- **Rapid fire**: a button held down pressed again and again, fifteen times a second, for the first button or for
  all of them; the first press seen at once.
- **Arcade timing** (CPS-2 games): the processor run as on the arcade board, its waits for memory and the video chip's use of its
  bus counted, so the game slows down where and as much as the board does (big supers, busy screens). Off unless
  chosen; checked frame by frame against a cycle-exact model of the board ([docs/TIMING.md](docs/TIMING.md)).
- **Run-ahead**: what you press seen a frame or two sooner than on the board, with the game running exactly as
  without it.
- **The picture your way**: sharp, whole pixels or smooth; 4:3, square pixels or the whole screen; full screen
  or a window of the size you choose; widescreen in the fighting games (all but Street Fighter Alpha, so far).
- **The board's operator settings** (difficulty, damage, timer speed, game speed, rounds and the rest), each as
  the board is set unless you choose, and the game's own test menu.
- **Straight to the title**: at a start or a reset, everything before the title runs unseen.
- **A custom soundtrack**: your own music files (an arranged soundtrack from a console version, say) in place of
  the board's music, song by song, with the board's sound effects and voices. [How →](#a-custom-soundtrack)
- **Your ROM sets are only ever read.** Settings, save states and each board's settings memory go in your own
  user folder.
- **Updates in the launcher**: new games and fixes come to the copy you have. When a new version is out the
  launcher says so at the top of its Games page, and **Update** downloads the new version, checks it, puts
  what changed in place and starts the new launcher. [More →](#updates)

![The Features page: what the games share, set for all of them at once](docs/screenshots/launcher-features.png)

## Playing

1. Download the build for your system from the [releases page](https://github.com/tmcleroy/cps2recomp/releases)
   and unpack it. Each release's page has the steps for macOS, Windows, Linux and the Steam devices.
2. Open the launcher (`CPS-2 Recomp.app`, `cps2recomp-launcher`). On its Games page, **Choose folder…** and pick
   the folder you keep your ROM sets in: it finds each game's by the set's name (`sfa.zip`, `sfa2u.zip`,
   `sgemf.zip`, `spf2t.zip`, `vsavj.zip`, `vsav2.zip`, `vhunt2.zip`, `avspu.zip`, `dino.zip`, or a folder of the set's files). Or drop a set, or the folder, on the window, or choose each
   game's set on its own. They stay where they are.
3. Press **Play** beside a game. Several games can run at once, each in its own window.

Each game you have a ROM set for gets a page of its own in the sidebar: its Display switches (full screen,
widescreen), Play, and the features only it has. The **Features** page has what the games share (free play,
cheats, run-ahead, the picture, training mode's displays and the dummy, and the rest), set for every game at
once; a feature changed in a game's own menu is set for the others too when you leave the game. The
**Controllers** page sets the keys and buttons for all of them.

While playing, Esc or `F1` (on a gamepad, the left stick's click, or Start and Select together) opens the
in-game menu. Only the menu's "Quit game" row (or closing the window) ends the game.

macOS: the app is not notarized, so the first time, right-click it and choose Open.
Linux: unpack the archive anywhere and run `cps2recomp-launcher`.
Windows: run `cps2recomp-launcher.exe`. Opening a game's own program (`sfa2.exe`, say) starts the launcher too,
at that game's page (or straight into the game once its ROM set has been chosen).

### Updates

When the launcher opens it asks GitHub for this project's releases (the Games page's Updates card has a switch to
stop that, and **Check now**). When there is a newer version it shows at the top of the Games page: **What's new**
opens its notes, and **Update** downloads the new version's download for your system (the same file as on the
releases page), checks it and every file that changed against the release's list, and only then
puts them all in place; if anything goes wrong nothing is changed. The launcher then starts again as the new
version. Your settings, save states and ROM sets are not touched. Quit the games first: Update waits until none is
running. A beta is offered only to a beta.

The launcher needs to be able to write to its own folder: on macOS, move `CPS-2 Recomp.app` to Applications (or
anywhere of your own) before opening it the first time, not run it from the downloaded zip's folder. A copy built
from source is told of a new version but not changed.

### From a terminal

```sh
cps2recomp-launcher --play sfa2                 # straight into a game, as the launcher has it set
cps2recomp-launcher --play sfa2 -- --features widescreen,no_damage
                                                # with more arguments for the game, after the launcher's own
cps2recomp-launcher --page vampire_savior       # the launcher, at a game's page
sfa2 --rom ~/roms/sfa2u.zip --settings my.ini   # a game's own program (beside the launcher), by itself
sfa2 --soundtrack ~/music/sfa2-arranged          # a custom soundtrack's folder (with the feature on)
sfa2 --help                                     # what a game's program takes
```

The games' ids are `sfa`, `sfa2`, `pocket_fighter`, `puzzle_fighter`, `vampire_savior`, `vsav2`, `vhunt2`, `vsav_definitive`, `avsp`, `ddsom` and `dino`. `--features` may be given more than once, and
a later one wins.

### A custom soundtrack

Each game can play your own music files in place of the board's music: an arranged soundtrack, say. The sound
effects and voices stay the board's.

1. Switch on **Custom soundtrack** (the Features page, under Sound; it is on in Enhanced, off in Pure).
2. On a game's page, under Sound, **Choose…** the folder your music files are in (`.ogg`, `.flac`, `.mp3` or
   `.wav`). With none chosen, the game uses the `soundtrack` folder in its own settings folder
   (`~/Library/Application Support/<Game>/` on macOS, `~/.local/share/<Game>/` on Linux and Steam Deck,
   `%APPDATA%\<Game>\` on Windows), which it makes.

That is all it takes: **a folder of music plays as a playlist**, in place of all the game's music, one file after
another in their names' order (`2 ...` before `10 ...`), and from the first again after the last. Choose **On,
shuffled** for a random order.

**To replace particular songs** instead, name each file after the piece of music it replaces: `ryu.flac`,
`player_select.ogg`, `ken_ending.mp3`. The game writes `songs.txt` into the folder, with every piece of its music
by name and number and what it is (Street Fighter Alpha 2 has 59: each character's theme, each ending, the staff
rolls, the title and the screens between fights). Once one file is named for a song, the folder works this way: a
song you have no file for plays as the board has it, and other files are left alone.

For files with other names, loops and loudness, put `soundtrack.ini` in the folder:

```ini
volume = 80                  ; every song, in percent

[ryu]                        ; a song by its name (or its number: [01])
file = 05 Ryu's Theme.flac
loop = 0:12.5 - 2:47.25      ; the part that repeats: start - end (minutes:seconds, or seconds)
volume = 90
```

A song's file repeats whole unless it says otherwise: by `loop` in `soundtrack.ini`, `loop = no` to play it once, or the
loop tags many soundtrack packs have (`LOOPSTART`, with `LOOPEND` or `LOOPLENGTH`, in samples). The music the board
plays once (a fanfare, the versus screen, a game over) is played once. The music keeps the game's time: pausing,
the menu, slow motion, a save state and run-ahead leave it where the game is (a playlist plays on, and waits only
while the game is held). The game plays exactly the same with
or without it.

No music is included, and this project can't help you get any: use files you made from discs you own.

### Coming from a game's own download

Earlier releases came one game at a time (Street Fighter Alpha 2 Recomp, Vampire Savior Recomp). This launcher
keeps each game's settings, save states and board memory in the same folder as before, and takes the ROM set you
chose in a game's own launcher, so everything carries on. A game's own release still works.

### Steam Deck

1. Hold the power button and choose **Switch to Desktop**.
2. Download `CPS2Recomp-linux-x86_64.tar.gz` from the [releases page](https://github.com/tmcleroy/cps2recomp/releases).
   In the file manager (Dolphin), right-click it and choose **Extract → Extract archive here**, then move the
   `CPS2Recomp` folder where it can stay (for example a new `Applications` folder in your home folder). If you
   move it later, do step 5 again.
3. Copy your ROM sets onto the Deck into one folder, for example from a USB stick or the SD card.
4. In the `CPS2Recomp` folder, open `cps2recomp-launcher`; on its Games page, **Choose folder…** and pick that
   folder.
5. On each game's page, press **Add to Steam**. It adds the game to your library (straight into it), and the
   first time also **CPS-2 Recomp** (the launcher: ROM sets, features and controls).
6. Go back to Gaming Mode with the **Return to Gaming Mode** icon on the desktop. They are in your Library under
   **Non-Steam**.

In a game, click the left stick, or press View and Menu together (the two buttons either side of the screen's
top), to open the in-game menu; its "Quit game" row ends the game.

## What you need

- The ROM set of each game you want to play (a CPS-2 game's with its key file): `sfa`
  (*Street Fighter Alpha: Warriors' Dreams*, Europe 950727), `sfa2u` (*Street Fighter Alpha 2*, USA 960430),
  `sgemf` (*Super Gem Fighter: Mini Mix*, USA 970904),   `spf2t` (*Super Puzzle Fighter II Turbo*, Europe 960529), `vsavj` (*Vampire Savior: The Lord of Vampire*,
  Japan 970519), `vsav2` (*Vampire Savior 2: The Lord of Vampire*, Japan 970913), `vhunt2` (*Vampire Hunter 2:
  Darkstalkers Revenge*, Japan 970929), `avspu` (*Alien vs. Predator*, USA 940520), `ddsom` (*Dungeons & Dragons: Shadow over Mystara*, USA 950101), `dino` (*Cadillacs and Dinosaurs*,
  World 930201). Vampire Savior Definitive needs both `vsavj` and `vsav2`. As a zip (with its parent set's zip beside it if your collection keeps the shared files there)
  or a folder of the set's files. The launcher checks every file by checksum and tells you which is missing or
  different. Sets of other regions and revisions are refused: each port is made from that one version's
  program, and its features use addresses found in it.
- One of: macOS 13 or later (Apple silicon or Intel); Linux x86_64 or SteamOS; Windows 10 or 11, 64-bit.
- A gamepad or stick (six buttons for the fighting games) is recommended; for three players in the beat-'em-ups, three
  gamepads (or keys chosen for each player).

## How it works

Each game's 68000 program is translated ahead of time into C++ and compiled into that game's program, and the
parts its features touch are rewritten by hand as readable C++, each checked call by call against the game's
own code. What the program expects of the board (its video, the QSound chip, the sound CPU and its registers,
the settings memory) is code written for this project, not an emulator. The [toolkit](docs/TOOLKIT.md) is shared
by every game, so a fix or a feature there reaches all of them; each game's page says how its port works. Games
Capcom made from another's program (Vampire Savior 2 and Vampire Hunter 2, from Vampire Savior's) build that game's
sources, with a list of where each place they name is in the other game, so they have all of its features. Vampire
Savior Definitive runs Vampire Savior 2's own code for the characters it takes from it, beside Vampire Savior's, with
that game's pictures and sound ([how and why](games/vsav_definitive/DEFINITIVE.md)).

## Building from source

With your own ROM sets in `rom/<id>/` (ignored by Git), CMake, Ninja, SDL 3 and Python 3:

```sh
make GAME=sfa2              # twice the first time
make GAME=sfa2 explore      # once, about ten minutes: finds the game's code by made-up play and translates it
make collection             # every game whose ROM set is here, and the launcher for all of them
```

[docs/BUILDING.md](docs/BUILDING.md) has every platform, the checks and the release builds, and
[docs/TOOLKIT.md](docs/TOOLKIT.md) how the toolkit works and how a game is added. `make check` needs no game.

## Feedback and contributing

Bug reports and pull requests are welcome: see [CONTRIBUTING.md](CONTRIBUTING.md) for the rules (above all:
nothing from any game, ever, and never a key) and each game's roadmap for what is next. Security reports go by
[SECURITY.md](SECURITY.md).

## Related projects

| Project | What |
| --- | --- |
| [satrecomp](https://github.com/tmcleroy/satrecomp) | The same kind of toolkit for Sega Saturn games |
| [recomp-common](https://github.com/tmcleroy/recomp-common) | The in-game menu, settings rules and launcher this project and satrecomp share |
| [Muscle March Recomp](https://github.com/tmcleroy/muscle-march-recomp) | A native port of *Muscle March* (WiiWare), by the same author |
| [doaxvb-re](https://github.com/NoRain211/doaxvb-re) | A native Windows port of *Dead or Alive Xtreme Beach Volleyball* (Xbox), by NoRain211; not made with these toolkits |

## License and credits

[GPL-3.0-only](LICENSE). [THIRD_PARTY_NOTICES.md](THIRD_PARTY_NOTICES.md) has the notices in full.

- The program cipher (`machine/cps2crypt.*`) is MAME's (BSD-3-Clause; its header says so), worked out by
  Andreas Naive and Nicola Salmoria. The QSound model follows MAME's high-level one (superctr, Valley Bell), and
  the hardware facts follow MAME's CPS-2 and CPS-1 drivers. The Kabuki cipher of the CPS-1's sound CPU
  (`machine/kabuki.*`) is written from the facts MAME's `kabuki.cpp` publishes (Nicola Salmoria); its keys are not
  here: the port works each out from the player's own ROM.
- The launcher's look, and the files that make it, come from the launcher of
  [wiiparty-recomp](https://github.com/arelkair/wiiparty-recomp) by arelkair (GPL-3.0-or-later). They are in
  `recomp-common/launcher/`, with the notices in recomp-common's `THIRD_PARTY_NOTICES.md`.
- The programs use SDL 3 (zlib license), Dear ImGui (MIT) and the Inter font (SIL OFL 1.1, compiled into the
  launcher); the release archives carry their licenses.

The repository holds no game data and nothing derived from it.
