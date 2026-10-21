// SPDX-License-Identifier: GPL-3.0-only
// Copyright (C) 2026 Tommy McLeroy. Part of cps2recomp.

#include "machine.h"

#include "../runtime/state.h"

#include <algorithm>
#include <map>
#include <cstdio>
#include <cstdlib>
#include <cstring>

namespace cps2recomp {

namespace {
    constexpr uint64_t kSoundInterruptPeriod = 32000; // Z80 cycles: 250 a second
    constexpr uint64_t kSoundClock = 8000000;

    inline uint16_t Word(const uint8_t *p) {
        return static_cast<uint16_t>((p[0] << 8) | p[1]);
    }
    inline void SetWord(uint8_t *p, uint16_t value) {
        p[0] = static_cast<uint8_t>(value >> 8);
        p[1] = static_cast<uint8_t>(value);
    }
    void Unmapped(const char *what, uint32_t address, uint32_t pc) {
        static int reports = getenv("CPS2RECOMP_UNMAPPED") ? 200 : 0;
        if (reports > 0) {
            reports--;
            fprintf(stderr, "%s of %06X (pc %06X): nothing there\n", what, address, pc);
        }
    }
} // namespace

Machine::Machine(const RomSet &roms)
    : cpu(*this)
    , soundCpu(*this)
    , qsound(roms.samples.data(), roms.samples.size())
    , video(Video::Sources{})
    , ram(0x10000)
    , videoRam(0x30000)
    , objectRam(0x4000)
    , extraRam(0x4000)
    , m_roms(roms)
    , m_picture(static_cast<size_t>(Video::kMaxWidth) * Video::kHeight)
    , m_cps1(roms.board.cps1)
    , m_cyclesPerLine(roms.board.cps1 ? kCps1CpuCyclesPerLine : kCpuCyclesPerLine) {
    Video::Sources sources;
    sources.graphics = roms.graphics.data();
    sources.graphicsSize = roms.graphics.size();
    sources.videoRam = videoRam.data();
    sources.cpsA = cpsA;
    sources.cpsB = cpsB;
    sources.objectRegs = objectRegs;
    sources.board = roms.board;
    video = Video(sources);
    eeprom.eightBit = m_cps1;
    if (const char *layers = getenv("CPS2RECOMP_LAYERS")) {
        video.layerMask = static_cast<unsigned>(strtoul(layers, nullptr, 0)); // (bit 0 objects, 1-3 scroll screens)
    }
    cpu.ram = ram.data();
    cpu.video = videoRam.data();
    cpu.videoSize = static_cast<uint32_t>(videoRam.size());
    cpu.rom = roms.program.data();
    cpu.opcodes = roms.opcodes.data();
    cpu.romSize = static_cast<uint32_t>(roms.program.size());
    if (SoundDriver::Knows(roms.soundProgram)) {
        soundDriver = std::make_unique<SoundDriver>(roms.soundProgram, soundRam, [this](uint8_t reg, uint16_t data, uint32_t gap) {
            if (soundSide == SoundSide::Check) {
                m_driverWrites.push_back({reg, data, 0, gap});
                return;
            }
            if (m_chipQueue.empty()) {
                m_chipNext = std::max(soundCpu.cycles, m_chipLast) + gap;
            }
            m_chipQueue.push_back({reg, data, 0, gap});
        });
    }
    if (getenv("CPS2RECOMP_SOUND_GAPS")) {
        m_gapStats = std::make_unique<std::map<uint16_t, GapStat>>();
    }
    Reset();
}

void Machine::Reset() {
    std::fill(ram.begin(), ram.end(), 0);
    std::fill(videoRam.begin(), videoRam.end(), 0);
    std::fill(objectRam.begin(), objectRam.end(), 0);
    std::fill(extraRam.begin(), extraRam.end(), 0);
    std::fill(scrollFromExtra.begin(), scrollFromExtra.end(), 0);
    memset(soundRam, 0, sizeof(soundRam));
    memset(soundRam2, 0, sizeof(soundRam2));
    soundtrack.Reset();
    memset(cpsA, 0, sizeof(cpsA));
    memset(cpsB, 0, sizeof(cpsB));
    memset(objectRegs, 0, sizeof(objectRegs));
    // (where the video chip's memory areas are until the game says)
    cpsA[0] = 0x9200;
    cpsA[1] = 0x9000;
    cpsA[2] = 0x9040;
    cpsA[3] = 0x9080;
    cpsA[4] = 0x9100;
    m_line = 0;
    m_lineStart = 0;
    cpu.cycles = 0;
    m_blankInterrupt = m_rasterInterrupt = false;
    m_rasterCounter[0] = m_rasterCounter[1] = m_rasterCounter[2] = 0;
    m_rasterReload[0] = m_rasterReload[1] = 0x1FF;
    m_rasterReload[2] = 0;
    m_objectBank = 0;
    m_soundRunning = false;
    m_soundBank = 0;
    m_soundTime = 0;
    m_soundInterruptAt = kSoundInterruptPeriod;
    m_sampleCount = 0;
    m_sampleAt = kSoundClock / QSound::kSampleRate;
    soundCpu.cycles = 0;
    frame = 0;
    audio.clear();
    video.Reset();
    qsound.Reset();
    soundCpu.Reset();
    if (extraSound) {
        extraSound->Reset();
    }
    cpu.Reset(true);
    if (m_cps1) {
        // (a CPS-1's Z80 is never held: it runs from the start)
        m_soundRunning = true;
        if (soundDriver && soundSide != SoundSide::Z80) {
            soundDriver->shadow = soundSide == SoundSide::Check;
            soundDriver->Reset();
        }
    }
}

// --- time ---

void Machine::UpdateInterruptLevel() {
    // The two interrupt sources drive two of the CPU's three level lines: together they read as level 6
    cpu.SetInterruptLevel(static_cast<uint8_t>((m_blankInterrupt ? 2 : 0) | (m_rasterInterrupt ? 4 : 0)));
}

void Machine::InterruptAck(unsigned) {
    m_blankInterrupt = m_rasterInterrupt = false;
    UpdateInterruptLevel();
}

void Machine::RunFrame() {
    // (the picture's width is the frame's: what is asked for while it runs is for the next)
    // (for tools: CPS2RECOMP_WIDE=n draws n more columns on each side whatever the game's sources ask, to see
    // what a game has beyond its picture's edges)
    static const char *const forced = getenv("CPS2RECOMP_WIDE");
    if (forced) {
        video.wide.side = atoi(forced);
    }
    video.wide.side = std::clamp(video.wide.side, 0, static_cast<int>(Video::kMaxSide));
    m_pictureWidth = video.Width();
    do {
        RunLine();
    } while (m_line != kBlankingLine);
    soundtrack.Frame();
    frame++;
    // (CPS2RECOMP_BUS_LOG=FILE: each frame's bus waits and DMA clocks, with the board's timing; docs/TIMING.md)
    static FILE *busLog = getenv("CPS2RECOMP_BUS_LOG") ? fopen(getenv("CPS2RECOMP_BUS_LOG"), "w") : nullptr;
    if (busLog) {
        fprintf(busLog, "%llu %llu %llu\n", static_cast<unsigned long long>(frame), static_cast<unsigned long long>(cpu.waitsCounted),
                static_cast<unsigned long long>(dmaCounted));
        cpu.waitsCounted = 0;
        dmaCounted = 0;
    }
}

void Machine::RunLine() {
    // The video chip's two line counters count down each line (and start again at the top of the frame);
    // one reaching zero asks for a raster interrupt, which comes as far into the line as the third says
    bool raster = false;
    for (int n = 0; n < 2 && !m_cps1; n++) {
        if (m_line == 0) {
            m_rasterCounter[n] = m_rasterReload[n];
        } else {
            m_rasterCounter[n] = (m_rasterCounter[n] - 1) & 0x1FF;
        }
        raster = raster || m_rasterCounter[n] == 0;
    }
    m_rasterCounter[2] = m_rasterReload[2];

    if (m_line == kBlankingLine) {
        m_blankInterrupt = true;
        UpdateInterruptLevel();
        if (m_cps1) {
            video.LatchObjectsCps1();
        } else {
            video.LatchObjects(&objectRam[m_objectBank ? 0x2000 : 0], m_extra ? objectsFromExtra[m_objectBank ? 1 : 0] : nullptr);
        }
    }

    // (the board's timing: the video chip's DMA holds the 68000's bus at the start of the line)
    if (boardTiming) {
        const uint32_t dma = LineDma();
        cpu.cycles += dma;
        dmaCounted += dma;
    }

    // (the CPU stops a little after the time it is given, and the next line is that much shorter for it)
    const uint64_t start = m_lineStart;
    m_lineStart += static_cast<uint64_t>(m_cyclesPerLine * cpuSpeed);
    if (raster) {
        cpu.Run(start + static_cast<uint64_t>(m_rasterCounter[2]) * 4, cpuMode);
        m_rasterInterrupt = true;
        UpdateInterruptLevel();
    }
    cpu.Run(m_lineStart, cpuMode);

    RunSound(kSoundCyclesPerLine);

    if (m_line >= Video::kFirstLine && m_line < Video::kFirstLine + Video::kHeight) {
        const int row = m_line - Video::kFirstLine;
        video.RenderLine(row, &m_picture[static_cast<size_t>(row) * m_pictureWidth]);
    }
    m_line = (m_line + 1) % kLines;
}

// The Z80 and the QSound chip for a stretch of time. The Z80's cycle count is the sound side's clock: it
// runs to the next thing due (its interrupt, 250 times a second; the chip's next sample; the end of the
// stretch), a little past it, and then what has come due happens.
void Machine::RunSound(int cycles) {
    m_soundTime += static_cast<uint64_t>(cycles);
    while (soundCpu.cycles < m_soundTime) {
        const uint64_t next = std::min({m_soundTime, m_soundInterruptAt, m_sampleAt});
        if (m_soundRunning && soundSide != SoundSide::Driver) {
            soundCpu.Run(next, soundMode);
        } else if (m_driverInterrupt) {
            // (where the Z80 would take it: as it next runs)
            m_driverInterrupt = false;
            soundDriver->Interrupt();
        }
        soundCpu.cycles = std::max(soundCpu.cycles, next); // (held in reset or halted: time passes all the same)
        while (m_soundInterruptAt <= soundCpu.cycles) {
            m_soundInterruptAt += kSoundInterruptPeriod;
            if (m_soundRunning) {
                if (soundSide != SoundSide::Driver) {
                    soundCpu.RaiseInterrupt();
                }
                if (soundSide == SoundSide::Check) {
                    soundDriver->Interrupt();
                } else if (soundSide == SoundSide::Driver) {
                    m_driverInterrupt = true;
                }
            }
        }
        while (m_sampleAt <= soundCpu.cycles) {
            if (!m_chipQueue.empty() && m_chipNext <= m_sampleAt) {
                const ChipWrite write = m_chipQueue.front();
                m_chipQueue.erase(m_chipQueue.begin());
                qsound.Write(0, static_cast<uint8_t>(write.data >> 8));
                qsound.Write(1, static_cast<uint8_t>(write.data));
                qsound.Write(2, write.reg);
                m_chipLast = m_sampleAt;
                if (!m_chipQueue.empty()) {
                    m_chipNext = m_chipLast + m_chipQueue.front().gap;
                }
            }
            int16_t sample[2];
            qsound.Step(sample);
            if (extraSound) {
                extraSound->RunTo(m_sampleAt);
                int16_t extra[2];
                extraSound->Step(extra);
                for (int n = 0; n < 2; n++) {
                    sample[n] = static_cast<int16_t>(std::clamp(sample[n] + extra[n], -32768, 32767));
                }
            }
            audio.push_back(sample[0]);
            audio.push_back(sample[1]);
            m_sampleCount++;
            m_sampleAt = (m_sampleCount + 1) * kSoundClock / QSound::kSampleRate;
        }
    }
    if (soundSide == SoundSide::Check) {
        CompareSound();
    }
}

// The Z80's writes to the chip and the driver's must be the same, in the same order (they need not come at
// the same moment: the Z80 takes time over its work)
void Machine::CompareSound() {
    const size_t count = std::min(m_z80Writes.size(), m_driverWrites.size());
    for (size_t n = 0; n < count; n++) {
        const ChipWrite &z80 = m_z80Writes[n], &driver = m_driverWrites[n];
        if (z80.reg == driver.reg && z80.data == driver.data) {
            if (m_gapStats) {
                GapStat &stat = (*m_gapStats)[z80.pc];
                stat.count++, stat.z80 += z80.gap, stat.driver += driver.gap;
            }
            continue;
        }
        m_soundHistory.insert(m_soundHistory.end(), m_z80Writes.begin(), m_z80Writes.begin() + static_cast<ptrdiff_t>(n));
        fprintf(stderr, "sound driver differs from the Z80 at chip write %llu (frame %llu)\n",
                static_cast<unsigned long long>(soundWritesChecked + n), static_cast<unsigned long long>(frame));
        fprintf(stderr, "  before:");
        for (const ChipWrite &w : m_soundHistory) {
            fprintf(stderr, " %02X=%04X@%04X", w.reg, w.data, w.pc);
        }
        fprintf(stderr, "\n");
        auto list = [&](const char *name, const std::vector<ChipWrite> &writes, bool pcs) {
            fprintf(stderr, "  %-7s", name);
            for (size_t k = n > 6 ? n - 6 : 0; k < writes.size() && k < n + 10; k++) {
                fprintf(stderr, "%s%02X=%04X", k == n ? " >" : " ", writes[k].reg, writes[k].data);
                if (pcs) {
                    fprintf(stderr, "@%04X", writes[k].pc);
                }
            }
            fprintf(stderr, "\n");
        };
        list("Z80:", m_z80Writes, true);
        list("driver:", m_driverWrites, false);
        // (what each has for the voice: the Z80's memory for its two tracks, in the sound program's layout)
        const unsigned voice = z80.reg >= 0x80 ? z80.reg & 15 : z80.reg >> 3;
        soundDriver->Describe(voice);
        for (int which = 0; which < 2; which++) {
            fprintf(stderr, "  Z80 %s %u:", which ? "music" : "effect", voice);
            for (unsigned at = 0; at < 0x68; at++) {
                fprintf(stderr, "%s%02X", at % 8 ? " " : "  ", soundRam2[0x200 + which * 0x680 + voice * 0x68 + at]);
            }
            fprintf(stderr, "\n");
        }
        for (unsigned n = 0; n < 3; n++) {
            fprintf(stderr, "  Z80 ADPCM %u:", n);
            for (unsigned at = 0; at < 0x18; at++) {
                fprintf(stderr, "%s%02X", at % 8 ? " " : "  ", soundRam2[0xF00 + n * 0x18 + at]);
            }
            fprintf(stderr, "\n");
        }
        fprintf(stderr, "  Z80 stereo %02X\n", soundRam2[8]);
        exit(3);
    }
    m_soundHistory.insert(m_soundHistory.end(), m_z80Writes.begin(), m_z80Writes.begin() + static_cast<ptrdiff_t>(count));
    if (m_soundHistory.size() > 24) {
        m_soundHistory.erase(m_soundHistory.begin(), m_soundHistory.end() - 24);
    }
    soundWritesChecked += count;
    m_z80Writes.erase(m_z80Writes.begin(), m_z80Writes.begin() + static_cast<ptrdiff_t>(count));
    m_driverWrites.erase(m_driverWrites.begin(), m_driverWrites.begin() + static_cast<ptrdiff_t>(count));
}

// --- the 68000's bus ---

uint16_t Machine::Fetch16(uint32_t address) {
    if (address < m_roms.opcodes.size()) {
        return Word(&m_roms.opcodes[address]);
    }
    if (InExtra(address)) {
        return Word(&m_extra->opcodes[address - m_extraBase]);
    }
    return Read16(address); // (code in RAM is not enciphered)
}

// The 68000's clocks the CPS-A's DMA holds the bus for at the start of this line (docs/TIMING.md), as jtcps2 has the
// chip (jtcps1_dma.v, after jotego's measurements of a board): a word every 4 clocks, of the row scroll's table (one
// a line), a scroll layer's next row of tiles (100, 98 and 30 words) when the line with its vertical scroll reaches
// one, and every enabled palette page (512 words) at the line after the game sets the palette's base. Lines are
// counted as the chip's DMA counts them, two ahead of the picture's; none in vertical blanking but the palette's.
uint32_t Machine::LineDma() {
    const int v = (m_line + 2) % kLines;
    const bool rows = v < 240 || v > 257;
    const uint16_t control = cpsA[0x22 / 2];
    const int next = v + 1;
    uint32_t words = 0;
    if ((control & 1) && rows) {
        words += 1;
    }
    if ((control & 2) && rows && ((next + cpsA[0x0E / 2]) & 7) == 0) {
        words += 100;
    }
    if ((control & 4) && rows && ((next + cpsA[0x12 / 2]) & 15) == 0) {
        words += 98;
    }
    if ((control & 8) && ((rows && ((next + cpsA[0x16 / 2]) & 15) == 0) || v == 0)) {
        words += 30;
    }
    if (m_palettePending) {
        m_palettePending = false;
        unsigned pages = 0;
        for (uint16_t enabled = cpsB[0x30 / 2] & 0x3F; enabled; enabled &= enabled - 1) {
            pages++;
        }
        words += pages * 512;
    }
    return words ? words * 4 + 2 : 0; // (and its start)
}

// 68000 cycles into the line
int Machine::LineCycles() const {
    return static_cast<int>(static_cast<int64_t>(cpu.cycles) - static_cast<int64_t>(m_lineStart - static_cast<uint64_t>(m_cyclesPerLine * cpuSpeed)));
}

uint16_t Machine::Read16(uint32_t address) {
    if (address >= 0xFF0000) {
        return Word(&ram[address & 0xFFFE]);
    }
    if (address < m_roms.program.size()) {
        return Word(&m_roms.program[address]);
    }
    if (address >= 0x900000 && address < 0x930000) {
        return Word(&videoRam[address - 0x900000]);
    }
    if (InExtra(address)) {
        ReadExtra();
        return Word(&m_extra->program[address - m_extraBase]);
    }
    return ReadRegister(address);
}

uint8_t Machine::Read8(uint32_t address) {
    if (address >= 0xFF0000) {
        return ram[address & 0xFFFF];
    }
    if (address < m_roms.program.size()) {
        return m_roms.program[address];
    }
    if (address >= 0x900000 && address < 0x930000) {
        return videoRam[address - 0x900000];
    }
    if (InExtra(address)) {
        ReadExtra();
        return m_extra->program[address - m_extraBase];
    }
    const uint16_t word = ReadRegister(address & ~1u);
    return static_cast<uint8_t>((address & 1) ? word : word >> 8);
}

void Machine::Write16(uint32_t address, uint16_t value) {
    if (address >= 0xFF0000) {
        SetWord(&ram[address & 0xFFFE], value);
        return;
    }
    if (address >= 0x900000 && address < 0x930000) {
        SetWord(&videoRam[address - 0x900000], value);
        VideoWritten(address);
        return;
    }
    WriteRegister(address, value, 0xFFFF);
}

void Machine::Write8(uint32_t address, uint8_t value) {
    if (address >= 0xFF0000) {
        ram[address & 0xFFFF] = value;
        return;
    }
    if (address >= 0x900000 && address < 0x930000) {
        videoRam[address - 0x900000] = value;
        VideoWritten(address);
        return;
    }
    if (address & 1) {
        WriteRegister(address & ~1u, value, 0x00FF);
    } else {
        WriteRegister(address, static_cast<uint16_t>(value << 8), 0xFF00);
    }
}

// The inputs are wired so that a line reads 0 while pressed
uint16_t Machine::Input(int port) const {
    const uint16_t p1 = players[0], p2 = players[1], p3 = players[2], p4 = players[3];
    uint16_t pressed = 0;
    switch (port) {
    case 0: // sticks and punches
        pressed = static_cast<uint16_t>((p1 & 0x7F) | ((p2 & 0x7F) << 8));
        break;
    case 1:
        // A six-button game's kicks (player 2's hard kick is on the next port); a game of three or four players has
        // players 3 and 4 here (stick and three buttons each), the same lines
        pressed = static_cast<uint16_t>(((p1 >> 7) & 7) | (((p2 >> 7) & 3) << 4) | (p3 & 0x7F) | ((p4 & 0x7F) << 8));
        break;
    default:
        pressed = static_cast<uint16_t>((testSwitch ? 0x0002 : 0) | (serviceButton ? 0x0004 : 0) | ((p1 & kStart) ? 0x0100 : 0) |
                                        ((p2 & kStart) ? 0x0200 : 0) | ((p3 & kStart) ? 0x0400 : 0) | ((p4 & kStart) ? 0x0800 : 0) |
                                        ((p1 & kCoin) ? 0x1000 : 0) | ((p2 & kCoin) ? 0x2000 : 0) | ((p3 & kCoin) ? 0x4000 : 0) |
                                        ((p4 & kCoin) ? 0x8000 : 0) | ((p2 & kKick3) ? 0x4000 : 0));
        return static_cast<uint16_t>((~pressed & 0xFFFE) | (eeprom.DataOut() ? 1 : 0));
    }
    return static_cast<uint16_t>(~pressed);
}

uint16_t Machine::ReadRegister(uint32_t address) {
    if (m_cps1) {
        return ReadCps1(address);
    }
    if (address >= 0x400000 && address < 0x40000C) {
        return objectRegs[(address - 0x400000) / 2];
    }
    if (address >= 0x618000 && address < 0x61A000) {
        return static_cast<uint16_t>(0xFF00 | soundRam[(address - 0x618000) / 2]);
    }
    if (address >= 0x660000 && address < 0x664000) {
        return Word(&extraRam[address - 0x660000]);
    }
    if (address >= 0x708000 && address < 0x710000) {
        return Word(&objectRam[(m_objectBank ^ 1) * 0x2000 + (address & 0x1FFE)]);
    }
    if ((address & ~0x4000u) >= 0x800140 && (address & ~0x4000u) < 0x800180) {
        // The CPS-B chip: a 16 x 16 multiplier, and the raster counters
        const unsigned index = (address & 0x3E) / 2;
        switch (index) {
        case 0x04 / 2: return static_cast<uint16_t>(cpsB[0] * cpsB[1]);
        case 0x06 / 2: return static_cast<uint16_t>((static_cast<uint32_t>(cpsB[0]) * cpsB[1]) >> 16);
        case 0x0E / 2:
            // (how far along the line, counted from the third counter in steps of two pixels)
            return static_cast<uint16_t>((((m_rasterCounter[2] - LineCycles() / 4) << 1) & 0x1FE) | (cpsB[0x0E / 2] & 1));
        case 0x10 / 2: return m_rasterCounter[0] & 0x1FF;
        case 0x12 / 2: return m_rasterCounter[1] & 0x1FF;
        default: return 0xFFFF;
        }
    }
    switch (address) {
    case 0x804000: return Input(0);
    case 0x804010: return Input(1);
    case 0x804020: return Input(2);
    case 0x804030: return 0xE021; // the volume setting (full); no extra memory or network board
    case 0x8040B0:
    case 0x8040B2: return 0xFFFF; // (switches only development boards have)
    default: break;
    }
    Unmapped("read", address, cpu.pc);
    return 0xFFFF;
}

void Machine::WriteRegister(uint32_t address, uint16_t value, uint16_t mask) {
    if (m_cps1) {
        WriteCps1(address, value, mask);
        return;
    }
    auto combine = [&](uint16_t &reg) {
        reg = static_cast<uint16_t>((reg & ~mask) | (value & mask));
        return reg;
    };
    auto combineBytes = [&](uint8_t *p) {
        if (mask & 0xFF00) {
            p[0] = static_cast<uint8_t>(value >> 8);
        }
        if (mask & 0x00FF) {
            p[1] = static_cast<uint8_t>(value);
        }
    };
    if (address >= 0x400000 && address < 0x40000C) {
        combine(objectRegs[(address - 0x400000) / 2]);
        return;
    }
    if (address >= 0x618000 && address < 0x61A000) {
        if (mask & 0x00FF) {
            const uint32_t at = (address - 0x618000) / 2;
            soundRam[at] = static_cast<uint8_t>(value);
            if (extraSound && at >= 0x010) { // (the game's settings and its greeting: the other side's too; not requests)
                extraSound->Shared(at, static_cast<uint8_t>(value));
            }
            if (at == 0x00F && soundRam[0x00F] != 0xFF) { // (a request is there: soundtrack.h)
                soundtrack.Request(soundRam, frame);
            }
        }
        return;
    }
    if (address >= 0x660000 && address < 0x664000) {
        combineBytes(&extraRam[address - 0x660000]);
        return;
    }
    if (address >= 0x700000 && address < 0x702000) {
        combineBytes(&objectRam[m_objectBank * 0x2000 + (address & 0x1FFE)]);
        ObjectWritten(m_objectBank, address);
        return;
    }
    if (address >= 0x708000 && address < 0x710000) {
        combineBytes(&objectRam[(m_objectBank ^ 1) * 0x2000 + (address & 0x1FFE)]);
        ObjectWritten(m_objectBank ^ 1, address);
        return;
    }
    const uint32_t chip = address & ~0x4000u; // (the video chips answer at 8001xx as well as 8041xx)
    if (chip >= 0x800100 && chip < 0x800140) {
        const unsigned index = (address & 0x3E) / 2;
        combine(cpsA[index]);
        if (index == 0x0A / 2) {
            video.CopyPalette(); // (writing the palette's address makes the chip take the palette)
            m_palettePending = boardTiming; // (... by DMA, at the next line: LineDma)
        }
        return;
    }
    if (chip >= 0x800140 && chip < 0x800180) {
        const unsigned index = (address & 0x3E) / 2;
        const uint16_t now = combine(cpsB[index]);
        if (index == 0x0E / 2) {
            m_rasterReload[2] = (now >> 1) & 0xFF;
        } else if (index == 0x10 / 2 || index == 0x12 / 2) {
            const int n = index & 1;
            m_rasterReload[n] = now & 0x1FF;
            if (now & 0x8000) {
                m_rasterCounter[n] = m_rasterReload[n];
            }
        }
        return;
    }
    switch (address) {
    case 0x804040:
        if (mask & 0xFF00) {
            eeprom.SetLines(value & 0x4000, value & 0x2000, value & 0x1000);
        }
        if (mask & 0x00FF) {
            // Bit 3 lets the Z80 run; the rest are coin counters and lock-outs
            const bool run = value & 0x0008;
            if (run && !m_soundRunning) {
                soundCpu.Reset();
                if (soundDriver && soundSide != SoundSide::Z80) {
                    soundDriver->shadow = soundSide == SoundSide::Check;
                    soundDriver->Reset();
                }
            }
            m_soundRunning = run;
            if (extraSound) {
                extraSound->SetRunning(run);
            }
        }
        return;
    case 0x8040A0: return; // (written once at start-up)
    case 0x8040E0:
        if (mask & 0x00FF) {
            m_objectBank = value & 1;
        }
        return;
    case 0x664000: return;
    default: break;
    }
    Unmapped("write", address, cpu.pc);
}

// --- a CPS-1 with QSound (MAME's cps1.cpp has the map, jtcps1_main.v the inputs' wiring) ---

// 800000: players 1 (low byte) and 2 (high): the stick and three buttons each. 800018: the system's switches in the
// high byte (coins 1 and 2, the service button, Start 1 and 2, the test switch). F1C000, F1C002: players 3 and 4,
// with their coin and Start (in both bytes). F1C006: the settings memory's data out. All 0 while pressed.
uint16_t Machine::ReadCps1(uint32_t address) {
    const uint16_t p1 = players[0], p2 = players[1];
    auto stick = [](uint16_t p) { return static_cast<uint16_t>(p & 0x7F); };
    if (address >= 0x800000 && address < 0x800008) {
        return static_cast<uint16_t>(~(stick(p1) | (stick(p2) << 8)));
    }
    if (address == 0x800018) {
        const unsigned pressed = ((p1 & kCoin) ? 0x01 : 0) | ((p2 & kCoin) ? 0x02 : 0) | (serviceButton ? 0x04 : 0) | ((p1 & kStart) ? 0x10 : 0) |
                                 ((p2 & kStart) ? 0x20 : 0) | (testSwitch ? 0x40 : 0);
        return static_cast<uint16_t>(((~pressed & 0xFF) << 8) | 0xFF);
    }
    if (address >= 0x80001A && address < 0x800020) {
        return 0xFFFF; // (no DIP switches: the settings are in the settings memory)
    }
    if (address == 0xF1C000 || address == 0xF1C002) {
        const uint16_t p = players[address == 0xF1C000 ? 2 : 3];
        const unsigned pressed = (p & 0x3F) | ((p & kCoin) ? 0x40 : 0) | ((p & kStart) ? 0x80 : 0);
        const uint16_t byte = static_cast<uint16_t>(~pressed & 0xFF);
        return static_cast<uint16_t>((byte << 8) | byte);
    }
    if (address == 0xF1C006) {
        return static_cast<uint16_t>(0xFFFE | (eeprom.DataOut() ? 1 : 0));
    }
    if (address >= 0xF18000 && address < 0xF1A000) {
        return static_cast<uint16_t>(0xFF00 | soundRam[(address - 0xF18000) / 2]);
    }
    if (address >= 0xF1E000 && address < 0xF20000) {
        return static_cast<uint16_t>(0xFF00 | soundRam2[(address - 0xF1E000) / 2]);
    }
    if (address >= 0xF00000 && address < 0xF10000) {
        const size_t at = (address - 0xF00000) / 2;
        return static_cast<uint16_t>(0xFF00 | (at < m_roms.soundProgram.size() ? m_roms.soundProgram[at] : 0xFF));
    }
    if (address >= 0x800140 && address < 0x800180) {
        return 0xFFFF; // (this CPS-B has no multiplier or ID to read)
    }
    Unmapped("read", address, cpu.pc);
    return 0xFFFF;
}

void Machine::WriteCps1(uint32_t address, uint16_t value, uint16_t mask) {
    auto combine = [&](uint16_t &reg) {
        reg = static_cast<uint16_t>((reg & ~mask) | (value & mask));
    };
    if (address >= 0x800100 && address < 0x800140) {
        const unsigned index = (address & 0x3E) / 2;
        combine(cpsA[index]);
        if (index == 0x0A / 2) {
            video.CopyPalette(); // (writing the palette's address makes the chip take the palette)
        }
        return;
    }
    if (address >= 0x800140 && address < 0x800180) {
        combine(cpsB[(address & 0x3E) / 2]);
        return;
    }
    if (address >= 0xF18000 && address < 0xF1A000) {
        if (mask & 0x00FF) {
            const uint32_t at = (address - 0xF18000) / 2;
            soundRam[at] = static_cast<uint8_t>(value);
            if (at == 0x00F && soundRam[0x00F] != 0xFF) { // (a request is there: soundtrack.h)
                soundtrack.Request(soundRam, frame);
            }
        }
        return;
    }
    if (address >= 0xF1E000 && address < 0xF20000) {
        if (mask & 0x00FF) {
            soundRam2[(address - 0xF1E000) / 2] = static_cast<uint8_t>(value);
        }
        return;
    }
    if (address == 0xF1C006) {
        if (mask & 0x00FF) {
            eeprom.SetLines(value & 0x80, value & 0x40, value & 0x01); // (select, clock, data in)
        }
        return;
    }
    if ((address >= 0x800030 && address < 0x800038) || address == 0xF1C004 || (address >= 0x800180 && address < 0x800190)) {
        return; // (coin counters and lock-outs; the CPS-1's sound latches, which a QSound board does not use)
    }
    Unmapped("write", address, cpu.pc);
}

// --- the Z80's bus ---

uint8_t Machine::Read(uint16_t address) {
    if (address < 0x8000) {
        return m_roms.soundProgram[address];
    }
    if (address < 0xC000) {
        const size_t at = 0x8000 + static_cast<size_t>(m_soundBank) * 0x4000 + (address - 0x8000);
        return at < m_roms.soundProgram.size() ? m_roms.soundProgram[at] : 0xFF;
    }
    if (address < 0xD000) {
        if (address == 0xCFFE && monoForTest) {
            return 0;
        }
        return soundRam[address - 0xC000];
    }
    if (address == 0xD007) {
        return qsound.Read();
    }
    if (address >= 0xF000) {
        return soundRam2[address - 0xF000];
    }
    return 0xFF;
}

void Machine::Write(uint16_t address, uint8_t value) {
    if (address >= 0xC000 && address < 0xD000) {
        if (soundSide == SoundSide::Check && address == 0xC00F && value == 0xFF) {
            soundDriver->TakeCommand(); // (the sound program's interrupt has taken a command)
        }
        soundRam[address - 0xC000] = value;
    } else if (address >= 0xD000 && address <= 0xD002) {
        qsound.Write(address - 0xD000, value);
        if (soundSide == SoundSide::Check) {
            if (address == 0xD000) {
                m_chipLatch = static_cast<uint16_t>((m_chipLatch & 0x00FF) | (value << 8));
            } else if (address == 0xD001) {
                m_chipLatch = static_cast<uint16_t>((m_chipLatch & 0xFF00) | value);
            } else {
                m_z80Writes.push_back({value, m_chipLatch, soundCpu.pc, static_cast<uint32_t>(soundCpu.cycles - m_z80Mark)});
                m_z80Mark = soundCpu.cycles;
            }
        }
    } else if (address == 0xD003) {
        m_soundBank = value & 0x0F;
    } else if (address >= 0xF000) {
        if (soundSide == SoundSide::Check) {
            // The sound program counts down what its interrupt counted up as it starts on each piece of work;
            // for commands, what tells is its taking one from its queue (an interrupt can add one between
            // the count and the look at the queue)
            if ((address == 0xF002 || address == 0xF003) && value == static_cast<uint8_t>(soundRam2[address - 0xF000] - 1)) {
                m_z80Mark = soundCpu.cycles;
                soundDriver->Do(static_cast<SoundDriver::Work>(address - 0xF000));
            } else if (address == 0xF016) {
                m_z80Mark = soundCpu.cycles;
                soundDriver->Do(SoundDriver::Work::Commands);
            }
        }
        if (soundSide == SoundSide::Check && address == 0xF008) {
            soundDriver->SetStereo(value);
        }
        if (address == 0xF016 && getenv("CPS2RECOMP_SOUND_COMMANDS")) {
            fprintf(stderr, "z80   command");
            for (int n = 0; n < 13; n++) {
                fprintf(stderr, " %02X", soundRam2[0x100 + soundRam2[0x16] + n]);
            }
            fprintf(stderr, " (frame %llu)\n", static_cast<unsigned long long>(frame));
        }
        soundRam2[address - 0xF000] = value;
    }
}

void Machine::Serialize(Archive &ar) {
    ar.Mark(0x4D414348);
    cpu.Serialize(ar);
    soundCpu.Serialize(ar);
    if (soundDriver) {
        soundDriver->Serialize(ar);
    }
    qsound.Serialize(ar);
    video.Serialize(ar);
    eeprom.Serialize(ar);
    ar.Vec(ram);
    ar.Vec(videoRam);
    ar.Vec(objectRam);
    if (m_extra) {
        ar.Pod(objectsFromExtra);
        ar.Vec(scrollFromExtra);
    }
    ar.Vec(extraRam);
    ar.Pod(soundRam);
    ar.Pod(soundRam2);
    ar.Pod(cpsA);
    ar.Pod(cpsB);
    ar.Pod(objectRegs);
    ar.Pod(frame);
    ar.Pod(m_line);
    ar.Pod(m_lineStart);
    ar.Pod(m_blankInterrupt);
    ar.Pod(m_rasterInterrupt);
    ar.Pod(m_rasterCounter);
    ar.Pod(m_rasterReload);
    ar.Pod(m_objectBank);
    ar.Pod(m_soundRunning);
    ar.Pod(m_soundBank);
    ar.Pod(m_soundTime);
    ar.Pod(m_soundInterruptAt);
    ar.Pod(m_sampleCount);
    ar.Pod(m_sampleAt);
    ar.Pod(m_driverInterrupt);
    ar.Vec(m_chipQueue);
    ar.Pod(m_chipNext);
    ar.Pod(m_chipLast);
    if (extraSound) {
        extraSound->Serialize(ar);
    }
    if (ar.version == 0 || ar.version >= 6) { // (a save state from before soundtracks has none)
        soundtrack.Serialize(ar);
    } else if (!ar.Saving()) {
        soundtrack.Reset();
    }
    if (ar.version == 0 || ar.version >= 7) { // (the board's timing: a palette the DMA is yet to copy)
        ar.Pod(m_palettePending);
    } else if (!ar.Saving()) {
        m_palettePending = false;
    }
}

// (CPS2RECOMP_SOUND_GAPS, with --sound check) how long the Z80 took to come to each of its writes and how
// long the driver reckons, by the place in the sound program that writes
void Machine::ReportSoundTimes() const {
    if (!m_gapStats) {
        return;
    }
    uint64_t z80 = 0, driver = 0;
    for (const auto &[pc, stat] : *m_gapStats) {
        fprintf(stderr, "  write at %04X: %8llu times, Z80 %6llu cycles on average, driver %6llu\n", pc,
                static_cast<unsigned long long>(stat.count), static_cast<unsigned long long>(stat.z80 / stat.count),
                static_cast<unsigned long long>(stat.driver / stat.count));
        z80 += stat.z80, driver += stat.driver;
    }
    fprintf(stderr, "  in all: Z80 %llu cycles, driver %llu (%.1f%%)\n", static_cast<unsigned long long>(z80),
            static_cast<unsigned long long>(driver), 100.0 * static_cast<double>(driver) / static_cast<double>(std::max<uint64_t>(z80, 1)));
}

} // namespace cps2recomp
