// SPDX-License-Identifier: GPL-3.0-only
// Copyright (C) 2026 Tommy McLeroy
//
// Super Puzzle Fighter II Turbo: a round of a match, from its announcement to who won it, written again as
// C++ (rewrites: lift.h).
//
// While a match is on (kMatchStep is kMatchRound) the game runs RoundFrame every frame: the round's step
// (kRoundStep), each with steps of its own (kSubStep), and then the frame's work on the two fields and the
// rest of the picture.
#include "spf2.h"

namespace cps2recomp {

namespace {
    using namespace spf2;

    // --- the game's variables used here (kVars and on) ---
    constexpr uint32_t kStepTimer = kVars + 0x12; // word: frames left of a step that waits
    constexpr uint32_t kStepWork = kVars + 0x14;  // long: cleared with the steps
    constexpr uint32_t kTurnsStopped = kVars + 0x10E; // byte: the players' turns are not run (the round has not begun, or is over)
    constexpr uint32_t kRoundHeld = kVars + 0x10F;    // byte: only the round's step is run, nothing of the frame's other work
    constexpr uint32_t kRoundOver = kVars + 0x107;    // byte
    constexpr uint32_t kWinners = kVars + kRoundWinners;
    constexpr uint32_t kWinner = kVars + 0x13A, kLoser = kVars + 0x13C; // words: the low halves of the two records' addresses
    constexpr uint32_t kWinnerNumber = kVars + 0x145; // byte
    constexpr uint32_t kRoundsToWin = kVars + 0x121;  // byte
    constexpr uint32_t kMatchWon = kVars + 0x105;     // byte
    constexpr uint32_t kRoundNumber = kVars + 0x102;  // word: rounds of this match played
    constexpr uint32_t kRoundFrames = kVars + 0xB4;   // word: counts the round's frames
    constexpr uint32_t kStage = kVars + 0x112, kLastStage = kVars + 0x114; // words: which opponent of the one-player game
    constexpr uint32_t kLastStageWon = kVars + 0x14C; // byte
    constexpr uint32_t kMatchesWon = kVars + 0x14F;   // byte: matches a person has won against the computer (99 at most)
    // What is being shown over the round. Set here and counted on by the things on the screen that show it:
    // 1 the round's number, 2 and 3 the win, 4 when the winner's words may start, 0 when all is done
    constexpr uint32_t kShowing = kVars + 0x12C;      // byte
    constexpr uint32_t kWinDelay = kVars + 0x125;     // byte: frames before the win is shown
    constexpr uint32_t kBigFinish = kVars + 0x126;    // byte: the round was ended by an attack too big for the field
    constexpr uint32_t kInterlude = kVars + 0x13E;    // word: not zero while something comes between two rounds
    constexpr uint32_t kInterrupted = kVars + 0x8A;   // byte
    constexpr uint32_t kMusicStarted = kVars + 0x156; // byte
    constexpr uint32_t kScreenSetting = kVars + 0x20, kScreenSetting2 = kVars + 0x48; // words: for the video hardware
    constexpr uint16_t kScreenRound = 0x90C0, kScreenBetween = 0x92A0;
    // The play time shown in the operator's records: frames of a second (counted down), seconds, minutes
    constexpr uint32_t kPlayFrames = kVars + 0x10A, kPlaySeconds = kVars + 0x109, kPlayMinutes = kVars + 0x108; // bytes
    constexpr uint32_t kTempo = kVars - 0x7336;       // word: goes up every frame of play (the computer player's pace)
    // The two characters on the screen: things of their own, after the players' records
    constexpr uint32_t kCharacters[2] = {kVars + 0xB00, kVars + 0xC00};
    constexpr uint32_t kThingOn = 1;       // byte, in a character and in a player's record: is run and shown
    constexpr uint32_t kThingDoing = 4;    // word: what a character is doing
    constexpr uint16_t kComingIn = 0x0208;

    // --- in a player's record ---
    constexpr uint32_t kRoundsWon = 0xA3, kRoundMarks = 0xA4; // byte; a byte a round (kMarkWon)
    constexpr uint8_t kMarkWon = 4;
    constexpr uint32_t kWinsShown = 0xA2;    // byte: matches won in the mode with handicaps, in decimal (two digits)
    constexpr uint32_t kQuickWin = 0xB3;     // byte: has won a round in under kQuickSeconds
    constexpr uint16_t kQuickSeconds = 60;
    constexpr uint32_t kLongestChain = 0x32E, kBiggestBroken = 0x32F; // bytes: this round's
    constexpr uint32_t kBestChain = 0xAF, kBestBroken = 0xB1;         // bytes: this match's
    // The winner's bonuses when a round is over (longs), and their sum
    constexpr uint32_t kBonusGiven = 0x348; // byte
    constexpr uint32_t kTimeBonus = 0x330, kChainBonusWon = 0x334, kBigBonusWon = 0x338, kStageBonus = 0x33C, kBonusTotal = 0x344;
    constexpr uint16_t kBonusSeconds = 180;
    constexpr uint32_t kBonusTime = 3000; // (what the game's code takes to work them out: it multiplies by adding)
    constexpr uint32_t kPointsASecond = 100, kPointsAChain = 500, kPointsABigCell = 100, kPointsAStage = 1000, kPointsStageBase = 2000;

    // --- the game's routines used here that are not written again ---
    constexpr uint32_t kBothPlayersTurns = 0x00BF48; // (turn.cpp)
    constexpr uint32_t kNewTask = 0x02DA46, kStartRoundMusic = 0x02BBF0, kSetUpRoundPicture = 0x016C98,
                       kSoundCommand = 0x02B22A, kSoundCommand2 = 0x02B202, kSoundCommand3 = 0x02B216, kMatchWonSound = 0x02B334,
                       kSkillAfterMatch = 0x0157F0, kWinnerSpeaks = 0x02B780, kComputerWinnerSpeaks = 0x02B79A, kClearText = 0x02D8C8,
                       kButtonPressed = 0x00B9D8, kMatchOver = 0x00AAF6, kNextRoundPicture = 0x02D7CC;
    // What a frame of a round runs after the players' turns (the fields' pictures, the characters, the rest of
    // the screen; not looked into), and the last of them, which is given two numbers
    constexpr uint32_t kFrameWork[] = {0x0140B8, 0x013C60, 0x016A3A, 0x016B78, 0x016BCA, 0x02B29A, 0x0178A8};
    constexpr uint32_t kFrameWorkAfter[] = {0x0151F0, 0x01495A};
    constexpr uint32_t kFrameLast = 0x0013A0;
    constexpr uint32_t kFrameCounter = kVars + 0x124; // byte: counted down each frame
    // What starts the next round
    constexpr uint32_t kNextRoundWork[] = {0x01788C, 0x016C0C, 0x016D16, 0x005AD2, 0x01674A};

    uint32_t Record(uint32_t lowHalf) {
        return 0xFF0000 | Ram16(lowHalf);
    }
    void NextSubStep() {
        SetRam16(kSubStep, static_cast<uint16_t>(Ram16(kSubStep) + 2));
    }
    void NextRoundStep() {
        SetRam16(kRoundStep, static_cast<uint16_t>(Ram16(kRoundStep) + 2));
    }

    // --- the round begins (kRoundStep 0) ---

    // 00A6DE. Everything of the round before is cleared, and the two players and their characters are put on
    void BeginSetUp(M68K &c) {
        NextSubStep();
        SetRam8(kTurnsStopped, 1);
        SetRam8(kRoundHeld, 1);
        SetRam32(kStepWork, 0);
        for (const uint32_t cleared : {0x10Cu, 0x107u, 0x12Cu, 0x125u, 0x126u}) {
            SetRam8(kVars + cleared, 0);
        }
        SetRam16(kWinner, 0);
        SetRam16(kLoser, 0);
        for (const uint32_t cleared : {0xBDu, 0x145u, 0x10Du, 0x12Au, 0x124u, 0x149u, 0x127u, 0x11Cu, 0x11Du, 0x11Eu, 0x151u}) {
            SetRam8(kVars + cleared, 0);
        }
        // (a byte and a count of 180 among the sounds' variables; not looked into)
        SetRam8(kVars - 0x6CB0, 0);
        SetRam16(kVars - 0x6CB2, 180);
        SetRam16(kVars + 0x32, Ram16(kVars + 0x32) | 0x000E);
        SetRam16(kScreenSetting, kScreenRound);
        SetRam16(kScreenSetting2, kScreenRound);
        SetRam8(Player(0) + kThingOn, 1);
        SetRam8(Player(1) + kThingOn, 1);
        SetRam8(kCharacters[0] + kThingOn, 1);
        SetRam8(kCharacters[1] + kThingOn, 1);
        SetRam8(kVars + 0x133, 1);
        SetRam8(kVars + 0x10D, 1);
        c.d[0] = 0x100;
        Call(c, kSoundCommand);
        Call(c, kSetUpRoundPicture);
    }

    // 00A78C. Whatever comes between two rounds runs to its end (the game's routine at 00BDDA, every frame
    // while kInterlude is set); then the frame's work starts, and before a match's first round a thing is
    // made that shows how many rounds win it
    void BeginShow(M68K &c) {
        // (not while kInterlude is set: the game's own code runs then, see RegisterRound)
        NextSubStep();
        c.d[0] = 0;
        SetRam8(kRoundHeld, 0);
        SetRam8(kVars + 0x8F, 0);
        SetRam8(kVars + 0xBD, 0);
        if (Ram16(kRoundNumber) != 0) {
            return;
        }
        Call(c, kNewTask);
        if (c.flagZ) {
            return; // (no room for one)
        }
        const uint32_t task = c.a[4] & 0xFFFFFF;
        SetRam8(task, static_cast<uint8_t>(Ram8(task) + 1));
        SetRam16(task + 2, 0x0308);
        SetLow8(c.d[0], Ram8(kRoundsToWin) + 0x2F);
        SetRam8(task + 0xA, static_cast<uint8_t>(c.d[0]));
    }

    // 00A7CE. Waits for both characters to have come in
    void BeginWaitForCharacters(M68K &) {
        if (Ram16(kCharacters[0] + kThingDoing) == kComingIn || Ram16(kCharacters[1] + kThingDoing) == kComingIn) {
            return;
        }
        NextSubStep();
        SetRam8(kShowing, 1);
    }

    // 00A7EA. When the round's number has been shown, play begins
    void BeginPlay(M68K &c) {
        if (Ram8(kShowing)) {
            return;
        }
        NextRoundStep();
        c.d[0] = 0;
        SetRam32(kSubStep, 0);
        SetRam8(kTurnsStopped, 0);
        if (!Ram8(kMusicStarted)) {
            SetRam8(kMusicStarted, 0xFF);
            Call(c, kStartRoundMusic);
        }
    }

    // --- the round is played (kRoundStep 2) ---

    // 00B284. The play time
    void CountPlayTime(M68K &) {
        SetRam8(kPlayFrames, static_cast<uint8_t>(Ram8(kPlayFrames) - 1));
        if (!(Ram8(kPlayFrames) & 0x80)) {
            return;
        }
        SetRam8(kPlayFrames, 59);
        SetRam8(kPlaySeconds, static_cast<uint8_t>(Ram8(kPlaySeconds) + 1));
        if (Ram8(kPlaySeconds) == 60) {
            SetRam8(kPlaySeconds, 0);
            SetRam8(kPlayMinutes, static_cast<uint8_t>(Ram8(kPlayMinutes) + 1));
        }
    }

    // 00A810. A frame of play, until a player has won (kRoundWinners: StepLost sets it). The winner is given
    // the round, and the match if that was the last round needed.
    void RoundPlay(M68K &c) {
        if (Ram8(kInterrupted)) {
            SetRam8(kVars + 0xBD, 0xFF);
            SetRam8(kTurnsStopped, 0xFF);
            return;
        }
        CountPlayTime(c);
        SetRam16(kTempo, static_cast<uint16_t>(Ram16(kTempo) + 1));
        const uint8_t winners = Ram8(kWinners);
        SetLow8(c.d[0], winners);
        if (winners == 0) {
            return;
        }
        const uint32_t winner = Player(winners & 1 ? 0 : 1), loser = Player(winners & 1 ? 1 : 0);
        c.a[0] = winner;
        c.a[1] = loser;
        SetRam16(kWinner, static_cast<uint16_t>(winner));
        SetRam16(kLoser, static_cast<uint16_t>(loser));
        c.a[2] = winner + kRoundMarks;
        SetRam8(winner + kRoundMarks + Ram8(winner + kRoundsWon), kMarkWon);
        SetRam8(winner + kRoundsWon, static_cast<uint8_t>(Ram8(winner + kRoundsWon) + 1));
        c.d[1] = (c.d[1] & 0xFFFFFF00) | Ram8(kRoundsToWin);
        if (Ram8(winner + kRoundsWon) == Ram8(kRoundsToWin)) {
            SetRam8(kMatchWon, 0xFF);
            Call(c, kMatchWonSound);
            if (Ram8(Var(c, kMode)) == kModeHandicap) {
                // One more win, in decimal, as far as 99
                uint8_t wins = Ram8(winner + kWinsShown);
                wins = wins == 0x99 ? 0x99 : (wins & 0x0F) == 9 ? static_cast<uint8_t>(wins + 7) : static_cast<uint8_t>(wins + 1);
                SetRam8(winner + kWinsShown, wins);
            } else if (!Ram8(winner + kComputer)) {
                if (Ram8(kMatchesWon) < 99) {
                    SetRam8(kMatchesWon, static_cast<uint8_t>(Ram8(kMatchesWon) + 1));
                }
                if (Ram16(kStage) == Ram16(kLastStage)) {
                    SetRam8(kLastStageWon, 0xFF);
                    SetRam8(kVars + 0xBD, 0xFF);
                }
            }
        }
        NextRoundStep();
        c.d[0] = 0;
        SetRam32(kSubStep, 0);
        SetRam8(kVars + 0x8F, 1);
        SetRam8(kRoundOver, 1);
        Call(c, kSkillAfterMatch);
    }

    // --- the round has been won (kRoundStep 4) ---

    // 00A928. After a wait the win is shown; an attack too big for the field ends the round with a sound
    void WonShow(M68K &c) {
        if (Ram8(kWinDelay)) {
            SetRam8(kWinDelay, static_cast<uint8_t>(Ram8(kWinDelay) - 1));
            return;
        }
        NextSubStep();
        SetRam8(kShowing, 2);
        c.a[0] = Record(kWinner) | 0xFF000000;
        SetRam8(kWinnerNumber, Ram8(Record(kWinner) + kPlayerNumber));
        if (Ram8(kBigFinish)) {
            c.d[0] = 0x020000FF;
            Call(c, kSoundCommand2);
        }
    }

    // 00A8DA. A person's longest chain and biggest break of the round are kept if they are the match's best
    void KeepBests(M68K &c) {
        for (int n = 0; n < 2; n++) {
            const uint32_t player = Player(n);
            c.a[4] = player;
            if (Ram8(player + kComputer)) {
                continue;
            }
            SetLow8(c.d[0], Ram8(player + kLongestChain));
            if (Ram8(player + kLongestChain) >= Ram8(player + kBestChain)) {
                SetRam8(player + kBestChain, Ram8(player + kLongestChain));
            }
            SetLow8(c.d[0], Ram8(player + kBiggestBroken));
            if (Ram8(player + kBiggestBroken) >= Ram8(player + kBestBroken)) {
                SetRam8(player + kBestBroken, Ram8(player + kBiggestBroken));
            }
        }
    }

    // 00A95C. When both fields have come to rest
    void WonSettle(M68K &c) {
        if (Ram8(Var(c, kFieldsBusy))) {
            return;
        }
        NextSubStep();
        SetRam8(kShowing, 3);
        KeepBests(c);
    }

    // 00A972. The winner's words
    void WonSpeak(M68K &c) {
        if (Ram8(kShowing) != 4) {
            return;
        }
        NextSubStep();
        const uint32_t winner = Record(kWinner);
        c.a[0] = winner | 0xFF000000;
        Call(c, Ram8(winner + kComputer) ? kComputerWinnerSpeaks : kWinnerSpeaks);
    }

    // 00A996. When they are over the round's last step begins: from its start for a person, past the first
    // parts of it when the computer has won
    void WonEnd(M68K &c) {
        if (Ram8(kShowing)) {
            return;
        }
        c.d[0] = 0x02800000;
        Call(c, kSoundCommand3);
        const uint32_t winner = Record(kWinner);
        c.a[6] = winner | 0xFF000000;
        c.d[0] = 0;
        NextRoundStep();
        SetRam16(kVars + 0x0E, 0);
        SetRam16(kSubStep, Ram8(winner + kComputer) ? 6 : 0);
        SetRam16(kStepTimer, 0);
        SetRam32(kStepWork, 0);
    }

    // 00A908. (the turns stop once both fields are at rest)
    constexpr uint32_t kWonSteps[] = {0x00A928, 0x00A95C, 0x00A972, 0x00A996};
    constexpr uint32_t kWonStepTable = 0x00A920;
    void RoundWon(M68K &c) {
        if (!Ram8(Var(c, kFieldsBusy))) {
            SetRam8(kTurnsStopped, 1);
        }
        const uint16_t step = Ram16(kSubStep);
        SetLow16(c.d[0], step);
        SetLow16(c.d[1], kWonSteps[step / 2] - kWonStepTable);
        Call(c, kWonSteps[step / 2]);
    }

    // --- after the round (kRoundStep 6) ---

    // Whether any of some players (bit 0 player 1, bit 1 player 2) has just pressed a button or Start. The
    // answer is left as the game's routines leave it: in D0 and in the flags.
    constexpr uint16_t kButtons = 0x7700;
    void Pressed(M68K &c, uint8_t who) {
        c.d[0] = c.d[1] = 0;
        for (int n = 0; n < 2; n++) {
            if (!(who & (1 << n))) {
                continue;
            }
            const uint16_t buttons = ~Ram16(kControlsBefore[n]) & Ram16(kControls[n]) & kButtons;
            const uint16_t start = ~Ram8(kStartsAndCoins + 1) & Ram8(kStartsAndCoins) & (1 << n);
            c.d[0] |= buttons | start;
            c.d[1] = start;
        }
        c.flagZ = (c.d[0] & 0xFFFF) == 0;
        c.flagN = (c.d[0] & 0x8000) != 0;
        c.flagV = c.flagC = false;
    }
    // 00B992. The players of the mode being played
    void ModesPlayersPressed(M68K &c) {
        Pressed(c, Ram8(Var(c, kMode)));
    }
    // 00B9B2. ... or before a mode is chosen, the players who are in
    void PlayersInPressed(M68K &c) {
        Pressed(c, Ram8(Var(c, kMode)) ? Ram8(Var(c, kMode)) : Ram8(kPlayersIn));
    }
    // 00B9D8. ... or with handicaps (where both play on), the round's winner
    void ButtonPressedRoutine(M68K &c) {
        Pressed(c, Ram8(Var(c, kMode)) == kModeHandicap ? Ram8(kWinners) : Ram8(Var(c, kMode)));
    }
    bool ButtonPressed(M68K &c) {
        Call(c, kButtonPressed);
        return (c.d[0] & 0xFFFF) != 0;
    }

    // 00A9F4. The players are taken off; a win in under a minute is noted
    void AfterClear(M68K &c) {
        NextSubStep();
        SetRam16(kStepTimer, 15);
        SetRam8(kVars + 0x10D, 0);
        c.d[0] = 0;
        SetRam8(Player(0) + kThingOn, 0);
        SetRam8(Player(1) + kThingOn, 0);
        SetRam8(kVars + 0x133, 0);
        SetRam8(kVars + 0x151, 1);
        const uint32_t winner = Record(kWinner);
        c.a[0] = winner | 0xFF000000;
        if (Ram16(winner + kRoundSeconds) < kQuickSeconds) {
            SetRam8(winner + kQuickWin, 0xFF);
        }
        Call(c, kClearText);
    }

    // 00AA2E. A wait of 16 frames; then the bonuses are shown for four seconds
    void AfterWait(M68K &) {
        SetRam16(kStepTimer, static_cast<uint16_t>(Ram16(kStepTimer) - 1));
        if (Ram16(kStepTimer) & 0x8000) {
            NextSubStep();
            SetRam16(kStepTimer, 240);
        }
    }

    // 00AA40. ... or until a button is pressed
    void AfterBonuses(M68K &c) {
        if (!ButtonPressed(c)) {
            SetRam16(kStepTimer, static_cast<uint16_t>(Ram16(kStepTimer) - 1));
            if (!(Ram16(kStepTimer) & 0x8000)) {
                return;
            }
        }
        NextSubStep();
        SetRam16(kStepTimer, 15);
        SetRam8(kVars + 0x151, 2);
    }

    // 00AA60. When what was shown has gone (kVars + 0x151 back to nothing), and after 16 frames more or a
    // button: the next round, or the end of the match
    void AfterEnd(M68K &c) {
        if (Ram8(kVars + 0x151)) {
            return;
        }
        if (!ButtonPressed(c)) {
            SetRam16(kStepTimer, static_cast<uint16_t>(Ram16(kStepTimer) - 1));
            if (!(Ram16(kStepTimer) & 0x8000)) {
                return;
            }
        }
        c.d[0] = 0;
        SetRam32(kRoundStep, 0);
        SetRam32(kSubStep, 0);
        SetRam32(kStepWork, 0);
        SetRam8(kTurnsStopped, 0);
        SetRam8(kVars + 0x10B, 0);
        SetRam8(kVars + 0xB8, 0);
        SetRam8(kVars + 0x8F, 1);
        SetRam8(kVars + 0x12A, 1);
        SetRam8(Player(0) + kThingOn, 0);
        SetRam8(Player(1) + kThingOn, 0);
        SetRam8(kCharacters[0] + kThingOn, 0);
        SetRam8(kCharacters[1] + kThingOn, 0);
        SetRam8(kVars + 0x133, 0);
        if (Ram8(kMatchWon)) {
            Call(c, kMatchOver);
            return;
        }
        Call(c, kNextRoundPicture);
        SetRam16(kRoundNumber, static_cast<uint16_t>(Ram16(kRoundNumber) + 1));
        SetRam16(kScreenSetting, kScreenBetween);
        SetRam16(kScreenSetting2, kScreenBetween);
        SetRam8(kRoundHeld, 1);
        for (const uint32_t work : kNextRoundWork) {
            Call(c, work);
        }
        Call(c, kStartRoundMusic);
    }

    // --- the players' turns, and the winner's bonuses ---

    // 00C7DE. Once the round is over and the fields are at rest, each player's bonuses are worked out, and the
    // winner is given them: for every second under 180, for the longest chain, for the most cells of big
    // gems broken at once, and for the stage.
    void RoundBonus(M68K &c) {
        const uint32_t player = c.a[6];
        if (Ram8(Var(c, kFieldsBusy)) || !Ram8(kRoundOver) || Ram8(player + kBonusGiven)) {
            return;
        }
        SetRam8(player + kBonusGiven, 1);
        Spend(c, kBonusTime);
        const uint16_t seconds = Ram16(player + kRoundSeconds);
        const uint32_t time = static_cast<int16_t>(kBonusSeconds - seconds) > 0 ? (kBonusSeconds - seconds) * kPointsASecond : 0;
        const uint32_t chain = Ram8(player + kLongestChain) * kPointsAChain;
        const uint32_t big = Ram8(player + kBiggestBroken) * kPointsABigCell;
        const uint32_t stage = Ram16(kStage) * kPointsAStage + kPointsStageBase;
        SetRam32(player + kTimeBonus, time);
        SetRam32(player + kChainBonusWon, chain);
        SetRam32(player + kBigBonusWon, big);
        SetRam32(player + kStageBonus, stage);
        const uint32_t total = time + chain + big + stage;
        SetRam32(player + kBonusTotal, Ram32(player + kBonusTotal) + total);
        if (Ram8(kWinners) & (1 << (Ram8(player + kPlayerNumber) & 7))) {
            SetRam32(player + kScore, Ram32(player + kScore) + total);
        }
    }

    // 00BEFA. The players' turns while the round is played; and when it is not, the bonuses.
    void PlayTurns(M68K &c) {
        if (!Ram8(kTurnsStopped)) {
            Call(c, kBothPlayersTurns);
            return;
        }
        for (int n = 0; n < 2; n++) {
            c.a[6] = Player(n);
            RoundBonus(c);
        }
        // (the game goes on to clear what it has of each player's pair: the mark that the first has been made,
        // its row and its two gems. But it gives each place as a distance from A6, the second player's record,
        // where it means the place itself: what it clears is memory some way below the records, and the
        // pairs stay as they were)
        const uint32_t from = Player(1);
        for (int n = 0; n < 2; n++) {
            const auto misplaced = [&](uint32_t field) {
                return (from + static_cast<uint32_t>(static_cast<int16_t>((Player(n) + field) & 0xFFFF))) & 0xFFFFFF;
            };
            SetRam8(misplaced(kPairStarted), 0);
            SetRam16(misplaced(kPairRow), 0);
            SetRam16(misplaced(kPairNow), 0);
            SetRam16(misplaced(kPairNow + 2), 0);
        }
    }

    // --- the opponents that come unasked (called before each match) ---

    constexpr uint32_t kOpponentsMet = kVars + 0x110;   // word: a bit for each opponent beaten; bit 8 the challenger
    constexpr uint32_t kChallengerIn = kVars + 0x139;   // byte: the challenger's coming in has been shown
    constexpr uint32_t kContinued = kVars + 0xAD;       // byte: a continue has been used
    constexpr uint32_t kOpponentShown = kVars + 0x100;  // word: the opponent's character, doubled
    constexpr uint32_t kBestChainOfGame = 0xAE, kBestBrokenOfGame = 0xB0, kBigFinishes = 0xB2; // bytes, in a player's record
    constexpr uint8_t kChallenger = 8, kLastOpponent = 9, kLastOpponentBefore = 10;
    constexpr uint16_t kChallengerStage = 6;
    constexpr uint8_t kChallengerChain = 4, kChallengerBroken = 20;
    enum : uint16_t { kInterludeLastOpponent = 2, kInterludeChallenger = 4 };

    // The person and the computer of a one-player game (as the game's code leaves them in A0 and A1)
    void PersonAndComputer(M68K &c) {
        const bool first = Ram8(Var(c, kMode)) & 1;
        c.a[0] = Player(first ? 0 : 1);
        c.a[1] = Player(first ? 1 : 0);
    }

    // 00BD38. The hidden challenger. At the seventh stage, a person who has used no continue and has in
    // this game won a round in under a minute, made a chain of four, broken twenty cells of big gems at once
    // and ended a round with an attack too big for the field, is met by the challenger: first what comes
    // between the rounds brings it in (kInterludeChallenger), and then, called again, this puts it in the
    // opponent's place.
    void MeetChallenger(M68K &c) {
        if (Ram8(kVars + 0xBF) || Ram8(Var(c, kMode)) == kModeHandicap) {
            return;
        }
        PersonAndComputer(c);
        const uint32_t person = c.a[0], computer = c.a[1];
        SetLow16(c.d[0], Ram16(kOpponentsMet));
        if (Ram16(kOpponentsMet) & 0x0100) {
            return;
        }
        if (Ram8(kChallengerIn)) {
            SetRam16(kInterlude, 0);
            SetRam8(computer + kCharacter, kChallenger);
            SetRam16(kOpponentShown, 2 * kChallenger);
            return;
        }
        if (Ram8(kContinued) || Ram16(kStage) != kChallengerStage || !Ram8(person + kQuickWin) ||
            Ram8(person + kBestChainOfGame) < kChallengerChain || Ram8(person + kBestBrokenOfGame) < kChallengerBroken ||
            !Ram8(person + kBigFinishes)) {
            return;
        }
        SetRam16(kInterlude, kInterludeChallenger);
    }

    // 00BDAC. The last opponent is given as character 10, which what comes between the rounds
    // (kInterludeLastOpponent) turns into character 9 as it shows it coming in.
    void MeetLastOpponent(M68K &c) {
        if (Ram8(Var(c, kMode)) == kModeHandicap) {
            return;
        }
        PersonAndComputer(c);
        if (Ram8(c.a[1] + kCharacter) == kLastOpponentBefore) {
            SetRam16(kInterlude, kInterludeLastOpponent);
            SetRam8(kMusicStarted, 0xFF);
        }
    }

    // --- a frame of a round ---

    constexpr uint32_t kRoundSteps[] = {
        0x00A6CA, // the round begins (RoundBegin)
        0x00A810, // it is played (RoundPlay)
        0x00A908, // it has been won (RoundWon)
        0x00A9E0, // after it (RoundAfter)
    };
    constexpr uint32_t kRoundStepTable = 0x00A658;
    constexpr uint32_t kBeginSteps[] = {0x00A6DE, 0x00A78C, 0x00A7CE, 0x00A7EA}, kBeginStepTable = 0x00A6D6;
    constexpr uint32_t kAfterSteps[] = {0x00A9F4, 0x00AA2E, 0x00AA40, 0x00AA60}, kAfterStepTable = 0x00A9EC;

    // 00A6CA, 00A9E0
    void RoundBegin(M68K &c) {
        const uint16_t step = Ram16(kSubStep);
        SetLow16(c.d[0], step);
        SetLow16(c.d[1], kBeginSteps[step / 2] - kBeginStepTable);
        Call(c, kBeginSteps[step / 2]);
    }
    void RoundAfter(M68K &c) {
        const uint16_t step = Ram16(kSubStep);
        SetLow16(c.d[0], step);
        SetLow16(c.d[1], kAfterSteps[step / 2] - kAfterStepTable);
        Call(c, kAfterSteps[step / 2]);
    }

    // 00A660. A frame of a round: its step, and then (unless the round is held) the players' turns and
    // everything that makes the picture.
    constexpr uint32_t kPlayTurns = 0x00BEFA;
    void RoundFrame(M68K &c) {
        const uint16_t step = Ram16(kRoundStep);
        SetLow16(c.d[0], step);
        SetLow16(c.d[1], kRoundSteps[step / 2] - kRoundStepTable);
        Call(c, kRoundSteps[step / 2]);
        if (Ram8(kRoundHeld)) {
            return;
        }
        SetRam16(kRoundFrames, static_cast<uint16_t>(Ram16(kRoundFrames) + 1));
        Call(c, kPlayTurns);
        for (const uint32_t work : kFrameWork) {
            Call(c, work);
        }
        if (Ram8(kFrameCounter)) {
            SetRam8(kFrameCounter, static_cast<uint8_t>(Ram8(kFrameCounter) - 1));
        }
        for (const uint32_t work : kFrameWorkAfter) {
            Call(c, work);
        }
        c.d[0] = 0x45;
        c.d[1] = 0x1B;
        Call(c, kFrameLast);
    }
} // namespace

namespace {
    // What comes between two rounds (kInterlude: the hidden challenger coming in) waits for the frame's
    // interrupt part of the way through, which a rewrite cannot do: while it is on, the three rewrites it is
    // reached through stand aside for the game's own code
    bool NoInterlude(M68K &) {
        return Ram16(kInterlude) == 0;
    }
    bool NotBeginningWithInterlude(M68K &) {
        return !(Ram16(kRoundStep) == 0 && Ram16(kSubStep) == 2 && Ram16(kInterlude) != 0);
    }
    bool NotRoundBeginWithInterlude(M68K &) {
        return !(Ram16(kSubStep) == 2 && Ram16(kInterlude) != 0);
    }
} // namespace

void RegisterRound() {
    // (the registers these leave are the ones the game's own code leaves wherever something after might read
    // them; the last number is the time a call is charged over and above the routines it calls)
    constexpr uint32_t kScratch = kFreeD0 | kFreeD1 | kFreeD2 | kFreeD3 | kFreeD4 | kFreeD5 | kFreeD6 | kFreeD7 | kFreeA0 | kFreeA1 |
                                  kFreeA2 | kFreeA3 | kFreeA4 | kFreeA6 | kFreeFlags;
    AddRewrite(0x00A660, "RoundFrame", RoundFrame, kScratch, 310);
    AddRewrite(0x00A6CA, "RoundBegin", RoundBegin, kScratch, 34);
    AddRewrite(0x00A6DE, "BeginSetUp", BeginSetUp, kScratch, 490);
    AddRewrite(0x00A78C, "BeginShow", BeginShow, kScratch, 194);
    RewriteWhen(0x00A660, NotBeginningWithInterlude);
    RewriteWhen(0x00A6CA, NotRoundBeginWithInterlude);
    RewriteWhen(0x00A78C, NoInterlude);
    AddRewrite(0x00A7CE, "BeginWaitForCharacters", BeginWaitForCharacters, kFreeFlags, 30);
    AddRewrite(0x00A7EA, "BeginPlay", BeginPlay, kScratch, 23);
    AddRewrite(0x00A810, "RoundPlay", RoundPlay, kScratch, 160);
    AddRewrite(0x00B284, "CountPlayTime", CountPlayTime, kFreeFlags, 28);
    AddRewrite(0x00A908, "RoundWon", RoundWon, kScratch, 50);
    AddRewrite(0x00A928, "WonShow", WonShow, kScratch, 112);
    AddRewrite(0x00A95C, "WonSettle", WonSettle, kScratch, 246);
    AddRewrite(0x00A8DA, "KeepBests", KeepBests, kFreeFlags, 180);
    AddRewrite(0x00A972, "WonSpeak", WonSpeak, kScratch, 27);
    AddRewrite(0x00A996, "WonEnd", WonEnd, kScratch, 25);
    AddRewrite(0x00A9E0, "RoundAfter", RoundAfter, kScratch, 34);
    AddRewrite(0x00A9F4, "AfterClear", AfterClear, kScratch, 142);
    AddRewrite(0x00AA2E, "AfterWait", AfterWait, kFreeFlags, 29);
    AddRewrite(0x00AA40, "AfterBonuses", AfterBonuses, kScratch, 158);
    AddRewrite(0x00AA60, "AfterEnd", AfterEnd, kScratch, 112);
    AddRewrite(0x00B992, "ModesPlayersPressed", ModesPlayersPressed, 0, 176);
    AddRewrite(0x00B9B2, "PlayersInPressed", PlayersInPressed, 0, 197);
    AddRewrite(0x00B9D8, "ButtonPressed", ButtonPressedRoutine, 0, 202);
    AddRewrite(0x00BD38, "MeetChallenger", MeetChallenger, kFreeFlags, 180);
    AddRewrite(0x00BDAC, "MeetLastOpponent", MeetLastOpponent, kFreeFlags, 95);
    AddRewrite(0x00C7DE, "RoundBonus", RoundBonus, kFreeD0 | kFreeD1 | kFreeD2 | kFreeFlags, 46);
    AddRewrite(0x00BEFA, "PlayTurns", PlayTurns, kScratch, 40);
}

} // namespace cps2recomp
