// SPDX-License-Identifier: GPL-3.0-only
// Copyright (C) 2026 Tommy McLeroy
//
// Super Puzzle Fighter II Turbo: a match being set up, who the computer's next character is, and what
// happens when a match is over, written again as C++ (rewrites: lift.h).
//
// Against the computer a game is a ladder of stages (kStage), each with an opponent taken from one of eight
// orders of the eight characters (kOpponentOrders); the last stage's opponent comes after those. A person
// who joins in and wins takes the ladder over where it stood, without meeting again the characters already
// beaten (kOpponentsMet).
#include "spf2.h"

namespace cps2recomp {

namespace {
    using namespace spf2;

    // --- the game's variables used here (kVars and on) ---
    constexpr uint32_t kStepWords = kVars + 0x0C;        // three longs: the round's steps and what they keep
    constexpr uint32_t kStage = kVars + 0x112, kLastStage = kVars + 0x114; // words
    constexpr uint32_t kOpponentsMet = kVars + 0x110;    // word: a bit for each character beaten; bit 8 the challenger
    constexpr uint32_t kOpponentShown = kVars + 0x100;   // word: the opponent's character, doubled
    constexpr uint16_t kLastStageShown = 0x12;           // (what the last stage shows, whoever the opponent is)
    constexpr uint16_t kLastStageNumber = 8;
    constexpr uint32_t kOrderChosen = kVars + 0x122;     // byte: which of the orders this game has, times kOrderSize
    constexpr uint32_t kOpponentOrders = 0x00BC78, kOrderSize = 16;
    constexpr uint32_t kOrder = kVars - 0x735A;          // the game's order, copied here: a character a stage
    constexpr uint32_t kChallengerIn = kVars + 0x139;    // byte: the hidden challenger has come in
    constexpr uint32_t kLastOpponentIn = kVars + 0x138;  // byte: the last opponent has come in
    constexpr int kChallengerStage = 6, kLastOpponentStage = 8;
    constexpr uint8_t kChallenger = 8, kLastOpponent = 9;
    constexpr uint32_t kLadderOwner = kVars + 0x132;     // byte: the number of the player whose ladder it is
    constexpr uint32_t kRoundsToWin = kVars + 0x121;     // byte
    constexpr uint32_t kRoundsSetting = kVars + 0xA2, kRoundsSettingHandicap = kVars + 0xA3; // bytes: the operator's
    constexpr uint32_t kWinners = kVars + kRoundWinners;
    constexpr uint32_t kScreenSetting = kVars + 0x20;    // word: for the video hardware
    constexpr uint16_t kScreenRound = 0x90C0;
    constexpr uint32_t kCharacters[2] = {kVars + 0xB00, kVars + 0xC00}; // the two characters on the screen
    constexpr uint32_t kThingOn = 1;                     // byte, in a character and in a player's record

    // --- in a player's record ---
    constexpr uint32_t kRoundsWon = 0xA3, kRoundMarks = 0xA4; // byte; four bytes
    constexpr uint32_t kScoreKept = 0x90;                // long: the score, carried from match to match
    constexpr uint32_t kBestChain = 0xAF, kBestBroken = 0xB1;             // bytes: this match's
    constexpr uint32_t kBestChainOfGame = 0xAE, kBestBrokenOfGame = 0xB0; // bytes: this game's
    // What a loser's record keeps of the match it lost (for the list of best players)
    constexpr uint32_t kLostAtStage = 0xF8, kLostWith = 0xF7, kLostAfter = 0xF9, kLostChain = 0xFA, kLostBroken = 0xFB; // bytes

    // --- the game's routines used here that are not written again ---
    constexpr uint32_t kNextRoundPicture = 0x02D7CC, kMeetLastOpponent = 0x00BDAC, kMeetChallenger = 0x00BD38; // (the two: round.cpp)
    constexpr uint32_t kMatchSetUpWork[] = {0x02CF2E, 0x005AD2, 0x015676};

    // 00BAB2. The game's order of opponents is copied out, with the challenger and the last opponent put in
    // their places once they have come in. A0: the player whose ladder it is.
    void SetOpponentOrder(M68K &c) {
        c.d[0] = Ram8(kOrderChosen);
        SetRam8(kLadderOwner, Mem8(c, c.a[0] + kPlayerNumber)); // (not always a record: one caller leaves something else in A0)
        c.a[2] = kOpponentOrders;
        for (uint32_t n = 0; n < kOrderSize; n++) {
            SetRam8(kOrder + n, Mem8(c, kOpponentOrders + Ram8(kOrderChosen) + n));
        }
        if (Ram8(kChallengerIn)) {
            SetLow16(c.d[0], Ram16(kOpponentsMet));
            if (!(Ram16(kOpponentsMet) & (1 << kChallenger))) {
                SetRam8(kOrder + kChallengerStage, kChallenger);
            }
        }
        if (Ram8(kLastOpponentIn)) {
            SetRam8(kOrder + kLastOpponentStage, kLastOpponent);
        }
    }

    // The first stage whose character has not been beaten (`met`), or the last stage: the stage is noted, and
    // the registers left as the game's loop leaves them
    uint8_t StageNotMet(M68K &c, uint16_t met) {
        uint16_t stage = 0;
        while (stage < Ram16(kLastStage) && (met & (1u << (Ram8(kOrder + stage) & 31)))) {
            stage++;
        }
        SetRam16(kStage, stage);
        c.d[0] = 0xFFFF0000 | stage;
        c.a[2] = kOrder;
        return Ram8(kOrder + stage);
    }

    // 00BAFE. (the last stage shows the same thing whoever the opponent is)
    void ShowOpponent(M68K &c, uint16_t shown) {
        SetRam16(kOpponentShown, shown);
        if (Ram8(Var(c, kMode)) != kModeHandicap && Ram16(kStage) == kLastStageNumber) {
            SetRam16(kOpponentShown, kLastStageShown);
        }
    }

    // 00BA44. The opponent for the stage the game is at. Against the computer: the first character of the
    // order not yet beaten. (With handicaps both are people: only who is shown as the opponent is set.)
    constexpr uint32_t kHandicapSides = kVars + 0x8D, kHandicapSwapped = kVars + 0x131; // bytes
    void SetOpponent(M68K &c) {
        c.a[0] = Player(0);
        c.a[1] = Player(1);
        const auto swap = [&]() {
            const uint32_t kept = c.a[0];
            c.a[0] = c.a[1];
            c.a[1] = kept;
        };
        if (Ram8(Var(c, kMode)) == kModeHandicap) {
            if (Ram8(kHandicapSides) == 3 ? !Ram8(kHandicapSwapped) : (Ram8(kHandicapSides) & 1)) {
                swap();
            }
        } else {
            if (!(Ram8(Var(c, kMode)) & 1)) {
                swap();
            }
            SetOpponentOrder(c);
            SetLow16(c.d[2], Ram16(kOpponentsMet));
            SetRam8(c.a[1] + kCharacter, StageNotMet(c, Ram16(kOpponentsMet)));
        }
        c.d[1] = Ram8(c.a[1] + kCharacter) * 2u;
        ShowOpponent(c, static_cast<uint16_t>(c.d[1]));
    }

    // 00BB16. A match is over (A0 the winner, A1 the loser). When a person has won, the character beaten is
    // marked, and the next opponent chosen.
    void NextOpponent(M68K &c) {
        const uint32_t winner = c.a[0], loser = c.a[1];
        if (Ram8(winner + kComputer)) {
            return;
        }
        uint16_t met = Ram16(kOpponentsMet);
        if (Ram8(loser + kComputer)) {
            SetLow8(c.d[0], Ram8(loser + kCharacter));
            met |= static_cast<uint16_t>(1u << (Ram8(loser + kCharacter) & 31));
            SetRam16(kOpponentsMet, met);
        }
        SetLow16(c.d[2], met);
        // (a ladder that changes hands starts some counts again; they are not looked into)
        if (Ram8(kVars + 0xCE) && Ram8(kVars + 0x141) && Ram8(winner + kPlayerNumber) != Ram8(kLadderOwner)) {
            SetRam8(kVars + 0x144, Ram8(kVars + 0xD4));
            SetRam8(kVars + 0x14F, 0);
            SetRam8(kVars + 0xD6, 0);
        }
        SetOpponentOrder(c);
        const uint8_t character = StageNotMet(c, met);
        SetRam8(loser + kCharacter, character);
        SetLow16(c.d[1], static_cast<uint16_t>(static_cast<int8_t>(character) * 2));
        ShowOpponent(c, static_cast<uint16_t>(c.d[1]));
    }

    // 00A378. A player's record made ready for a match (A0). A person playing the computer keeps the score.
    void ClearForMatch(M68K &c, uint32_t player) {
        c.a[0] = player;
        SetRam8(player + kRoundsWon, 0);
        SetRam32(player + kRoundMarks, 0);
        SetRam32(player + kScore, 0);
        SetRam8(player + kBestChain, 0);
        SetRam8(player + kBestBroken, 0);
        if (Ram8(Var(c, kMode)) != kModeHandicap && !Ram8(player + kComputer)) {
            SetRam32(player + kScore, Ram32(player + kScoreKept));
        }
    }
    void ClearForMatchRoutine(M68K &c) {
        ClearForMatch(c, c.a[0]);
    }

    // 00A262. A match begins: everything of the match before is cleared, the operator's settings are taken,
    // and the opponents that come unasked are looked for.
    void MatchSetUp(M68K &c) {
        SetRam16(kMatchStep, static_cast<uint16_t>(Ram16(kMatchStep) + 2));
        Call(c, kNextRoundPicture);
        SetRam16(kScreenSetting, kScreenRound);
        SetRam16(kVars + 0x32, Ram16(kVars + 0x32) | 0x000E);
        c.d[0] = 0;
        for (uint32_t n = 0; n < 12; n += 4) {
            SetRam32(kStepWords + n, 0);
        }
        SetRam16(kVars + 0x102, 0);
        for (const uint32_t cleared : {0x12Cu, 0x105u, 0x107u, 0x10Du, 0x14Du, 0xB8u, 0x8Au, 0x10Cu, 0x10Bu, 0x10Fu, 0x118u, 0x12Au, 0x125u,
                                       0x126u, 0x124u, 0x149u, 0x127u, 0x11Cu, 0x11Du, 0x11Eu, 0x151u, 0x108u, 0x109u, 0x152u, 0x156u, 0x133u}) {
            SetRam8(kVars + cleared, 0);
        }
        SetRam8(kVars + 0x10A, 59); // (the play time's frames: round.cpp)
        SetRam8(Player(0) + kThingOn, 0);
        SetRam8(Player(1) + kThingOn, 0);
        SetRam8(kCharacters[0] + kThingOn, 0);
        SetRam8(kCharacters[1] + kThingOn, 0);
        SetRam8(kVars + 0xBD, 1);
        SetRam8(kVars + 0x8F, 1);
        SetRam8(kRoundsToWin, Ram8(kRoundsSetting));
        SetRam8(kVars + 0xB9, Ram8(Var(c, kMode)));
        SetRam8(kVars + 0x11F, Ram8(kVars + 0x98));
        if (Ram8(kVars + 0xA5) == 2 && Ram8(kVars + 0xE0)) {
            SetRam8(kVars + 0x11F, 0);
        }
        ClearForMatch(c, Player(0));
        ClearForMatch(c, Player(1));
        if (Ram8(Var(c, kMode)) == kModeHandicap) {
            SetRam8(kVars + 0x141, 0xFF);
            SetRam8(kRoundsToWin, Ram8(kRoundsSettingHandicap));
        }
        Call(c, kMeetLastOpponent);
        Call(c, kMeetChallenger);
        for (const uint32_t work : kMatchSetUpWork) {
            Call(c, work);
        }
    }

    // 00B2FC. When a match is over, a person's best chain and biggest break of it are kept if they are the
    // game's best (the hidden challenger asks for them: round.cpp)
    void KeepGameBests(M68K &c) {
        for (int n = 0; n < 2; n++) {
            const uint32_t player = Player(n);
            c.a[4] = player;
            if (Ram8(player + kComputer)) {
                continue;
            }
            SetLow8(c.d[0], Ram8(player + kBestChain));
            if (Ram8(player + kBestChain) >= Ram8(player + kBestChainOfGame)) {
                SetRam8(player + kBestChainOfGame, Ram8(player + kBestChain));
            }
            SetLow8(c.d[0], Ram8(player + kBestBroken));
            if (Ram8(player + kBestBroken) >= Ram8(player + kBestBrokenOfGame)) {
                SetRam8(player + kBestBrokenOfGame, Ram8(player + kBestBroken));
            }
        }
    }

    // 00B32A. ... and a person's score is kept for the next match; a person who lost has noted where the
    // game ended: the stage, the character played, and the game's bests.
    void KeepScores(M68K &c) {
        for (int n = 0; n < 2; n++) {
            const uint32_t player = Player(n);
            c.a[2] = player;
            if (Ram8(player + kComputer)) {
                continue;
            }
            if (Ram8(Var(c, kMode)) != kModeHandicap) {
                SetRam32(player + kScoreKept, Ram32(player + kScore));
            }
            SetLow8(c.d[0], Ram8(player + kPlayerNumber));
            if (Ram8(kWinners) & (1 << (Ram8(player + kPlayerNumber) & 7))) {
                continue;
            }
            SetRam8(player + kLostAtStage, Ram8(kOpponentShown + 1));
            SetRam8(player + kLostWith, Ram8(player + 0x9F));
            SetRam8(player + kLostAfter, Ram8(kVars - 0x7374)); // (the course: EASY, NORMAL or HARD)
            SetRam8(player + kLostChain, Ram8(player + kBestChainOfGame));
            SetRam8(player + kLostBroken, Ram8(player + kBestBrokenOfGame));
        }
    }

    // --- a match is over ---

    constexpr uint32_t kWinner = kVars + 0x13A, kLoser = kVars + 0x13C; // words: the low halves of the two records' addresses
    constexpr uint32_t kWinQuote = kVars + 0x136;        // word: which of the winner's words are shown
    constexpr uint32_t kQuoteTurn = kVars + 0x153;       // byte: counts the matches, for the choice of words
    constexpr uint32_t kQuoteChoices = 0x00BCF8;         // 64 bytes: a number 0 to 7, or kQuoteForOpponent
    constexpr uint32_t kQuotesFirst = 0x00BC44;          // words: a character's first quote (19 each)
    constexpr uint8_t kQuoteForOpponent = 8;             // the words kept for the character that was beaten
    constexpr uint32_t kShortQuotes = kVars + 0x92;      // word: not zero where the words are one of four, by chance
    constexpr uint32_t kRandom = 0x02DB02;

    // 00BC02. The winner's words: each character has nineteen, eight for anyone and one for each opponent,
    // taken in a fixed round of 64 choices. (In this set kShortQuotes is always set, as far as the checks' runs
    // go: the long way has not been checked against the game's code.)
    void ChooseWinQuote(M68K &c) {
        if (Ram16(kShortQuotes)) {
            Call(c, kRandom);
            const uint32_t winner = 0xFFFF0000 | Ram16(kWinner);
            c.a[0] = winner;
            c.d[1] = Ram8((winner & 0xFFFFFF) + kCharacter) * 4u;
            SetLow16(c.d[0], (c.d[0] & 3) + c.d[1]);
            SetRam16(kWinQuote, static_cast<uint16_t>(c.d[0]));
            return;
        }
        const uint8_t turn = (Ram8(kQuoteTurn) + 1) & 0x3F;
        SetRam8(kQuoteTurn, turn);
        uint8_t choice = Mem8(c, kQuoteChoices + turn);
        if (choice == kQuoteForOpponent) {
            choice = static_cast<uint8_t>(choice + Ram8((0xFF0000 | Ram16(kLoser)) + kCharacter));
        }
        const uint32_t winner = 0xFFFF0000 | Ram16(kWinner);
        c.a[0] = winner;
        c.d[1] = Ram8((winner & 0xFFFFFF) + kCharacter) * 2u;
        SetLow16(c.d[0], choice + Code16(c, kQuotesFirst + c.d[1]));
        SetRam16(kWinQuote, static_cast<uint16_t>(c.d[0]));
    }

    // 00B576. The last stage has been won (A0: the winner): the game goes on to its ending.
    constexpr uint32_t kNoteLost = 0x00B352; // (the part of KeepScores that notes where a game ended, A2 the record)
    constexpr uint16_t kGameEnding = 0x0A;
    void LadderWon(M68K &c) {
        const uint32_t winner = c.a[0] & 0xFFFFFF;
        SetRam8(kVars + 0x130, Ram8(winner + kCharacter));
        SetRam8(kVars + 0x123, Ram8(winner + 0x9F));
        c.a[2] = c.a[0];
        Call(c, kNoteLost);
        c.d[0] = 0;
        SetRam16(kGameStep, kGameEnding);
        SetRam8(kVars + 0x106, 1);
        SetRam32(kMatchStep, 0);
        for (uint32_t n = 0; n < 12; n += 4) {
            SetRam32(kStepWords + n, 0);
        }
        for (const uint32_t cleared : {0xACu, 0x107u, 0x105u, 0x12Au, 0x133u, 0xB8u, 0xBDu, 0x14Au}) {
            SetRam8(kVars + cleared, 0);
        }
        SetRam8(Player(0) + kThingOn, 0);
        SetRam8(Player(1) + kThingOn, 0);
        SetRam8(kCharacters[0] + kThingOn, 0);
        SetRam8(kCharacters[1] + kThingOn, 0);
        SetRam8(kVars + 0x8F, 1);
        SetRam8(Player(0) + 0x80, 0);
        SetRam8(Player(1) + 0x80, 0);
        SetRam8(kVars + 0xD0, static_cast<uint8_t>(Ram8(kVars + 0xD0) + 1));
    }

    // 00AAF6. A match is over. What is kept is kept, the winner's words are chosen, and then: a person who
    // beat the computer goes on to the next opponent (or to the ending); a person who beat a person plays
    // on against the computer in the loser's place; and when the computer has won, or the operator's
    // setting ends the game there, the loser's game is over.
    constexpr uint32_t kMatchOverWork = 0x02D8C8, kGiveLadder = 0x01F2C2, kLadderWonRoutine = 0x00B576;
    constexpr uint32_t kLastStageWon = kVars + 0x14C;    // byte
    constexpr uint32_t kRoundHeld = kVars + 0x10F;       // byte (round.cpp)
    constexpr uint32_t kGameEndsHere = kVars + 0xB8;     // byte
    constexpr uint32_t kPlayerState = 0x80;              // byte, in a player's record
    void MatchOver(M68K &c) {
        SetRam16(kMatchStep, static_cast<uint16_t>(Ram16(kMatchStep) + 2));
        SetRam8(kRoundHeld, 1);
        SetRam8(Player(0) + kPlayerState, 0);
        SetRam8(Player(1) + kPlayerState, 0);
        KeepGameBests(c);
        KeepScores(c);
        ChooseWinQuote(c);
        Call(c, kMatchOverWork);
        // The loser's character stays on the screen
        const uint32_t character = kCharacters[Ram8(kWinners) & 1 ? 1 : 0];
        c.a[0] = character;
        SetRam8(character + kThingOn, 1);
        SetRam8(kVars + 0x154, Ram8(character + kCharacter));
        SetRam8(kVars + 0x155, Ram8(character + 0x9F));
        // (the operator's setting kVars + 0xA5: 3, or 2 with kVars + 0xE0 set, ends the game with the match)
        bool over = Ram8(kVars + 0xA5) == 3 || (Ram8(kVars + 0xA5) == 2 && Ram8(kVars + 0xE0));
        if (!over) {
            const uint32_t winner = 0xFFFF0000 | Ram16(kWinner), loser = 0xFFFF0000 | Ram16(kLoser);
            c.a[0] = winner;
            c.a[1] = loser;
            NextOpponent(c);
            if (Ram8(kLastStageWon)) {
                Call(c, kLadderWonRoutine);
                return;
            }
            SetLow8(c.d[0], Ram8((loser & 0xFFFFFF) + kPlayerNumber));
            SetRam8(Var(c, kMode), Ram8(Var(c, kMode)) & ~(1 << (Ram8((loser & 0xFFFFFF) + kPlayerNumber) & 7)));
            over = Ram8((winner & 0xFFFFFF) + kComputer) != 0;
            if (!over) {
                Call(c, kGiveLadder);
                SetRam8((c.a[0] & 0xFFFFFF) + kPlayerState, 2);
                SetRam8((c.a[1] & 0xFFFFFF) + kComputer, 1);
                if (!Ram8(kVars + 0xA4) && Ram8(kVars + 0xE0)) {
                    SetRam8(kVars + 0xE0, 0);
                    SetRam8((c.a[1] & 0xFFFFFF) + 0xB4, 1);
                }
                return;
            }
        }
        // (A1: the loser, or when the setting ended the game, whatever the routine before left there, as in
        // the game's own code)
        c.Write(c.a[1] + kComputer, 0, 1);
        SetRam8(kGameEndsHere, 1);
        SetRam8(kRoundHeld, 1);
        c.d[1] = 0;
        SetRam8(Var(c, kMode), 0);
        SetRam8(kVars + 0xE0, 0);
        SetRam8(Player(0) + kPlayerState, 0);
        SetRam8(Player(1) + kPlayerState, 0);
    }
} // namespace

void RegisterMatch() {
    // (the last number is the time a call is charged)
    constexpr uint32_t kScratch = kFreeD0 | kFreeD1 | kFreeD2 | kFreeD3 | kFreeD4 | kFreeD5 | kFreeD6 | kFreeD7 | kFreeA0 | kFreeA1 |
                                  kFreeA2 | kFreeA3 | kFreeA4 | kFreeA6 | kFreeFlags;
    AddRewrite(0x00BAB2, "SetOpponentOrder", SetOpponentOrder, kFreeFlags, 210);
    AddRewrite(0x00BA44, "SetOpponent", SetOpponent, kFreeFlags, 620);
    AddRewrite(0x00BB16, "NextOpponent", NextOpponent, kFreeFlags, 800);
    AddRewrite(0x00A378, "ClearForMatch", ClearForMatchRoutine, kFreeFlags, 127);
    AddRewrite(0x00A262, "MatchSetUp", MatchSetUp, kScratch, 1060);
    AddRewrite(0x00BC02, "ChooseWinQuote", ChooseWinQuote, kFreeFlags, 104);
    AddRewrite(0x00B576, "LadderWon", LadderWon, kScratch, 382);
    AddRewrite(0x00AAF6, "MatchOver", MatchOver, kScratch, 1800);
    AddRewrite(0x00B2FC, "KeepGameBests", KeepGameBests, kFreeFlags, 170);
    AddRewrite(0x00B32A, "KeepScores", KeepScores, kFreeFlags, 180);
}

} // namespace cps2recomp
