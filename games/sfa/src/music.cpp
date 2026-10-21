// SPDX-License-Identifier: GPL-3.0-only
//
// Street Fighter Alpha: its music, for a custom soundtrack (port/soundtrack.h, machine/soundtrack.h). The sound
// program (driver 1.05A) keeps a table of its songs at 0x9006, four bytes a song, as Alpha 2's 1.05C does: the
// word at 0x9000 is how many (0x300), and a song whose data starts with a 0 is music. Its pieces are named by
// number in songs.txt until they are found by playing.
#include "game.h"

namespace cps2recomp {

void RegisterMusic() {
    TheGame().songTable = 0x9006;
    TheGame().songCount = 0x300;
}

} // namespace cps2recomp
