// SPDX-License-Identifier: GPL-3.0-only
// Copyright (C) 2026 Tommy McLeroy
//
// Super Puzzle Fighter II Turbo: the words that come up over a player's field ("3 CHAIN!", "TECH BONUS",
// "ALL CLEAR!", "1ST ATTACK"), written again as C++ (rewrites: lift.h). A6 is the player's record.
//
// They are put into the text layer of the picture, which is in video memory: twelve characters across,
// each two tiles high. A banner stays kBannerFrames frames (PlayerTurn counts kBannerTimer down, and draws
// or clears it each frame); it comes in a tile lower than it rests and leaves a tile higher.
#include "spf2.h"

namespace cps2recomp {

namespace {
    using namespace spf2;

    constexpr uint32_t kBannerTimer = 0x381; // byte: frames the banner still has
    constexpr uint32_t kBannerPlace = 0x382; // long: where in the text layer the player's banner goes
    constexpr uint32_t kBannerKind = 0x38D;  // byte
    enum : uint8_t { kBannerChain = 0, kBannerTechBonus = 1, kBannerAllClear = 2, kBannerFirstAttack = 3 };
    constexpr uint32_t kBannerNumber = 0x38E; // byte: the chain's length, for kBannerChain
    constexpr uint8_t kBannerFrames = 60;
    // The text layer: a tile is two words (its number, its colours); the next tile down is the next in
    // memory, and the next across is kAcross further
    constexpr uint32_t kDown = 4, kAcross = 0x80;
    constexpr uint16_t kBlankTile = 0x20, kBannerColours = 5;
    constexpr int kBannerLength = 12;
    constexpr int kChainShownMost = 9;
    constexpr uint32_t kClearTime = 2900, kCharacterTime = 120; // (the time the game's code takes, about)

    void PutTile(M68K &c, uint32_t at, uint16_t tile, uint16_t colours) {
        c.Write(at, 1, tile);
        c.Write(at + 2, 1, colours);
    }
    // The top tile of a character in the game's tall letters; the bottom one is kLowerHalf after it
    constexpr uint16_t kLowerHalf = 0x10;
    uint16_t TallTile(char character) {
        if (character >= '0' && character <= '9') {
            return static_cast<uint16_t>(0xA0 + (character - '0'));
        }
        if (character >= 'A' && character <= 'O') {
            return static_cast<uint16_t>(0xC1 + (character - 'A'));
        }
        if (character >= 'P' && character <= 'Z') {
            return static_cast<uint16_t>(0xE0 + (character - 'P'));
        }
        return 0x81; // '!'
    }
    // Text from the character after `place` on; a space is left as it is
    void PutText(M68K &c, uint32_t place, const char *text) {
        for (; *text; text++) {
            place += kAcross;
            if (*text != ' ') {
                PutTile(c, place, TallTile(*text), kBannerColours);
                PutTile(c, place + kDown, TallTile(*text) + kLowerHalf, kBannerColours);
                Spend(c, kCharacterTime);
            }
        }
    }
    void ClearLine(M68K &c, uint32_t place) {
        for (int n = 0; n < kBannerLength; n++, place += kAcross) {
            PutTile(c, place, kBlankTile, 0);
            PutTile(c, place + kDown, kBlankTile, 0);
        }
    }
    // The banner's three places, one tile apart, cleared
    void ClearAt(M68K &c, uint32_t place) {
        ClearLine(c, place - kDown);
        ClearLine(c, place - 2 * kDown);
        ClearLine(c, place);
    }

    // 00C948. Takes the player's banner off the screen.
    void ClearBanner(M68K &c) {
        ClearAt(c, Ram32(c.a[6] + kBannerPlace));
    }
    // 00C9BE. ... and the opponent's
    void ClearOpponentBanner(M68K &c) {
        c.a[2] = Ram32(c.a[6] + kOpponent);
        ClearAt(c, Ram32(c.a[2] + kBannerPlace));
    }

    // 00C8F8. Draws the player's banner where it is this frame. A chain's banner is not shown for a chain of
    // one, and says X for ten or more.
    void ShowBanner(M68K &c) {
        const uint32_t player = c.a[6];
        const uint8_t timer = Ram8(player + kBannerTimer);
        const uint8_t last = 1, firstAtRest = kBannerFrames - 3;
        if (timer == last || timer == firstAtRest) {
            ClearBanner(c); // (it moves this frame)
            Spend(c, kClearTime);
        }
        const uint32_t place = Ram32(player + kBannerPlace) - (timer <= last ? 2 * kDown : timer <= firstAtRest ? kDown : 0);
        switch (Ram8(player + kBannerKind)) {
        case kBannerChain: {
            const int chain = Ram8(player + kBannerNumber);
            if (chain == 1) {
                break;
            }
            char text[] = " 0 CHAIN!";
            text[1] = chain > kChainShownMost ? 'X' : static_cast<char>('0' + chain);
            PutText(c, place, text);
            break;
        }
        case kBannerTechBonus:
            PutText(c, place, "TECH BONUS");
            break;
        case kBannerAllClear:
            PutText(c, place, "ALL CLEAR!");
            break;
        case kBannerFirstAttack:
            PutText(c, place, "1ST ATTACK");
            break;
        }
    }

    void StartBanner(M68K &c, uint32_t player, uint8_t kind) {
        c.d[0] = 0;
        SetRam8(player + kBannerKind, kind);
        SetRam8(player + kBannerTimer, kBannerFrames);
    }
    // 00C89E. A break has been scored: the chain's length so far
    void ShowChainBanner(M68K &c) {
        const uint32_t player = c.a[6];
        SetRam8(player + kBannerNumber, Ram8(player + kChainLength));
        StartBanner(c, player, kBannerChain);
        ClearBanner(c);
    }
    // 00C8B8. The diamond has landed on the floor
    void ShowTechBonus(M68K &c) {
        StartBanner(c, c.a[6], kBannerTechBonus);
        ClearBanner(c);
    }
    // 00C8CC. The field has been emptied
    void ShowAllClear(M68K &c) {
        StartBanner(c, c.a[6], kBannerAllClear);
        ClearBanner(c);
    }
    // 00C8E0. The round's first attack is coming in: the words go over the attacker's field
    void ShowFirstAttack(M68K &c) {
        StartBanner(c, Ram32(c.a[6] + kOpponent), kBannerFirstAttack);
        ClearOpponentBanner(c);
    }
} // namespace

void RegisterBanner() {
    // (these write to the picture's memory, which the check of a rewrite does not look at: what they draw is
    // checked by the pictures of whole runs, in check_rewrites.sh. The last number is the time a call is charged.)
    constexpr uint32_t kScratch = kFreeD0 | kFreeD1 | kFreeA0 | kFreeA1 | kFreeFlags;
    AddRewrite(0x00C948, "ClearBanner", ClearBanner, kScratch, 2896);
    AddRewrite(0x00C9BE, "ClearOpponentBanner", ClearOpponentBanner, kScratch, 2920);
    AddRewrite(0x00C8F8, "ShowBanner", ShowBanner, kScratch, 210);
    AddRewrite(0x00C89E, "ShowChainBanner", ShowChainBanner, kScratch, 2990);
    AddRewrite(0x00C8B8, "ShowTechBonus", ShowTechBonus, kScratch, 2980);
    AddRewrite(0x00C8CC, "ShowAllClear", ShowAllClear, kScratch, 2980);
    AddRewrite(0x00C8E0, "ShowFirstAttack", ShowFirstAttack, kScratch | kFreeA2, 3000);
}

} // namespace cps2recomp
