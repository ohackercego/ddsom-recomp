#!/bin/sh
# SPDX-License-Identifier: GPL-3.0-only
# Copyright (C) 2026 Tommy McLeroy. Part of cps2recomp.
# Starts what 'make example' built, with no game and no window: the program lists its features (the
# stand-in game has none) and the launcher draws its pages to pictures. Used by CI and by 'make smoke'.
set -eu
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
B="$ROOT/example/build/example"
EXE=""
[ -f "$B/example.exe" ] && EXE=.exe
"$B/example$EXE" --list-features > /dev/null
echo "ok: the program starts"
OUT="$B/smoke"
rm -rf "$OUT" && mkdir -p "$OUT/home"
env SDL_VIDEO_DRIVER=dummy SDL_RENDER_DRIVER=software SDL_AUDIODRIVER=dummy "EXAMPLE_HOME=$OUT/home" \
    "$B/example-launcher$EXE" --screenshot "$OUT/page"
pages="$(ls "$OUT"/page*.png | wc -l | tr -d ' ')"
[ "$pages" -ge 3 ] || { echo "the launcher drew $pages pages, not 3"; exit 1; }
echo "ok: the launcher starts and draws its pages ($pages)"

