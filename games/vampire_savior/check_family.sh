#!/bin/sh
# SPDX-License-Identifier: GPL-3.0-only
# Copyright (C) 2026 Tommy McLeroy
#
# Checks Vampire Savior's rewritten routines (src/) in a game made from its program that builds them too
# (Vampire Savior 2, Vampire Hunter 2): every call of each, run both ways and compared (docs/READABLE.md), over
# made-up play (one player and two, from the attract mode with free play) and a training session on each of
# the game's stages with the players moved by made-up controls. Inner rewrites are only called by outer ones,
# so each run is made again with the outer ones left out. Run from the repository's top: check_family.sh GAME
GAME=$1
port=${B:-build/$GAME}/$GAME
status=0
. tools/at-once.sh # (the runs at once, a few at a time; what each wrote printed in order)
eeprom=$(mktemp -d)
play() { # seed, players, frames, rewrites to leave out
    out=$(CPS2RECOMP_SKIP_REWRITES="$4" $port --rom "${ROM:-rom/$GAME}" --eeprom "$eeprom/play-$1-$2-$$.bin" --headless --frames "$3" \
        --auto "$1:400:$2" --features free_play --check-rewrites 2>&1)
    failed=$?
    echo "== made-up play, seed $1, $2 player(s)${4:+ (without $4)}"
    out=$(echo "$out" | grep -E "^rewrite|did not" | grep -v " 0 calls")
    echo "$out"
    echo "$out" | grep -E "differs|did not|calls, [1-9][0-9]* differed" >/dev/null && return 1
    return $failed
}
inPicture="DrawnIfInWide,DrawnIfInPicture,ThrownGone,PieceByCamera,KeepIn,KeepBothIn,AllPieces"
scripts="PiecePlain,PieceFlickering,PieceDancers,PieceLady,PieceThrower"
for skip in "" "AllHitTests,CameraAcross,CameraUp,$inPicture" "$inPicture,Drawn,$scripts" "AllHitTests,PlayersAgainstPlayers,ThrownAgainstPlayers,GuardRange" \
    "AllHitTests,PlayersAgainstPlayers,ThrownAgainstPlayers,HitTest,AttackAgainst"; do
    at_once play 3 2 10000 "$skip"
    at_once play 5 1 12000 "$skip"
done
check_stage() { # stage, rewrites to leave out, more of the port's options
    out=$(CPS2RECOMP_SKIP_REWRITES="$2" $port --rom "${ROM:-rom/$GAME}" --eeprom "$eeprom/stage-$1-$$.bin" --headless --frames 6000 \
        --auto 7:1500 --keys "5:ftraining_stage=$1,10:ctraining_start" $3 --check-rewrites 2>&1)
    echo "== stage $1${2:+ (without the outer ones)}${3:+ ($3)}"
    # (training's own changes to the round's rules differ from the game's, as they are meant to: only the
    # stage's rewrites are looked at)
    out=$(echo "$out" | grep -E "^rewrite|did not" | grep -E "Camera|Players(Across|Up)|Back8|Far32|Rows|Drawn|Piece|Script[0-9]|Plane(Of|AndRow)|PlaceOf|Animat|GiveBack|ThrownGone|KeepIn|KeepBothIn|did not" | grep -v " 0 calls")
    echo "$out"
    echo "$out" | grep -q "CameraPlaced" || return 1
    echo "$out" | grep -E "differs|did not|calls, [1-9][0-9]* differed" >/dev/null && return 1
    return 0
}
outer=$(grep -oE '"(Back8|Far32)(Across|Up)[A-Za-z0-9]*"' games/vampire_savior/src/planes.cpp | tr -d '"' | paste -sd, -)
# (the stages: the choices of the training session's "Stage", but for the game's own)
stages=$(($($port --list-features 2>/dev/null | grep "^training_stage" | cut -f6 | tr '|' '\n' | wc -l) - 1))
n=1
while [ $n -le "$stages" ]; do
    at_once check_stage $n ""
    at_once check_stage $n "CameraAcross,CameraUp,$outer,$inPicture"
    at_once check_stage $n "CameraAcross,CameraUp,$outer,$inPicture,$scripts,Drawn,SetStageAnimation"
    at_once check_stage $n "CameraAcross,CameraUp,$outer,$inPicture" "--poke FF8107=01"
    n=$((n + 1))
done
at_once_wait || status=1
rm -rf "$eeprom"
[ $status -eq 0 ] && echo "ok: every rewrite does what $GAME's code does" || echo "FAILED"
exit $status
