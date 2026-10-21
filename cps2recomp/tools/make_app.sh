#!/bin/sh
# SPDX-License-Identifier: GPL-3.0-only
# Copyright (C) 2026 Tommy McLeroy. Part of cps2recomp.
#
# Builds a macOS app bundle from the launcher and the port: make_app.sh GAME, or make_app.sh --collection
# (the collection's launcher with every game built beside it in build/collection: make collection-app).
# The bundle holds the launcher (what opens), the game programs and SDL3. No game data: the player
# points the launcher at their own ROM sets. A game program does contain code translated
# from the game: whether a bundle is given to others is the port owner's decision. (A release's app is
# made by tools/release/macos.sh, with SDL built in.)
set -e
CFG="python3 $(dirname "$0")/gamecfg.py"
if [ "$1" = "--collection" ]; then
    B=${B:-build/collection}
    NAME="CPS-2 Recomp"
    APP_ID=io.github.tmcleroy.cps2recomp
    LAUNCHER=cps2recomp-launcher
    PROGRAMS=$(cd "$B" && for cfg in ../../games/*/game.cfg; do g=$(basename "$(dirname "$cfg")"); [ -f "$g" ] && echo "$g"; done)
else
    GAME=$1
    B=${B:-build/$GAME}
    NAME=$($CFG name "$GAME")
    APP_ID=$($CFG app_id "$GAME")
    LAUNCHER=$GAME-launcher
    PROGRAMS=$GAME
fi
APP="$B/$NAME.app"

# (nothing to do when the bundle is newer than both programs and this script: `make` asks for it every time)
IN="$APP/Contents/MacOS"
fresh=1
for exe in $PROGRAMS $LAUNCHER; do
    { [ -f "$IN/$exe" ] && [ "$IN/$exe" -nt "$B/$exe" ] && [ "$IN/$exe" -nt "$0" ]; } || fresh=""
done
if [ -z "$FORCE" ] && [ -f "$APP/Contents/Info.plist" ] && [ -n "$fresh" ]; then
    echo "$APP is up to date"
    exit 0
fi

rm -rf "$APP"
mkdir -p "$APP/Contents/MacOS" "$APP/Contents/Frameworks" "$APP/Contents/Resources"
for exe in $LAUNCHER $PROGRAMS; do
    cp "$B/$exe" "$APP/Contents/MacOS/"
done

# SDL3 from Homebrew, carried inside the bundle
for exe in "$APP/Contents/MacOS/"*; do
    otool -L "$exe" | awk '/libSDL3/ {print $1}' | while read -r lib; do
        base=$(basename "$lib")
        if [ ! -f "$APP/Contents/Frameworks/$base" ]; then
            cp -L "$lib" "$APP/Contents/Frameworks/$base"
            chmod u+w "$APP/Contents/Frameworks/$base"
            install_name_tool -id "@executable_path/../Frameworks/$base" "$APP/Contents/Frameworks/$base"
        fi
        install_name_tool -change "$lib" "@executable_path/../Frameworks/$base" "$exe"
    done
done

cat > "$APP/Contents/Info.plist" <<PLIST
<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">
<plist version="1.0">
<dict>
    <key>CFBundleName</key><string>$NAME</string>
    <key>CFBundleDisplayName</key><string>$NAME</string>
    <key>CFBundleIdentifier</key><string>$APP_ID</string>
    <key>CFBundleExecutable</key><string>$LAUNCHER</string>
    <key>CFBundlePackageType</key><string>APPL</string>
    <key>CFBundleShortVersionString</key><string>0.1</string>
    <key>CFBundleVersion</key><string>1</string>
    <key>LSMinimumSystemVersion</key><string>13.0</string>
    <key>NSHighResolutionCapable</key><true/>
    <key>LSApplicationCategoryType</key><string>public.app-category.games</string>
</dict>
</plist>
PLIST

codesign --force --deep -s - "$APP" 2>/dev/null
echo "built $APP"
