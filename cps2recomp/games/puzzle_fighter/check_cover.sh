#!/bin/sh
# SPDX-License-Identifier: GPL-3.0-only
# Copyright (C) 2026 Tommy McLeroy
#
# Checks the rewrites over long runs of made-up play (more of the rules' rarer cases than the movies reach):
#   check_cover.sh [RUNS [FRAMES]]     from the repository's top; a few minutes
GAME=puzzle_fighter
port=${B:-build/$GAME}/$GAME
out=work/$GAME/cover
runs=${1:-8}; frames=${2:-60000}
mkdir -p $out
rm -f $out/*.txt
one() { # seed, layer's name, rewrites to leave out
    CPS2RECOMP_SKIP_REWRITES="$3" $port --rom "${ROM:-rom/$GAME}" --headless --frames $frames --auto $1:400 --check-rewrites > $out/$2_$1.txt 2>&1
}
. games/$GAME/layers.sh
# (half the runs with only two colours of gems dealt, for big gems and chains: pair.cpp)
for seed in $(seq 1 $runs); do
    [ $((seed % 2)) -eq 0 ] && export SPF2_TEST_COLOURS=2 || unset SPF2_TEST_COLOURS
    (for layer in round turns turn steps inner inmost; do
         eval skip=\$$layer
         one $seed $layer "$skip"
     done) &
    [ $((seed % 6)) -eq 0 ] && wait
done
wait
# (one player against the computer, made to plan as each of the characters that score every place for a pair
# (computer.cpp: SeekerPlans); the made-up play meets only the first opponents otherwise)
n=0
for character in 00 02 05 08 09 05; do
    n=$((n + 1))
    [ $((n % 2)) -eq 0 ] && export SPF2_TEST_COLOURS=2 || unset SPF2_TEST_COLOURS
    CPS2RECOMP_SKIP_REWRITES="$inner" $port --rom "${ROM:-rom/$GAME}" --headless --frames $frames --auto $n:400:1 --features free_play \
        --poke 8782=$character --check-rewrites > $out/seeker_$n.txt 2>&1 &
done
wait
cat $out/*.txt | grep "^rewrite" | grep "differs" | cut -c1-260 | head -20
cat $out/*.txt | grep "calls," | awk '{c[$2]+=$3; d[$2]+=$5} END {for (k in c) printf "%-24s %9d calls, %d differed\n", k, c[k], d[k]}' | sort
