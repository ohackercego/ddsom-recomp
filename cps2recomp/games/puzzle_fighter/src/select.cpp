// SPDX-License-Identifier: GPL-3.0-only
// Copyright (C) 2026 Tommy McLeroy
//
// Super Puzzle Fighter II Turbo: what comes before a match, written again as C++ (rewrites: lift.h): the
// choice of mode (against the computer, or two players), of level (easy, normal, hard), of characters, and
// the ladder of opponents shown with the next one picked out.
//
// kMatchStep says where it is: set up, choosing, leaving. While choosing, kRoundStep is the screen (kScreen...)
// and kSubStep the screen's own step. The screens' moving parts (the words that slide in, the cursors, the
// portraits' frames) are tasks, which this code starts and which are not written again; they say they are
// done by counting kTasksDone up, or setting it to kTasksGone when they have left the screen.
//
// A player chooses with the stick among eight characters, four to a row, and a button takes the one the
// cursor is on (the second button: in its other colours). Three more can be had by a code (HiddenChoice).
#include "spf2.h"

#include <utility>

namespace cps2recomp {

namespace {
    using namespace spf2;

    // --- the game's variables used here ---
    enum : uint16_t { kStepSetUp = 0, kStepChoosing = 2, kStepLeaving = 4 }; // kMatchStep
    enum : uint16_t { // kRoundStep, while choosing
        kScreenMode = 0, kScreenLevel = 2, kScreenCharacter = 4, kScreenCharacters = 6, kScreenJoining = 8, kScreenOpponent = 0x0A,
    };
    constexpr uint32_t kTimer = kVars + 0x12;       // word: frames the step still has
    constexpr uint32_t kSeconds = kVars + 0x16, kSecondFrames = kVars + 0x17; // bytes: a choice's time (decimal digits), and frames of its second
    constexpr uint8_t kChoiceSeconds = 0x19, kHandicapSeconds = 0x09, kFramesASecond = 60;
    constexpr uint32_t kTasksDone = 0xFF0C8A;       // byte
    constexpr uint8_t kTasksGone = 0xFF;
    constexpr uint32_t kLevelCursor = 0xFF0C8C;     // byte: 0 easy, 1 normal, 2 hard
    constexpr uint32_t kPlayersSeen = 0xFF0C8E;     // byte: kChoosing as it was when the screen came up
    constexpr uint32_t kPortraitsStay = 0xFF0C8F;   // byte: given to the tasks that frame the portraits
    constexpr uint32_t kOpponentShowing = 0xFF0C90, kBackdropOn = 0xFF0C91; // bytes
    constexpr uint32_t kLadderPage = 0xFF0C92;      // word: which of the layer's two pages the ladder is drawn in
    constexpr uint32_t kHelpText = 0xFF0C94;        // word: the line of help shown (kNoHelp: none)
    constexpr uint16_t kNoHelp = 0xFFFF;
    constexpr uint32_t kLadderMark = 0xFF0C96;      // long: where a beaten opponent's mark goes
    // What each player has on the screen: the character, the cursor's place (0-7), the handicap level (0-4)
    constexpr uint32_t kChosen[2] = {0xFF0C82, 0xFF0C86};
    constexpr uint32_t kCursor = 1, kHandicapChosen = 2;
    constexpr uint32_t kOrder = 0xFF0CA6;           // the ladder's order: a character a stage (match.cpp)
    constexpr uint32_t kChoosing = kVars + 0x8D;    // byte: the players choosing now (bit 0 player 1, bit 1 player 2)
    constexpr uint32_t kPlaying = kVars + 0xAC;     // byte: the players already in the game
        constexpr uint32_t kHandicaps = kVars + 0xBB;   // byte: not zero when the players choose handicap levels
    constexpr uint32_t kShortLadder = kVars + 0xBF; // byte: the ladder is a stage shorter
    constexpr uint32_t kDebug = kVars + 0x85;       // byte: left over from the game's making; nothing sets it
    constexpr uint32_t kOtherLanguage = kVars + 0x92; // word: the help comes from another list
    constexpr uint32_t kStage = kVars + 0x112, kLastStage = kVars + 0x114, kOpponentsMet = kVars + 0x110; // words (match.cpp)
    constexpr uint16_t kLastStageNumber = 8;
    constexpr uint32_t kLastOpponentIn = kVars + 0x138; // byte
    constexpr uint32_t kHeld1 = kVars + 0x8F, kHeld2 = kVars + 0xBD; // bytes: set while the screen changes
    // For the video hardware: words the frame interrupt hands on
    constexpr uint32_t kScreenSetting = kVars + 0x20, kLayerOrder = kVars + 0x32, kLayerSetting = kVars + 0x3A;
    constexpr uint32_t kScrollX = kVars + 0x26, kScrollY = kVars + 0x28; // the backdrop's place
    constexpr uint16_t kScreenPlain = 0x90C0, kScreenPortraits = 0x9280;

    // --- the two characters on the screen (A6 in the game's code): records like the players' ---
    constexpr uint32_t kCharacters[2] = {kVars + 0xB00, kVars + 0xC00};
    constexpr uint32_t kOn = 1;                     // byte
    constexpr uint32_t kChoiceStep = 4;             // byte: 0 choosing a character, 2 a handicap level
    constexpr uint32_t kFacing = 0x0B, kColoursAt = 0x0D; // bytes
    constexpr uint32_t kChoiceSecondsLeft = 0x0E, kChoiceFrames = 0x0F; // bytes
    constexpr uint32_t kX = 0x10, kY = 0x14;        // words
    constexpr uint32_t kCodes = 0x50;               // three records of four bytes: how far each code has been followed
    constexpr uint32_t kCodeStep = 0, kCodePlace = 1, kCodeDone = 2, kCodeNumber = 3;
    constexpr uint32_t kOtherColours = 0x9F;        // byte (in the player's record too)
    // In a player's record
    constexpr uint32_t kLevel = 0xA1, kPlayerState = 0x80;
    constexpr uint16_t kButtons = 0x0300, kSecondButton = 0x0200, kStick = 0x000F, kStickRight = 1, kStickLeft = 2, kStartHeld = 0x8000;
    constexpr uint16_t kKicks = 0x7000, kFirstKick = 0x1000; // (the panel's fourth to sixth buttons, which the game does not use)
    constexpr int kHandicapLevels = 5;

    // --- the game's tables ---
    constexpr uint32_t kLastStages = 0x006FBC, kLastStagesShort = 0x006FC4; // words, by level
    constexpr uint32_t kCursorMoves = 0x0078FA;     // bytes, by the stick: how far the cursor goes
    constexpr uint32_t kCharacterAt = 0x007906;     // bytes: the character at each of the cursor's places
    constexpr uint32_t kPlaceOf = 0x00867A;         // bytes: the place of each character
    constexpr uint32_t kLadderPlaces = 0x008682, kLadderMarks = 0x0086A2; // longs, by stage
    constexpr uint32_t kCodeLists = 0x0086C2;       // the codes: for each, player 1's and player 2's moves of the stick
    constexpr uint32_t kDigitTiles = 0x008246;      // words
    constexpr uint32_t kWords = 0x008602;           // the screens' headings
    constexpr uint32_t kShownAfterLadder = 0x007DEA; // bytes: the small pictures the ladder then shows
    constexpr uint32_t kPortraits = 0x084560, kSmallPortraits = 0x08DC3C, kColours = 0x0A0000;
    constexpr uint8_t kHiddenTimed = 8, kHiddenFirst = 9, kHiddenSecond = 10;

    // --- video memory ---
    // Colours: sixteen words a row; a colour's top four bits are how bright it is
    constexpr uint32_t kColourRows = 0x90C000, kColourRow = 0x20;
    constexpr uint16_t kDim = 0x8000, kBright = 0xF000;
    constexpr uint32_t kModeColours[2] = {0x90C060, 0x90C080}, kLevelColours[3] = {0x90C040, 0x90C000, 0x90C020};
    // The text layer (banner.cpp): a tile is two words, the next tile down the next in memory
    constexpr uint32_t kDown = 4, kAcross = 0x80;
    constexpr uint32_t kBlank = 0x00200000;
    constexpr uint32_t kTimerAt = 0x903310, kChoiceTimerAt[2] = {0x902838, 0x902F38};
    constexpr uint32_t kPortraitAt[2] = {0x90212C, 0x90302C}, kPortraitFrameAt[2] = {0x902028, 0x903028};
    constexpr uint32_t kStageNumberAt = 0x90420C;

    // --- the game's routines used here that are not written again ---
    constexpr uint32_t kNewTask = 0x02DA46, kNewTaskBehind = 0x02DAC6, kSetAct = 0x014390, kShowCharacter = 0x02D29C,
                       kDrawPicture = 0x0179E4, kDrawPictureWide = 0x017BA8, kShowStageNumber = 0x00BB8C, kSayCharacter = 0x01F244,
                       kSetOpponent = 0x00BA44, kPressed1 = 0x00BA00, kPressed2 = 0x00BA22, kStartFade = 0x006634, kEndFade = 0x006650,
                       kNextRoundPicture = 0x02D7CC;
    constexpr uint32_t kSoundMove = 0x02B5BA, kSoundChoose = 0x02B5D4, kSoundCursor = 0x02B572, kSoundTaken = 0x02B596,
                       kSoundModeScreen = 0x02B7EC, kSoundLevelScreen = 0x02B806, kSoundCharacters = 0x02B820, kSoundMode = 0x02B83A,
                       kSoundLevel = 0x02B866, kSoundDone = 0x02B88C;
    constexpr uint32_t kFrameWork[] = {0x016A3A, 0x016BCA, 0x01495A};
    // (the time the game's code takes, about)
    constexpr uint32_t kBackdropTime = 79200, kClearTime = 32000, kColoursTime = 700, kPortraitTime = 2600, kWordsTime = 1500,
                       kSmallTime = 620, kBrightTime = 620, kDigitsTime = 300, kLadderTime = 5200, kBringTime = 650, kFrameTime = 90, kMenuTime = 200;

    void NextSubStep() {
        SetRam16(kSubStep, static_cast<uint16_t>(Ram16(kSubStep) + 2));
    }
    void Add16(uint32_t at, int amount) {
        SetRam16(at, static_cast<uint16_t>(Ram16(at) + amount));
    }
    // Counts the step's frames down: true when they are over
    bool TimerOut() {
        Add16(kTimer, -1);
        return Ram16(kTimer) == 0;
    }
    // A choice's time: a second less every kFramesASecond frames, in decimal digits. True when it has run out.
    bool SecondsOut(uint32_t seconds, uint32_t frames) {
        SetRam8(frames, static_cast<uint8_t>(Ram8(frames) - 1));
        if (Ram8(frames) != 0) {
            return false;
        }
        SetRam8(frames, kFramesASecond);
        const uint8_t left = Ram8(seconds);
        if (left == 0) {
            return true;
        }
        SetRam8(seconds, static_cast<uint8_t>((left & 0x0F) ? left - 1 : left - 7));
        return false;
    }
    // A new task of this kind, or 0 when there is no room for one
    uint32_t NewTask(M68K &c, uint16_t kind, uint32_t routine = kNewTask) {
        Call(c, routine);
        if (c.flagZ) {
            return 0;
        }
        const uint32_t task = c.a[4] & 0xFFFFFF;
        SetRam8(task, static_cast<uint8_t>(Ram8(task) + 1));
        SetRam16(task + 2, kind);
        return task;
    }
    void Sound(M68K &c, uint32_t routine) {
        Call(c, routine);
    }
    int16_t Offset(M68K &c, uint32_t at) {
        return static_cast<int16_t>(Mem16(c, at));
    }

    // --- putting things into video memory ---

    // 008282. How bright a row of colours is.
    void SetBrightness(M68K &c, uint32_t row, uint16_t brightness) {
        for (int n = 0; n < 16; n++) {
            c.Write(row + 2 * n, 1, (Mem16(c, row + 2 * n) & 0x0FFF) | brightness);
        }
        Spend(c, kBrightTime);
    }
    // A letter or digit of four tiles
    void PutBig(M68K &c, uint32_t at, uint16_t tile, uint16_t colours) {
        c.Write(at, 1, tile);
        c.Write(at + 2, 1, colours);
        c.Write(at + kAcross, 1, static_cast<uint16_t>(tile + 1));
        c.Write(at + kAcross + 2, 1, colours);
        c.Write(at + kDown, 1, static_cast<uint16_t>(tile + 0x10));
        c.Write(at + kDown + 2, 1, colours);
        c.Write(at + kAcross + kDown, 1, static_cast<uint16_t>(tile + 0x11));
        c.Write(at + kAcross + kDown + 2, 1, colours);
    }
    void PutBlank(M68K &c, uint32_t at) {
        c.Write(at, 2, kBlank);
        c.Write(at + kDown, 2, kBlank);
        c.Write(at + kAcross, 2, kBlank);
        c.Write(at + kAcross + kDown, 2, kBlank);
    }
    // 008220. A time of two decimal digits, in big digits.
    void ShowSeconds(M68K &c, uint32_t at, uint8_t seconds, uint16_t colours) {
        PutBig(c, at, Code16(c, kDigitTiles + 2 * (seconds >> 4)), colours);
        PutBig(c, at + 2 * kAcross, Code16(c, kDigitTiles + 2 * (seconds & 0x0F)), colours);
        Spend(c, kDigitsTime);
    }
    // 00857A. One of the screens' headings (a list: where, the colours, then a tile a letter).
    void PutWords(M68K &c, int which) {
        uint32_t list = kWords + Offset(c, kWords + 2 * which);
        const uint16_t column = Mem16(c, list), row = Mem16(c, list + 2), colours = Mem16(c, list + 4);
        uint32_t at = 0x900000 + static_cast<int16_t>((column << 7) + ((row & 0x1F) << 2) + ((row & 0x20) << 8));
        for (list += 6; !(Mem16(c, list) & 0x8000); list += 2, at += 2 * kAcross) {
            if (Mem16(c, list) == 0) {
                PutBlank(c, at);
            } else {
                PutBig(c, at, Mem16(c, list), colours);
            }
        }
        Spend(c, kWordsTime);
    }
    // 008470. The backdrop of the screens before a match: a pattern over the whole of its layer; and the
    // tasks that go with it.
    void DrawBackdrop(M68K &c) {
        uint32_t tiles[4] = {0x63D30016, 0x63E30016, 0x63F30016, 0x64030016};
        uint32_t at = 0x908000;
        for (int column = 0; column < 64; column++) {
            for (int part = 0; part < 4; part++) {
                for (int n = 0; n < 4; n++) {
                    at -= 16;
                    for (int tile = 0; tile < 4; tile++) {
                        c.Write(at + 4 * tile, 2, tiles[tile]);
                    }
                }
                for (uint32_t &tile : tiles) {
                    tile = (tile + 0x30000) & 0xFFF3FFFF;
                }
            }
            std::swap(tiles[0], tiles[2]);
            std::swap(tiles[1], tiles[3]);
        }
        Spend(c, kBackdropTime);
        if (NewTask(c, 0x2700)) {
            NewTask(c, 0x0400, kNewTaskBehind);
        }
        SetRam16(kLayerOrder, 0x1B0E);
        SetRam8(kBackdropOn, 1);
        SetRam16(kHelpText, 0);
    }
    // 008094. The layers the choices are drawn in, emptied.
    void ClearLayers(M68K &c) {
        constexpr uint32_t kEmptyTile = 0x58200000;
        constexpr uint32_t kParts[][2] = {{0x904000, 0x906000}, {0x906800, 0x907000}, {0x907800, 0x908000}};
        for (const auto &part : kParts) {
            for (uint32_t at = part[1]; at > part[0];) {
                at -= 4;
                c.Write(at, 2, kEmptyTile);
            }
        }
        Spend(c, kClearTime);
    }
    // 008532. A character's colours (two rows; its other colours are the next two in the game's list) put
    // where the character's picture looks for them, as bright as what was there.
    void SetColours(M68K &c, uint32_t character) {
        c.a[6] = character;
        if (!Ram8(character + kOn)) {
            return;
        }
        uint32_t from = kColours + ((Ram8(character + kCharacter) * 2u + Ram8(character + kOtherColours)) << 7);
        uint32_t to = kColourRows + ((Ram8(character + kColoursAt) + 0x0Cu) & 0x1F) * kColourRow;
        const uint32_t brightness = Mem32(c, to) & 0xF000F000;
        for (int n = 0; n < 16; n++, from += 4, to += 4) {
            c.Write(to, 2, Mem32(c, from) | brightness);
        }
        Spend(c, kColoursTime);
    }
    // 0079B0. The big portrait of the character a player's cursor is on.
    void ShowPortrait(M68K &c, int n) {
        const uint32_t character = kCharacters[n];
        uint32_t from = kPortraits + Offset(c, kPortraits + 0x0C);
        from = Mem32(c, from + (Ram8(character + kCharacter) << 4) + 4) + 6;
        uint32_t to = kPortraitAt[n];
        for (int row = 0; row < 14; row++, to += kAcross) {
            for (int tile = 0; tile < 5; tile++, from += 4) {
                c.Write(to + 4 * tile, 2, Mem32(c, from));
            }
        }
        Spend(c, kPortraitTime);
    }
    // 007A00. What is around the portrait, taken away.
    void ClearPortraitFrame(M68K &c, int n) {
        uint32_t to = kPortraitFrameAt[n];
        for (int row = 0; row < 16; row++, to += kAcross) {
            for (int tile = 0; tile < 6; tile++) {
                c.Write(to + 4 * tile, 2, kBlank);
            }
        }
        Spend(c, kPortraitTime);
    }
    // 008102, 00815E. The time a player still has to choose, in tall digits, and the same taken away.
    void ShowChoiceTimer(M68K &c, uint32_t character) {
        uint32_t at = kChoiceTimerAt[Ram8(character + kPlayerNumber) ? 1 : 0];
        const uint8_t seconds = Ram8(character + kChoiceSecondsLeft);
        for (const uint16_t digit : {static_cast<uint16_t>(seconds >> 4), static_cast<uint16_t>(seconds & 0x0F)}) {
            c.Write(at, 1, static_cast<uint16_t>(0xA0 + digit));
            c.Write(at + 2, 1, 0x1F);
            c.Write(at + kDown, 1, static_cast<uint16_t>(0xB0 + digit));
            c.Write(at + kDown + 2, 1, 0x1F);
            at += kAcross;
        }
    }
    void ClearChoiceTimer(M68K &c, uint32_t character) {
        PutBlank(c, kChoiceTimerAt[Ram8(character + kPlayerNumber) ? 1 : 0]);
    }
    // 007D5E. A small portrait: three tiles by three, in the ladder's layer.
    void PutSmall(M68K &c, uint32_t to, uint32_t pictures, uint32_t which) {
        uint32_t from = Mem32(c, pictures + (which << 4) + 4) + 6;
        for (int row = 0; row < 3; row++, to += 0x40) {
            for (int tile = 0; tile < 3; tile++, from += 4) {
                c.Write(to + 4 * tile, 2, Mem32(c, from) + 0x58000000);
            }
        }
        Spend(c, kSmallTime);
    }
    uint32_t SmallPictures(M68K &c) {
        return kSmallPortraits + Offset(c, kSmallPortraits + 4);
    }
    uint32_t LadderPlace(M68K &c, int stage, uint16_t page) {
        return Mem32(c, kLadderPlaces + 4 * stage) + static_cast<int16_t>(page);
    }
    // Which page of the ladder's layer is showing
    uint16_t PageShowing() {
        return Ram16(kScrollX) ? 0x800 : 0;
    }

    // --- the characters on the screen ---

    void SetAct(M68K &c, uint32_t character, uint32_t act) {
        c.a[6] = character;
        c.d[0] = act;
        Call(c, kSetAct);
    }
    constexpr uint32_t kActWaiting = 0x17, kActChosen = 0x18;

    // The start of 0074DE and 008324: a player's character put in its place at its side of the screen
    void PlaceCharacter(int n, uint32_t character) {
        SetRam8(character + kOn, 1);
        SetRam16(character + kX, n ? 0x150 : 0x30);
        SetRam16(character + kY, 0x30);
        SetRam16(character + 0xD6, 0x30);
        SetRam16(character + 0x30, 0xC000);
        SetRam8(character + kFacing, n ? 0 : 1);
        SetRam16(character + 0x0C, n ? 0 : 0xFF19);
    }
    // The tasks that go with a character: what it stands on, and the frame of its portrait
    bool CharacterTasks(M68K &c, int n, uint16_t stand) {
        uint32_t task = NewTask(c, stand);
        if (!task) {
            return false;
        }
        SetRam16(task + kX, n ? 0x150 : 0x30);
        SetRam16(task + kY, 0x28);
        SetRam16(task + 0x0C, 0xFF08);
        SetRam16(task + 0x32, static_cast<uint16_t>(kCharacters[n]));
        task = NewTask(c, static_cast<uint16_t>(0x1800 + n));
        if (!task) {
            return false;
        }
        SetRam8(task + 0x0A, 0);
        SetRam32(task + 0x32, kChosen[n]);
        return true;
    }
    // 0074DE, 0075DA. A player's character comes on to be chosen.
    void BringIn(M68K &c, int n) {
        const uint32_t character = kCharacters[n], player = Player(n);
        const bool choosing = Ram8(kChoosing) & (1 << n);
        c.a[6] = character;
        PlaceCharacter(n, character);
        SetRam8(character + kCharacter, Ram8(player + kCharacter));
        SetRam8(kChosen[n], Ram8(player + kCharacter));
        SetRam8(character + kPlayerNumber, static_cast<uint8_t>(n));
        SetRam8(character + kChoiceSecondsLeft, kChoiceSeconds);
        SetRam8(character + kChoiceFrames, kFramesASecond);
        SetRam16(character + kPadRead, 0);
        SetRam16(character + kPadReadBefore, 0);
        if (choosing) {
            SetRam8(character + kOtherColours, 0);
            SetRam8(player + kOtherColours, 0);
        }
        SetRam8(player + kHandicap, 2);
        if (!CharacterTasks(c, n, 0x2200)) {
            return;
        }
        if (choosing) {
            const uint32_t task = NewTask(c, static_cast<uint16_t>(0x2100 + n)); // the cursor
            if (!task) {
                return;
            }
            SetRam32(task + 0x32, kChosen[n]);
        }
        SetRam8(kChosen[n] + kCursor, Mem8(c, kPlaceOf + Ram8(kChosen[n])));
        SetAct(c, character, Ram8(kPlaying) & (1 << n) ? kActChosen : kActWaiting);
        SetColours(c, character);
    }
    // 008324, 0083CC. A character comes back on as it was chosen (when the opponent is shown).
    void BringBack(M68K &c, int n) {
        const uint32_t character = kCharacters[n], player = Player(n);
        c.a[6] = character;
        Spend(c, kBringTime);
        PlaceCharacter(n, character);
        SetRam8(character + kCharacter, Ram8(kChosen[n]));
        SetRam8(character + kPlayerNumber, 0); // (for player 2 as well)
        if (Ram8(player + kComputer)) {
            SetRam8(character + kOtherColours, 0);
            SetRam8(player + kOtherColours, 0);
        }
        if (!CharacterTasks(c, n, 0x2202)) {
            return;
        }
        SetAct(c, character, Ram8(kPlaying) & (1 << n) ? kActChosen : kActWaiting);
        SetColours(c, character);
    }
    // 0082B0. The frames of the two portraits.
    void PortraitFrames(M68K &c) {
        for (int n = 0; n < 2; n++) {
            const uint32_t task = NewTask(c, static_cast<uint16_t>(0x1800 + n));
            if (!task) {
                return;
            }
            SetRam8(task + 0x0A, 1);
            SetRam8(task + 0x0E, Ram8(kPortraitsStay));
        }
    }
    // 008294. Eight tasks: the small portraits coming in.
    void SmallPortraitTasks(M68K &c) {
        for (int n = 7; n >= 0; n--) {
            Call(c, kNewTask);
            if (c.flagZ) {
                return;
            }
            const uint32_t task = c.a[4] & 0xFFFFFF;
            SetRam8(task, static_cast<uint8_t>(Ram8(task) + 1));
            SetRam8(task + 2, 0x17);
            SetRam8(task + 3, static_cast<uint8_t>(n));
        }
    }
    // 0077BE. The two characters are shown.
    void ShowCharacters(M68K &c) {
        Spend(c, kFrameTime);
        for (const uint32_t character : kCharacters) {
            c.a[6] = character;
            if (Ram8(character + kOn)) {
                Call(c, kShowCharacter);
            }
        }
    }

    // --- a player choosing ---

    uint16_t Pressed(uint32_t character) {
        return Ram16(character + kPadRead) & ~Ram16(character + kPadReadBefore);
    }
    // "Hidden characters" for this player (0 player 1, 1 player 2): a kick chooses one of the three
    bool HiddenFor(int n) {
        return (FeatureValue("hidden_characters") & (1 << n)) != 0;
    }
    // 008186. A code is followed: while Start is held with the player's cursor on the code's character
    // (character 0 for player 1, 6 for player 2), each move of the stick has to be the code's next; any
    // other starts it again. Once the code is done, any move at all undoes it.
    void FollowCode(M68K &c, uint32_t character, uint32_t chosen, uint32_t code) {
        const auto startAgain = [&]() {
            SetRam8(code + kCodeStep, 0);
            SetRam8(code + kCodePlace, 0);
            SetRam8(code + kCodeDone, 0);
        };
        const bool second = Ram8(character + kPlayerNumber) != 0;
        if (!(Ram16(character + kPadRead) & kStartHeld) || Ram8(chosen) != (second ? 6 : 0)) {
            startAgain();
            return;
        }
        const uint16_t moved = Pressed(character) & kStick;
        if (Ram8(code + kCodeStep) != 0) {
            if (moved) {
                startAgain();
            }
            return;
        }
        const uint32_t list = kCodeLists + Offset(c, kCodeLists + Ram8(code + kCodeNumber) * 4 + (second ? 2 : 0));
        const uint16_t next = Mem16(c, list + Ram8(code + kCodePlace));
        if (next == 0xFFFF) {
            SetRam8(code + kCodeStep, 2);
            SetRam8(code + kCodeDone, 1);
        } else if (moved == next) {
            SetRam8(code + kCodePlace, static_cast<uint8_t>(Ram8(code + kCodePlace) + 2));
        } else if (moved) {
            startAgain();
        }
    }
    // 00790E. A player who has done a code gets its character in place of the one chosen: the third code
    // (thirteen downs) while the time shows 10; else the second (the same thirteen downs); else the first.
    void HiddenChoice(uint32_t character, uint32_t chosen) {
        uint8_t hidden;
        if (Ram8(character + kCodes + 8 + kCodeDone) && Ram8(character + kChoiceSecondsLeft) == 0x10) {
            hidden = kHiddenTimed;
        } else if (Ram8(character + kCodes + 4 + kCodeDone)) {
            hidden = kHiddenSecond;
        } else if (Ram8(character + kCodes + kCodeDone)) {
            hidden = kHiddenFirst;
        } else {
            return;
        }
        SetRam8(character + kCharacter, hidden);
        SetRam8(chosen, hidden);
    }
    // 00794C. Which colours the character has: the other ones when chosen with the second button; and when
    // the other player, a person, has already chosen the same character, whichever that player has not.
    void ChooseColours(int n) {
        const uint32_t character = kCharacters[n], other = kCharacters[n ^ 1];
        uint8_t colours = Pressed(character) & kSecondButton ? 1 : 0;
        if (Ram8(character + kCharacter) == Ram8(other + kCharacter) && !Ram8(Player(n ^ 1) + kComputer) &&
            (Ram8(kCharactersChosen) & (1 << (n ^ 1)))) {
            colours = Ram8(other + kOtherColours) ^ 1;
        }
        SetRam8(character + kOtherColours, colours);
        SetRam8(Player(n) + kOtherColours, colours);
    }
    // 0077F2. A frame of a player choosing a character.
    void ChoosingCharacter(M68K &c, int n) {
        const uint32_t character = kCharacters[n], chosen = kChosen[n];
        uint16_t kick = 0;
        ShowChoiceTimer(c, character);
        ShowPortrait(c, n);
        if (!SecondsOut(character + kChoiceSecondsLeft, character + kChoiceFrames)) {
            for (uint32_t code = 0; code < 3; code++) {
                SetRam8(character + kCodes + 4 * code + kCodeNumber, static_cast<uint8_t>(code));
                FollowCode(c, character, chosen, character + kCodes + 4 * code);
            }
            const uint16_t pressed = Pressed(character);
            kick = HiddenFor(n) ? pressed & kKicks : 0;
            if (!(pressed & kButtons) && !kick) {
                // The cursor moves, but not off the end of a row of four on to the other row
                const int8_t move = static_cast<int8_t>(Code8(c, kCursorMoves + (pressed & kStick)));
                if (!(pressed & kStick) || move == 0) {
                    return;
                }
                if (Ram8(chosen + kCursor) >= 8) {
                    SetRam8(chosen + kCursor, 0);
                }
                const uint8_t was = Ram8(chosen + kCursor), place = static_cast<uint8_t>(was + move);
                if (place >= 8 || (place == 4 && was == 3) || (place == 3 && was == 4)) {
                    return;
                }
                SetRam8(chosen + kCursor, place);
                SetRam8(chosen, Code8(c, kCharacterAt + place));
                SetRam8(character + kCharacter, Ram8(chosen));
                c.a[6] = character;
                Sound(c, kSoundCursor);
                SetAct(c, character, kActWaiting);
                SetColours(c, character);
                return;
            }
        }
        // Chosen, or out of time
        SetRam8(kCharactersChosen, Ram8(kCharactersChosen) | (1 << n));
        HiddenChoice(character, chosen);
        if (kick) {
            // ("Hidden characters": a kick chooses one of them outright, as the game's leftover at 0076EC does)
            const uint8_t hidden = static_cast<uint8_t>(kHiddenTimed + (kick & kFirstKick ? 0 : kick & (kFirstKick << 1) ? 1 : 2));
            SetRam8(character + kCharacter, hidden);
            SetRam8(chosen, hidden);
        }
        c.a[6] = character;
        c.a[4] = chosen;
        c.d[7] = static_cast<uint32_t>(n);
        Sound(c, kSoundTaken);
        Sound(c, kSoundDone);
        c.d[0] = Ram8(character + kCharacter);
        Call(c, kSayCharacter);
        ClearChoiceTimer(c, character);
        ChooseColours(n);
        SetAct(c, character, kActChosen);
        SetColours(c, character);
        ClearPortraitFrame(c, n);
    }
    // 007A38. A frame of a player choosing a handicap level, with the stick left and right.
    void ChoosingHandicap(M68K &c, int n) {
        const uint32_t character = kCharacters[n], chosen = kChosen[n];
        ShowChoiceTimer(c, character);
        const bool out = SecondsOut(character + kChoiceSecondsLeft, character + kChoiceFrames);
        const uint16_t pressed = Pressed(character);
        if (out || (pressed & kButtons)) {
            SetRam8(kCharactersChosen, Ram8(kCharactersChosen) | (1 << n));
            c.a[6] = character;
            Sound(c, kSoundTaken);
            ClearChoiceTimer(c, character);
            return;
        }
        if (!(pressed & (kStickLeft | kStickRight))) {
            return;
        }
        int level = Ram8(chosen + kHandicapChosen);
        level += pressed & kStickLeft ? -1 : 1;
        if (level < 0 || level >= kHandicapLevels) {
            return;
        }
        SetRam8(chosen + kHandicapChosen, static_cast<uint8_t>(level));
        SetRam8(Player(n) + kHandicap, static_cast<uint8_t>(level));
        c.a[6] = character;
        Sound(c, kSoundCursor);
    }
    // 0076D6. A frame of the players choosing, each until done; when both are, the screen's next step.
    void PlayersChoose(M68K &c) {
        for (int n = 0; n < 2; n++) {
            const uint32_t character = kCharacters[n], chosen = kChosen[n];
            if (Ram8(kCharactersChosen) & (1 << n)) {
                continue;
            }
            c.d[7] = static_cast<uint32_t>(n);
            c.a[6] = character;
            c.a[4] = chosen;
            if (Ram8(character + kChoiceStep) == 0) {
                ChoosingCharacter(c, n);
            } else {
                ChoosingHandicap(c, n);
            }
            if (Ram8(kDebug)) {
                // (the panel's fourth to sixth buttons take the hidden characters outright)
                for (int button = 0; button < 3; button++) {
                    if (Ram16(character + kPadRead) & (0x1000 << button)) {
                        SetRam8(character + kCharacter, static_cast<uint8_t>(8 + button));
                        SetRam8(Player(n) + kCharacter, static_cast<uint8_t>(8 + button));
                        SetRam8(chosen, static_cast<uint8_t>(8 + button));
                    }
                }
            }
        }
        if (Ram8(kCharactersChosen) == 3) {
            NextSubStep();
            SetRam16(kTimer, 0x1E);
            SetRam8(kHeld1, 1);
            SetRam8(kHeld2, 1);
        }
    }

    // --- the ladder of opponents ---

    // 008006. A small portrait put on the ladder, and the computer's side made that character.
    void PutOpponent(M68K &c, uint32_t to, uint32_t character, uint32_t picture) {
        const int n = Ram8(Player(0) + kComputer) ? 0 : 1;
        SetRam8(kChosen[n], static_cast<uint8_t>(character));
        SetRam8(kCharacters[n] + kCharacter, static_cast<uint8_t>(character));
        PutSmall(c, to, SmallPictures(c), picture);
        SetAct(c, kCharacters[n], kActWaiting);
        SetColours(c, kCharacters[n]);
    }
    // 007C7E, 007C86. The ladder drawn: each stage's opponent if already beaten (with a mark over it), else
    // a blank.
    void DrawLadder(M68K &c, uint16_t page) {
        SetRam16(kLadderPage, page);
        Spend(c, kLadderTime);
        for (int stage = 7; stage >= 0; stage--) {
            const uint32_t character = Ram8(kOrder + stage);
            const uint32_t to = LadderPlace(c, stage, page);
            SetRam32(kLadderMark, Mem32(c, kLadderMarks + 4 * stage));
            if (!(Ram16(kOpponentsMet) & (1u << (character & 31)))) {
                // (007FEA: the blank)
                uint16_t tile = 0x5BAA;
                for (int column = 0; column < 3; column++, tile += 0x10) {
                    for (int row = 0; row < 3; row++) {
                        c.Write(to + 4 * column + 0x40 * row, 1, static_cast<uint16_t>(tile + row));
                        c.Write(to + 4 * column + 0x40 * row + 2, 1, 0x17);
                    }
                }
                continue;
            }
            PutOpponent(c, to, character, character + 0x0B);
            uint32_t from = kPortraits + Offset(c, kPortraits + 0x0A);
            from = Mem32(c, from + 4) + 6;
            uint32_t mark = Ram32(kLadderMark);
            for (int row = 0; row < 6; row++, mark += kAcross) {
                for (int tile = 0; tile < 6; tile++, from += 4) {
                    c.Write(mark + 4 * tile, 2, Mem32(c, from));
                }
            }
            Spend(c, kSmallTime);
        }
    }
    void SpinSound(M68K &c) {
        if ((Ram8(kVars + kFrameCount) & 3) == 0) {
            Sound(c, kSoundMove);
        }
    }
    // 007D0C. All eight places of the ladder run through the characters.
    void SpinLadder(M68K &c) {
        SpinSound(c);
        for (int stage = 7; stage >= 0; stage--) {
            PutSmall(c, LadderPlace(c, stage, PageShowing()), SmallPictures(c), ((Ram16(kTimer) >> 2) + stage) & 7);
        }
    }
    // 007D94. The stage's place on the ladder runs through the characters.
    void SpinStage(M68K &c) {
        SpinSound(c);
        const uint16_t stage = Ram16(kStage);
        if (stage < 8) {
            const uint32_t character = (Ram16(kTimer) & 0x1C) >> 2;
            PutOpponent(c, LadderPlace(c, stage, PageShowing()), character, character);
        }
    }
    // The computer's side becomes a character no one chose, where a person is not playing it
    void GiveSides(M68K &c, uint8_t character) {
        for (int n = 0; n < 2; n++) {
            if ((Ram8(kChoosing) | Ram8(kPlaying)) & (1 << n)) {
                continue;
            }
            SetRam8(Player(n) + kCharacter, character);
            SetRam8(kCharacters[n] + kCharacter, character);
            SetRam8(kChosen[n], character);
            BringBack(c, n);
            c.d[7] = static_cast<uint32_t>(n);
            SetAct(c, kCharacters[n], kActChosen);
            ShowPortrait(c, n);
        }
    }
    // 007EA2. The whole ladder shows the last opponent, who comes on.
    void ShowLastOpponent(M68K &c) {
        for (int stage = 7; stage >= 0; stage--) {
            PutSmall(c, LadderPlace(c, stage, PageShowing()), SmallPictures(c), 9);
        }
        GiveSides(c, 9);
    }
    // 007DF2. The same for the opponent who comes after the ladder, unless the last opponent has come in.
    void ShowAfterLadder(M68K &c) {
        if (Ram8(kLastOpponentIn)) {
            ShowLastOpponent(c);
            return;
        }
        for (int stage = 7; stage >= 0; stage--) {
            PutSmall(c, LadderPlace(c, stage, PageShowing()), SmallPictures(c), Code8(c, kShownAfterLadder + stage));
        }
        GiveSides(c, 0x0A);
    }
    // 007F46, 007F4C. The stage's opponent is put on the ladder and on the computer's side of the screen, in
    // its other colours if the person has the same character in the first.
    void ShowStageOpponent(M68K &c, uint16_t page) {
        const uint16_t stage = Ram16(kStage);
        if (stage >= 8) {
            return;
        }
        const uint8_t character = Ram8(kOrder + stage);
        PutOpponent(c, LadderPlace(c, stage, page), character, character);
        const int n = Ram8(Player(0) + kComputer) ? 0 : 1;
        const uint32_t side = kCharacters[n], player = Player(n), person = Player(n ^ 1);
        c.d[7] = static_cast<uint32_t>(n);
        SetRam8(side + kCharacter, character);
        SetRam8(player + kCharacter, character);
        SetRam8(side + kOtherColours, 0);
        SetRam8(player + kOtherColours, 0);
        if (Ram8(person + kCharacter) == character) {
            const uint8_t colours = Ram8(person + kOtherColours) ^ 1;
            SetRam8(player + kOtherColours, colours);
            SetRam8(side + kOtherColours, colours);
            SetColours(c, side);
        }
        ShowPortrait(c, n);
    }
    // 0082EA. The characters come back on after a match.
    void BringBackAfterMatch(M68K &c) {
        const bool last = Ram16(kStage) == kLastStageNumber;
        for (int n = 0; n < 2; n++) {
            if (!last || (Ram8(kPlaying) & (1 << n))) {
                BringBack(c, n);
            }
        }
        SetRam8(kPortraitsStay, 1);
        PortraitFrames(c);
    }
    // 007C5E. Whether a player who is choosing has pressed a button.
    bool AnyButton(M68K &c) {
        c.d[0] = 0;
        c.d[1] = 0;
        if (Ram8(kChoosing) & 1) {
            Call(c, kPressed1);
        }
        if (Ram8(kChoosing) & 2) {
            Call(c, kPressed2);
        }
        return (c.d[0] & 0xFFFF) != 0;
    }

    // --- the screens ---

    void ScrollBackdrop() {
        Add16(kScrollX, -3);
        Add16(kScrollY, 2);
    }
    // The backdrop is stopped where its pattern repeats, and the portraits' frames and the small portraits
    // start coming in (the end of the choice of mode, and of level)
    void TurnToCharacters(M68K &c) {
        SetRam8(kTasksDone, 0);
        SetRam8(kBackdropOn, 0);
        SetRam16(kHelpText, kNoHelp);
        SetRam16(kTimer, 2);
        PutWords(c, 2);
        SetRam16(kScrollX, Ram16(kScrollX) & 0x3F);
        SetRam16(kScrollY, static_cast<uint16_t>((Ram16(kScrollY) & 0x3F) + 0x100));
        SetRam8(kPortraitsStay, 0);
        PortraitFrames(c);
        SmallPortraitTasks(c);
    }
    // The backdrop has gone by: on to a screen of characters
    bool BackdropGone() {
        SetRam16(kScrollY, Ram16(kScrollY) & 0x3FF);
        if (Ram16(kScrollY) < 0x200) {
            return false;
        }
        SetRam16(kScrollX, 0);
        SetRam16(kScrollY, 0x300);
        SetRam16(kSubStep, 0);
        SetRam16(kTimer, 0x14);
        return true;
    }
    // The stick's and buttons' new presses on the choice of mode and of level: both players' when both are
    // in, else the one's who is
    uint16_t MenuPressed(bool both) {
        uint16_t now = Ram16(kCharacters[0] + kPadRead), before = Ram16(kCharacters[0] + kPadReadBefore);
        if (!(Ram8(kChoosing) & 1)) {
            now = Ram16(kCharacters[1] + kPadRead);
            before = Ram16(kCharacters[1] + kPadReadBefore);
        } else if (both && (Ram8(kChoosing) & 2)) {
            now |= Ram16(kCharacters[1] + kPadRead);
            before |= Ram16(kCharacters[1] + kPadReadBefore);
        }
        return now & ~before;
    }
    void ShowHelp(M68K &c, uint8_t cursor, uint16_t first, uint16_t firstOther) {
        SetRam16(kHelpText, static_cast<uint16_t>(cursor + (Ram16(kOtherLanguage) ? firstOther : first)));
        ShowSeconds(c, kTimerAt, Ram8(kSeconds), 0x1F);
    }

    // 0069F8. The choice of mode.
    void ShowModeCursor(M68K &c) {
        SetBrightness(c, kModeColours[0], Ram8(kModeCursor) ? kDim : kBright);
        SetBrightness(c, kModeColours[1], Ram8(kModeCursor) ? kBright : kDim);
    }
    void ChooseMode(M68K &c) {
        Spend(c, kMenuTime);
        switch (Ram16(kSubStep)) {
        case 0: { // the two choices come in
            NextSubStep();
            SetBrightness(c, kModeColours[0], kDim);
            SetBrightness(c, kModeColours[1], kDim);
            PutWords(c, 0);
            for (int n = 0; n < 2; n++) {
                const uint32_t task = NewTask(c, static_cast<uint16_t>(0x1900 + n));
                if (!task) {
                    break;
                }
                SetRam32(task + 0x32, kModeCursor);
                if (n == 1) {
                    SetRam16(kLayerOrder, 0x1B0E);
                }
            }
            break;
        }
        case 2:
            if (Ram8(kTasksDone) == 3) {
                NextSubStep();
                SetRam8(kSeconds, kChoiceSeconds);
                SetRam8(kSecondFrames, kFramesASecond);
                Sound(c, kSoundModeScreen);
            }
            break;
        case 4: { // choosing: the stick moves the cursor, a button takes it
            const bool out = SecondsOut(kSeconds, kSecondFrames);
            const uint16_t pressed = out ? 0 : MenuPressed(false);
            if (out || (pressed & kButtons)) {
                NextSubStep();
                SetRam16(kTimer, 0x1E);
                Sound(c, kSoundChoose);
                SetRam8(kVars + 0xE0, Ram8(kModeCursor));
                c.d[1] = Ram8(kModeCursor);
                Sound(c, kSoundMode);
            } else {
                if ((pressed & kStickLeft) && Ram8(kModeCursor)) {
                    SetRam8(kModeCursor, 0);
                    Sound(c, kSoundMove);
                }
                if ((pressed & kStickRight) && !Ram8(kModeCursor)) {
                    SetRam8(kModeCursor, 1);
                    Sound(c, kSoundMove);
                }
            }
            ShowModeCursor(c);
            ShowHelp(c, Ram8(kModeCursor), 0xD2, 0x2C);
            break;
        }
        case 6: // the choice flashes; then two players go on to the characters, one to the choice of level
            if (TimerOut()) {
                if (!Ram8(kModeCursor)) {
                    NextSubStep();
                    SetRam8(kTasksDone, 0);
                    SetRam16(kHelpText, 0);
                    PutWords(c, 2);
                } else {
                    SetRam16(kTimer, 1);
                    if ((Ram16(kScrollX) & 0x7F) < 0x40) {
                        SetRam16(kSubStep, 0x0A);
                        TurnToCharacters(c);
                    }
                }
            }
            if (Ram16(kTimer) >= 0x0A) {
                SetBrightness(c, kModeColours[Ram8(kModeCursor) ? 1 : 0], Ram16(kTimer) & 1 ? kDim : kBright);
            }
            break;
        case 8:
            if (Ram8(kTasksDone) == 3) {
                SetRam16(kRoundStep, kScreenLevel);
                SetRam16(kSubStep, 0);
                SetRam8(kTasksDone, 0);
            }
            break;
        case 0x0A:
            if (TimerOut()) {
                NextSubStep();
                SetRam16(kTimer, 0x40);
                ClearLayers(c);
            }
            break;
        case 0x0C:
            if (BackdropGone()) {
                SetRam16(kRoundStep, kScreenCharacters);
                SetRam8(kChoosing, 3);
                for (int n = 0; n < 2; n++) {
                    SetRam8(Player(n) + kPlayerState, 2);
                    SetRam8(Player(n) + 0x84, 6);
                    SetRam8(Player(n) + 0x85, 0);
                    SetRam8(Player(n) + kComputer, 0);
                    SetRam8(Player(n) + 0xA0, 0);
                }
            }
            ScrollBackdrop();
            break;
        }
        ScrollBackdrop();
    }

    // 006CD2. The choice of level.
    void ShowLevelCursor(M68K &c) {
        for (int level = 0; level < 3; level++) {
            SetBrightness(c, kLevelColours[level], Ram8(kLevelCursor) == level ? kBright : kDim);
        }
    }
    void ChooseLevel(M68K &c) {
        Spend(c, kMenuTime);
        switch (Ram16(kSubStep)) {
        case 0:
            NextSubStep();
            SetRam8(kTasksDone, 0);
            for (const uint32_t row : kLevelColours) {
                SetBrightness(c, row, kDim);
            }
            for (uint16_t n = 0; n < 3; n++) {
                if (!NewTask(c, static_cast<uint16_t>(0x1F00 + n))) {
                    break;
                }
            }
            break;
        case 2:
            if (Ram8(kTasksDone) == 7) {
                NextSubStep();
                SetRam8(kSeconds, kChoiceSeconds);
                SetRam8(kSecondFrames, kFramesASecond);
                Sound(c, kSoundLevelScreen);
                PutWords(c, 1);
            }
            break;
        case 4: {
            const bool out = SecondsOut(kSeconds, kSecondFrames);
            const uint16_t pressed = out ? 0 : MenuPressed(true);
            if (out || (pressed & kButtons)) {
                NextSubStep();
                SetRam16(kTimer, 0x1E);
                Sound(c, kSoundChoose);
                c.d[1] = Ram8(kLevelCursor);
                Sound(c, kSoundLevel);
            } else {
                if (pressed & kStickLeft) {
                    if (Ram8(kLevelCursor) == 0) {
                        SetRam8(kLevelCursor, 0);
                    } else {
                        SetRam8(kLevelCursor, static_cast<uint8_t>(Ram8(kLevelCursor) - 1));
                        Sound(c, kSoundMove);
                    }
                }
                if (pressed & kStickRight) {
                    if (Ram8(kLevelCursor) + 1 >= 3) {
                        SetRam8(kLevelCursor, 2);
                    } else {
                        SetRam8(kLevelCursor, static_cast<uint8_t>(Ram8(kLevelCursor) + 1));
                        Sound(c, kSoundMove);
                    }
                }
            }
            ShowLevelCursor(c);
            ShowHelp(c, Ram8(kLevelCursor), 0xD4, 0x2E);
            break;
        }
        case 6:
            if (TimerOut()) {
                SetRam16(kTimer, 1);
                if ((Ram16(kScrollX) & 0x7F) < 0x40) {
                    NextSubStep();
                    SetRam8(Player(0) + kLevel, Ram8(kLevelCursor));
                    SetRam8(Player(1) + kLevel, Ram8(kLevelCursor));
                    TurnToCharacters(c);
                }
            }
            if (Ram16(kTimer) >= 0x0A) {
                SetBrightness(c, kLevelColours[Ram8(kLevelCursor) == 0 ? 0 : Ram8(kLevelCursor) == 1 ? 1 : 2], Ram16(kTimer) & 1 ? kDim : kBright);
            }
            break;
        case 8:
            if (TimerOut()) {
                NextSubStep();
                SetRam16(kTimer, 0x40);
                ClearLayers(c);
            }
            break;
        case 0x0A:
            if (BackdropGone()) {
                // The level sets how long the ladder is. (When a second player came in during the choice,
                // both choose characters.)
                const uint32_t stages = Ram8(kShortLadder) ? kLastStagesShort : kLastStages;
                SetRam16(kLastStage, Code16(c, stages + 2 * Ram8(kLevelCursor)));
                SetRam16(kRoundStep, Ram8(kModeCursor) ? kScreenCharacters : kScreenCharacter);
            }
            ScrollBackdrop();
            break;
        }
        ScrollBackdrop();
    }

    // The screen of characters comes up
    void DrawCharacterScreen(M68K &c) {
        constexpr struct {
            uint32_t routine, number, at;
        } kPictures[] = {{kDrawPicture, 4, 0x902000}, {kDrawPicture, 5, 0x903000}, {kDrawPictureWide, 0x66, 0x904000},
                         {kDrawPictureWide, 0x67, 0x904400}};
        for (const auto &picture : kPictures) {
            c.d[0] = picture.number;
            c.a[1] = picture.at;
            Call(c, picture.routine);
        }
        c.a[1] = kStageNumberAt;
        c.d[0] = 0;
        Call(c, kShowStageNumber);
    }
    void SetCharacterScreen() {
        SetRam16(kScrollX, 0);
        SetRam16(kScrollY, 0x300);
        SetRam16(kLayerOrder, 0x270E);
        SetRam16(kLayerSetting, 0x2641);
        SetRam8(kTasksDone, 0);
        SetRam16(kScreenSetting, kScreenPortraits);
        SetRam16(kTimer, 8);
    }
    // 00709E. The players' time to choose starts.
    void StartChoosing(M68K &c) {
        if (!TimerOut()) {
            return;
        }
        NextSubStep();
        for (const uint32_t character : kCharacters) {
            SetRam8(character + kChoiceSecondsLeft, kChoiceSeconds);
            SetRam8(character + kChoiceFrames, kFramesASecond);
            SetRam32(character + kChoiceStep, 0);
        }
        SetRam8(kHeld1, 0);
        SetRam8(kHeld2, 0);
        SetRam16(kScreenSetting, kScreenPlain);
        Sound(c, kSoundCharacters);
    }
    // 0070DE. The players choose. A player who comes in meanwhile is given a character to choose too, and
    // the screen becomes the two players' one.
    void ChoosingStep(M68K &c) {
        if (Ram8(kChoosing) != Ram8(kPlayersSeen)) {
            for (int n = 0; n < 2; n++) {
                if (!(Ram8(kPlayersSeen) & (1 << n))) {
                    SetRam8(kCharactersChosen, Ram8(kCharactersChosen) & ~(1 << n));
                    BringIn(c, n);
                }
            }
            SetRam16(kRoundStep, kScreenCharacters);
            SetRam8(kModeCursor, 1);
            SetRam8(kPlayersSeen, Ram8(kChoosing));
        }
        PlayersChoose(c);
    }
    // 007124. The choices are taken: on to the opponent's being shown, or with two people to the match.
    void TakeChoices() {
        if (!TimerOut()) {
            return;
        }
        for (int n = 0; n < 2; n++) {
            SetRam8(Player(n) + kCharacter, Ram8(kChosen[n]));
            SetRam8(kCharacters[n] + kCharacter, Ram8(kChosen[n]));
            SetRam8(Player(n) + 0xF6, Ram8(kChosen[n]));
        }
        SetRam16(kSubStep, 0);
        SetRam8(kVars + 0xAB, 0);
        if ((Ram8(kChoosing) | Ram8(kPlaying)) != 3) {
            SetRam16(kRoundStep, kScreenOpponent);
        } else {
            SetRam16(kMatchStep, static_cast<uint16_t>(Ram16(kMatchStep) + 2));
            SetRam16(kRoundStep, 0);
            SetRam8(kHeld1, 1);
            SetRam8(kHeld2, 1);
        }
    }
    // 007242 (and the end of 0073F2). With handicaps the levels' pictures come in; without, the choice is over.
    void StartHandicaps(M68K &c, bool handicaps, uint16_t stepWithout) {
        if (!handicaps) {
            SetRam16(kSubStep, stepWithout);
            return;
        }
        NextSubStep();
        SetRam8(kCharactersChosen, 0);
        SetRam8(kTasksDone, 0);
        for (int n = 0; n < 2; n++) {
            const uint32_t task = NewTask(c, static_cast<uint16_t>(0x2A00 + n));
            if (!task) {
                return;
            }
            SetRam32(task + 0x32, kChosen[n] + kHandicapChosen);
        }
    }
    // 007292. Their cursors come in, and the players' time to choose a level starts.
    void StartChoosingHandicaps(M68K &c) {
        if (Ram8(kTasksDone) != 3) {
            return;
        }
        NextSubStep();
        for (int n = 0; n < 2; n++) {
            const uint32_t task = NewTask(c, static_cast<uint16_t>(0x2A02 + n));
            if (!task) {
                return;
            }
            SetRam32(task + 0x32, kChosen[n] + kHandicapChosen);
        }
        for (int n = 0; n < 2; n++) {
            SetRam8(kCharacters[n] + kChoiceStep, static_cast<uint8_t>(Ram8(kCharacters[n] + kChoiceStep) + 2));
            SetRam8(kChosen[n] + kHandicapChosen, 2);
            SetRam8(kCharacters[n] + kChoiceSecondsLeft, kHandicapSeconds);
            SetRam8(kCharacters[n] + kChoiceFrames, kFramesASecond);
        }
    }
    // 007304
    void AfterHandicaps() {
        NextSubStep();
        SetRam16(kTimer, 0x3C);
        SetRam8(kTasksDone, 0);
    }
    // The steps the two players' screen and the screen for a player joining have alike
    void CharactersStep(M68K &c) {
        switch (Ram16(kSubStep)) {
        case 2: StartChoosing(c); break;
        case 4: ChoosingStep(c); break;
        case 8: StartChoosingHandicaps(c); break;
        case 0x0A: PlayersChoose(c); break;
        case 0x0C: AfterHandicaps(); break;
        case 0x0E: TakeChoices(); break;
        }
    }

    // 006FCC. One player chooses a character.
    void ChooseCharacter(M68K &c) {
        switch (Ram16(kSubStep)) {
        case 0:
            if (Ram8(kTasksDone) == kTasksGone && TimerOut()) {
                NextSubStep();
                SetCharacterScreen();
                DrawCharacterScreen(c);
                SetRam8(kCharactersChosen, Ram8(kPlaying) | (Ram8(kChoosing) ^ 3));
                SetRam8(kPlayersSeen, Ram8(kChoosing));
                for (int n = 0; n < 2; n++) {
                    if (Ram8(kChoosing) & (1 << n)) {
                        BringIn(c, n);
                    }
                }
            }
            break;
        case 2: StartChoosing(c); break;
        case 4: ChoosingStep(c); break;
        case 6: TakeChoices(); break;
        }
        ShowCharacters(c);
    }
    // 00718A. Two players choose their characters, and with handicaps their levels.
    void ChooseCharacters(M68K &c) {
        switch (Ram16(kSubStep)) {
        case 0:
            if (Ram8(kTasksDone) == kTasksGone && TimerOut()) {
                NextSubStep();
                SetCharacterScreen();
                DrawCharacterScreen(c);
                SetRam8(kCharactersChosen, 0);
                SetRam8(kPlayersSeen, Ram8(kChoosing));
                BringIn(c, 0);
                BringIn(c, 1);
            }
            break;
        case 6: StartHandicaps(c, Ram8(kHandicaps) != 0, 0x0C); break;
        default: CharactersStep(c); break;
        }
        ShowCharacters(c);
    }
    // 007314. A player who joins a game that is on, or goes on after losing, chooses a character.
    void ChooseOnJoining(M68K &c) {
        const uint8_t in = Ram8(kPlaying) | Ram8(kChoosing);
        switch (Ram16(kSubStep)) {
        case 0:
            NextSubStep();
            SetCharacterScreen();
            SetRam8(kPortraitsStay, 1);
            PortraitFrames(c);
            DrawCharacterScreen(c);
            SetRam8(kCharactersChosen, Ram8(kPlaying) | (Ram8(kChoosing) ^ 3));
            SetRam8(kPlayersSeen, Ram8(kChoosing));
            for (int n = 0; n < 2; n++) {
                if (in & (1 << n)) {
                    BringIn(c, n);
                }
            }
            break;
        case 6: StartHandicaps(c, Ram8(kHandicaps) != 0 && in == 3, 0x0E); break;
        default: CharactersStep(c); break;
        }
        ShowCharacters(c);
    }

    // 007AC6. The ladder is shown: against the first opponents the stage's place runs through the
    // characters and stops at the opponent; a button cuts it short.
    void LeaveForMatch() {
        SetRam16(kMatchStep, static_cast<uint16_t>(Ram16(kMatchStep) + 2));
        SetRam16(kRoundStep, 0);
        SetRam16(kSubStep, 0);
        SetRam8(kHeld1, 1);
        SetRam8(kHeld2, 1);
    }
    void ShowOpponentScreen(M68K &c) {
        switch (Ram16(kSubStep)) {
        case 0:
            NextSubStep();
            SetRam8(kTasksDone, 0);
            SetRam16(kTimer, 0x0A);
            SetRam8(kHeld1, 1);
            SetRam8(kHeld2, 1);
            SmallPortraitTasks(c);
            break;
        case 2:
            if (Ram8(kTasksDone) == kTasksGone && TimerOut()) {
                NextSubStep();
                SetRam16(kScrollX, 0);
                SetRam16(kScrollY, 0x300);
                SetRam8(kTasksDone, 0);
                SetRam8(kOpponentShowing, 1);
                SetRam16(kScreenSetting, kScreenPortraits);
                SetRam16(kTimer, 8);
                if (Ram16(kStage) != kLastStageNumber) {
                    BringBack(c, Ram8(kChoosing) & 1 ? 1 : 0);
                }
            }
            break;
        case 4:
            if (TimerOut()) {
                NextSubStep();
                SetRam16(kTimer, 0x78);
                DrawLadder(c, 0);
                c.a[1] = kStageNumberAt;
                c.d[0] = (Ram16(kStage) == Ram16(kLastStage) ? 9u : Ram16(kStage)) + 1;
                Call(c, kShowStageNumber);
                if (Ram8(kLevelCursor) && Ram16(kStage) == kLastStageNumber) {
                    SetRam16(kSubStep, 0x0A);
                }
                SetRam16(kScreenSetting, kScreenPlain);
            }
            break;
        case 6:
            if (AnyButton(c)) {
                Sound(c, kSoundChoose);
                LeaveForMatch();
                ShowStageOpponent(c, 0);
            } else if (TimerOut()) {
                NextSubStep();
                SetRam16(kTimer, 0x78);
                Sound(c, kSoundChoose);
                ShowStageOpponent(c, 0);
                Sound(c, kSoundDone);
            } else {
                SpinStage(c);
            }
            break;
        case 8:
            if (AnyButton(c) || TimerOut()) {
                LeaveForMatch();
                ShowStageOpponent(c, 0);
            }
            break;
        case 0x0A: // (the last stage, but for the easy level: the whole ladder runs)
            if (AnyButton(c)) {
                Sound(c, kSoundChoose);
                LeaveForMatch();
                ShowLastOpponent(c);
            } else if (TimerOut()) {
                NextSubStep();
                SetRam16(kTimer, 0x78);
                Sound(c, kSoundChoose);
                ShowLastOpponent(c);
                Sound(c, kSoundDone);
            } else {
                SpinLadder(c);
            }
            break;
        case 0x0C:
            if (AnyButton(c) || TimerOut()) {
                LeaveForMatch();
                ShowLastOpponent(c);
            }
            break;
        }
        ShowCharacters(c);
    }

    // 0069E0
    void Choose(M68K &c) {
        switch (Ram16(kRoundStep)) {
        case kScreenMode: ChooseMode(c); break;
        case kScreenLevel: ChooseLevel(c); break;
        case kScreenCharacter: ChooseCharacter(c); break;
        case kScreenCharacters: ChooseCharacters(c); break;
        case kScreenJoining: ChooseOnJoining(c); break;
        case kScreenOpponent: ShowOpponentScreen(c); break;
        }
    }

    // 007452. The screen fades out, and the match is set up.
    void BeforeLeave(M68K &c) {
        switch (Ram16(kRoundStep)) {
        case 0:
            c.d[0] = 0;
            Call(c, kStartFade);
            if (!c.flagZ) {
                SetRam16(kRoundStep, 2);
                SetRam8(kVars + 0x8C, 0xFF);
            }
            break;
        case 2:
            if (Ram8(kVars + 0x290A)) {
                SetRam16(kRoundStep, 4);
                SetRam8(kCharacters[0] + kOn, 0);
                SetRam8(kCharacters[1] + kOn, 0);
                Call(c, kEndFade);
                Call(c, kNextRoundPicture);
            }
            break;
        case 4:
            if (!Ram8(kVars + 0x8C)) {
                SetRam16(kGameStep, static_cast<uint16_t>(Ram16(kGameStep) + 2));
                SetRam16(kMatchStep, 0);
                SetRam16(kRoundStep, 0);
                SetRam16(kSubStep, 0);
                SetRam8(kVars + 0xAB, 0);
                SetRam8(kPlaying, Ram8(kPlaying) | Ram8(kChoosing));
                for (int n = 0; n < 2; n++) {
                    if (Ram8(kPlaying) & (1 << n)) {
                        SetRam8(Player(n) + kComputer, 0);
                    }
                }
                Call(c, kSetOpponent);
                SetRam8(kChoosing, 0);
            }
            break;
        }
    }

    // 0068B4. A frame of what comes before a match.
    void BeforeMatch(M68K &c) {
        switch (Ram16(kMatchStep)) {
        // (kStepSetUp, 0068D8, waits for the frame's interrupt part of the way through: the game's own code
        // runs then, see RegisterSelect. Of what it does only the backdrop is here.)
        case kStepChoosing: Choose(c); break;
        case kStepLeaving: BeforeLeave(c); break;
        }
        for (const uint32_t routine : kFrameWork) {
            Call(c, routine);
        }
    }

    bool NotSettingUp(M68K &) {
        return Ram16(kMatchStep) != kStepSetUp;
    }
    void ShowLadderOnOtherPage(M68K &c) {
        DrawLadder(c, 0x800);
    }
    void ShowStageOpponentOnOtherPage(M68K &c) {
        ShowStageOpponent(c, 0x800);
    }
} // namespace

void RegisterSelect() {
    TheGame().features.push_back({"hidden_characters", "Game", "Hidden characters",
                                  "At the choice of characters a kick chooses one of the three hidden characters outright, with no "
                                  "code: light kick Devilot, medium kick Akuma, hard kick Dan. (The board's codes still work.) "
                                  "A person playing one of them sends three quarters of the usual attack, as on the board. "
                                  "For player 1, player 2 or both.",
                                  0, {"Off", "Player 1", "Player 2", "Both"}});
    // (the last number is the time a call is charged)
    constexpr uint32_t kScratch = kFreeD0 | kFreeD1 | kFreeD2 | kFreeD3 | kFreeD4 | kFreeD5 | kFreeD6 | kFreeD7 | kFreeA0 | kFreeA1 |
                                  kFreeA2 | kFreeA3 | kFreeA4 | kFreeA6 | kFreeFlags;
    AddRewrite(0x0068B4, "BeforeMatch", BeforeMatch, kScratch, 1);
    RewriteWhen(0x0068B4, NotSettingUp);
    AddRewrite(0x008470, "DrawBackdrop", DrawBackdrop, kScratch, 1);
    AddRewrite(0x0069E0, "Choose", Choose, kScratch, 1);
    AddRewrite(0x007452, "BeforeLeave", BeforeLeave, kScratch, 1);
    AddRewrite(0x0069F8, "ChooseMode", ChooseMode, kScratch, 1);
    AddRewrite(0x006CD2, "ChooseLevel", ChooseLevel, kScratch, 1);
    AddRewrite(0x006FCC, "ChooseCharacter", ChooseCharacter, kScratch, 1);
    AddRewrite(0x00718A, "ChooseCharacters", ChooseCharacters, kScratch, 1);
    AddRewrite(0x007314, "ChooseOnJoining", ChooseOnJoining, kScratch, 1);
    AddRewrite(0x007AC6, "ShowOpponentScreen", ShowOpponentScreen, kScratch, 1);
    // What the screens after a match use of these
    AddRewrite(0x0077BE, "ShowCharacters", ShowCharacters, kScratch, 1);
    AddRewrite(0x007C7E, "ShowLadderOnOtherPage", ShowLadderOnOtherPage, kScratch, 1);
    AddRewrite(0x007D0C, "SpinLadder", SpinLadder, kScratch, 1);
    AddRewrite(0x007D94, "SpinStage", SpinStage, kScratch, 1);
    AddRewrite(0x007DF2, "ShowAfterLadder", ShowAfterLadder, kScratch, 1);
    AddRewrite(0x007F46, "ShowStageOpponentOnOtherPage", ShowStageOpponentOnOtherPage, kScratch, 1);
    AddRewrite(0x0082EA, "BringBackAfterMatch", BringBackAfterMatch, kScratch, 1);
}

} // namespace cps2recomp
