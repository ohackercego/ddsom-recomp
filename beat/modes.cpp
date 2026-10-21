// SPDX-License-Identifier: GPL-3.0-only
// Copyright (C) 2026 Tommy McLeroy. Part of cps2recomp.
#include "beat.h"

#include "state.h"

#include <algorithm>
#include <climits>
#include <cstdlib>
#include <cstdio>
#include <fstream>
#include <string>

namespace cps2recomp::beat {

namespace {
    constexpr const char *kModes = "Alternative modes", *kAttract = "Attract mode", *kScore = "Score attack", *kViewer = "Stage viewer";
    constexpr int kStartMost = 9000; // frames after which getting to a game is given up
    constexpr int kShownFor = 6 * 60; // frames a score attack's words stay up

    Description g_d;

    enum class Mode : int32_t { None, Attract, Score, Viewer, Model };
    struct State {
        Mode mode = Mode::None;
        bool starting = false;
        int32_t frames = 0;
        // score attack: a credit put in, the game begun, its score as last seen, and the words shown (frames left)
        bool credited = false, played = false, over = false;
        uint32_t score = 0, best = 0;
        int32_t shown = 0;
        bool newBest = false;
        // stage viewer: the stage, how far along its way the camera is and where (in 256ths of the game's unit; still
        // `approaching` the way from where the game had it), which way it goes by itself, and whether every stage is
        // shown in turn
        int32_t stage = 0, along = 0, cameraX = 0, cameraY = 0, direction = 1, chosen = -1;
        bool approaching = false, infinite = false;
    } g;
    bool g_fast = false;

    void Fast(bool on) {
        if (on != g_fast) {
            g_fast = on;
            SetFastForward(on);
        }
    }

    void End() {
        if (g.mode == Mode::None) {
            return;
        }
        if (g.mode == Mode::Model) {
            EndModel();
        }
        g = State();
        Fast(false);
        SetHud({});
    }

    // --- the best score, beside settings.ini ---

    std::string BestPath() {
        const std::string &settings = SettingsPath();
        const size_t slash = settings.find_last_of('/');
        return (slash == std::string::npos ? std::string(".") : settings.substr(0, slash)) + "/score_attack.txt";
    }
    uint32_t ReadBest() {
        std::ifstream file(BestPath());
        std::string line;
        while (std::getline(file, line)) {
            unsigned long value = 0;
            if (std::sscanf(line.c_str(), "best = %lu", &value) == 1) {
                return static_cast<uint32_t>(value);
            }
        }
        return 0;
    }
    void WriteBest(uint32_t best) {
        if (SettingsPath().empty()) {
            return; // (a test run with no settings: nothing kept)
        }
        std::ofstream file(BestPath());
        file << "# The best score of the in-game menu's score attack\nbest = " << best << "\n";
    }

    std::string Grouped(uint32_t value) {
        std::string digits = std::to_string(value), text;
        for (size_t n = 0; n < digits.size(); n++) {
            if (n && (digits.size() - n) % 3 == 0) {
                text += ',';
            }
            text += digits[n];
        }
        return text;
    }

    // --- the modes ---

    void StartAttract(Machine &machine) {
        End();
        g.mode = Mode::Attract;
        g.starting = true;
        machine.Reset();
    }

    void StartScore(Machine &machine) {
        End();
        g.mode = Mode::Score;
        g.starting = true;
        g.best = ReadBest();
        machine.Reset();
    }

    // --- the stage viewer ---

    constexpr int32_t kStick = 3 * 256;                         // (a frame, in 256ths)
    constexpr int32_t kScroll[] = {0, 128, 256, 512};           // with the stick, slow, medium, fast
    bool HideAll(uint32_t, uint16_t) {
        return true;
    }
    int Stages() {
        return static_cast<int>(g_d.stages.size());
    }

    void StartStage(Machine &machine, int stage) {
        const int chosen = FeatureValue("viewer_stage");
        End();
        g.mode = Mode::Viewer;
        g.starting = true;
        g.chosen = chosen;
        g.infinite = chosen >= Stages();
        g.stage = stage;
        machine.Reset();
    }
    void StartViewer(Machine &machine) {
        const int chosen = FeatureValue("viewer_stage");
        StartStage(machine, chosen >= Stages() ? 0 : chosen);
    }

    // The stage's way (its places in 256ths), its length (a step counted by the larger of across and up and down, so
    // that neither goes faster than the speed), and the place a length along it is
    std::vector<Description::Place> g_path;
    void LoadPath() {
        std::vector<Description::Place> path;
        g_d.stagePath(g.stage, path);
        g_path.clear();
        for (const Description::Place &place : path) {
            g_path.push_back({place.x * 256, place.y < 0 ? -1 : place.y * 256});
        }
    }
    int32_t PathLength() {
        int32_t length = 0;
        for (size_t n = 1; n < g_path.size(); n++) {
            length += std::max(std::abs(g_path[n].x - g_path[n - 1].x), std::abs(g_path[n].y - g_path[n - 1].y));
        }
        return length;
    }
    Description::Place PathAt(int32_t along) {
        for (size_t n = 1; n < g_path.size(); n++) {
            const Description::Place a = g_path[n - 1], b = g_path[n];
            const int32_t step = std::max(std::abs(b.x - a.x), std::abs(b.y - a.y));
            if (along <= step || n + 1 == g_path.size()) {
                const int32_t t = step ? std::clamp(along, 0, step) : 0;
                const auto part = [&](int from, int to) { return from + static_cast<int>(step ? (to - from) * static_cast<int64_t>(t) / step : 0); };
                return {part(a.x, b.x), a.y < 0 ? -1 : part(a.y, b.y)};
            }
            along -= step;
        }
        return g_path.empty() ? Description::Place{-1, -1} : g_path.front();
    }
    // How far along the way the place nearest (x, y) is (y -1: by x alone)
    int32_t Nearest(int32_t x, int32_t y) {
        int32_t best = 0, bestDistance = INT32_MAX, along = 0;
        for (size_t n = 1; n < g_path.size(); n++) {
            const Description::Place a = g_path[n - 1], b = g_path[n];
            const int32_t step = std::max(std::abs(b.x - a.x), std::abs(b.y - a.y));
            for (int32_t t = 0; t <= step; t += 256) {
                const Description::Place p = PathAt(along + t);
                const int32_t distance = std::abs(p.x - x) + (y < 0 || p.y < 0 ? 0 : std::abs(p.y - y));
                if (distance < bestDistance) {
                    bestDistance = distance;
                    best = along + t;
                }
            }
            along += step;
        }
        return best;
    }
    int32_t Toward(int32_t from, int32_t to) {
        return from + std::clamp(to - from, -kStick, kStick);
    }

    // (the stage and the character viewers: unseen to the stage; true once there)
    bool ToStage(Machine &machine) {
        Fast(true);
        for (uint16_t &held : machine.players) {
            held = 0;
        }
        if (g_d.toStage(machine, g.stage, g.frames)) {
            g.starting = false;
            Fast(false);
            return true;
        }
        if (++g.frames > kStartMost) {
            End();
        }
        return false;
    }

    void ViewerBeforeFrame(Machine &machine) {
        if (g.starting) {
            if (ToStage(machine)) {
                LoadPath();
                int x = 0, y = 0;
                g_d.stageCamera(x, y);
                g.cameraX = x * 256;
                g.cameraY = y * 256;
                g.along = Nearest(g.cameraX, g_path.empty() || g_path[0].y < 0 ? -1 : g.cameraY);
                g.approaching = true;
                g.direction = 1;
            }
            return;
        }
        uint16_t stick = 0;
        for (uint16_t &held : machine.players) {
            stick |= held & (kLeft | kRight);
            held = 0;
        }
        if (g_path.size() < 2) {
            g_d.view(machine, g.stage, -1, -1);
            return;
        }
        Description::Place at = PathAt(g.along);
        if (g.approaching) {
            // (from where the game had it, onto the way)
            g.cameraX = Toward(g.cameraX, at.x);
            g.cameraY = at.y < 0 ? -1 : Toward(g.cameraY, at.y);
            g.approaching = g.cameraX != at.x || g.cameraY != at.y;
            g_d.view(machine, g.stage, g.cameraX / 256, g.cameraY < 0 ? -1 : g.cameraY / 256);
            return;
        }
        const int scroll = std::clamp(FeatureValue("viewer_scroll"), 0, 3);
        const int32_t length = PathLength();
        if (scroll == 0) {
            g.along += stick == kRight ? kStick : stick == kLeft ? -kStick : 0;
        } else {
            g.along += g.direction * kScroll[scroll];
        }
        if (g.along >= length) {
            g.along = length;
            g.direction = -1;
            if (g.infinite && scroll != 0) {
                StartStage(machine, (g.stage + 1) % Stages());
                g.infinite = true;
                return;
            }
        } else if (g.along <= 0) {
            g.along = 0;
            g.direction = 1;
        }
        at = PathAt(g.along);
        g.cameraX = at.x;
        g.cameraY = at.y;
        g_d.view(machine, g.stage, at.x / 256, at.y < 0 ? -1 : at.y / 256);
    }

    void ViewerFrame(Machine &machine) {
        // (another stage chosen in the menu: the viewer goes there; the board reset: it is over)
        if (FeatureValue("viewer_stage") != g.chosen) {
            StartViewer(machine);
            return;
        }
        if (!g.starting && !g_d.inGame()) {
            End();
            return;
        }
        SetHud({});
    }

    void ScoreFrame() {
        if (g.over) {
            if (--g.shown <= 0) {
                End();
            }
            return;
        }
        if (g_d.inGame()) {
            // (the most it showed: a game clears the score before its game over is through)
            g.played = true;
            g.score = std::max(g.score, g_d.score());
        } else if (g.played) {
            // (the game is over)
            g.over = true;
            g.shown = kShownFor;
            g.newBest = g.score > g.best;
            if (g.newBest && !RunningAhead()) {
                g.best = g.score;
                WriteBest(g.best);
            }
        }
        if (g.shown > 0) {
            g.shown--;
        }
    }

    void ScoreHud() {
        if (g.mode != Mode::Score || g.starting) {
            return;
        }
        if (g.over) {
            const std::string text = g.newBest ? "A new best: " + Grouped(g.score) + "!"
                                               : "Score " + Grouped(g.score) + ", best " + Grouped(g.best);
            SetHud({{0.5f, 0.74f, 0, "Score attack over"}, {0.5f, 0.82f, 0, text}});
        } else if (g.shown > 0) {
            SetHud({{0.5f, 0.88f, 0, "Score attack: best " + Grouped(g.best)}});
        } else {
            SetHud({});
        }
    }
} // namespace

bool AttractMode() {
    return g.mode == Mode::Attract;
}
int AttractShow() {
    return g.mode == Mode::Attract ? FeatureValue("attract_show") : 0;
}
bool ScoreAttack() {
    return g.mode == Mode::Score;
}
bool StageViewer() {
    return g.mode == Mode::Viewer;
}
bool CharacterViewer() {
    return g.mode == Mode::Model;
}
void BeginModel(Machine &machine, int stage) {
    End();
    g.mode = Mode::Model;
    g.starting = true;
    g.stage = stage;
    machine.Reset();
}
int ViewedStage() {
    return g.stage;
}
bool Starting() {
    return g.mode != Mode::None && g.starting;
}
void EndModes() {
    End();
}

void BeforeFrame(Machine &machine) {
    if (g.mode == Mode::Attract) {
        // (Start or a coin ends it, and is the game's: a game can begin from the show)
        if ((machine.players[0] | machine.players[1] | machine.players[2]) & (kStart | kCoin)) {
            End();
            return;
        }
        for (uint16_t &held : machine.players) {
            held = 0;
        }
    } else if (g.mode == Mode::Score && g.starting) {
        // Unseen to the title, a credit in, and Start until the game takes it (the choice of character is the player's)
        Fast(true);
        for (uint16_t &held : machine.players) {
            held = 0;
        }
        if (++g.frames > kStartMost) {
            End();
            return;
        }
        if (!g.credited && g_d.canStart(machine)) {
            g_d.credit();
            g.credited = true;
        }
        if (g.credited) {
            // (Start until the credit is taken: the game is at its choice of character, which is seen)
            if (g_d.credits() == 0) {
                g.starting = false;
                g.shown = kShownFor;
                Fast(false);
            } else {
                machine.players[0] = (g.frames & 4) ? kStart : 0;
            }
        }
    } else if (g.mode == Mode::Viewer) {
        ViewerBeforeFrame(machine);
    } else if (g.mode == Mode::Model) {
        if (g.starting) {
            ToStage(machine);
        } else {
            g_d.view(machine, g.stage, -1, -1);
            ModelBeforeFrame(machine);
        }
    } else if (g.mode == Mode::Score) {
        // (one player only: the others' Starts and coins are not taken)
        for (int n = 1; n < Machine::kMaxPlayers; n++) {
            machine.players[n] &= static_cast<uint16_t>(~(kStart | kCoin));
        }
        machine.players[0] &= static_cast<uint16_t>(~kCoin);
    }
}

void Frame(Machine &machine) {
    if (g.mode == Mode::Attract && g.starting) {
        // (the board's start, unseen unless everything is wanted)
        Fast(FeatureValue("attract_show") != 0);
        if (g_d.showBegun(machine) || ++g.frames > kStartMost) {
            g.starting = false;
            Fast(false);
        }
    } else if (g.mode == Mode::Score && !g.starting) {
        ScoreFrame();
    } else if (g.mode == Mode::Viewer) {
        ViewerFrame(machine);
    } else if (g.mode == Mode::Model && !g.starting) {
        if (!g_d.inGame()) {
            End(); // (the board reset)
        } else {
            ModelFrame(machine);
        }
    }
    ScoreHud();
    // (nobody shown while viewing (the character viewer draws its own): and only what this file set is put back)
    static bool hid = false;
    const bool viewing = (g.mode == Mode::Viewer || g.mode == Mode::Model) && !g.starting;
    if (viewing || hid) {
        machine.video.hideObject = viewing ? HideAll : nullptr;
        machine.video.layerMask = viewing ? g_d.viewLayers : 0xF;
        hid = viewing;
    }
}

void State(Archive &ar) {
    ar.Mark(0x42454154); // BEAT
    ar.Pod(g);
    if (!ar.Saving() && g.mode == Mode::Viewer) {
        LoadPath();
    }
    if (!ar.Saving() && !RunningAhead() && g.mode == Mode::Model) {
        End(); // (the character viewer's own is not kept: a state loaded while viewing ends it)
    }
    if (!ar.Saving() && !RunningAhead()) {
        g_fast = !g.starting; // (put the window's running unseen as the state has it)
        Fast(g.starting);
    }
}

void Register(const Description &description) {
    g_d = description;
    Game &game = TheGame();
    game.pages.push_back({kModes, "", "Other ways to run the game: attract mode, score attack, viewers for the stages and the characters.", true});
    game.pages.push_back({kAttract, kModes, "The game's own show on a loop, with no coin in and its sound on: all of it, or one part."});
    game.pages.push_back({kScore, kModes, "One player and one credit from the first round, for the best score."});
    game.commands.push_back({"attract_start", kAttract, "Start",
                             "Starts attract mode: the game's own show, over and over. Start (or a coin) ends it.", StartAttract});
    std::vector<const char *> shows = {"Everything"};
    shows.insert(shows.end(), description.shows.begin(), description.shows.end());
    game.features.push_back({"attract_show", kAttract, "Shows", description.showsWords, 0, shows});
    if (!description.stages.empty()) {
        game.pages.push_back({kViewer, kModes, "A stage with nobody on it, to look at and listen to."});
        game.commands.push_back({"viewer_start", kViewer, "Start",
                                 "Shows the stage chosen below with nobody on it. Left and right scroll it, or it scrolls by itself. "
                                 "Reset ends it.",
                                 StartViewer});
        std::vector<const char *> stages = description.stages;
        stages.push_back("Infinite");
        game.features.push_back({"viewer_stage", kViewer, "Stage",
                                 "The stage the viewer shows, or Infinite: every stage in turn, each from one end to the other. "
                                 "Changing it while viewing goes to the new one.",
                                 0, stages});
        game.features.push_back({"viewer_scroll", kViewer, "Scrolling",
                                 "How the viewer moves across the stage: with the stick, or by itself from one end to the other and "
                                 "back (Infinite: on to the next stage at the end).",
                                 0, {"With the stick", "Slow", "Medium", "Fast"}});
    }
    RegisterModel(description);
    // (their choices are this game's: in the collection, the fighting games' rows of the same names are others')
    OwnFeatures({"attract_show", "viewer_stage", "model_character", "model_background", "model_animation"});
    game.commands.push_back({"score_start", kScore, "Start",
                             "Starts a game for player 1 alone: one credit and no continue, from the first round, with free play, "
                             "the cheats and \"Start at round\" left out. The best score is kept. The game over ends it.",
                             StartScore});
}

} // namespace cps2recomp::beat
