#!/bin/sh
# SPDX-License-Identifier: GPL-3.0-only
# Copyright (C) 2026 Tommy McLeroy. Part of cps2recomp.
#
# Checks the build with NO translated code (NO_GAME_CODE=1 in tools/release/lib.sh), which runs the ROM
# set's program in the port's interpreters: what CI builds, and what a checkout is before `make explore`.
# (A release holds the translated build, which `make check` and `check-blocks` cover.) Over the ladder movie:
#   1. two runs of it make the same pictures;
#   2. it makes the pictures the translated build makes, but for a few frames (it takes interrupts between
#      instructions, the translated build between blocks), and ends on the same picture.
# The file of picture hashes it prints the checksum of is what a build for any other system must write too.
#
#   GAME=<id> check_release.sh [FRAMES]   (make GAME=<id> check-release: from the repository's top, after `make`; ROM=your set)
set -eu
GAME=${GAME:?which game: GAME=<id>}
ROM=${ROM:-rom/$GAME}
frames=${1:-40000}
allowed=${ALLOWED:-$(python3 tools/gamecfg.py release_allowed "$GAME")} # (the interpreted build may show a stretch one frame early)
allowed=${allowed:-0}
# (the features the ladder is played with: the game's, or what lets player 1 win every match)
features=$(python3 tools/gamecfg.py release_features "$GAME")
features=${features:-free_play,no_damage,one_hit}
work=work/$GAME/check
build=build/$GAME-nocode
mkdir -p "$work" "$build/no-game-code/gen"
python3 tools/gamecfg.py --header "$build/no-game-code/gen/game_info.h" "$GAME"
cmake -S . -B "$build" -G Ninja -DCMAKE_BUILD_TYPE=Release -DGAME="$GAME" -DGAME_DIR="$PWD/games/$GAME" \
    -DWORK_DIR="$PWD/$build/no-game-code" > /dev/null
ninja -C "$build" "$GAME" > /dev/null
run() { # program, name
    "$1" --rom "$ROM" --headless --frames "$frames" --movie "games/$GAME/movies/ladder.txt" \
        --features "$features" --hash-picture --hash-log "$work/$2.txt" 2> "$work/$2.err" || {
        echo "FAILED: the $2 run stopped"; cat "$work/$2.err"; exit 1; }
}
run "$build/$GAME" release & run "$build/$GAME" release-again & run "build/$GAME/$GAME" translated & wait
status=0
if cmp -s "$work/release.txt" "$work/release-again.txt"; then echo "ok: two runs of the interpreted build alike ($frames frames)"; else echo "FAILED: two runs of the interpreted build differ"; status=1; fi
differ=$(diff "$work/release.txt" "$work/translated.txt" | grep -c '^<' || true)
if [ "$differ" -le "$allowed" ] && [ "$(tail -1 "$work/release.txt")" = "$(tail -1 "$work/translated.txt")" ]; then
    echo "ok: the interpreted build's pictures are the translated build's but for $differ frames, and it ends on the same one"
else
    echo "FAILED: the interpreted build's pictures differ from the translated build's in $differ frames (allowed: $allowed), or it ends elsewhere"; status=1
fi
if command -v sha256sum > /dev/null; then sum=sha256sum; else sum="shasum -a 256"; fi
echo "picture hashes: $($sum < "$work/release.txt" | cut -d' ' -f1)"
exit $status
