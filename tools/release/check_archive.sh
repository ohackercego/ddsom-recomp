#!/bin/sh
# SPDX-License-Identifier: GPL-3.0-only
# Copyright (C) 2026 Tommy McLeroy. Part of cps2recomp.
#
# Fails if a release archive holds anything it must not (game files, a builder's home path) or lacks
# what it must carry (the programs, the licences). A release's game program holds the translated code;
# an archive made with NO_GAME_CODE=1 (CI's) does not, and --no-game-code checks that kind instead, so
# that neither is taken for the other. Every game (games/*, or GAMES="a b") must have its program in it.
#   check_archive.sh [--no-game-code] ARCHIVE...
set -eu
ROOT="$(cd "$(dirname "$0")/../.." && pwd)"
GAMES="${GAMES:-$(cd "$ROOT/games" && for cfg in */game.cfg; do printf '%s ' "${cfg%/game.cfg}"; done)}"
status=0
kind=release
if [ "${1:-}" = "--no-game-code" ]; then kind=nocode; shift; fi
bad() { echo "ARCHIVE FAIL ($archive): $1"; status=1; }
for archive in "$@"; do
    dir="$(mktemp -d)"
    case "$archive" in
        *.zip) unzip -q "$archive" -d "$dir" ;;
        *) tar -xzf "$archive" -C "$dir" ;;
    esac
    names="$(cd "$dir" && find . -type f | sort)"
    found="$(echo "$names" | grep -iE '\.(zip|7z|key|bin|rom|state|sav|eep|lst)$|/[a-z0-9]{2,5}\.[0-9]{2}[a-z]?m?$' || true)"
    [ -z "$found" ] || bad "game or data files: $found"
    for need in LICENSE.txt THIRD_PARTY_NOTICES.md licenses/SDL3.txt licenses/DearImGui.txt licenses/Inter-OFL.txt licenses/sound-decoders.txt SOURCE.txt; do
        echo "$names" | grep -q "/$need\$" || bad "missing $need"
    done
    # A game's program: the port, its interpreters and SDL come to 3 MB or so for each processor it is built
    # for, and the game's translated code adds 3 to 10 MB or more for each. So its size says which it is: 8 MB,
    # unless the game's game.cfg says otherwise (release_size, release_size_windows).
    : > "$dir/.programs"
    for game in $GAMES; do
        find "$dir" -type f \( -name "$game" -o -name "$game.exe" \) > "$dir/.one"
        [ -s "$dir/.one" ] || bad "no program for $game"
        cat "$dir/.one" >> "$dir/.programs"
    done
    find "$dir" -type f \( -name 'cps2recomp-launcher' -o -name 'cps2recomp-launcher.exe' \) | grep -q . || bad "no launcher"
    while IFS= read -r program; do
        size=$(wc -c < "$program" | tr -d ' ')
        game=$(basename "$program" .exe)
        case "$program" in
            *.exe) line=$(cd "$ROOT" && python3 tools/gamecfg.py release_size_windows "$game") ;;
            *) line=$(cd "$ROOT" && python3 tools/gamecfg.py release_size "$game") ;;
        esac
        line=${line:-8000000}
        if [ "$kind" = release ]; then
            [ "$size" -ge "$line" ] || bad "$(basename "$program") is $size bytes: it has no translated code (a CI build, not a release)"
        else
            [ "$size" -lt "$line" ] || bad "$(basename "$program") is $size bytes: it holds translated code"
        fi
    done < "$dir/.programs"
    paths="$(cd "$dir" && grep -rlaE '/Users/[a-z]|/home/[a-z]+/|C:\\Users\\' . || true)"
    [ -z "$paths" ] || bad "a builder's home path inside: $paths"
    rm -rf "$dir"
    [ $status -ne 0 ] || echo "ok: $archive"
done
exit $status
