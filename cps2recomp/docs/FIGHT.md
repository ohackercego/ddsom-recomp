# Fighting games: the shared training parts

`fight/` holds what a training mode needs that does not depend on the game (`fight/fight.h`); a game's sources
say, from its own memory, what the players are doing, and these keep count and say what to show (the HUD's lines
and boxes, `runtime/game.h`).

| Piece | What it does | What the game tells it |
| --- | --- | --- |
| `Frames` | Startup, active and recovery of each move; the advantage after a hit or a guarded one; a meter of the last 60 steps | Each player once a step (a `StepView`: who is in charge of it, whether it can act, is frozen by a hit, attacking, in the air, has an attack box out), and each hit landing or guarded (`Contact`) |
| `MoveLog` | Every move's numbers as lines of `frame_data.csv` beside settings.ini (character, input, air, which move, startup, active, recovery, advantage) | Each player once a step after `Frames::Step`, and a function that tells a move's start (asked only as one begins) |
| `InputHistory`, `Notation` | What each player holds and for how long, the stick as a keypad number (from the player's side, if the game says which way it faces) | What the game read off the panel each frame |
| `Combos` | The last hit, the combo (hits landing before the player is free again), what of it comes back by itself, the round's best | Each hit's damage, and whether the player was still reeling |
| `Dummy`, `RegisterDummy` | Player 2 stands, crouches, jumps, records and replays (three slots, or any at random), answers with a recording, guards (always, after a hit, at random) high or low; the game's own actions while guarding, while held, getting up | Each frame a `DummyView` (fighting, what it is doing, which side, whether an attack is near enough to guard and must be guarded standing, whether it is knocked down); the game's own actions as `DummyOptions` |
| `StageBox`, `ToPicture` | A box in the stage drawn on the picture through the camera, coloured by its kind | The boxes, read the way the game's own hit tests read them |

The features the dummy registers have the same names in every game (`dummy_stand`, `dummy_guard`, ...), so a
player's `settings.ini` reads the same across the ports.

`runtime/spare.h` holds the machines a game's modes keep as save states and run beside the port's (a stage that
scrolls into the next, the next fight made ready while the last one ends): `RunSpare`, `Promote`, `Take`, `Give`,
and `SetSpareKeeping` for what a game keeps of its own around a spare's frame.

## The Alternative modes page and the training session

Two modules hold what every fighting game's "Alternative modes" page and training mode had as copies of one another
(Vampire Savior's, Street Fighter Alpha 2's): a game fills in a description and registers it, and keeps only its own
facts and hooks.

- `fight/alternative.h` (`modes::`): the page itself and its training, attract mode and stage viewer pages; attract mode
  on a loop (the game says where its loop begins and ends and steers its parts with `GoTo`: `attractStep`); the stage
  viewer and its infinite stage on a spare machine; and what the modes share: getting a machine to a round (`ToRound`),
  a sound asked for (`PlaySound`), the match and the camera a mode wants (`Wanted`, `CameraAt`), what is kept around a
  spare's frame (the game adds its own: `keep`, `fresh`, `putBack`).
- `fight/session.h` (`session::`): the training session. Getting there from switching on, player 1's controls choosing
  for both, the round kept as its fighting begins for "Start over" (the middle or a corner, sides swapped), the stage,
  no clock and no knock-outs (`NoClock`, `NoKnockOut`, for the game's own clock and damage hooks), and the aids of
  `fight.h` put together on the HUD. The game tells it, from its own hooks, each player's step (`Step`, a `StepView`),
  a hit landing (`Contact`) and a hit's damage (`Hit`); the description says where the game keeps the rest (players,
  health, the clock, the camera and its copies, the stage's ends, the panel's words, the boxes, the guard test) and
  what the game adds of its own (its idea of fighting, a refill's conditions, a player's readout, the HUD's places, the
  dummy's extras, its own features beside the toolkit's).

Vampire Savior (and the games built from its sources), Street Fighter Alpha 2 and Super Gem Fighter Mini Mix use both;
moving the first two onto them changed none of their pictures, frame hashes, frame data logs or feature lists.

## The fighting modes

`fight/battle.h`: survival, the endless battle and the ring out, for a game's "Alternative modes" page. A game fills
in a `battle::Description` (its result byte, camera, players' and thrown things' records, health, characters and
stages, and hooks for its own: getting a machine to a round, a sound, what is wanted of the next match, which
objects are the stage's, the panel's and the fighters') and calls `battle::Register`; the menu's pages, commands and
features are the same in every game.

The ring out needs two things of the game's player code, at each player's step of a fight (`RingOutMove`):

- **Flung out** (`kFlungOut`, then `kFalling`): the one knocked past where the game would keep it in
  (`RingOutLetsOut`, asked where the game keeps the players in) is put in the game's knock-down in the air, with
  the game's own animation of being thrown back, speeds up and away, and from then on only flies and animates
  (the game's routine for it not run), so it falls through the floor and out of the picture.
- **Dropping in** (`kDropsIn`): the winner jumps off the old stage; at the top of its jump its record there is handed
  to the next match's machine, where the game puts it in the air as it was (its frame, speeds, steps), at the same
  place in the picture, a picture's height above the floor. That machine's camera is held up by as much
  (`CameraWanted::beyondEnds`) and comes down as the winner falls, the old stage's picture going up and the next
  one's coming in below it, the fighters over both, until the game's own jump lands it.

Every match and stage machine after the first is begun from a machine kept at the character choice with both players
in (the first spare that came there), so the next one is ready in about 500 frames, not 2,000 from switching on.

Street Fighter Alpha 2 makes them in its PlayerFighting rewrite; the Vampire Savior games in their player step hook
(02244C).

## Custom colours and the colour editor

`fight/colours.h` gives a game the players' own colours for its characters and an editor to make them, as
Vampire Savior had them first. A game registers a `colours::Description` (in its `RegisterGame`) and calls
`colours::BeforeFrame` before its viewer's own work each frame; its model viewer calls `EditorBeforeFrame`
(after its round is on, before it shows its animation), `EditorFrame` (after its picture is made),
`EditorShift` (the character drawn that far left) and `EditorAnimation` (the choice to use for "Animation"
while the editor is on), and `EndEditor` when its own Start is chosen.

| The game tells | What for |
| --- | --- |
| `characters` (number, name, the name abroad), `setNames`, `palettesInSet`, `lineNames` | The files' names and sections, the editor's rows |
| `setAddress`, `ranges` | Where a set is in the ROM image (`Program`), and every place of the characters' colours (sets and effects) |
| `romFile`, `romFileAt`, `romFileSize`, `same`, `romFileSwapped`, `romFileWhose` | The ROM file PalMod changes: only its words inside `ranges` are taken (the rest are counted and left), and the file is refused unless `same` is the game's |
| `featureDescription`, `originalHelp` | The feature's words in the menu, and the remark at the head of each file "Write the game's colours" writes |
| `showSet` | The editor's preview: player 1 made to use the set, and where its palettes are in the buffer the board shows from (each frame the set is written there, the brightness kept) |
| `startViewer`, `viewerOn`, `viewerCharacter`, `characterNumber`, `animationStep`, `animationNames`, `backgrounds` | The model viewer the editor works in; it uses the viewer's `model_character`, `model_background` and `model_size` choices |
| `animationForLine` (optional) | The animation that best shows a line of the set, for the Line row to go to (0 standing, -1 none, -2 not known yet: asked again the next frame). Vampire Savior finds them by showing every frame of every animation on a spare machine, a few a frame, and counting the objects in each of the player's palettes |

The files, the same in every game: `palettes/<Character>.txt` (a `[SET]` section a set, up to a line a palette
of 16 `#RRGGBB` or `#RGB` colours, `-` for the game's own; a `#` with no hex digit after it starts a remark),
`palettes/original/` (the game's own, written by the command), `palettes/library/<Character>/<Name N>.txt` (one
set a file). The editor saves a character's file 45 frames after the last change and when it ends, with only
the sets and lines that are not the game's.
