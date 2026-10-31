#include "WoodlandStructures.h"
#include "WoodlandData.h"

#include "../../world/level/Level.h"

#include <cstring>

int WoodlandStructures::find(const char* name)
{
    for (int i = 0; i < WL_STRUCTURE_COUNT; ++i)
        if (strcmp(WL_STRUCTURES[i].name, name) == 0) return i;
    return -1;
}

void WoodlandStructures::place(Level* level, int index, int x0, int y0, int z0)
{
    if (index < 0 || index >= WL_STRUCTURE_COUNT) return;
    const WlStructure& s = WL_STRUCTURES[index];
    int pos = 0;                                  // linear index: (y * sz + z) * sx + x
    for (int r = 0; r < s.runCount; ++r) {
        const WlRun& run = WL_RUNS[s.runStart + r];
        for (int k = 0; k < run.n; ++k, ++pos) {
            if (run.id == 255) continue;          // structure_void: leave the world as is
            int x = pos % s.sx;
            int z = (pos / s.sx) % s.sz;
            int y = pos / (s.sx * s.sz);
            int wy = y0 + y;
            if (wy < 0 || wy >= 127) continue;
            // ENGINE: Level::setTileAndData(x, y, z, tile, data)
            level->setTileAndData(x0 + x, wy, z0 + z, run.id, run.data);
        }
    }
}
