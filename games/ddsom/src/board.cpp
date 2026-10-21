// SPDX-License-Identifier: GPL-3.0-only
// Copyright (C) 2026  Part of cps2recomp.
//
// Dungeons & Dragons: Shadow over Mystara: the board's settings, written again as C++.
//
// The board (the arcade board the game is in) has a set of switches and a set of settings. The game keeps
// them in a record in its work RAM, and the routine at 02D358 turns that record into the settings the
// game plays with. This is that routine, written with names (the game's own code for it no longer runs;
// `--check-rewrites` runs both and compares what they leave behind).
//
// What the record holds, and where the routine puts each part:
//
//   record+0   the kind of record: the board's own switches (0), or one of the two the test menu makes
//              (0x11 and 0x12). The kind says what the rest of the record means.
//   record+1   the index of the setting the board is set to. The game's table at 2D3FC turns the index
//              into the chute type (how many players can play, and how many coin chutes they have) and
//              the coin mode (how many credits a coin gives); a third table at 2D426 gives the continue
//              setting. These are what the game shows in its own operator menu (Configuration, System).
//   record+2   the setting itself, as a byte.
//   record+3   ... and five more bytes, and two more further on, and three longs: the game's other
//              settings, which it keeps where its code reads them.
//
// The two kinds the test menu makes differ from the board's own: they set a word of switches rather than
// taking the board's switch table, so the routine puts 0x0101 in place of what the board's own kind
// would have.
#include "ddsom.h"

#include "lift.h"

namespace cps2recomp {

namespace {
    using namespace ddsom;

    // The game's tables, as the program holds them (the ROM as the code reads it, not the work RAM).
    //
    // The table at 2D40E: the index of the setting gives the chute type and the coin mode, a word apart.
    // The chute type is what says how many players can play (3 is four players) and how many coin chutes
    // they have; the coin mode is how many credits a coin gives.
    struct BoardTable {
        uint8_t chuteType;
        uint8_t coinMode;
    };
    constexpr BoardTable kBoardTable[] = {
        {0, 0}, {1, 0}, {1, 1}, {0, 0}, {1, 0}, {2, 0}, {2, 1}, {0, 0},
        {1, 0}, {1, 1}, {3, 0}, {3, 1}, {0, 0}, {0, 0}, {0, 0}, {0, 0},
    };
    // The table at 2E426: the index of the setting gives the continue setting.
    constexpr uint8_t kContinueTable[] = {0, 0, 0, 1, 1, 1, 1, 2, 2, 2, 2, 2, 0, 0, 0, 0};
    // The table at 2D38C: the board's own switches, a word per row (the record's first byte names the row).
    constexpr uint16_t kSwitchTable[] = {
        0x0101, 0x0102, 0x0103, 0x0104, 0x0105, 0x0106, 0x0107, 0x0108,
        0x0109, 0x0201, 0x0301, 0x0401, 0x0501, 0x0601, 0x0701, 0x0801,
        0x0901, 0x0101,
    };

    // The ROM as the program reads it where it names a table by its distance from the instruction.
    uint16_t Code16(M68K &c, uint32_t address) {
        return static_cast<uint16_t>(c.ReadProgram(address, 1));
    }

    // 02D358. The board's settings, applied: the record becomes the settings the game plays with.
    void ApplyBoardSettings(M68K &c) {
        // (the three settings that the kind of the record decides, cleared first)
        SetRam8(kSettingByte1, 0);
        SetRam8(kSettingByte2, 0);
        SetRam8(kSettingByte3, 0);

        const uint8_t kind = Ram8(kRecordKind);
        SetRam8(kSettingKind, kind);
        if (kind == kRecordMenuOne || kind == kRecordMenuTwo) {
            // (the test menu's kinds: the switches are all on, and the board's switch table is not read)
            SetRam16(kSettingWords, 0x0101);
            SetRam8(kSettingByte3, 0x01);
            SetRam8(kSettingByte1, 0x01);
            SetRam8(kSettingByte2, 0x01);
        } else {
            // (the board's own kind: the switches come from the table the board's switch setting names)
            const uint32_t row = static_cast<uint32_t>(kind) * 2;
            const uint16_t switches = row + 1 < sizeof(kSwitchTable) / sizeof(kSwitchTable[0])
                                          ? kSwitchTable[row]
                                          : Code16(c, 0x02D38C + row);
            SetRam16(kSettingWords, switches);
        }
        // (the record's own bytes, as they are)
        SetRam8(kSettingByte1, Ram8(kRecordSetting));
        SetRam8(kSettingByte4, Ram8(kRecordBytes1));
        SetRam8(kSettingByte5, Ram8(kRecordBytes2));
        SetRam8(kSettingByte6, Ram8(kRecordBytes3));

        // (the index of the setting the board is set to, and the tables it names)
        const uint8_t index = Ram8(kSettingsIndex);
        SetRam8(kSettingIndex, index);
        if (index < sizeof(kBoardTable) / sizeof(kBoardTable[0])) {
            SetRam8(kChuteType, kBoardTable[index].chuteType);
            SetRam8(kSettingCoinMode, kBoardTable[index].coinMode);
        }
        if (index < sizeof(kContinueTable)) {
            SetRam8(kSettingContinue, kContinueTable[index]);
        }
        // (the rest of the record: the game's other settings)
        SetRam8(kSettingByte7, Ram8(kRecordByte4));   // FF8029
        SetRam8(kSettingByte8, 0);                   // FF8072
        SetRam8(kSettingByte9, Ram8(kRecordByte5));   // FF8073
        SetRam8(kSettingByte10, 0);                  // FF8084
        SetRam8(kSettingByte11, Ram8(kRecordByte6));  // FF8085
        SetRam8(kSettingByte12, Ram8(kRecordByte8));  // FF802A
        SetRam8(kSettingByte13, Ram8(kRecordByte9));  // FF802B
        SetRam32(kSettingLong1, Ram32(kRecordLong1));
        SetRam32(kSettingLong2, Ram32(kRecordLong2));
        SetRam32(kSettingLong3, Ram32(kRecordLong3));
        SetRam32(kSettingLong4, Ram32(kRecordLong4));

        // (the registers the routine leaves: D1 holds the index of the setting, D0 the index doubled (the
        // tables are read with it), and A4 the record)
        c.d[1] = index;
        c.d[0] = index * 2;
        c.a[4] = kSettings;
    }

    // 02DACA. A player joins: the game counts them, and (unless it is in its test menu, or the count has
    // not changed) it puts the count down and waits a while.
    void PlayerJoins(M68K &c) {
        const uint32_t joined = Ram32(kPlayersJoined) + 1;
        SetRam32(kPlayersJoined, joined);
        if (Ram8(kInTestMenu) != 0) {
            return;
        }
        if (joined == Ram32(kPlayersJoinedBefore)) {
            return;
        }
        SetRam32(kPlayersJoinedBefore, joined);
        SetRam16(kWaitValue, 1);
        SetRam16(kWaitDelay, 0x60);
        // (the game's own routine that waits)
        Call(c, 0x02D8AA);
    }
    // The port's own choice of how many can play, put into the record before the routine that applies
    // the settings reads it. The index the board is set to by default is 2 (two players, two chutes,
    // multi); 0x0A is four players (the only other index the game's table gives a chute type of 3 for).
    //
    // This is a hook and not part of the rewrite, because the rewrite has to do exactly what the game's
    // own routine does: the check compares the two, and the choice is the port's, not the game's.
    //
    // The hook is at the two blocks that reach the routine that applies the settings with the record
    // ready: 02D344 (the board's start, after the routine that fills the record) and 02D33E (the other
    // way there, the record already filled). A hook cannot be at the routine itself, as the rewrite is
    // there, and the record is written after a hook at the routine's own start.
    bool ChoosePlayers(M68K &) {
        const int players = FeatureValue("board_players");
        if (players > 0) {
            SetRam8(kSettingsIndex, players == 2 ? kSettingFourPlayers : kSettingTwoPlayers);
        }
        return false;
    }
} // namespace

void RegisterBoard() {
    // (the routine that applies the settings, written again as C++. The original takes 622 cycles.)
    AddRewrite(kApplySettings, "ApplyBoardSettings", ApplyBoardSettings, kFreeFlags, 622);
    // (the routine that counts a player joining, written again as C++)
    AddRewrite(kPlayerJoins, "PlayerJoins", PlayerJoins, kFreeFlags, 128);
    // (the port's own choice of how many can play, put into the record before the routine above reads it)
    AddHook(kBoardStart, "ChoosePlayers", ChoosePlayers);
    AddHook(kBoardStartToo, "ChoosePlayersAgain", ChoosePlayers);
    AddHook(kTestMenuLeft, "ChoosePlayersOnceMore", ChoosePlayers);
}

} // namespace cps2recomp
