# Releasing

Versions are `MAJOR.MINOR.PATCH`, with `-beta.N` or `-rc.N` while pre-release, one for the whole repository:
the toolkit, the launcher and every game. They stay pre-release until someone has played every game's build on
every platform by hand.

## What a release is

Three archives, their checksums and the launcher's three manifests, as a GitHub release of a tag. Nothing else:
a player sees the three downloads.

| File | For |
| --- | --- |
| `CPS2Recomp-macOS.zip` | macOS 13 or later, Apple silicon and Intel (one app, "CPS-2 Recomp"; signed ad hoc, not notarized) |
| `CPS2Recomp-linux-x86_64.tar.gz` | Linux x86_64, Steam Deck, Steam Machine |
| `CPS2Recomp-windows-x64.zip` | Windows 10 and 11, 64-bit |
| `SHA256SUMS.txt` | |
| `update-macos.txt`, `update-linux-x86_64.txt`, `update-windows-x64.txt` | The launcher's updates: each system's manifest |

The release's text is `tools/release/notes.sh`'s. At the top, the paragraph the version's section of `CHANGELOG.md`
begins with: a few sentences on what is in the release, before the section's list (the script stops if there is
none). Then the table of downloads, how to install on macOS, Windows, Linux and the Steam devices, how updating
works, and the rest of the version's section. Change the install steps there, once, for every release.

**The launcher updates itself from these** (`recomp-common/launcher/src/update.h`): when it opens it reads the
repository's releases, takes the newest with a manifest for its system (a beta only while it is a beta itself),
and compares the files the manifest lists with its own. **Update** downloads that system's archive (the same
file as a player's download), checks it against the manifest's SHA-256, unpacks it, checks each file that
changed, and only then puts them in place. Each release script writes its system's manifest after its archive
(`tools/release/lib.sh`, `update_files`), and `tools/release/all.sh` checks that each names its archive as it
is. A release without them is not offered to the launchers, and a release whose `version` line is not its tag is
refused by them. (0.4.0-beta.2 carried a gzipped file for every file of the three, 46 more files, and its
launchers read only that kind: they cannot update themselves to a later release.)

Each holds the launcher (`cps2recomp-launcher`), every game's program (`sfa2`, `vampire_savior`,
`puzzle_fighter`) and the licences. No ROM, graphics, sound or key is in them: the player supplies their own ROM
sets. **Each game's program does hold that game's 68000 and Z80 programs translated to C++ and compiled**, as a
build from source does and as most published recompilations' downloads do. That is a form of the games' code,
and giving it out is a risk the maintainer has accepted (2026-10-12). It also means an archive can only be built
by someone with every game's ROM set, on their own machine: CI never builds or holds one.
`tools/release/check_archive.sh` checks each archive (no game files, no home path, the licences there, every
game's program present and really the translated one, not CI's stand-in).

The tag's source on GitHub is the source the GPL asks for; each archive's `SOURCE.txt` names it. The translated
code is not part of that source: it is made from the player's own ROM sets by the tools that are.

## The steps

1. In a pull request: set `VERSION`, move the "Unreleased" lines of `CHANGELOG.md` under a heading for the
   version, and begin the section with a paragraph saying what is in the release (the release's page opens with it). `make check` passes; for every game, with its ROM set: `make GAME=<id> check` over its ladder movie,
   `check-blocks`, `check-rewrites` and `check-release` (40,000 frames; Puzzle Fighter's ladder is 66,000).
   Merge it.
2. On `main`: `git tag -a vX.Y.Z -m "X.Y.Z"` and `git push origin vX.Y.Z`.
3. `.github/workflows/release.yml` runs on tags `v*`: the audit, and the builds of `ci.yml` on GitHub's macOS,
   Linux and Windows machines (every game's program with no translated code and the launcher, each started with
   no display: the proof the tag compiles everywhere). It then makes a **draft** release whose notes are the
   version's section of the changelog, with no files. A tag with `-beta` or `-rc` is marked pre-release.
4. On a Mac with Docker and every ROM set, at the tag: `make GAME=<id>` for each game, the checks of step 1
   again, then `tools/release/all.sh`, which writes the three archives and `SHA256SUMS.txt` in `build/release/`.
5. Each archive's game programs are run over their ladder movies with `--hash-picture` on their systems (macOS
   both halves, Linux in a container, Windows under Wine): the same picture hashes as the Mac's build. A person
   plays each archive on its system (picture, sound, a gamepad, the launcher's pages, a save state, training
   mode).
6. `gh release upload vX.Y.Z build/release/CPS2Recomp-* build/release/SHA256SUMS.txt build/release/update-*.txt`
   (seven files), then publish the draft. A published release's files are never replaced: a fix is a new
   version. Publishing is what offers it to every launcher that checks, so the archives are played (step 5)
   first.

## A new game

A game is a folder in `games/` (the toolkit's [TOOLKIT.md](TOOLKIT.md) says what goes in it). Before it is
committed here, the repository being public: the same legal audit as the others (nothing from the game, the key
from the player's own set, its names and pictures used only to say what it works with), every source file with
its licence line (`make audit`), and its README in `games/<id>/` with the main README's table of games. The
release scripts take every game in `games/`; the archive check then asks for its program too.
