// SPDX-License-Identifier: GPL-3.0-only
// Copyright (C) 2026 Tommy McLeroy
//
// Vampire Hunter 2 as one of the games made from Vampire Savior's program, whose sources it builds
// (games/vampire_savior/src; places.h there): where the places those sources name are in this game.
#pragma once

#include <array>
#include <cstdint>

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
    {0x008F56, 0x0076B4}, // an instruction, 24 on alike
    {0x0097DC, 0x007F7E}, // an instruction, 7 on alike
    {0x009880, 0x008022}, // an instruction, 15 on alike
    {0x00995A, 0x0080FC}, // an instruction, 12 on alike
    {0x0099A8, 0x008162}, // an instruction, 24 on alike
    {0x00A3B2, 0x008B58}, // an instruction, 8 on alike
    {0x00A3EA, 0x008B92}, // an instruction, 6 on alike
    {0x00A4BA, 0x008C64}, // the same bytes, once only
    {0x00A60A, 0x008DC0}, // an instruction, 24 on alike
    {0x00AE7C, 0x00964C}, // an instruction, 10 on alike
    {0x00D560, 0x00BB9E}, // an instruction, 24 on alike
    {0x014E8A, 0x013584}, // an instruction, 9 on alike
    {0x015030, 0x01372A}, // an instruction, 10 on alike
    {0x015036, 0x013730}, // an instruction, 8 on alike
    {0x015084, 0x01377E}, // an instruction, 7 on alike
    {0x01520A, 0x013904}, // an instruction, 9 on alike
    {0x015466, 0x013B60}, // an instruction, 9 on alike
    {0x01549A, 0x013B94}, // an instruction, 14 on alike
    {0x0154CC, 0x013BC6}, // an instruction, 10 on alike
    {0x0154FE, 0x013BF8}, // an instruction, 10 on alike
    {0x01551A, 0x013C14}, // an instruction, 19 on alike
    {0x015546, 0x013C40}, // an instruction, 5 on alike
    {0x01555C, 0x013C56}, // the same entry of a jump table, 1 on alike
    {0x015572, 0x013C6C}, // an instruction, 14 on alike
    {0x01559E, 0x013C98}, // an instruction, 4 on alike
    {0x0155BA, 0x013CB4}, // the same entry of a jump table, 5 on alike
    {0x0155FC, 0x013CF6}, // the same entry of a jump table, 3 on alike
    {0x01560C, 0x013D06}, // the same entry of a jump table, 3 on alike
    {0x015696, 0x013DA2}, // an instruction, 4 on alike
    {0x015698, 0x013DA4}, // the same entry of a jump table, 3 on alike
    {0x016FD0, 0x01571E}, // the same entry of a jump table, 7 on alike
    {0x016FFC, 0x01574A}, // the same entry of a jump table, 7 on alike
    {0x0170A6, 0x0157F4}, // an instruction, 12 on alike
    {0x017F0C, 0x01678A}, // an instruction, 6 on alike
    {0x017F22, 0x0167A0}, // an instruction, 24 on alike
    {0x017FF4, 0x016872}, // an instruction, 20 on alike
    {0x018054, 0x0168D2}, // an instruction, 24 on alike
    {0x018230, 0x016AF2}, // HitLands itself (the hit test calls a test of the guarding pose first, which goes on into it: kHitTestHit) by hand
    {0x01841C, 0x016CEE}, // an instruction, 19 on alike
    {0x01884E, 0x01717E}, // an instruction, 9 on alike
    {0x0188BA, 0x017200}, // an instruction, 24 on alike
    {0x018952, 0x0172B6}, // where the code calls or jumps to it (3/3), not its first instruction's place
    {0x018968, 0x0172E0}, // an instruction, 9 on alike
    {0x018980, 0x0172F8}, // an instruction, 16 on alike
    {0x0189BA, 0x017332}, // an instruction, 24 on alike
    {0x018AB0, 0x017428}, // an instruction, 3 on alike
    {0x018E8A, 0x017856}, // an instruction, 7 on alike
    {0x019030, 0x017A30}, // an instruction, 8 on alike
    {0x01904E, 0x017A4E}, // an instruction, 24 on alike
    {0x019172, 0x017B7A}, // an instruction, 24 on alike
    {0x01923C, 0x017C44}, // an instruction, 13 on alike
    {0x01926A, 0x017C72}, // an instruction, 24 on alike
    {0x0193B8, 0x017DC0}, // an instruction, 24 on alike
    {0x01A728, 0x019138}, // an instruction, 24 on alike
    {0x01BBC2, 0x01A634}, // an instruction, 24 on alike
    {0x01BC3A, 0x01A6AC}, // an instruction, 24 on alike
    {0x01BC88, 0x01A6FA}, // an instruction, 7 on alike
    {0x01C3A4, 0x01AE1A}, // an instruction, 17 on alike
    {0x01DE40, 0x01C9B8}, // an instruction, 6 on alike
    {0x01DE56, 0x01C9CE}, // an instruction, 8 on alike
    {0x01DE6E, 0x01C9E6}, // an instruction, 8 on alike
    {0x01DE86, 0x01C9FE}, // an instruction, 8 on alike
    {0x01DE9E, 0x01CA16}, // an instruction, 5 on alike
    {0x01DEB0, 0x01CA28}, // an instruction, 8 on alike
    {0x01DECE, 0x01CA46}, // an instruction, 24 on alike
    {0x01DEEA, 0x01CA62}, // an instruction, 18 on alike
    {0x01DF3A, 0x01CAB2}, // an instruction, 2 on alike
    {0x01DF40, 0x01CAB8}, // an instruction, 5 on alike
    {0x01DF50, 0x01CAC8}, // an instruction, 7 on alike
    {0x01DF64, 0x01CADC}, // an instruction, 21 on alike
    {0x01DF70, 0x01CAE8}, // an instruction, 18 on alike
    {0x01DFC0, 0x01CB38}, // an instruction, 12 on alike
    {0x01E36E, 0x01CECE}, // an instruction, 17 on alike
    {0x01E3F2, 0x01CF52}, // an instruction, 10 on alike
    {0x01E450, 0x01CFB0}, // an instruction, 18 on alike
    {0x01E4A0, 0x01D000}, // an instruction, 18 on alike
    {0x01E4F0, 0x01D050}, // an instruction, 12 on alike
    {0x01E75E, 0x01D2C0}, // an instruction, 9 on alike
    {0x01E77A, 0x01D2DC}, // an instruction, 8 on alike
    {0x01E792, 0x01D2F4}, // an instruction, 8 on alike
    {0x01E7AA, 0x01D30C}, // an instruction, 5 on alike
    {0x01E7BC, 0x01D31E}, // an instruction, 8 on alike
    {0x01E7DA, 0x01D33C}, // an instruction, 24 on alike
    {0x01E7F6, 0x01D358}, // an instruction, 18 on alike
    {0x01E846, 0x01D3A8}, // an instruction, 5 on alike
    {0x01E856, 0x01D3B8}, // an instruction, 7 on alike
    {0x01E86A, 0x01D3CC}, // an instruction, 21 on alike
    {0x01E876, 0x01D3D8}, // an instruction, 18 on alike
    {0x01E8C6, 0x01D428}, // an instruction, 12 on alike
    {0x01E8F4, 0x01D456}, // an instruction, 11 on alike
    {0x01E962, 0x01D4C4}, // an instruction, 7 on alike
    {0x01EE22, 0x01D990}, // an instruction, 19 on alike
    {0x01EE58, 0x01D9C6}, // an instruction, 16 on alike
    {0x01EEFA, 0x01DA68}, // an instruction, 4 on alike
    {0x01EF16, 0x01DA84}, // an instruction, 24 on alike
    {0x01EFA0, 0x01DB0E}, // an instruction, 24 on alike
    {0x01F280, 0x01DDF6}, // an instruction, 19 on alike
    {0x01F2B6, 0x01DE2C}, // an instruction, 17 on alike
    {0x01F31E, 0x01DE94}, // an instruction, 4 on alike
    {0x01F33A, 0x01DEB0}, // an instruction, 24 on alike
    {0x01F3C4, 0x01DF3A}, // an instruction, 24 on alike
    {0x01F64E, 0x01E18C}, // an instruction, 18 on alike
    {0x01F682, 0x01E1C0}, // an instruction, 15 on alike
    {0x01F71C, 0x01E25A}, // an instruction, 4 on alike
    {0x01F73A, 0x01E278}, // an instruction, 24 on alike
    {0x01F7C2, 0x01E300}, // an instruction, 24 on alike
    {0x01FDFE, 0x01E98A}, // an instruction, 11 on alike
    {0x01FE88, 0x01EA14}, // a place the code names so (2/2)
    {0x01FEA0, 0x01EA2E}, // moved as the code before or after it, 24 on alike
    {0x01FF1A, 0x01EAA8}, // an instruction, 24 on alike
    {0x01FF76, 0x01EB04}, // an instruction, 24 on alike
    {0x01FFCE, 0x01EB5C}, // an instruction, 24 on alike
    {0x02002E, 0x01EBBC}, // an instruction, 24 on alike
    {0x0200A2, 0x01EC30}, // an instruction, 24 on alike
    {0x020108, 0x01EC96}, // an instruction, 24 on alike
    {0x02016A, 0x01ECF8}, // an instruction, 24 on alike
    {0x0201CC, 0x01ED5A}, // an instruction, 24 on alike
    {0x020236, 0x01EDC4}, // an instruction, 24 on alike
    {0x020B38, 0x000000}, // ChooseColours: this game's chooses other hidden characters (no rewrite) by hand
    {0x020C98, 0x000000}, // CountShadowCode: this game's is not the same (no rewrite) by hand
    {0x02244C, 0x020E30}, // an instruction, 4 on alike
    {0x027694, 0x026898}, // an instruction, 24 on alike
    {0x0276A6, 0x0268AA}, // an instruction, 24 on alike
    {0x027EC0, 0x027142}, // the reelings' animation set (battle.cpp), found by its instructions: the second of three tables, then 0x382(a6) by hand
    {0x027F70, 0x0271F2}, // the animation on a step (battle.cpp), found by its instructions: subq.b #1,0x20(a6) and the frame's flags by hand
    {0x028192, 0x027414}, // an instruction, 24 on alike
    {0x02824C, 0x0274D6}, // an instruction, 24 on alike
    {0x07D240, 0x08B586}, // an instruction, 11 on alike
    {0x07D354, 0x08B69A}, // the same entry of a jump table, 3 on alike
    {0x07D372, 0x08B6B8}, // an instruction, 5 on alike
    {0x07D3A2, 0x08B6E8}, // script 1, from the table of scripts after AllPieces (07D26C) by hand
    {0x07D3F6, 0x08B73A}, // script 2, from the table of scripts after AllPieces (07D26C) by hand
    {0x07D4BA, 0x08B7FA}, // the same entry of a jump table, 3 on alike
    {0x07D522, 0x08B860}, // a place the code names so (1/1)
    {0x07D534, 0x08B872}, // the same entry of a jump table, 3 on alike
    {0x07D59C, 0x08B8D8}, // the same entry of a jump table, 3 on alike
    {0x07D5B0, 0x08B8EC}, // the same entry of a jump table, 12 on alike
    {0x07D600, 0x08B93A}, // the same entry of a jump table, 5 on alike
    {0x07D61E, 0x08B958}, // a place the code names so (2/2)
    {0x07D62A, 0x08B964}, // an instruction, 10 on alike
    {0x07D656, 0x08B990}, // an instruction, 11 on alike
    {0x07D68A, 0x08B9C4}, // an instruction, 13 on alike
    {0x07D6D4, 0x08BA0E}, // an instruction, 5 on alike
    {0x07D6E6, 0x08BA20}, // an instruction, 6 on alike
    {0x07D6FE, 0x08BA38}, // inside code that moved alike, 4 on alike
    {0x07D70A, 0x08BA44}, // the same entry of a jump table, 1 on alike
    {0x07D710, 0x08BAE2}, // the same entry of a jump table, 3 on alike
    {0x07D724, 0x08BAF6}, // the same entry of a jump table, 24 on alike
    {0x07D7C6, 0x08BB98}, // the same entry of a jump table, 21 on alike
    {0x07D820, 0x08BBF2}, // the same entry of a jump table, 1 on alike
    {0x07D87C, 0x08BC4E}, // a place the code names so (1/1)
    {0x07D8C4, 0x08BC96}, // a place the code names so (1/1)
    {0x07D8D4, 0x08BCA6}, // a place the code names so (1/1)
    {0x07D8DC, 0x08BCAE}, // a place the code names so (1/1)
    {0x07DDD8, 0x08C1AA}, // the same entry of a jump table, 3 on alike
    {0x07DDEC, 0x08C1BE}, // the same entry of a jump table, 7 on alike
    {0x07DE0A, 0x08C1DC}, // the same entry of a jump table, 9 on alike
    {0x07DE2C, 0x08C1FE}, // the same entry of a jump table, 1 on alike
    {0x07DE32, 0x08BA4A}, // the same entry of a jump table, 3 on alike
    {0x07DE46, 0x08BA5E}, // the same entry of a jump table, 3 on alike
    {0x07DE78, 0x08BA8C}, // the same entry of a jump table, 9 on alike
    {0x07DEC8, 0x08BADC}, // the same entry of a jump table, 1 on alike
    {0x07DECE, 0x08C204}, // the same entry of a jump table, 3 on alike
    {0x07DEE2, 0x08C218}, // the same entry of a jump table, 3 on alike
    {0x07DEEE, 0x08C224}, // the same entry of a jump table, 11 on alike
    {0x07DF18, 0x08C24E}, // the same entry of a jump table, 1 on alike
    {0x07DF1E, 0x08C254}, // the same entry of a jump table, 3 on alike
    {0x07DF32, 0x08C268}, // the same entry of a jump table, 6 on alike
    {0x07DF4C, 0x08C282}, // the same entry of a jump table, 12 on alike
    {0x07DF96, 0x08C2CC}, // the same entry of a jump table, 1 on alike
    {0x07DF9C, 0x08C2D2}, // the same entry of a jump table, 3 on alike
    {0x07DFB0, 0x08C2E6}, // the same entry of a jump table, 24 on alike
    {0x07E040, 0x08C376}, // the same entry of a jump table, 5 on alike
    {0x07E058, 0x08C38E}, // a place the code names so (2/2)
    {0x07E05E, 0x08C394}, // an instruction, 12 on alike
    {0x07E0A2, 0x08C3D8}, // an instruction, 5 on alike
    {0x07E0BA, 0x08C3F0}, // an instruction, 7 on alike
    {0x07E108, 0x08C43E}, // the same entry of a jump table, 1 on alike
    {0x07E10E, 0x08C444}, // the same entry of a jump table, 3 on alike
    {0x07E122, 0x08C458}, // the same entry of a jump table, 3 on alike
    {0x07E12E, 0x08C464}, // the same entry of a jump table, 12 on alike
    {0x07E15C, 0x08C492}, // the same entry of a jump table, 1 on alike
    {0x07E162, 0x08C498}, // the same entry of a jump table, 3 on alike
    {0x07E176, 0x08C4AC}, // the same entry of a jump table, 17 on alike
    {0x07E1D6, 0x08C50C}, // the same entry of a jump table, 12 on alike
    {0x07E208, 0x08C53E}, // the same entry of a jump table, 1 on alike
    {0x07E20E, 0x08C544}, // the same entry of a jump table, 3 on alike
    {0x07E222, 0x08C558}, // the same entry of a jump table, 24 on alike
    {0x07E2A8, 0x08C5DE}, // the same entry of a jump table, 19 on alike
    {0x07E2F6, 0x08C62C}, // the same entry of a jump table, 1 on alike
    {0x07E360, 0x08C696}, // a place the code names so (1/1)
    {0x07E3E0, 0x08C716}, // a place the code names so (1/1)
    {0x07E4E0, 0x08C816}, // a place the code names so (1/1)
    {0x07E5A8, 0x08C8DE}, // the same entry of a jump table, 3 on alike
    {0x07E5BC, 0x08C8F2}, // the same entry of a jump table, 7 on alike
    {0x07E5D6, 0x08C90C}, // the same entry of a jump table, 12 on alike
    {0x07E604, 0x08C93A}, // the same entry of a jump table, 1 on alike
    {0x07E62C, 0x08C962}, // the same entry of a jump table, 3 on alike
    {0x07E640, 0x08C976}, // the same entry of a jump table, 10 on alike
    {0x07E672, 0x08C9A8}, // an instruction, 13 on alike
    {0x07E6A4, 0x08C9DA}, // the same entry of a jump table, 3 on alike
    {0x07E6BC, 0x08C9F2}, // a place the code names so (2/2)
    {0x07E6C0, 0x08C9F6}, // an instruction, 10 on alike
    {0x07E746, 0x08CA7C}, // an instruction, 14 on alike
    {0x07E778, 0x08CAAE}, // the same entry of a jump table, 1 on alike
    {0x07E77E, 0x08CAB4}, // an instruction, 14 on alike
    {0x07E7B4, 0x08CAEA}, // the same bytes, once only
    {0x07E832, 0x08CB68}, // the same bytes, once only
    {0x07E8B0, 0x08CBE6}, // the same bytes, once only
    {0x07E9A8, 0x08CCDE}, // the same bytes, once only
    {0x07EA82, 0x08CDB8}, // the same entry of a jump table, 3 on alike
    {0x07EA96, 0x08CDCC}, // the same entry of a jump table, 12 on alike
    {0x07EAD8, 0x08CE0E}, // the same entry of a jump table, 3 on alike
    {0x07EAF0, 0x08CE26}, // a place the code names so (2/2)
    {0x07EAF6, 0x08CE2C}, // an instruction, 5 on alike
    {0x07EB0A, 0x08CE40}, // an instruction, 9 on alike
    {0x07EB36, 0x08CE6C}, // an instruction, 4 on alike
    {0x07EB42, 0x08CE78}, // the same entry of a jump table, 1 on alike
    {0x07EB48, 0x08CE7E}, // the same bytes, once only
    {0x07EB88, 0x08CEBE}, // the same bytes, once only
    {0x07EBAA, 0x08CEE0}, // the same bytes, once only
    {0x07EBEA, 0x08CF20}, // the same entry of a jump table, 3 on alike
    {0x07EBFE, 0x08CF34}, // the same entry of a jump table, 3 on alike
    {0x07EC0A, 0x08CF40}, // the same entry of a jump table, 22 on alike
    {0x07EC6E, 0x08CFA4}, // the same entry of a jump table, 24 on alike
    {0x07ED14, 0x08D04A}, // a place the code names so (1/1)
    {0x07ED16, 0x08D04C}, // a place the code names so (1/1)
    {0x07ED2C, 0x08D062}, // the same entry of a jump table, 1 on alike
    {0x07ED32, 0x08D068}, // the same bytes, once only
    {0x07ED42, 0x08D078}, // a place the code names so (1/1)
    {0x07ED44, 0x08D07A}, // the same bytes, once only
    {0x07ED52, 0x08D088}, // the same bytes, once only
    {0x07ED64, 0x08D09A}, // a table between two that moved alike (other bytes)
    {0x07ED72, 0x08D0A8}, // the same entry of a jump table, 3 on alike
    {0x07ED86, 0x08D0BC}, // the same entry of a jump table, 12 on alike
    {0x07EDBA, 0x08D0F0}, // the same entry of a jump table, 6 on alike
    {0x07EDD8, 0x08D10E}, // the same entry of a jump table, 1 on alike
    {0x07EDDE, 0x08D114}, // the same entry of a jump table, 3 on alike
    {0x07EDF2, 0x08D128}, // the same entry of a jump table, 23 on alike
    {0x07EE54, 0x08D18A}, // a place the code names so (1/1)
    {0x07EE56, 0x08D18C}, // a place the code names so (1/1)
    {0x07EE64, 0x08D19A}, // the same entry of a jump table, 9 on alike
    {0x07EE8A, 0x08D1C0}, // the same entry of a jump table, 1 on alike
    {0x07EE90, 0x08D1C6}, // the same entry of a jump table, 3 on alike
    {0x07EEA4, 0x08D1DA}, // the same entry of a jump table, 4 on alike
    {0x07EEB6, 0x08D1EC}, // the same entry of a jump table, 12 on alike
    {0x07EEE6, 0x08D21C}, // the same entry of a jump table, 1 on alike
    {0x07EEEC, 0x08D222}, // the same entry of a jump table, 3 on alike
    {0x07EF00, 0x08D236}, // the same entry of a jump table, 3 on alike
    {0x07EF0C, 0x08D242}, // the same entry of a jump table, 5 on alike
    {0x07EF22, 0x08D258}, // the same entry of a jump table, 17 on alike
    {0x07EF76, 0x08D2AC}, // the same entry of a jump table, 1 on alike
    {0x07EF7C, 0x08D2B2}, // the same entry of a jump table, 3 on alike
    {0x07EF90, 0x08D2C6}, // the same entry of a jump table, 19 on alike
    {0x07EFE8, 0x08D31E}, // a place the code names so (1/1)
    {0x07EFE9, 0x08D31F}, // a place the code names so (1/1)
    {0x07EFF0, 0x08D326}, // a place the code names so (1/1)
    {0x07EFF2, 0x08D328}, // the same bytes, once only
    {0x07F030, 0x08D366}, // the same entry of a jump table, 6 on alike
    {0x07F04C, 0x08D382}, // the same entry of a jump table, 1 on alike
    {0x07F052, 0x08D388}, // the same entry of a jump table, 3 on alike
    {0x07F066, 0x08D39C}, // the same entry of a jump table, 6 on alike
    {0x07F07C, 0x08D3B2}, // the same entry of a jump table, 5 on alike
    {0x07F09A, 0x08D3D0}, // a place the code names so (2/2)
    {0x07F0A6, 0x08D3DC}, // an instruction, 24 on alike
    {0x07F126, 0x08D45C}, // an instruction, 11 on alike
    {0x07F150, 0x08D486}, // an instruction, 6 on alike
    {0x07F168, 0x08D49E}, // the same bytes, once only
    {0x07F19E, 0x08D4D4}, // the same entry of a jump table, 1 on alike
    {0x07F1A4, 0x08D4DA}, // the same entry of a jump table, 3 on alike
    {0x07F1B8, 0x08D4EE}, // the same entry of a jump table, 4 on alike
    {0x07F1C8, 0x08D4FE}, // the same entry of a jump table, 12 on alike
    {0x07F1FC, 0x08D532}, // the same entry of a jump table, 1 on alike
    {0x07F202, 0x08D538}, // the same entry of a jump table, 3 on alike
    {0x07F216, 0x08D54C}, // the same entry of a jump table, 11 on alike
    {0x07F26A, 0x08D5A0}, // the same entry of a jump table, 24 on alike
    {0x07F2EA, 0x08D620}, // the same entry of a jump table, 1 on alike
    {0x07F2F0, 0x08D626}, // the same entry of a jump table, 3 on alike
    {0x07F304, 0x08D63A}, // the same entry of a jump table, 24 on alike
    {0x07F3A2, 0x08D6D8}, // the same entry of a jump table, 24 on alike
    {0x07F432, 0x08D768}, // the same entry of a jump table, 1 on alike
    {0x07F438, 0x08D76E}, // the same entry of a jump table, 3 on alike
    {0x07F44C, 0x08D782}, // the same entry of a jump table, 24 on alike
    {0x07F546, 0x08D87C}, // the same entry of a jump table, 3 on alike
    {0x07F552, 0x08D888}, // a place the code names so (2/2)
    {0x07F55A, 0x08D890}, // the same entry of a jump table, 5 on alike
    {0x07F56A, 0x08D8A0}, // the same entry of a jump table, 15 on alike
    {0x07F5A8, 0x08D8DE}, // the same entry of a jump table, 11 on alike
    {0x07F5DA, 0x08D910}, // the same entry of a jump table, 20 on alike
    {0x07F634, 0x08D96A}, // the same entry of a jump table, 1 on alike
    {0x07F63A, 0x08D970}, // a place the code names so (4/4)
    {0x07F79E, 0x08DAD4}, // the same entry of a jump table, 3 on alike
    {0x07F7B2, 0x08DAE8}, // the same entry of a jump table, 16 on alike
    {0x07F808, 0x08DB3E}, // the same entry of a jump table, 4 on alike
    {0x07F818, 0x08DB4E}, // a place the code names so (2/2)
    {0x07F81C, 0x08DB52}, // the same entry of a jump table, 4 on alike
    {0x07F828, 0x08DB5E}, // the same entry of a jump table, 12 on alike
    {0x07F86A, 0x08DBA0}, // the same entry of a jump table, 1 on alike
    {0x07F870, 0x08DBA6}, // the same entry of a jump table, 3 on alike
    {0x07F884, 0x08DBBA}, // the same entry of a jump table, 4 on alike
    {0x07F896, 0x08DBCC}, // the same entry of a jump table, 7 on alike
    {0x07F8B8, 0x08DBEE}, // a place the code names so (2/2)
    {0x07F8BC, 0x08DBF2}, // an instruction, 11 on alike
    {0x07F8EA, 0x08DC20}, // an instruction, 15 on alike
    {0x07F922, 0x08DC58}, // the same entry of a jump table, 1 on alike
    {0x07F928, 0x08DC5E}, // the same entry of a jump table, 3 on alike
    {0x07F93C, 0x08DC72}, // the same entry of a jump table, 6 on alike
    {0x07F95C, 0x08DC92}, // the same entry of a jump table, 5 on alike
    {0x07F97A, 0x08DCB0}, // a place the code names so (2/2)
    {0x07F97E, 0x08DCB4}, // an instruction, 5 on alike
    {0x07F9B2, 0x08DCE8}, // an instruction, 11 on alike
    {0x07F9DE, 0x08DD14}, // the same entry of a jump table, 1 on alike
    {0x07F9E4, 0x08DD1A}, // the same entry of a jump table, 3 on alike
    {0x07F9F8, 0x08DD2E}, // the same entry of a jump table, 9 on alike
    {0x07FA1C, 0x08DD52}, // the same entry of a jump table, 9 on alike
    {0x07FA6A, 0x08DDA0}, // the same entry of a jump table, 4 on alike
    {0x07FA7C, 0x08DDB2}, // the same entry of a jump table, 1 on alike
    {0x07FA82, 0x08DDB8}, // the same entry of a jump table, 3 on alike
    {0x07FA96, 0x08DDCC}, // the same entry of a jump table, 13 on alike
    {0x07FADA, 0x08DE10}, // the same entry of a jump table, 9 on alike
    {0x07FAFC, 0x08DE32}, // the same entry of a jump table, 1 on alike
    {0x07FB02, 0x08DE38}, // the same entry of a jump table, 3 on alike
    {0x07FB16, 0x08DE4C}, // the same entry of a jump table, 3 on alike
    {0x07FB22, 0x08DE58}, // the same entry of a jump table, 13 on alike
    {0x07FB58, 0x08DE8E}, // the same entry of a jump table, 1 on alike
    {0x07FB5E, 0x08DE94}, // the same entry of a jump table, 3 on alike
    {0x07FB72, 0x08DEA8}, // the same entry of a jump table, 4 on alike
    {0x07FB82, 0x08DEB8}, // the same entry of a jump table, 10 on alike
    {0x07FBAC, 0x08DEE2}, // the same entry of a jump table, 4 on alike
    {0x07FBBE, 0x08DEF4}, // the same entry of a jump table, 1 on alike
    {0x07FBC4, 0x08DEFA}, // the same entry of a jump table, 3 on alike
    {0x07FBD8, 0x08DF0E}, // the same entry of a jump table, 4 on alike
    {0x07FBE8, 0x08DF1E}, // the same entry of a jump table, 7 on alike
    {0x07FC06, 0x08DF3C}, // the same entry of a jump table, 1 on alike
    {0x07FC0C, 0x08DF42}, // the same entry of a jump table, 3 on alike
    {0x07FC20, 0x08DF56}, // the same entry of a jump table, 12 on alike
    {0x07FC56, 0x08DF8C}, // the same entry of a jump table, 24 on alike
    {0x07FC6E, 0x08DFA4}, // the same entry of a jump table, 18 on alike
    {0x07FCB6, 0x08DFEC}, // the same entry of a jump table, 1 on alike
    {0x07FCF0, 0x08E026}, // the same entry of a jump table, 3 on alike
    {0x07FD04, 0x08E03A}, // the same entry of a jump table, 15 on alike
    {0x07FD4A, 0x08E080}, // the same entry of a jump table, 10 on alike
    {0x07FE00, 0x08E136}, // the same entry of a jump table, 1 on alike
    {0x07FE06, 0x08E13C}, // the same entry of a jump table, 3 on alike
    {0x07FE1A, 0x08E150}, // the same entry of a jump table, 3 on alike
    {0x07FE26, 0x08E15C}, // the same entry of a jump table, 11 on alike
    {0x07FE54, 0x08E18A}, // the same entry of a jump table, 1 on alike
    {0x07FE5A, 0x08E190}, // the same entry of a jump table, 3 on alike
    {0x07FE6E, 0x08E1A4}, // the same entry of a jump table, 5 on alike
    {0x07FE84, 0x08E1BA}, // the same entry of a jump table, 5 on alike
    {0x07FEA6, 0x08E1DC}, // a place the code names so (2/2)
    {0x07FEAA, 0x08E1E0}, // an instruction, 8 on alike
    {0x07FECA, 0x08E200}, // an instruction, 17 on alike
    {0x07FF1A, 0x08E250}, // the same entry of a jump table, 5 on alike
    {0x07FF30, 0x08E266}, // the same entry of a jump table, 1 on alike
    {0x07FF76, 0x08E2AC}, // the same entry of a jump table, 3 on alike
    {0x07FF8A, 0x08E2C0}, // the same entry of a jump table, 4 on alike
    {0x07FF9A, 0x08E2D0}, // the same entry of a jump table, 4 on alike
    {0x07FFAE, 0x08E2E4}, // the same entry of a jump table, 1 on alike
    {0x07FFB4, 0x08E2EA}, // a table between two that moved alike (other bytes)
    {0x07FFDE, 0x08E314}, // the same entry of a jump table, 3 on alike
    {0x07FFF2, 0x08E328}, // the same entry of a jump table, 6 on alike
    {0x08000E, 0x08E344}, // the same entry of a jump table, 12 on alike
    {0x08003C, 0x08E372}, // the same entry of a jump table, 9 on alike
    {0x080060, 0x08E396}, // the same entry of a jump table, 1 on alike
    {0x080094, 0x08E3CA}, // the same entry of a jump table, 3 on alike
    {0x0800A8, 0x08E3DE}, // the same entry of a jump table, 4 on alike
    {0x0800B8, 0x08E3EE}, // the same entry of a jump table, 7 on alike
    {0x0800D4, 0x08E40A}, // the same entry of a jump table, 4 on alike
    {0x0800E6, 0x08E41C}, // the same entry of a jump table, 1 on alike
    {0x0800EC, 0x08E422}, // the same entry of a jump table, 3 on alike
    {0x080100, 0x08E436}, // the same entry of a jump table, 7 on alike
    {0x08011C, 0x08E452}, // the same entry of a jump table, 24 on alike
    {0x0801CE, 0x08E504}, // the same entry of a jump table, 6 on alike
    {0x0801EC, 0x08E522}, // the same entry of a jump table, 1 on alike
    {0x0801F2, 0x08E528}, // a place the code names so (1/1)
    {0x080378, 0x08E6AE}, // the same entry of a jump table, 3 on alike
    {0x08038C, 0x08E6C2}, // the same entry of a jump table, 5 on alike
    {0x0803A4, 0x08E6DA}, // the same entry of a jump table, 16 on alike
    {0x0803E2, 0x08E718}, // the same entry of a jump table, 1 on alike
    {0x0803E8, 0x08E71E}, // the same entry of a jump table, 3 on alike
    {0x0803FC, 0x08E732}, // the same entry of a jump table, 5 on alike
    {0x080414, 0x08E74A}, // the same entry of a jump table, 21 on alike
    {0x08045C, 0x08E792}, // the same entry of a jump table, 1 on alike
    {0x080462, 0x08E798}, // the same entry of a jump table, 3 on alike
    {0x080476, 0x08E7AC}, // the same entry of a jump table, 5 on alike
    {0x08048E, 0x08E7C4}, // the same entry of a jump table, 18 on alike
    {0x0804D2, 0x08E808}, // the same entry of a jump table, 1 on alike
    {0x0804D8, 0x08E80E}, // the same bytes, once only
    {0x0805A4, 0x08E8DA}, // the same entry of a jump table, 3 on alike
    {0x0806FA, 0x08EA30}, // the same entry of a jump table, 3 on alike
    {0x08070E, 0x08EA44}, // the same entry of a jump table, 20 on alike
    {0x080724, 0x08EA5A}, // the same entry of a jump table, 15 on alike
    {0x080762, 0x08EA98}, // the same entry of a jump table, 1 on alike
    {0x080768, 0x08EA9E}, // the same entry of a jump table, 3 on alike
    {0x08077C, 0x08EAB2}, // the same entry of a jump table, 10 on alike
    {0x080792, 0x08EAC8}, // the same entry of a jump table, 5 on alike
    {0x0807A8, 0x08EADE}, // the same entry of a jump table, 4 on alike
    {0x0807BA, 0x08EAF0}, // the same entry of a jump table, 1 on alike
    {0x0807C0, 0x08EAF6}, // the same entry of a jump table, 3 on alike
    {0x0807D4, 0x08EB0A}, // the same entry of a jump table, 21 on alike
    {0x0807F4, 0x08EB2A}, // the same entry of a jump table, 14 on alike
    {0x080834, 0x08EB6A}, // the same entry of a jump table, 1 on alike
    {0x08083A, 0x08EB70}, // the same entry of a jump table, 4 on alike
    {0x08084A, 0x08EB80}, // a place the code names so (2/2)
    {0x080852, 0x08EB88}, // the same entry of a jump table, 7 on alike
    {0x080864, 0x08EB9A}, // the same entry of a jump table, 3 on alike
    {0x080876, 0x08EBAC}, // the same entry of a jump table, 1 on alike
    {0x08087C, 0x08EBB2}, // the same entry of a jump table, 3 on alike
    {0x080890, 0x08EBC6}, // the same entry of a jump table, 23 on alike
    {0x0808A6, 0x08EBDC}, // the same entry of a jump table, 18 on alike
    {0x08096A, 0x08ECA0}, // the same entry of a jump table, 1 on alike
    {0x080970, 0x08ECA6}, // the same entry of a jump table, 3 on alike
    {0x080984, 0x08ECBA}, // the same entry of a jump table, 7 on alike
    {0x0809A6, 0x08ECDC}, // the same entry of a jump table, 7 on alike
    {0x0809DE, 0x08ED14}, // the same entry of a jump table, 1 on alike
    {0x0809E4, 0x08ED1A}, // the same entry of a jump table, 3 on alike
    {0x0809F8, 0x08ED2E}, // the same entry of a jump table, 9 on alike
    {0x080A20, 0x08ED56}, // the same entry of a jump table, 10 on alike
    {0x080A7A, 0x08EDB0}, // the same entry of a jump table, 1 on alike
    {0x09190A, 0x09C0E2}, // an instruction, 10 on alike
    {0x09192A, 0x09C102}, // an instruction, 7 on alike
    {0x0B7640, 0x0D0D50}, // a place the code names so (3/3)
    {0x0B7740, 0x0D0E50}, // a place the code names so (1/1)
    {0x0B7940, 0x0D1050}, // a place the code names so (2/2)
    {0x0BCE7A, 0x0D68AA}, // a place the code names so (2/2)
    {0x0BCEFA, 0x0D692A}, // a place the code names so (1/1)
    {0x0BCF7A, 0x0D69AA}, // a place the code names so (4/4)
    {0x100000, 0x100000}, // a place the code names so (18/18)
    {0x2776B0, 0x2981FC}, // a place the code names so (2/2)
    {0x357CA4, 0x348C94}, // a place the code names so (3/3)
    {0x35AD18, 0x34BD08}, // a place the code names so (2/2)
    {0x35C918, 0x34D288}, // a place the code names so (2/2)
    {0x38C198, 0x376B68}, // a place the code names so (5/5)
    {0x38C218, 0x376BE8}, // a place the code names so (4/4)
    {0x38C298, 0x376C68}, // a place the code names so (3/3)
    {0x3A0B20, 0x397FB0}, // the same bytes, once only
    {0x3A1420, 0x3989F0}, // the same bytes, once only
    {0x3A1660, 0x398C30}, // a table between two that moved alike (the same bytes)
    {0x3A1680, 0x398C50}, // a table between two that moved alike (the same bytes)
    {0x3A16A0, 0x398C70}, // the same bytes, once only
    {0x3A2640, 0x399C10}, // the same bytes, once only
    {0x3A2F40, 0x39A650}, // the same bytes, once only
    {0x3A3180, 0x39A890}, // a table between two that moved alike (the same bytes)
    {0x3A31A0, 0x39A8B0}, // a table between two that moved alike (the same bytes)
    {0x3A31C0, 0x39A8D0}, // the same bytes, once only
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
    {0xFF80D4, 0xFF80D4}, // the random number (battle.cpp): 0xD4(a5) in this game's routine of it too (013584) by hand
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
// what this set calls them and, if another, what the American and European sets do. Night Warriors' (numbers
// seen in the attract mode's fights and the character choice's code)
struct Character {
    uint8_t number;
    const char *name, *otherName;
};
inline constexpr Character kRoster[] = {
    {0x01, "Demitri", nullptr},  {0x02, "Gallon", "Talbain"},  {0x03, "Victor", nullptr},   {0x04, "Zabel", "Lord Raptor"},
    {0x05, "Morrigan", nullptr}, {0x06, "Anakaris", nullptr},  {0x07, "Felicia", nullptr},  {0x08, "Bishamon", nullptr},
    {0x09, "Aulbath", "Rikuo"},  {0x0A, "Sasquatch", nullptr}, {0x0D, "Lei-Lei", "Hsien-Ko"}, {0x10, "Phobos", "Huitzil"},
    {0x11, "Pyron", nullptr},    {0x13, "Donovan", nullptr},
};
// The hidden ones (src/choosing.cpp). This game's own choosing code (not rewritten: its hidden characters differ)
// gives Oboro Bishamon as 19 (Bishamon chosen with Start held and a button: 01F838), Shadow (Start five times on the
// "?" box: 01F8BC) and Marionette (seven times: 01F928), and Phobos and Donovan another way with Start held (the
// byte at +3C2). Dark Gallon (12) is in its tables but no code chooses him; 18 is Oboro Bishamon as the last
// opponent. The "Hidden characters" feature hooks that code: where it stores the colours (01F7D8) and where it
// counts Shadow's code
inline constexpr Character kHidden[] = {{0x12, "Dark Gallon", "Dark Talbain"},
                                        {0x19, "Oboro Bishamon", nullptr},
                                        {0x18, "Oboro Bishamon (the boss)", nullptr}};
inline constexpr uint8_t kDarkGallon = 0x12, kOboroBishamon = 0x19;
inline constexpr uint32_t kHiddenAtColours = 0x01F7D8, kHiddenShadow = 0x01F8BC;
inline constexpr const char *kHiddenWords =
    "Choosing Gallon gives Dark Gallon (whom no code on the board chooses) and choosing Bishamon gives Oboro Bishamon "
    "(on the board: Start held as he is chosen); a punch chooses their first colours, a kick their second. The \"?\" "
    "box gives Shadow, who becomes each opponent beaten (on the board: Start five times on it). For player 1, player 2 "
    "or both; the alternative modes' fighters too. (Also on the board: Start seven times on the box for Marionette, "
    "and Start held as Phobos or Donovan is chosen for their other way.)";

// The stages, in the order of their numbers (the word at FF8100 is the number doubled)
inline constexpr const char *kStageNames[] = {
    "Feast of the Damned", "Concrete Cave", "Tower of Arrogance", "Red Thirst", "Deserted Chateau", "Abaraya",
    "Vanity Paradise", "War Agony", "Forever Torment", "Green Scream", "Iron Horse, Iron Terror", "Fetus of God",
    "Feast of the Damned, empty",
};

// The frame's lists of objects to draw that the objects of kind 12 have, one for each depth (src/picture.cpp): one more
// than Vampire Savior's (the table at 013D0C), so that the stage's pieces' lists come one later
inline constexpr int kTwelveLists = 8;

// Each stage's own tune (src/modes.cpp: the stage viewer's music). This game plays a character's theme, not a
// stage's (09 and the character's number): a stage's is the theme of the character it belongs to (whose row of the
// table of stages by character, 00ACC2, has it most), Iron Horse's and Fetus of God's none (nobody's)
inline constexpr uint16_t kStageTunes[] = {0x0A, 0x0B, 0x1A, 0x0F, 0x0E, 0x11, 0x16, 0x19, 0x0D, 0x12, 0, 0, 0x1C};

// Its music, for a custom soundtrack (src/music.cpp): the sound program keeps a table of its songs at 9000 (a word,
// how many; song 0's entry four bytes on). Which are music, from that table (their data starts with a 0); what each
// is, from the arcade ladder played to its end with a character held, attract mode and the stages
// (CPS2RECOMP_SOUNDTRACK_LOG)
struct Song {
    uint16_t number;
    const char *name, *title;
    bool once = false;
};
inline constexpr uint32_t kSongTable = 0x9006, kSongCount = 0xA5B;
inline constexpr Song kSongs[] = {
    // Around the fights (as Vampire Savior has them: the character select, the versus screen, game over seen)
    {0x01, "opening", "The opening", true},
    {0x02, "character_select", "Character select"},
    {0x03, "next", "Between a win and the next versus screen"},
    {0x05, "versus", "Versus screen", true},
    {0x06, "continue", "Continue?"},
    {0x07, "game_over", "Game over", true},
    {0x08, "ranking", "Score ranking", true},
    // Each character's theme, played in a fight against them (09 and the number; kStageTunes)
    {0x0A, "demitri_theme", "Demitri's theme"},
    {0x0B, "gallon_theme", "Gallon's theme"},
    {0x0C, "victor_theme", "Victor's theme"},
    {0x0D, "zabel_theme", "Zabel's theme"},
    {0x0E, "morrigan_theme", "Morrigan's theme"},
    {0x0F, "anakaris_theme", "Anakaris's theme"},
    {0x10, "felicia_theme", "Felicia's theme"},
    {0x11, "bishamon_theme", "Bishamon's theme"},
    {0x12, "aulbath_theme", "Aulbath's theme"},
    {0x13, "sasquatch_theme", "Sasquatch's theme"},
    {0x16, "leilei_theme", "Lei-Lei's theme"},
    {0x19, "phobos_theme", "Phobos's theme"},
    {0x1A, "pyron_theme", "Pyron's theme"},
    {0x1C, "donovan_theme", "Donovan's theme"},
    {0x3B, "rival", "Before the last fight (the rival)"},
    {0xEA, "pyron_appears", "Pyron appears", true},
    // After Pyron is beaten: the ending, then one of two ways on
    {0x3D, "ending", "The ending"},
    {0x3E, "ending_2", "The ending, on (one way)"},
    {0x3F, "ending_3", "The ending, on (the other way)"},
    // The winner's, on the screen after a win (22 and the character's number)
    {0x23, "demitri_win", "Demitri wins"},
    {0x24, "gallon_win", "Gallon wins"},
    {0x25, "victor_win", "Victor wins"},
    {0x26, "zabel_win", "Zabel wins"},
    {0x27, "morrigan_win", "Morrigan wins"},
    {0x28, "anakaris_win", "Anakaris wins"},
    {0x29, "felicia_win", "Felicia wins"},
    {0x2A, "bishamon_win", "Bishamon wins"},
    {0x2B, "aulbath_win", "Aulbath wins"},
    {0x2C, "sasquatch_win", "Sasquatch wins"},
    {0x2F, "leilei_win", "Lei-Lei wins"},
    {0x32, "phobos_win", "Phobos wins"},
    {0x33, "pyron_win", "Pyron wins"},
    {0x35, "donovan_win", "Donovan wins"},
    // Music the game has whose place is not known yet
    {0x04, "music_04", "Where it plays is not known yet"},
    {0x21, "music_21", "Where it plays is not known yet"},
    {0x36, "music_36", "Where it plays is not known yet"},
    {0x37, "music_37", "Where it plays is not known yet"},
    {0x3A, "music_3a", "Where it plays is not known yet"},
    {0x3C, "music_3c", "Where it plays is not known yet"},
    {0x40, "music_40", "Where it plays is not known yet"},
    {0x41, "music_41", "Where it plays is not known yet"},
    {0x4A, "music_4a", "Where it plays is not known yet"},
    {0x4B, "music_4b", "Where it plays is not known yet"},
    {0x4C, "music_4c", "Where it plays is not known yet"},
    {0x4D, "music_4d", "Where it plays is not known yet"},
    {0x4E, "music_4e", "Where it plays is not known yet"},
    {0x4F, "music_4f", "Where it plays is not known yet"},
    {0x50, "music_50", "Where it plays is not known yet"},
    {0x51, "music_51", "Where it plays is not known yet"},
    {0x52, "music_52", "Where it plays is not known yet"},
    {0x53, "music_53", "Where it plays is not known yet"},
    {0x59, "music_59", "Where it plays is not known yet"},
    {0x5A, "music_5a", "Where it plays is not known yet"},
    {0xD0, "music_d0", "Where it plays is not known yet"},
};

// Where the hit test goes on to when an attack has hit (src/collision.cpp): not HitLands itself as in Vampire Savior
// (018230), but a test before it of the victim's guarding pose (whose code goes on into HitLands)
inline constexpr uint32_t kHitTestHit = 0x016AAE;

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

// The characters whose three other palettes go elsewhere when their effects need them, a bit by number (02A136)
inline constexpr uint32_t kFirstLineOnly = 0xEF9AEF96;

// The ROM file whose colours PalMod changes, and which set it is: none, the colours run over from vh2j.09 into vh2j.10
// (376C70-38FB30), so only the text files and the colour editor change them
inline constexpr const char *kColoursFile = nullptr, *kColoursFileWhose = "Vampire Hunter 2's, 970929";

// What this game's own sources add (none: Vampire Savior's sources are all it has)
inline void RegisterOwnSources() {}

// What a character put in by a mode in place of the one chosen needs besides its number (src/training.cpp)
inline void CharacterPut(uint32_t, int) {}


} // namespace vsav::family
