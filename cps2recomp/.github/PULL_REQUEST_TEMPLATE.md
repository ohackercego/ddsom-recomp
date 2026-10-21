<!-- What this changes, and why. -->

- [ ] Nothing from a game (files, save states, generated code, entry lists, listings, dumps, screenshots other than the README's), no keys or key values, nothing from Capcom's own materials or a leak, no emulator's code. `make audit` passes.
- [ ] What is about one game is in that game's `games/<id>/` (its `game.cfg` or sources), not the toolkit.
- [ ] If the translators, the machine, the picture, the sound or a game's code changed: `make GAME=<id> check` over the ladder movie and `make GAME=<id> check-rewrites` pass for every game (with every feature off a game runs as before).
- [ ] Gameplay and control changes are an optional feature, off in Pure.
- [ ] No added input lag or lost speed (say what was measured, if timing, input, sound or presentation changed).
- [ ] Keyboard and gamepad both still work.
- [ ] `recomp-common/` is unchanged (it is a copy: change recomp-common and run `tools/sync-common.sh`).
- [ ] A line in `CHANGELOG.md` under "Unreleased"; the README (the main one, or the game's) says what players get.
