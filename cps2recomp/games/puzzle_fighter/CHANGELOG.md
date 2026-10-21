# Changelog

This game's changes until it moved into CPS-2 Recomp (2026-10-16). From then on they are in the repository's
[CHANGELOG.md](../../CHANGELOG.md).

All notable changes to this project are written here. The format follows
[Keep a Changelog](https://keepachangelog.com/en/1.1.0/), and versions follow
[Semantic Versioning](https://semver.org/).

## [Unreleased]

### Added

- The arcade game *Super Puzzle Fighter II Turbo* (ROM set `spf2t`, Europe 960529) as a native program,
  translated ahead of time, with 185 of its routines rewritten as readable C++.
- Launcher (ROM set, features, controllers, Add to Steam) and in-game menu (save states, speed, cheats).
- Display options (the toolkit's, cps2recomp#19): Scaling (Sharp, Whole pixels, Smooth), Picture
  shape (4:3, Square pixels, Fill) and Window size (3x to 6x) on the Display page.
- Straight to the title (Game page): the board's start, the notices and the introduction run unseen at
  every start and reset.
- Hidden characters (Game page, for player 1, 2 or both): a kick chooses Devilot, Akuma or Dan outright.
- Alternative modes: attract mode (all of it, the introduction, the demonstration matches or the dojo),
  CPU battle (the computer on both sides), and a colour editor.
- Custom colours (Display page, on in Enhanced): files in the `palettes` folder (or a `pzf.04` changed
  with PalMod) give the characters and the gems new colours; the colour editor makes them.
- Run-ahead (Game page, off unless chosen): off, 1 frame or 2 frames; what you press is seen sooner than
  on the board, and the game runs as it does without it.
- Up drops the pair: the computer drops its pairs too, where it would push them down.
- Board settings (the launcher's and the menu's Board page): the test menu's operator settings (difficulty,
  damage, game speed, game mode, rounds, join-in, handicap, continue, demo sound), each as the board is set
  unless chosen; the board's settings memory is not changed.
- Release preparation: CI on macOS, Linux and Windows, release scripts for each, `make audit`,
  `make check-release`, CONTRIBUTING, CODE_OF_CONDUCT, SECURITY, issue and PR templates, `docs/`,
  `.recomp.json`.
- Training mode: attack readout, input display, dummy settings, crash gems, a diamond next, fall speed,
  empty field, start over. Landing guide; free play and pushing up to drop the pair (on in Enhanced).
- In-game menu: Right on a row that leads to a page goes into that page. The launcher's Features
  page has a search box (Ctrl+F or Cmd+F; Esc empties it, a second Esc leaves it; with a gamepad, A
  goes in and B leaves with what was typed kept). (toolkit cps2recomp 0.3.0, recomp-common 0.5.0)
