// SPDX-License-Identifier: GPL-3.0-only
// Copyright (C) 2026 Tommy McLeroy. Part of cps2recomp.

#include "sound_board.h"

#include "../runtime/state.h"

#include <algorithm>
#include <cstring>

namespace cps2recomp {

namespace {
    constexpr uint64_t kInterruptPeriod = 32000; // Z80 cycles: 250 a second (as the machine's)
    // Where the 68000 puts a request's bytes in the shared memory (the queue entry's byte n at kPlaces[n]), and the
    // byte it clears last to say a request is there (the Z80 sets it to FF as it takes one)
    constexpr uint8_t kPlaces[12] = {0x03, 0x04, 0x00, 0x01, 0x02, 0x06, 0x07, 0x08, 0x0B, 0x0C, 0x09, 0x0A};
    constexpr uint16_t kTaken = 0x0F;
} // namespace

SoundBoard::SoundBoard(const RomSet &roms)
    : m_roms(roms)
    , m_cpu(*this)
    , m_chip(roms.samples.data(), roms.samples.size()) {
    Reset();
}

void SoundBoard::Reset() {
    memset(m_shared, 0, sizeof(m_shared));
    memset(m_own, 0, sizeof(m_own));
    m_bank = 0;
    m_running = false;
    m_interruptAt = kInterruptPeriod;
    m_waiting.clear();
    m_chip.Reset();
    m_cpu.Reset();
    m_cpu.cycles = 0;
}

void SoundBoard::SetRunning(bool run) {
    if (run && !m_running) {
        m_cpu.Reset();
    }
    m_running = run;
}

void SoundBoard::Request(uint32_t number, uint32_t second, uint32_t third) {
    std::array<uint8_t, 12> entry;
    const uint32_t longs[3] = {number, second, third};
    for (int n = 0; n < 12; n++) {
        entry[static_cast<size_t>(n)] = static_cast<uint8_t>(longs[n / 4] >> (24 - 8 * (n % 4)));
    }
    if (m_waiting.size() < 256) { // (the game's own ring holds 256)
        m_waiting.push_back(entry);
    }
}

// The next request, if the Z80 has taken the last
void SoundBoard::Send() {
    if (m_waiting.empty() || m_shared[kTaken] != 0xFF) {
        return;
    }
    const std::array<uint8_t, 12> &entry = m_waiting.front();
    for (int n = 0; n < 12; n++) {
        m_shared[kPlaces[n]] = entry[static_cast<size_t>(n)];
    }
    m_shared[kTaken] = 0;
    m_waiting.erase(m_waiting.begin());
}

void SoundBoard::RunTo(uint64_t cycles) {
    Send();
    while (m_cpu.cycles < cycles) {
        const uint64_t next = std::min(cycles, m_interruptAt);
        if (m_running) {
            m_cpu.Run(next, Z80::Mode::Interpreter);
        }
        m_cpu.cycles = std::max(m_cpu.cycles, next);
        while (m_interruptAt <= m_cpu.cycles) {
            m_interruptAt += kInterruptPeriod;
            if (m_running) {
                m_cpu.RaiseInterrupt();
            }
        }
    }
}

uint8_t SoundBoard::Read(uint16_t address) {
    const std::vector<uint8_t> &program = m_roms.soundProgram;
    if (address < 0x8000) {
        return address < program.size() ? program[address] : 0xFF;
    }
    if (address < 0xC000) {
        const size_t at = 0x8000 + static_cast<size_t>(m_bank) * 0x4000 + (address - 0x8000);
        return at < program.size() ? program[at] : 0xFF;
    }
    if (address < 0xD000) {
        return m_shared[address - 0xC000];
    }
    if (address == 0xD007) {
        return m_chip.Read();
    }
    if (address >= 0xF000) {
        return m_own[address - 0xF000];
    }
    return 0xFF;
}

void SoundBoard::Write(uint16_t address, uint8_t value) {
    if (address >= 0xC000 && address < 0xD000) {
        m_shared[address - 0xC000] = value;
    } else if (address >= 0xD000 && address <= 0xD002) {
        m_chip.Write(address - 0xD000, value);
    } else if (address == 0xD003) {
        m_bank = value & 0x0F;
    } else if (address >= 0xF000) {
        m_own[address - 0xF000] = value;
    }
}

void SoundBoard::Serialize(Archive &ar) {
    ar.Mark(0x534E4432);
    m_cpu.Serialize(ar);
    m_chip.Serialize(ar);
    ar.Pod(m_shared);
    ar.Pod(m_own);
    ar.Pod(m_bank);
    ar.Pod(m_running);
    ar.Pod(m_interruptAt);
    ar.Vec(m_waiting);
}

} // namespace cps2recomp
