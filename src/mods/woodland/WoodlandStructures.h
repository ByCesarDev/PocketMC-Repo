#ifndef POCKETMC_MODS_WOODLAND_STRUCTURES_H__
#define POCKETMC_MODS_WOODLAND_STRUCTURES_H__

class Level;

// The 55 woodland .nbt structures, converted to run-length encoded 0.6.1 tile data.
// Block names that don't exist in 0.6.1 were mapped to the closest tile (see tools/gen_woodland.py).
class WoodlandStructures {
public:
    static int  find(const char* name);                               // -1 if unknown
    // Places structure `index` with its south-west-bottom corner at x,y,z.
    static void place(Level* level, int index, int x, int y, int z);
};

#endif
