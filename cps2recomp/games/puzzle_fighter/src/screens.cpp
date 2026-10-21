// SPDX-License-Identifier: GPL-3.0-only
// Copyright (C) 2026 Tommy McLeroy
//
// Super Puzzle Fighter II Turbo: the screens around a match, written again as C++ (rewrites: lift.h). They
// are steps of kMatchStep, each with its own steps in kRoundStep (and some of those theirs in kSubStep):
//
//   MatchIntro (00A3A2)      the two characters face each other before the first round
//   AfterMatch (00ABCC)      the winner speaks, with the winner's score, best chain and biggest break
//   StageInterlude (00ADD2)  once in a game, after the fourth stage: a scene
//   LadderAfterMatch (00AF50) the ladder again, with the next opponent picked out
//
// Much of what they show is done by tasks and by routines not written again (the pictures, the fades, the
// winner's words); this is the order of things, the waits, and what a button cuts short.
#include "spf2.h"

#include <initializer_list>
#include <utility>

namespace cps2recomp {

namespace {
    using namespace spf2;

    // --- the game's variables used here ---
    constexpr uint32_t kStepTimer = kVars + 0x0E, kSubTimer = kVars + 0x12; // words: frames the step still has
    constexpr uint32_t kInterrupted = kVars + 0x8A;  // byte (round.cpp)
    constexpr uint32_t kFading = kVars + 0x8C;       // byte: set when a fade is started, cleared when it is over
    constexpr uint32_t kFadeDone = kVars + 0x290A;   // byte
    constexpr uint32_t kFadeSetting = kVars + 0x290C; // word
    constexpr uint32_t kHeld1 = kVars + 0x8F, kHeld2 = kVars + 0xBD; // bytes: set while the screen changes
    constexpr uint32_t kScreenPhase = kVars + 0x14D; // byte: tasks look at it
    constexpr uint32_t kScreenSetting = kVars + 0x20; // word: for the video hardware
    constexpr uint16_t kScreenPlain = 0x90C0, kScreenPortraits = 0x9280, kScreenThird = 0x92A0;
    constexpr uint32_t kWinner = kVars + 0x13A;      // word: the low half of the winner's record's address
    constexpr uint32_t kWinners = kVars + kRoundWinners;
    constexpr uint32_t kGameEndsHere = kVars + 0xB8; // byte (match.cpp)
    constexpr uint32_t kStage = kVars + 0x112, kLastStage = kVars + 0x114; // words (match.cpp)
    constexpr uint16_t kLastStageNumber = 8, kInterludeStage = 4;
    constexpr uint32_t kShortLadder = kVars + 0xBF;  // byte (select.cpp)
    constexpr uint32_t kLevelCursor = 0xFF0C8C;      // byte: the level chosen (select.cpp)
    constexpr uint32_t kLastOpponentIn = kVars + 0x138; // byte
    constexpr uint32_t kInterludeSeen = kVars + 0x148; // word
    constexpr uint32_t kOtherLanguage = kVars + 0x90; // word
    constexpr uint32_t kCharacters[2] = {kVars + 0xB00, kVars + 0xC00}; // the two characters on the screen
    constexpr uint32_t kOn = 1;                      // byte, in a character and in a player's record
    constexpr uint32_t kDecimal = 0xFF1358;          // long: what kToDecimal makes of a number, a digit a nibble
    // In a player's record
    constexpr uint32_t kBestChain = 0xAF, kBestBroken = 0xB1; // bytes: this match's (match.cpp)
    constexpr uint32_t kPlayMinutes = kVars + 0x108, kPlaySeconds = kVars + 0x109; // bytes (round.cpp: CountPlayTime)

    // --- video memory (select.cpp) ---
    constexpr uint32_t kDown = 4, kAcross = 0x80;
    constexpr uint32_t kBlank = 0x00200000;
    constexpr uint32_t kTotalsAt[2] = {0x902E30, 0x902130}; // by the winner's number: the totals go on the loser's side
    constexpr uint32_t kStageNumberAt = 0x904A0C;
    constexpr uint32_t kDigitTiles = 0x00B4B0;       // words
    constexpr uint32_t kPictures = 0x084560, kColours = 0x0A0000;

    // --- the game's routines used here that are not written again ---
    constexpr uint32_t kNewTask = 0x02DA46, kNewTaskBehind = 0x02DAC6, kStartFade = 0x006634, kEndFade = 0x006650,
                       kNextRoundPicture = 0x02D7CC, kClearText = 0x02D8C8, kCopyColours = 0x020F2C, kToDecimal = 0x02D0AE,
                       kSetActNow = 0x0143B2, kShowStageNumber = 0x00BB8C, kDrawPicture = 0x0179E4, kRandom = 0x02DB02,
                       kModesPlayersPressed = 0x00B992, kPlayersInPressed = 0x00B9B2, kSoundCommand3 = 0x02B216;
    constexpr uint32_t kSoundChoose = 0x02B5D4, kSoundDone = 0x02B88C, kSoundLast = 0x02C74C;
    // (select.cpp)
    constexpr uint32_t kShowCharacters = 0x0077BE, kShowLadderOnOtherPage = 0x007C7E, kSpinLadder = 0x007D0C, kSpinStage = 0x007D94,
                       kShowAfterLadder = 0x007DF2, kShowStageOpponentOnOtherPage = 0x007F46, kBringBackAfterMatch = 0x0082EA;
    // (the time the game's code takes, about)
    constexpr uint32_t kTotalsTime = 6000, kClearTotalsTime = 5000, kStepTime = 200;

    void Add16(uint32_t at, int amount) {
        SetRam16(at, static_cast<uint16_t>(Ram16(at) + amount));
    }
    void NextStep() {
        Add16(kRoundStep, 2);
    }
    void NextSubStep() {
        Add16(kSubStep, 2);
    }
    // Counts frames down: true once the count has gone below nought
    bool Below(uint32_t timer) {
        Add16(timer, -1);
        return (Ram16(timer) & 0x8000) != 0;
    }
    // Counts frames down: true when the count comes to nought
    bool Out(uint32_t timer) {
        Add16(timer, -1);
        return Ram16(timer) == 0;
    }
    void Calls(M68K &c, std::initializer_list<uint32_t> routines) {
        for (const uint32_t routine : routines) {
            Call(c, routine);
        }
    }
    bool Pressed(M68K &c, uint32_t routine = kModesPlayersPressed) {
        Call(c, routine);
        return !c.flagZ;
    }
    // A fade is asked for: true once it has started
    bool StartFade(M68K &c, uint32_t kind) {
        c.d[0] = kind;
        Call(c, kStartFade);
        return !c.flagZ;
    }
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
    void ClearSteps(uint32_t from) {
        for (uint32_t at = from; at < kVars + 0x18; at += 4) {
            SetRam32(at, 0);
        }
    }
    // The match's next screen
    void NextScreen() {
        Add16(kMatchStep, 2);
        ClearSteps(kRoundStep);
    }
    void StageNumber(M68K &c, uint32_t number) {
        c.d[0] = number;
        c.a[1] = kStageNumberAt;
        Call(c, kShowStageNumber);
    }
    // The fade out that ends a screen: its three steps. True when it is over.
    void FadeOutStep(M68K &c, uint16_t setting) {
        if (StartFade(c, 0)) {
            NextStep();
            SetRam8(kFading, 0xFF);
            SetRam16(kFadeSetting, setting);
        }
    }
    void FadeEndStep(M68K &c) {
        if (Ram8(kFadeDone)) {
            NextStep();
            Call(c, kEndFade);
            Call(c, kNextRoundPicture);
        }
    }

    // --- before the first round ---

    // 00A548. What the two characters stand on.
    void IntroStands(M68K &c) {
        for (int n = 0; n < 2; n++) {
            const uint32_t task = NewTask(c, 0x2200);
            if (!task) {
                return;
            }
            SetRam16(task + 0x10, n ? 0x130 : 0x50);
            SetRam16(task + 0x14, 0x40);
            SetRam16(task + 0x0C, 0xFF15);
            SetRam16(task + 0x32, static_cast<uint16_t>(Player(n)));
        }
    }
    void IntroShown() {
        NextSubStep();
        SetRam16(kScreenSetting, kScreenPortraits);
        SetRam8(kScreenPhase, 1);
    }
    void IntroStep(M68K &c) {
        const auto flash = [&](uint16_t setting) {
            NextSubStep();
            SetRam16(kScreenSetting, setting);
        };
        switch (Ram16(kSubStep)) {
        case 0:
        case 2: NextSubStep(); break;
        case 4: // the picture is made
            flash(kScreenPlain);
            Calls(c, {0x01788C, 0x016C0C, 0x016D16, 0x020D78});
            c.a[0] = 0x0AB7E0;
            c.a[1] = 0x90C2A0;
            c.d[7] = 0;
            Call(c, kCopyColours);
            c.a[0] = 0x0A0C20;
            c.d[7] = 0;
            Call(c, kCopyColours);
            Call(c, 0x01674A);
            break;
        case 6: { // once the fade in is over, the fields and the players' names come on
            if (Ram8(kFading)) {
                break;
            }
            NextSubStep();
            SetRam16(kSubTimer, 0x20);
            c.d[0] = 0;
            SetRam8(kHeld1, 0);
            SetRam8(kHeld2, 0);
            SetRam8(kScreenPhase, 0);
            if (!NewTask(c, 0x0400) || !NewTask(c, 0x0401)) {
                break;
            }
            for (int n = 0; n < 2; n++) {
                const uint32_t task = NewTask(c, 0x0501);
                if (!task) {
                    break;
                }
                SetRam8(task + 0x0A, static_cast<uint8_t>(n));
                SetRam16(task + 0x32, static_cast<uint16_t>(Player(n)));
            }
            break;
        }
        case 8: // a wait, which a button cuts short; then the characters come on
            if (Pressed(c)) {
                NextSubStep();
                IntroStands(c);
                IntroShown();
            } else if (Below(kSubTimer)) {
                NextSubStep();
                SetRam16(kSubTimer, 0x3F);
                SetRam16(kVars + 0x48, kScreenPortraits);
                IntroStands(c);
                c.d[6] = 0x3FFF3FFF;
                c.a[1] = 0x90C340;
                c.d[7] = 5;
                Call(c, 0x02107E);
                c.a[1] = 0x90C800;
                Call(c, 0x02107C);
                Call(c, 0x02107C);
            }
            break;
        case 0x0A:
            if (Pressed(c) || Below(kSubTimer)) {
                IntroShown();
            }
            break;
        case 0x0C: flash(kScreenThird); break; // (the screen flashes)
        case 0x0E: flash(kScreenPortraits); break;
        case 0x10: flash(kScreenThird); break;
        case 0x12:
            flash(kScreenPortraits);
            Call(c, 0x021068);
            break;
        case 0x14:
            flash(kScreenPlain);
            SetRam16(kSubTimer, 0x78);
            break;
        case 0x16: // two seconds, or a button
            if (Pressed(c) || Below(kSubTimer)) {
                NextSubStep();
                SetRam16(kSubTimer, 0x18);
                SetRam8(kScreenPhase, 2);
            }
            break;
        case 0x18:
            if (Below(kSubTimer)) {
                NextSubStep();
            }
            break;
        case 0x1A: // on to the round
            c.d[0] = 0;
            NextScreen();
            SetRam8(kScreenPhase, 0);
            SetRam8(kHeld1, 1);
            SetRam8(kHeld2, 0xFF);
            Calls(c, {0x020DAE, 0x020E18});
            break;
        }
    }
    // 00A3A2
    void MatchIntro(M68K &c) {
        Spend(c, kStepTime);
        if (Ram16(kRoundStep) == 0) {
            if (StartFade(c, 1)) {
                c.d[0] = 0;
                NextStep();
                SetRam32(kSubStep, 0);
                SetRam32(kVars + 0x14, 0);
                SetRam8(kScreenPhase, 0);
                SetRam16(kFadeSetting, 0);
                SetRam8(kFading, 0xFF);
                Call(c, 0x02BC88);
            }
            return;
        }
        if (Ram8(kInterrupted)) {
            SetRam16(kScreenSetting, kScreenPlain);
        } else {
            IntroStep(c);
        }
        Calls(c, {0x016A3A, 0x016B78, 0x0178A8, 0x01495A});
    }

    // --- after a match ---

    // 00B48E, 00B466. A digit in tall figures; a number of eight decimal digits, a digit a nibble, without
    // the noughts before it. A1 goes along as the game's does.
    void PutDigit(M68K &c, uint32_t digit) {
        const uint16_t tile = Code16(c, kDigitTiles + 2 * digit);
        c.Write(c.a[1], 1, tile);
        c.Write(c.a[1] + 2, 1, 0x16);
        c.Write(c.a[1] + kDown, 1, static_cast<uint16_t>(tile + 0x10));
        c.Write(c.a[1] + kDown + 2, 1, 0x16);
    }
    void PutNumber(M68K &c, uint32_t digits) {
        bool begun = false;
        for (int place = 7; place > 0; place--, c.a[1] += kAcross) {
            const uint32_t digit = (digits >> (4 * place)) & 0x0F;
            if (digit || begun) {
                begun = true;
                PutDigit(c, digit);
            }
        }
        PutDigit(c, digits & 0x0F);
    }
    void ToDecimal(M68K &c, uint32_t number) {
        c.d[0] = number;
        Call(c, kToDecimal);
    }
    // 00B372. A person who has won is shown the match's totals: the score, the best chain, the biggest
    // break, and the time played.
    void ShowTotals(M68K &c) {
        const uint32_t winner = 0xFF0000 | Ram16(kWinner);
        c.a[4] = 0xFFFF0000 | Ram16(kWinner);
        if (Ram8(winner + kComputer)) {
            return;
        }
        // The panel they are written on
        uint32_t from = kPictures + static_cast<int16_t>(Mem16(c, kPictures + 0x12)) + (Ram16(kOtherLanguage) ? 0x10 : 0);
        from = Mem32(c, from + 4);
        const int rows = Mem16(c, from + 4) + 1;
        from += 6;
        uint32_t to = kTotalsAt[Ram8(winner + kPlayerNumber) ? 1 : 0];
        for (int row = 0; row < rows; row++, to += kAcross) {
            for (int tile = 0; tile < 8; tile++, from += 4) {
                c.Write(to + 4 * tile, 2, Mem32(c, from));
            }
        }
        c.a[0] = to - 0x200;
        ToDecimal(c, Ram32(winner + kScore));
        c.a[1] = c.a[0];
        PutNumber(c, Ram32(kDecimal));
        ToDecimal(c, Ram8(winner + kBestChain));
        c.a[1] = c.a[0] + 8;
        PutNumber(c, Ram32(kDecimal));
        ToDecimal(c, Ram8(winner + kBestBroken));
        c.a[1] = c.a[0] + 0x10;
        PutNumber(c, Ram32(kDecimal));
        // The time: minutes, a colon, seconds in two digits
        ToDecimal(c, Ram8(kPlayMinutes));
        c.a[1] = c.a[0] - 0x168;
        PutNumber(c, Ram8(kDecimal + 3));
        c.a[1] += kAcross;
        c.Write(c.a[1], 2, 0x04DD0014);
        ToDecimal(c, Ram8(kPlaySeconds));
        c.a[1] += kAcross;
        PutDigit(c, Ram8(kDecimal + 3) >> 4);
        c.a[1] += kAcross;
        PutDigit(c, Ram8(kDecimal + 3) & 0x0F);
        Spend(c, kTotalsTime);
    }
    // 00B4D0. The totals and the characters are taken off.
    void ClearTotals(M68K &c) {
        c.a[0] = 0x0A0BE0;
        c.a[1] = 0x90C0E0;
        c.d[7] = 0;
        Call(c, kCopyColours);
        for (int n = 0; n < 2; n++) {
            SetRam8(Player(n) + kOn, 0);
            SetRam8(kCharacters[n] + kOn, 0);
        }
        uint32_t to = 0x902130;
        for (int row = 0; row < 44; row++, to += kAcross) {
            for (int tile = 0; tile < 8; tile++) {
                c.Write(to + 4 * tile, 2, kBlank);
            }
        }
        Spend(c, kClearTotalsTime);
    }
    // 00B522. The colours of the loser's character (A6, which stays on the screen) and of the panel.
    void LoserColours(M68K &c) {
        const uint32_t character = c.a[6] & 0xFFFFFF;
        c.a[0] = kColours + ((Ram8(kVars + 0x154) * 2u + Ram8(kVars + 0x155)) << 7);
        c.a[1] = Ram8(character + kPlayerNumber) ? 0x90C200 : 0x90C180;
        c.d[7] = 3;
        Call(c, kCopyColours);
        c.a[0] = 0x0A0BE0;
        c.a[1] = 0x90C140;
        c.d[7] = 0;
        Call(c, kCopyColours);
        c.a[0] = 0x0A0C20;
        c.d[7] = 0;
        Call(c, kCopyColours);
    }
    // 00B2A6. (Something a person earns once, by a count that matches; what it shows is not looked into.)
    void MaybeAward(M68K &c) {
        const uint32_t winner = 0xFF0000 | Ram16(kWinner);
        if (!Ram8(kVars + 0xCE) || Ram8(kVars + 0xD6)) {
            return;
        }
        c.a[0] = 0xFFFF0000 | Ram16(kWinner);
        if (Ram8(winner + kComputer)) {
            return;
        }
        if (Ram8(kVars + 0x141) ? Ram8(kVars + 0x14E) != Ram8(winner + 0xA2) : Ram8(kVars + 0x144) != Ram8(kVars + 0x14F)) {
            return;
        }
        SetRam8(kVars + 0xD6, 0xFF);
        if (Ram8(kVars + 0xCF) & 0x80) {
            return;
        }
        SetRam8(kVars + 0xD0, static_cast<uint8_t>(Ram8(kVars + 0xD0) + 1));
        SetRam16(kVars + 0x136, 0x31);
        Calls(c, {0x01E33C, 0x01F18C});
    }
    // 00ADB0. Away from the match's screens: the next match is set up.
    void LeaveMatch() {
        SetRam32(kMatchStep, 0);
        SetRam32(kRoundStep, 0);
        SetRam8(kHeld1, 1);
        SetRam8(kHeld2, 0xFF);
        SetRam8(Player(0) + kOn, 0);
        SetRam8(Player(1) + kOn, 0);
        SetRam8(kVars + 0x133, 0);
    }

    // 00ABCC. The winner speaks.
    void AfterMatch(M68K &c) {
        Spend(c, kStepTime);
        switch (Ram16(kRoundStep)) {
        case 0: { // once the loser's field has gone: the winner's picture and words
            c.a[2] = 0xFFFF0000 | Ram16(kWinner);
            const uint32_t winner = 0xFF0000 | Ram16(kWinner);
            if (Ram8(kVars + 0x581C) < 2) {
                break;
            }
            // (two tasks, the picture in the one further on in memory; the game does not look whether it got them)
            Call(c, kNewTask);
            c.a[3] = c.a[4];
            Call(c, kNewTask);
            if (c.a[4] >= static_cast<uint32_t>(static_cast<int32_t>(static_cast<int16_t>(c.a[3])))) {
                std::swap(c.a[3], c.a[4]);
            }
            const uint32_t picture = c.a[3] & 0xFFFFFF, words = c.a[4] & 0xFFFFFF;
            SetRam8(picture, static_cast<uint8_t>(Ram8(picture) + 1));
            SetRam16(picture + 2, 0x0500);
            SetRam8(picture + 0x0A, Ram8(winner + kPlayerNumber));
            SetRam16(picture + 0x32, Ram16(kWinner));
            SetRam8(words, static_cast<uint8_t>(Ram8(words) + 1));
            SetRam16(words + 2, 0x0A00);
            SetRam8(words + 0x0A, Ram8(winner + kPlayerNumber));
            SetRam16(words + 0x32, Ram16(kWinner));
            NextStep();
            SetRam16(kStepTimer, 0x30);
            c.d[0] = 0;
            for (const uint32_t cleared : {0x10Fu, 0x14Du, 0x133u, 0x118u}) {
                SetRam8(kVars + cleared, 0);
            }
            SetRam8(kHeld1, 1);
            SetRam8(kHeld2, 0xFF);
            SetRam8(kVars + 0x10D, 3);
            Call(c, 0x02BCA8);
            c.a[6] = kCharacters[Ram8(kWinners) & 1 ? 1 : 0];
            c.d[0] = 0x1E;
            Call(c, kSetActNow);
            LoserColours(c);
            break;
        }
        case 2: { // a wait, which a button cuts short; then the totals
            if (!Below(kStepTimer) && !Pressed(c, kPlayersInPressed)) {
                break;
            }
            NextStep();
            SetRam8(kScreenPhase, 0xFF);
            SetRam8(kHeld1, Ram8(Var(c, kMode)) ? 0 : 1);
            if (const uint32_t task = NewTask(c, 0x2500)) {
                SetRam16(task + 0x0C, 0xFF02);
            }
            MaybeAward(c);
            if (const uint32_t task = NewTask(c, 0x0000, kNewTaskBehind)) {
                SetRam16(task + 0x1A, Ram16(kVars + 0x136));
                Call(c, 0x029DDA);
            }
            ShowTotals(c);
            break;
        }
        case 4:
            NextStep();
            SetRam16(kStepTimer, 0xD2);
            break;
        case 6: // three and a half seconds, or a button: then the game is over, or goes on
            if (Ram8(kInterrupted)) {
                ClearTotals(c);
                break;
            }
            if (!Pressed(c, kPlayersInPressed) && !Below(kStepTimer)) {
                break;
            }
            SetRam8(kVars + 0x118, 0xFF);
            ClearTotals(c);
            if (Ram8(kGameEndsHere)) {
                Add16(kGameStep, 2);
                c.d[0] = 0;
                ClearSteps(kMatchStep);
                SetRam8(kScreenPhase, 0);
            } else {
                NextStep();
            }
            SetRam8(kHeld1, 1);
            Call(c, kClearText);
            break;
        case 8: FadeOutStep(c, 0xFF03); break;
        case 0x0A: FadeEndStep(c); break;
        case 0x0C:
            if (!Ram8(kFading)) {
                c.d[0] = 0;
                NextScreen();
            }
            break;
        }
        Calls(c, {0x016A3A, 0x016B78, 0x016BCA, 0x0178A8, 0x01495A});
    }

    // 00ADD2. After the fourth stage (once a game, and not on the short ladder) a scene is shown; any
    // other time this goes straight on.
    void StageInterlude(M68K &c) {
        constexpr uint32_t kShowScene = 0x0210C0, kSceneKinds = 0x00AE7A;
        Spend(c, kStepTime);
        switch (Ram16(kRoundStep)) {
        case 0:
            if (Ram8(kShortLadder) || Ram16(kStage) != kInterludeStage || Ram16(kInterludeSeen)) {
                c.d[0] = 0;
                NextScreen();
                break;
            }
            if (!StartFade(c, 1)) {
                break;
            }
            NextStep();
            SetRam16(kStepTimer, 3);
            SetRam8(kInterludeSeen, 0xFF);
            SetRam16(kScreenSetting, kScreenPlain);
            SetRam8(kFading, 0xFF);
            c.d[0] = 0;
            SetRam16(kFadeSetting, 0);
            SetRam8(kVars + 0x125, 0);
            SetRam8(kVars + 0x105, 0);
            SetRam8(kVars + 0x12A, 0);
            Call(c, kRandom);
            SetRam8(kVars + 0x120, Code8(c, kSceneKinds + (c.d[0] & 0x0F))); // (one of six scenes, by chance)
            Calls(c, {kNextRoundPicture, 0x020E18, 0x0214A6, 0x020DE2, 0x02BCC8});
            break;
        case 2:
            if (Below(kStepTimer)) {
                NextStep();
                SetRam16(kStepTimer, 0x1B);
                Calls(c, {0x01788C, 0x0178A8, 0x01674E, 0x020ECC, 0x020EFC});
                c.d[0] = Ram16(kOtherLanguage) ? 1 : 0;
                Call(c, 0x0057A4);
            }
            Call(c, kShowScene);
            break;
        case 4:
            if (Below(kStepTimer)) {
                NextStep();
                SetRam16(kStepTimer, 0x1A4);
            }
            Call(c, kShowScene);
            break;
        case 6: // seven seconds, or a button
            if ((Below(kStepTimer) || Pressed(c)) && StartFade(c, 0)) {
                NextStep();
                SetRam8(kFading, 0xFF);
                c.d[0] = 0x03300000;
                Call(c, kSoundCommand3);
            }
            Call(c, kShowScene);
            break;
        case 8:
            if (Ram8(kFadeDone)) {
                FadeEndStep(c);
            } else {
                Call(c, kShowScene);
            }
            break;
        case 0x0A:
            if (!Ram8(kFading)) {
                c.d[0] = 0;
                NextScreen();
            }
            break;
        }
        Calls(c, {0x016A3A, 0x016B78, 0x01495A});
    }

    // 00AF50. The ladder after a match: the opponent just beaten is marked, and the next stage's place runs
    // through the characters and stops at the next opponent (before the last stage, but on the easy level:
    // the whole ladder runs, and the last opponent comes on). A button cuts it short.
    void LeaveLadder(M68K &c, uint16_t step) {
        SetRam16(kRoundStep, step);
        SetRam16(kStepTimer, 0x10);
        SetRam16(kSubStep, 0);
        SetRam8(kScreenPhase, 0xFF);
        SetRam8(kHeld1, 1);
        SetRam8(kHeld2, 0xFF);
        (void)c;
    }
    void SoundOfLast(M68K &c) {
        Call(c, Ram8(kLastOpponentIn) ? kSoundChoose : kSoundLast);
    }
    void LadderAfterMatch(M68K &c) {
        Spend(c, kStepTime);
        switch (Ram16(kRoundStep)) {
        case 0:
            if (StartFade(c, 1)) {
                NextStep();
                SetRam16(kScreenSetting, kScreenPlain);
                SetRam8(kFading, 0xFF);
                SetRam16(kFadeSetting, 0);
                c.d[0] = 0;
                SetRam8(kScreenPhase, 0);
            }
            break;
        case 2:
            NextStep();
            SetRam16(kStepTimer, 2);
            break;
        case 4:
            if (Below(kStepTimer)) {
                NextStep();
                SetRam16(kStepTimer, 0);
            }
            break;
        case 6: { // the screen is made
            NextStep();
            constexpr uint16_t kPlaces[][2] = {{0x22, 0}, {0x24, 0}, {0x26, 0x200}, {0x28, 0x300}, {0x2A, 0x40}, {0x2C, 0x710},
                                               {0x32, 0x270E}, {0x3A, 0x2641}};
            for (const auto &place : kPlaces) {
                SetRam16(kVars + place[0], place[1]);
            }
            Call(c, kNextRoundPicture);
            SetRam16(kVars + 0x128, 2);
            Call(c, 0x020D86);
            Call(c, kBringBackAfterMatch);
            c.a[1] = 0x902000;
            c.d[0] = 4;
            Call(c, kDrawPicture);
            c.a[1] = 0x903000;
            c.d[0] = 5;
            Call(c, kDrawPicture);
            Call(c, kShowLadderOnOtherPage);
            // (the stage's number; none at the last stage of the longer ladders)
            const bool last = Ram16(kStage) == Ram16(kLastStage);
            c.a[1] = kStageNumberAt;
            c.d[0] = (c.d[0] & 0xFFFF0000) | Ram16(kStage);
            if (last) {
                c.d[0] = 9;
            }
            if (!last || Ram8(kShortLadder) || !Ram8(kLevelCursor)) {
                SetLow16(c.d[0], c.d[0] + 1);
                Call(c, kShowStageNumber);
            }
            Call(c, 0x02525C);
            break;
        }
        case 8:
            if (Ram8(kFading)) {
                break;
            }
            NextStep();
            c.d[0] = 0;
            SetRam16(kSubStep, 0);
            if (!Ram8(kVars + 0x152)) {
                SetRam8(kHeld1, 0);
                SetRam8(kHeld2, 0);
            }
            SetRam16(kStepTimer, 0x12C);
            SetRam16(kScreenSetting, kScreenPlain);
            if (Ram8(kLevelCursor) && Ram16(kStage) == kLastStageNumber) {
                SetRam16(kRoundStep, 0x12);
            }
            break;
        case 0x0A: // the next opponent
            if (Ram8(kInterrupted)) {
                SetRam16(kSubStep, 6);
                break;
            }
            switch (Ram16(kSubStep)) {
            case 0:
                NextSubStep();
                SetRam16(kSubTimer, 0x78);
                break;
            case 2:
                if (Pressed(c)) {
                    Call(c, kSoundChoose);
                    LeaveLadder(c, 0x0C);
                    Call(c, kShowStageOpponentOnOtherPage);
                } else if (Out(kSubTimer)) {
                    NextSubStep();
                    SetRam16(kSubTimer, 0x78);
                    Call(c, kSoundChoose);
                    Call(c, kShowStageOpponentOnOtherPage);
                    Call(c, kSoundDone);
                } else {
                    Call(c, kSpinStage);
                }
                break;
            case 4:
                if (Pressed(c) || Out(kSubTimer)) {
                    LeaveLadder(c, 0x0C);
                    Call(c, kShowStageOpponentOnOtherPage);
                }
                break;
            }
            break;
        case 0x0C:
            if (Below(kStepTimer) && StartFade(c, 0)) {
                NextStep();
                SetRam8(kFading, 0xFF);
                SetRam16(kFadeSetting, 0);
            }
            break;
        case 0x0E: FadeEndStep(c); break;
        case 0x10:
            if (!Ram8(kFading)) {
                c.d[0] = 0;
                LeaveMatch();
            }
            break;
        case 0x12: // the last opponent
            if (Ram8(kInterrupted)) {
                SetRam16(kSubStep, 0x0A);
                break;
            }
            switch (Ram16(kSubStep)) {
            case 0:
                NextSubStep();
                SetRam16(kSubTimer, 0x2D);
                break;
            case 2:
                if (Out(kSubTimer)) {
                    NextSubStep();
                    SetRam16(kScreenSetting, kScreenPortraits);
                    SetRam16(kSubTimer, 8);
                }
                break;
            case 4:
                if (Out(kSubTimer)) {
                    NextSubStep();
                    SetRam16(kSubTimer, 0x78);
                    StageNumber(c, 0x0A);
                    SetRam16(kScreenSetting, kScreenPlain);
                }
                break;
            case 6:
                if (Pressed(c)) {
                    SoundOfLast(c);
                    LeaveLadder(c, 0x0C);
                    Call(c, kShowAfterLadder);
                } else if (Out(kSubTimer)) {
                    NextSubStep();
                    SetRam16(kSubTimer, 0x78);
                    SoundOfLast(c);
                    Call(c, kShowAfterLadder);
                } else {
                    Call(c, kSpinLadder);
                }
                break;
            case 8: // (a button does nothing here)
                if (!Pressed(c) && Out(kSubTimer)) {
                    LeaveLadder(c, 0x0C);
                    Call(c, kShowAfterLadder);
                }
                break;
            }
            break;
        }
        Calls(c, {kShowCharacters, 0x016A3A, 0x016B78, 0x016BCA, 0x01495A});
    }
    // --- after the game ---

    constexpr uint32_t kMatchTimer = kVars + 0x0A;   // word: frames the step still has
    constexpr uint32_t kContinuing = kVars + 0x8E;   // byte: the players who may go on (bit 0 player 1, bit 1 player 2)
    constexpr uint32_t kContinuer = kVars + 0x116;   // word: the low half of the address of the record of the one shown
    constexpr uint32_t kCountLeft = 0x8C;            // byte, in a player's record: the count shown to that player

    // 00B760. Whose count the screen shows: the one player who may go on, or of two the one with more left.
    uint32_t WhoContinues() {
        if (Ram8(kContinuing) == 3) {
            return static_cast<int8_t>(Ram8(Player(0) + kCountLeft)) >= static_cast<int8_t>(Ram8(Player(1) + kCountLeft)) ? Player(0) : Player(1);
        }
        return Ram8(kContinuing) & 1 ? Player(0) : Player(1);
    }
    void NoteWhoContinues(M68K &c) {
        c.a[0] = WhoContinues();
        SetRam16(kContinuer, static_cast<uint16_t>(c.a[0]));
    }
    // 00B636. The loser may go on with another coin: the count down is a task's; this sets the screen up
    // and waits for the end, which is the title again (the game ended meanwhile), or game over.
    void ContinueScreen(M68K &c) {
        Spend(c, kStepTime);
        switch (Ram16(kMatchStep)) {
        case 0:
            Add16(kMatchStep, 2);
            c.d[0] = 0;
            SetRam8(kScreenPhase, 0);
            SetRam8(kVars + 0x10F, 0);
            SetRam8(kVars + 0x10D, 3);
            SetRam8(kHeld1, 1);
            SetRam8(kHeld2, 0xFF);
            NoteWhoContinues(c);
            Calls(c, {kClearText, 0x020DAE});
            if (!Ram8(kVars + 0x11F)) {
                break;
            }
            if (Ram8(kVars + 0xCE) && !Ram8(kVars + 0x141) && !Ram8(kVars + 0xD6)) {
                c.d[0] = static_cast<uint8_t>(Ram8(kVars + 0x144) - 1);
                if (c.d[0] != Ram8(kVars + 0x14F)) {
                    SetRam8(kVars + 0x144, static_cast<uint8_t>(c.d[0]));
                }
            }
            Calls(c, {0x02B900, 0x02BCE8, 0x020E18});
            c.a[0] = 0x0AB880;
            c.a[1] = 0x90C160;
            c.d[7] = 0;
            Call(c, kCopyColours);
            {
                // The loser's character, and the words
                uint32_t task = NewTask(c, 0x0302);
                if (!task) {
                    break;
                }
                SetRam8(task + 0x0A, 9);
                SetRam16(task + 0x10, 0xC0);
                SetRam16(task + 0x14, 0xD0);
                task = NewTask(c, 0x0303);
                if (!task) {
                    break;
                }
                SetRam8(task + 0x0A, 0x13);
                SetRam16(task + 0x10, 0x100);
                SetRam16(task + 0x14, 0x80);
                Call(c, kNewTask); // (asked for and not used)
                c.d[6] = 0;
                Call(c, kNewTask);
                if (c.flagZ) {
                    break;
                }
                task = c.a[4] & 0xFFFFFF;
                const bool first = Ram8(kVars + 0xB9) & 1;
                SetRam8(task, static_cast<uint8_t>(Ram8(task) + 1));
                SetRam8(task + 2, 0x0C);
                SetRam16(task + 0x0C, first ? 0 : 0xFF04);
                SetRam8(task + 3, Ram8(Player(first ? 0 : 1) + 0xF6));
            }
            break;
        case 2:
            Add16(kMatchStep, 2);
            c.d[0] = 0;
            SetRam8(kHeld1, 0);
            SetRam8(kHeld2, 0);
            break;
        case 4:
            NoteWhoContinues(c);
            // (when the game has ended meanwhile, back to the title: that waits for the frame's interrupt, and
            // the game's own code runs it, see RegisterScreens)
            if (!Ram8(kContinuing)) {
                c.d[0] = 0;
                SetRam32(kGameStep, 0x00080000);
                ClearSteps(kMatchStep);
                SetRam8(kGameEndsHere, 0);
                SetRam8(kHeld2, 0);
                SetRam16(kVars + 0x110, 0);
                SetRam8(kScreenPhase, 1);
                SetRam8(kVars + 0x10F, 0);
                SetRam8(kHeld1, 1);
                SetRam8(kVars + 0x106, 0);
                SetRam8(kVars + 0x104, 1);
                SetRam16(kScreenSetting, kScreenPlain);
            }
            break;
        }
        Calls(c, {0x016A3A, 0x016BCA, 0x0178A8, 0x01495A});
    }

    // 00B846. Game over: the words come on (four tasks), and after three seconds, or a button, the game
    // goes back to its title (that last step waits for the frame's interrupt: the game's own code runs it,
    // see RegisterScreens).
    void GameOver(M68K &c) {
        Spend(c, 50);
        switch (Ram16(kMatchStep)) {
        case 0:
            Add16(kMatchStep, 2);
            SetRam16(kMatchTimer, 0x1F);
            c.d[0] = 0;
            SetRam8(kScreenPhase, 0);
            SetRam8(kVars + 0x10F, 0);
            SetRam8(kVars + 0x10D, 4);
            Calls(c, {kClearText, 0x02BD28, 0x020DA6});
            c.a[0] = 0x0AB8A0;
            c.a[1] = 0x90C160;
            c.d[7] = 0;
            Call(c, kCopyColours);
            for (const uint8_t part : {0, 2, 8, 0x0A}) {
                const uint32_t task = NewTask(c, 0x0307);
                if (!task) {
                    break;
                }
                SetRam8(task + 0x0A, 0x14);
                SetRam8(task + 0x0E, part);
            }
            break;
        case 2:
            if (Below(kMatchTimer)) {
                Add16(kMatchStep, 2);
                SetRam16(kMatchTimer, 0x2F);
                Call(c, 0x02B7D2);
            }
            break;
        case 4:
            if (Below(kMatchTimer)) {
                Add16(kMatchStep, 2);
                SetRam8(kScreenPhase, 0);
                SetRam16(kMatchTimer, 0xB4);
            }
            break;
        case 6:
            if (Pressed(c) || Below(kMatchTimer)) {
                Add16(kMatchStep, 2);
            }
            break;
        }
        Calls(c, {0x016A3A, 0x01495A});
    }
    bool ContinueDoesNotWait(M68K &) {
        return !(Ram16(kMatchStep) == 4 && (Ram8(kGameEndsHere) & 0x80));
    }
    // (with someone still in, the last steps do nothing more)
    bool GameOverDoesNotWait(M68K &) {
        return Ram16(kMatchStep) < 6 || Ram8(kPlayersIn) != 0;
    }
} // namespace

void RegisterScreens() {
    // (the last number is the time a call is charged)
    constexpr uint32_t kScratch = kFreeD0 | kFreeD1 | kFreeD2 | kFreeD3 | kFreeD4 | kFreeD5 | kFreeD6 | kFreeD7 | kFreeA0 | kFreeA1 |
                                  kFreeA2 | kFreeA3 | kFreeA4 | kFreeA6 | kFreeFlags;
    AddRewrite(0x00A3A2, "MatchIntro", MatchIntro, kScratch, 1);
    AddRewrite(0x00ABCC, "AfterMatch", AfterMatch, kScratch, 1);
    AddRewrite(0x00ADD2, "StageInterlude", StageInterlude, kScratch, 1);
    AddRewrite(0x00AF50, "LadderAfterMatch", LadderAfterMatch, kScratch, 1);
    AddRewrite(0x00B636, "ContinueScreen", ContinueScreen, kScratch, 1);
    RewriteWhen(0x00B636, ContinueDoesNotWait);
    AddRewrite(0x00B846, "GameOver", GameOver, kScratch, 1);
    RewriteWhen(0x00B846, GameOverDoesNotWait);
}

} // namespace cps2recomp
