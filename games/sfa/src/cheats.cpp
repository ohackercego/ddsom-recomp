// SPDX-License-Identifier: GPL-3.0-only
//
// Street Fighter Alpha: cheats (optional features: the launcher's Features page and the in-game menu switch
// them). Made-up play needs them to go up the ladder to its end ('make explore', game.cfg's explore features).
#include "sfa.h"

#include "state.h"

namespace cps2recomp {

namespace {
    using namespace sfa;

    // Which player's record an address is (0, 1), or -1
    int PlayerOf(uint32_t record) {
        record &= 0xFFFFFF;
        return record == Player(0) ? 0 : record == Player(1) ? 1 : -1;
    }

    // D3 is the damage about to come off the health of the player whose record is `victim` (anything else's is
    // left alone: none was seen over the movies and 130,000 frames of made-up play, but nothing rules one out)
    bool DamageTo(M68K &c, uint32_t victim) {
        const int player = PlayerOf(victim);
        if (player < 0) {
            return false;
        }
        if (FeatureOn(player == 0 ? "no_damage" : "no_damage_2")) {
            SetLow16(c.d[3], 0);
        } else if (player == 1 && FeatureOn("one_hit")) {
            SetLow16(c.d[3], kHealthFull + 1);
        }
        return false;
    }

    // The hit's routine also takes one point off by itself (00FA20-00FA26, the way in from 00F8E2 and 00F906)
    // without D3: for no damage, the player's health is noted where that starts and put back where the two ways
    // meet again (00FA2A), into that player's record only. (Kept with the game's state: a frame can end between the
    // two, and a state saved or run-ahead's frame taken back there must not leave one player's health to be put
    // into the other's.)
    struct Kept {
        uint32_t record = 0; // whose health is kept (0: none)
        int16_t health = 0;
    };
    Kept g_kept;
} // namespace

void CheatsState(Archive &ar) {
    ar.Mark(0x43484541); // CHEA
    ar.Pod(g_kept);
}

void RegisterCheats() {
    // 00FA06: sub.w d3,0x40(a1), a hit's damage off the victim's health (A1)
    AddHook(0x00FA06, "HitDamage", [](M68K &c) { return DamageTo(c, c.a[1]); });
    // 021148: sub.w d3,0x40(a4), a throw's
    AddHook(0x021148, "ThrowDamage", [](M68K &c) { return DamageTo(c, c.a[4]); });
    AddHook(0x00FA20, "HitPoint", [](M68K &c) {
        const uint32_t record = c.a[1] & 0xFFFFFF;
        const int player = PlayerOf(record);
        g_kept = {};
        if (player >= 0 && FeatureOn(player == 0 ? "no_damage" : "no_damage_2")) {
            g_kept = {record, static_cast<int16_t>(Ram16(record + kHealth))};
        }
        return false;
    });
    AddHook(0x00FA2A, "HitPointDone", [](M68K &c) {
        if (g_kept.record != 0 && g_kept.record == (c.a[1] & 0xFFFFFF)) {
            SetRam16(g_kept.record + kHealth, static_cast<uint16_t>(g_kept.health));
        }
        g_kept = {};
        return false;
    });
    // 004B3A: subq.b #1 off the round's clock, a second gone (below nothing, time is up). With the cheat the second
    // is given back first, so the clock stands still.
    AddHook(0x004B3A, "ClockSecond", [](M68K &) {
        if (FeatureOn("infinite_time")) {
            SetRam8(kRoundClock, static_cast<uint8_t>(Ram8(kRoundClock) + 1));
        }
        return false;
    });
    Game &game = TheGame();
    game.features.push_back({"no_damage", "Cheats", "No damage", "Player 1's health does not go down."});
    game.features.push_back({"no_damage_2", "Cheats", "No damage, player 2", "Player 2's health does not go down (the opponent's, in a one-player game)."});
    game.features.push_back({"one_hit", "Cheats", "One-hit knockouts", "Player 2 (the opponent, in a one-player game) goes down with the first hit."});
    game.features.push_back({"infinite_time", "Cheats", "Infinite time", "The round's clock stands still."});
}

} // namespace cps2recomp
