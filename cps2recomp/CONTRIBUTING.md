# Contributing

Thanks for helping! This repository is CPS-2 Recomp: the toolkit, the launcher, and every game ported with
them (`games/<id>/`). A change to the toolkit reaches every game at once.

## The rules that keep this project legal

These aren't negotiable; pull requests that break them are closed.

1. **Never add anything from a game.** No ROM or part of one, no extracted graphics, sounds or text,
   no settings memory, save states, RAM dumps or video, and nothing generated from a game: generated
   C++ (`gen/`), entry lists (`entries.txt`), disassembly listings. What a game's folder may hold is its
   own code, its `game.cfg` (the ROM set's file names and checksums), notes found by playing and
   watching memory, and input movies (text: which buttons were held on which frame). The README's
   pictures of the games are made by the programs and kept few. The same goes for issues: attach
   logs, not game files.
2. **Never add a key**, a key file, key values, or a tool that writes key files; and never point to
   where ROM sets or keys can be found.
3. **Nothing from leaked material**: no leaked source, and no names taken from it.
4. **No emulator's code**, beyond what `THIRD_PARTY_NOTICES.md` lists (MAME's implementation of the
   program cipher). Reading an emulator to learn how the hardware behaves is fine; copying its code
   in is not.
5. **Only add code you have the right to contribute**, under the GPL-3.0. Code from other projects
   must have a compatible license and be credited in `THIRD_PARTY_NOTICES.md`.

## Where things go

The toolkit (everything outside `games/`) may not be about one game: no addresses, names or special cases
of a game's. What a game needs that others do not goes in that game's `game.cfg` or its own sources
(`games/<id>/src`), through the hooks in `runtime/lift.h` and `runtime/game.h`. A feature every game can have
belongs in the toolkit; the launcher shows a feature that more than one game has, under the same name and
with the same choices, once for all of them (a game marks one that must stay its own: `OwnFeatures`).

`recomp-common/` is a copy of another repository and is never changed here: changes go to
[recomp-common](https://github.com/tmcleroy/recomp-common), then `tools/sync-common.sh`.

## Building and testing

`make check` needs no game: it audits the files (`tools/audit.sh`), checks the copy of recomp-common,
builds the program with no translated code and the launcher for the stand-in game in `example/`, and
starts both. CI builds and starts every game's program (with no translated code) and the collection's
launcher on macOS, Linux and Windows. [docs/BUILDING.md](docs/BUILDING.md) has the rest.

A change to the translators, the machine, the picture, the sound or a game's code needs the games as well,
which only someone with their ROM sets can run (`rom/<id>/`):

- `make GAME=<id> check FRAMES=40000 MOVIE=games/<id>/movies/ladder.txt`: the interpreter and the translated
  code must give the same state after every frame, and two runs must be alike;
- `make GAME=<id> check-rewrites`: every hand-written routine of the game against the code it replaces;
  `check-blocks`: every translated block beside the interpreter; `check-sound`: the sound driver against the
  game's sound program;
- with every feature off a game must run exactly as before: the same movie must give the same hashes
  (`--movie`, `--hash-log`).

A toolkit change is checked with every game, a game's change with that game. Say in the pull request which
were run. Nothing may add input lag or slow a game
down: measure when in doubt. Both the keyboard and gamepads must keep working.

## Code style

- Match the surrounding code: naming, comment density and idiom. Comments explain why.
- C++20; 4-space indents, braces on the same line, lines up to about 120 columns.
- Every source file starts with the license and copyright lines the others have; the audit fails
  on one without.
- Commit messages: a short summary line, then what changed and why.

## Code of conduct

Be kind and constructive. See [CODE_OF_CONDUCT.md](CODE_OF_CONDUCT.md).
