#!/bin/sh
# SPDX-License-Identifier: GPL-3.0-only
# Copyright (C) 2026 Tommy McLeroy. Part of cps2recomp.
#
# Release build for Linux x86_64 (Steam Deck and Steam Machine included):
# build/release/CPS2Recomp-linux-x86_64.tar.gz (the launcher, every game's program, licences). Run on x86_64 Linux
# with the packages of tools/release/Dockerfile (docs/BUILDING.md), and the translated code of a build of each
# game with its ROM set (lib.sh). SDL 3 is built in, so the programs need only the system's usual X11 or Wayland,
# sound and zlib libraries.
set -eu
. "$(dirname "$0")/lib.sh"
BUILD="$ROOT/build/release-linux"
sdl_static linux
build_programs "$BUILD" -DCMAKE_PREFIX_PATH="$DEPS/sdl-linux" -DCMAKE_EXE_LINKER_FLAGS="-static-libstdc++ -static-libgcc"

STAGE="$BUILD/stage/$ARCHIVE"
rm -rf "$BUILD/stage" && mkdir -p "$STAGE"
copy_programs "$BUILD" "$STAGE" ""
strip "$STAGE/$EXE-launcher" $(for game in $GAMES; do echo "$STAGE/$game"; done)
add_notices "$STAGE" "$BUILD/launcher"
rm -f "$OUT/$ARCHIVE-linux-x86_64.tar.gz"
tar -czf "$OUT/$ARCHIVE-linux-x86_64.tar.gz" -C "$BUILD/stage" "$ARCHIVE"
check_archive "$OUT/$ARCHIVE-linux-x86_64.tar.gz"
update_files linux-x86_64 "$STAGE" "$OUT/$ARCHIVE-linux-x86_64.tar.gz"
echo "wrote $OUT/$ARCHIVE-linux-x86_64.tar.gz"
