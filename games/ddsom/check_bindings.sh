#!/bin/sh
# SPDX-License-Identifier: GPL-3.0-only
# Copyright (C) 2026  Part of cps2recomp.
#
# Checks the four players' controls: the usual layout of the keys, and that no key is two players' at
# once (a key two of them share would do both things at the same time). The check reads the port's own
# table (port/bindings.h), so it is the table the game plays with. Run from the repository's top.
GAME=ddsom
port=${B:-build/$GAME}/$GAME
status=0
# (the repository's own folder: the compiler is run from the temporary folder, and the paths to the
# port's own sources are those of the repository. The compiler is a Windows program, so it is given
# the paths the way Windows names them)
REPO="$PWD"
REPO="$(cygpath -m "$REPO" 2>/dev/null || echo "$REPO")"

# (the table: the game's own (not the collection's), with the three buttons the game has)
build_check() {
    out=$(cd "$TMP_DIR_POSIX" && ${CXX:-g++} -std=c++20 -O0 \
        -I"${SDL3_INCLUDE:-C:/msys64/ucrt64/include}" \
        -I"$REPO/port" -I"$REPO/recomp-common/launcher/src" \
        "$REPO/games/$GAME/tests/bindings_test.cpp" -o bindings-test.exe \
        -L"${SDL3_LIB:-C:/msys64/ucrt64/lib}" -lSDL3 2>&1)
    if [ $? -ne 0 ]; then
        echo "$out"
        return 1
    fi
    "$TMP" 4 "Attack | Jump | Magic"
}

# (the test's own program: it is built in a temporary folder. The compiler here is a Windows program,
# so it is given a path in the way Windows names one (with /, not \). The folder the environment names
# may be named the way a shell names it, and the compiler would not know it. The shell is given the
# path the way a shell names it, and the compiler the path the way Windows names one)
TMP_DIR="${TMPDIR:-$PWD/work}"
TMP_DIR_POSIX="$(cygpath -u "$TMP_DIR" 2>/dev/null || echo "$TMP_DIR")"
TMP_DIR="$(cygpath -m "$TMP_DIR" 2>/dev/null || echo "$TMP_DIR")"
mkdir -p "$TMP_DIR_POSIX" 2>/dev/null
# (the compiler makes its own temporary files in the folder the environment names, joined with the name
# of the program it writes, so the program is written there under that name alone: a path of its own
# would be joined to the folder, and the two would be one name that is not there. The compiler is
# therefore run from that folder)
TMP="$TMP_DIR/bindings-test.exe"
export TMPDIR="$TMP_DIR"
trap 'rm -f "$TMP"' EXIT

check() {
    out=$(build_check)
    if [ $? -ne 0 ]; then
        echo "the table could not be built"
        echo "$out"
        return 1
    fi
    echo "$out"
    # (a key two players have at once is a conflict: the check prints them)
    if echo "$out" | grep -q "the same key for two players"; then
        echo "the check FAILED: a key is two players' at once"
        return 1
    fi
    # (every player has a key for each of the four ways, and one for start and one for a coin)
    for player in 1 2 3 4; do
        for action in up down left right start coin; do
            if ! echo "$out" | grep -A9 "^player $player:" | grep -qE "^  $action .+ \(none\)"; then
                continue
            fi
            echo "the check FAILED: the player $player has no key for $action"
            status=1
        done
    done
    return $status
}

check || status=1
if [ $status -eq 0 ]; then
    echo "ok: the four players' controls are there, and no key is two players' at once"
fi
exit $status
