#!/bin/sh
# SPDX-License-Identifier: GPL-3.0-only
# Copyright (C) 2026 Tommy McLeroy. Part of cps2recomp.
#
# A release's text: what is in the release (the paragraph a version's section of CHANGELOG.md begins with,
# before its list), what to download, how to install it on each system, and the rest of that section. Written by .github/workflows/release.yml for the draft (docs/RELEASING.md).
#   tools/release/notes.sh 0.5.0 > notes.md
set -eu
ROOT="$(cd "$(dirname "$0")/../.." && pwd)"
VERSION=${1:?usage: notes.sh VERSION}
changes=$(awk -v v="## $VERSION" '$0 ~ "^## " { on = (index($0, v) == 1); next } on' "$ROOT/CHANGELOG.md")
if [ -z "$changes" ]; then
    echo "error: CHANGELOG.md has no section for $VERSION" >&2
    exit 1
fi
# The section's opening paragraph (everything before its first "- " line), and the list after it
summary=$(printf '%s\n' "$changes" | awk '/^- / { exit } { line[n++] = $0 }
    END { s = 0; while (s < n && line[s] == "") s++; e = n - 1; while (e >= s && line[e] == "") e--
          for (i = s; i <= e; i++) print line[i] }')
list=$(printf '%s\n' "$changes" | awk '/^- / { on = 1 } on')
if [ -z "$summary" ]; then
    echo "error: CHANGELOG.md's section for $VERSION has no paragraph before its list (what is in the release, for" >&2
    echo "the top of the release's page)" >&2
    exit 1
fi
cat <<EOF
$summary

Unofficial native ports of Capcom's CPS-2 (and CPS-1) arcade games, in one download with one launcher. **No game
is included: you need your own ROM sets** ([which ones](https://github.com/tmcleroy/cps2recomp#what-you-need)).

## Download

| Your system | Download |
| --- | --- |
| macOS 13 or later, Apple silicon or Intel | **CPS2Recomp-macOS.zip** |
| Windows 10 or 11, 64-bit | **CPS2Recomp-windows-x64.zip** |
| Linux x86_64, Steam Deck, Steam Machine | **CPS2Recomp-linux-x86_64.tar.gz** |

\`SHA256SUMS.txt\` has the downloads' checksums. The \`update-*.txt\` files are what the launcher reads to update
itself; you don't need them.

## Install

### macOS
1. Open the zip: it gives **CPS-2 Recomp.app**. Move it to your **Applications** folder (it updates itself, so it
   must be somewhere it can write, not left in Downloads' unzipped folder).
2. The app is not notarized by Apple, so the first time: right-click it and choose **Open**, then **Open**. If
   macOS only offers **Done** or **Move to Trash**, choose **Done**, then open **System Settings → Privacy &
   Security**, scroll down and press **Open Anyway** next to "CPS-2 Recomp".
3. On the **Games** page, press **Choose folder…** and pick the folder with your ROM sets. Press **Play** beside a
   game.

### Windows
1. Right-click the zip, choose **Extract All…**, and extract it to a folder of your own (for example
   \`Documents\\Games\`), not Program Files: the launcher updates itself and needs to write there.
2. In the \`CPS2Recomp\` folder, open **cps2recomp-launcher.exe**. The programs are not signed, so the first time
   Windows may say it protected your PC: press **More info**, then **Run anyway**.
3. On the **Games** page, press **Choose folder…** and pick the folder with your ROM sets. Press **Play** beside a
   game.

### Linux
1. Unpack it where it can stay: \`tar xzf CPS2Recomp-linux-x86_64.tar.gz -C ~/Applications\` (any folder of your
   own works).
2. Run \`~/Applications/CPS2Recomp/cps2recomp-launcher\`. It needs only an X11 or Wayland desktop and the usual
   sound libraries (SDL 3 is built in).
3. On the **Games** page, press **Choose folder…** and pick the folder with your ROM sets. Press **Play** beside a
   game.

### Steam Deck and Steam Machine
1. Hold the power button and choose **Switch to Desktop**.
2. Download the Linux file. In the file manager (Dolphin), right-click it, choose **Extract → Extract archive
   here**, and move the \`CPS2Recomp\` folder where it can stay (for example a new \`Applications\` folder in your
   home folder).
3. Put your ROM sets in one folder on the device (from a USB stick or the SD card, say).
4. In the \`CPS2Recomp\` folder, open **cps2recomp-launcher**. On the **Games** page, press **Choose folder…** and
   pick that folder.
5. On each game's page, press **Add to Steam**. It adds the game to your library, and the first time also
   **CPS-2 Recomp** (the launcher).
6. Go back with **Return to Gaming Mode** on the desktop. The games are in your Library under **Non-Steam**.

In a game, Esc or F1 (on a gamepad, a click of the left stick, or Start and Select together) opens the in-game
menu. Its **Quit game** row ends the game.

## Update

The launcher checks for a new version when it opens and offers it at the top of the **Games** page: **Update**
downloads it, checks it and starts the new version. Or download this release and put it in place of the old app
or folder. Your settings, save states and ROM sets are kept either way: they are in your own user folder.

## What's new in $VERSION

$list
EOF
