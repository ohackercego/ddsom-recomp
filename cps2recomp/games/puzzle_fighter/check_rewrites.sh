#!/bin/sh
# SPDX-License-Identifier: GPL-3.0-only
# Copyright (C) 2026 Tommy McLeroy
#
# Checks the game's rewritten routines (src/) against the code they replace: every call of each, over the
# game's movies, run both ways and compared (docs/READABLE.md). Inner rewrites are only called by
# outer ones, so each movie is run again with the outer ones left out (layers.sh), six runs at a time. Then each movie is run with the
# rewrites and with the game's own code throughout, and the two runs' states compared picture by picture, with the
# 68000 four times as fast as the board's (a rewrite is charged the average time of the code it replaces, not
# the exact time, so at the board's speed the two can come to be short of time in different frames): that catches a rewrite leaving something a caller reads in a register it was allowed not to match.
# Run from the repository's top.
GAME=puzzle_fighter
port=${B:-build/$GAME}/$GAME
movies=games/$GAME/movies
work=work/$GAME
status=0
# (challenger.txt: the same with the cheat that earns the hidden challenger, as far as the last opponent)
# (ladder.txt: one player through all nine opponents five times, with the cheat that wins every match (one that works from outside the rewrites))
ladder=free_play,quick_win
. games/$GAME/layers.sh
out=$work/rewrites; mkdir -p $out
run() { # layer's name, movie, frames, features, rewrites to leave out
    CPS2RECOMP_SKIP_REWRITES="$5" $port --rom "${ROM:-rom/$GAME}" --headless --frames "$3" --movie "$movies/$2.txt" --features "$4" --check-rewrites > $out/$1_$2.txt 2>&1 || touch $out/failed
}
whole() { # movie, frames, features
    $port --rom "${ROM:-rom/$GAME}" --headless --frames "$2" --movie "$movies/$1.txt" --features "$3" --cpu-speed 4 --hash-picture --hash-log $work/hash_rewrites.txt > /dev/null 2>&1
    $port --rom "${ROM:-rom/$GAME}" --headless --frames "$2" --movie "$movies/$1.txt" --features "$3" --cpu-speed 4 --hash-picture --hash-log $work/hash_own.txt --no-rewrites > /dev/null 2>&1
    # (A frame or two may differ in passing: the picture is taken at a fixed moment, and where the game is
    # changing the screen just then, a rewrite's time being an average shows for that one frame. Anything a
    # rewrite does wrong stays wrong, so more than kPassing frames, or a different end, fails.)
    differing=$(paste -d' ' $work/hash_rewrites.txt $work/hash_own.txt | awk '$2 != $4 {n++} END {print n + 0}')
    if cmp -s $work/hash_rewrites.txt $work/hash_own.txt; then
        echo "== $1: with the rewrites as with the game's own code, $2 frames"
    elif [ "$differing" -le 3 ] && [ "$(tail -200 $work/hash_rewrites.txt | cksum)" = "$(tail -200 $work/hash_own.txt | cksum)" ]; then
        echo "== $1: with the rewrites as with the game's own code, $2 frames (but for $differing frames' pictures in passing)"
    else
        echo "== $1: DIFFERS from the game's own code at line $(cmp $work/hash_rewrites.txt $work/hash_own.txt | sed 's/.*line //')"
        status=1
    fi
}
rm -f $out/*
n=0
for layer in all round round2 turns turn steps inner inmost; do
    [ $layer = all ] && skip="" || eval skip=\$$layer
    run $layer versus 20000 "" "$skip" &
    run $layer one_player 30000 "" "$skip" &
    run $layer ladder 66000 "$ladder" "$skip" &
    run $layer challenger 16000 "$ladder,challenger" "$skip" &
    n=$((n + 1)); [ $((n % 2)) -eq 0 ] && wait
done
wait
# (the screens before a match, select.cpp: movies made by hand for what the others do not reach there. two:
# two players chosen with the cursor, the cursor all over, the same character twice, handicap levels. codes
# and timed: the three codes for the hidden characters. wait: every choice runs out of time, and the match is
# lost and the game over (screens.cpp). join: the hard
# level, and a second player who comes into the match.)
selects="two:3000 codes:2700 timed:2800 wait:9200 join:4200"
for movie in $selects; do
    run all select_${movie%%:*} ${movie##*:} "" "" &
    run before select_${movie%%:*} ${movie##*:} "" "$before" &
done
run before ladder 66000 "$ladder" "$before" &
run before versus 20000 "" "$before" &
wait
others=$(grep -h -o 'AddRewrite(0x[0-9A-Fa-f]*, "[A-Za-z0-9]*"' games/$GAME/src/*.cpp | sed 's/.*"\(.*\)"/\1/' | grep -v -x -E "$common" | tr '\n' ',')
run common ladder 66000 "$ladder" "$others" &
run common one_player 30000 "" "$others" &
run common versus 20000 "" "$others" &
run common select_wait 9200 "" "$others" &
wait
others="$others$(echo $tasks | tr '|' ',')"
run common2 ladder 66000 "$ladder" "$others" &
run common2 one_player 30000 "" "$others" &
run common2 versus 20000 "" "$others" &
run common2 select_wait 9200 "" "$others" &
wait
[ -e $out/failed ] && status=1
grep -h -E "differs|did not" $out/*.txt | cut -c1-260 | head -20
cat $out/*.txt | grep "calls," | awk '{c[$2]+=$3; d[$2]+=$5} END {for (k in c) printf "%-28s %9d calls, %d differed\n", k, c[k], d[k]}' | sort
whole versus 20000 ""
whole one_player 30000 ""
whole ladder 66000 "$ladder"
whole challenger 16000 "$ladder,challenger"
for movie in $selects; do
    whole select_${movie%%:*} ${movie##*:} ""
done
[ $status -eq 0 ] && echo "ok: every rewrite does what the game's code does" || echo "FAILED"
exit $status
