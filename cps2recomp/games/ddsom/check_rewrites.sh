#!/bin/sh
# SPDX-License-Identifier: GPL-3.0-only
# Copyright (C) 2026  Part of cps2recomp.
#
# Checks the game's rewritten routines (src/) against the code they replace: every call of each, over the
# game's movies, run both ways and compared (docs/READABLE.md). Run from the repository's top.
GAME=ddsom
port=${B:-build/$GAME}/$GAME
movies=games/$GAME/movies
status=0
# (the rom: the ROM variable when it is there, and the folder of the roms as it is when the script is
# run from the repository's top. The variable is read in the check itself, and not here, because a
# variable that a shell's parent sets is not always in the job the check runs in.)
rom_folder() {
    if [ -n "$ROM" ] && [ -d "$ROM" ]; then
        echo "$ROM"
        return 0
    fi
    for candidate in "$PWD/../../ddsom" "$PWD/../ddsom" "$PWD/rom/$GAME" "$PWD/../../rom/$GAME"; do
        if [ -d "$candidate" ]; then
            echo "$candidate"
            return 0
        fi
    done
    echo "${ROM:-rom/$GAME}"
}
. tools/at-once.sh
check() { # movie, frames, features, rewrites to leave out, more of the port's options
    out=$(CPS2RECOMP_SKIP_REWRITES="$4" $port --rom "$(rom_folder)" --headless --frames "$2" --movie "$movies/$1.txt" --features "$3" $5 --check-rewrites 2>&1)
    failed=$?
    echo "== $1${4:+ (without $4)}${5:+ ($5)}"
    echo "$out" | grep -E "^rewrite|did not" | grep -v " 0 calls"
    echo "$out" | grep -E "differs|calls, [1-9][0-9]* differed" > /dev/null && return 1
    return $failed
}
run() {
    at_once check "$@"
}
# (the board's settings are applied at the game's start, and again when the test menu is left)
run menu 1600 ""
run menu 1600 "board_players=2"
run menu 1600 "free_play"
# (the play: the four players in, and playing)
run play 6000 "board_players=2,free_play"
at_once_wait || status=1
at_once_clean
exit $status
