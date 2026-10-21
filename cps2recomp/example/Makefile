# SPDX-License-Identifier: GPL-3.0-only
# Copyright (C) 2026 Tommy McLeroy. Part of cps2recomp.
# A stand-in game with no game data: what CI builds with no ROM set (the program with no translated code,
# and the launcher). The real games are in ../games/ and are built from the top (make GAME=<id>).
GAME ?= example
CR ?= ..
NO_ROM = 1
include $(CR)/build.mk
