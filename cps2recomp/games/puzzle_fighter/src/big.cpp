// SPDX-License-Identifier: GPL-3.0-only
// Copyright (C) 2026 Tommy McLeroy
//
// Super Puzzle Fighter II Turbo: big gems joining into bigger ones, written again as C++ (rewrites: lift.h).
// A6 is the player's record.
//
// A big gem is a rectangle of cells of one colour. Each of its cells has the mark kCellBig, and in the
// second field (kFieldExtra) a word: the gem's number in the high byte, and in the low byte a mark for the
// rectangle's corners (kBottomLeft ...). The corners' cells also have a code in their own high byte, which
// the picture is drawn from.
#include "spf2.h"

namespace cps2recomp {

namespace {
    using namespace spf2;

    constexpr int32_t kRight = 2, kLeft = -2, kUp = -16;
    enum : uint8_t { kBottomLeft = 1, kTopLeft = 2, kBottomRight = 4, kTopRight = 8 };
    constexpr uint8_t kTwoGems = 2 * (kBottomLeft | kTopLeft | kBottomRight | kTopRight), kThreeGems = 3 * 15;
    constexpr uint32_t kLastBigGem = 0x32C;  // byte: the number given to the big gem made last (never 0)
    // What the game keeps while it looks at one gem (left in the record afterwards)
    constexpr uint32_t kThisGem = 0x38A, kInLine = 0x38B, kOtherFound = 0x38F, kOtherGem = 0x3A5; // bytes
    constexpr uint32_t kLineCount = 0x34A, kLines = 0x34C;                                         // words

    uint8_t Number(uint32_t cell) {
        return Ram8(cell + kFieldExtra);
    }
    uint8_t Corner(uint32_t cell) {
        return Ram8(cell + kFieldExtra + 1);
    }

    // How one of the game's three routines goes about it. Each starts from one corner of a big gem, looks
    // along that gem's edge to the next corner, and then takes line after line of that length, going across,
    // for as long as every cell is of the gem's colour and belongs to it or to one other big gem that starts
    // squarely on the line. If what it took is exactly two whole gems (by their corners), they become one.
    struct Join {
        uint8_t from, to;       // the corner it starts from, and the one at the other end of the edge
        int32_t along, across;  // the steps along a line, and from a line to the next
        bool numbered;          // cells must have a big gem's number
        bool forgetOther;       // starts by forgetting the other gem's number
        bool anyOther;          // (the third routine's looser test of the other gem)
        bool three;             // three whole gems will do as well as two
        bool lastLineStays;     // the count of lines is not let go below one when the cells are renumbered
        bool edgeShort;         // the far corner along the edge is put one cell short (where it belongs)
        uint8_t corners[4];     // the corner marks: where it started, across, along, and both
        uint8_t codes[4];       // ... and the codes the picture is drawn from
    };

    void JoinFrom(uint32_t player, uint32_t start, const Join &j) {
        if (Corner(start) != j.from) {
            return;
        }
        SetRam8(player + kThisGem, Number(start));
        SetRam8(player + kInLine, 0);
        SetRam8(player + kOtherFound, 0);
        if (j.forgetOther) {
            SetRam8(player + kOtherGem, 0);
        }
        const uint16_t colour = Ram16(start) & kColour;
        uint16_t edge = 0; // cells from the starting corner to the other end of the edge
        for (uint32_t at = start;;) {
            at += j.along;
            edge++;
            if (Corner(at) == j.to) {
                break;
            }
        }
        SetRam16(player + kLineCount, 0);
        SetRam16(player + kLines, 0);
        uint8_t corners = 0;
        bool whole = false; // stopped at the start of a line (so what was taken is a rectangle)
        for (uint32_t line = start; !whole; line += j.across) {
            uint32_t cell = line;
            for (int n = 0; n <= edge; n++, cell += j.along) {
                const uint16_t value = Ram16(cell);
                bool fits = (value & kColour) != kCounter && (value & kCellBig) && (value & kColour) == colour &&
                            (!j.numbered || Number(cell) != 0);
                bool stop = false;
                if (fits && Number(cell) != Ram8(player + kThisGem)) {
                    if (j.anyOther) {
                        if (!Ram8(player + kInLine) && !Ram8(player + kOtherFound)) {
                            if (Corner(cell) != kBottomLeft) {
                                fits = false;
                            } else {
                                SetRam8(player + kOtherFound, 1);
                            }
                        }
                    } else if (Ram8(player + kOtherFound)) {
                        stop = Number(cell) != Ram8(player + kOtherGem); // a third gem: what was taken so far is it
                    } else if (Corner(cell) != kBottomLeft) {
                        fits = false;
                    } else {
                        SetRam8(player + kOtherFound, 1);
                        SetRam8(player + kOtherGem, Number(cell));
                    }
                }
                if (stop) {
                    whole = true;
                    break;
                }
                if (!fits) {
                    if (n != 0) {
                        return; // in the middle of a line: no rectangle
                    }
                    whole = true;
                    break;
                }
                corners = static_cast<uint8_t>(corners + Corner(cell));
                SetRam16(player + kLineCount, static_cast<uint16_t>(Ram16(player + kLineCount) + 1));
                SetRam8(player + kInLine, static_cast<uint8_t>(Ram8(player + kInLine) + 1));
            }
            if (whole) {
                break;
            }
            SetRam16(player + kLineCount, 0);
            SetRam16(player + kLines, static_cast<uint16_t>(Ram16(player + kLines) + 1));
            SetRam8(player + kInLine, 0);
        }
        if (corners != kTwoGems && !(j.three && corners == kThreeGems)) {
            return;
        }

        // One gem of it all: a new number for every cell, and the four corners marked
        uint8_t number = static_cast<uint8_t>(Ram8(player + kLastBigGem) + 1);
        if (number == 0) {
            number = 1;
        }
        SetRam8(player + kLastBigGem, number);
        SetRam16(player + kLineCount, edge);
        int lines = static_cast<int16_t>(Ram16(player + kLines));
        int renumbered = lines - 1;
        if (j.lastLineStays && renumbered < 0) {
            renumbered = 0;
        }
        // (the game counts down to nothing with a word: a count of nothing is one line all the same)
        for (int l = 0; l <= static_cast<uint16_t>(renumbered); l++) {
            for (int n = 0; n <= edge; n++) {
                const uint32_t cell = start + l * j.across + n * j.along;
                SetRam16(cell, Ram16(cell) & 0x00FF);
                SetRam16(cell + kFieldExtra, static_cast<uint16_t>(number << 8));
            }
        }
        SetRam16(player + kLines, static_cast<uint16_t>(lines - 2));
        const int farLine = static_cast<uint16_t>(lines - 2) + 1, farCell = j.edgeShort ? edge : edge + 1;
        const uint32_t places[4] = {start, start + farLine * j.across, start + farCell * j.along,
                                    start + farLine * j.across + farCell * j.along};
        for (int n = 0; n < 4; n++) {
            SetRam8(places[n], j.codes[n]);
            SetRam8(places[n] + kFieldExtra + 1, j.corners[n]);
        }
    }

    // 00F060. From each big gem's bottom left corner, along its bottom edge and upwards: joins it with a gem
    // of the same width standing on it.
    void JoinUpwards(M68K &c) {
        static const Join kJoin = {kBottomLeft, kBottomRight, kRight, kUp, false, true, false, false, false, true,
                                   {kBottomLeft, kTopLeft, kBottomRight, kTopRight}, {0x30, 0x60, 0x90, 0xC0}};
        const uint32_t player = c.a[6], bottom = Ram32(player + kFieldBottom) + kBottomRow + 2;
        for (int column = 0; column < kFieldColumns; column++) {
            for (int row = 0; row < kFieldRows - 1; row++) {
                JoinFrom(player, bottom + column * kRight + row * kUp, kJoin);
            }
        }
    }

    // 00EEB0. From each big gem's bottom right corner, leftwards along its bottom edge and upwards.
    void JoinUpwardsFromRight(M68K &c) {
        static const Join kJoin = {kBottomRight, kBottomLeft, kLeft, kUp, false, false, true, false, true, false,
                                   {kBottomRight, kTopRight, kBottomLeft, kTopLeft}, {0x30, 0x60, 0x90, 0xC0}};
        const uint32_t player = c.a[6], bottom = Ram32(player + kFieldBottom) + kBottomRow + 2;
        for (int column = kFieldColumns - 1; column >= 0; column--) {
            for (int row = 0; row < kFieldRows - 1; row++) {
                JoinFrom(player, bottom + column * kRight + row * kUp, kJoin);
            }
        }
    }

    // 00ECEC. From each big gem's bottom left corner, up its left edge and to the right: joins it with a
    // gem of the same height beside it. (The game goes through the field a row at a time here, thirteen
    // cells to a row of six and only the six lowest rows.)
    void JoinSideways(M68K &c) {
        static const Join kJoin = {kBottomLeft, kTopLeft, kUp, kRight, true, false, false, true, false, false,
                                   {kBottomLeft, kBottomRight, kTopLeft, kTopRight}, {0x30, 0x60, 0x90, 0xC0}};
        const uint32_t player = c.a[6], bottom = Ram32(player + kFieldBottom) + kBottomRow + 2;
        for (int row = 0; row < 6; row++) {
            for (int n = 0; n < 13; n++) {
                JoinFrom(player, bottom + row * kUp + n * kRight, kJoin);
            }
        }
    }

    // --- the big gems' shapes, for the picture ---
    //
    // Every cell of a big gem has a code in its own high byte for the piece of the gem's picture drawn
    // there, and the gem's four corners are marked in the second field (kBottomLeft ...). Both are worked
    // out again from the gems' numbers after every pass that makes or joins big gems.
    constexpr uint32_t kSecondField = 0x168;   // long: the second field's address, as kFieldBottom is the first's
    constexpr uint32_t kCornerMarks = 0x00FFD8; // table of words: the mark, by which neighbours are the same gem
    // (the time the game's code takes to go through the field, and over each gem and each cell of a big gem, about)
    constexpr uint32_t kFieldTime = 8700, kCellTime = 50, kBigCellTime = 1100;
    struct Beside {
        int32_t cell;
        uint8_t same, joined; // the bit for this side in the count of neighbours, and in a cell's code
    };
    constexpr Beside kBeside[4] = {{kUp, 1, 0x10}, {kRight, 2, 0x20}, {-kUp, 4, 0x40}, {kLeft, 8, 0x80}};
    // The codes: a bit for each side on which the gem goes on (kBeside's `joined`); but a cell in the top
    // or bottom edge that is not a corner has one of three, by where it is along the edge
    constexpr uint8_t kTopEdge = 0xE0, kBottomEdge = 0xB0, kTopLeftCode = 0x60, kBottomLeftCode = 0x30;
    struct EdgeCodes {
        uint8_t nearEnd, middle, last;
    };
    constexpr EdgeCodes kTopCodes = {0x20, 0x10, kTopEdge}, kBottomCodes = {0x50, 0x40, kBottomEdge};

    // 00DAFA. The code for a cell of a big gem.
    uint8_t ShapeCode(uint32_t cell) {
        const uint8_t number = Number(cell);
        uint8_t code = 0;
        for (const Beside &beside : kBeside) {
            const uint16_t kind = Ram16(cell + beside.cell) & (kKind | kCellBreaking);
            if (!(kind & kCellBreaking) && kind != kCounter && kind != kCounterFlash && Number(cell + beside.cell) == number) {
                code |= beside.joined;
            }
        }
        if (code != kTopEdge && code != kBottomEdge) {
            return code;
        }
        // (the cells to the left have their codes already: the field is gone through from the left)
        const EdgeCodes &codes = code == kTopEdge ? kTopCodes : kBottomCodes;
        const uint8_t corner = code == kTopEdge ? kTopLeftCode : kBottomLeftCode;
        const auto cornerIsLeft = [&](int cells) { return (Ram8(cell + cells * kLeft) & 0xF0) == corner; };
        const auto gemGoesOn = [&](int cells) { return Number(cell + cells * kRight) == number; };
        if (cornerIsLeft(4) || cornerIsLeft(3)) {
            return codes.nearEnd;
        }
        if (cornerIsLeft(2)) {
            return !gemGoesOn(2) ? codes.nearEnd : gemGoesOn(3) ? codes.middle : codes.last;
        }
        return gemGoesOn(2) ? codes.middle : codes.last;
    }

    // 00D9AE. Gives the cells of the big gems their codes and marks their corners (not while a break is
    // under way). A cell that has the big gem mark and no gem's number loses its code.
    void MakePictureCodes(M68K &c) {
        const uint32_t player = c.a[6];
        if (Ram8(player + kBreaking)) {
            return;
        }
        Spend(c, kFieldTime);
        const uint32_t toSecond = Ram32(player + kSecondField) - Ram32(player + kFieldBottom);
        for (int column = 0; column < kFieldColumns; column++) {
            for (int row = 0; row < kFieldRows - 1; row++) {
                const uint32_t cell = Cell(player, column, row), mark = cell + toSecond;
                if (Ram16(cell) == kWall) {
                    continue;
                }
                if (Ram8(cell + 1) == 0) {
                    SetRam16(cell, 0);
                    SetRam16(cell + kFieldExtra, 0);
                    continue;
                }
                SetRam16(mark, Ram16(mark) & 0xFF00);
                Spend(c, kCellTime);
                if ((Ram16(cell) & kKind) == kCounter) {
                    continue;
                }
                const uint8_t number = Number(cell);
                if (number == 0) {
                    SetRam8(cell, 0);
                    SetRam8(cell + kFieldExtra, 0);
                    continue;
                }
                SetRam8(cell, ShapeCode(cell));
                Spend(c, kBigCellTime);
                // A corner has exactly two neighbours that are the same gem in the same state
                uint8_t same = 0;
                for (const Beside &beside : kBeside) {
                    const uint16_t other = Ram16(cell + beside.cell);
                    if ((other & kKind) != kCounter && (other & 0xFF) == Ram8(cell + 1) && Number(cell + beside.cell) == number) {
                        same |= beside.same;
                    }
                }
                SetRam16(mark, static_cast<uint16_t>(Ram16(mark) + Mem16(c, kCornerMarks + 2u * same)));
            }
        }
    }
    void ShapeCodeRoutine(M68K &c) {
        SetRam8(c.a[1], ShapeCode(c.a[1]));
    }

    // --- big gems being made, and growing ---
    //
    // The game looks from a cell upwards and to one side for the biggest rectangle of gems of the cell's
    // colour that it can make into one big gem: plain gems, and at most one big gem already there (the
    // rectangle then has to start at that gem's corner and cover it exactly). It does this three times a
    // turn, each time from a different corner and with slightly different rules (MakeFrom's `Make`).
    constexpr uint32_t kFieldTop = 0x398;   // long: nothing at or above this address is part of the field
    constexpr uint32_t kMadeNew = 0x267;    // byte: a plain gem became part of a big gem in this pass
    constexpr uint32_t kGemTime = 585; // (the time the game's code takes over each gem it starts from, about)
    constexpr int kStartRows = 11, kMostRows = 11, kMostColumns = 6, kColumnsLooked = 7;
    // Where the game keeps its working while it does this: nothing else reads it, so the rewrite keeps its
    // own in variables and the check leaves these out
    constexpr uint32_t kWorking[][2] = {{0x212, 0x23E}, {0x1E6, 0x1E8}, {0x386, 0x388}};

    struct Make {
        int32_t side;        // towards the columns it widens into
        int startColumn;     // the column it starts from, going the same way
        uint8_t corner;      // the corner of a big gem that the rectangle may start at
        bool nothingIsFine;  // a cell of no big gem passes where the gem's number is compared
        uint8_t topCorner;   // a column that starts on this corner of the gem already found ends the widening (0: no such rule)
        bool checkAbove;     // the cells above the first row are held to the gem's number as well
        uint8_t made[4];     // the corner marks given: the starting cell, the far column's bottom and top, the first column's top
    };

    void MakeFrom(M68K &c, uint32_t player, uint32_t start, const Make &m) {
        const uint16_t first = Ram16(start);
        const uint16_t colour = first & kKind;
        if (first == 0 || first == kWall || colour == kCounterFlash || colour == kCounter) {
            return;
        }
        Spend(c, kGemTime);
        const int32_t top = static_cast<int32_t>(Ram32(player + kFieldTop));
        uint8_t gem = 0;      // the big gem the rectangle is being built on, once one is found
        bool onGem = false;
        if (Number(start)) {
            if (Corner(start) != m.corner) {
                return;
            }
            onGem = true;
            gem = Number(start);
        }
        auto other = [&](uint8_t number) { return m.nothingIsFine ? number != 0 && number != gem : number != gem; };

        // Upwards from the starting cell: how tall the rectangle can be
        int rowsAbove = 0;
        for (; rowsAbove < kMostRows; rowsAbove++) {
            const uint32_t cell = start + (rowsAbove + 1) * kUp;
            if ((Ram16(cell + kFieldExtra) & 0x0F) == m.corner) {
                if (gem != 0 && other(Number(cell))) {
                    break;
                }
                onGem = true;
                gem = Number(cell);
            }
            const uint16_t value = Ram16(cell);
            if ((value & kCellBig) && (!onGem || other(Number(cell)))) {
                break;
            }
            if ((value & kKind) == 0 || (value & kKind) == kCounter || (value & kKind) != colour || static_cast<int32_t>(cell) <= top) {
                break;
            }
        }
        if (rowsAbove == 0) {
            return;
        }

        // Sideways: column after column, each as far up as the rectangle goes (and a cell further, which may
        // fail); a column that stops short makes the rectangle that much lower
        int columnsBeside = 0;
        for (; columnsBeside < kMostColumns; columnsBeside++) {
            const uint32_t foot = start + (columnsBeside + 1) * m.side;
            const uint8_t number = Number(foot);
            if (number) {
                if (Corner(foot) == m.corner) {
                    if (gem != 0 && number != gem) {
                        break;
                    }
                    onGem = true;
                    gem = number;
                }
                if (!onGem || number != gem) {
                    break;
                }
            }
            const uint16_t kind = Ram16(foot) & kKind;
            if (kind == kCounter || (m.topCorner && Corner(foot) == m.topCorner && onGem) || kind != colour) {
                break;
            }
            int taken = 0;
            bool stopped = false;
            while (taken <= rowsAbove) {
                const uint32_t cell = foot + (taken + 1) * kUp;
                const uint16_t above = Ram16(cell) & kKind;
                if ((Number(foot) && (!onGem || Number(foot) != gem)) || above == kCounter || above != colour ||
                    (m.checkAbove && Number(cell) != 0 && Number(cell) != gem) || static_cast<int32_t>(cell) <= top) {
                    stopped = true;
                    break;
                }
                if (Corner(cell) == m.corner && (gem == 0 || Number(foot) == gem)) {
                    onGem = true;
                    gem = Number(cell);
                }
                taken++;
            }
            if (stopped && taken == 0) {
                break;
            }
            if (stopped && taken < rowsAbove) {
                rowsAbove = taken;
            }
        }
        if (columnsBeside == 0) {
            return;
        }
        auto at = [&](int column, int row) { return start + column * m.side + row * kUp; };

        // With a big gem in it the rectangle has to be that gem's corners exactly (or two gems')
        if (onGem) {
            uint16_t corners = 0;
            for (int column = 0; column <= columnsBeside; column++) {
                for (int row = 0; row <= rowsAbove; row++) {
                    corners = static_cast<uint16_t>(corners + Corner(at(column, row)));
                }
            }
            if (corners != 0 && (corners & 0xFF) != 15 && (corners & 0xFF) != kTwoGems) {
                return;
            }
        }

        // The rectangle becomes one big gem
        uint8_t number = static_cast<uint8_t>(Ram8(player + kLastBigGem) + 1);
        if (number == 0) {
            number = 1;
        }
        SetRam8(player + kLastBigGem, number);
        uint8_t count = (Ram8(player + kMadeCount) + 1) & 7;
        SetRam8(player + kMadeCount, count == 7 || count == 0 ? 1 : count);
        for (int column = 0; column <= columnsBeside; column++) {
            for (int row = 0; row <= rowsAbove; row++) {
                const uint32_t cell = at(column, row);
                if (!(Ram16(cell) & kCellBig)) {
                    SetRam8(player + kMadeNew, 1);
                }
                SetRam16(cell, Ram16(cell) | kCellBig);
                SetRam16(cell + kFieldExtra, static_cast<uint16_t>(number << 8));
            }
        }
        SetRam16(start, Ram16(start) | kCellBig); // (a wall's column apart, the loop has done this already)
        SetRam8(at(columnsBeside, 0) + kFieldExtra + 1, m.made[1]);
        SetRam8(at(0, rowsAbove) + kFieldExtra + 1, m.made[3]);
        SetRam8(at(columnsBeside, rowsAbove) + kFieldExtra + 1, m.made[2]);
        SetRam8(player + kNoteChanged, 1);
        SetRam8(start + kFieldExtra, number);
        SetRam8(start + kFieldExtra + 1, m.made[0]);
    }

    void MakeBigGems(M68K &c, const Make &m) {
        const uint32_t player = c.a[6];
        SetRam8(player + kMadeNew, 0);
        const uint32_t bottom = Ram32(player + kFieldBottom) + kBottomRow + 2 + 2 * m.startColumn;
        // (seven columns: the last is the wall, where nothing starts)
        for (int column = 0; column < kColumnsLooked; column++) {
            for (int row = 0; row < kStartRows; row++) {
                MakeFrom(c, player, bottom + column * m.side + row * kUp, m);
            }
        }
        if (Ram8(player + kMadeNew)) {
            SetRam8(player + kRedrawBigGems, 1);
        }
    }

    // 00DCD6. From the bottom left: upwards and to the right.
    void MakeBigGemsRightwards(M68K &c) {
        static const Make kMake = {kRight, 0, kBottomLeft, true, kTopLeft, true, {kBottomLeft, kBottomRight, kTopRight, kTopLeft}};
        MakeBigGems(c, kMake);
    }
    // 00E8BC. From the bottom right: upwards and to the left, with fewer questions asked.
    void MakeBigGemsLeftwards(M68K &c) {
        static const Make kMake = {kLeft, 5, kBottomRight, false, 0, false, {kBottomRight, kBottomLeft, kTopLeft, kTopRight}};
        MakeBigGems(c, kMake);
    }
    // 00E0EE. From the bottom right again, with the first pass's rules. (The starting cell is marked as the
    // top right corner here, not the bottom right: so the game has it.)
    void MakeBigGemsLeftwardsStrictly(M68K &c) {
        static const Make kMake = {kLeft, 5, kBottomRight, true, kTopRight, true, {kTopRight, kBottomLeft, kTopLeft, kTopRight}};
        MakeBigGems(c, kMake);
    }
} // namespace

void RegisterBig() {
    for (int n = 0; n < 2; n++) {
        for (const auto &range : kWorking) {
            AddScratchMemory(Player(n) + range[0], Player(n) + range[1]);
        }
    }
    // (the last number is the time a call is charged: the average the check measured for the game's own code)
    constexpr uint32_t kScratch = kFreeD0 | kFreeD1 | kFreeD2 | kFreeD3 | kFreeD4 | kFreeD5 | kFreeD6 | kFreeA0 | kFreeA1 | kFreeA2 |
                                  kFreeA3 | kFreeA4 | kFreeFlags;
    AddRewrite(0x00D9AE, "MakePictureCodes", MakePictureCodes, kScratch, 0);
    AddRewrite(0x00DAFA, "ShapeCode", ShapeCodeRoutine, kFreeD1 | kFreeD4 | kFreeD5 | kFreeD6 | kFreeFlags, 390);
    AddRewrite(0x00F060, "JoinUpwards", JoinUpwards, kScratch, 4700);
    AddRewrite(0x00EEB0, "JoinUpwardsFromRight", JoinUpwardsFromRight, kScratch, 4700);
    AddRewrite(0x00ECEC, "JoinSideways", JoinSideways, kScratch, 5800);
    AddRewrite(0x00DCD6, "MakeBigGemsRightwards", MakeBigGemsRightwards, kScratch | kFreeD7, 9600);
    AddRewrite(0x00E8BC, "MakeBigGemsLeftwards", MakeBigGemsLeftwards, kScratch | kFreeD7, 9600);
    AddRewrite(0x00E0EE, "MakeBigGemsLeftwardsStrictly", MakeBigGemsLeftwardsStrictly, kScratch | kFreeD7, 9600);
}

} // namespace cps2recomp
