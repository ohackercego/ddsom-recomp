// A test that reads work/ddsom/settings.ini with the port's own code (port/bindings.h and
// recomp-common/launcher/src/bindings.h) and prints what each key does. Not part of the port.
#include <cstdio>
#include <string>

#include "../../port/bindings.h"

int main() {
    const port::BoundTable &t = port::MakeBoundTable(port::GamePlayers(), port::GameButtons(), false);
    const std::string path = "C:/Users/azure/Downloads/cps2/cps2recomp/work/ddsom/settings.ini";
    const port::Bindings b = port::LoadBindings(path, t);

    std::printf("players: %d   actions: %d\n", t.table.players, t.table.count);
    std::printf("--- player 1 keyboard (from %s) ---\n", path.c_str());
    for (int a = 0; a < t.table.count; a++) {
        const SDL_Scancode k = b.key[0][a];
        const char *name = k == SDL_SCANCODE_UNKNOWN ? "none" : SDL_GetScancodeName(k);
        std::printf("  %-6s (%-14s) -> %s\n", t.actions[a].name, t.actions[a].title, name);
    }
    return 0;
}
