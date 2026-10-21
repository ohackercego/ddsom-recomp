# Roadmap

Where the port is against where it is going: a native, readable program with nothing between the
game's rules and the player's machine.

## Where it stands

| Part | Today | |
| --- | --- | --- |
| The game's 68000 program | Translated ahead of time to C++ (about 32,800 instructions); code no run has reached yet runs in the port's own interpreter | No emulator. No instruction falls back to the interpreter over the ladder to its end, every alternative mode and long runs of play |
| Readable rewrites | 185 routines written by hand as named C++: every rule of the game, the turn and the round, the computer player, and every screen from the choice of mode to game over, each checked call by call | The game's presentation (its tasks: cursors, words, the characters' acts), the title, the demonstrations and the ending are still translated code |
| Sound | The game's sound program (Capcom's QSound driver) as ordinary code, checked write for write against the Z80 running it; the port's own model of what the QSound chip's program does | No "Q1" stereo filter; volume and pan curves are approximations. Nobody has listened to it against a board |
| Picture | The port's own line renderer for the board's scroll screens and objects, shown through SDL's GPU API | A model of the hardware, not of the game's drawing calls |
| Input | The board's ports, as the game's frame interrupt reads them | |
| Settings memory | Read and written directly where the game's own routines do, kept in a file | |
| The ROM set's cipher | MAME's implementation (BSD-3-Clause), the only file here from another emulator | To be written again from the published description of the cipher |

## Next

1. **Hand play on every system**: picture, sound, sticks and pads, speed, Steam Deck's Game Mode,
   Windows; the training mode, the alternative modes and the colour editor in a real window.
2. **Sound by ear** against a board or a trusted recording.
3. **Training mode**: the drop patterns shown; preset fields.
4. **Gem colours for colour-blind players** as sets that come with the port, built on the custom
   colours.
5. **More of the game as readable code**: the ending, the tasks of the screens around a match.
6. **Other releases of the game** (the USA, Japan, Asia and Hispanic sets differ in one program ROM
   and the key).
