// SPDX-License-Identifier: GPL-3.0-only
// Copyright (C) 2026 Tommy McLeroy
//
// Vampire Savior 2 as one of the games made from Vampire Savior's program, whose sources it builds
// (games/vampire_savior/src; places.h there): where the places those sources name are in this game.
#pragma once

#include <array>
#include <cstdint>

#include "game.h"

namespace vsav::family {

struct Place {
    uint32_t vsav, here;
};
inline constexpr Place kPlaceList[] = {
    // places: begin (tools/places.py; a line ending "by hand" is kept)
    {0x00116E, 0x00116E}, // an instruction, 4 on alike
    {0x001576, 0x001576}, // an instruction, 24 on alike
    {0x00214C, 0x00214C}, // an instruction, 17 on alike
    {0x002214, 0x002214}, // an instruction, 24 on alike
    {0x0036A4, 0x0036BE}, // an instruction, 6 on alike
    {0x008F56, 0x007724}, // an instruction, 24 on alike
    {0x0097DC, 0x007FEC}, // an instruction, 7 on alike
    {0x009880, 0x008090}, // an instruction, 15 on alike
    {0x00995A, 0x00816A}, // an instruction, 12 on alike
    {0x0099A8, 0x0081D0}, // an instruction, 24 on alike
    {0x00A3B2, 0x008BC6}, // an instruction, 8 on alike
    {0x00A3EA, 0x008C00}, // an instruction, 6 on alike
    {0x00A4BA, 0x008CD2}, // the same bytes, once only
    {0x00A60A, 0x008E2E}, // an instruction, 24 on alike
    {0x00AE7C, 0x0096BC}, // an instruction, 10 on alike
    {0x00D560, 0x00BBAC}, // an instruction, 24 on alike
    {0x014E8A, 0x01357E}, // an instruction, 9 on alike
    {0x015030, 0x013724}, // an instruction, 10 on alike
    {0x015036, 0x01372A}, // an instruction, 8 on alike
    {0x015084, 0x013778}, // an instruction, 7 on alike
    {0x01520A, 0x0138FE}, // an instruction, 9 on alike
    {0x015466, 0x013B5A}, // an instruction, 9 on alike
    {0x01549A, 0x013B8E}, // an instruction, 14 on alike
    {0x0154CC, 0x013BC0}, // an instruction, 10 on alike
    {0x0154FE, 0x013BF2}, // an instruction, 10 on alike
    {0x01551A, 0x013C0E}, // an instruction, 19 on alike
    {0x015546, 0x013C3A}, // an instruction, 5 on alike
    {0x01555C, 0x013C50}, // the same entry of a jump table, 1 on alike
    {0x015572, 0x013C66}, // an instruction, 14 on alike
    {0x01559E, 0x013C92}, // an instruction, 4 on alike
    {0x0155BA, 0x013CAE}, // the same entry of a jump table, 5 on alike
    {0x0155FC, 0x013CF0}, // the same entry of a jump table, 3 on alike
    {0x01560C, 0x013D00}, // the same entry of a jump table, 3 on alike
    {0x015696, 0x013D9C}, // an instruction, 4 on alike
    {0x015698, 0x013D9E}, // the same entry of a jump table, 3 on alike
    {0x016FD0, 0x015718}, // the same entry of a jump table, 7 on alike
    {0x016FFC, 0x015744}, // the same entry of a jump table, 7 on alike
    {0x0170A6, 0x0157EE}, // an instruction, 12 on alike
    {0x017F0C, 0x016784}, // an instruction, 6 on alike
    {0x017F22, 0x01679A}, // an instruction, 24 on alike
    {0x017FF4, 0x01686C}, // an instruction, 20 on alike
    {0x018054, 0x0168CC}, // an instruction, 24 on alike
    {0x018230, 0x016AEC}, // HitLands itself (the hit test calls a test of the guarding pose first, which goes on into it: kHitTestHit) by hand
    {0x01841C, 0x016CE8}, // an instruction, 19 on alike
    {0x01884E, 0x017178}, // an instruction, 9 on alike
    {0x0188BA, 0x0171FA}, // an instruction, 24 on alike
    {0x018952, 0x0172B0}, // where the code calls or jumps to it (3/3), not its first instruction's place
    {0x018968, 0x0172DA}, // an instruction, 9 on alike
    {0x018980, 0x0172F2}, // an instruction, 16 on alike
    {0x0189BA, 0x01732C}, // an instruction, 24 on alike
    {0x018AB0, 0x017422}, // an instruction, 3 on alike
    {0x018E8A, 0x017850}, // an instruction, 7 on alike
    {0x019030, 0x017A2A}, // an instruction, 8 on alike
    {0x01904E, 0x017A48}, // an instruction, 24 on alike
    {0x019172, 0x017B74}, // an instruction, 24 on alike
    {0x01923C, 0x017C3E}, // an instruction, 13 on alike
    {0x01926A, 0x017C6C}, // an instruction, 24 on alike
    {0x0193B8, 0x017DBA}, // an instruction, 24 on alike
    {0x01A728, 0x019132}, // an instruction, 24 on alike
    {0x01BBC2, 0x01A62E}, // an instruction, 24 on alike
    {0x01BC3A, 0x01A6A6}, // an instruction, 24 on alike
    {0x01BC88, 0x01A6F4}, // an instruction, 7 on alike
    {0x01C3A4, 0x01AE14}, // an instruction, 17 on alike
    {0x01DE40, 0x01C9B2}, // an instruction, 6 on alike
    {0x01DE56, 0x01C9C8}, // an instruction, 8 on alike
    {0x01DE6E, 0x01C9E0}, // an instruction, 8 on alike
    {0x01DE86, 0x01C9F8}, // an instruction, 8 on alike
    {0x01DE9E, 0x01CA10}, // an instruction, 5 on alike
    {0x01DEB0, 0x01CA22}, // an instruction, 8 on alike
    {0x01DECE, 0x01CA40}, // an instruction, 24 on alike
    {0x01DEEA, 0x01CA5C}, // an instruction, 18 on alike
    {0x01DF3A, 0x01CAAC}, // an instruction, 2 on alike
    {0x01DF40, 0x01CAB2}, // an instruction, 5 on alike
    {0x01DF50, 0x01CAC2}, // an instruction, 7 on alike
    {0x01DF64, 0x01CAD6}, // an instruction, 21 on alike
    {0x01DF70, 0x01CAE2}, // an instruction, 18 on alike
    {0x01DFC0, 0x01CB32}, // an instruction, 12 on alike
    {0x01E36E, 0x01CEE2}, // an instruction, 17 on alike
    {0x01E3F2, 0x01CF66}, // an instruction, 10 on alike
    {0x01E450, 0x01CFC4}, // an instruction, 18 on alike
    {0x01E4A0, 0x01D014}, // an instruction, 18 on alike
    {0x01E4F0, 0x01D064}, // an instruction, 12 on alike
    {0x01E75E, 0x01D2D4}, // an instruction, 9 on alike
    {0x01E77A, 0x01D2F0}, // an instruction, 8 on alike
    {0x01E792, 0x01D308}, // an instruction, 8 on alike
    {0x01E7AA, 0x01D320}, // an instruction, 5 on alike
    {0x01E7BC, 0x01D332}, // an instruction, 8 on alike
    {0x01E7DA, 0x01D350}, // an instruction, 24 on alike
    {0x01E7F6, 0x01D36C}, // an instruction, 18 on alike
    {0x01E846, 0x01D3BC}, // an instruction, 5 on alike
    {0x01E856, 0x01D3CC}, // an instruction, 7 on alike
    {0x01E86A, 0x01D3E0}, // an instruction, 21 on alike
    {0x01E876, 0x01D3EC}, // an instruction, 18 on alike
    {0x01E8C6, 0x01D43C}, // an instruction, 12 on alike
    {0x01E8F4, 0x01D46A}, // an instruction, 11 on alike
    {0x01E962, 0x01D4D8}, // an instruction, 7 on alike
    {0x01EE22, 0x01D9A4}, // an instruction, 19 on alike
    {0x01EE58, 0x01D9DA}, // an instruction, 16 on alike
    {0x01EEFA, 0x01DA7C}, // an instruction, 4 on alike
    {0x01EF16, 0x01DA98}, // an instruction, 24 on alike
    {0x01EFA0, 0x01DB22}, // an instruction, 24 on alike
    {0x01F280, 0x01DE0A}, // an instruction, 19 on alike
    {0x01F2B6, 0x01DE40}, // an instruction, 17 on alike
    {0x01F31E, 0x01DEA8}, // an instruction, 4 on alike
    {0x01F33A, 0x01DEC4}, // an instruction, 24 on alike
    {0x01F3C4, 0x01DF4E}, // an instruction, 24 on alike
    {0x01F64E, 0x01E1A0}, // an instruction, 18 on alike
    {0x01F682, 0x01E1D4}, // an instruction, 15 on alike
    {0x01F71C, 0x01E26E}, // an instruction, 4 on alike
    {0x01F73A, 0x01E28C}, // an instruction, 24 on alike
    {0x01F7C2, 0x01E314}, // an instruction, 24 on alike
    {0x01FDFE, 0x01E99E}, // an instruction, 11 on alike
    {0x01FE88, 0x01EA28}, // a place the code names so (2/2)
    {0x01FEA0, 0x01EA42}, // moved as the code before or after it, 24 on alike
    {0x01FF1A, 0x01EABC}, // an instruction, 24 on alike
    {0x01FF76, 0x01EB18}, // an instruction, 24 on alike
    {0x01FFCE, 0x01EB70}, // an instruction, 24 on alike
    {0x02002E, 0x01EBD0}, // an instruction, 24 on alike
    {0x0200A2, 0x01EC44}, // an instruction, 24 on alike
    {0x020108, 0x01ECAA}, // an instruction, 24 on alike
    {0x02016A, 0x01ED0C}, // an instruction, 24 on alike
    {0x0201CC, 0x01ED6E}, // an instruction, 24 on alike
    {0x020236, 0x01EDD8}, // an instruction, 24 on alike
    {0x020B38, 0x000000}, // ChooseColours: this game's chooses other hidden characters (no rewrite) by hand
    {0x020C98, 0x000000}, // CountShadowCode: this game's is not the same (no rewrite) by hand
    {0x02244C, 0x020E02}, // an instruction, 4 on alike
    {0x027694, 0x02686A}, // an instruction, 24 on alike
    {0x0276A6, 0x02687C}, // an instruction, 24 on alike
    {0x027EC0, 0x027114}, // the reelings' animation set (battle.cpp), found by its instructions: the second of three tables, then 0x382(a6) by hand
    {0x027F70, 0x0271C4}, // the animation on a step (battle.cpp), found by its instructions: subq.b #1,0x20(a6) and the frame's flags by hand
    {0x028192, 0x0273E6}, // an instruction, 24 on alike
    {0x02824C, 0x0274A8}, // an instruction, 24 on alike
    {0x07D240, 0x08B97A}, // an instruction, 11 on alike
    {0x07D354, 0x08BA8E}, // the same entry of a jump table, 3 on alike
    {0x07D372, 0x08BAAC}, // an instruction, 5 on alike
    {0x07D3A2, 0x08BADC}, // script 1, from the table of scripts after AllPieces (07D26C) by hand
    {0x07D3F6, 0x08BB2E}, // script 2, from the table of scripts after AllPieces (07D26C) by hand
    {0x07D4BA, 0x08BBEE}, // the same entry of a jump table, 3 on alike
    {0x07D522, 0x08BC54}, // a place the code names so (1/1)
    {0x07D534, 0x08BC66}, // the same entry of a jump table, 3 on alike
    {0x07D59C, 0x08BCCC}, // the same entry of a jump table, 3 on alike
    {0x07D5B0, 0x08BCE0}, // the same entry of a jump table, 12 on alike
    {0x07D600, 0x08BD2E}, // the same entry of a jump table, 5 on alike
    {0x07D61E, 0x08BD4C}, // a place the code names so (2/2)
    {0x07D62A, 0x08BD58}, // an instruction, 10 on alike
    {0x07D656, 0x08BD84}, // an instruction, 11 on alike
    {0x07D68A, 0x08BDB8}, // an instruction, 13 on alike
    {0x07D6D4, 0x08BE02}, // an instruction, 5 on alike
    {0x07D6E6, 0x08BE14}, // an instruction, 6 on alike
    {0x07D6FE, 0x08BE2C}, // inside code that moved alike, 4 on alike
    {0x07D70A, 0x08BE38}, // the same entry of a jump table, 1 on alike
    {0x07D710, 0x08BED6}, // the same entry of a jump table, 3 on alike
    {0x07D724, 0x08BEEA}, // the same entry of a jump table, 24 on alike
    {0x07D7C6, 0x08BF8C}, // the same entry of a jump table, 21 on alike
    {0x07D820, 0x08BFE6}, // the same entry of a jump table, 1 on alike
    {0x07D87C, 0x08C042}, // a place the code names so (1/1)
    {0x07D8C4, 0x08C08A}, // a place the code names so (1/1)
    {0x07D8D4, 0x08C09A}, // a place the code names so (1/1)
    {0x07D8DC, 0x08C0A2}, // a place the code names so (1/1)
    {0x07DDD8, 0x08C59E}, // the same entry of a jump table, 3 on alike
    {0x07DDEC, 0x08C5B2}, // the same entry of a jump table, 7 on alike
    {0x07DE0A, 0x08C5D0}, // the same entry of a jump table, 9 on alike
    {0x07DE2C, 0x08C5F2}, // the same entry of a jump table, 1 on alike
    {0x07DE32, 0x08BE3E}, // the same entry of a jump table, 3 on alike
    {0x07DE46, 0x08BE52}, // the same entry of a jump table, 3 on alike
    {0x07DE78, 0x08BE80}, // the same entry of a jump table, 9 on alike
    {0x07DEC8, 0x08BED0}, // the same entry of a jump table, 1 on alike
    {0x07DECE, 0x08C5F8}, // the same entry of a jump table, 3 on alike
    {0x07DEE2, 0x08C60C}, // the same entry of a jump table, 3 on alike
    {0x07DEEE, 0x08C618}, // the same entry of a jump table, 11 on alike
    {0x07DF18, 0x08C642}, // the same entry of a jump table, 1 on alike
    {0x07DF1E, 0x08C648}, // the same entry of a jump table, 3 on alike
    {0x07DF32, 0x08C65C}, // the same entry of a jump table, 6 on alike
    {0x07DF4C, 0x08C676}, // the same entry of a jump table, 12 on alike
    {0x07DF96, 0x08C6C0}, // the same entry of a jump table, 1 on alike
    {0x07DF9C, 0x08C6C6}, // the same entry of a jump table, 3 on alike
    {0x07DFB0, 0x08C6DA}, // the same entry of a jump table, 24 on alike
    {0x07E040, 0x08C76A}, // the same entry of a jump table, 5 on alike
    {0x07E058, 0x08C782}, // a place the code names so (2/2)
    {0x07E05E, 0x08C788}, // an instruction, 12 on alike
    {0x07E0A2, 0x08C7CC}, // an instruction, 5 on alike
    {0x07E0BA, 0x08C7E4}, // an instruction, 7 on alike
    {0x07E108, 0x08C832}, // the same entry of a jump table, 1 on alike
    {0x07E10E, 0x08C838}, // the same entry of a jump table, 3 on alike
    {0x07E122, 0x08C84C}, // the same entry of a jump table, 3 on alike
    {0x07E12E, 0x08C858}, // the same entry of a jump table, 12 on alike
    {0x07E15C, 0x08C886}, // the same entry of a jump table, 1 on alike
    {0x07E162, 0x08C88C}, // the same entry of a jump table, 3 on alike
    {0x07E176, 0x08C8A0}, // the same entry of a jump table, 17 on alike
    {0x07E1D6, 0x08C900}, // the same entry of a jump table, 12 on alike
    {0x07E208, 0x08C932}, // the same entry of a jump table, 1 on alike
    {0x07E20E, 0x08C938}, // the same entry of a jump table, 3 on alike
    {0x07E222, 0x08C94C}, // the same entry of a jump table, 24 on alike
    {0x07E2A8, 0x08C9D2}, // the same entry of a jump table, 19 on alike
    {0x07E2F6, 0x08CA20}, // the same entry of a jump table, 1 on alike
    {0x07E360, 0x08CA8A}, // a place the code names so (1/1)
    {0x07E3E0, 0x08CB0A}, // a place the code names so (1/1)
    {0x07E4E0, 0x08CC0A}, // a place the code names so (1/1)
    {0x07E5A8, 0x08CCD2}, // the same entry of a jump table, 3 on alike
    {0x07E5BC, 0x08CCE6}, // the same entry of a jump table, 7 on alike
    {0x07E5D6, 0x08CD00}, // the same entry of a jump table, 12 on alike
    {0x07E604, 0x08CD2E}, // the same entry of a jump table, 1 on alike
    {0x07E62C, 0x08CD56}, // the same entry of a jump table, 3 on alike
    {0x07E640, 0x08CD6A}, // the same entry of a jump table, 10 on alike
    {0x07E672, 0x08CD9C}, // an instruction, 13 on alike
    {0x07E6A4, 0x08CDCE}, // the same entry of a jump table, 3 on alike
    {0x07E6BC, 0x08CDE6}, // a place the code names so (2/2)
    {0x07E6C0, 0x08CDEA}, // an instruction, 10 on alike
    {0x07E746, 0x08CE70}, // an instruction, 14 on alike
    {0x07E778, 0x08CEA2}, // the same entry of a jump table, 1 on alike
    {0x07E77E, 0x08CEA8}, // an instruction, 14 on alike
    {0x07E7B4, 0x08CEDE}, // the same bytes, once only
    {0x07E832, 0x08CF5C}, // the same bytes, once only
    {0x07E8B0, 0x08CFDA}, // the same bytes, once only
    {0x07E9A8, 0x08D0D2}, // the same bytes, once only
    {0x07EA82, 0x08D1AC}, // the same entry of a jump table, 3 on alike
    {0x07EA96, 0x08D1C0}, // the same entry of a jump table, 12 on alike
    {0x07EAD8, 0x08D202}, // the same entry of a jump table, 3 on alike
    {0x07EAF0, 0x08D21A}, // a place the code names so (2/2)
    {0x07EAF6, 0x08D220}, // an instruction, 5 on alike
    {0x07EB0A, 0x08D234}, // an instruction, 9 on alike
    {0x07EB36, 0x08D260}, // an instruction, 4 on alike
    {0x07EB42, 0x08D26C}, // the same entry of a jump table, 1 on alike
    {0x07EB48, 0x08D272}, // the same bytes, once only
    {0x07EB88, 0x08D2B2}, // the same bytes, once only
    {0x07EBAA, 0x08D2D4}, // the same bytes, once only
    {0x07EBEA, 0x08D314}, // the same entry of a jump table, 3 on alike
    {0x07EBFE, 0x08D328}, // the same entry of a jump table, 3 on alike
    {0x07EC0A, 0x08D334}, // the same entry of a jump table, 22 on alike
    {0x07EC6E, 0x08D398}, // the same entry of a jump table, 24 on alike
    {0x07ED14, 0x08D43E}, // a place the code names so (1/1)
    {0x07ED16, 0x08D440}, // a place the code names so (1/1)
    {0x07ED2C, 0x08D456}, // the same entry of a jump table, 1 on alike
    {0x07ED32, 0x08D45C}, // the same bytes, once only
    {0x07ED42, 0x08D46C}, // a place the code names so (1/1)
    {0x07ED44, 0x08D46E}, // the same bytes, once only
    {0x07ED52, 0x08D47C}, // the same bytes, once only
    {0x07ED64, 0x08D48E}, // a table between two that moved alike (other bytes)
    {0x07ED72, 0x08D49C}, // the same entry of a jump table, 3 on alike
    {0x07ED86, 0x08D4B0}, // the same entry of a jump table, 12 on alike
    {0x07EDBA, 0x08D4E4}, // the same entry of a jump table, 6 on alike
    {0x07EDD8, 0x08D502}, // the same entry of a jump table, 1 on alike
    {0x07EDDE, 0x08D508}, // the same entry of a jump table, 3 on alike
    {0x07EDF2, 0x08D51C}, // the same entry of a jump table, 23 on alike
    {0x07EE54, 0x08D57E}, // a place the code names so (1/1)
    {0x07EE56, 0x08D580}, // a place the code names so (1/1)
    {0x07EE64, 0x08D58E}, // the same entry of a jump table, 9 on alike
    {0x07EE8A, 0x08D5B4}, // the same entry of a jump table, 1 on alike
    {0x07EE90, 0x08D5BA}, // the same entry of a jump table, 3 on alike
    {0x07EEA4, 0x08D5CE}, // the same entry of a jump table, 4 on alike
    {0x07EEB6, 0x08D5E0}, // the same entry of a jump table, 12 on alike
    {0x07EEE6, 0x08D610}, // the same entry of a jump table, 1 on alike
    {0x07EEEC, 0x08D616}, // the same entry of a jump table, 3 on alike
    {0x07EF00, 0x08D62A}, // the same entry of a jump table, 3 on alike
    {0x07EF0C, 0x08D636}, // the same entry of a jump table, 5 on alike
    {0x07EF22, 0x08D64C}, // the same entry of a jump table, 17 on alike
    {0x07EF76, 0x08D6A0}, // the same entry of a jump table, 1 on alike
    {0x07EF7C, 0x08D6A6}, // the same entry of a jump table, 3 on alike
    {0x07EF90, 0x08D6BA}, // the same entry of a jump table, 19 on alike
    {0x07EFE8, 0x08D712}, // a place the code names so (1/1)
    {0x07EFE9, 0x08D713}, // a place the code names so (1/1)
    {0x07EFF0, 0x08D71A}, // a place the code names so (1/1)
    {0x07EFF2, 0x08D71C}, // the same bytes, once only
    {0x07F030, 0x08D75A}, // the same entry of a jump table, 6 on alike
    {0x07F04C, 0x08D776}, // the same entry of a jump table, 1 on alike
    {0x07F052, 0x08D77C}, // the same entry of a jump table, 3 on alike
    {0x07F066, 0x08D790}, // the same entry of a jump table, 6 on alike
    {0x07F07C, 0x08D7A6}, // the same entry of a jump table, 5 on alike
    {0x07F09A, 0x08D7C4}, // a place the code names so (2/2)
    {0x07F0A6, 0x08D7D0}, // an instruction, 24 on alike
    {0x07F126, 0x08D850}, // an instruction, 11 on alike
    {0x07F150, 0x08D87A}, // an instruction, 6 on alike
    {0x07F168, 0x08D892}, // the same bytes, once only
    {0x07F19E, 0x08D8C8}, // the same entry of a jump table, 1 on alike
    {0x07F1A4, 0x08D8CE}, // the same entry of a jump table, 3 on alike
    {0x07F1B8, 0x08D8E2}, // the same entry of a jump table, 4 on alike
    {0x07F1C8, 0x08D8F2}, // the same entry of a jump table, 12 on alike
    {0x07F1FC, 0x08D926}, // the same entry of a jump table, 1 on alike
    {0x07F202, 0x08D92C}, // the same entry of a jump table, 3 on alike
    {0x07F216, 0x08D940}, // the same entry of a jump table, 11 on alike
    {0x07F26A, 0x08D994}, // the same entry of a jump table, 24 on alike
    {0x07F2EA, 0x08DA14}, // the same entry of a jump table, 1 on alike
    {0x07F2F0, 0x08DA1A}, // the same entry of a jump table, 3 on alike
    {0x07F304, 0x08DA2E}, // the same entry of a jump table, 24 on alike
    {0x07F3A2, 0x08DACC}, // the same entry of a jump table, 24 on alike
    {0x07F432, 0x08DB5C}, // the same entry of a jump table, 1 on alike
    {0x07F438, 0x08DB62}, // the same entry of a jump table, 3 on alike
    {0x07F44C, 0x08DB76}, // the same entry of a jump table, 24 on alike
    {0x07F546, 0x08DC70}, // the same entry of a jump table, 3 on alike
    {0x07F552, 0x08DC7C}, // a place the code names so (2/2)
    {0x07F55A, 0x08DC84}, // the same entry of a jump table, 5 on alike
    {0x07F56A, 0x08DC94}, // the same entry of a jump table, 15 on alike
    {0x07F5A8, 0x08DCD2}, // the same entry of a jump table, 11 on alike
    {0x07F5DA, 0x08DD04}, // the same entry of a jump table, 20 on alike
    {0x07F634, 0x08DD5E}, // the same entry of a jump table, 1 on alike
    {0x07F63A, 0x08DD64}, // a place the code names so (4/4)
    {0x07F79E, 0x08DEC8}, // the same entry of a jump table, 3 on alike
    {0x07F7B2, 0x08DEDC}, // the same entry of a jump table, 16 on alike
    {0x07F808, 0x08DF32}, // the same entry of a jump table, 4 on alike
    {0x07F818, 0x08DF42}, // a place the code names so (2/2)
    {0x07F81C, 0x08DF46}, // the same entry of a jump table, 4 on alike
    {0x07F828, 0x08DF52}, // the same entry of a jump table, 12 on alike
    {0x07F86A, 0x08DF94}, // the same entry of a jump table, 1 on alike
    {0x07F870, 0x08DF9A}, // the same entry of a jump table, 3 on alike
    {0x07F884, 0x08DFAE}, // the same entry of a jump table, 4 on alike
    {0x07F896, 0x08DFC0}, // the same entry of a jump table, 7 on alike
    {0x07F8B8, 0x08DFE2}, // a place the code names so (2/2)
    {0x07F8BC, 0x08DFE6}, // an instruction, 11 on alike
    {0x07F8EA, 0x08E014}, // an instruction, 15 on alike
    {0x07F922, 0x08E04C}, // the same entry of a jump table, 1 on alike
    {0x07F928, 0x08E052}, // the same entry of a jump table, 3 on alike
    {0x07F93C, 0x08E066}, // the same entry of a jump table, 6 on alike
    {0x07F95C, 0x08E086}, // the same entry of a jump table, 5 on alike
    {0x07F97A, 0x08E0A4}, // a place the code names so (2/2)
    {0x07F97E, 0x08E0A8}, // an instruction, 5 on alike
    {0x07F9B2, 0x08E0DC}, // an instruction, 11 on alike
    {0x07F9DE, 0x08E108}, // the same entry of a jump table, 1 on alike
    {0x07F9E4, 0x08E10E}, // the same entry of a jump table, 3 on alike
    {0x07F9F8, 0x08E122}, // the same entry of a jump table, 9 on alike
    {0x07FA1C, 0x08E146}, // the same entry of a jump table, 9 on alike
    {0x07FA6A, 0x08E194}, // the same entry of a jump table, 4 on alike
    {0x07FA7C, 0x08E1A6}, // the same entry of a jump table, 1 on alike
    {0x07FA82, 0x08E1AC}, // the same entry of a jump table, 3 on alike
    {0x07FA96, 0x08E1C0}, // the same entry of a jump table, 13 on alike
    {0x07FADA, 0x08E204}, // the same entry of a jump table, 9 on alike
    {0x07FAFC, 0x08E226}, // the same entry of a jump table, 1 on alike
    {0x07FB02, 0x08E22C}, // the same entry of a jump table, 3 on alike
    {0x07FB16, 0x08E240}, // the same entry of a jump table, 3 on alike
    {0x07FB22, 0x08E24C}, // the same entry of a jump table, 13 on alike
    {0x07FB58, 0x08E282}, // the same entry of a jump table, 1 on alike
    {0x07FB5E, 0x08E288}, // the same entry of a jump table, 3 on alike
    {0x07FB72, 0x08E29C}, // the same entry of a jump table, 4 on alike
    {0x07FB82, 0x08E2AC}, // the same entry of a jump table, 10 on alike
    {0x07FBAC, 0x08E2D6}, // the same entry of a jump table, 4 on alike
    {0x07FBBE, 0x08E2E8}, // the same entry of a jump table, 1 on alike
    {0x07FBC4, 0x08E2EE}, // the same entry of a jump table, 3 on alike
    {0x07FBD8, 0x08E302}, // the same entry of a jump table, 4 on alike
    {0x07FBE8, 0x08E312}, // the same entry of a jump table, 7 on alike
    {0x07FC06, 0x08E330}, // the same entry of a jump table, 1 on alike
    {0x07FC0C, 0x08E336}, // the same entry of a jump table, 3 on alike
    {0x07FC20, 0x08E34A}, // the same entry of a jump table, 12 on alike
    {0x07FC56, 0x08E380}, // the same entry of a jump table, 24 on alike
    {0x07FC6E, 0x08E398}, // the same entry of a jump table, 18 on alike
    {0x07FCB6, 0x08E3E0}, // the same entry of a jump table, 1 on alike
    {0x07FCF0, 0x08E41A}, // the same entry of a jump table, 3 on alike
    {0x07FD04, 0x08E42E}, // the same entry of a jump table, 15 on alike
    {0x07FD4A, 0x08E474}, // the same entry of a jump table, 10 on alike
    {0x07FE00, 0x08E52A}, // the same entry of a jump table, 1 on alike
    {0x07FE06, 0x08E530}, // the same entry of a jump table, 3 on alike
    {0x07FE1A, 0x08E544}, // the same entry of a jump table, 3 on alike
    {0x07FE26, 0x08E550}, // the same entry of a jump table, 11 on alike
    {0x07FE54, 0x08E57E}, // the same entry of a jump table, 1 on alike
    {0x07FE5A, 0x08E584}, // the same entry of a jump table, 3 on alike
    {0x07FE6E, 0x08E598}, // the same entry of a jump table, 5 on alike
    {0x07FE84, 0x08E5AE}, // the same entry of a jump table, 5 on alike
    {0x07FEA6, 0x08E5D0}, // a place the code names so (2/2)
    {0x07FEAA, 0x08E5D4}, // an instruction, 8 on alike
    {0x07FECA, 0x08E5F4}, // an instruction, 17 on alike
    {0x07FF1A, 0x08E644}, // the same entry of a jump table, 5 on alike
    {0x07FF30, 0x08E65A}, // the same entry of a jump table, 1 on alike
    {0x07FF76, 0x08E6A0}, // the same entry of a jump table, 3 on alike
    {0x07FF8A, 0x08E6B4}, // the same entry of a jump table, 4 on alike
    {0x07FF9A, 0x08E6C4}, // the same entry of a jump table, 4 on alike
    {0x07FFAE, 0x08E6D8}, // the same entry of a jump table, 1 on alike
    {0x07FFB4, 0x08E6DE}, // a table between two that moved alike (other bytes)
    {0x07FFDE, 0x08E708}, // the same entry of a jump table, 3 on alike
    {0x07FFF2, 0x08E71C}, // the same entry of a jump table, 6 on alike
    {0x08000E, 0x08E738}, // the same entry of a jump table, 12 on alike
    {0x08003C, 0x08E766}, // the same entry of a jump table, 9 on alike
    {0x080060, 0x08E78A}, // the same entry of a jump table, 1 on alike
    {0x080094, 0x08E7BE}, // the same entry of a jump table, 3 on alike
    {0x0800A8, 0x08E7D2}, // the same entry of a jump table, 4 on alike
    {0x0800B8, 0x08E7E2}, // the same entry of a jump table, 7 on alike
    {0x0800D4, 0x08E7FE}, // the same entry of a jump table, 4 on alike
    {0x0800E6, 0x08E810}, // the same entry of a jump table, 1 on alike
    {0x0800EC, 0x08E816}, // the same entry of a jump table, 3 on alike
    {0x080100, 0x08E82A}, // the same entry of a jump table, 7 on alike
    {0x08011C, 0x08E846}, // the same entry of a jump table, 24 on alike
    {0x0801CE, 0x08E8F8}, // the same entry of a jump table, 6 on alike
    {0x0801EC, 0x08E916}, // the same entry of a jump table, 1 on alike
    {0x0801F2, 0x08E91C}, // a place the code names so (1/1)
    {0x080378, 0x08EAA2}, // the same entry of a jump table, 3 on alike
    {0x08038C, 0x08EAB6}, // the same entry of a jump table, 5 on alike
    {0x0803A4, 0x08EACE}, // the same entry of a jump table, 16 on alike
    {0x0803E2, 0x08EB0C}, // the same entry of a jump table, 1 on alike
    {0x0803E8, 0x08EB12}, // the same entry of a jump table, 3 on alike
    {0x0803FC, 0x08EB26}, // the same entry of a jump table, 5 on alike
    {0x080414, 0x08EB3E}, // the same entry of a jump table, 21 on alike
    {0x08045C, 0x08EB86}, // the same entry of a jump table, 1 on alike
    {0x080462, 0x08EB8C}, // the same entry of a jump table, 3 on alike
    {0x080476, 0x08EBA0}, // the same entry of a jump table, 5 on alike
    {0x08048E, 0x08EBB8}, // the same entry of a jump table, 18 on alike
    {0x0804D2, 0x08EBFC}, // the same entry of a jump table, 1 on alike
    {0x0804D8, 0x08EC02}, // the same bytes, once only
    {0x0805A4, 0x08ECCE}, // the same entry of a jump table, 3 on alike
    {0x0806FA, 0x08EE24}, // the same entry of a jump table, 3 on alike
    {0x08070E, 0x08EE38}, // the same entry of a jump table, 20 on alike
    {0x080724, 0x08EE4E}, // the same entry of a jump table, 15 on alike
    {0x080762, 0x08EE8C}, // the same entry of a jump table, 1 on alike
    {0x080768, 0x08EE92}, // the same entry of a jump table, 3 on alike
    {0x08077C, 0x08EEA6}, // the same entry of a jump table, 10 on alike
    {0x080792, 0x08EEBC}, // the same entry of a jump table, 5 on alike
    {0x0807A8, 0x08EED2}, // the same entry of a jump table, 4 on alike
    {0x0807BA, 0x08EEE4}, // the same entry of a jump table, 1 on alike
    {0x0807C0, 0x08EEEA}, // the same entry of a jump table, 3 on alike
    {0x0807D4, 0x08EEFE}, // the same entry of a jump table, 21 on alike
    {0x0807F4, 0x08EF1E}, // the same entry of a jump table, 14 on alike
    {0x080834, 0x08EF5E}, // the same entry of a jump table, 1 on alike
    {0x08083A, 0x08EF64}, // the same entry of a jump table, 4 on alike
    {0x08084A, 0x08EF74}, // a place the code names so (2/2)
    {0x080852, 0x08EF7C}, // the same entry of a jump table, 7 on alike
    {0x080864, 0x08EF8E}, // the same entry of a jump table, 3 on alike
    {0x080876, 0x08EFA0}, // the same entry of a jump table, 1 on alike
    {0x08087C, 0x08EFA6}, // the same entry of a jump table, 3 on alike
    {0x080890, 0x08EFBA}, // the same entry of a jump table, 23 on alike
    {0x0808A6, 0x08EFD0}, // the same entry of a jump table, 18 on alike
    {0x08096A, 0x08F094}, // the same entry of a jump table, 1 on alike
    {0x080970, 0x08F09A}, // the same entry of a jump table, 3 on alike
    {0x080984, 0x08F0AE}, // the same entry of a jump table, 7 on alike
    {0x0809A6, 0x08F0D0}, // the same entry of a jump table, 7 on alike
    {0x0809DE, 0x08F108}, // the same entry of a jump table, 1 on alike
    {0x0809E4, 0x08F10E}, // the same entry of a jump table, 3 on alike
    {0x0809F8, 0x08F122}, // the same entry of a jump table, 9 on alike
    {0x080A20, 0x08F14A}, // the same entry of a jump table, 10 on alike
    {0x080A7A, 0x08F1A4}, // the same entry of a jump table, 1 on alike
    {0x09190A, 0x09C4D6}, // an instruction, 10 on alike
    {0x09192A, 0x09C4F6}, // an instruction, 7 on alike
    {0x0B7640, 0x0D14BE}, // a place the code names so (3/3)
    {0x0B7740, 0x0D15BE}, // a place the code names so (1/1)
    {0x0B7940, 0x0D17BE}, // a place the code names so (2/2)
    {0x0BCE7A, 0x0D7018}, // a place the code names so (2/2)
    {0x0BCEFA, 0x0D7098}, // a place the code names so (1/1)
    {0x0BCF7A, 0x0D7118}, // a place the code names so (4/4)
    {0x100000, 0x100000}, // a place the code names so (18/18)
    {0x2776B0, 0x2ABD58}, // a place the code names so (2/2)
    {0x357CA4, 0x368CC0}, // a place the code names so (3/3)
    {0x35AD18, 0x36BD34}, // a place the code names so (2/2)
    {0x35C918, 0x36D2B4}, // a place the code names so (2/2)
    {0x38C198, 0x396B94}, // a place the code names so (5/5)
    {0x38C218, 0x396C14}, // a place the code names so (4/4)
    {0x38C298, 0x396C94}, // a place the code names so (3/3)
    {0x3A0B20, 0x3B7FDC}, // the same bytes, once only
    {0x3A1420, 0x3B8A1C}, // the same bytes, once only
    {0x3A1660, 0x3B8C5C}, // a table between two that moved alike (the same bytes)
    {0x3A1680, 0x3B8C7C}, // a table between two that moved alike (the same bytes)
    {0x3A16A0, 0x3B8C9C}, // the same bytes, once only
    {0x3A2640, 0x3B9C3C}, // the same bytes, once only
    {0x3A2F40, 0x3BA67C}, // the same bytes, once only
    {0x3A3180, 0x3BA8BC}, // a table between two that moved alike (the same bytes)
    {0x3A31A0, 0x3BA8DC}, // a table between two that moved alike (the same bytes)
    {0x3A31C0, 0x3BA8FC}, // the same bytes, once only
    {0xFF0D66, 0xFF0D66}, // a variable the code names so (6/6)
    {0xFF0D6C, 0xFF0D6C}, // a variable the code names so (6/6)
    {0xFF0D70, 0xFF0D70}, // a variable the code names so (6/6)
    {0xFF0E0E, 0xFF0E0E}, // a variable the code names so (3/3)
    {0xFF1E0E, 0xFF1E0E}, // a variable the code names so (4/4)
    {0xFF1E2A, 0xFF1E2A}, // a variable the code names so (1/1)
    {0xFF310E, 0xFF3120}, // the end of the first list (the things thrown), as ThrownAgainstPlayers (0193C4) has it by hand
    {0xFF3436, 0xFF3488}, // a variable the code names so (3/3)
    {0xFF3438, 0xFF348A}, // a variable the code names so (3/3)
    {0xFF343A, 0xFF348C}, // a variable the code names so (20/20)
    {0xFF343C, 0xFF348E}, // a variable the code names so (7/7)
    {0xFF343E, 0xFF3490}, // a variable the code names so (2/2)
    {0xFF3442, 0xFF3494}, // a variable the code names so (5/5)
    {0xFF3444, 0xFF3496}, // a variable the code names so (5/5)
    {0xFF3448, 0xFF349A}, // a variable the code names so (2/2)
    {0xFF344A, 0xFF349C}, // a variable the code names so (8/8)
    {0xFF344C, 0xFF349E}, // a variable the code names so (8/8)
    {0xFF344E, 0xFF34A0}, // a variable the code names so (12/12)
    {0xFF3450, 0xFF34A2}, // a variable the code names so (12/12)
    {0xFF3452, 0xFF34A4}, // a variable the code names so (3/3)
    {0xFF345A, 0xFF34AC}, // a variable the code names so (3/3)
    {0xFF3462, 0xFF34B4}, // a variable the code names so (3/3)
    {0xFF346A, 0xFF34BC}, // a variable the code names so (5/5)
    {0xFF346C, 0xFF34BE}, // a variable the code names so (6/6)
    {0xFF346E, 0xFF34C0}, // a variable the code names so (5/5)
    {0xFF3470, 0xFF34C2}, // a variable the code names so (5/5)
    {0xFF3471, 0xFF34C3}, // a variable the code names so (6/6)
    {0xFF3476, 0xFF34C8}, // a variable the code names so (2/2)
    {0xFF347E, 0xFF34D0}, // a variable the code names so (1/1)
    {0xFF3480, 0xFF34D2}, // a variable the code names so (1/1)
    {0xFF4302, 0xFF4382}, // a variable the code names so (5/5)
    {0xFF8004, 0xFF8004}, // a variable the code names so (39/39)
    {0xFF8008, 0xFF8008}, // a variable the code names so (80/80)
    {0xFF800C, 0xFF800C}, // a variable the code names so (101/101)
    {0xFF8010, 0xFF8010}, // a variable the code names so (62/62)
    {0xFF801A, 0xFF801A}, // a variable the code names so (12/12)
    {0xFF801E, 0xFF801E}, // a variable the code names so (8/8)
    {0xFF8022, 0xFF8022}, // a variable the code names so (23/23)
    {0xFF8024, 0xFF8024}, // a variable the code names so (23/23)
    {0xFF8026, 0xFF8026}, // a variable the code names so (22/22)
    {0xFF8028, 0xFF8028}, // a variable the code names so (23/23)
    {0xFF802A, 0xFF802A}, // a variable the code names so (18/18)
    {0xFF802C, 0xFF802C}, // a variable the code names so (18/18)
    {0xFF802E, 0xFF802E}, // a variable the code names so (4/4)
    {0xFF8058, 0xFF8058}, // a variable the code names so (20/20)
    {0xFF805A, 0xFF805A}, // a variable the code names so (8/8)
    {0xFF805C, 0xFF805C}, // a variable the code names so (11/11)
    {0xFF805E, 0xFF805E}, // a variable the code names so (6/6)
    {0xFF8060, 0xFF8060}, // a variable the code names so (16/16)
    {0xFF8064, 0xFF8064}, // a variable the code names so (7/7)
    {0xFF8076, 0xFF8076}, // a variable the code names so (7/7)
    {0xFF8085, 0xFF8085}, // a variable the code names so (4/4)
    {0xFF8087, 0xFF8087}, // a variable the code names so (6/6)
    {0xFF808A, 0xFF808A}, // a variable the code names so (34/34)
    {0xFF808C, 0xFF808C}, // a variable the code names so (10/10)
    {0xFF808D, 0xFF808D}, // a variable the code names so (15/15)
    {0xFF808F, 0xFF808F}, // a variable the code names so (39/39)
    {0xFF8090, 0xFF8090}, // a variable the code names so (112/112)
    {0xFF8098, 0xFF8098}, // a variable the code names so (18/18)
    {0xFF8099, 0xFF8099}, // a variable the code names so (9/9)
    {0xFF80A0, 0xFF80A0}, // a variable the code names so (10/10)
    {0xFF80A1, 0xFF80A1}, // a variable the code names so (10/10)
    {0xFF80A2, 0xFF80A2}, // a variable the code names so (10/10)
    {0xFF80A3, 0xFF80A3}, // a variable the code names so (15/15)
    {0xFF80A4, 0xFF80A4}, // a variable the code names so (12/12)
    {0xFF80A5, 0xFF80A5}, // a variable the code names so (17/17)
    {0xFF80AC, 0xFF80AC}, // a variable the code names so (49/49)
    {0xFF80B4, 0xFF80B4}, // a variable the code names so (51/51)
    {0xFF80B5, 0xFF80B5}, // a variable the code names so (22/22)
    {0xFF80BB, 0xFF80BB}, // a variable the code names so (10/10)
    {0xFF80BD, 0xFF80BD}, // a variable the code names so (33/33)
    {0xFF80BF, 0xFF80BF}, // a variable the code names so (10/12)
    {0xFF80D0, 0xFF80D0}, // a variable the code names so (28/28)
    {0xFF80D4, 0xFF80D4}, // the random number (battle.cpp): 0xD4(a5) in this game's routine of it too (01357E) by hand
    {0xFF8100, 0xFF8100}, // a variable the code names so (41/41)
    {0xFF8107, 0xFF8107}, // a variable the code names so (26/26)
    {0xFF8109, 0xFF8109}, // a variable the code names so (12/12)
    {0xFF810A, 0xFF810A}, // a variable the code names so (2/2)
    {0xFF810B, 0xFF810B}, // a variable the code names so (50/50)
    {0xFF810C, 0xFF810C}, // a variable the code names so (10/10)
    {0xFF810D, 0xFF810D}, // a variable the code names so (61/61)
    {0xFF810E, 0xFF810E}, // a variable the code names so (35/35)
    {0xFF8118, 0xFF8118}, // a variable the code names so (8/8)
    {0xFF8125, 0xFF8125}, // a variable the code names so (4/4)
    {0xFF812C, 0xFF812C}, // a variable the code names so (14/14)
    {0xFF812D, 0xFF812D}, // a variable the code names so (7/7)
    {0xFF8131, 0xFF8131}, // a variable the code names so (6/6)
    {0xFF8133, 0xFF8165}, // a variable the code names so (6/9)
    {0xFF8134, 0xFF8134}, // a variable the code names so (74/74)
    {0xFF8137, 0xFF8137}, // a variable the code names so (10/10)
    {0xFF813E, 0xFF813E}, // a variable the code names so (16/16)
    {0xFF8140, 0xFF8140}, // a variable the code names so (10/10)
    {0xFF814C, 0xFF814C}, // a variable the code names so (10/10)
    {0xFF815D, 0xFF815D}, // a variable the code names so (18/19)
    {0xFF8200, 0xFF8200}, // a variable the code names so (3/3)
    {0xFF8280, 0xFF8280}, // a variable the code names so (5/5)
    {0xFF8290, 0xFF8290}, // a variable the code names so (56/56)
    {0xFF8294, 0xFF8294}, // a variable the code names so (20/20)
    {0xFF82A0, 0xFF82A0}, // a variable the code names so (15/15)
    {0xFF82A8, 0xFF82A8}, // a variable between two that moved alike
    {0xFF8300, 0xFF8300}, // a variable the code names so (4/4)
    {0xFF8380, 0xFF8380}, // a variable the code names so (10/10)
    {0xFF8390, 0xFF8390}, // a variable between two that moved alike
    {0xFF8398, 0xFF8398}, // a variable the code names so (9/9)
    {0xFF83A0, 0xFF83A0}, // a variable the code names so (6/6)
    {0xFF8400, 0xFF8400}, // a variable the code names so (61/61)
    {0xFF8800, 0xFF8800}, // a variable the code names so (60/60)
    {0xFF9400, 0xFF9400}, // a variable the code names so (4/4)
    {0xFFC800, 0xFFC800}, // a variable the code names so (2/2)
    {0xFFF420, 0xFFF420}, // a variable the code names so (1/1)
    {0xFFF421, 0xFFF421}, // a variable the code names so (1/1)
    {0xFFF424, 0xFFF424}, // a variable the code names so (4/4)
    {0xFFF425, 0xFFF425}, // a variable the code names so (2/2)
    {0xFFF972, 0xFFF9F2}, // a variable the code names so (5/5)
    {0xFFF982, 0xFFFA02}, // a variable the code names so (6/6)
    {0xFFF9C1, 0xFFFA45}, // a variable the code names so (6/6)
    {0xFFF9C3, 0xFFFA47}, // a variable the code names so (4/4)
    {0xFFF9C5, 0xFFFA49}, // a variable the code names so (4/5)
    // places: end
};
inline constexpr auto kPlaces = std::to_array(kPlaceList);

// The characters a player chooses, by the number the game keeps for each (a player's record, +382), with
// what this set calls them and, if another, what the American and European sets do. Vampire Savior's but for
// Gallon, Aulbath and Sasquatch; Phobos, Pyron and Donovan in their place (numbers seen in the attract mode's
// fights and the character choice's code, 01F832)
struct Character {
    uint8_t number;
    const char *name, *otherName;
};
inline constexpr Character kRoster[] = {
    {0x00, "Bulleta", "BB Hood"}, {0x01, "Demitri", nullptr}, {0x03, "Victor", nullptr},   {0x04, "Zabel", "Lord Raptor"},
    {0x05, "Morrigan", nullptr},  {0x06, "Anakaris", nullptr}, {0x07, "Felicia", nullptr}, {0x08, "Bishamon", nullptr},
    {0x0C, "Q-Bee", nullptr},     {0x0D, "Lei-Lei", "Hsien-Ko"}, {0x0E, "Lilith", nullptr}, {0x0F, "Jedah", nullptr},
    {0x10, "Phobos", "Huitzil"},  {0x11, "Pyron", nullptr},   {0x13, "Donovan", nullptr},
};
// The hidden ones (src/choosing.cpp). This game's own choosing code (not rewritten: its hidden characters differ)
// gives Oboro Bishamon as 19 (Bishamon chosen with Start held and a button: 01F84C), Shadow (Start five times on the
// "?" box: 01F8C4) and Marionette (seven times: 01F930), and Phobos and Donovan another way with Start held (the
// byte at +3C2); 18 is Oboro Bishamon as the last opponent. The "Hidden characters" feature hooks that code: where
// it stores the colours (01F7EC) and where it counts Shadow's code
inline constexpr Character kHidden[] = {{0x19, "Oboro Bishamon", nullptr}, {0x18, "Oboro Bishamon (the boss)", nullptr}};
inline constexpr uint8_t kDarkGallon = 0, kOboroBishamon = 0x19;
inline constexpr uint32_t kHiddenAtColours = 0x01F7EC, kHiddenShadow = 0x01F8C4;
inline constexpr const char *kHiddenWords =
    "Choosing Bishamon gives Oboro Bishamon (on the board: Start held as he is chosen); a punch chooses his first "
    "colours, a kick his second. The \"?\" box gives Shadow, who becomes each opponent beaten (on the board: Start five "
    "times on it). For player 1, player 2 or both; the alternative modes' fighters too. (Also on the board: Start seven "
    "times on the box for Marionette, and Start held as Phobos or Donovan is chosen for their other way.)";

// The stages, in the order of their numbers (the word at FF8100 is the number doubled)
inline constexpr const char *kStageNames[] = {
    "Feast of the Damned", "Concrete Cave", "Tower of Arrogance", "Red Thirst", "Deserted Chateau", "Abaraya",
    "Vanity Paradise", "War Agony", "Forever Torment", "Green Scream", "Iron Horse, Iron Terror", "Fetus of God",
    "Feast of the Damned, empty",
};

// The frame's lists of objects to draw that the objects of kind 12 have, one for each depth (src/picture.cpp): one more
// than Vampire Savior's (the table at 013D0C), so that the stage's pieces' lists come one later
inline constexpr int kTwelveLists = 8;

// Each stage's own tune (src/modes.cpp: the stage viewer's music): Vampire Savior's, and the thirteenth stage's (the
// empty hall, Donovan's) a new one
inline constexpr uint16_t kStageTunes[] = {0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15};

// Its music, for a custom soundtrack (src/music.cpp): the sound program keeps a table of its songs at 9000 (a word,
// how many; song 0's entry four bytes on). Which are music, from that table (their data starts with a 0); what each
// is, from the arcade ladder played to its end with a character held, attract mode and the stages
// (CPS2RECOMP_SOUNDTRACK_LOG)
struct Song {
    uint16_t number;
    const char *name, *title;
    bool once = false;
};
inline constexpr uint32_t kSongTable = 0x9006, kSongCount = 0xA70;
inline constexpr Song kSongs[] = {
    // Around the fights (as Vampire Savior has them: the character select, the versus screen, game over seen)
    {0x01, "opening", "The opening", true},
    {0x02, "character_select", "Character select"},
    {0x03, "next", "Between a win and the next versus screen"},
    {0x05, "versus", "Versus screen", true},
    {0x06, "continue", "Continue?"},
    {0x07, "game_over", "Game over", true},
    {0x08, "ranking", "Score ranking", true},
    // Each stage's own (kStageTunes)
    {0x09, "feast_of_the_damned", "Feast of the Damned"},
    {0x0A, "concrete_cave", "Concrete Cave"},
    {0x0B, "tower_of_arrogance", "Tower of Arrogance"},
    {0x0C, "red_thirst", "Red Thirst"},
    {0x0D, "deserted_chateau", "Deserted Chateau"},
    {0x0E, "abaraya", "Abaraya"},
    {0x0F, "vanity_paradise", "Vanity Paradise"},
    {0x10, "war_agony", "War Agony"},
    {0x11, "forever_torment", "Forever Torment"},
    {0x12, "green_scream", "Green Scream"},
    {0x13, "iron_horse_iron_terror", "Iron Horse, Iron Terror"},
    {0x14, "fetus_of_god", "Fetus of God"},
    {0x15, "feast_of_the_damned_empty", "Feast of the Damned, empty"},
    {0x30, "rival", "Before the last fight (the rival)"},
    {0xE2, "jedah_appears", "Jedah appears", true},
    // After Jedah is beaten: the ending, then one of two ways on
    {0x38, "ending", "The ending"},
    {0x39, "ending_2", "The ending, on (one way)"},
    {0x3A, "ending_3", "The ending, on (the other way)"},
    // The winner's, on the screen after a win (17 and the character's number)
    {0x17, "bulleta_win", "Bulleta wins"},
    {0x18, "demitri_win", "Demitri wins"},
    {0x1A, "victor_win", "Victor wins"},
    {0x1B, "zabel_win", "Zabel wins"},
    {0x1C, "morrigan_win", "Morrigan wins"},
    {0x1D, "anakaris_win", "Anakaris wins"},
    {0x1E, "felicia_win", "Felicia wins"},
    {0x1F, "bishamon_win", "Bishamon wins"},
    {0x23, "qbee_win", "Q-Bee wins"},
    {0x24, "leilei_win", "Lei-Lei wins"},
    {0x25, "lilith_win", "Lilith wins"},
    {0x26, "jedah_win", "Jedah wins"},
    {0x27, "phobos_win", "Phobos wins"},
    {0x28, "pyron_win", "Pyron wins"},
    {0x2A, "donovan_win", "Donovan wins"},
    // Music the game has whose place is not known yet
    {0x04, "music_04", "Where it plays is not known yet"},
    {0x16, "music_16", "Where it plays is not known yet"},
    {0x2B, "music_2b", "Where it plays is not known yet"},
    {0x2C, "music_2c", "Where it plays is not known yet"},
    {0x2F, "music_2f", "Where it plays is not known yet"},
    {0x31, "music_31", "Where it plays is not known yet"},
    {0x3B, "music_3b", "Where it plays is not known yet"},
    {0x3C, "music_3c", "Where it plays is not known yet"},
};

// Where the hit test goes on to when an attack has hit (src/collision.cpp): not HitLands itself as in Vampire Savior
// (018230), but a test before it of the victim's guarding pose (whose code goes on into HitLands)
inline constexpr uint32_t kHitTestHit = 0x016AA8;

// How the fight's rules differ from Vampire Savior's (src/fight.cpp, src/edges.cpp; 0: as in Vampire Savior), from
// this game's code:
// - a byte of the attacker's record that, set, has all of a hit's damage taken for good (TakeDamage, 017432), and
//   where (from A5) the game keeps A0 while it looks
// - one more kind of attack that is an overhead (HitLands, 016C82)
// - a byte of the victim's record that, set, lets it guard the attacks that cannot be guarded (HitLands, 016BAC)
// - how near (across) the other player has to be for a player to guard by being near: twice Vampire Savior's
//   (GuardRangeAttack, 026938)
// - a byte of a player's record set (FF) when the player is kept in at the picture's edge (KeepIn, 027444)
inline constexpr uint32_t kAllForGood = 0x1C3;
inline constexpr int32_t kAllForGoodKeep = -0x4B3C;
inline constexpr uint8_t kOverheadToo = 0x50;
inline constexpr uint32_t kGuardsAnything = 0x143;
inline constexpr uint16_t kGuardNear = 0x100;
inline constexpr uint32_t kAtEdgeToo = 0x148;

// Tiles of the objects the port tells apart (src/machines.cpp, src/widescreen.cpp), from one to one less than the
// other: the shadows (colour set 22), the panel's faces beside the bars (drawn in the players' own colours), the red
// curtain between a match's screens (set 3). The first two are Vampire Savior's moved in this game's graphics (the
// same pictures, found there); the curtain's, those of its objects at the versus screen.
inline constexpr uint32_t kShadowTiles[2] = {0x30F88, 0x30FC8};
inline constexpr uint32_t kPanelTiles[2] = {0x14200, 0x14E00};
inline constexpr uint32_t kCurtainTiles[2] = {0x04400, 0x05100};

// The characters whose three other palettes go elsewhere when their effects need them, a bit by number (02A108)
inline constexpr uint32_t kFirstLineOnly = 0xEF9AEF96;

// The ROM file whose colours PalMod changes, and which set it is
inline constexpr const char *kColoursFile = "vs2j.10", *kColoursFileWhose = "Vampire Savior 2's, 970913";

// What this game's own sources add (none: Vampire Savior's sources are all it has)
inline void RegisterOwnSources() {}

// What a character put in by a mode in place of the one chosen needs besides its number (src/training.cpp): Phobos's
// and Donovan's form, +3C2, which the character choice sets from Start held as they are chosen (01F832); the form
// chosen without it
inline void CharacterPut(uint32_t player, int character) {
    if (character == 0x10 || character == 0x13) {
        cps2recomp::SetRam8(player + 0x3C2, character == 0x13 ? 1 : 0);
    }
}


} // namespace vsav::family
