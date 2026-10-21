# SPDX-License-Identifier: GPL-3.0-only
# Copyright (C) 2026 Tommy McLeroy. Part of cps2recomp.
#
# For a game's check scripts: run checks at once, a few at a time (JOBS, 8 unless said), and print what each wrote
# in the order they were started. Sourced, not run:
#
#   . cps2recomp/tools/at-once.sh
#   at_once some_check its arguments...   (a function or a program: what it writes and whether it failed are kept)
#   at_once_wait || status=1              (prints them all, in order; fails if any did)
# (the folder the checks' own words are kept in. A temporary folder in /tmp is not always where the
# shell that writes there can see it (on Windows, the mktemp of one shell and the shell that writes
# do not always share a /tmp), so the folder is made here, under the repository's own work folder.)
AT_ONCE_DIR=${AT_ONCE_DIR:-"$PWD/work/at-once.$$"}
mkdir -p "$AT_ONCE_DIR" 2>/dev/null || AT_ONCE_DIR="$PWD/.at-once.$$"
mkdir -p "$AT_ONCE_DIR" 2>/dev/null
# (the folder is emptied first: a folder left by an earlier run would have the old checks' words in
# it, and the run would read them as its own)
rm -f "$AT_ONCE_DIR"/* 2>/dev/null
AT_ONCE_N=0
at_once() {
    AT_ONCE_N=$((AT_ONCE_N + 1))
    while [ "$(jobs -r | wc -l)" -ge "${JOBS:-8}" ]; do sleep 1; done
    mkdir -p "$AT_ONCE_DIR" 2>/dev/null
    (
        "$@" > "$AT_ONCE_DIR/$AT_ONCE_N.out" 2>&1 || touch "$AT_ONCE_DIR/$AT_ONCE_N.failed"
    ) &
}
at_once_wait() {
    wait
    at_once_i=1
    at_once_bad=0
    while [ $at_once_i -le $AT_ONCE_N ]; do
        cat "$AT_ONCE_DIR/$at_once_i.out" 2>/dev/null
        [ -e "$AT_ONCE_DIR/$at_once_i.failed" ] && at_once_bad=1
        at_once_i=$((at_once_i + 1))
    done
    rm -rf "$AT_ONCE_DIR"
    AT_ONCE_N=0
    return $at_once_bad
}
# (the folder the checks wrote in, when the script is left before at_once_wait)
at_once_clean() { rm -rf "$AT_ONCE_DIR"; }
