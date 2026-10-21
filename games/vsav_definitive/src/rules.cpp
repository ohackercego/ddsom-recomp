// SPDX-License-Identifier: GPL-3.0-only
// Copyright (C) 2026 Tommy McLeroy
//
// Vampire Savior Definitive: what Vampire Savior 2's engine does otherwise for the characters taken from it, where
// this game's engine has the same code without it (Vampire Savior 2's code compares the character's number there).
// Each is a hook at the place of this game's code that Vampire Savior 2's changed; the place there is in its remark.
// What Vampire Savior 2 changed for every character (its own rules) is not here: the characters taken in play by
// this game's rules.
#include "definitive.h"
#include "vsav.h"

namespace cps2recomp {

namespace {
    using namespace vsav;
    using namespace definitive;

    uint8_t CharacterOf(uint32_t record) {
        return Ram8((record & 0xFFFFFF) + kCharacter);
    }
    // The player a player's record faces (its +32: the other's record, a word)
    uint32_t Opponent(uint32_t record) {
        return 0xFF0000u | Ram16((record & 0xFFFFFF) + 0x32);
    }

    // 00A718 (008F50 there): with Jedah (A1) against anyone but Donovan (A0) and A5+160 at 4, A5+146 is set to 4
    bool JedahAgainst(M68K &c) {
        if (CharacterOf(c.a[0]) != kDonovan) {
            return false;
        }
        c.pc = At(0x00A730);
        return true;
    }

    // 018460, 018508 (016D2C, 016DDC there): the reaction to an attack, by its kind (A3+17; D0 doubled), for the
    // player hit (A1): to one of kind 33, Donovan's is 0F, not the kind's own (0170C8 there)
    bool ReactionByKind(M68K &c) {
        if ((c.d[0] & 0xFFFF) == 0x33 * 2 && CharacterOf(c.a[1]) == kDonovan) {
            SetRam8((c.a[1] & 0xFFFFFF) + 0x54, 0x0F);
            c.cycles += 32;
            c.pc = c.Pop32();
            return true;
        }
        return OffsetTable(c); // (and the kinds Vampire Savior 2 has more of: graft.cfg, extend)
    }

    // 024C80 (0238C4 there): Donovan does not call 004122 (0041BE there) as this state starts
    bool NoCallForDonovan(M68K &c) {
        if (CharacterOf(c.a[6]) != kDonovan) {
            return false;
        }
        c.pc = At(0x024C86);
        return true;
    }

    // 025CA0 (0248FA there): Donovan does not turn about as this state starts
    bool NoTurnForDonovan(M68K &c) {
        if (CharacterOf(c.a[6]) != kDonovan) {
            return false;
        }
        c.pc = At(0x025CA6);
        return true;
    }

    // 05F152 (06B18A-06B1A0 there): a palette by the character (D6) and its colours (D0), in a table of 32 bytes each
    // (3AC000 here; 3C117C there): theirs are Vampire Savior 2's: Donovan's at D0 + C6, Pyron's at D0 + BC, Phobos's
    // as character 0B's
    constexpr uint32_t kTheirPalettes = 0x3C117C;
    bool TheirPalette(M68K &c) {
        const uint8_t character = c.d[6] & 0xFF;
        if (!IsTaken(character) || !ExtraBase()) {
            return false;
        }
        c.a[0] = ExtraBase() + kTheirPalettes;
        if (character == kPhobos) {
            SetLow8(c.d[6], 0x0B);
            c.pc = At(0x05F15E); // (on to lsl.w #4,d0 and add.w d6,d0)
            return true;
        }
        SetLow16(c.d[0], (c.d[0] + (character == kPyron ? 0xBC : 0xC6)) & 0xFFFF);
        c.pc = At(0x05F162); // (on to lsl.w #5,d0 and the lea)
        return true;
    }

    // 07A95C, 07A98C (085A68, 085ABA there): how high the thing is put by its player's (A4) opponent's height
    // (D0: its +36 halved): 8 less for Donovan, 4 more for Phobos and Pyron
    bool HeightAgainstTaken(M68K &c) {
        const uint8_t opponent = CharacterOf(Opponent(c.a[4]));
        if (opponent == kDonovan) {
            SetLow16(c.d[0], (c.d[0] - 8) & 0xFFFF);
        } else if (opponent == kPhobos || opponent == kPyron) {
            SetLow16(c.d[0], (c.d[0] + 4) & 0xFFFF);
        }
        return false; // (the game's own code goes on)
    }

    // 030AB0 (0304D8 there): a player in this state against Phobos, unless +3B4, takes another way (+0A 78): always
    // with +394 at 5, else one time in 16 (by the game's random numbers, 014E8A here)
    bool AgainstPhobos(M68K &c) {
        const uint32_t player = c.a[6] & 0xFFFFFF;
        if (Ram8(player + 0x3B4) || CharacterOf(Opponent(player)) != kPhobos) {
            return false;
        }
        bool other = Ram8(player + 0x394) == 5;
        if (!other) {
            Call(c, At(0x014E8A));
            other = (c.d[0] & 0xF) == 0;
        }
        if (other) {
            SetRam8(player + 0x0A, 0x78);
        }
        return false;
    }

    // 060ED6 (06D304 there): a thing put on its player's (A6) opponent takes Pyron's place (+0E) and palette, less 3
    bool OnPyron(M68K &c) {
        const uint32_t opponent = Opponent(c.a[6]);
        if (CharacterOf(opponent) == kPyron) {
            const uint32_t thing = c.a[4] & 0xFFFFFF;
            SetRam16(thing + 0x0E, Ram16(opponent + 0x0E));
            SetRam8(thing + 0x0F, Ram8(thing + 0x0F) - 3);
        }
        return false;
    }

    // 00A446 (008C58 there): the arcade's winner, whose ending is shown (A5+130), is the player's character (A0)
    // with its high bits off here (this game's endings are by 0-F), as it is there for theirs: their ending is
    // Vampire Savior 2's (graft.cfg: divert 021A20)
    bool Winner(M68K &c) {
        const uint8_t character = CharacterOf(c.a[0]);
        if (IsTaken(character)) {
            SetLow16(c.d[0], character);
        }
        return false;
    }

    // 01C6FA, 01C712 (01B180 there): a character's (D0) palette for the pictures between matches, in a table of 32
    // bytes each (3AD400 here, whose places from 10 on are other palettes; 3C117C there, by the number its byte table
    // at 01B1C0 gives the character): theirs is Vampire Savior 2's
    constexpr uint32_t kTheirPictureColours = 0x01B1C0;
    bool PictureColours(M68K &c) {
        const uint8_t character = c.d[0] & 0xFF;
        if (!IsTaken(character) || !ExtraBase()) {
            return false;
        }
        const uint8_t number = *ExtraOpcodes(kTheirPictureColours + character);
        c.a[0] = ExtraBase() + kTheirPalettes + static_cast<uint32_t>(number) * 32;
        c.pc = (c.pc & 0xFFFFFF) + 12; // (on to the lea of where they go: movea.l #, lsl.w #5, lea passed)
        return true;
    }

    // 01A888 (019292 there): a thing thrown's kind of meeting another, by its type (D0), in a byte table after the
    // routine (this game's has 40, theirs 50): the types Vampire Savior 2 added, theirs (3B on) by its table
    constexpr uint8_t kTheirThrownTypes = 0x3B;
    constexpr uint32_t kTheirMeetings = 0x019298;
    bool ThrownMeeting(M68K &c) {
        const uint8_t type = c.d[0] & 0xFF;
        if (type < kTheirThrownTypes || !ExtraBase()) {
            return false;
        }
        SetLow8(c.d[0], *ExtraOpcodes(kTheirMeetings + type));
        c.cycles += 14;
        c.pc = c.Pop32();
        return true;
    }

    // 05F1B6 (06B290 there): a player's (A4) five palettes for the talk before the last opponent, by its character
    // (D6) and its colours (+3AE), in a table of five palettes each (3AD700 here, 17 by the colours; 3C2BBC there,
    // 18, at the place its byte table at 06B2F2 gives the character): theirs are Vampire Savior 2's
    constexpr uint32_t kTheirTalkPalettes = 0x3C2BBC, kTheirTalkPlaces = 0x06B2F2;
    bool TalkPalettes(M68K &c) {
        const uint8_t character = c.d[6] & 0xFF;
        if (!IsTaken(character) || !ExtraBase()) {
            return false;
        }
        const uint32_t colours = Ram8((c.a[4] & 0xFFFFFF) + 0x3AE);
        const uint32_t place = colours * 18 + *ExtraOpcodes(kTheirTalkPlaces + character);
        c.a[0] = ExtraBase() + kTheirTalkPalettes + place * 5 * 32;
        c.d[7] = 4; // (moveq)
        c.pc = At(0x01C3A4);
        return true;
    }
} // namespace

void definitive::RegisterRules() {
    AddHook(At(0x05F1B6), "TalkPalettes", TalkPalettes);
    AddHook(At(0x01A888), "ThrownMeeting", ThrownMeeting);
    AddHook(At(0x01C6FA), "PictureColours", PictureColours);
    AddHook(At(0x01C712), "PictureColours", PictureColours);
    AddHook(At(0x00A446), "Winner", Winner);
    AddHook(At(0x00A720), "JedahAgainst", JedahAgainst);
    AddHook(At(0x018460), "ReactionByKind", ReactionByKind);
    AddHook(At(0x018508), "ReactionByKind", ReactionByKind);
    AddHook(At(0x024C80), "NoCallForDonovan", NoCallForDonovan);
    AddHook(At(0x025CA0), "NoTurnForDonovan", NoTurnForDonovan);
    AddHook(At(0x05F152), "TheirPalette", TheirPalette);
    AddHook(At(0x07A95C), "HeightAgainstTaken", HeightAgainstTaken);
    AddHook(At(0x07A98C), "HeightAgainstTaken", HeightAgainstTaken);
    AddHook(At(0x030AB0), "AgainstPhobos", AgainstPhobos);
    AddHook(At(0x060ED6), "OnPyron", OnPyron);
}

} // namespace cps2recomp
