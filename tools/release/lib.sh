# SPDX-License-Identifier: GPL-3.0-only
# Copyright (C) 2026 Tommy McLeroy. Part of cps2recomp.
#
# Shared by the release scripts (sourced, not run). A release is the collection: its launcher and every game's
# program (games/*), as one download for each system. A game's program holds the game's program translated to
# C++, as a build from source does: the generated code in work/<game>/gen, made by `make GAME=<id>` and
# `make GAME=<id> explore` from the builder's own ROM set. So releases are built on the machine of someone who
# has every game, never by CI. With NO_GAME_CODE=1 the programs are built with no translated code instead (they
# run a ROM set in the port's interpreters): that is what CI builds, to prove everything compiles and starts on
# each system, and it is never what a release holds. GAMES="a b" builds those games only (a test; a release
# has them all).
ROOT="$(cd "$(dirname "$0")/../.." && pwd)"
GAMES="${GAMES:-$(cd "$ROOT/games" && for cfg in */game.cfg; do printf '%s ' "${cfg%/game.cfg}"; done)}"
VERSION="${VERSION:-$(cat "$ROOT/VERSION")}"
OUT="$ROOT/build/release"
DEPS="$ROOT/build/deps"
NAME="CPS-2 Recomp"       # the collection: its launcher's window, app and folder
EXE=cps2recomp            # its launcher: cps2recomp-launcher
ARCHIVE=CPS2Recomp        # the downloads: CPS2Recomp-macOS.zip and the rest
APP_ID=io.github.tmcleroy.cps2recomp
SDL_VERSION=3.4.16
SDL_SHA256=7322236cd12090c3eb40b9728be4d49c76f66ad17d04369584d4ecad5cf77c68
SDL_MINGW_SHA256=c7ef65bd72eabac6e5b535411dbd8d5824d0aab24fd62ff8812666b336f18a9c
sha256() { if command -v sha256sum >/dev/null; then sha256sum "$@"; else shasum -a 256 "$@"; fi; }
mkdir -p "$OUT" "$DEPS"

# SDL 3's source, checked against its published checksum: $DEPS/SDL3-$SDL_VERSION
sdl_source() {
    if [ ! -d "$DEPS/SDL3-$SDL_VERSION/src" ]; then
        curl -fsSL -o "$DEPS/SDL3-$SDL_VERSION.tar.gz" "https://github.com/libsdl-org/SDL/releases/download/release-$SDL_VERSION/SDL3-$SDL_VERSION.tar.gz"
        echo "$SDL_SHA256  $DEPS/SDL3-$SDL_VERSION.tar.gz" | sha256 -c - >/dev/null
        tar -xzf "$DEPS/SDL3-$SDL_VERSION.tar.gz" -C "$DEPS"
    fi
}
# SDL 3 as a static library in $DEPS/sdl-$1 (the rest: more arguments for CMake)
sdl_static() {
    name=$1; shift
    if [ ! -f "$DEPS/sdl-$name/lib/cmake/SDL3/SDL3Config.cmake" ]; then
        sdl_source
        cmake -S "$DEPS/SDL3-$SDL_VERSION" -B "$DEPS/sdl-$name-build" -G Ninja -DCMAKE_BUILD_TYPE=Release -DSDL_SHARED=OFF \
            -DSDL_STATIC=ON -DSDL_TEST_LIBRARY=OFF -DCMAKE_INSTALL_PREFIX="$DEPS/sdl-$name" "$@" >/dev/null
        cmake --build "$DEPS/sdl-$name-build" --target install >/dev/null
    fi
}
# Every game's program, in $1/game-<id>/, and the collection's launcher, in $1/launcher/ (the rest: more
# arguments for CMake)
build_programs() {
    build=$1; shift
    list=""
    for game in $GAMES; do
        generated="$ROOT/work/$game"
        if [ "${NO_GAME_CODE:-}" = 1 ]; then
            generated="$build/no-game-code-$game"
            mkdir -p "$generated/gen"
            (cd "$ROOT" && python3 tools/gamecfg.py --header "$generated/gen/game_info.h" "$game")
        elif ! ls "$generated"/gen/native_*.cpp >/dev/null 2>&1; then
            echo "error: no translated code for $game in $generated/gen: run 'make GAME=$game' and 'make GAME=$game explore' with its ROM set first" >&2
            exit 1
        fi
        cmake -S "$ROOT" -B "$build/game-$game" -G Ninja -DCMAKE_BUILD_TYPE=Release -DGAME="$game" \
            -DGAME_DIR="$ROOT/games/$game" -DWORK_DIR="$generated" "$@" | grep "cps2recomp:"
        cmake --build "$build/game-$game" --target "$game"
        list="$list${list:+;}$game"
    done
    cmake -S "$ROOT/launcher" -B "$build/launcher" -G Ninja -DCMAKE_BUILD_TYPE=Release -DCOLLECTION=ON -DGAMES="$list" \
        -DGAME_NAME="$NAME" -DGAME_EXE="$EXE" -DAPP_ID="$APP_ID" -DHOME_ENV=CPS2RECOMP_HOME \
        -DDISCLAIMER="Not affiliated with Capcom." -DCOLLECTION_VERSION="$VERSION" "$@" >/dev/null
    cmake --build "$build/launcher"
}
# Every game's program and the launcher, from $1 (build_programs') into $2 (the end of a program's name: $3)
copy_programs() {
    cp "$1/launcher/$EXE-launcher$3" "$2/"
    for game in $GAMES; do
        cp "$1/game-$game/$game$3" "$2/"
    done
}
check_archive() { # the archive
    if [ "${NO_GAME_CODE:-}" = 1 ]; then
        GAMES="$GAMES" "$ROOT/tools/release/check_archive.sh" --no-game-code "$1"
    else
        GAMES="$GAMES" "$ROOT/tools/release/check_archive.sh" "$1"
    fi
}
# What every archive carries beside the programs: $1 the folder, $2 the launcher's build folder
add_notices() {
    mkdir -p "$1/licenses"
    cp "$ROOT/LICENSE" "$1/LICENSE.txt"
    cp "$ROOT/THIRD_PARTY_NOTICES.md" "$ROOT/README.md" "$ROOT/CHANGELOG.md" "$1/"
    cp "$DEPS/SDL3-$SDL_VERSION/LICENSE.txt" "$1/licenses/SDL3.txt"
    cp "$2/_deps/imgui-src/LICENSE.txt" "$1/licenses/DearImGui.txt"
    cp "$(find "$2/_deps/inter-src" -iname 'LICENSE*' | head -1)" "$1/licenses/Inter-OFL.txt"
    # (the sound file decoders of the custom soundtrack: their licence statements, from the end of each file)
    { echo "stb_vorbis (third_party/stb_vorbis.c):"; sed -n '/^ALTERNATIVE A - MIT License/,$p' "$ROOT/third_party/stb_vorbis.c"
      for f in dr_flac dr_mp3 dr_wav; do echo; echo "$f (third_party/$f.h):"; sed -n '/^ALTERNATIVE 1 - Public Domain/,$p' "$ROOT/third_party/$f.h"; done
    } > "$1/licenses/sound-decoders.txt"
    printf 'Source for this version (GPL-3.0): https://github.com/tmcleroy/cps2recomp/tree/v%s\n' "$VERSION" > "$1/SOURCE.txt"
}
# What the launcher updates itself from (recomp-common/launcher/src/update.h): $OUT/update-$1.txt, the manifest of
# the archive $3 (its SHA-256 and size) and of every file in $2 (the app, or the folder at the archive's top) with
# its SHA-256 and size. Uploaded to the release beside the archives (docs/RELEASING.md); the launcher downloads the
# archive itself.
update_files() {
    platform=$1 top=$2 archive=$3
    manifest="$OUT/update-$platform.txt"
    printf 'recomp-update 2\nversion %s\n' "$VERSION" > "$manifest"
    printf 'archive %s %s %s\n' "$(sha256 "$archive" | cut -d' ' -f1)" "$(wc -c < "$archive" | tr -d ' ')" "$(basename "$archive")" >> "$manifest"
    printf 'top %s\n' "$(basename "$top")" >> "$manifest"
    (cd "$top" && find . -type f | sed 's|^\./||' | LC_ALL=C sort) | while IFS= read -r path; do
        mode=-
        [ -x "$top/$path" ] && mode=x
        printf 'file %s %s %s %s\n' "$(sha256 "$top/$path" | cut -d' ' -f1)" "$(wc -c < "$top/$path" | tr -d ' ')" "$mode" "$path" >> "$manifest"
    done
    echo "wrote $manifest ($(grep -c '^file ' "$manifest") files)"
}
