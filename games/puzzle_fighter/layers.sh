# SPDX-License-Identifier: GPL-3.0-only
# Copyright (C) 2026 Tommy McLeroy
#
# The rewrites in layers, from the outside in, for the checks (check_rewrites.sh, check_cover.sh). A rewrite
# is only checked when the game's own code calls it, so each run leaves out the layers outside the one it
# checks (CPS2RECOMP_SKIP_REWRITES): a frame of a round; the round's steps; a frame of both turns; a turn;
# the turn's steps; what the steps call; what those call.
round="RoundFrame"
round2="$round,RoundBegin,RoundPlay,RoundWon,RoundAfter,PlayTurns"
turns="$round2,BothPlayersTurns"
turn="$turns,PlayerTurn,MatchOver,MatchSetUp,SetOpponent,NextOpponent,BeginSetUp,BeginShow,BeginPlay,WonShow,WonSettle,WonSpeak,WonEnd,AfterClear,AfterBonuses,AfterEnd"
steps="$turn,StepNewPair,StepFall,StepCount,StepMakeBig,StepWait,StepMakeBig2,StepMakeBig3,StepFindBreaks,StepBigBonus,ShowBreak,StepJoinBig,StepJoinBig2,StepJoinBig3,StepNoteAttack,StepCountAgain,StepAttacked,StepEnd,StepLost,ShowScore"
inner="$steps,PlanPair,MarkBreaks,ScoreBreak,NextPair,MakeNextPair,FallGems,DropCounterGems,SteerPair,MakePictureCodes,DiamondBreaks,CountField,CheckLost"
inmost="$inner,LevelForMatch,SkillAfterMatch,SkillForChain,SkillAfterContinue,ComputerSteers,TurnLeft,TurnRight,LandPair,SwapPair,UnderPair,ShowChainBanner,ShowTechBonus,ShowAllClear,ShowFirstAttack"
# What comes before a match (select.cpp): a frame of it; then its screens
before="BeforeMatch,Choose,BeforeLeave"
# What the whole program calls (common.cpp), as a pattern: checked with every other rewrite left out
common="RunTasks|RunTasks2|RunTasks3|NewTask|NewTask2|NewTask3|EndTask|EndTask2|EndTask3|ClearThirdPool|Random|ToDecimal|CopyColours|DimColours|DimColours32|DimAllColours|StepAnimation|StartAnimation|SetAct|StartFade|PutTall|ShowStageNumber|HideCharacters|ShowCharactersAgain"
# ... and then without the tasks' frames, which call most of the rest
tasks="RunTasks|RunTasks2|RunTasks3"
