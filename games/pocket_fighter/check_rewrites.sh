#!/bin/sh
# SPDX-License-Identifier: GPL-3.0-only
# Copyright (C) 2026 Tommy McLeroy
#
# Checks the game's rewritten routines (src/) against the code they replace: every call of each, over the game's
# movies, run both ways and compared (docs/READABLE.md). The rewrites (the camera's place, keeping the players inside
# the picture: widescreen.cpp) do not call one another through the game's code, so each movie is run once. Run from the
# repository's top.
GAME=pocket_fighter
port=${B:-build/$GAME}/$GAME
movies=games/$GAME/movies
status=0
. tools/at-once.sh
check() { # movie, frames, features, rewrites to leave out, more of the port's options
    out=$(CPS2RECOMP_SKIP_REWRITES="$4" $port --rom "${ROM:-rom/$GAME}" --headless --frames "$2" --movie "$movies/$1.txt" --features "$3" $5 --check-rewrites 2>&1)
    failed=$?
    echo "== $1${4:+ (without $4)}${5:+ ($5)}"
    echo "$out" | grep -E "^rewrite|did not" | grep -v " 0 calls"
    echo "$out" | grep -E "differs|calls, [1-9][0-9]* differed" > /dev/null && return 1
    return $failed
}
run() {
    at_once check "$@"
}
for skip in "" "CameraAcross"; do
    run versus 14000 free_play "$skip"
    run ladder 40000 free_play,no_damage,one_hit "$skip"
    # (two players, made up: they walk into each other, jump and throw)
    run versus 20000 free_play "$skip" "--auto 7:1400"
    run versus 20000 free_play "$skip" "--auto 13:1400"
done
at_once_wait || status=1
[ $status -eq 0 ] && echo "ok: every rewrite does what the game's code does" || echo "FAILED"
exit $status
