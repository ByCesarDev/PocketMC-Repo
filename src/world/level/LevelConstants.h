#ifndef _MINECRAFT_WORLD_LEVELCONSTANTS_H_
#define _MINECRAFT_WORLD_LEVELCONSTANTS_H_


const int LEVEL_HEIGHT = 256;
const int CHUNK_WIDTH = 16; // in blocks
const int CHUNK_DEPTH = 16;

// World limits (±30,000,000 blocks to avoid floating-point precision issues)
const int WORLD_LIMIT = 30000000;
const int WORLD_MIN_X = -WORLD_LIMIT;
const int WORLD_MAX_X = WORLD_LIMIT;
const int WORLD_MIN_Z = -WORLD_LIMIT;
const int WORLD_MAX_Z = WORLD_LIMIT;

// View distance in chunks (adjustable)
const int VIEW_DISTANCE_CHUNKS = 16;

const int CHUNK_COLUMNS = CHUNK_WIDTH * CHUNK_DEPTH;
const int CHUNK_BLOCK_COUNT = CHUNK_COLUMNS * LEVEL_HEIGHT;

#endif
