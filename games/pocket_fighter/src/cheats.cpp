// SPDX-License-Identifier: GPL-3.0-only
// Copyright (C) 2026 Tommy McLeroy
//
// Super Gem Fighter Mini Mix: cheats (optional features: the launcher's Features page and the in-game menu switch
// them). Made-up play needs them to go up the ladder to its end ('make explore', game.cfg's explore features).
#include "pocket.h"
#include "session.h"

namespace cps2recomp {

namespace {
    using namespace pocket;

    // Which player's record an address is (0, 1), or -1
    int PlayerOf(uint32_t record) {
        record &= 0xFFFFFF;
        return record == Player(0) ? 0 : record == Player(1) ? 1 : -1;
    }

    // 015D6E: sub.w d4,0x40(a1), a hit's or a throw's damage (D4) off the health of the victim (A1). Health below
    // nothing after it is a knockout (015CE6), so one-hit knockouts take off more than a full bar; with no knockouts
    // (a training session, the modes that want it) a hit takes no more than the health there is.
    bool Damage(M68K &c) {
        const int player = PlayerOf(c.a[1]);
        if (player < 0) {
            return false;
        }
        if (c.d[4] & 0xFFFF) {
            session::Hit(player, c.d[4] & 0xFFFF); // (a guarded hit takes nothing, and is not a hit)
        }
        if (FeatureOn(player == 0 ? "no_damage" : "no_damage_2")) {
            SetLow16(c.d[4], 0);
        } else if (player == 1 && FeatureOn("one_hit")) {
            SetLow16(c.d[4], kHealthFull + 1);
        }
        const int16_t health = static_cast<int16_t>(Ram16(Player(player) + kHealth));
        if (session::NoKnockOut() && static_cast<int16_t>(c.d[4] & 0xFFFF) > health) {
            SetLow16(c.d[4], static_cast<uint32_t>(health > 0 ? health : 0));
        }
        return false;
    }
} // namespace

void RegisterCheats() {
    AddHook(0x015D6E, "Damage", Damage);
    // 0083DC: subq.b #1 off the frames left of the round's second; at nothing a second comes off the clock (0083E6)
    // and the frames start again. With the cheat the frame is given back first, so the clock stands still.
    AddHook(0x0083DC, "ClockFrame", [](M68K &) {
        if (FeatureOn("infinite_time") || session::NoClock()) {
            SetRam8(kClockFrames, static_cast<uint8_t>(Ram8(kClockFrames) + 1));
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
