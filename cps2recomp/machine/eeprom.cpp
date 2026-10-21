// SPDX-License-Identifier: GPL-3.0-only
// Copyright (C) 2026 Tommy McLeroy. Part of cps2recomp.

#include "eeprom.h"

#include "../runtime/state.h"

namespace cps2recomp {

uint32_t Eeprom::Get(int at) const {
    return eightBit ? reinterpret_cast<const uint8_t *>(words)[at] : words[at];
}
void Eeprom::Set(int at, uint32_t value) {
    if (eightBit) {
        reinterpret_cast<uint8_t *>(words)[at] = static_cast<uint8_t>(value);
    } else {
        words[at] = static_cast<uint16_t>(value);
    }
}

Eeprom::Eeprom() {
    for (auto &word : words) {
        word = 0xFFFF; // (erased)
    }
}

void Eeprom::SetLines(bool select, bool clock, bool dataIn) {
    if (!select) {
        // Deselecting ends a command
        m_select = false;
        m_clock = clock;
        m_started = false;
        m_count = 0;
        m_shift = 0;
        m_reading = -1;
        m_writing = 0;
        m_out = true; // (ready)
        return;
    }
    const bool rising = clock && !m_clock;
    m_select = true;
    m_clock = clock;
    if (!rising) {
        return;
    }
    if (m_reading >= 0) {
        // A read sends its sixteen bits (or eight), high bit first, and goes on to the next word
        if (m_reading == 0) {
            m_address = (m_address + 1) & AddressMask();
            m_sending = Get(m_address);
            m_reading = Bits();
        }
        m_reading--;
        m_out = (m_sending >> m_reading) & 1;
        return;
    }
    if (!m_started) {
        m_started = dataIn;
        return;
    }
    m_shift = (m_shift << 1) | (dataIn ? 1 : 0);
    m_count++;
    if (m_writing) {
        if (m_count == Bits()) {
            if (m_writable) {
                if (m_writing == 2) {
                    for (int n = 0; n <= AddressMask(); n++) {
                        Set(n, m_shift);
                    }
                } else {
                    Set(m_address, m_shift);
                }
                changed = true;
            }
            m_writing = 0;
            m_started = false;
            m_count = 0;
            m_shift = 0;
        }
        return;
    }
    const int addressBits = eightBit ? 7 : 6;
    if (m_count < 2 + addressBits) {
        return;
    }
    // Two bits of command and six of address (seven of bytes)
    const unsigned command = (m_shift >> addressBits) & 3;
    m_address = static_cast<int>(m_shift) & AddressMask();
    m_count = 0;
    m_shift = 0;
    switch (command) {
    case 2: // read (a zero bit comes first)
        m_sending = Get(m_address);
        m_reading = Bits();
        m_out = false;
        break;
    case 1: m_writing = 1; break;
    case 3: // erase
        if (m_writable) {
            Set(m_address, 0xFFFF);
            changed = true;
        }
        m_started = false;
        break;
    default:
        switch (m_address >> (addressBits - 2)) {
        case 0: m_writable = false; break;
        case 3: m_writable = true; break;
        case 1: m_writing = 2; break;
        default: // erase all
            if (m_writable) {
                for (auto &word : words) {
                    word = 0xFFFF;
                }
                changed = true;
            }
            break;
        }
        if (!m_writing) {
            m_started = false;
        }
        break;
    }
}

void Eeprom::Serialize(Archive &ar) {
    ar.Mark(0x45455052);
    ar.Pod(words);
    ar.Pod(m_select);
    ar.Pod(m_clock);
    ar.Pod(m_out);
    ar.Pod(m_writable);
    ar.Pod(m_started);
    ar.Pod(m_shift);
    ar.Pod(m_count);
    ar.Pod(m_reading);
    ar.Pod(m_sending);
    ar.Pod(m_address);
    ar.Pod(m_writing);
}

} // namespace cps2recomp
