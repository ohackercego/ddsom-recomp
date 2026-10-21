#!/bin/sh
# SPDX-License-Identifier: GPL-3.0-only
# Copyright (C) 2026 Tommy McLeroy
#
# Checks the widescreen feature (src/widescreen.cpp). On every stage a training session is walked to both of the
# stage's ends with widescreen on, and the wide picture's outer 64 columns are looked at there: a column of a plane the
# game never painted shows as a run of black down it. The columns with 40 or more black pixels running down them (above
# the floor) are counted at each end, and more than kMost of them fails. With invisible walls (widescreen_walls) the
# game must be the board's: the ladder is played with widescreen and the walls on and with neither, and the game's
# memory is compared every 4,000 frames (the frame hash would not do: it takes the picture in, which is meant to
# differ). The ladder is played with widescreen on, and no instruction may be left to the interpreter. Run from the
# repository's top.
GAME=pocket_fighter
port=${B:-build/$GAME}/$GAME
status=0
dir=$(mktemp -d)
for stage in 1 2 3 4 5 6 7 8; do
    POCKET_FIGHTER_HOME=$dir/home_s$stage $port --rom "${ROM:-rom/$GAME}" --headless --frames 3300 --features free_play,widescreen \
        --movie games/$GAME/movies/corners.txt --keys "3:ftraining_stage=$stage,5:ctraining_start" --shot-every 1100:$dir/s${stage}_ > /dev/null 2>&1 &
done
wait
python3 - "$dir" <<'PY' || status=1
import struct, sys, zlib
kMost = 20  # columns
def png(path):
    d = open(path, 'rb').read(); i = 8; idat = b''
    while i < len(d):
        n = struct.unpack('>I', d[i:i + 4])[0]; t = d[i + 4:i + 8]; c = d[i + 8:i + 8 + n]
        if t == b'IHDR': w, h, _, kind = struct.unpack('>IIBB', c[:10])
        if t == b'IDAT': idat += c
        i += 12 + n
    raw = zlib.decompress(idat); bpp = 3 if kind == 2 else 4; stride = w * bpp; rows = []; prev = bytearray(stride)
    for y in range(h):
        f = raw[y * (stride + 1)]; line = bytearray(raw[y * (stride + 1) + 1:(y + 1) * (stride + 1)])
        for x in range(stride):
            a = line[x - bpp] if x >= bpp else 0; b = prev[x]; c = prev[x - bpp] if x >= bpp else 0
            if f == 1: line[x] = (line[x] + a) & 255
            elif f == 2: line[x] = (line[x] + b) & 255
            elif f == 3: line[x] = (line[x] + (a + b) // 2) & 255
            elif f == 4:
                p = a + b - c; pa, pb, pc = abs(p - a), abs(p - b), abs(p - c)
                line[x] = (line[x] + (a if pa <= pb and pa <= pc else b if pb <= pc else c)) & 255
        rows.append(line); prev = line
    return w, bpp, rows
bad = 0
for stage in range(1, 9):
    shares = []
    for frame, side in ((2200, 0), (3300, 1)):
        w, bpp, rows = png('%s/s%d_%06d.png' % (sys.argv[1], stage, frame))
        if w != 512:
            print('stage %d: the picture is %d wide, not 512' % (stage, w)); bad += 1; shares.append(0); continue
        x0 = 0 if side == 0 else w - 64
        columns = 0
        for x in range(x0, x0 + 64):
            run = best = 0
            for y in range(24, 180):
                run = run + 1 if sum(rows[y][x * bpp:x * bpp + 3]) < 12 else 0
                best = max(best, run)
            columns += best >= 40
        shares.append(columns)
    print('stage %d: unpainted columns at the left end %2d, at the right end %2d%s' % (stage, shares[0], shares[1], '  TOO MANY' if max(shares) > kMost else ''))
    bad += max(shares) > kMost
sys.exit(1 if bad else 0)
PY
# Invisible walls: the game as the board plays it
for run in walls none; do
    features=free_play,no_damage,one_hit
    [ $run = walls ] && features=$features,widescreen,widescreen_walls
    dumps=""
    for f in 4000 8000 12000 16000 20000 24000 28000 32000 36000; do dumps="$dumps --dump-ram $f:$dir/${run}_$f"; done
    POCKET_FIGHTER_HOME=$dir/home_$run $port --rom "${ROM:-rom/$GAME}" --headless --frames 40000 --features $features \
        --movie games/$GAME/movies/ladder.txt $dumps > /dev/null 2>&1 &
done
wait
same=yes
for f in 4000 8000 12000 16000 20000 24000 28000 32000 36000; do
    cmp -s $dir/walls_$f $dir/none_$f || { echo "invisible walls: the game's memory differs at frame $f"; same=no; status=1; }
done
[ $same = yes ] && echo "invisible walls: the game's memory is the board's at every 4,000 frames of the ladder"
# The ladder with widescreen on: no instruction to the interpreter
out=$(POCKET_FIGHTER_HOME=$dir/home_w $port --rom "${ROM:-rom/$GAME}" --headless --frames 40000 --features free_play,no_damage,one_hit,widescreen \
    --movie games/$GAME/movies/ladder.txt --stats 2>&1)
echo "$out" | grep "ran in the interpreter"
echo "$out" | grep -q "^0 instructions had no translation and ran in the interpreter" || status=1
rm -rf "$dir"
[ $status -eq 0 ] && echo "ok: widescreen" || echo "FAILED"
exit $status
