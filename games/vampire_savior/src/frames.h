// SPDX-License-Identifier: GPL-3.0-only
// Copyright (C) 2026 Tommy McLeroy
//
// Vampire Savior: frame data (frames.cpp; the counting is the toolkit's fight module, fight.h).
#pragma once

#include "fight.h"
#include "vsav.h"

#include <vector>

namespace cps2recomp {

// What a player is doing at a step of the game
using Doing = fight::Doing;

void RegisterFrames();
// A hit has landed on a player, or been guarded; what a player is doing (the training session's frame data:
// fight/session.h); a move's start, for the frame data log
void FramesContact(int victim, bool guarded);
Doing FramesDoing(int player);
fight::MoveLog::Start FramesMoveStart(int n);

} // namespace cps2recomp
