#!/bin/sh
# SPDX-License-Identifier: GPL-3.0-only
# Copyright (C) 2026 Tommy McLeroy. Part of cps2recomp.
# Checks the repository's tracked files for things that must never be committed (CONTRIBUTING.md):
# game files, keys, anything made from a game, an emulator's sources, personal paths and addresses,
# large files; and that every file marked as another project's is listed in the notices. Run it
# before pushing (make audit); CI runs it on every push and pull request. Exits non-zero on a finding.
set -eu
cd "$(git rev-parse --show-toplevel)"
# (the list of the files: a temporary file's place is not always where the shell that writes it can
# see it, so it is kept under the repository's own work folder)
LIST="$PWD/work/audit-files.$$"
mkdir -p "$PWD/work"
AUDIT_PATTERN="$PWD/work/audit-pattern.$$"
AUDIT_FILES="$PWD/work/audit-files-list.$$"
trap 'rm -f "$LIST" "$AUDIT_PATTERN" "$AUDIT_FILES"' EXIT
git ls-files > "$LIST"

fail=0
check() { if [ -n "$2" ]; then echo "AUDIT FAIL: $1"; echo "$2" | head -10; fail=1; else echo "ok: $1"; fi; }
# (this script names the things it looks for, so it is not searched itself; nor are third_party/'s decoders, other
# projects' files as they publish them, which only the ones the notices list may be: checked below)
grep_files() { grep -v -e '^tools/audit\.sh$' -e '^third_party/' "$LIST" | tr '\n' '\0' | xargs -0 grep -nIE "$@" 2>/dev/null || true; }
# (the same, but for a pattern that needs the {n,} repetition: the grep of a Windows shell does not know
# it, and takes the pattern for the name of a file, so the audit would find nothing and say so.
# The helper is in the tools folder of the repository, which is where the audit is run from.
# The pattern goes through a file: a shell's xargs changes a pattern that has the characters it treats
# as its own, and the files are given in a list for the same reason.)
AUDIT_HELPER="$PWD/tools/find-hex.py"
hex_files() {
    printf '%s' "$1" > "$AUDIT_PATTERN"
    grep -v -e '^tools/audit\.sh$' -e '^third_party/' "$LIST" > "$AUDIT_FILES"
    python3 "$AUDIT_HELPER" --pattern-file "$AUDIT_PATTERN" "$AUDIT_FILES" 2>/dev/null || true
}

check "no game, key, save or archive file types" "$(python3 "$AUDIT_HELPER" --names '\.(zip|7z|rar|key|bin|rom|chd|state|sav|eep|nv|lst)$|(^|/)[a-z0-9]{2,5}\.[0-9]{2}[a-z]?m?$' "$LIST" || true)"
# (a game's movies are the player's key presses, as text: they are allowed, in games/<id>/movies/)
check "nothing made from a game (generated code, entry lists, listings, memory dumps)" "$(grep -E '(^|/)(work|build)/|(^|/)(native|z80)_[0-9]+\.cpp$|(^|/)(entries|guessed)[^/]*\.txt|(^|/)game_info\.h$|(^|/)gen/|\.(dump|ram)$' "$LIST" || true)"
check "movies only in a game's movies folder, and as text" "$(grep -E '(^|/)movies/' "$LIST" | grep -vE '^games/[a-z0-9_]+/movies/[a-z0-9_]+\.txt$' || true)"
check "every game has its game.cfg" "$(grep -E '^games/[^/]+/' "$LIST" | cut -d/ -f2 | sort -u | while read -r g; do [ -f "games/$g/game.cfg" ] || echo "games/$g"; done)"
check "no pictures but the README's, no sound or video" "$(grep -iE '\.(png|jpe?g|gif|bmp|ppm|webp|mp4|mov|webm|wav|ogg|mp3|flac)$' "$LIST" | grep -vE '^docs/screenshots/([a-z0-9_]+/)?[a-z0-9-]+\.(png|jpg|gif)$' || true)"
# (the published checksums of the libraries the builds download, and the hashes of the copied files, are not keys)
check "no key-like hex strings (32 digits or more)" "$(hex_files '(?<![0-9a-zA-Z_.])[0-9a-fA-F]{32,}(?![0-9a-zA-F])' | grep -vE 'URL_HASH SHA256=[0-9a-f]{64}|SHA256=[0-9a-f]{64}|= [0-9a-f]{64} \]|^recomp-common/COMMIT:|^\.github/workflows/|^games/[^/]+/movies/' || true)"
# (a CPS-2 board's key is two 32-bit numbers; a tool that wrote key files from them was taken out, and neither
# it nor its numbers may come back)
check "no board key written out, and no tool that writes key files" "$(hex_files 'makekey\.py[^|]*[ =](0x)?[0-9a-fA-F]{8}[ ,]+(0x)?[0-9a-fA-F]{8}\b' || true; grep -iE '(^|/)makekey' "$LIST" || true)"
check "no home folder paths" "$(grep_files -e '/Users/[a-z]' -e '/home/[a-z]+/' -e 'C:\\\\Users\\\\[a-z]' || true)"
check "no private session links or private network addresses" "$(grep_files -e 'claude\.ai/code/session' -e '\b192\.168\.[0-9]+\.[0-9]+' -e '\b10\.[0-9]+\.[0-9]+\.[0-9]+\b' || true)"
# (the README's pictures may be larger: a moving picture is a few megabytes)
# (the sound file decoders in third_party/ are kept as their projects publish them: dr_flac is a little over)
check "no file over 512 KB (the README's pictures: 6 MB; third_party/'s decoders: 1 MB)" "$( (grep -v -e '^docs/screenshots/' -e '^third_party/' "$LIST" | tr '\n' '\0' | xargs -0 find 2>/dev/null -prune -type f -size +512k; \
    grep '^third_party/' "$LIST" | tr '\n' '\0' | xargs -0 find 2>/dev/null -prune -type f -size +1024k; \
    grep '^docs/screenshots/' "$LIST" | tr '\n' '\0' | xargs -0 find 2>/dev/null -prune -type f -size +6144k) || true)"
# (ports that are not public yet are not named here)
check "no names of ports that are still private" "$(grep_files -i -e 'snesrecomp' -e 'fighting.?vipers' -e 'bonkers' -e 'muscle.?march-dev' || true)"
check "no words that point to where game files or keys are found" "$(grep_files -i -e 'archive\.org' -e 'rom ?sites?\b' -e 'emuparadise' -e 'vimm' -e 'romsfun' -e 'cps2crpt\.c' -e 'torrent' -e 'download (the|a|an) (rom|rom set|key|key file)' || true)"
check "no submodules" "$(git ls-files -s | awk '$1 == "160000" {print $4}'; [ ! -f .gitmodules ] || echo .gitmodules)"
check "no patch files" "$(grep -iE '\.(patch|diff)$|(^|/)patches/' "$LIST" || true)"
check "no emulator's sources in the tree" "$(grep -iE '(^|/)(third_party|vendor|external|deps)/.+/.+|(^|/)(mame|fbneo|fbalpha|musashi|libretro)[^/]*/' "$LIST" || true)"
# Files of another project's say so in their first lines; each must be one THIRD_PARTY_NOTICES.md lists
check "files of another project's are only the ones the notices list" "$(grep_files -l -e '^(//|#) *license: *BSD' -e 'SPDX-License-Identifier: (GPL-3\.0-or-later|BSD|MIT|Zlib)' \
    | grep -vE '^machine/cps2crypt\.cpp$|^recomp-common/launcher/src/(ui\.cpp|ui\.h|embedded\.h)$|^(THIRD_PARTY_NOTICES|CONTRIBUTING)\.md$' || true)"
# (third_party/'s files have their licences at their ends, and are named in the notices)
check "every source file has a license line" "$(grep -E '\.(h|c|cpp|inc|frag|vert|py|sh|mk|cmake|cfg)$|CMakeLists\.txt$|(^|/)Makefile$' "$LIST" | grep -v '^third_party/' | tr '\n' '\0' | xargs -0 grep -LiE 'SPDX-License-Identifier:|^// license:' 2>/dev/null || true)"
check "third_party/ holds only what the notices list" "$(grep '^third_party/' "$LIST" | while read -r f; do grep -qF "\`$f\`" THIRD_PARTY_NOTICES.md || echo "$f"; done)"
check "the README and the notices are there" "$(for f in README.md LICENSE THIRD_PARTY_NOTICES.md CONTRIBUTING.md CODE_OF_CONDUCT.md SECURITY.md CHANGELOG.md VERSION; do [ -f "$f" ] || echo "missing: $f"; done)"
[ "$fail" = 0 ] || { echo "audit failed"; exit 1; }
echo "audit passed ($(wc -l < "$LIST" | tr -d ' ') files)"
