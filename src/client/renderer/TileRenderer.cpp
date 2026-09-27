#include "TileRenderer.h"
#include "Chunk.h"
#include "../Minecraft.h"
#include "Tesselator.h"
#include "Textures.h"
#include <set>

#include "../../world/level/LevelSource.h"
#include "../../world/level/tile/Tile.h"
#include "../../world/level/tile/LeafTile.h"
#include "../../world/level/FoliageColor.h"
#include "../../world/level/tile/DoorTile.h"
#include "../../world/level/tile/LiquidTile.h"
#include "../../world/level/tile/FenceTile.h"
#include "../../world/level/tile/FenceGateTile.h"
#include "../../world/level/tile/ThinFenceTile.h"
#include "../../world/level/tile/BedTile.h"
#include "../../world/level/tile/StemTile.h"
#include "../../world/level/tile/StairTile.h"
#include "../../world/level/tile/FireTile.h"
#include "../../world/Direction.h"
#include "../../world/Facing.h"
#include "../../world/level/tile/AnvilTile.h"
#include "../../world/level/tile/CauldronTile.h"
#include "../../world/level/tile/HopperTile.h"
#include "EntityTileRenderer.h"

bool TileRenderer::sideTinting = true;

static inline float getAtlasU(int slot, float px) {
	int tileX = slot & 0xf;
	return (tileX * 16.0f + px) / 256.0f;
}

static inline float getAtlasV(int slot, float py) {
	int tileY = slot >> 4;
	return (tileY * 16.0f + py) / 256.0f;
}

TileRenderer::TileRenderer(LevelSource* level /* = NULL */, Textures* textures /* = nullptr */ )
	:	level(level),
	textures(textures),
	fixedTexture(-1),
	xFlipTexture(false),
	noCulling(false),
	blsmooth(1),
	applyAmbienceOcclusion(false),
	atlasFilter(-1)
{
}

bool TileRenderer::tesselateBlockInWorld(Tile* tt, int x, int y, int z) {
	Tesselator& t = Tesselator::instance;
	// Atlas filter: check if this block belongs to the current atlas pass
	if (atlasFilter != -1) {
		// Check all 6 face textures to determine which atlas this block uses
		bool hasMain = false;
		bool hasAlt = false;
		for (int face = 0; face < 6; face++) {
			int tex = tt->getTexture(level, x, y, z, face);
			if (tex & Tile::TEXTURE_ALT_FLAG) hasAlt = true;
			else hasMain = true;
		}
		// If this atlas pass has nothing to render for this block, skip it entirely
		if (atlasFilter == 0 && !hasMain) return false;
		if (atlasFilter == 1 && !hasAlt) return false;
	}

	int col = tt->getColor(level, x, y, z);
	float r = ((col >> 16) & 0xff) / 255.0f;
	float g = ((col >> 8) & 0xff) / 255.0f;
	float b = ((col) & 0xff) / 255.0f; // xFlipTexture = (x & 1) != (y & 1);

	if (Minecraft::useAmbientOcclusion) {
		return tesselateBlockInWorldWithAmbienceOcclusion(tt, x, y, z, r, g, b);
	} else
	{
		return tesselateBlockInWorld(tt, x, y, z, r, g, b);
	}
}

bool TileRenderer::tesselateBlockInWorld( Tile* tt, int x, int y, int z, float r, float g, float b )
{
	applyAmbienceOcclusion = false;
	float xf = (float)x;
	float yf = (float)y;
	float zf = (float)z;

	Tesselator& t = Tesselator::instance;

	bool changed = false;
	float c10 = 0.5f;
	float c11 = 1;
	float c2 = 0.8f;
	float c3 = 0.6f;

	// added these to get biome color and save it before its overriden - shredder
	float biomeR = r;
	float biomeG = g;
	float biomeB = b;


	float r11 = c11 * r;
	float g11 = c11 * g;
	float b11 = c11 * b;

	if (tt == (Tile*)Tile::grass) {
		r = g = b = 1.0f;
	}

	float r10 = c10 * r;
	float r2 = c2 * r;
	float r3 = c3 * r;

	float g10 = c10 * g;
	float g2 = c2 * g;
	float g3 = c3 * g;

	float b10 = c10 * b;
	float b2 = c2 * b;
	float b3 = c3 * b;

	float centerBrightness = tt->getBrightness(level, x, y, z);

	if (noCulling || tt->shouldRenderFace(level, x, y - 1, z, Facing::DOWN)) {
		int texDown = tt->getTexture(level, x, y, z, 0);
		bool isAltDown = (texDown & Tile::TEXTURE_ALT_FLAG) != 0;
		if (atlasFilter == -1 || (atlasFilter == 0 && !isAltDown) || (atlasFilter == 1 && isAltDown)) {
			float br = tt->getBrightness(level, x, y - 1, z);
			t.color(r10 * br, g10 * br, b10 * br);
			renderFaceDown(tt, xf, yf, zf, texDown & ~Tile::TEXTURE_ALT_FLAG);
			changed = true;
		}
	}

	if (noCulling || tt->shouldRenderFace(level, x, y + 1, z, Facing::UP)) {
		int texUp = tt->getTexture(level, x, y, z, 1);
		bool isAltUp = (texUp & Tile::TEXTURE_ALT_FLAG) != 0;
		if (atlasFilter == -1 || (atlasFilter == 0 && !isAltUp) || (atlasFilter == 1 && isAltUp)) {
			float br = tt->getBrightness(level, x, y + 1, z);
			if (tt->yy1 != 1 && !tt->material->isLiquid()) br = centerBrightness;
			t.color(r11 * br, g11 * br, b11 * br);
			renderFaceUp(tt, xf, yf, zf, texUp & ~Tile::TEXTURE_ALT_FLAG);
			changed = true;
		}
	}

	if (noCulling || tt->shouldRenderFace(level, x, y, z - 1, Facing::NORTH)) {
		int texNorth = tt->getTexture(level, x, y, z, 2);
		bool isAltNorth = (texNorth & Tile::TEXTURE_ALT_FLAG) != 0;
		if (atlasFilter == -1 || (atlasFilter == 0 && !isAltNorth) || (atlasFilter == 1 && isAltNorth)) {
			float br = tt->getBrightness(level, x, y, z - 1);
			if (tt->zz0 > 0) br = centerBrightness;
			t.color(r2 * br, g2 * br, b2 * br);
			int cleanTex = texNorth & ~Tile::TEXTURE_ALT_FLAG;
			int baseTex = (cleanTex == 3 && sideTinting) ? 236 : cleanTex;
			renderNorth(tt, xf, yf, zf, baseTex);
			if (cleanTex == 3 && sideTinting) {
				t.color(c2 * br * biomeR, c2 * br * biomeG, c2 * br * biomeB);
				renderNorth(tt, xf, yf, zf - 0.001f, 38);
			}
			changed = true;
		}
	}

	if (noCulling || tt->shouldRenderFace(level, x, y, z + 1, Facing::SOUTH)) {
		int texSouth = tt->getTexture(level, x, y, z, 3);
		bool isAltSouth = (texSouth & Tile::TEXTURE_ALT_FLAG) != 0;
		if (atlasFilter == -1 || (atlasFilter == 0 && !isAltSouth) || (atlasFilter == 1 && isAltSouth)) {
			float br = tt->getBrightness(level, x, y, z + 1);
			if (tt->zz1 < 1) br = centerBrightness;
			t.color(r2 * br, g2 * br, b2 * br);
			int cleanTex = texSouth & ~Tile::TEXTURE_ALT_FLAG;
			int baseTex = (cleanTex == 3 && sideTinting) ? 236 : cleanTex;
			renderSouth(tt, xf, yf, zf, baseTex);
			if (cleanTex == 3 && sideTinting) {
				t.color(c2 * br * biomeR, c2 * br * biomeG, c2 * br * biomeB);
				renderSouth(tt, xf, yf, zf + 0.001f, 38);
			}
			changed = true;
		}
	}

	if (noCulling || tt->shouldRenderFace(level, x - 1, y, z, Facing::WEST)) {
		int texWest = tt->getTexture(level, x, y, z, 4);
		bool isAltWest = (texWest & Tile::TEXTURE_ALT_FLAG) != 0;
		if (atlasFilter == -1 || (atlasFilter == 0 && !isAltWest) || (atlasFilter == 1 && isAltWest)) {
			float br = tt->getBrightness(level, x - 1, y, z);
			if (tt->xx0 > 0) br = centerBrightness;
			t.color(r3 * br, g3 * br, b3 * br);
			int cleanTex = texWest & ~Tile::TEXTURE_ALT_FLAG;
			int baseTex = (cleanTex == 3 && sideTinting) ? 236 : cleanTex;
			renderWest(tt, xf, yf, zf, baseTex);
			if (cleanTex == 3 && sideTinting) {
				t.color(c3 * br * biomeR, c3 * br * biomeG, c3 * br * biomeB);
				renderWest(tt, xf - 0.001f, yf, zf, 38);
			}
			changed = true;
		}
	}

	if (noCulling || tt->shouldRenderFace(level, x + 1, y, z, Facing::EAST)) {
		int texEast = tt->getTexture(level, x, y, z, 5);
		bool isAltEast = (texEast & Tile::TEXTURE_ALT_FLAG) != 0;
		if (atlasFilter == -1 || (atlasFilter == 0 && !isAltEast) || (atlasFilter == 1 && isAltEast)) {
			float br = tt->getBrightness(level, x + 1, y, z);
			if (tt->xx1 < 1) br = centerBrightness;
			t.color(r3 * br, g3 * br, b3 * br);
			int cleanTex = texEast & ~Tile::TEXTURE_ALT_FLAG;
			int baseTex = (cleanTex == 3 && sideTinting) ? 236 : cleanTex;
			renderEast(tt, xf, yf, zf, baseTex);
			if (cleanTex == 3 && sideTinting) {
				t.color(c3 * br * biomeR, c3 * br * biomeG, c3 * br * biomeB);
				renderEast(tt, xf + 0.001f, yf, zf, 38);
			}
			changed = true;
		}
	}

	return changed;
}


void TileRenderer::tesselateInWorld( Tile* tile, int x, int y, int z, int fixedTexture )
{
	this->fixedTexture = fixedTexture;
	tesselateInWorld(tile, x, y, z);
	this->fixedTexture = -1;
}

bool TileRenderer::tesselateInWorld( Tile* tt, int x, int y, int z )
{
	int shape = tt->getRenderShape();
	tt->updateShape(level, x, y, z);

	if (atlasFilter != -1 && shape != Tile::SHAPE_BLOCK && shape != Tile::SHAPE_STAIRS) {
		int tex = tt->getTexture(0, 0);
		bool isAlt = (tex & Tile::TEXTURE_ALT_FLAG) != 0;
		if (atlasFilter == 0 && isAlt) return false;
		if (atlasFilter == 1 && !isAlt) return false;
	}

	if (shape == Tile::SHAPE_BLOCK) {
		return tesselateBlockInWorld(tt, x, y, z);
	} else if (shape == Tile::SHAPE_WATER) {
		return tesselateWaterInWorld(tt, x, y, z);
	} else if (shape == Tile::SHAPE_CACTUS) {
		return tesselateCactusInWorld(tt, x, y, z);
	} else if (shape == Tile::SHAPE_CROSS_TEXTURE) {
		return tesselateCrossInWorld(tt, x, y, z);
	} else if(shape == Tile::SHAPE_STEM) {
		return tesselateStemInWorld(tt, x, y, z);
	} else if (shape == Tile::SHAPE_ROWS) {
		return tesselateRowInWorld(tt, x, y, z);
	} else if (shape == Tile::SHAPE_TORCH) {
		return tesselateTorchInWorld(tt, x, y, z);
	} else if (shape == Tile::SHAPE_FIRE) {
		return tesselateFireInWorld(tt, x, y, z);
		//} else if (shape == Tile::SHAPE_RED_DUST) {
		//    return tesselateDustInWorld(tt, x, y, z);
	} else if (shape == Tile::SHAPE_LADDER) {
		return tesselateLadderInWorld(tt, x, y, z);
	} else if (shape == Tile::SHAPE_DOOR) {
		return tesselateDoorInWorld(tt, x, y, z);
		//} else if (shape == Tile::SHAPE_RAIL) {
		//    return tesselateRailInWorld(tt, x, y, z);
	} else if (shape == Tile::SHAPE_STAIRS) {
		return tesselateStairsInWorld((StairTile*)tt, x, y, z);
	} else if (shape == Tile::SHAPE_FENCE) {
		return tesselateFenceInWorld((FenceTile*)tt, x, y, z);
	} else if (shape == Tile::SHAPE_FENCE_GATE) {
		return tesselateFenceGateInWorld((FenceGateTile*) tt, x, y, z);
		//} else if (shape == Tile::SHAPE_LEVER) {
		//    return tesselateLeverInWorld(tt, x, y, z);
		//} else if (shape == Tile::SHAPE_BED) {
		//    return tesselateBedInWorld(tt, x, y, z);
		//} else if (shape == Tile::SHAPE_DIODE) {
		//    return tesselateDiodeInWorld(tt, x, y, z);
	} else if (shape == Tile::SHAPE_IRON_FENCE) {
		return tesselateThinFenceInWorld((ThinFenceTile*) tt, x, y, z);
	} else if(shape == Tile::SHAPE_BED) {
		return tesselateBedInWorld(tt, x, y, z);
	} else if (shape == Tile::SHAPE_LANTERN) {
		return tesselateLanternInWorld(tt, x, y, z);
	} else if (shape == Tile::SHAPE_CAMPFIRE) {
		return tesselateCampfireInWorld(tt, x, y, z);
	} else if (shape == Tile::SHAPE_GRINDSTONE) {
		return tesselateGrindstoneInWorld(tt, x, y, z);
	} else if (shape == Tile::SHAPE_LECTERN) {
		return tesselateLecternInWorld(tt, x, y, z);
	} else if (shape == Tile::SHAPE_COMPOSTER) {
		return tesselateComposterInWorld(tt, x, y, z);
	} else if (shape == Tile::SHAPE_STONECUTTER) {
		return tesselateStonecutterInWorld(tt, x, y, z);
	} else if (shape == Tile::SHAPE_CHAIN) {
		return tesselateChainInWorld(tt, x, y, z);
	} else if (shape == Tile::SHAPE_SCAFFOLDING) {
		return tesselateScaffoldingInWorld(tt, x, y, z);
	} else if (shape == Tile::SHAPE_CANDLE) {
		return tesselateCandleInWorld(tt, x, y, z);
	} else if (shape == Tile::SHAPE_CAULDRON) {
		return tesselateCauldronInWorld(tt, x, y, z);
	} else if (shape == Tile::SHAPE_ANVIL) {
		return tesselateAnvilInWorld(tt, x, y, z);
	} else if (shape == Tile::SHAPE_HOPPER) {
		return tesselateHopperInWorld(tt, x, y, z);
	} else {
		return false;
	}
}

void TileRenderer::tesselateInWorldNoCulling( Tile* tile, int x, int y, int z )
{
	noCulling = true;
	tesselateInWorld(tile, x, y, z);
	noCulling = false;
}

bool TileRenderer::tesselateTorchInWorld( Tile* tt, int x, int y, int z )
{
	int dir = level->getData(x, y, z);

	Tesselator& t = Tesselator::instance;

	float br = tt->getBrightness(level, x, y, z);
	if (Tile::lightEmission[tt->id] > 0) br = 1.0f;
	t.color(br, br, br);

	float r = 0.40f;
	float r2 = 0.5f - r;
	float h = 0.20f;
	if (dir == 1) {
		tesselateTorch(tt, (float)x - r2, (float)y + h, (float)z, -r, 0);
	} else if (dir == 2) {
		tesselateTorch(tt, (float)x + r2, (float)y + h, (float)z, +r, 0);
	} else if (dir == 3) {
		tesselateTorch(tt, (float)x, (float)y + h, (float)z - r2, 0, -r);
	} else if (dir == 4) {
		tesselateTorch(tt, (float)x, (float)y + h, (float)z + r2, 0, +r);
	} else {
		tesselateTorch(tt, (float)x, (float)y, (float)z, 0, 0);
	}
	return true;
}

bool TileRenderer::tesselateFireInWorld( Tile* tt, int x, int y, int z )
{
	// fire transparency has been fixed - shredder

	Tesselator& t = Tesselator::instance;

	int tex = tt->getTexture(0);

	if (fixedTexture >= 0) tex = fixedTexture;

	float br = tt->getBrightness( level, x, y, z );
	if (Tile::lightEmission[tt->id] > 0) br = 1.0f;
	t.color( br, br, br );

	int cleanTex = tex & ~Tile::TEXTURE_ALT_FLAG;
	int xt = ((cleanTex & 0xf) << 4);
	int yt = cleanTex & 0xf0;

	float u0 = (xt) / 256.0f;
	float u1 = (xt + 15.99f) / 256.0f;
	float v0 = (yt) / 256.0f;
	float v1 = (yt + 15.99f) / 256.0f;
	float h = 1.4f;

	if ( level->isSolidBlockingTile( x, y - 1, z ) || (Tile::fire && Tile::fire->canBurn( level, x, y - 1, z )) || (Tile::soulSand && level->getTile(x, y - 1, z) == Tile::soulSand->id) )
	{
		float	x0 = x + 0.5f + 0.2f;
		float	x1 = x + 0.5f - 0.2f;
		float	z0 = z + 0.5f + 0.2f;
		float	z1 = z + 0.5f - 0.2f;

		float	x0_ = x + 0.5f - 0.3f;
		float	x1_ = x + 0.5f + 0.3f;
		float	z0_ = z + 0.5f - 0.3f;
		float	z1_ = z + 0.5f + 0.3f;

		t.vertexUV( ( float )( x0_ ), ( float )( y + h ), ( float )( z + 1 ), ( float )( u1 ), ( float )( v0 ) );
		t.vertexUV( ( float )( x0 ), ( float )( y + 0 ), ( float )( z + 1 ), ( float )( u1 ), ( float )( v1 ) );
		t.vertexUV( ( float )( x0 ), ( float )( y + 0 ), ( float )( z + 0 ), ( float )( u0 ), ( float )( v1 ) );
		t.vertexUV( ( float )( x0_ ), ( float )( y + h ), ( float )( z + 0 ), ( float )( u0 ), ( float )( v0 ) );

		t.vertexUV( ( float )( x1_ ), ( float )( y + h ), ( float )( z + 0 ), ( float )( u1 ), ( float )( v0 ) );
		t.vertexUV( ( float )( x1 ), ( float )( y + 0 ), ( float )( z + 0 ), ( float )( u1 ), ( float )( v1 ) );
		t.vertexUV( ( float )( x1 ), ( float )( y + 0 ), ( float )( z + 1 ), ( float )( u0 ), ( float )( v1 ) );
		t.vertexUV( ( float )( x1_ ), ( float )( y + h ), ( float )( z + 1 ), ( float )( u0 ), ( float )( v0 ) );



		u0 = (xt) / 256.0f;
		u1 = (xt + 15.99f) / 256.0f;
		v0 = (yt) / 256.0f;
		v1 = (yt + 15.99f) / 256.0f;

		t.vertexUV( ( float )( x + 1 ), ( float )( y + h ), ( float )( z1_ ), ( float )( u1 ), ( float )( v0 ) );
		t.vertexUV( ( float )( x + 1 ), ( float )( y + 0 ), ( float )( z1 ), ( float )( u1 ), ( float )( v1 ) );
		t.vertexUV( ( float )( x + 0 ), ( float )( y + 0 ), ( float )( z1 ), ( float )( u0 ), ( float )( v1 ) );
		t.vertexUV( ( float )( x + 0 ), ( float )( y + h ), ( float )( z1_ ), ( float )( u0 ), ( float )( v0 ) );

		t.vertexUV( ( float )( x + 0 ), ( float )( y + h ), ( float )( z0_ ), ( float )( u1 ), ( float )( v0 ) );
		t.vertexUV( ( float )( x + 0 ), ( float )( y + 0 ), ( float )( z0 ), ( float )( u1 ), ( float )( v1 ) );
		t.vertexUV( ( float )( x + 1 ), ( float )( y + 0 ), ( float )( z0 ), ( float )( u0 ), ( float )( v1 ) );
		t.vertexUV( ( float )( x + 1 ), ( float )( y + h ), ( float )( z0_ ), ( float )( u0 ), ( float )( v0 ) );

		x0 = x + 0.5f - 0.5f;
		x1 = x + 0.5f + 0.5f;
		z0 = z + 0.5f - 0.5f;
		z1 = z + 0.5f + 0.5f;

		x0_ = x + 0.5f - 0.4f;
		x1_ = x + 0.5f + 0.4f;
		z0_ = z + 0.5f - 0.4f;
		z1_ = z + 0.5f + 0.4f;

		t.vertexUV( ( float )( x0_ ), ( float )( y + h ), ( float )( z + 0 ), ( float )( u0 ), ( float )( v0 ) );
		t.vertexUV( ( float )( x0 ), ( float )( y + 0 ), ( float )( z + 0 ), ( float )( u0 ), ( float )( v1 ) );
		t.vertexUV( ( float )( x0 ), ( float )( y + 0 ), ( float )( z + 1 ), ( float )( u1 ), ( float )( v1 ) );
		t.vertexUV( ( float )( x0_ ), ( float )( y + h ), ( float )( z + 1 ), ( float )( u1 ), ( float )( v0 ) );

		t.vertexUV( ( float )( x1_ ), ( float )( y + h ), ( float )( z + 1 ), ( float )( u0 ), ( float )( v0 ) );
		t.vertexUV( ( float )( x1 ), ( float )( y + 0 ), ( float )( z + 1 ), ( float )( u0 ), ( float )( v1 ) );
		t.vertexUV( ( float )( x1 ), ( float )( y + 0 ), ( float )( z + 0 ), ( float )( u1 ), ( float )( v1 ) );
		t.vertexUV( ( float )( x1_ ), ( float )( y + h ), ( float )( z + 0 ), ( float )( u1 ), ( float )( v0 ) );


		u0 = (xt) / 256.0f;
		u1 = (xt + 15.99f) / 256.0f;
		v0 = (yt) / 256.0f;
		v1 = (yt + 15.99f) / 256.0f;

		t.vertexUV( ( float )( x + 0 ), ( float )( y + h ), ( float )( z1_ ), ( float )( u0 ), ( float )( v0 ) );
		t.vertexUV( ( float )( x + 0 ), ( float )( y + 0 ), ( float )( z1 ), ( float )( u0 ), ( float )( v1 ) );
		t.vertexUV( ( float )( x + 1 ), ( float )( y + 0 ), ( float )( z1 ), ( float )( u1 ), ( float )( v1 ) );
		t.vertexUV( ( float )( x + 1 ), ( float )( y + h ), ( float )( z1_ ), ( float )( u1 ), ( float )( v0 ) );

		t.vertexUV( ( float )( x + 1 ), ( float )( y + h ), ( float )( z0_ ), ( float )( u0 ), ( float )( v0 ) );
		t.vertexUV( ( float )( x + 1 ), ( float )( y + 0 ), ( float )( z0 ), ( float )( u0 ), ( float )( v1 ) );
		t.vertexUV( ( float )( x + 0 ), ( float )( y + 0 ), ( float )( z0 ), ( float )( u1 ), ( float )( v1 ) );
		t.vertexUV( ( float )( x + 0 ), ( float )( y + h ), ( float )( z0_ ), ( float )( u1 ), ( float )( v0 ) );
	}
	else
	{
		float	r = 0.2f;
		float	yo = 1 / 16.0f;
		if ( ( ( x + y + z ) & 1 ) == 1 )
		{
			u0 = (xt) / 256.0f;
			u1 = (xt + 15.99f) / 256.0f;
			v0 = (yt) / 256.0f;
			v1 = (yt + 15.99f) / 256.0f;
		}
		if ( ( ( x / 2 + y / 2 + z / 2 ) & 1 ) == 1 )
		{
			float tmp = u1;
			u1 = u0;
			u0 = tmp;
		}
		if ( Tile::fire->canBurn( level, x - 1, y, z ) )
		{
			t.vertexUV( ( float )( x + r ), ( float )( y + h + yo ), ( float )( z +
				1.0f ), ( float )( u1 ), ( float )( v0 ) );
			t.vertexUV( ( float )( x + 0.0f ), ( float )( y + 0.0f + yo ), ( float )( z +
				1.0f ), ( float )( u1 ), ( float )( v1 ) );
			t.vertexUV( ( float )( x + 0.0f ), ( float )( y + 0.0f + yo ), ( float )( z +
				0.0f ), ( float )( u0 ), ( float )( v1 ) );
			t.vertexUV( ( float )( x + r ), ( float )( y + h + yo ), ( float )( z +
				0.0f ), ( float )( u0 ), ( float )( v0 ) );

			t.vertexUV( ( float )( x + r ), ( float )( y + h + yo ), ( float )( z +
				0.0f ), ( float )( u0 ), ( float )( v0 ) );
			t.vertexUV( ( float )( x + 0.0f ), ( float )( y + 0.0f + yo ), ( float )( z +
				0.0f ), ( float )( u0 ), ( float )( v1 ) );
			t.vertexUV( ( float )( x + 0.0f ), ( float )( y + 0.0f + yo ), ( float )( z +
				1.0f ), ( float )( u1 ), ( float )( v1 ) );
			t.vertexUV( ( float )( x + r ), ( float )( y + h + yo ), ( float )( z +
				1.0f ), ( float )( u1 ), ( float )( v0 ) );
		}
		if ( Tile::fire->canBurn( level, x + 1, y, z ) )
		{
			t.vertexUV( ( float )( x + 1 - r ), ( float )( y + h + yo ), ( float )( z +
				0.0f ), ( float )( u0 ), ( float )( v0 ) );
			t.vertexUV( ( float )( x + 1 - 0 ), ( float )( y + 0 + yo ), ( float )( z +
				0.0f ), ( float )( u0 ), ( float )( v1 ) );
			t.vertexUV( ( float )( x + 1 - 0 ), ( float )( y + 0 + yo ), ( float )( z +
				1.0f ), ( float )( u1 ), ( float )( v1 ) );
			t.vertexUV( ( float )( x + 1 - r ), ( float )( y + h + yo ), ( float )( z +
				1.0f ), ( float )( u1 ), ( float )( v0 ) );

			t.vertexUV( ( float )( x + 1.0f - r ), ( float )( y + h + yo ), ( float )( z +
				1.0f ), ( float )( u1 ), ( float )( v0 ) );
			t.vertexUV( ( float )( x + 1.0f - 0.0f ), ( float )( y + 0.0f + yo ), ( float )( z +
				1.0f ), ( float )( u1 ), ( float )( v1 ) );
			t.vertexUV( ( float )( x + 1.0f - 0 ), ( float )( y + 0.0f + yo ), ( float )( z +
				0.0f ), ( float )( u0 ), ( float )( v1 ) );
			t.vertexUV( ( float )( x + 1.0f - r ), ( float )( y + h + yo ), ( float )( z +
				0.0f ), ( float )( u0 ), ( float )( v0 ) );
		}
		if ( Tile::fire->canBurn( level, x, y, z - 1 ) )
		{
			t.vertexUV( ( float )( x + 0.0f ), ( float )( y + h + yo ), ( float )( z +
				r ), ( float )( u1 ), ( float )( v0 ) );
			t.vertexUV( ( float )( x + 0.0f ), ( float )( y + 0.0f + yo ), ( float )( z +
				0.0f ), ( float )( u1 ), ( float )( v1 ) );
			t.vertexUV( ( float )( x + 1.0f ), ( float )( y + 0.0f + yo ), ( float )( z +
				0.0f ), ( float )( u0 ), ( float )( v1 ) );
			t.vertexUV( ( float )( x + 1.0f ), ( float )( y + h + yo ), ( float )( z +
				r ), ( float )( u0 ), ( float )( v0 ) );

			t.vertexUV( ( float )( x + 1.0f ), ( float )( y + h + yo ), ( float )( z +
				r ), ( float )( u0 ), ( float )( v0 ) );
			t.vertexUV( ( float )( x + 1.0f ), ( float )( y + 0.0f + yo ), ( float )( z +
				0.0f ), ( float )( u0 ), ( float )( v1 ) );
			t.vertexUV( ( float )( x + 0.0f ), ( float )( y + 0.0f + yo ), ( float )( z +
				0.0f ), ( float )( u1 ), ( float )( v1 ) );
			t.vertexUV( ( float )( x + 0.0f ), ( float )( y + h + yo ), ( float )( z +
				r ), ( float )( u1 ), ( float )( v0 ) );
		}
		if ( Tile::fire->canBurn( level, x, y, z + 1 ) )
		{
			t.vertexUV( ( float )( x + 1.0f ), ( float )( y + h + yo ), ( float )( z + 1.0f -
				r ), ( float )( u0 ), ( float )( v0 ) );
			t.vertexUV( ( float )( x + 1.0f ), ( float )( y + 0.0f + yo ), ( float )( z + 1.0f -
				0.0f ), ( float )( u0 ), ( float )( v1 ) );
			t.vertexUV( ( float )( x + 0.0f ), ( float )( y + 0.0f + yo ), ( float )( z + 1.0f -
				0.0f ), ( float )( u1 ), ( float )( v1 ) );
			t.vertexUV( ( float )( x + 0.0f ), ( float )( y + h + yo ), ( float )( z + 1.0f -
				r ), ( float )( u1 ), ( float )( v0 ) );

			t.vertexUV( ( float )( x + 0.0f ), ( float )( y + h + yo ), ( float )( z + 1.0f -
				r ), ( float )( u1 ), ( float )( v0 ) );
			t.vertexUV( ( float )( x + 0.0f ), ( float )( y + 0.0f + yo ), ( float )( z + 1.0f -
				0.0f ), ( float )( u1 ), ( float )( v1 ) );
			t.vertexUV( ( float )( x + 1.0f ), ( float )( y + 0.0f + yo ), ( float )( z + 1.0f -
				0.0f ), ( float )( u0 ), ( float )( v1 ) );
			t.vertexUV( ( float )( x + 1.0f ), ( float )( y + h + yo ), ( float )( z + 1.0f -
				r ), ( float )( u0 ), ( float )( v0 ) );
		}
		if ( Tile::fire->canBurn( level, x, y + 1.0f, z ) )
		{
			double	x0 = x + 0.5f + 0.5f;
			double	x1 = x + 0.5f - 0.5f;
			double	z0 = z + 0.5f + 0.5f;
			double	z1 = z + 0.5f - 0.5f;

			double	x0_ = x + 0.5f - 0.5f;
			double	x1_ = x + 0.5f + 0.5f;
			double	z0_ = z + 0.5f - 0.5f;
			double	z1_ = z + 0.5f + 0.5f;

			u0 = (xt) / 256.0f;
			u1 = (xt + 15.99f) / 256.0f;
			v0 = (yt) / 256.0f;
			v1 = (yt + 15.99f) / 256.0f;

			y += 1;
			h = -0.2f;

			if ( ( ( x + y + z ) & 1 ) == 0 )
			{
				t.vertexUV( ( float )( x0_ ), ( float )( y + h ), ( float )( z +
					0 ), ( float )( u1 ), ( float )( v0 ) );
				t.vertexUV( ( float )( x0 ), ( float )( y + 0 ), ( float )( z +
					0 ), ( float )( u1 ), ( float )( v1 ) );
				t.vertexUV( ( float )( x0 ), ( float )( y + 0 ), ( float )( z +
					1 ), ( float )( u0 ), ( float )( v1 ) );
				t.vertexUV( ( float )( x0_ ), ( float )( y + h ), ( float )( z +
					1 ), ( float )( u0 ), ( float )( v0 ) );

				u0 = (xt) / 256.0f;
				u1 = (xt + 15.99f) / 256.0f;
				v0 = (yt) / 256.0f;
				v1 = (yt + 15.99f) / 256.0f;

				t.vertexUV( ( float )( x1_ ), ( float )( y + h ), ( float )( z +
					1.0f ), ( float )( u1 ), ( float )( v0 ) );
				t.vertexUV( ( float )( x1 ), ( float )( y + 0.0f ), ( float )( z +
					1.0f ), ( float )( u1 ), ( float )( v1 ) );
				t.vertexUV( ( float )( x1 ), ( float )( y + 0.0f ), ( float )( z +
					0 ), ( float )( u0 ), ( float )( v1 ) );
				t.vertexUV( ( float )( x1_ ), ( float )( y + h ), ( float )( z +
					0 ), ( float )( u0 ), ( float )( v0 ) );
			}
			else
			{
				t.vertexUV( ( float )( x + 0.0f ), ( float )( y +
					h ), ( float )( z1_ ), ( float )( u1 ), ( float )( v0 ) );
				t.vertexUV( ( float )( x + 0.0f ), ( float )( y +
					0.0f ), ( float )( z1 ), ( float )( u1 ), ( float )( v1 ) );
				t.vertexUV( ( float )( x + 1.0f ), ( float )( y +
					0.0f ), ( float )( z1 ), ( float )( u0 ), ( float )( v1 ) );
				t.vertexUV( ( float )( x + 1.0f ), ( float )( y +
					h ), ( float )( z1_ ), ( float )( u0 ), ( float )( v0 ) );

				u0 = (xt) / 256.0f;
				u1 = (xt + 15.99f) / 256.0f;
				v0 = (yt) / 256.0f;
				v1 = (yt + 15.99f) / 256.0f;

				t.vertexUV( ( float )( x + 1.0f ), ( float )( y +
					h ), ( float )( z0_ ), ( float )( u1 ), ( float )( v0 ) );
				t.vertexUV( ( float )( x + 1.0f ), ( float )( y +
					0.0f ), ( float )( z0 ), ( float )( u1 ), ( float )( v1 ) );
				t.vertexUV( ( float )( x + 0.0f ), ( float )( y +
					0.0f ), ( float )( z0 ), ( float )( u0 ), ( float )( v1 ) );
				t.vertexUV( ( float )( x + 0.0f ), ( float )( y +
					h ), ( float )( z0_ ), ( float )( u0 ), ( float )( v0 ) );
			}
		}
	}

	return true;

}


bool TileRenderer::tesselateLadderInWorld( Tile* tt, int x, int y, int z )
{
	Tesselator& t = Tesselator::instance;

	int tex = tt->getTexture(0);

	if (fixedTexture >= 0) tex = fixedTexture;

	float br = tt->getBrightness(level, x, y, z);
	t.color(br, br, br);
	int xt = ((tex & 0xf) << 4);
	int yt = tex & 0xf0;

	float u0 = (xt) / 256.0f;
	float u1 = (xt + 15.99f) / 256.0f;
	float v0 = (yt) / 256.0f;
	float v1 = (yt + 15.99f) / 256.0f;

	int face = level->getData(x, y, z);

	float o = 0 / 16.0f;
	float r = 0.05f;
	if (face == 5) {
		t.vertexUV(x + r, y + 1 + o, z + 1 + o, u0, v0);
		t.vertexUV(x + r, y + 0 - o, z + 1 + o, u0, v1);
		t.vertexUV(x + r, y + 0 - o, z + 0 - o, u1, v1);
		t.vertexUV(x + r, y + 1 + o, z + 0 - o, u1, v0);
	}
	if (face == 4) {
		t.vertexUV(x + 1 - r, y + 0 - o, z + 1 + o, u1, v1);
		t.vertexUV(x + 1 - r, y + 1 + o, z + 1 + o, u1, v0);
		t.vertexUV(x + 1 - r, y + 1 + o, z + 0 - o, u0, v0);
		t.vertexUV(x + 1 - r, y + 0 - o, z + 0 - o, u0, v1);
	}
	if (face == 3) {
		t.vertexUV(x + 1 + o, y + 0 - o, z + r, u1, v1);
		t.vertexUV(x + 1 + o, y + 1 + o, z + r, u1, v0);
		t.vertexUV(x + 0 - o, y + 1 + o, z + r, u0, v0);
		t.vertexUV(x + 0 - o, y + 0 - o, z + r, u0, v1);
	}
	if (face == 2) {
		t.vertexUV(x + 1 + o, y + 1 + o, z + 1 - r, u0, v0);
		t.vertexUV(x + 1 + o, y + 0 - o, z + 1 - r, u0, v1);
		t.vertexUV(x + 0 - o, y + 0 - o, z + 1 - r, u1, v1);
		t.vertexUV(x + 0 - o, y + 1 + o, z + 1 - r, u1, v0);
	}

	return true;
}

bool TileRenderer::tesselateCrossInWorld( Tile* tt, int x, int y, int z )
{
	Tesselator& t = Tesselator::instance;

	float br = tt->getBrightness(level, x, y, z);
	int col = tt->getColor(level, x, y, z);
	float r = ((col >> 16) & 0xff) / 255.0f;
	float g = ((col >> 8) & 0xff) / 255.0f;
	float b = ((col) & 0xff) / 255.0f;
	t.color(br * r, br * g, br * b);

	float xt = float(x);
	float yt = float(y);
	float zt = float(z);

	if (tt == Tile::tallgrass) {
		long seed = (x * 3129871) ^ (z * 116129781l) ^ (y);
		seed = seed * seed * 42317861 + seed * 11;

		xt += ((((seed >> 16) & 0xf) / 15.0f) - 0.5f) * 0.5f;
		yt += ((((seed >> 20) & 0xf) / 15.0f) - 1.0f) * 0.2f;
		zt += ((((seed >> 24) & 0xf) / 15.0f) - 0.5f) * 0.5f;
	}

	tesselateCrossTexture(tt, level->getData(x, y, z), xt, yt, zt);
	return true;
	//return true;
	/*Tesselator& t = Tesselator::instance;

	float br = tt->getBrightness(level, x, y, z);
	t.color(br, br, br);

	tesselateCrossTexture(tt, level->getData(x, y, z), (float)x, (float)y, (float)z);
	return true;*/
}
bool TileRenderer::tesselateStemInWorld( Tile* _tt, int x, int y, int z ) {
	StemTile* tt = (StemTile*) _tt;
	Tesselator& t = Tesselator::instance;

	float br = tt->getBrightness(level, x, y, z);

	int col = tt->getColor(level, x, y, z);
	float r = ((col >> 16) & 0xff) / 255.0f;
	float g = ((col >> 8) & 0xff) / 255.0f;
	float b = ((col) & 0xff) / 255.0f;

	t.color(br * r, br * g, br * b);

	tt->updateShape(level, x, y, z);
	int dir = tt->getConnectDir(level, x, y, z);
	if (dir < 0) {
		tesselateStemTexture(tt, level->getData(x, y, z), tt->yy1, float(x), float(y - 1 / 16.0f), float(z));
	} else {
		tesselateStemTexture(tt, level->getData(x, y, z), 0.5f, float(x), float(y - 1 / 16.0f), float(z));
		tesselateStemDirTexture(tt, level->getData(x, y, z), dir, tt->yy1, float(x), float(y - 1 / 16.0f), float(z));
	}
	return true;
}
void TileRenderer::tesselateTorch( Tile* tt, float x, float y, float z, float xxa, float zza )
{
	Tesselator& t = Tesselator::instance;
	int tex = tt->getTexture(0);

	if (fixedTexture >= 0) tex = fixedTexture;
	int cleanTex = tex & ~Tile::TEXTURE_ALT_FLAG;
	int xt = (cleanTex & 0xf) << 4;
	int yt = cleanTex & 0xf0;
	float u0 = (xt) / 256.0f;
	float u1 = (xt + 15.99f) / 256.0f;
	float v0 = (yt) / 256.0f;
	float v1 = (yt + 15.99f) / 256.0f;


	float uc0 = u0 + 7 / 256.0f;
	float vc0 = v0 + 6 / 256.0f;
	float uc1 = u0 + 9 / 256.0f;
	float vc1 = v0 + 8 / 256.0f;
	x += 0.5f;
	z += 0.5f;

	float x0 = x - 0.5f;
	float x1 = x + 0.5f;
	float z0 = z - 0.5f;
	float z1 = z + 0.5f;
	float r = 1 / 16.0f;

	float h = 10.0f / 16.0f;
	t.vertexUV(x + xxa * (1 - h) - r, y + h, z + zza * (1 - h) - r, uc0, vc0);
	t.vertexUV(x + xxa * (1 - h) - r, y + h, z + zza * (1 - h) + r, uc0, vc1);
	t.vertexUV(x + xxa * (1 - h) + r, y + h, z + zza * (1 - h) + r, uc1, vc1);
	t.vertexUV(x + xxa * (1 - h) + r, y + h, z + zza * (1 - h) - r, uc1, vc0);

	t.vertexUV(x - r, y + 1, z0, u0, v0);
	t.vertexUV(x - r + xxa, y + 0, z0 + zza, u0, v1);
	t.vertexUV(x - r + xxa, y + 0, z1 + zza, u1, v1);
	t.vertexUV(x - r, y + 1, z1, u1, v0);

	t.vertexUV(x + r, y + 1, z1, u0, v0);
	t.vertexUV(x + xxa + r, y + 0, z1 + zza, u0, v1);
	t.vertexUV(x + xxa + r, y + 0, z0 + zza, u1, v1);
	t.vertexUV(x + r, y + 1, z0, u1, v0);

	t.vertexUV(x0, y + 1, z + r, u0, v0);
	t.vertexUV(x0 + xxa, y + 0, z + r + zza, u0, v1);
	t.vertexUV(x1 + xxa, y + 0, z + r + zza, u1, v1);
	t.vertexUV(x1, y + 1, z + r, u1, v0);

	t.vertexUV(x1, y + 1, z - r, u0, v0);
	t.vertexUV(x1 + xxa, y + 0, z - r + zza, u0, v1);
	t.vertexUV(x0 + xxa, y + 0, z - r + zza, u1, v1);
	t.vertexUV(x0, y + 1, z - r, u1, v0);
}

void TileRenderer::tesselateCrossTexture( Tile* tt, int data, float x, float y, float z )
{
	Tesselator& t = Tesselator::instance;

	int tex = tt->getTexture(0, data);

	if (fixedTexture >= 0) tex = fixedTexture;
	int cleanTex = tex & ~Tile::TEXTURE_ALT_FLAG;
	int xt = (cleanTex & 0xf) << 4;
	int yt = cleanTex & 0xf0;
	float u0 = (xt) / 256.0f;
	float u1 = (xt + 15.99f) / 256.0f;
	float v0 = (yt) / 256.0f;
	float v1 = (yt + 15.99f) / 256.0f;

	float x0 = x + 0.5f - 0.45f;
	float x1 = x + 0.5f + 0.45f;
	float z0 = z + 0.5f - 0.45f;
	float z1 = z + 0.5f + 0.45f;

	t.vertexUV(x0, y + 1, z0, u0, v0);
	t.vertexUV(x0, y + 0, z0, u0, v1);
	t.vertexUV(x1, y + 0, z1, u1, v1);
	t.vertexUV(x1, y + 1, z1, u1, v0);

	t.vertexUV(x1, y + 1, z1, u0, v0);
	t.vertexUV(x1, y + 0, z1, u0, v1);
	t.vertexUV(x0, y + 0, z0, u1, v1);
	t.vertexUV(x0, y + 1, z0, u1, v0);

	t.vertexUV(x0, y + 1, z1, u0, v0);
	t.vertexUV(x0, y + 0, z1, u0, v1);
	t.vertexUV(x1, y + 0, z0, u1, v1);
	t.vertexUV(x1, y + 1, z0, u1, v0);

	t.vertexUV(x1, y + 1, z0, u0, v0);
	t.vertexUV(x1, y + 0, z0, u0, v1);
	t.vertexUV(x0, y + 0, z1, u1, v1);
	t.vertexUV(x0, y + 1, z1, u1, v0);
}
void TileRenderer::tesselateStemTexture( Tile* tt, int data, float h, float x, float y, float z ) {
	Tesselator& t = Tesselator::instance;
	int tex = tt->getTexture(0, data);
	if(fixedTexture >= 0) tex = fixedTexture;
	int cleanTex = tex & ~Tile::TEXTURE_ALT_FLAG;
	int xt = (cleanTex & 0xf) << 4;
	int yt = cleanTex & 0xf0;
	float u0 = (xt) / 256.0f;
	float u1 = (xt + 15.99f) / 256.0f;
	float v0 = (yt) / 256.0f;
	float v1 = (yt + 15.99f * h) / 256.0f;

	float x0 = x + 0.5f - 0.45f;
	float x1 = x + 0.5f + 0.45f;
	float z0 = z + 0.5f - 0.45f;
	float z1 = z + 0.5f + 0.45f;

	t.vertexUV(x0, y + h, z0, u0, v0);
	t.vertexUV(x0, y + 0, z0, u0, v1);
	t.vertexUV(x1, y + 0, z1, u1, v1);
	t.vertexUV(x1, y + h, z1, u1, v0);

	t.vertexUV(x1, y + h, z1, u0, v0);
	t.vertexUV(x1, y + 0, z1, u0, v1);
	t.vertexUV(x0, y + 0, z0, u1, v1);
	t.vertexUV(x0, y + h, z0, u1, v0);

	t.vertexUV(x0, y + h, z1, u0, v0);
	t.vertexUV(x0, y + 0, z1, u0, v1);
	t.vertexUV(x1, y + 0, z0, u1, v1);
	t.vertexUV(x1, y + h, z0, u1, v0);

	t.vertexUV(x1, y + h, z0, u0, v0);
	t.vertexUV(x1, y + 0, z0, u0, v1);
	t.vertexUV(x0, y + 0, z1, u1, v1);
	t.vertexUV(x0, y + h, z1, u1, v0);
}
void TileRenderer::tesselateStemDirTexture( Tile* tt, int data, int dir, float h, float x, float y, float z ) {
	Tesselator& t = Tesselator::instance;

	int tex = tt->getTexture(0, data) + 16;

	if (fixedTexture >= 0) tex = fixedTexture;
	int cleanTex = tex & ~Tile::TEXTURE_ALT_FLAG;
	int xt = (cleanTex & 0xf) << 4;
	int yt = cleanTex & 0xf0;
	float u0 = (xt) / 256.0f;
	float u1 = (xt + 15.99f) / 256.0f;
	float v0 = (yt) / 256.0f;
	float v1 = (yt + 15.99f * h) / 256.0f;

	float x0 = x + 0.5f - 0.5f;
	float x1 = x + 0.5f + 0.5f;
	float z0 = z + 0.5f - 0.5f;
	float z1 = z + 0.5f + 0.5f;

	float xm = x + 0.5f;
	float zm = z + 0.5f;

	if ((dir + 1) / 2 % 2 == 1) {
		float tmp = u1;
		u1 = u0;
		u0 = tmp;
	}

	if (dir < 2) {
		t.vertexUV(x0, y + h, zm, u0, v0);
		t.vertexUV(x0, y + 0, zm, u0, v1);
		t.vertexUV(x1, y + 0, zm, u1, v1);
		t.vertexUV(x1, y + h, zm, u1, v0);

		t.vertexUV(x1, y + h, zm, u1, v0);
		t.vertexUV(x1, y + 0, zm, u1, v1);
		t.vertexUV(x0, y + 0, zm, u0, v1);
		t.vertexUV(x0, y + h, zm, u0, v0);
	} else {

		t.vertexUV(xm, y + h, z1, u0, v0);
		t.vertexUV(xm, y + 0, z1, u0, v1);
		t.vertexUV(xm, y + 0, z0, u1, v1);
		t.vertexUV(xm, y + h, z0, u1, v0);

		t.vertexUV(xm, y + h, z0, u1, v0);
		t.vertexUV(xm, y + 0, z0, u1, v1);
		t.vertexUV(xm, y + 0, z1, u0, v1);
		t.vertexUV(xm, y + h, z1, u0, v0);
	}
}

bool TileRenderer::tesselateWaterInWorld( Tile* tt, int x, int y, int z )
{
	Tesselator& t = Tesselator::instance;

	bool up = tt->shouldRenderFace(level, x, y + 1, z, 1);
	bool down = tt->shouldRenderFace(level, x, y - 1, z, 0);

	bool dirs[4]; // static?
	dirs[0] = tt->shouldRenderFace(level, x, y, z - 1, 2);
	dirs[1] = tt->shouldRenderFace(level, x, y, z + 1, 3);
	dirs[2] = tt->shouldRenderFace(level, x - 1, y, z, 4);
	dirs[3] = tt->shouldRenderFace(level, x + 1, y, z, 5);

	if (!up && !down && !dirs[0] && !dirs[1] && !dirs[2] && !dirs[3]) return false;

	bool changed = false;
	float c10 = 0.5f;
	float c11 = 1;
	float c2 = 0.8f;
	float c3 = 0.6f;

	const float yo0 = 0;
	const float yo1 = 1;

	const Material* m = tt->material;
	int data = level->getData(x, y, z);

	float h0 = getWaterHeight(x, y, z, m);
	float h1 = getWaterHeight(x, y, z + 1, m);
	float h2 = getWaterHeight(x + 1, y, z + 1, m);
	float h3 = getWaterHeight(x + 1, y, z, m);

	// renderFaceUp(tt, x, y, z, tt->getTexture(0));
	if (noCulling || up) {
		changed = true;
		int tex = tt->getTexture(1, data);
		float angle = (float) LiquidTile::getSlopeAngle(level, x, y, z, m);
		if (angle > -999) {
			tex = tt->getTexture(2, data);
		}
		int xt = (tex & 0xf) << 4;
		int yt = tex & 0xf0;

		float uc = (xt + 0.5f * 16) / 256.0f;
		float vc = (yt + 0.5f * 16) / 256.0f;
		if (angle < -999) {
			angle = 0;
		} else {
			uc = (xt + 1 * 16) / 256.0f;
			vc = (yt + 1 * 16) / 256.0f;
		}
		float s = (Mth::sin(angle) * 8) / 256.5f; // @attn: to get rid of "jitter" (caused
		float c = (Mth::cos(angle) * 8) / 256.5f; //  of fp rounding errors) in big oceans)

		float br = tt->getBrightness(level, x, y, z);
		t.color(c11 * br, c11 * br, c11 * br);
		t.vertexUV((float)x + 0, (float)y + h0, (float)z + 0, uc - c - s, vc - c + s);
		t.vertexUV((float)x + 0, (float)y + h1, (float)z + 1, uc - c + s, vc + c + s);
		t.vertexUV((float)x + 1, (float)y + h2, (float)z + 1, uc + c + s, vc + c - s);
		t.vertexUV((float)x + 1, (float)y + h3, (float)z + 0, uc + c - s, vc - c - s);
	}

	if (noCulling || down) {
		float br = tt->getBrightness(level, x, y - 1, z);
		t.color(c10 * br, c10 * br, c10 * br);
		renderFaceDown(tt, (float)x, (float)y, (float)z, tt->getTexture(0));
		changed = true;
	}

	for (int face = 0; face < 4; face++) {
		int xt = x;
		int yt = y;
		int zt = z;

		if (face == 0) zt--;
		if (face == 1) zt++;
		if (face == 2) xt--;
		if (face == 3) xt++;

		int tex = tt->getTexture(face + 2, data);
		int xTex = (tex & 0xf) << 4;
		int yTex = tex & 0xf0;

		if (noCulling || dirs[face]) {
			float hh0;
			float hh1;
			float x0, z0, x1, z1;
			if (face == 0) {
				hh0 = h0;
				hh1 = h3;
				x0 = (float)(x    );
				x1 = (float)(x + 1);
				z0 = (float)(z    );
				z1 = (float)(z    );
			} else if (face == 1) {
				hh0 = h2;
				hh1 = h1;
				x0 = (float)(x + 1);
				x1 = (float)(x    );
				z0 = (float)(z + 1);
				z1 = (float)(z + 1);
			} else if (face == 2) {
				hh0 = h1;
				hh1 = h0;
				x0 = (float)(x    );
				x1 = (float)(x    );
				z0 = (float)(z + 1);
				z1 = (float)(z    );
			} else {
				hh0 = h3;
				hh1 = h2;
				x0 = (float)(x + 1);
				x1 = (float)(x + 1);
				z0 = (float)(z    );
				z1 = (float)(z + 1);
			}

			changed = true;
			float u0 = (xTex + 0 * 16) / 256.0f;
			float u1 = (xTex + 1 * 16 - 0.01f) / 256.0f;

			float v01 = (yTex + (1 - hh0) * 16) / 256.0f;
			float v02 = (yTex + (1 - hh1) * 16) / 256.0f;
			float v1 = (yTex + 1 * 16 - 0.01f) / 256.0f;

			float br = tt->getBrightness(level, xt, yt, zt);
			if (face < 2) br *= c2;
			else br *= c3;

			float yf = (float)y;
			t.color(c11 * br, c11 * br, c11 * br);
			t.vertexUV(x0, yf + hh0, z0, u0, v01);
			t.vertexUV(x1, yf + hh1, z1, u1, v02);
			t.vertexUV(x1, yf + 0, z1, u1, v1);
			t.vertexUV(x0, yf + 0, z0, u0, v1);
		}
	}

	//printf("w: %d ", (dirs[0] + dirs[1] + dirs[2] + dirs[3] + up + down));

	tt->yy0 = yo0;
	tt->yy1 = yo1;

	return changed;
}

float TileRenderer::getWaterHeight( int x, int y, int z, const Material* m )
{
	int count = 0;
	float h = 0;
	for (int i = 0; i < 4; i++) {
		int xx = x - (i & 1);
		int yy = y;
		int zz = z - ((i >> 1) & 1);
		if (level->getMaterial(xx, yy + 1, zz) == m) {
			return 1;
		}
		const Material* tm = level->getMaterial(xx, yy, zz);
		if (tm == m) {
			int d = level->getData(xx, yy, zz);
			if (d >= 8 || d == 0) {
				h += (LiquidTile::getHeight(d)) * 10;
				count += 10;
			}
			h += LiquidTile::getHeight(d);
			count++;
		} else if (!tm->isSolid()) {
			h += 1;
			count++;
		}
	}
	return 1 - h / count;
}

void TileRenderer::renderBlock(Tile* tt, LevelSource* level, int x, int y, int z) {
	float c10 = 0.5f;
	float c11 = 1;
	float c2 = 0.8f;
	float c3 = 0.6f;

	Tesselator& t = Tesselator::instance;
	t.begin();

	float center = tt->getBrightness(level, x, y, z);
	float br = tt->getBrightness(level, x, y - 1, z);
	if (br < center) br = center;

	t.color(c10 * br, c10 * br, c10 * br);
	renderFaceDown(tt, -0.5f, -0.5f, -0.5f, tt->getTexture(0));

	br = tt->getBrightness(level, x, y + 1, z);
	if (br < center) br = center;
	t.color(c11 * br, c11 * br, c11 * br);
	renderFaceUp(tt, -0.5f, -0.5f, -0.5f, tt->getTexture(1));

	br = tt->getBrightness(level, x, y, z - 1);
	if (br < center) br = center;
	t.color(c2 * br, c2 * br, c2 * br);
	renderNorth(tt, -0.5f, -0.5f, -0.5f, tt->getTexture(2));

	br = tt->getBrightness(level, x, y, z + 1);
	if (br < center) br = center;
	t.color(c2 * br, c2 * br, c2 * br);
	renderSouth(tt, -0.5f, -0.5f, -0.5f, tt->getTexture(3));

	br = tt->getBrightness(level, x - 1, y, z);
	if (br < center) br = center;
	t.color(c3 * br, c3 * br, c3 * br);
	renderWest(tt, -0.5f, -0.5f, -0.5f, tt->getTexture(4));

	br = tt->getBrightness(level, x + 1, y, z);
	if (br < center) br = center;
	t.color(c3 * br, c3 * br, c3 * br);
	renderEast(tt, -0.5f, -0.5f, -0.5f, tt->getTexture(5));
	t.draw();
}

bool TileRenderer::tesselateBlockInWorldWithAmbienceOcclusion( Tile* tt, int pX, int pY, int pZ, float pBaseRed, float pBaseGreen, float pBaseBlue )
{
	applyAmbienceOcclusion = true;
	bool isJungleWorld = (tt == (Tile*)Tile::jungleLeaves || (Tile::leaves && tt == (Tile*)Tile::leaves && level->getData(pX, pY, pZ) == LeafTile::JUNGLE_LEAF));
	bool i = false;
	float ll1 = ll000;
	float ll2 = ll000;
	float ll3 = ll000;
	float ll4 = ll000;
	bool tint0 = true;
	bool tint1 = true;
	bool tint2 = true;
	bool tint3 = true;
	bool tint4 = true;
	bool tint5 = true;

	ll000 = tt->getBrightness(level, pX, pY, pZ);
	llx00 = tt->getBrightness(level, pX - 1, pY, pZ);
	ll0y0 = tt->getBrightness(level, pX, pY - 1, pZ);
	ll00z = tt->getBrightness(level, pX, pY, pZ - 1);
	llX00 = tt->getBrightness(level, pX + 1, pY, pZ);
	ll0Y0 = tt->getBrightness(level, pX, pY + 1, pZ);
	ll00Z = tt->getBrightness(level, pX, pY, pZ + 1);

	llTransXY0 = Tile::translucent[level->getTile(pX + 1, pY + 1, pZ)];
	llTransXy0 = Tile::translucent[level->getTile(pX + 1, pY - 1, pZ)];
	llTransX0Z = Tile::translucent[level->getTile(pX + 1, pY, pZ + 1)];
	llTransX0z = Tile::translucent[level->getTile(pX + 1, pY, pZ - 1)];
	llTransxY0 = Tile::translucent[level->getTile(pX - 1, pY + 1, pZ)];
	llTransxy0 = Tile::translucent[level->getTile(pX - 1, pY - 1, pZ)];
	llTransx0z = Tile::translucent[level->getTile(pX - 1, pY, pZ - 1)];
	llTransx0Z = Tile::translucent[level->getTile(pX - 1, pY, pZ + 1)];
	llTrans0YZ = Tile::translucent[level->getTile(pX, pY + 1, pZ + 1)];
	llTrans0Yz = Tile::translucent[level->getTile(pX, pY + 1, pZ - 1)];
	llTrans0yZ = Tile::translucent[level->getTile(pX, pY - 1, pZ + 1)];
	llTrans0yz = Tile::translucent[level->getTile(pX, pY - 1, pZ - 1)];

	if (tt->tex == 3) tint0 = tint2 = tint3 = tint4 = tint5 = false;

	if ((noCulling) || (tt->shouldRenderFace(level, pX, pY - 1, pZ, 0))) {
		if (blsmooth > 0) {
			pY--;

			llxy0 = tt->getBrightness(level, pX - 1, pY, pZ);
			ll0yz = tt->getBrightness(level, pX, pY, pZ - 1);
			ll0yZ = tt->getBrightness(level, pX, pY, pZ + 1);
			llXy0 = tt->getBrightness(level, pX + 1, pY, pZ);

			if (llTrans0yz || llTransxy0) {
				llxyz = tt->getBrightness(level, pX - 1, pY, pZ - 1);
			} else {
				llxyz = llxy0;
			}
			if (llTrans0yZ || llTransxy0) {
				llxyZ = tt->getBrightness(level, pX - 1, pY, pZ + 1);
			} else {
				llxyZ = llxy0;
			}
			if (llTrans0yz || llTransXy0) {
				llXyz = tt->getBrightness(level, pX + 1, pY, pZ - 1);
			} else {
				llXyz = llXy0;
			}
			if (llTrans0yZ || llTransXy0) {
				llXyZ = tt->getBrightness(level, pX + 1, pY, pZ + 1);
			} else {
				llXyZ = llXy0;
			}

			pY++;
			ll1 = (llxyZ + llxy0 + ll0yZ + ll0y0) / 4.0f;
			ll4 = (ll0yZ + ll0y0 + llXyZ + llXy0) / 4.0f;
			ll3 = (ll0y0 + ll0yz + llXy0 + llXyz) / 4.0f;
			ll2 = (llxy0 + llxyz + ll0y0 + ll0yz) / 4.0f;
		} else ll1 = ll2 = ll3 = ll4 = ll0y0;
		c1r = c2r = c3r = c4r = (tint0 ? pBaseRed : 1.0f) * 0.5f;
		c1g = c2g = c3g = c4g = (tint0 ? pBaseGreen : 1.0f) * 0.5f;
		c1b = c2b = c3b = c4b = (tint0 ? pBaseBlue : 1.0f) * 0.5f;
		c1r *= ll1;
		c1g *= ll1;
		c1b *= ll1;
		c2r *= ll2;
		c2g *= ll2;
		c2b *= ll2;
		c3r *= ll3;
		c3g *= ll3;
		c3b *= ll3;
		c4r *= ll4;
		c4g *= ll4;
		c4b *= ll4;

		{
			int tex0 = tt->getTexture(level, pX, pY, pZ, 0);
			bool isAlt0 = (tex0 & Tile::TEXTURE_ALT_FLAG) != 0;
			if (atlasFilter == -1 || (atlasFilter == 0 && !isAlt0) || (atlasFilter == 1 && isAlt0)) {
				renderFaceDown(tt, (float) pX, (float) pY, (float) pZ, tex0 & ~Tile::TEXTURE_ALT_FLAG);
				i = true;
			}
			if (isJungleWorld && (atlasFilter == -1 || atlasFilter == 1)) {
				c1r = c2r = c3r = c4r = 0.5f;
				c1g = c2g = c3g = c4g = 0.5f;
				c1b = c2b = c3b = c4b = 0.5f;
				c1r *= ll1; c1g *= ll1; c1b *= ll1;
				c2r *= ll2; c2g *= ll2; c2b *= ll2;
				c3r *= ll3; c3g *= ll3; c3b *= ll3;
				c4r *= ll4; c4g *= ll4; c4b *= ll4;
				renderFaceDown(tt, (float) pX, (float) pY, (float) pZ, 55);
			}
		}
	}
	if ((noCulling) || (tt->shouldRenderFace(level, pX, pY + 1, pZ, 1))) {
		if (blsmooth > 0) {
			pY++;

			llxY0 = tt->getBrightness(level, pX - 1, pY, pZ);
			llXY0 = tt->getBrightness(level, pX + 1, pY, pZ);
			ll0Yz = tt->getBrightness(level, pX, pY, pZ - 1);
			ll0YZ = tt->getBrightness(level, pX, pY, pZ + 1);

			if (llTrans0Yz || llTransxY0) {
				llxYz = tt->getBrightness(level, pX - 1, pY, pZ - 1);
			} else {
				llxYz = llxY0;
			}
			if (llTrans0Yz || llTransXY0) {
				llXYz = tt->getBrightness(level, pX + 1, pY, pZ - 1);
			} else {
				llXYz = llXY0;
			}
			if (llTrans0YZ || llTransxY0) {
				llxYZ = tt->getBrightness(level, pX - 1, pY, pZ + 1);
			} else {
				llxYZ = llxY0;
			}
			if (llTrans0YZ || llTransXY0) {
				llXYZ = tt->getBrightness(level, pX + 1, pY, pZ + 1);
			} else {
				llXYZ = llXY0;
			}
			pY--;

			ll4 = (llxYZ + llxY0 + ll0YZ + ll0Y0) / 4.0f;
			ll1 = (ll0YZ + ll0Y0 + llXYZ + llXY0) / 4.0f;
			ll2 = (ll0Y0 + ll0Yz + llXY0 + llXYz) / 4.0f;
			ll3 = (llxY0 + llxYz + ll0Y0 + ll0Yz) / 4.0f;
		} else ll1 = ll2 = ll3 = ll4 = ll0Y0;
		c1r = c2r = c3r = c4r = (tint1 ? pBaseRed : 1.0f);
		c1g = c2g = c3g = c4g = (tint1 ? pBaseGreen : 1.0f);
		c1b = c2b = c3b = c4b = (tint1 ? pBaseBlue : 1.0f);
		c1r *= ll1;
		c1g *= ll1;
		c1b *= ll1;
		c2r *= ll2;
		c2g *= ll2;
		c2b *= ll2;
		c3r *= ll3;
		c3g *= ll3;
		c3b *= ll3;
		c4r *= ll4;
		c4g *= ll4;
		c4b *= ll4;
		{
			int tex1 = tt->getTexture(level, pX, pY, pZ, 1);
			bool isAlt1 = (tex1 & Tile::TEXTURE_ALT_FLAG) != 0;
			if (atlasFilter == -1 || (atlasFilter == 0 && !isAlt1) || (atlasFilter == 1 && isAlt1)) {
				renderFaceUp(tt, (float) pX, (float) pY, (float) pZ, tex1 & ~Tile::TEXTURE_ALT_FLAG);
				i = true;
			}
			if (isJungleWorld && (atlasFilter == -1 || atlasFilter == 1)) {
				c1r = c2r = c3r = c4r = 1.0f;
				c1g = c2g = c3g = c4g = 1.0f;
				c1b = c2b = c3b = c4b = 1.0f;
				c1r *= ll1; c1g *= ll1; c1b *= ll1;
				c2r *= ll2; c2g *= ll2; c2b *= ll2;
				c3r *= ll3; c3g *= ll3; c3b *= ll3;
				c4r *= ll4; c4g *= ll4; c4b *= ll4;
				renderFaceUp(tt, (float) pX, (float) pY, (float) pZ, 55);
			}
		}
	}
	if ((noCulling) || (tt->shouldRenderFace(level, pX, pY, pZ - 1, 2))) {
		if (blsmooth > 0) {
			pZ--;
			llx0z = tt->getBrightness(level, pX - 1, pY, pZ);
			ll0yz = tt->getBrightness(level, pX, pY - 1, pZ);
			ll0Yz = tt->getBrightness(level, pX, pY + 1, pZ);
			llX0z = tt->getBrightness(level, pX + 1, pY, pZ);

			if (llTransx0z || llTrans0yz) {
				llxyz = tt->getBrightness(level, pX - 1, pY - 1, pZ);
			} else {
				llxyz = llx0z;
			}
			if (llTransx0z || llTrans0Yz) {
				llxYz = tt->getBrightness(level, pX - 1, pY + 1, pZ);
			} else {
				llxYz = llx0z;
			}
			if (llTransX0z || llTrans0yz) {
				llXyz = tt->getBrightness(level, pX + 1, pY - 1, pZ);
			} else {
				llXyz = llX0z;
			}
			if (llTransX0z || llTrans0Yz) {
				llXYz = tt->getBrightness(level, pX + 1, pY + 1, pZ);
			} else {
				llXYz = llX0z;
			}
			pZ++;
			ll1 = (llx0z + llxYz + ll00z + ll0Yz) / 4.0f;
			ll2 = (ll00z + ll0Yz + llX0z + llXYz) / 4.0f;
			ll3 = (ll0yz + ll00z + llXyz + llX0z) / 4.0f;
			ll4 = (llxyz + llx0z + ll0yz + ll00z) / 4.0f;
		} else ll1 = ll2 = ll3 = ll4 = ll00z;
		c1r = c2r = c3r = c4r = (tint2 ? pBaseRed : 1.0f) * 0.8f;
		c1g = c2g = c3g = c4g = (tint2 ? pBaseGreen : 1.0f) * 0.8f;
		c1b = c2b = c3b = c4b = (tint2 ? pBaseBlue : 1.0f) * 0.8f;
		c1r *= ll1;
		c1g *= ll1;
		c1b *= ll1;
		c2r *= ll2;
		c2g *= ll2;
		c2b *= ll2;
		c3r *= ll3;
		c3g *= ll3;
		c3b *= ll3;
		c4r *= ll4;
		c4g *= ll4;
		c4b *= ll4;
		{
			int tex2 = tt->getTexture(level, pX, pY, pZ, 2);
			bool isAlt2 = (tex2 & Tile::TEXTURE_ALT_FLAG) != 0;
			if (atlasFilter == -1 || (atlasFilter == 0 && !isAlt2) || (atlasFilter == 1 && isAlt2)) {
				int cleanTex2 = tex2 & ~Tile::TEXTURE_ALT_FLAG;
				int baseTex2 = (cleanTex2 == 3 && sideTinting) ? 236 : cleanTex2;
				renderNorth(tt, (float) pX, (float) pY, (float) pZ, baseTex2);
				if (cleanTex2 == 3 && sideTinting) 
				{
					c1r *= pBaseRed; c1g *= pBaseGreen; c1b *= pBaseBlue;
					c2r *= pBaseRed; c2g *= pBaseGreen; c2b *= pBaseBlue;
					c3r *= pBaseRed; c3g *= pBaseGreen; c3b *= pBaseBlue;
					c4r *= pBaseRed; c4g *= pBaseGreen; c4b *= pBaseBlue;

					renderNorth(tt, (float) pX, (float) pY, (float) pZ - 0.001f, 38);
				}
				if (isJungleWorld && (atlasFilter == -1 || atlasFilter == 1)) {
					c1r = c2r = c3r = c4r = 0.8f;
					c1g = c2g = c3g = c4g = 0.8f;
					c1b = c2b = c3b = c4b = 0.8f;
					c1r *= ll1; c1g *= ll1; c1b *= ll1;
					c2r *= ll2; c2g *= ll2; c2b *= ll2;
					c3r *= ll3; c3g *= ll3; c3b *= ll3;
					c4r *= ll4; c4g *= ll4; c4b *= ll4;
					renderNorth(tt, (float) pX, (float) pY, (float) pZ - 0.0005f, 55);
				}
				i = true;
			}
		}
	}
	if ((noCulling) || (tt->shouldRenderFace(level, pX, pY, pZ + 1, 3))) {
		if (blsmooth > 0) {
			pZ++;

			llx0Z = tt->getBrightness(level, pX - 1, pY, pZ);
			llX0Z = tt->getBrightness(level, pX + 1, pY, pZ);
			ll0yZ = tt->getBrightness(level, pX, pY - 1, pZ);
			ll0YZ = tt->getBrightness(level, pX, pY + 1, pZ);

			if (llTransx0Z || llTrans0yZ) {
				llxyZ = tt->getBrightness(level, pX - 1, pY - 1, pZ);
			} else {
				llxyZ = llx0Z;
			}
			if (llTransx0Z || llTrans0YZ) {
				llxYZ = tt->getBrightness(level, pX - 1, pY + 1, pZ);
			} else {
				llxYZ = llx0Z;
			}
			if (llTransX0Z || llTrans0yZ) {
				llXyZ = tt->getBrightness(level, pX + 1, pY - 1, pZ);
			} else {
				llXyZ = llX0Z;
			}
			if (llTransX0Z || llTrans0YZ) {
				llXYZ = tt->getBrightness(level, pX + 1, pY + 1, pZ);
			} else {
				llXYZ = llX0Z;
			}
			pZ--;
			ll1 = (llx0Z + llxYZ + ll00Z + ll0YZ) / 4.0f;
			ll4 = (ll00Z + ll0YZ + llX0Z + llXYZ) / 4.0f;
			ll3 = (ll0yZ + ll00Z + llXyZ + llX0Z) / 4.0f;
			ll2 = (llxyZ + llx0Z + ll0yZ + ll00Z) / 4.0f;
		} else ll1 = ll2 = ll3 = ll4 = ll00Z;
		c1r = c2r = c3r = c4r = (tint3 ? pBaseRed : 1.0f) * 0.8f;
		c1g = c2g = c3g = c4g = (tint3 ? pBaseGreen : 1.0f) * 0.8f;
		c1b = c2b = c3b = c4b = (tint3 ? pBaseBlue : 1.0f) * 0.8f;
		c1r *= ll1;
		c1g *= ll1;
		c1b *= ll1;
		c2r *= ll2;
		c2g *= ll2;
		c2b *= ll2;
		c3r *= ll3;
		c3g *= ll3;
		c3b *= ll3;
		c4r *= ll4;
		c4g *= ll4;
		c4b *= ll4;
		{
			int tex3 = tt->getTexture(level, pX, pY, pZ, 3);
			bool isAlt3 = (tex3 & Tile::TEXTURE_ALT_FLAG) != 0;
			if (atlasFilter == -1 || (atlasFilter == 0 && !isAlt3) || (atlasFilter == 1 && isAlt3)) {
				int cleanTex3 = tex3 & ~Tile::TEXTURE_ALT_FLAG;
				int baseTex3 = (cleanTex3 == 3 && sideTinting) ? 236 : cleanTex3;
				renderSouth(tt, (float) pX, (float) pY, (float) pZ, baseTex3);
				if (cleanTex3 == 3 && sideTinting) 
				{
					c1r *= pBaseRed; c1g *= pBaseGreen; c1b *= pBaseBlue;
					c2r *= pBaseRed; c2g *= pBaseGreen; c2b *= pBaseBlue;
					c3r *= pBaseRed; c3g *= pBaseGreen; c3b *= pBaseBlue;
					c4r *= pBaseRed; c4g *= pBaseGreen; c4b *= pBaseBlue;

					renderSouth(tt, (float) pX, (float) pY, (float) pZ + 0.001f, 38);
				}
				if (isJungleWorld && (atlasFilter == -1 || atlasFilter == 1)) {
					c1r = c2r = c3r = c4r = 0.8f;
					c1g = c2g = c3g = c4g = 0.8f;
					c1b = c2b = c3b = c4b = 0.8f;
					c1r *= ll1; c1g *= ll1; c1b *= ll1;
					c2r *= ll2; c2g *= ll2; c2b *= ll2;
					c3r *= ll3; c3g *= ll3; c3b *= ll3;
					c4r *= ll4; c4g *= ll4; c4b *= ll4;
					renderSouth(tt, (float) pX, (float) pY, (float) pZ + 0.0005f, 55);
				}
				i = true;
			}
		}
	}
	if ((noCulling) || (tt->shouldRenderFace(level, pX - 1, pY, pZ, 4))) {
		if (blsmooth > 0) {
			pX--;
			llxy0 = tt->getBrightness(level, pX, pY - 1, pZ);
			llx0z = tt->getBrightness(level, pX, pY, pZ - 1);
			llx0Z = tt->getBrightness(level, pX, pY, pZ + 1);
			llxY0 = tt->getBrightness(level, pX, pY + 1, pZ);

			if (llTransx0z || llTransxy0) {
				llxyz = tt->getBrightness(level, pX, pY - 1, pZ - 1);
			} else {
				llxyz = llx0z;
			}
			if (llTransx0Z || llTransxy0) {
				llxyZ = tt->getBrightness(level, pX, pY - 1, pZ + 1);
			} else {
				llxyZ = llx0Z;
			}
			if (llTransx0z || llTransxY0) {
				llxYz = tt->getBrightness(level, pX, pY + 1, pZ - 1);
			} else {
				llxYz = llx0z;
			}
			if (llTransx0Z || llTransxY0) {
				llxYZ = tt->getBrightness(level, pX, pY + 1, pZ + 1);
			} else {
				llxYZ = llx0Z;
			}
			pX++;
			ll4 = (llxy0 + llxyZ + llx00 + llx0Z) / 4.0f;
			ll1 = (llx00 + llx0Z + llxY0 + llxYZ) / 4.0f;
			ll2 = (llx0z + llx00 + llxYz + llxY0) / 4.0f;
			ll3 = (llxyz + llxy0 + llx0z + llx00) / 4.0f;
		} else ll1 = ll2 = ll3 = ll4 = llx00;
		c1r = c2r = c3r = c4r = (tint4 ? pBaseRed : 1.0f) * 0.6f;
		c1g = c2g = c3g = c4g = (tint4 ? pBaseGreen : 1.0f) * 0.6f;
		c1b = c2b = c3b = c4b = (tint4 ? pBaseBlue : 1.0f) * 0.6f;
		c1r *= ll1;
		c1g *= ll1;
		c1b *= ll1;
		c2r *= ll2;
		c2g *= ll2;
		c2b *= ll2;
		c3r *= ll3;
		c3g *= ll3;
		c3b *= ll3;
		c4r *= ll4;
		c4g *= ll4;
		c4b *= ll4;
		{
			int tex4 = tt->getTexture(level, pX, pY, pZ, 4);
			bool isAlt4 = (tex4 & Tile::TEXTURE_ALT_FLAG) != 0;
			if (atlasFilter == -1 || (atlasFilter == 0 && !isAlt4) || (atlasFilter == 1 && isAlt4)) {
				int cleanTex4 = tex4 & ~Tile::TEXTURE_ALT_FLAG;
				int baseTex4 = (cleanTex4 == 3 && sideTinting) ? 236 : cleanTex4;
				renderWest(tt, (float) pX, (float) pY, (float) pZ, baseTex4);
				if (cleanTex4 == 3 && sideTinting) 
				{
					c1r *= pBaseRed; c1g *= pBaseGreen; c1b *= pBaseBlue;
					c2r *= pBaseRed; c2g *= pBaseGreen; c2b *= pBaseBlue;
					c3r *= pBaseRed; c3g *= pBaseGreen; c3b *= pBaseBlue;
					c4r *= pBaseRed; c4g *= pBaseGreen; c4b *= pBaseBlue;

					renderWest(tt, (float) pX - 0.001f, (float) pY, (float) pZ, 38);
				}
				if (isJungleWorld && (atlasFilter == -1 || atlasFilter == 1)) {
					c1r = c2r = c3r = c4r = 0.6f;
					c1g = c2g = c3g = c4g = 0.6f;
					c1b = c2b = c3b = c4b = 0.6f;
					c1r *= ll1; c1g *= ll1; c1b *= ll1;
					c2r *= ll2; c2g *= ll2; c2b *= ll2;
					c3r *= ll3; c3g *= ll3; c3b *= ll3;
					c4r *= ll4; c4g *= ll4; c4b *= ll4;
					renderWest(tt, (float) pX - 0.0005f, (float) pY, (float) pZ, 55);
				}
				i = true;
			}
		}
	}
	if ((noCulling) || (tt->shouldRenderFace(level, pX + 1, pY, pZ, 5))) {
		if (blsmooth > 0) {
			pX++;
			llXy0 = tt->getBrightness(level, pX, pY - 1, pZ);
			llX0z = tt->getBrightness(level, pX, pY, pZ - 1);
			llX0Z = tt->getBrightness(level, pX, pY, pZ + 1);
			llXY0 = tt->getBrightness(level, pX, pY + 1, pZ);

			if (llTransXy0 || llTransX0z) {
				llXyz = tt->getBrightness(level, pX, pY - 1, pZ - 1);
			} else {
				llXyz = llX0z;
			}
			if (llTransXy0 || llTransX0Z) {
				llXyZ = tt->getBrightness(level, pX, pY - 1, pZ + 1);
			} else {
				llXyZ = llX0Z;
			}
			if (llTransXY0 || llTransX0z) {
				llXYz = tt->getBrightness(level, pX, pY + 1, pZ - 1);
			} else {
				llXYz = llX0z;
			}
			if (llTransXY0 || llTransX0Z) {
				llXYZ = tt->getBrightness(level, pX, pY + 1, pZ + 1);
			} else {
				llXYZ = llX0Z;
			}
			pX--;
			ll1 = (llXy0 + llXyZ + llX00 + llX0Z) / 4.0f;
			ll4 = (llX00 + llX0Z + llXY0 + llXYZ) / 4.0f;
			ll3 = (llX0z + llX00 + llXYz + llXY0) / 4.0f;
			ll2 = (llXyz + llXy0 + llX0z + llX00) / 4.0f;
		} else ll1 = ll2 = ll3 = ll4 = llX00;
		c1r = c2r = c3r = c4r = (tint5 ? pBaseRed : 1.0f) * 0.6f;
		c1g = c2g = c3g = c4g = (tint5 ? pBaseGreen : 1.0f) * 0.6f;
		c1b = c2b = c3b = c4b = (tint5 ? pBaseBlue : 1.0f) * 0.6f;
		c1r *= ll1;
		c1g *= ll1;
		c1b *= ll1;
		c2r *= ll2;
		c2g *= ll2;
		c2b *= ll2;
		c3r *= ll3;
		c3g *= ll3;
		c3b *= ll3;
		c4r *= ll4;
		c4g *= ll4;
		c4b *= ll4;

		{
			int tex5 = tt->getTexture(level, pX, pY, pZ, 5);
			bool isAlt5 = (tex5 & Tile::TEXTURE_ALT_FLAG) != 0;
			if (atlasFilter == -1 || (atlasFilter == 0 && !isAlt5) || (atlasFilter == 1 && isAlt5)) {
				int cleanTex5 = tex5 & ~Tile::TEXTURE_ALT_FLAG;
				int baseTex5 = (cleanTex5 == 3 && sideTinting) ? 236 : cleanTex5;
				renderEast(tt, (float) pX, (float) pY, (float) pZ, baseTex5);
				if (cleanTex5 == 3 && sideTinting) 
				{
					c1r *= pBaseRed; c1g *= pBaseGreen; c1b *= pBaseBlue;
					c2r *= pBaseRed; c2g *= pBaseGreen; c2b *= pBaseBlue;
					c3r *= pBaseRed; c3g *= pBaseGreen; c3b *= pBaseBlue;
					c4r *= pBaseRed; c4g *= pBaseGreen; c4b *= pBaseBlue;

					renderEast(tt, (float) pX + 0.001f, (float) pY, (float) pZ, 38);
				}
				if (isJungleWorld && (atlasFilter == -1 || atlasFilter == 1)) {
					c1r = c2r = c3r = c4r = 0.6f;
					c1g = c2g = c3g = c4g = 0.6f;
					c1b = c2b = c3b = c4b = 0.6f;
					c1r *= ll1; c1g *= ll1; c1b *= ll1;
					c2r *= ll2; c2g *= ll2; c2b *= ll2;
					c3r *= ll3; c3g *= ll3; c3b *= ll3;
					c4r *= ll4; c4g *= ll4; c4b *= ll4;
					renderEast(tt, (float) pX + 0.0005f, (float) pY, (float) pZ, 55);
				}
				i = true;
			}
		}
	}
	applyAmbienceOcclusion = false;
	return i;
}

bool TileRenderer::tesselateCactusInWorld(Tile* tt, int x, int y, int z) {
	int col = tt->getColor(level, x, y, z);
	float r = ((col >> 16) & 0xff) / 255.0f;
	float g = ((col >> 8) & 0xff) / 255.0f;
	float b = ((col) & 0xff) / 255.0f;
	return tesselateCactusInWorld(tt, x, y, z, r, g, b);
}

bool TileRenderer::tesselateCactusInWorld(Tile* tt, int x, int y, int z, float r, float g, float b) {
	Tesselator& t = Tesselator::instance;

	bool changed = false;
	float c10 = 0.5f;
	float c11 = 1;
	float c2 = 0.8f;
	float c3 = 0.6f;

	float r10 = c10 * r;
	float r11 = c11 * r;
	float r2 = c2 * r;
	float r3 = c3 * r;

	float g10 = c10 * g;
	float g11 = c11 * g;
	float g2 = c2 * g;
	float g3 = c3 * g;

	float b10 = c10 * b;
	float b11 = c11 * b;
	float b2 = c2 * b;
	float b3 = c3 * b;

	float s = 1 / 16.0f;
	const float X = (float)x;
	const float Y = (float)y;
	const float Z = (float)z;

	float centerBrightness = tt->getBrightness(level, x, y, z);

	if (noCulling || tt->shouldRenderFace(level, x, y - 1, z, 0)) {
		float br = tt->getBrightness(level, x, y - 1, z);
		// if (Tile::lightEmission[tt->id] > br*Level.MAX_BRIGHTNESS) br =
		// Tile::lightEmission[tt->id]/Level.MAX_BRIGHTNESS;
		t.color(r10 * br, g10 * br, b10 * br);
		renderFaceDown(tt, X, Y, Z, tt->getTexture(level, x, y, z, 0));
		changed = true;
	}

	if (noCulling || tt->shouldRenderFace(level, x, y + 1, z, 1)) {
		float br = tt->getBrightness(level, x, y + 1, z);
		if (tt->yy1 != 1 && !tt->material->isLiquid()) br = centerBrightness;
		// if (Tile::lightEmission[tt->id] > br*Level.MAX_BRIGHTNESS) br =
		// Tile::lightEmission[tt->id]/Level.MAX_BRIGHTNESS;
		t.color(r11 * br, g11 * br, b11 * br);
		renderFaceUp(tt, X, Y, Z, tt->getTexture(level, x, y, z, 1));
		changed = true;
	}

	if (noCulling || tt->shouldRenderFace(level, x, y, z - 1, 2)) {
		float br = tt->getBrightness(level, x, y, z - 1);
		if (tt->zz0 > 0) br = centerBrightness;
		// if (Tile::lightEmission[tt->id] > br*Level.MAX_BRIGHTNESS) br =
		// Tile::lightEmission[tt->id]/Level.MAX_BRIGHTNESS;
		t.color(r2 * br, g2 * br, b2 * br);
		t.addOffset(0, 0, s);
		renderNorth(tt, X, Y, Z, tt->getTexture(level, x, y, z, 2));
		t.addOffset(0, 0, -s);
		changed = true;
	}

	if (noCulling || tt->shouldRenderFace(level, x, y, z + 1, 3)) {
		float br = tt->getBrightness(level, x, y, z + 1);
		if (tt->zz1 < 1) br = centerBrightness;
		// if (Tile::lightEmission[tt->id] > br*Level.MAX_BRIGHTNESS) br =
		// Tile::lightEmission[tt->id]/Level.MAX_BRIGHTNESS;
		t.color(r2 * br, g2 * br, b2 * br);
		t.addOffset(0, 0, -s);
		renderSouth(tt, X, Y, Z, tt->getTexture(level, x, y, z, 3));
		t.addOffset(0, 0, s);
		changed = true;
	}

	if (noCulling || tt->shouldRenderFace(level, x - 1, y, z, 4)) {
		float br = tt->getBrightness(level, x - 1, y, z);
		if (tt->xx0 > 0) br = centerBrightness;
		// if (Tile::lightEmission[tt->id] > br*Level.MAX_BRIGHTNESS) br =
		// Tile::lightEmission[tt->id]/Level.MAX_BRIGHTNESS;
		t.color(r3 * br, g3 * br, b3 * br);
		t.addOffset(s, 0, 0);
		renderWest(tt, X, Y, Z, tt->getTexture(level, x, y, z, 4));
		t.addOffset(-s, 0, 0);
		changed = true;
	}

	if (noCulling || tt->shouldRenderFace(level, x + 1, y, z, 5)) {
		float br = tt->getBrightness(level, x + 1, y, z);
		if (tt->xx1 < 1) br = centerBrightness;
		// if (Tile::lightEmission[tt->id] > br*Level.MAX_BRIGHTNESS) br =
		// Tile::lightEmission[tt->id]/Level.MAX_BRIGHTNESS;
		t.color(r3 * br, g3 * br, b3 * br);
		t.addOffset(-s, 0, 0);
		renderEast(tt, X, Y, Z, tt->getTexture(level, x, y, z, 5));
		t.addOffset(s, 0, 0);
		changed = true;
	}

	return changed;
}

bool TileRenderer::tesselateFenceInWorld(FenceTile* tt, int x, int y, int z) {
	bool changed = true;

	float a = 6 / 16.0f;
	float b = 10 / 16.0f;
	tt->setShape(a, 0, a, b, 1, b);
	tesselateBlockInWorld(tt, x, y, z);

	bool vertical = false;
	bool horizontal = false;

	bool l = tt->connectsTo(level, x - 1, y, z);
	bool r = tt->connectsTo(level, x + 1, y, z);
	bool u = tt->connectsTo(level, x, y, z - 1);
	bool d = tt->connectsTo(level, x, y, z + 1);

	if (l || r) vertical = true;
	if (u || d) horizontal = true;

	if (!vertical && !horizontal) vertical = true;

	a = 7 / 16.0f;
	b = 9 / 16.0f;
	float h0 = 12 / 16.0f;
	float h1 = 15 / 16.0f;

	float x0 = l ? 0 : a;
	float x1 = r ? 1 : b;
	float z0 = u ? 0 : a;
	float z1 = d ? 1 : b;

	if (vertical) {
		tt->setShape(x0, h0, a, x1, h1, b);
		tesselateBlockInWorld(tt, x, y, z);
	}
	if (horizontal) {
		tt->setShape(a, h0, z0, b, h1, z1);
		tesselateBlockInWorld(tt, x, y, z);
	}

	h0 = 6 / 16.0f;
	h1 = 9 / 16.0f;
	if (vertical) {
		tt->setShape(x0, h0, a, x1, h1, b);
		tesselateBlockInWorld(tt, x, y, z);
	}
	if (horizontal) {
		tt->setShape(a, h0, z0, b, h1, z1);
		tesselateBlockInWorld(tt, x, y, z);
	}

	tt->setShape(0, 0, 0, 1, 1, 1);
	return changed;
}

bool TileRenderer::tesselateFenceGateInWorld(FenceGateTile* tt, int x, int y, int z) {
	bool changed = true;

	int data = level->getData(x, y, z);
	bool isOpen = FenceGateTile::isOpen(data);
	int direction = FenceGateTile::getDirection(data);

	const float h00 = 6 / 16.0f;
	const float h01 = 9 / 16.0f;
	const float h10 = 12 / 16.0f;
	const float h11 = 15 / 16.0f;
	const float h20 = 5 / 16.0f;
	const float h21 = 16 / 16.0f;

	// edge sticks
	if (direction == Direction::EAST || direction == Direction::WEST) {
		float x0 = 7 / 16.0f;
		float x1 = 9 / 16.0f;
		float z0 = 0 / 16.0f;
		float z1 = 2 / 16.0f;
		tt->setShape(x0, h20, z0, x1, h21, z1);
		tesselateBlockInWorld(tt, x, y, z);

		z0 = 14 / 16.0f;
		z1 = 16 / 16.0f;
		tt->setShape(x0, h20, z0, x1, h21, z1);
		tesselateBlockInWorld(tt, x, y, z);
	} else {
		float x0 = 0 / 16.0f;
		float x1 = 2 / 16.0f;
		float z0 = 7 / 16.0f;
		float z1 = 9 / 16.0f;
		tt->setShape(x0, h20, z0, x1, h21, z1);
		tesselateBlockInWorld(tt, x, y, z);

		x0 = 14 / 16.0f;
		x1 = 16 / 16.0f;
		tt->setShape(x0, h20, z0, x1, h21, z1);
		tesselateBlockInWorld(tt, x, y, z);
	}
	if (!isOpen) {
		if (direction == Direction::EAST || direction == Direction::WEST) {
			float x0 = 7 / 16.0f;
			float x1 = 9 / 16.0f;
			float z0 = 6 / 16.0f;
			float z1 = 8 / 16.0f;
			tt->setShape(x0, h00, z0, x1, h11, z1);
			tesselateBlockInWorld(tt, x, y, z);
			z0 = 8 / 16.0f;
			z1 = 10 / 16.0f;
			tt->setShape(x0, h00, z0, x1, h11, z1);
			tesselateBlockInWorld(tt, x, y, z);
			z0 = 10 / 16.0f;
			z1 = 14 / 16.0f;
			tt->setShape(x0, h00, z0, x1, h01, z1);
			tesselateBlockInWorld(tt, x, y, z);
			tt->setShape(x0, h10, z0, x1, h11, z1);
			tesselateBlockInWorld(tt, x, y, z);
			z0 = 2 / 16.0f;
			z1 = 6 / 16.0f;
			tt->setShape(x0, h00, z0, x1, h01, z1);
			tesselateBlockInWorld(tt, x, y, z);
			tt->setShape(x0, h10, z0, x1, h11, z1);
			tesselateBlockInWorld(tt, x, y, z);
		} else {
			float x0 = 6 / 16.0f;
			float x1 = 8 / 16.0f;
			float z0 = 7 / 16.0f;
			float z1 = 9 / 16.0f;
			tt->setShape(x0, h00, z0, x1, h11, z1);
			tesselateBlockInWorld(tt, x, y, z);
			x0 = 8 / 16.0f;
			x1 = 10 / 16.0f;
			tt->setShape(x0, h00, z0, x1, h11, z1);
			tesselateBlockInWorld(tt, x, y, z);
			x0 = 10 / 16.0f;
			x1 = 14 / 16.0f;
			tt->setShape(x0, h00, z0, x1, h01, z1);
			tesselateBlockInWorld(tt, x, y, z);
			tt->setShape(x0, h10, z0, x1, h11, z1);
			tesselateBlockInWorld(tt, x, y, z);
			x0 = 2 / 16.0f;
			x1 = 6 / 16.0f;
			tt->setShape(x0, h00, z0, x1, h01, z1);
			tesselateBlockInWorld(tt, x, y, z);
			tt->setShape(x0, h10, z0, x1, h11, z1);
			tesselateBlockInWorld(tt, x, y, z);

		}
	} else {
		if (direction == Direction::EAST) {

			const float z00 = 0 / 16.0f;
			const float z01 = 2 / 16.0f;
			const float z10 = 14 / 16.0f;
			const float z11 = 16 / 16.0f;

			const float x0 = 9 / 16.0f;
			const float x1 = 13 / 16.0f;
			const float x2 = 15 / 16.0f;

			tt->setShape(x1, h00, z00, x2, h11, z01);
			tesselateBlockInWorld(tt, x, y, z);
			tt->setShape(x1, h00, z10, x2, h11, z11);
			tesselateBlockInWorld(tt, x, y, z);

			tt->setShape(x0, h00, z00, x1, h01, z01);
			tesselateBlockInWorld(tt, x, y, z);
			tt->setShape(x0, h00, z10, x1, h01, z11);
			tesselateBlockInWorld(tt, x, y, z);

			tt->setShape(x0, h10, z00, x1, h11, z01);
			tesselateBlockInWorld(tt, x, y, z);
			tt->setShape(x0, h10, z10, x1, h11, z11);
			tesselateBlockInWorld(tt, x, y, z);
		} else if (direction == Direction::WEST) {
			const float z00 = 0 / 16.0f;
			const float z01 = 2 / 16.0f;
			const float z10 = 14 / 16.0f;
			const float z11 = 16 / 16.0f;

			const float x0 = 1 / 16.0f;
			const float x1 = 3 / 16.0f;
			const float x2 = 7 / 16.0f;

			tt->setShape(x0, h00, z00, x1, h11, z01);
			tesselateBlockInWorld(tt, x, y, z);
			tt->setShape(x0, h00, z10, x1, h11, z11);
			tesselateBlockInWorld(tt, x, y, z);

			tt->setShape(x1, h00, z00, x2, h01, z01);
			tesselateBlockInWorld(tt, x, y, z);
			tt->setShape(x1, h00, z10, x2, h01, z11);
			tesselateBlockInWorld(tt, x, y, z);

			tt->setShape(x1, h10, z00, x2, h11, z01);
			tesselateBlockInWorld(tt, x, y, z);
			tt->setShape(x1, h10, z10, x2, h11, z11);
			tesselateBlockInWorld(tt, x, y, z);
		} else if (direction == Direction::SOUTH) {

			const float x00 = 0 / 16.0f;
			const float x01 = 2 / 16.0f;
			const float x10 = 14 / 16.0f;
			const float x11 = 16 / 16.0f;

			const float z0 = 9 / 16.0f;
			const float z1 = 13 / 16.0f;
			const float z2 = 15 / 16.0f;

			tt->setShape(x00, h00, z1, x01, h11, z2);
			tesselateBlockInWorld(tt, x, y, z);
			tt->setShape(x10, h00, z1, x11, h11, z2);
			tesselateBlockInWorld(tt, x, y, z);

			tt->setShape(x00, h00, z0, x01, h01, z1);
			tesselateBlockInWorld(tt, x, y, z);
			tt->setShape(x10, h00, z0, x11, h01, z1);
			tesselateBlockInWorld(tt, x, y, z);

			tt->setShape(x00, h10, z0, x01, h11, z1);
			tesselateBlockInWorld(tt, x, y, z);
			tt->setShape(x10, h10, z0, x11, h11, z1);
			tesselateBlockInWorld(tt, x, y, z);
		} else if (direction == Direction::NORTH) {
			const float x00 = 0 / 16.0f;
			const float x01 = 2 / 16.0f;
			const float x10 = 14 / 16.0f;
			const float x11 = 16 / 16.0f;

			const float z0 = 1 / 16.0f;
			const float z1 = 3 / 16.0f;
			const float z2 = 7 / 16.0f;

			tt->setShape(x00, h00, z0, x01, h11, z1);
			tesselateBlockInWorld(tt, x, y, z);
			tt->setShape(x10, h00, z0, x11, h11, z1);
			tesselateBlockInWorld(tt, x, y, z);

			tt->setShape(x00, h00, z1, x01, h01, z2);
			tesselateBlockInWorld(tt, x, y, z);
			tt->setShape(x10, h00, z1, x11, h01, z2);
			tesselateBlockInWorld(tt, x, y, z);

			tt->setShape(x00, h10, z1, x01, h11, z2);
			tesselateBlockInWorld(tt, x, y, z);
			tt->setShape(x10, h10, z1, x11, h11, z2);
			tesselateBlockInWorld(tt, x, y, z);
		}
	}

	tt->setShape(0, 0, 0, 1, 1, 1);
	return changed;
}

bool TileRenderer::tesselateBedInWorld(Tile *tt, int x, int y, int z) {
	Tesselator& t = Tesselator::instance;
	int data = level->getData(x, y, z);
	int direction = BedTile::getDirection(data);
	bool isHead = BedTile::isHeadPiece(data);

	float c10 = 0.5f;
	float c11 = 1;
	float c2 = 0.8f;
	float c3 = 0.6f;

	float r11 = c11;
	float g11 = c11;
	float b11 = c11;

	float r10 = c10;
	float r2 = c2;
	float r3 = c3;

	float g10 = c10;
	float g2 = c2;
	float g3 = c3;

	float b10 = c10;
	float b2 = c2;
	float b3 = c3;

	float centerBrightness = tt->getBrightness(level, x, y, z);
	// render wooden underside
	{
		t.color(r10 * centerBrightness, g10 * centerBrightness, b10 * centerBrightness);
		int tex = tt->getTexture(level, x, y, z, Facing::DOWN);

		int xt = (tex & 0xf) << 4;
		int yt = tex & 0xf0;

		float u0 = (xt) / 256.0f;
		float u1 = (xt + 16 - 0.01f) / 256.0f;
		float v0 = (yt) / 256.0f;
		float v1 = (yt + 16 - 0.01f) / 256.0f;

		float x0 = x + tt->xx0;
		float x1 = x + tt->xx1;
		float y0 = y + tt->yy0 + 3.0f / 16.0f;
		float z0 = z + tt->zz0;
		float z1 = z + tt->zz1;

		t.vertexUV(x0, y0, z1, u0, v1);
		t.vertexUV(x0, y0, z0, u0, v0);
		t.vertexUV(x1, y0, z0, u1, v0);
		t.vertexUV(x1, y0, z1, u1, v1);
	}

	// render bed top

	float brightness = tt->getBrightness(level, x, y + 1, z);
	t.color(r11 * brightness, g11 * brightness, b11 * brightness);

	int tex = tt->getTexture(level, x, y, z, Facing::UP);

	int xt = (tex & 0xf) << 4;
	int yt = tex & 0xf0;

	float u0 = (xt) / 256.0f;
	float u1 = (xt + 16 ) / 256.0f;
	float v0 = (yt) / 256.0f;
	float v1 = (yt + 16) / 256.0f;

	// Default is west
	float topLeftU = u0;
	float topRightU = u1;
	float topLeftV = v0;
	float topRightV = v0;
	float bottomLeftU = u0;
	float bottomRightU = u1;
	float bottomLeftV = v1;
	float bottomRightV = v1;

	if (direction == Direction::SOUTH) {
		// rotate 90 degrees clockwise
		topRightU = u0;
		topLeftV = v1;
		bottomLeftU = u1;
		bottomRightV = v0;
	} else if (direction == Direction::NORTH) {
		// rotate 90 degrees counter-clockwise
		topLeftU = u1;
		topRightV = v1;
		bottomRightU = u0;
		bottomLeftV = v0;
	} else if (direction == Direction::EAST) {
		// rotate 180 degrees
		topLeftU = u1;
		topRightV = v1;
		bottomRightU = u0;
		bottomLeftV = v0;
		topRightU = u0;
		topLeftV = v1;
		bottomLeftU = u1;
		bottomRightV = v0;
	}

	float x0 = x + tt->xx0;
	float x1 = x + tt->xx1;
	float y1 = y + tt->yy1;
	float z0 = z + tt->zz0;
	float z1 = z + tt->zz1;

	t.vertexUV(x1, y1, z1, bottomLeftU, bottomLeftV);
	t.vertexUV(x1, y1, z0, topLeftU, topLeftV);
	t.vertexUV(x0, y1, z0, topRightU, topRightV);
	t.vertexUV(x0, y1, z1, bottomRightU, bottomRightV);

	// determine which edge to skip (the one between foot and head piece)
	int skipEdge = Direction::DIRECTION_FACING[direction];
	if (isHead) {
		skipEdge = Direction::DIRECTION_FACING[Direction::DIRECTION_OPPOSITE[direction]];
	}
	// and which edge to x-flip
	int flipEdge = Facing::WEST;
	switch (direction) {
	case Direction::NORTH:
		break;
	case Direction::SOUTH:
		flipEdge = Facing::EAST;
		break;
	case Direction::EAST:
		flipEdge = Facing::NORTH;
		break;
	case Direction::WEST:
		flipEdge = Facing::SOUTH;
		break;
	}

	if ((skipEdge != Facing::NORTH) && (noCulling || tt->shouldRenderFace(level, x, y, z - 1, Facing::NORTH))) {
		float br = tt->getBrightness(level, x, y, z - 1);
		if (tt->zz0 > 0) br = centerBrightness;

		t.color(r2 * br, g2 * br, b2 * br);
		xFlipTexture = flipEdge == Facing::NORTH;
		renderNorth(tt, float(x), float(y), float(z), tt->getTexture(level, x, y, z, 2));
	}

	if ((skipEdge != Facing::SOUTH) && (noCulling || tt->shouldRenderFace(level, x, y, z + 1, Facing::SOUTH))) {
		float br = tt->getBrightness(level, x, y, z + 1);
		if (tt->zz1 < 1) br = centerBrightness;

		t.color(r2 * br, g2 * br, b2 * br);

		xFlipTexture = flipEdge == Facing::SOUTH;
		renderSouth(tt, float(x), float(y), float(z), tt->getTexture(level, x, y, z, 3));
	}

	if ((skipEdge != Facing::WEST) && (noCulling || tt->shouldRenderFace(level, x - 1, y, z, Facing::WEST))) {
		float br = tt->getBrightness(level, x - 1, y, z);
		if (tt->xx0 > 0) br = centerBrightness;

		t.color(r3 * br, g3 * br, b3 * br);
		xFlipTexture = flipEdge == Facing::WEST;
		renderWest(tt, float(x), float(y), float(z), tt->getTexture(level, x, y, z, 4));
	}

	if ((skipEdge != Facing::EAST) && (noCulling || tt->shouldRenderFace(level, x + 1, y, z, Facing::EAST))) {
		float br = tt->getBrightness(level, x + 1, y, z);
		if (tt->xx1 < 1) br = centerBrightness;

		t.color(r3 * br, g3 * br, b3 * br);
		xFlipTexture = flipEdge == Facing::EAST;
		renderEast(tt, float(x), float(y), float(z), tt->getTexture(level, x, y, z, 5));
	}
	xFlipTexture = false;
	return true;
}

bool TileRenderer::tesselateStairsInWorld( StairTile* tt, int x, int y, int z )
{

	tt->setBaseShape(level, x, y, z);
	tesselateBlockInWorld(tt, x, y, z);

	bool checkInnerPiece = tt->setStepShape(level, x, y, z);
	tesselateBlockInWorld(tt, x, y, z);

	if (checkInnerPiece) {
		if (tt->setInnerPieceShape(level, x, y, z)) {
			tesselateBlockInWorld(tt, x, y, z);
		}
	}

	//        setShape(0, 0, 0, 1, 1, 1);
	return true;
}

bool TileRenderer::tesselateDoorInWorld( Tile* tt, int x, int y, int z )
{
	Tesselator& t = Tesselator::instance;

	DoorTile* dt = (DoorTile*) tt;

	bool changed = false;
	float c10 = 0.5f;
	float c11 = 1;
	float c2 = 0.8f;
	float c3 = 0.6f;

	float centerBrightness = tt->getBrightness(level, x, y, z);

	{
		float br = tt->getBrightness(level, x, y - 1, z);
		if (dt->yy0 > 0) br = centerBrightness;
		if (Tile::lightEmission[tt->id] > 0) br = 1.0f;
		t.color(c10 * br, c10 * br, c10 * br);
		renderFaceDown(tt, (float)x, (float)y, (float)z, tt->getTexture(level, x, y, z, 0));
		changed = true;
	}

	{
		float br = tt->getBrightness(level, x, y + 1, z);
		if (dt->yy1 < 1) br = centerBrightness;
		if (Tile::lightEmission[tt->id] > 0) br = 1.0f;
		t.color(c11 * br, c11 * br, c11 * br);
		renderFaceUp(tt, (float)x, (float)y, (float)z, tt->getTexture(level, x, y, z, 1));
		changed = true;
	}

	{
		float br = tt->getBrightness(level, x, y, z - 1);
		if (dt->zz0 > 0) br = centerBrightness;
		if (Tile::lightEmission[tt->id] > 0) br = 1.0f;
		t.color(c2 * br, c2 * br, c2 * br);
		int tex = tt->getTexture(level, x, y, z, 2);
		if (tex < 0) {
			xFlipTexture = true;
			tex = -tex;
		}
		renderNorth(tt, (float)x, (float)y, (float)z, tex);
		changed = true;
		xFlipTexture = false;
	}

	{
		float br = tt->getBrightness(level, x, y, z + 1);
		if (dt->zz1 < 1) br = centerBrightness;
		if (Tile::lightEmission[tt->id] > 0) br = 1.0f;
		t.color(c2 * br, c2 * br, c2 * br);
		int tex = tt->getTexture(level, x, y, z, 3);
		if (tex < 0) {
			xFlipTexture = true;
			tex = -tex;
		}
		renderSouth(tt, (float)x, (float)y, (float)z, tex);
		changed = true;
		xFlipTexture = false;
	}

	{
		float br = tt->getBrightness(level, x - 1, y, z);
		if (dt->xx0 > 0) br = centerBrightness;
		if (Tile::lightEmission[tt->id] > 0) br = 1.0f;
		t.color(c3 * br, c3 * br, c3 * br);
		int tex = tt->getTexture(level, x, y, z, 4);
		if (tex < 0) {
			xFlipTexture = true;
			tex = -tex;
		}
		renderWest(tt, (float)x, (float)y, (float)z, tex);
		changed = true;
		xFlipTexture = false;
	}

	{
		float br = tt->getBrightness(level, x + 1, y, z);
		if (dt->xx1 < 1) br = centerBrightness;
		if (Tile::lightEmission[tt->id] > 0) br = 1.0f;
		t.color(c3 * br, c3 * br, c3 * br);
		int tex = tt->getTexture(level, x, y, z, 5);
		if (tex < 0) {
			xFlipTexture = true;
			tex = -tex;
		}
		renderEast(tt, (float)x, (float)y, (float)z, tex);
		changed = true;
		xFlipTexture = false;
	}

	return changed;
}

bool TileRenderer::tesselateRowInWorld( Tile* tt, int x, int y, int z ) {
	Tesselator& t = Tesselator::instance;
	float br = tt->getBrightness(level, x, y, z);
	t.color(br, br, br);
	tesselateRowTexture(tt, level->getData(x, y, z), float(x), y - 1 / 16.0f, float(z));
	return true;
}

void TileRenderer::renderFaceDown( Tile* tt, float x, float y, float z, int tex )
{
	Tesselator& t = Tesselator::instance;

	if (fixedTexture >= 0) tex = fixedTexture;
	int xt = (tex & 0xf) << 4;
	int yt = tex & 0xf0;

	const float atlasSize = 256.0f;

	float u0 = (xt + tt->xx0 * 15.99f) / atlasSize;
	float u1 = (xt + tt->xx1 * 15.99f) / atlasSize;
	float v0 = (yt + tt->zz0 * 15.99f) / atlasSize;
	float v1 = (yt + tt->zz1 * 15.99f) / atlasSize;

	if (tt->xx0 < 0 || tt->xx1 > 1) {
		u0 = (xt + 0.0f) / atlasSize;
		u1 = (xt + 15.99f) / atlasSize;
	}
	if (tt->zz0 < 0 || tt->zz1 > 1) {
		v0 = (yt + 0.0f) / atlasSize;
		v1 = (yt + 15.99f) / atlasSize;
	}

	float x0 = x + tt->xx0;
	float x1 = x + tt->xx1;
	float y0 = y + tt->yy0;
	float z0 = z + tt->zz0;
	float z1 = z + tt->zz1;

	if (applyAmbienceOcclusion) {
		if (c1r + c3r < c2r + c4r) {
			t.color(c2r, c2g, c2b);
			t.vertexUV(x0, y0, z0, u0, v0);
			t.color(c3r, c3g, c3b);
			t.vertexUV(x1, y0, z0, u1, v0);
			t.color(c4r, c4g, c4b);
			t.vertexUV(x1, y0, z1, u1, v1);
			t.color(c1r, c1g, c1b);
			t.vertexUV(x0, y0, z1, u0, v1);
		} else {
			t.color(c1r, c1g, c1b);
			t.vertexUV(x0, y0, z1, u0, v1);
			t.color(c2r, c2g, c2b);
			t.vertexUV(x0, y0, z0, u0, v0);
			t.color(c3r, c3g, c3b);
			t.vertexUV(x1, y0, z0, u1, v0);
			t.color(c4r, c4g, c4b);
			t.vertexUV(x1, y0, z1, u1, v1);
		}
	} else {
		t.vertexUV(x0, y0, z1, u0, v1);
		t.vertexUV(x0, y0, z0, u0, v0);
		t.vertexUV(x1, y0, z0, u1, v0);
		t.vertexUV(x1, y0, z1, u1, v1);
	}
}

void TileRenderer::renderFaceUp( Tile* tt, float x, float y, float z, int tex )
{
	Tesselator& t = Tesselator::instance;

	if (fixedTexture >= 0) tex = fixedTexture;
	int xt = (tex & 0xf) << 4;
	int yt = tex & 0xf0;

	const float atlasSize = 256.0f;

	float u0 = (xt + tt->xx0 * 15.99f) / atlasSize;
	float u1 = (xt + tt->xx1 * 15.99f) / atlasSize;
	float v0 = (yt + tt->zz0 * 15.99f) / atlasSize;
	float v1 = (yt + tt->zz1 * 15.99f) / atlasSize;

	if (tt->xx0 < 0 || tt->xx1 > 1) {
		u0 = (xt + 0.0f) / atlasSize;
		u1 = (xt + 15.99f) / atlasSize;
	}
	if (tt->zz0 < 0 || tt->zz1 > 1) {
		v0 = (yt + 0.0f) / atlasSize;
		v1 = (yt + 15.99f) / atlasSize;
	}

	float x0 = x + tt->xx0;
	float x1 = x + tt->xx1;
	float y1 = y + tt->yy1;
	float z0 = z + tt->zz0;
	float z1 = z + tt->zz1;

	if (applyAmbienceOcclusion) {
		if (c1r + c3r < c2r + c4r) {
			t.color(c2r, c2g, c2b);
			t.vertexUV(x1, y1, z0, u1, v0);
			t.color(c3r, c3g, c3b);
			t.vertexUV(x0, y1, z0, u0, v0);
			t.color(c4r, c4g, c4b);
			t.vertexUV(x0, y1, z1, u0, v1);
			t.color(c1r, c1g, c1b);
			t.vertexUV(x1, y1, z1, u1, v1);
		} else {
			t.color(c1r, c1g, c1b);
			t.vertexUV(x1, y1, z1, u1, v1);
			t.color(c2r, c2g, c2b);
			t.vertexUV(x1, y1, z0, u1, v0);
			t.color(c3r, c3g, c3b);
			t.vertexUV(x0, y1, z0, u0, v0);
			t.color(c4r, c4g, c4b);
			t.vertexUV(x0, y1, z1, u0, v1);
		}
	} else {
		t.vertexUV(x1, y1, z1, u1, v1);
		t.vertexUV(x1, y1, z0, u1, v0);
		t.vertexUV(x0, y1, z0, u0, v0);
		t.vertexUV(x0, y1, z1, u0, v1);
	}
}

void TileRenderer::renderNorth( Tile* tt, float x, float y, float z, int tex )
{
	Tesselator& t = Tesselator::instance;

	if (fixedTexture >= 0) tex = fixedTexture;
	int xt = (tex & 0xf) << 4;
	int yt = tex & 0xf0;

	const float atlasSize = 256.0f;

	float u0 = (xt + tt->xx0 * 15.99f) / atlasSize;
	float u1 = (xt + tt->xx1 * 15.99f) / atlasSize;
	float v0 = (yt + (1.0f - tt->yy1) * 15.99f) / atlasSize;
	float v1 = (yt + (1.0f - tt->yy0) * 15.99f) / atlasSize;
	if (xFlipTexture) {
		float tmp = u0;
		u0 = u1;
		u1 = tmp;
	}

	if (tt->xx0 < 0 || tt->xx1 > 1) {
		u0 = (xt + 0.0f) / atlasSize;
		u1 = (xt + 15.99f) / atlasSize;
	}
	if (tt->yy0 < 0 || tt->yy1 > 1) {
		v0 = (yt + 0.0f) / atlasSize;
		v1 = (yt + 15.99f) / atlasSize;
	}

	float x0 = x + tt->xx0;
	float x1 = x + tt->xx1;
	float y0 = y + tt->yy0;
	float y1 = y + tt->yy1;
	float z0 = z + tt->zz0;

	if (applyAmbienceOcclusion) {
		if (c1r + c3r < c2r + c4r) {
			t.color(c2r, c2g, c2b);
			t.vertexUV(x1, y1, z0, u0, v0);
			t.color(c3r, c3g, c3b);
			t.vertexUV(x1, y0, z0, u0, v1);
			t.color(c4r, c4g, c4b);
			t.vertexUV(x0, y0, z0, u1, v1);
			t.color(c1r, c1g, c1b);
			t.vertexUV(x0, y1, z0, u1, v0);
		} else {
			t.color(c1r, c1g, c1b);
			t.vertexUV(x0, y1, z0, u1, v0);
			t.color(c2r, c2g, c2b);
			t.vertexUV(x1, y1, z0, u0, v0);
			t.color(c3r, c3g, c3b);
			t.vertexUV(x1, y0, z0, u0, v1);
			t.color(c4r, c4g, c4b);
			t.vertexUV(x0, y0, z0, u1, v1);
		}
	} else {
		t.vertexUV(x0, y1, z0, u1, v0);
		t.vertexUV(x1, y1, z0, u0, v0);
		t.vertexUV(x1, y0, z0, u0, v1);
		t.vertexUV(x0, y0, z0, u1, v1);
	}
}

void TileRenderer::renderSouth( Tile* tt, float x, float y, float z, int tex )
{
	Tesselator& t = Tesselator::instance;

	if (fixedTexture >= 0) tex = fixedTexture;
	int xt = (tex & 0xf) << 4;
	int yt = tex & 0xf0;

	const float atlasSize = 256.0f;

	float u0 = (xt + tt->xx0 * 15.99f) / atlasSize;
	float u1 = (xt + tt->xx1 * 15.99f) / atlasSize;
	float v0 = (yt + (1.0f - tt->yy1) * 15.99f) / atlasSize;
	float v1 = (yt + (1.0f - tt->yy0) * 15.99f) / atlasSize;
	if (xFlipTexture) {
		float tmp = u0;
		u0 = u1;
		u1 = tmp;
	}

	if (tt->xx0 < 0 || tt->xx1 > 1) {
		u0 = (xt + 0.0f) / atlasSize;
		u1 = (xt + 15.99f) / atlasSize;
	}
	if (tt->yy0 < 0 || tt->yy1 > 1) {
		v0 = (yt + 0.0f) / atlasSize;
		v1 = (yt + 15.99f) / atlasSize;
	}

	float x0 = x + tt->xx0;
	float x1 = x + tt->xx1;
	float y0 = y + tt->yy0;
	float y1 = y + tt->yy1;
	float z1 = z + tt->zz1;

	if (applyAmbienceOcclusion) {
		if (c1r + c3r < c2r + c4r) {
			t.color(c2r, c2g, c2b);
			t.vertexUV(x0, y0, z1, u0, v1);
			t.color(c3r, c3g, c3b);
			t.vertexUV(x1, y0, z1, u1, v1);
			t.color(c4r, c4g, c4b);
			t.vertexUV(x1, y1, z1, u1, v0);
			t.color(c1r, c1g, c1b);
			t.vertexUV(x0, y1, z1, u0, v0);
		} else {
			t.color(c1r, c1g, c1b);
			t.vertexUV(x0, y1, z1, u0, v0);
			t.color(c2r, c2g, c2b);
			t.vertexUV(x0, y0, z1, u0, v1);
			t.color(c3r, c3g, c3b);
			t.vertexUV(x1, y0, z1, u1, v1);
			t.color(c4r, c4g, c4b);
			t.vertexUV(x1, y1, z1, u1, v0);
		}
	} else {
		t.vertexUV(x0, y1, z1, u0, v0);
		t.vertexUV(x0, y0, z1, u0, v1);
		t.vertexUV(x1, y0, z1, u1, v1);
		t.vertexUV(x1, y1, z1, u1, v0);
	}
}

void TileRenderer::renderWest( Tile* tt, float x, float y, float z, int tex )
{
	Tesselator& t = Tesselator::instance;

	if (fixedTexture >= 0) tex = fixedTexture;
	int xt = (tex & 0xf) << 4;
	int yt = tex & 0xf0;

	const float atlasSize = 256.0f;

	float u0 = (xt + tt->zz0 * 15.99f) / atlasSize;
	float u1 = (xt + tt->zz1 * 15.99f) / atlasSize;
	float v0 = (yt + (1.0f - tt->yy1) * 15.99f) / atlasSize;
	float v1 = (yt + (1.0f - tt->yy0) * 15.99f) / atlasSize;
	if (xFlipTexture) {
		float tmp = u0;
		u0 = u1;
		u1 = tmp;
	}

	if (tt->zz0 < 0 || tt->zz1 > 1) {
		u0 = (xt + 0.0f) / atlasSize;
		u1 = (xt + 15.99f) / atlasSize;
	}
	if (tt->yy0 < 0 || tt->yy1 > 1) {
		v0 = (yt + 0.0f) / atlasSize;
		v1 = (yt + 15.99f) / atlasSize;
	}

	float x0 = x + tt->xx0;
	float y0 = y + tt->yy0;
	float y1 = y + tt->yy1;
	float z0 = z + tt->zz0;
	float z1 = z + tt->zz1;

	if (applyAmbienceOcclusion) {
		if (c1r + c3r < c2r + c4r) {
			t.color(c2r, c2g, c2b);
			t.vertexUV(x0, y1, z0, u0, v0);
			t.color(c3r, c3g, c3b);
			t.vertexUV(x0, y0, z0, u0, v1);
			t.color(c4r, c4g, c4b);
			t.vertexUV(x0, y0, z1, u1, v1);
			t.color(c1r, c1g, c1b);
			t.vertexUV(x0, y1, z1, u1, v0);
		} else {
			t.color(c1r, c1g, c1b);
			t.vertexUV(x0, y1, z1, u1, v0);
			t.color(c2r, c2g, c2b);
			t.vertexUV(x0, y1, z0, u0, v0);
			t.color(c3r, c3g, c3b);
			t.vertexUV(x0, y0, z0, u0, v1);
			t.color(c4r, c4g, c4b);
			t.vertexUV(x0, y0, z1, u1, v1);
		}
	} else {
		t.vertexUV(x0, y1, z1, u1, v0);
		t.vertexUV(x0, y1, z0, u0, v0);
		t.vertexUV(x0, y0, z0, u0, v1);
		t.vertexUV(x0, y0, z1, u1, v1);
	}
}

void TileRenderer::renderEast( Tile* tt, float x, float y, float z, int tex )
{
	Tesselator& t = Tesselator::instance;

	if (fixedTexture >= 0) tex = fixedTexture;
	int xt = (tex & 0xf) << 4;
	int yt = tex & 0xf0;

	const float atlasSize = 256.0f;

	float u0 = (xt + tt->zz0 * 15.99f) / atlasSize;
	float u1 = (xt + tt->zz1 * 15.99f) / atlasSize;
	float v0 = (yt + (1.0f - tt->yy1) * 15.99f) / atlasSize;
	float v1 = (yt + (1.0f - tt->yy0) * 15.99f) / atlasSize;
	if (xFlipTexture) {
		float tmp = u0;
		u0 = u1;
		u1 = tmp;
	}

	if (tt->zz0 < 0 || tt->zz1 > 1) {
		u0 = (xt + 0.0f) / atlasSize;
		u1 = (xt + 15.99f) / atlasSize;
	}
	if (tt->yy0 < 0 || tt->yy1 > 1) {
		v0 = (yt + 0.0f) / atlasSize;
		v1 = (yt + 15.99f) / atlasSize;
	}

	float x1 = x + tt->xx1;
	float y0 = y + tt->yy0;
	float y1 = y + tt->yy1;
	float z0 = z + tt->zz0;
	float z1 = z + tt->zz1;

	if (applyAmbienceOcclusion) {
		if (c1r + c3r < c2r + c4r) {
			t.color(c2r, c2g, c2b);
			t.vertexUV(x1, y0, z0, u1, v1);
			t.color(c3r, c3g, c3b);
			t.vertexUV(x1, y1, z0, u1, v0);
			t.color(c4r, c4g, c4b);
			t.vertexUV(x1, y1, z1, u0, v0);
			t.color(c1r, c1g, c1b);
			t.vertexUV(x1, y0, z1, u0, v1);
		} else {
			t.color(c1r, c1g, c1b);
			t.vertexUV(x1, y0, z1, u0, v1);
			t.color(c2r, c2g, c2b);
			t.vertexUV(x1, y0, z0, u1, v1);
			t.color(c3r, c3g, c3b);
			t.vertexUV(x1, y1, z0, u1, v0);
			t.color(c4r, c4g, c4b);
			t.vertexUV(x1, y1, z1, u0, v0);
		}
	} else {
		t.vertexUV(x1, y0, z1, u0, v1);
		t.vertexUV(x1, y0, z0, u1, v1);
		t.vertexUV(x1, y1, z0, u1, v0);
		t.vertexUV(x1, y1, z1, u0, v0);
	}
}

void TileRenderer::renderTile( Tile* tile, int data, int color )
{
	Tesselator& t = Tesselator::instance;

	float tr = ((color >> 16) & 0xff) / 255.0f;
	float tg = ((color >> 8) & 0xff) / 255.0f;
	float tb = (color & 0xff) / 255.0f;

	bool isGrassTile = (tile == (Tile*)Tile::grass || tile == (Tile*)Tile::grass_carried);
	bool isLeafTile = (tile == (Tile*)Tile::leaves || tile == (Tile*)Tile::leaves_carried
		|| (Tile::spruceLeaves && tile == (Tile*)Tile::spruceLeaves)
		|| (Tile::birchLeaves && tile == (Tile*)Tile::birchLeaves)
		|| (Tile::jungleLeaves && tile == (Tile*)Tile::jungleLeaves)
		|| (Tile::acaciaLeaves && tile == (Tile*)Tile::acaciaLeaves)
		|| (Tile::darkOakLeaves && tile == (Tile*)Tile::darkOakLeaves));
	bool isJungleLeaf = (tile == (Tile*)Tile::jungleLeaves || (tile == (Tile*)Tile::leaves && data == LeafTile::JUNGLE_LEAF));

	if (!isGrassTile && !isLeafTile) {
		tr = tg = tb = 1.0f;
	}

	int shape = tile->getRenderShape();

	if (shape == Tile::SHAPE_BLOCK) {
		tile->updateDefaultShape();
		t.addOffset(-0.5f, -0.5f, -0.5f);
		t.begin();

		t.normal(0.0f, -1.0f, 0.0f);
		if (isGrassTile) {
			t.color(1.0f, 1.0f, 1.0f);
			renderFaceDown(tile, 0, 0, 0, 2);
		} else if (isLeafTile) {
			t.color(tr, tg, tb);
			renderFaceDown(tile, 0, 0, 0, tile->getTexture(0, data) & ~Tile::TEXTURE_ALT_FLAG);
			if (isJungleLeaf) {
				t.color(1.0f, 1.0f, 1.0f);
				renderFaceDown(tile, 0, 0, 0, 55);
			}
		} else {
			t.color(1.0f, 1.0f, 1.0f);
			renderFaceDown(tile, 0, 0, 0, tile->getTexture(0, data) & ~Tile::TEXTURE_ALT_FLAG);
		}

		t.normal(0.0f, 1.0f, 0.0f); 
		if (isGrassTile) {
			t.color(tr, tg, tb);
			renderFaceUp(tile, 0, 0, 0, 0); // grass_top
		} else if (isLeafTile) {
			t.color(tr, tg, tb);
			renderFaceUp(tile, 0, 0, 0, tile->getTexture(1, data) & ~Tile::TEXTURE_ALT_FLAG);
			if (isJungleLeaf) {
				t.color(1.0f, 1.0f, 1.0f);
				renderFaceUp(tile, 0, 0, 0, 55);
			}
		} else {
			t.color(1.0f, 1.0f, 1.0f);
			renderFaceUp(tile, 0, 0, 0, tile->getTexture(1, data) & ~Tile::TEXTURE_ALT_FLAG);
		}

		t.normal(0.0f, 0.0f, -1.0f);
		if (isGrassTile) {
			t.color(1.0f, 1.0f, 1.0f);
			renderNorth(tile, 0, 0, 0, 236); // dirt_grass
			t.color(tr, tg, tb);
			renderNorth(tile, 0, 0, -0.001f, 38); // grass_side_overlay
		} else if (isLeafTile) {
			t.color(tr, tg, tb);
			renderNorth(tile, 0, 0, 0, tile->getTexture(2, data) & ~Tile::TEXTURE_ALT_FLAG);
			if (isJungleLeaf) {
				t.color(1.0f, 1.0f, 1.0f);
				renderNorth(tile, 0, 0, -0.0005f, 55);
			}
		} else {
			t.color(1.0f, 1.0f, 1.0f);
			renderNorth(tile, 0, 0, 0, tile->getTexture(2, data) & ~Tile::TEXTURE_ALT_FLAG);
		}

		t.normal(0.0f, 0.0f, 1.0f);
		if (isGrassTile) {
			t.color(1.0f, 1.0f, 1.0f);
			renderSouth(tile, 0, 0, 0, 236); // dirt_grass
			t.color(tr, tg, tb);
			renderSouth(tile, 0, 0, 0.001f, 38); // grass_side_overlay
		} else if (isLeafTile) {
			t.color(tr, tg, tb);
			renderSouth(tile, 0, 0, 0, tile->getTexture(3, data) & ~Tile::TEXTURE_ALT_FLAG);
			if (isJungleLeaf) {
				t.color(1.0f, 1.0f, 1.0f);
				renderSouth(tile, 0, 0, 0.0005f, 55);
			}
		} else {
			t.color(1.0f, 1.0f, 1.0f);
			renderSouth(tile, 0, 0, 0, tile->getTexture(3, data) & ~Tile::TEXTURE_ALT_FLAG);
		}

		t.normal(-1.0f, 0.0f, 0.0f);
		if (isGrassTile) {
			t.color(1.0f, 1.0f, 1.0f);
			renderWest(tile, 0, 0, 0, 236); // dirt_grass
			t.color(tr, tg, tb);
			renderWest(tile, -0.001f, 0, 0, 38); // grass_side_overlay
		} else if (isLeafTile) {
			t.color(tr, tg, tb);
			renderWest(tile, 0, 0, 0, tile->getTexture(4, data) & ~Tile::TEXTURE_ALT_FLAG);
			if (isJungleLeaf) {
				t.color(1.0f, 1.0f, 1.0f);
				renderWest(tile, -0.0005f, 0, 0, 55);
			}
		} else {
			t.color(1.0f, 1.0f, 1.0f);
			renderWest(tile, 0, 0, 0, tile->getTexture(4, data) & ~Tile::TEXTURE_ALT_FLAG);
		}

		t.normal(1.0f, 0.0f, 0.0f);
		if (isGrassTile) {
			t.color(1.0f, 1.0f, 1.0f);
			renderEast(tile, 0, 0, 0, 236); // dirt_grass
			t.color(tr, tg, tb);
			renderEast(tile, 0.001f, 0, 0, 38); // grass_side_overlay
		} else if (isLeafTile) {
			t.color(tr, tg, tb);
			renderEast(tile, 0, 0, 0, tile->getTexture(5, data) & ~Tile::TEXTURE_ALT_FLAG);
			if (isJungleLeaf) {
				t.color(1.0f, 1.0f, 1.0f);
				renderEast(tile, 0.0005f, 0, 0, 55);
			}
		} else {
			t.color(1.0f, 1.0f, 1.0f);
			renderEast(tile, 0, 0, 0, tile->getTexture(5, data) & ~Tile::TEXTURE_ALT_FLAG);
		}
		t.draw();

		t.addOffset(0.5f, 0.5f, 0.5f);

	} else if (shape == Tile::SHAPE_CROSS_TEXTURE) { // uhh java has this but is this even ever used??? - shredder
		t.begin();
		t.color(tr, tg, tb);
		t.normal(0.0f, -1.0f, 0.0f);

		tesselateCrossTexture(tile, data, -0.5f, -0.5f, -0.5f);
		t.draw();
	} else if(shape == Tile::SHAPE_STEM) {
		t.begin();
		t.color(tr, tg, tb);
		t.normal(0.0f, -1.0f, 0.0f);

		tile->updateDefaultShape();
		tesselateStemTexture(tile, data, tile->yy1, -0.5f, -0.5f, -0.5f);
		t.draw();
	} else if (shape == Tile::SHAPE_CACTUS) {
		tile->updateDefaultShape();
		t.offset(-0.5f, -0.5f, -0.5f);
		float s = 1 / 16.0f;
		t.begin();
		t.color(tr, tg, tb);

		t.normal(0.0f, -1.0f, 0.0f);

		renderFaceDown(tile, 0, 0, 0, tile->getTexture(0) & ~Tile::TEXTURE_ALT_FLAG);

		t.normal(0.0f, 1.0f, 0.0f);

		renderFaceUp(tile, 0, 0, 0, tile->getTexture(1) & ~Tile::TEXTURE_ALT_FLAG);

		t.normal(0.0f, 0.0f, -1.0f);

		t.addOffset(0, 0, s);


		renderNorth(tile, 0, 0, 0, tile->getTexture(2) & ~Tile::TEXTURE_ALT_FLAG);

		t.normal(0.0f, 0.0f, 1.0f);

		t.addOffset(0, 0, -s);
		t.addOffset(0, 0, -s);



		renderSouth(tile, 0, 0, 0, tile->getTexture(3) & ~Tile::TEXTURE_ALT_FLAG);

		t.normal(-1.0f, 0.0f, 0.0f);

		t.addOffset(0, 0, s);
		t.addOffset(s, 0, 0);
		renderWest(tile, 0, 0, 0, tile->getTexture(4) & ~Tile::TEXTURE_ALT_FLAG);

		t.normal(1.0f, 0.0f, 0.0f);

		t.addOffset(-s, 0, 0);
		t.addOffset(-s, 0, 0);
		renderEast(tile, 0, 0, 0, tile->getTexture(5) & ~Tile::TEXTURE_ALT_FLAG);
		t.addOffset(s, 0, 0);
		t.draw();
		t.offset(0, 0, 0);//0.5f, 0.5f, 0.5f);
	} else if (shape == Tile::SHAPE_ROWS) {
		t.begin();
		t.color(tr, tg, tb);
		t.normal(0, -1, 0);
		tesselateRowTexture(tile, data, -0.5f, -0.5f, -0.5f);
	} else if (shape == Tile::SHAPE_ENTITYTILE_ANIMATED) {
		EntityTileRenderer::instance->render(tile, data, 1.0f);
	} else if (shape == Tile::SHAPE_STAIRS) {
		t.addOffset(-0.5f, -0.5f, -0.5f);
		t.begin();
		t.color(tr, tg, tb);
		for (int i = 0; i < 2; i++) {
			if (i == 0) tile->setShape(0, 0, 0, 1, 1, 0.5f);
			if (i == 1) tile->setShape(0, 0, 0.5f, 1, 0.5f, 1);


			t.normal(0.0f, -1.0f, 0.0f);
			renderFaceDown(tile, 0, 0, 0, tile->getTexture(0) & ~Tile::TEXTURE_ALT_FLAG);

			t.normal(0.0f, 1.0f, 0.0f);
			renderFaceUp(tile, 0, 0, 0, tile->getTexture(1) & ~Tile::TEXTURE_ALT_FLAG);

			t.normal(0.0f, 0.0f, -1.0f);
			renderNorth(tile, 0, 0, 0, tile->getTexture(2) & ~Tile::TEXTURE_ALT_FLAG);

			t.normal(0.0f, 0.0f, 1.0f);
			renderSouth(tile, 0, 0, 0, tile->getTexture(3) & ~Tile::TEXTURE_ALT_FLAG);

			t.normal(-1.0f, 0.0f, 0.0f);
			renderWest(tile, 0, 0, 0, tile->getTexture(4) & ~Tile::TEXTURE_ALT_FLAG);

			t.normal(1.0f, 0.0f, 0.0f);
			renderEast(tile, 0, 0, 0, tile->getTexture(5) & ~Tile::TEXTURE_ALT_FLAG);
		}
		t.draw();
		t.addOffset(0.5f, 0.5f, 0.5f);
	}
	else if (shape == Tile::SHAPE_FENCE) {
		t.addOffset(-0.5f, -0.5f, -0.5f);
		t.begin();
		t.color(tr, tg, tb);
		for (int i = 0; i < 4; i++) {
			float w = 2 / 16.0f;
			if (i == 0) tile->setShape(0.5f - w, 0, 0, 0.5f + w, 1, w * 2);
			if (i == 1) tile->setShape(0.5f - w, 0, 1 - w * 2, 0.5f + w, 1, 1);
			w = 1 / 16.0f;
			if (i == 2) tile->setShape(0.5f - w, 1 - w * 3, -w * 2, 0.5f + w, 1 - w, 1 + w * 2);
			if (i == 3) tile->setShape(0.5f - w, 0.5f - w * 3, -w * 2, 0.5f + w, 0.5f - w, 1 + w * 2);

			t.normal(0.0f, -1.0f, 0.0f);
			renderFaceDown(tile, 0, 0, 0, tile->getTexture(0) & ~Tile::TEXTURE_ALT_FLAG);

			t.normal(0.0f, 1.0f, 0.0f);
			renderFaceUp(tile, 0, 0, 0, tile->getTexture(1) & ~Tile::TEXTURE_ALT_FLAG);

			t.normal(0.0f, 0.0f, -1.0f);
			renderNorth(tile, 0, 0, 0, tile->getTexture(2) & ~Tile::TEXTURE_ALT_FLAG);

			t.normal(0.0f, 0.0f, 1.0f);
			renderSouth(tile, 0, 0, 0, tile->getTexture(3) & ~Tile::TEXTURE_ALT_FLAG);

			t.normal(-1.0f, 0.0f, 0.0f);
			renderWest(tile, 0, 0, 0, tile->getTexture(4) & ~Tile::TEXTURE_ALT_FLAG);

			t.normal(1.0f, 0.0f, 0.0f);
			renderEast(tile, 0, 0, 0, tile->getTexture(5) & ~Tile::TEXTURE_ALT_FLAG);
		}
		t.draw();
		t.addOffset(0.5f, 0.5f, 0.5f);
		tile->setShape(0, 0, 0, 1, 1, 1);
	} else if (shape == Tile::SHAPE_FENCE_GATE) {
		t.addOffset(-0.5f, -0.5f, -0.5f);
		t.begin();
		t.color(tr, tg, tb);
		for (int i = 0; i < 3; i++) {
			float w = 1 / 16.0f;
			if (i == 0) tile->setShape(0.5f - w, .3f, 0, 0.5f + w, 1, w * 2);
			if (i == 1) tile->setShape(0.5f - w, .3f, 1 - w * 2, 0.5f + w, 1, 1);
			if (i == 2) tile->setShape(0.5f - w, .5f, w * 2, 0.5f + w, 1 - w, 1 - w * 2);

			t.normal(0.0f, -1.0f, 0.0f);
			renderFaceDown(tile, 0, 0, 0, tile->getTexture(0) & ~Tile::TEXTURE_ALT_FLAG);

			t.normal(0.0f, 1.0f, 0.0f);
			renderFaceUp(tile, 0, 0, 0, tile->getTexture(1) & ~Tile::TEXTURE_ALT_FLAG);

			t.normal(0.0f, 0.0f, -1.0f);
			renderNorth(tile, 0, 0, 0, tile->getTexture(2) & ~Tile::TEXTURE_ALT_FLAG);

			t.normal(0.0f, 0.0f, 1.0f);
			renderSouth(tile, 0, 0, 0, tile->getTexture(3) & ~Tile::TEXTURE_ALT_FLAG);

			t.normal(-1.0f, 0.0f, 0.0f);
			renderWest(tile, 0, 0, 0, tile->getTexture(4) & ~Tile::TEXTURE_ALT_FLAG);

			t.normal(1.0f, 0.0f, 0.0f);
			renderEast(tile, 0, 0, 0, tile->getTexture(5) & ~Tile::TEXTURE_ALT_FLAG);
		}
		t.draw();
		t.addOffset(0.5f, 0.5f, 0.5f);
		tile->setShape(0, 0, 0, 1, 1, 1);
	} else if (shape == Tile::SHAPE_LANTERN) {
		t.addOffset(-0.5f, -0.5f, -0.5f);
		t.begin();

		int tex = tile->getTexture(0, data) & ~Tile::TEXTURE_ALT_FLAG;
		int xt = (tex & 0xf) << 4;
		int yt = tex & 0xf0;
		const float atlasSize = 256.0f;

		float uSide0 = (xt + 0.0f) / atlasSize;
		float uSide1 = (xt + 6.0f) / atlasSize;
		float vSide0 = (yt + 2.0f) / atlasSize;
		float vSide1 = (yt + 9.0f) / atlasSize;

		float uLid0 = (xt + 0.0f) / atlasSize;
		float uLid1 = (xt + 6.0f) / atlasSize;
		float vLid0 = (yt + 9.0f) / atlasSize;
		float vLid1 = (yt + 15.0f) / atlasSize;

		float uCap0 = (xt + 1.0f) / atlasSize;
		float uCap1 = (xt + 5.0f) / atlasSize;
		float vCap0 = (yt + 0.0f) / atlasSize;
		float vCap1 = (yt + 2.0f) / atlasSize;

		float uCapTop0 = (xt + 1.0f) / atlasSize;
		float uCapTop1 = (xt + 5.0f) / atlasSize;
		float vCapTop0 = (yt + 9.0f) / atlasSize;
		float vCapTop1 = (yt + 13.0f) / atlasSize;

		float uRing0 = (xt + 11.0f) / atlasSize;
		float uRing1 = (xt + 14.0f) / atlasSize;
		float vRing0 = (yt + 1.0f) / atlasSize;
		float vRing1 = (yt + 5.0f) / atlasSize;

		float bx0 = 5.0f / 16.0f, bx1 = 11.0f / 16.0f;
		float bz0 = 5.0f / 16.0f, bz1 = 11.0f / 16.0f;
		float by0 = 0.0f / 16.0f, by1 = 7.0f / 16.0f;

		// Up
		t.color(tr, tg, tb);
		t.vertexUV(bx0, by1, bz0, uLid0, vLid0);
		t.vertexUV(bx0, by1, bz1, uLid0, vLid1);
		t.vertexUV(bx1, by1, bz1, uLid1, vLid1);
		t.vertexUV(bx1, by1, bz0, uLid1, vLid0);

		// Down
		t.color(tr * 0.5f, tg * 0.5f, tb * 0.5f);
		t.vertexUV(bx0, by0, bz1, uLid0, vLid1);
		t.vertexUV(bx0, by0, bz0, uLid0, vLid0);
		t.vertexUV(bx1, by0, bz0, uLid1, vLid0);
		t.vertexUV(bx1, by0, bz1, uLid1, vLid1);

		// North & South
		t.color(tr * 0.8f, tg * 0.8f, tb * 0.8f);
		t.vertexUV(bx0, by1, bz0, uSide1, vSide0);
		t.vertexUV(bx1, by1, bz0, uSide0, vSide0);
		t.vertexUV(bx1, by0, bz0, uSide0, vSide1);
		t.vertexUV(bx0, by0, bz0, uSide1, vSide1);
		t.vertexUV(bx1, by1, bz0, uSide0, vSide0);
		t.vertexUV(bx0, by1, bz0, uSide1, vSide0);
		t.vertexUV(bx0, by0, bz0, uSide1, vSide1);
		t.vertexUV(bx1, by0, bz0, uSide0, vSide1);

		t.vertexUV(bx0, by1, bz1, uSide0, vSide0);
		t.vertexUV(bx0, by0, bz1, uSide0, vSide1);
		t.vertexUV(bx1, by0, bz1, uSide1, vSide1);
		t.vertexUV(bx1, by1, bz1, uSide1, vSide0);
		t.vertexUV(bx1, by1, bz1, uSide1, vSide0);
		t.vertexUV(bx1, by0, bz1, uSide1, vSide1);
		t.vertexUV(bx0, by0, bz1, uSide0, vSide1);
		t.vertexUV(bx0, by1, bz1, uSide0, vSide0);

		// West & East
		t.color(tr * 0.6f, tg * 0.6f, tb * 0.6f);
		t.vertexUV(bx0, by1, bz1, uSide1, vSide0);
		t.vertexUV(bx0, by1, bz0, uSide0, vSide0);
		t.vertexUV(bx0, by0, bz0, uSide0, vSide1);
		t.vertexUV(bx0, by0, bz1, uSide1, vSide1);
		t.vertexUV(bx0, by1, bz0, uSide0, vSide0);
		t.vertexUV(bx0, by1, bz1, uSide1, vSide0);
		t.vertexUV(bx0, by0, bz1, uSide1, vSide1);
		t.vertexUV(bx0, by0, bz0, uSide0, vSide1);

		t.vertexUV(bx1, by0, bz1, uSide0, vSide1);
		t.vertexUV(bx1, by0, bz0, uSide1, vSide1);
		t.vertexUV(bx1, by1, bz0, uSide1, vSide0);
		t.vertexUV(bx1, by1, bz1, uSide0, vSide0);
		t.vertexUV(bx1, by0, bz0, uSide1, vSide1);
		t.vertexUV(bx1, by0, bz1, uSide0, vSide1);
		t.vertexUV(bx1, by1, bz1, uSide0, vSide0);
		t.vertexUV(bx1, by1, bz0, uSide1, vSide0);

		// Cap
		float cx0 = 6.0f / 16.0f, cx1 = 10.0f / 16.0f;
		float cz0 = 6.0f / 16.0f, cz1 = 10.0f / 16.0f;
		float cy0 = 7.0f / 16.0f, cy1 = 9.0f / 16.0f;

		t.color(tr, tg, tb);
		t.vertexUV(cx0, cy1, cz0, uCapTop0, vCapTop0);
		t.vertexUV(cx0, cy1, cz1, uCapTop0, vCapTop1);
		t.vertexUV(cx1, cy1, cz1, uCapTop1, vCapTop1);
		t.vertexUV(cx1, cy1, cz0, uCapTop1, vCapTop0);

		t.color(tr * 0.8f, tg * 0.8f, tb * 0.8f);
		t.vertexUV(cx0, cy1, cz0, uCap1, vCap0);
		t.vertexUV(cx1, cy1, cz0, uCap0, vCap0);
		t.vertexUV(cx1, cy0, cz0, uCap0, vCap1);
		t.vertexUV(cx0, cy0, cz0, uCap1, vCap1);

		t.vertexUV(cx0, cy1, cz1, uCap0, vCap0);
		t.vertexUV(cx0, cy0, cz1, uCap0, vCap1);
		t.vertexUV(cx1, cy0, cz1, uCap1, vCap1);
		t.vertexUV(cx1, cy1, cz1, uCap1, vCap0);

		t.color(tr * 0.6f, tg * 0.6f, tb * 0.6f);
		t.vertexUV(cx0, cy1, cz1, uCap1, vCap0);
		t.vertexUV(cx0, cy1, cz0, uCap0, vCap0);
		t.vertexUV(cx0, cy0, cz0, uCap0, vCap1);
		t.vertexUV(cx0, cy0, cz1, uCap1, vCap1);

		t.vertexUV(cx1, cy0, cz1, uCap0, vCap1);
		t.vertexUV(cx1, cy0, cz0, uCap1, vCap1);
		t.vertexUV(cx1, cy1, cz0, uCap1, vCap0);
		t.vertexUV(cx1, cy1, cz1, uCap0, vCap0);

		// Ring
		t.color(tr * 0.8f, tg * 0.8f, tb * 0.8f);
		float rx0 = 6.5f / 16.0f, rx1 = 9.5f / 16.0f;
		float rz0 = 6.5f / 16.0f, rz1 = 9.5f / 16.0f;
		float ry0 = 9.0f / 16.0f, ry1 = 13.0f / 16.0f;
		float rxMid = 8.0f / 16.0f, rzMid = 8.0f / 16.0f;

		t.vertexUV(rx0, ry1, rzMid, uRing0, vRing0);
		t.vertexUV(rx1, ry1, rzMid, uRing1, vRing0);
		t.vertexUV(rx1, ry0, rzMid, uRing1, vRing1);
		t.vertexUV(rx0, ry0, rzMid, uRing0, vRing1);

		t.vertexUV(rx1, ry1, rzMid, uRing1, vRing0);
		t.vertexUV(rx0, ry1, rzMid, uRing0, vRing0);
		t.vertexUV(rx0, ry0, rzMid, uRing0, vRing1);
		t.vertexUV(rx1, ry0, rzMid, uRing1, vRing1);

		t.vertexUV(rxMid, ry1, rz0, uRing0, vRing0);
		t.vertexUV(rxMid, ry1, rz1, uRing1, vRing0);
		t.vertexUV(rxMid, ry0, rz1, uRing1, vRing1);
		t.vertexUV(rxMid, ry0, rz0, uRing0, vRing1);

		t.vertexUV(rxMid, ry1, rz1, uRing1, vRing0);
		t.vertexUV(rxMid, ry1, rz0, uRing0, vRing0);
		t.vertexUV(rxMid, ry0, rz0, uRing0, vRing1);
		t.vertexUV(rxMid, ry0, rz1, uRing1, vRing1);

		t.draw();
		t.addOffset(0.5f, 0.5f, 0.5f);
		tile->setShape(0, 0, 0, 1, 1, 1);
	} else if (shape == Tile::SHAPE_CAMPFIRE) {
		t.addOffset(-0.5f, -0.5f, -0.5f);
		t.begin();

		const int TEX_LOG = 118;
		const int TEX_FIRE = 109;
		const int TEX_LIT = 110;

		float colUpR = tr, colUpG = tg, colUpB = tb;
		float colDownR = tr * 0.5f, colDownG = tg * 0.5f, colDownB = tb * 0.5f;
		float colNSR = tr * 0.8f, colNSG = tg * 0.8f, colNSB = tb * 0.8f;
		float colWER = tr * 0.6f, colWEG = tg * 0.6f, colWEB = tb * 0.6f;

		// 1. Bottom Left Log (West Log: x: 1..5, y: 0..4, z: 0..16)
		float lx0 = 1.0f / 16.0f, lx1 = 5.0f / 16.0f;
		float ly0 = 0.0f, ly1 = 4.0f / 16.0f;
		float lz0 = 0.0f, lz1 = 1.0f;

		// Up (rot 90)
		t.color(colUpR, colUpG, colUpB);
		t.vertexUV(lx0, ly1, lz0, getAtlasU(TEX_LOG, 0), getAtlasV(TEX_LOG, 4));
		t.vertexUV(lx0, ly1, lz1, getAtlasU(TEX_LOG, 16), getAtlasV(TEX_LOG, 4));
		t.vertexUV(lx1, ly1, lz1, getAtlasU(TEX_LOG, 16), getAtlasV(TEX_LOG, 0));
		t.vertexUV(lx1, ly1, lz0, getAtlasU(TEX_LOG, 0), getAtlasV(TEX_LOG, 0));

		// Down (rot 90)
		t.color(colDownR, colDownG, colDownB);
		t.vertexUV(lx0, ly0, lz1, getAtlasU(TEX_LOG, 16), getAtlasV(TEX_LOG, 0));
		t.vertexUV(lx0, ly0, lz0, getAtlasU(TEX_LOG, 0), getAtlasV(TEX_LOG, 0));
		t.vertexUV(lx1, ly0, lz0, getAtlasU(TEX_LOG, 0), getAtlasV(TEX_LOG, 4));
		t.vertexUV(lx1, ly0, lz1, getAtlasU(TEX_LOG, 16), getAtlasV(TEX_LOG, 4));

		// North (end cut)
		t.color(colNSR, colNSG, colNSB);
		t.vertexUV(lx0, ly1, lz0, getAtlasU(TEX_LOG, 0), getAtlasV(TEX_LOG, 4));
		t.vertexUV(lx1, ly1, lz0, getAtlasU(TEX_LOG, 4), getAtlasV(TEX_LOG, 4));
		t.vertexUV(lx1, ly0, lz0, getAtlasU(TEX_LOG, 4), getAtlasV(TEX_LOG, 8));
		t.vertexUV(lx0, ly0, lz0, getAtlasU(TEX_LOG, 0), getAtlasV(TEX_LOG, 8));

		// South (end cut)
		t.vertexUV(lx0, ly1, lz1, getAtlasU(TEX_LOG, 4), getAtlasV(TEX_LOG, 4));
		t.vertexUV(lx0, ly0, lz1, getAtlasU(TEX_LOG, 4), getAtlasV(TEX_LOG, 8));
		t.vertexUV(lx1, ly0, lz1, getAtlasU(TEX_LOG, 0), getAtlasV(TEX_LOG, 8));
		t.vertexUV(lx1, ly1, lz1, getAtlasU(TEX_LOG, 0), getAtlasV(TEX_LOG, 4));

		// West (bark)
		t.color(colWER, colWEG, colWEB);
		t.vertexUV(lx0, ly1, lz1, getAtlasU(TEX_LOG, 0), getAtlasV(TEX_LOG, 0));
		t.vertexUV(lx0, ly1, lz0, getAtlasU(TEX_LOG, 16), getAtlasV(TEX_LOG, 0));
		t.vertexUV(lx0, ly0, lz0, getAtlasU(TEX_LOG, 16), getAtlasV(TEX_LOG, 4));
		t.vertexUV(lx0, ly0, lz1, getAtlasU(TEX_LOG, 0), getAtlasV(TEX_LOG, 4));

		// East (lit log inner)
		t.vertexUV(lx1, ly1, lz0, getAtlasU(TEX_LIT, 16), getAtlasV(TEX_LIT, 1));
		t.vertexUV(lx1, ly1, lz1, getAtlasU(TEX_LIT, 0), getAtlasV(TEX_LIT, 1));
		t.vertexUV(lx1, ly0, lz1, getAtlasU(TEX_LIT, 0), getAtlasV(TEX_LIT, 5));
		t.vertexUV(lx1, ly0, lz0, getAtlasU(TEX_LIT, 16), getAtlasV(TEX_LIT, 5));

		// 2. Bottom Right Log (East Log: x: 11..15, y: 0..4, z: 0..16)
		float rx0 = 11.0f / 16.0f, rx1 = 15.0f / 16.0f;
		float ry0 = 0.0f, ry1 = 4.0f / 16.0f;
		float rz0 = 0.0f, rz1 = 1.0f;

		// Up (rot 90)
		t.color(colUpR, colUpG, colUpB);
		t.vertexUV(rx0, ry1, rz0, getAtlasU(TEX_LOG, 0), getAtlasV(TEX_LOG, 4));
		t.vertexUV(rx0, ry1, rz1, getAtlasU(TEX_LOG, 16), getAtlasV(TEX_LOG, 4));
		t.vertexUV(rx1, ry1, rz1, getAtlasU(TEX_LOG, 16), getAtlasV(TEX_LOG, 0));
		t.vertexUV(rx1, ry1, rz0, getAtlasU(TEX_LOG, 0), getAtlasV(TEX_LOG, 0));

		// Down (rot 90)
		t.color(colDownR, colDownG, colDownB);
		t.vertexUV(rx0, ry0, rz1, getAtlasU(TEX_LOG, 16), getAtlasV(TEX_LOG, 0));
		t.vertexUV(rx0, ry0, rz0, getAtlasU(TEX_LOG, 0), getAtlasV(TEX_LOG, 0));
		t.vertexUV(rx1, ry0, rz0, getAtlasU(TEX_LOG, 0), getAtlasV(TEX_LOG, 4));
		t.vertexUV(rx1, ry0, rz1, getAtlasU(TEX_LOG, 16), getAtlasV(TEX_LOG, 4));

		// North (end cut)
		t.color(colNSR, colNSG, colNSB);
		t.vertexUV(rx0, ry1, rz0, getAtlasU(TEX_LOG, 0), getAtlasV(TEX_LOG, 4));
		t.vertexUV(rx1, ry1, rz0, getAtlasU(TEX_LOG, 4), getAtlasV(TEX_LOG, 4));
		t.vertexUV(rx1, ry0, rz0, getAtlasU(TEX_LOG, 4), getAtlasV(TEX_LOG, 8));
		t.vertexUV(rx0, ry0, rz0, getAtlasU(TEX_LOG, 0), getAtlasV(TEX_LOG, 8));

		// South (end cut)
		t.vertexUV(rx0, ry1, rz1, getAtlasU(TEX_LOG, 4), getAtlasV(TEX_LOG, 4));
		t.vertexUV(rx0, ry0, rz1, getAtlasU(TEX_LOG, 4), getAtlasV(TEX_LOG, 8));
		t.vertexUV(rx1, ry0, rz1, getAtlasU(TEX_LOG, 0), getAtlasV(TEX_LOG, 8));
		t.vertexUV(rx1, ry1, rz1, getAtlasU(TEX_LOG, 0), getAtlasV(TEX_LOG, 4));

		// West (lit log inner)
		t.color(colWER, colWEG, colWEB);
		t.vertexUV(rx0, ry1, rz1, getAtlasU(TEX_LIT, 0), getAtlasV(TEX_LIT, 1));
		t.vertexUV(rx0, ry1, rz0, getAtlasU(TEX_LIT, 16), getAtlasV(TEX_LIT, 1));
		t.vertexUV(rx0, ry0, rz0, getAtlasU(TEX_LIT, 16), getAtlasV(TEX_LIT, 5));
		t.vertexUV(rx0, ry0, rz1, getAtlasU(TEX_LIT, 0), getAtlasV(TEX_LIT, 5));

		// East (bark)
		t.vertexUV(rx1, ry1, rz0, getAtlasU(TEX_LOG, 16), getAtlasV(TEX_LOG, 0));
		t.vertexUV(rx1, ry1, rz1, getAtlasU(TEX_LOG, 0), getAtlasV(TEX_LOG, 0));
		t.vertexUV(rx1, ry0, rz1, getAtlasU(TEX_LOG, 0), getAtlasV(TEX_LOG, 4));
		t.vertexUV(rx1, ry0, rz0, getAtlasU(TEX_LOG, 16), getAtlasV(TEX_LOG, 4));

		// 3. Top North Log (x: 0..16, y: 3..7, z: 1..5)
		float nx0 = 0.0f, nx1 = 1.0f;
		float ny0 = 3.0f / 16.0f, ny1 = 7.0f / 16.0f;
		float nz0 = 1.0f / 16.0f, nz1 = 5.0f / 16.0f;

		// Up (rot 180)
		t.color(colUpR, colUpG, colUpB);
		t.vertexUV(nx0, ny1, nz0, getAtlasU(TEX_LOG, 16), getAtlasV(TEX_LOG, 4));
		t.vertexUV(nx0, ny1, nz1, getAtlasU(TEX_LOG, 16), getAtlasV(TEX_LOG, 0));
		t.vertexUV(nx1, ny1, nz1, getAtlasU(TEX_LOG, 0), getAtlasV(TEX_LOG, 0));
		t.vertexUV(nx1, ny1, nz0, getAtlasU(TEX_LOG, 0), getAtlasV(TEX_LOG, 4));

		// Down
		t.color(colDownR, colDownG, colDownB);
		t.vertexUV(nx0, ny0, nz1, getAtlasU(TEX_LIT, 0), getAtlasV(TEX_LIT, 4));
		t.vertexUV(nx0, ny0, nz0, getAtlasU(TEX_LIT, 0), getAtlasV(TEX_LIT, 8));
		t.vertexUV(nx1, ny0, nz0, getAtlasU(TEX_LIT, 16), getAtlasV(TEX_LIT, 8));
		t.vertexUV(nx1, ny0, nz1, getAtlasU(TEX_LIT, 16), getAtlasV(TEX_LIT, 4));

		// West (end cut)
		t.color(colWER, colWEG, colWEB);
		t.vertexUV(nx0, ny1, nz1, getAtlasU(TEX_LOG, 4), getAtlasV(TEX_LOG, 4));
		t.vertexUV(nx0, ny1, nz0, getAtlasU(TEX_LOG, 0), getAtlasV(TEX_LOG, 4));
		t.vertexUV(nx0, ny0, nz0, getAtlasU(TEX_LOG, 0), getAtlasV(TEX_LOG, 8));
		t.vertexUV(nx0, ny0, nz1, getAtlasU(TEX_LOG, 4), getAtlasV(TEX_LOG, 8));

		// East (end cut)
		t.vertexUV(nx1, ny1, nz0, getAtlasU(TEX_LOG, 4), getAtlasV(TEX_LOG, 4));
		t.vertexUV(nx1, ny1, nz1, getAtlasU(TEX_LOG, 0), getAtlasV(TEX_LOG, 4));
		t.vertexUV(nx1, ny0, nz1, getAtlasU(TEX_LOG, 0), getAtlasV(TEX_LOG, 8));
		t.vertexUV(nx1, ny0, nz0, getAtlasU(TEX_LOG, 4), getAtlasV(TEX_LOG, 8));

		// North (outer lit side)
		t.color(colNSR, colNSG, colNSB);
		t.vertexUV(nx0, ny1, nz0, getAtlasU(TEX_LIT, 0), getAtlasV(TEX_LIT, 0));
		t.vertexUV(nx1, ny1, nz0, getAtlasU(TEX_LIT, 16), getAtlasV(TEX_LIT, 0));
		t.vertexUV(nx1, ny0, nz0, getAtlasU(TEX_LIT, 16), getAtlasV(TEX_LIT, 4));
		t.vertexUV(nx0, ny0, nz0, getAtlasU(TEX_LIT, 0), getAtlasV(TEX_LIT, 4));

		// South (inner lit side)
		t.vertexUV(nx0, ny1, nz1, getAtlasU(TEX_LIT, 0), getAtlasV(TEX_LIT, 0));
		t.vertexUV(nx0, ny0, nz1, getAtlasU(TEX_LIT, 0), getAtlasV(TEX_LIT, 4));
		t.vertexUV(nx1, ny0, nz1, getAtlasU(TEX_LIT, 16), getAtlasV(TEX_LIT, 4));
		t.vertexUV(nx1, ny1, nz1, getAtlasU(TEX_LIT, 16), getAtlasV(TEX_LIT, 0));

		// 4. Top South Log (x: 0..16, y: 3..7, z: 11..15)
		float sx0 = 0.0f, sx1 = 1.0f;
		float sy0 = 3.0f / 16.0f, sy1 = 7.0f / 16.0f;
		float sz0 = 11.0f / 16.0f, sz1 = 15.0f / 16.0f;

		// Up (rot 180)
		t.color(colUpR, colUpG, colUpB);
		t.vertexUV(sx0, sy1, sz0, getAtlasU(TEX_LOG, 16), getAtlasV(TEX_LOG, 4));
		t.vertexUV(sx0, sy1, sz1, getAtlasU(TEX_LOG, 16), getAtlasV(TEX_LOG, 0));
		t.vertexUV(sx1, sy1, sz1, getAtlasU(TEX_LOG, 0), getAtlasV(TEX_LOG, 0));
		t.vertexUV(sx1, sy1, sz0, getAtlasU(TEX_LOG, 0), getAtlasV(TEX_LOG, 4));

		// Down
		t.color(colDownR, colDownG, colDownB);
		t.vertexUV(sx0, sy0, sz1, getAtlasU(TEX_LIT, 0), getAtlasV(TEX_LIT, 4));
		t.vertexUV(sx0, sy0, sz0, getAtlasU(TEX_LIT, 0), getAtlasV(TEX_LIT, 8));
		t.vertexUV(sx1, sy0, sz0, getAtlasU(TEX_LIT, 16), getAtlasV(TEX_LIT, 8));
		t.vertexUV(sx1, sy0, sz1, getAtlasU(TEX_LIT, 16), getAtlasV(TEX_LIT, 4));

		// West (end cut)
		t.color(colWER, colWEG, colWEB);
		t.vertexUV(sx0, sy1, sz1, getAtlasU(TEX_LOG, 4), getAtlasV(TEX_LOG, 4));
		t.vertexUV(sx0, sy1, sz0, getAtlasU(TEX_LOG, 0), getAtlasV(TEX_LOG, 4));
		t.vertexUV(sx0, sy0, sz0, getAtlasU(TEX_LOG, 0), getAtlasV(TEX_LOG, 8));
		t.vertexUV(sx0, sy0, sz1, getAtlasU(TEX_LOG, 4), getAtlasV(TEX_LOG, 8));

		// East (end cut)
		t.vertexUV(sx1, sy1, sz0, getAtlasU(TEX_LOG, 4), getAtlasV(TEX_LOG, 4));
		t.vertexUV(sx1, sy1, sz1, getAtlasU(TEX_LOG, 0), getAtlasV(TEX_LOG, 4));
		t.vertexUV(sx1, sy0, sz1, getAtlasU(TEX_LOG, 0), getAtlasV(TEX_LOG, 8));
		t.vertexUV(sx1, sy0, sz0, getAtlasU(TEX_LOG, 4), getAtlasV(TEX_LOG, 8));

		// North (inner lit side)
		t.color(colNSR, colNSG, colNSB);
		t.vertexUV(sx0, sy1, sz0, getAtlasU(TEX_LIT, 16), getAtlasV(TEX_LIT, 0));
		t.vertexUV(sx1, sy1, sz0, getAtlasU(TEX_LIT, 0), getAtlasV(TEX_LIT, 0));
		t.vertexUV(sx1, sy0, sz0, getAtlasU(TEX_LIT, 0), getAtlasV(TEX_LIT, 4));
		t.vertexUV(sx0, sy0, sz0, getAtlasU(TEX_LIT, 16), getAtlasV(TEX_LIT, 4));

		// South (outer lit side)
		t.vertexUV(sx0, sy1, sz1, getAtlasU(TEX_LIT, 16), getAtlasV(TEX_LIT, 0));
		t.vertexUV(sx0, sy0, sz1, getAtlasU(TEX_LIT, 16), getAtlasV(TEX_LIT, 4));
		t.vertexUV(sx1, sy0, sz1, getAtlasU(TEX_LIT, 0), getAtlasV(TEX_LIT, 4));
		t.vertexUV(sx1, sy1, sz1, getAtlasU(TEX_LIT, 0), getAtlasV(TEX_LIT, 0));

		// 5. Coals / Ash bed (x: 5..11, y: 0..1, z: 0..16)
		float ax0 = 5.0f / 16.0f, ax1 = 11.0f / 16.0f;
		float ay0 = 0.0f, ay1 = 1.0f / 16.0f;
		float az0 = 0.0f, az1 = 1.0f;

		// Up (ash coals glowing)
		t.color(colUpR, colUpG, colUpB);
		t.vertexUV(ax0, ay1, az0, getAtlasU(TEX_LIT, 0), getAtlasV(TEX_LIT, 14));
		t.vertexUV(ax0, ay1, az1, getAtlasU(TEX_LIT, 16), getAtlasV(TEX_LIT, 14));
		t.vertexUV(ax1, ay1, az1, getAtlasU(TEX_LIT, 16), getAtlasV(TEX_LIT, 8));
		t.vertexUV(ax1, ay1, az0, getAtlasU(TEX_LIT, 0), getAtlasV(TEX_LIT, 8));

		// Down
		t.color(colDownR, colDownG, colDownB);
		t.vertexUV(ax0, ay0, az1, getAtlasU(TEX_LOG, 16), getAtlasV(TEX_LOG, 8));
		t.vertexUV(ax0, ay0, az0, getAtlasU(TEX_LOG, 0), getAtlasV(TEX_LOG, 8));
		t.vertexUV(ax1, ay0, az0, getAtlasU(TEX_LOG, 0), getAtlasV(TEX_LOG, 14));
		t.vertexUV(ax1, ay0, az1, getAtlasU(TEX_LOG, 16), getAtlasV(TEX_LOG, 14));

		// North ash edge
		t.color(colNSR, colNSG, colNSB);
		t.vertexUV(ax0, ay1, az0, getAtlasU(TEX_LOG, 0), getAtlasV(TEX_LOG, 15));
		t.vertexUV(ax1, ay1, az0, getAtlasU(TEX_LOG, 6), getAtlasV(TEX_LOG, 15));
		t.vertexUV(ax1, ay0, az0, getAtlasU(TEX_LOG, 6), getAtlasV(TEX_LOG, 16));
		t.vertexUV(ax0, ay0, az0, getAtlasU(TEX_LOG, 0), getAtlasV(TEX_LOG, 16));

		// South ash edge
		t.vertexUV(ax0, ay1, az1, getAtlasU(TEX_LOG, 16), getAtlasV(TEX_LOG, 15));
		t.vertexUV(ax0, ay0, az1, getAtlasU(TEX_LOG, 16), getAtlasV(TEX_LOG, 16));
		t.vertexUV(ax1, ay0, az1, getAtlasU(TEX_LOG, 10), getAtlasV(TEX_LOG, 16));
		t.vertexUV(ax1, ay1, az1, getAtlasU(TEX_LOG, 10), getAtlasV(TEX_LOG, 15));

		// 6. Fire Cross Planes (Full Brightness)
		t.color(1.0f, 1.0f, 1.0f);
		float fx0 = 0.8f / 16.0f, fx1 = 15.2f / 16.0f;
		float fz0 = 0.8f / 16.0f, fz1 = 15.2f / 16.0f;
		float fy0 = 1.0f / 16.0f, fy1 = 17.0f / 16.0f;

		float fu0 = getAtlasU(TEX_FIRE, 0), fu1 = getAtlasU(TEX_FIRE, 16);
		float fv0 = getAtlasV(TEX_FIRE, 0), fv1 = getAtlasV(TEX_FIRE, 16);

		// Diagonal 1
		t.vertexUV(fx0, fy1, fz0, fu0, fv0);
		t.vertexUV(fx0, fy0, fz0, fu0, fv1);
		t.vertexUV(fx1, fy0, fz1, fu1, fv1);
		t.vertexUV(fx1, fy1, fz1, fu1, fv0);

		t.vertexUV(fx1, fy1, fz1, fu1, fv0);
		t.vertexUV(fx1, fy0, fz1, fu1, fv1);
		t.vertexUV(fx0, fy0, fz0, fu0, fv1);
		t.vertexUV(fx0, fy1, fz0, fu0, fv0);

		// Diagonal 2
		t.vertexUV(fx0, fy1, fz1, fu0, fv0);
		t.vertexUV(fx0, fy0, fz1, fu0, fv1);
		t.vertexUV(fx1, fy0, fz0, fu1, fv1);
		t.vertexUV(fx1, fy1, fz0, fu1, fv0);

		t.vertexUV(fx1, fy1, fz0, fu1, fv0);
		t.vertexUV(fx1, fy0, fz0, fu1, fv1);
		t.vertexUV(fx0, fy0, fz1, fu0, fv1);
		t.vertexUV(fx0, fy1, fz1, fu0, fv0);

		t.draw();
		t.addOffset(0.5f, 0.5f, 0.5f);
		tile->setShape(0, 0, 0, 1, 1, 1);
	} else if (shape == Tile::SHAPE_GRINDSTONE) {
		t.addOffset(-0.5f, -0.5f, -0.5f);
		t.begin();

		// Left Leg (151)
		tile->setShape(2.0f / 16.0f, 0.0f, 6.0f / 16.0f, 4.0f / 16.0f, 7.0f / 16.0f, 10.0f / 16.0f);
		t.color(tr, tg, tb); renderFaceUp(tile, 0, 0, 0, 151);
		t.color(tr * 0.5f, tg * 0.5f, tb * 0.5f); renderFaceDown(tile, 0, 0, 0, 151);
		t.color(tr * 0.8f, tg * 0.8f, tb * 0.8f); renderNorth(tile, 0, 0, 0, 151); renderSouth(tile, 0, 0, 0, 151);
		t.color(tr * 0.6f, tg * 0.6f, tb * 0.6f); renderWest(tile, 0, 0, 0, 151); renderEast(tile, 0, 0, 0, 151);

		// Right Leg (151)
		tile->setShape(12.0f / 16.0f, 0.0f, 6.0f / 16.0f, 14.0f / 16.0f, 7.0f / 16.0f, 10.0f / 16.0f);
		t.color(tr, tg, tb); renderFaceUp(tile, 0, 0, 0, 151);
		t.color(tr * 0.5f, tg * 0.5f, tb * 0.5f); renderFaceDown(tile, 0, 0, 0, 151);
		t.color(tr * 0.8f, tg * 0.8f, tb * 0.8f); renderNorth(tile, 0, 0, 0, 151); renderSouth(tile, 0, 0, 0, 151);
		t.color(tr * 0.6f, tg * 0.6f, tb * 0.6f); renderWest(tile, 0, 0, 0, 151); renderEast(tile, 0, 0, 0, 151);

		// Left Pivot Bracket (151)
		tile->setShape(2.0f / 16.0f, 7.0f / 16.0f, 5.0f / 16.0f, 4.0f / 16.0f, 13.0f / 16.0f, 11.0f / 16.0f);
		t.color(tr, tg, tb); renderFaceUp(tile, 0, 0, 0, 151);
		t.color(tr * 0.5f, tg * 0.5f, tb * 0.5f); renderFaceDown(tile, 0, 0, 0, 151);
		t.color(tr * 0.8f, tg * 0.8f, tb * 0.8f); renderNorth(tile, 0, 0, 0, 151); renderSouth(tile, 0, 0, 0, 151);
		t.color(tr * 0.6f, tg * 0.6f, tb * 0.6f); renderWest(tile, 0, 0, 0, 151); renderEast(tile, 0, 0, 0, 151);

		// Right Pivot Bracket (151)
		tile->setShape(12.0f / 16.0f, 7.0f / 16.0f, 5.0f / 16.0f, 14.0f / 16.0f, 13.0f / 16.0f, 11.0f / 16.0f);
		t.color(tr, tg, tb); renderFaceUp(tile, 0, 0, 0, 151);
		t.color(tr * 0.5f, tg * 0.5f, tb * 0.5f); renderFaceDown(tile, 0, 0, 0, 151);
		t.color(tr * 0.8f, tg * 0.8f, tb * 0.8f); renderNorth(tile, 0, 0, 0, 151); renderSouth(tile, 0, 0, 0, 151);
		t.color(tr * 0.6f, tg * 0.6f, tb * 0.6f); renderWest(tile, 0, 0, 0, 151); renderEast(tile, 0, 0, 0, 151);

		// Wheel (Round 17 on Up/Down/North/South, Side 21 on West/East)
		tile->setShape(4.0f / 16.0f, 4.0f / 16.0f, 2.0f / 16.0f, 12.0f / 16.0f, 1.0f, 14.0f / 16.0f);
		t.color(tr, tg, tb); renderFaceUp(tile, 0, 0, 0, 17);
		t.color(tr * 0.5f, tg * 0.5f, tb * 0.5f); renderFaceDown(tile, 0, 0, 0, 17);
		t.color(tr * 0.8f, tg * 0.8f, tb * 0.8f); renderNorth(tile, 0, 0, 0, 17); renderSouth(tile, 0, 0, 0, 17);
		t.color(tr * 0.6f, tg * 0.6f, tb * 0.6f); renderWest(tile, 0, 0, 0, 21); renderEast(tile, 0, 0, 0, 21);

		t.draw();
		t.addOffset(0.5f, 0.5f, 0.5f);
		tile->setShape(0, 0, 0, 1, 1, 1);
	} else if (shape == Tile::SHAPE_LECTERN) {
		t.addOffset(-0.5f, -0.5f, -0.5f);
		t.begin();

		const int TEX_BASE = 162;
		const int TEX_FRONT = 22;
		const int TEX_SIDES = 106;
		const int TEX_TOP = 107;

		float colUpR = tr, colUpG = tg, colUpB = tb;
		float colDownR = tr * 0.5f, colDownG = tg * 0.5f, colDownB = tb * 0.5f;
		float colNSR = tr * 0.8f, colNSG = tg * 0.8f, colNSB = tb * 0.8f;
		float colWER = tr * 0.6f, colWEG = tg * 0.6f, colWEB = tb * 0.6f;

		// 1. Base pedestal (x: 0..1, y: 0..2/16, z: 0..1)
		float bx0 = 0.0f, bx1 = 1.0f;
		float by0 = 0.0f, by1 = 2.0f / 16.0f;
		float bz0 = 0.0f, bz1 = 1.0f;

		// Base Up
		t.color(colUpR, colUpG, colUpB);
		t.vertexUV(bx0, by1, bz0, getAtlasU(TEX_BASE, 0), getAtlasV(TEX_BASE, 0));
		t.vertexUV(bx0, by1, bz1, getAtlasU(TEX_BASE, 0), getAtlasV(TEX_BASE, 16));
		t.vertexUV(bx1, by1, bz1, getAtlasU(TEX_BASE, 16), getAtlasV(TEX_BASE, 16));
		t.vertexUV(bx1, by1, bz0, getAtlasU(TEX_BASE, 16), getAtlasV(TEX_BASE, 0));

		// Base Down
		t.color(colDownR, colDownG, colDownB);
		t.vertexUV(bx0, by0, bz1, getAtlasU(TEX_BASE, 0), getAtlasV(TEX_BASE, 16));
		t.vertexUV(bx0, by0, bz0, getAtlasU(TEX_BASE, 0), getAtlasV(TEX_BASE, 0));
		t.vertexUV(bx1, by0, bz0, getAtlasU(TEX_BASE, 16), getAtlasV(TEX_BASE, 0));
		t.vertexUV(bx1, by0, bz1, getAtlasU(TEX_BASE, 16), getAtlasV(TEX_BASE, 16));

		// Base North ([0, 14, 16, 16])
		t.color(colNSR, colNSG, colNSB);
		t.vertexUV(bx0, by1, bz0, getAtlasU(TEX_BASE, 0), getAtlasV(TEX_BASE, 14));
		t.vertexUV(bx1, by1, bz0, getAtlasU(TEX_BASE, 16), getAtlasV(TEX_BASE, 14));
		t.vertexUV(bx1, by0, bz0, getAtlasU(TEX_BASE, 16), getAtlasV(TEX_BASE, 16));
		t.vertexUV(bx0, by0, bz0, getAtlasU(TEX_BASE, 0), getAtlasV(TEX_BASE, 16));

		// Base South ([0, 6, 16, 8])
		t.vertexUV(bx0, by1, bz1, getAtlasU(TEX_BASE, 0), getAtlasV(TEX_BASE, 6));
		t.vertexUV(bx0, by0, bz1, getAtlasU(TEX_BASE, 0), getAtlasV(TEX_BASE, 8));
		t.vertexUV(bx1, by0, bz1, getAtlasU(TEX_BASE, 16), getAtlasV(TEX_BASE, 8));
		t.vertexUV(bx1, by1, bz1, getAtlasU(TEX_BASE, 16), getAtlasV(TEX_BASE, 6));

		// Base West ([0, 6, 16, 8])
		t.color(colWER, colWEG, colWEB);
		t.vertexUV(bx0, by1, bz1, getAtlasU(TEX_BASE, 16), getAtlasV(TEX_BASE, 6));
		t.vertexUV(bx0, by1, bz0, getAtlasU(TEX_BASE, 0), getAtlasV(TEX_BASE, 6));
		t.vertexUV(bx0, by0, bz0, getAtlasU(TEX_BASE, 0), getAtlasV(TEX_BASE, 8));
		t.vertexUV(bx0, by0, bz1, getAtlasU(TEX_BASE, 16), getAtlasV(TEX_BASE, 8));

		// Base East ([0, 6, 16, 8])
		t.vertexUV(bx1, by1, bz0, getAtlasU(TEX_BASE, 0), getAtlasV(TEX_BASE, 6));
		t.vertexUV(bx1, by1, bz1, getAtlasU(TEX_BASE, 16), getAtlasV(TEX_BASE, 6));
		t.vertexUV(bx1, by0, bz1, getAtlasU(TEX_BASE, 16), getAtlasV(TEX_BASE, 8));
		t.vertexUV(bx1, by0, bz0, getAtlasU(TEX_BASE, 0), getAtlasV(TEX_BASE, 8));

		// 2. Central pillar (x: 4..12, y: 2..14, z: 4..12)
		float px0 = 4.0f / 16.0f, px1 = 12.0f / 16.0f;
		float py0 = 2.0f / 16.0f, py1 = 14.0f / 16.0f;
		float pz0 = 4.0f / 16.0f, pz1 = 12.0f / 16.0f;

		// Pillar North (front open face: [0, 0, 8, 12])
		t.color(colNSR, colNSG, colNSB);
		t.vertexUV(px0, py1, pz0, getAtlasU(TEX_FRONT, 0), getAtlasV(TEX_FRONT, 0));
		t.vertexUV(px1, py1, pz0, getAtlasU(TEX_FRONT, 8), getAtlasV(TEX_FRONT, 0));
		t.vertexUV(px1, py0, pz0, getAtlasU(TEX_FRONT, 8), getAtlasV(TEX_FRONT, 12));
		t.vertexUV(px0, py0, pz0, getAtlasU(TEX_FRONT, 0), getAtlasV(TEX_FRONT, 12));

		// Pillar South (back face: [8, 4, 16, 16])
		t.vertexUV(px0, py1, pz1, getAtlasU(TEX_FRONT, 8), getAtlasV(TEX_FRONT, 4));
		t.vertexUV(px0, py0, pz1, getAtlasU(TEX_FRONT, 8), getAtlasV(TEX_FRONT, 16));
		t.vertexUV(px1, py0, pz1, getAtlasU(TEX_FRONT, 16), getAtlasV(TEX_FRONT, 16));
		t.vertexUV(px1, py1, pz1, getAtlasU(TEX_FRONT, 16), getAtlasV(TEX_FRONT, 4));

		// Pillar West (side face: [0, 2, 8, 14])
		t.color(colWER, colWEG, colWEB);
		t.vertexUV(px0, py1, pz1, getAtlasU(TEX_SIDES, 8), getAtlasV(TEX_SIDES, 2));
		t.vertexUV(px0, py1, pz0, getAtlasU(TEX_SIDES, 0), getAtlasV(TEX_SIDES, 2));
		t.vertexUV(px0, py0, pz0, getAtlasU(TEX_SIDES, 0), getAtlasV(TEX_SIDES, 14));
		t.vertexUV(px0, py0, pz1, getAtlasU(TEX_SIDES, 8), getAtlasV(TEX_SIDES, 14));

		// Pillar East (side face: [0, 2, 8, 14])
		t.vertexUV(px1, py1, pz0, getAtlasU(TEX_SIDES, 0), getAtlasV(TEX_SIDES, 2));
		t.vertexUV(px1, py1, pz1, getAtlasU(TEX_SIDES, 8), getAtlasV(TEX_SIDES, 2));
		t.vertexUV(px1, py0, pz1, getAtlasU(TEX_SIDES, 8), getAtlasV(TEX_SIDES, 14));
		t.vertexUV(px1, py0, pz0, getAtlasU(TEX_SIDES, 0), getAtlasV(TEX_SIDES, 14));

		// 3. Top reading desk (x: 0..1, y: 12..15, z: 2..16)
		float dx0 = 0.0f, dx1 = 1.0f;
		float dy0 = 12.0f / 16.0f, dy1 = 15.0f / 16.0f;
		float dz0 = 2.0f / 16.0f, dz1 = 1.0f;

		// Top surface (the book/desk face: [0, 1, 16, 15] on TEX_TOP!)
		t.color(colUpR, colUpG, colUpB);
		t.vertexUV(dx0, dy1, dz0, getAtlasU(TEX_TOP, 0), getAtlasV(TEX_TOP, 1));
		t.vertexUV(dx0, dy1, dz1, getAtlasU(TEX_TOP, 0), getAtlasV(TEX_TOP, 15));
		t.vertexUV(dx1, dy1, dz1, getAtlasU(TEX_TOP, 16), getAtlasV(TEX_TOP, 15));
		t.vertexUV(dx1, dy1, dz0, getAtlasU(TEX_TOP, 16), getAtlasV(TEX_TOP, 1));

		// Bottom surface (wood bottom: [0, 0, 16, 14] on TEX_BASE)
		t.color(colDownR, colDownG, colDownB);
		t.vertexUV(dx0, dy0, dz1, getAtlasU(TEX_BASE, 0), getAtlasV(TEX_BASE, 14));
		t.vertexUV(dx0, dy0, dz0, getAtlasU(TEX_BASE, 0), getAtlasV(TEX_BASE, 0));
		t.vertexUV(dx1, dy0, dz0, getAtlasU(TEX_BASE, 16), getAtlasV(TEX_BASE, 0));
		t.vertexUV(dx1, dy0, dz1, getAtlasU(TEX_BASE, 16), getAtlasV(TEX_BASE, 14));

		// Desk North edge: [0, 0, 16, 3] on TEX_SIDES
		t.color(colNSR, colNSG, colNSB);
		t.vertexUV(dx0, dy1, dz0, getAtlasU(TEX_SIDES, 0), getAtlasV(TEX_SIDES, 0));
		t.vertexUV(dx1, dy1, dz0, getAtlasU(TEX_SIDES, 16), getAtlasV(TEX_SIDES, 0));
		t.vertexUV(dx1, dy0, dz0, getAtlasU(TEX_SIDES, 16), getAtlasV(TEX_SIDES, 3));
		t.vertexUV(dx0, dy0, dz0, getAtlasU(TEX_SIDES, 0), getAtlasV(TEX_SIDES, 3));

		// Desk South edge: [0, 4, 16, 7] on TEX_SIDES
		t.vertexUV(dx0, dy1, dz1, getAtlasU(TEX_SIDES, 0), getAtlasV(TEX_SIDES, 4));
		t.vertexUV(dx0, dy0, dz1, getAtlasU(TEX_SIDES, 0), getAtlasV(TEX_SIDES, 7));
		t.vertexUV(dx1, dy0, dz1, getAtlasU(TEX_SIDES, 16), getAtlasV(TEX_SIDES, 7));
		t.vertexUV(dx1, dy1, dz1, getAtlasU(TEX_SIDES, 16), getAtlasV(TEX_SIDES, 4));

		// Desk West edge: [0, 4, 14, 7] on TEX_SIDES
		t.color(colWER, colWEG, colWEB);
		t.vertexUV(dx0, dy1, dz1, getAtlasU(TEX_SIDES, 14), getAtlasV(TEX_SIDES, 4));
		t.vertexUV(dx0, dy1, dz0, getAtlasU(TEX_SIDES, 0), getAtlasV(TEX_SIDES, 4));
		t.vertexUV(dx0, dy0, dz0, getAtlasU(TEX_SIDES, 0), getAtlasV(TEX_SIDES, 7));
		t.vertexUV(dx0, dy0, dz1, getAtlasU(TEX_SIDES, 14), getAtlasV(TEX_SIDES, 7));

		// Desk East edge: [0, 4, 14, 7] on TEX_SIDES
		t.vertexUV(dx1, dy1, dz0, getAtlasU(TEX_SIDES, 0), getAtlasV(TEX_SIDES, 4));
		t.vertexUV(dx1, dy1, dz1, getAtlasU(TEX_SIDES, 14), getAtlasV(TEX_SIDES, 4));
		t.vertexUV(dx1, dy0, dz1, getAtlasU(TEX_SIDES, 14), getAtlasV(TEX_SIDES, 7));
		t.vertexUV(dx1, dy0, dz0, getAtlasU(TEX_SIDES, 0), getAtlasV(TEX_SIDES, 7));

		t.draw();
		t.addOffset(0.5f, 0.5f, 0.5f);
		tile->setShape(0, 0, 0, 1, 1, 1);
	} else if (shape == Tile::SHAPE_COMPOSTER) {
		t.addOffset(-0.5f, -0.5f, -0.5f);
		t.begin();

		// Bottom
		tile->setShape(2.0f / 16.0f, 0.0f, 2.0f / 16.0f, 14.0f / 16.0f, 2.0f / 16.0f, 14.0f / 16.0f);
		t.color(tr, tg, tb); renderFaceUp(tile, 0, 0, 0, 14);
		t.color(tr * 0.5f, tg * 0.5f, tb * 0.5f); renderFaceDown(tile, 0, 0, 0, 255);
		t.color(tr * 0.8f, tg * 0.8f, tb * 0.8f); renderNorth(tile, 0, 0, 0, 14); renderSouth(tile, 0, 0, 0, 14);
		t.color(tr * 0.6f, tg * 0.6f, tb * 0.6f); renderWest(tile, 0, 0, 0, 14); renderEast(tile, 0, 0, 0, 14);

		// North wall
		tile->setShape(0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 2.0f / 16.0f);
		t.color(tr, tg, tb); renderFaceUp(tile, 0, 0, 0, 131);
		t.color(tr * 0.5f, tg * 0.5f, tb * 0.5f); renderFaceDown(tile, 0, 0, 0, 255);
		t.color(tr * 0.8f, tg * 0.8f, tb * 0.8f); renderNorth(tile, 0, 0, 0, 14); renderSouth(tile, 0, 0, 0, 14);
		t.color(tr * 0.6f, tg * 0.6f, tb * 0.6f); renderWest(tile, 0, 0, 0, 14); renderEast(tile, 0, 0, 0, 14);

		// South wall
		tile->setShape(0.0f, 0.0f, 14.0f / 16.0f, 1.0f, 1.0f, 1.0f);
		t.color(tr, tg, tb); renderFaceUp(tile, 0, 0, 0, 131);
		t.color(tr * 0.5f, tg * 0.5f, tb * 0.5f); renderFaceDown(tile, 0, 0, 0, 255);
		t.color(tr * 0.8f, tg * 0.8f, tb * 0.8f); renderNorth(tile, 0, 0, 0, 14); renderSouth(tile, 0, 0, 0, 14);
		t.color(tr * 0.6f, tg * 0.6f, tb * 0.6f); renderWest(tile, 0, 0, 0, 14); renderEast(tile, 0, 0, 0, 14);

		// West wall
		tile->setShape(0.0f, 0.0f, 2.0f / 16.0f, 2.0f / 16.0f, 1.0f, 14.0f / 16.0f);
		t.color(tr, tg, tb); renderFaceUp(tile, 0, 0, 0, 131);
		t.color(tr * 0.5f, tg * 0.5f, tb * 0.5f); renderFaceDown(tile, 0, 0, 0, 255);
		t.color(tr * 0.8f, tg * 0.8f, tb * 0.8f); renderNorth(tile, 0, 0, 0, 14); renderSouth(tile, 0, 0, 0, 14);
		t.color(tr * 0.6f, tg * 0.6f, tb * 0.6f); renderWest(tile, 0, 0, 0, 14); renderEast(tile, 0, 0, 0, 14);

		// East wall
		tile->setShape(14.0f / 16.0f, 0.0f, 2.0f / 16.0f, 1.0f, 1.0f, 14.0f / 16.0f);
		t.color(tr, tg, tb); renderFaceUp(tile, 0, 0, 0, 131);
		t.color(tr * 0.5f, tg * 0.5f, tb * 0.5f); renderFaceDown(tile, 0, 0, 0, 255);
		t.color(tr * 0.8f, tg * 0.8f, tb * 0.8f); renderNorth(tile, 0, 0, 0, 14); renderSouth(tile, 0, 0, 0, 14);
		t.color(tr * 0.6f, tg * 0.6f, tb * 0.6f); renderWest(tile, 0, 0, 0, 14); renderEast(tile, 0, 0, 0, 14);

		t.draw();
		t.addOffset(0.5f, 0.5f, 0.5f);
		tile->setShape(0, 0, 0, 1, 1, 1);
	} else if (shape == Tile::SHAPE_STONECUTTER) {
		t.addOffset(-0.5f, -0.5f, -0.5f);
		t.begin();

		// 1. Table base
		tile->setShape(0.0f, 0.0f, 0.0f, 1.0f, 9.0f / 16.0f, 1.0f);
		t.color(tr, tg, tb); renderFaceUp(tile, 0, 0, 0, 251);
		t.color(tr * 0.5f, tg * 0.5f, tb * 0.5f); renderFaceDown(tile, 0, 0, 0, 252);
		t.color(tr * 0.8f, tg * 0.8f, tb * 0.8f); renderNorth(tile, 0, 0, 0, 253); renderSouth(tile, 0, 0, 0, 253);
		t.color(tr * 0.6f, tg * 0.6f, tb * 0.6f); renderWest(tile, 0, 0, 0, 253); renderEast(tile, 0, 0, 0, 253);

		// 2. Saw blade
		tile->setShape(1.0f / 16.0f, 9.0f / 16.0f, 7.5f / 16.0f, 15.0f / 16.0f, 1.0f, 8.5f / 16.0f);
		t.color(tr, tg, tb); renderFaceUp(tile, 0, 0, 0, 254);
		t.color(tr * 0.5f, tg * 0.5f, tb * 0.5f); renderFaceDown(tile, 0, 0, 0, 254);
		t.color(tr * 0.8f, tg * 0.8f, tb * 0.8f); renderNorth(tile, 0, 0, 0, 254); renderSouth(tile, 0, 0, 0, 254);
		t.color(tr * 0.6f, tg * 0.6f, tb * 0.6f); renderWest(tile, 0, 0, 0, 254); renderEast(tile, 0, 0, 0, 254);

		t.draw();
		t.addOffset(0.5f, 0.5f, 0.5f);
		tile->setShape(0, 0, 0, 1, 1, 1);
	} else if (shape == Tile::SHAPE_CHAIN) {
		t.addOffset(-0.5f, -0.5f, -0.5f);
		t.begin();

		tile->setShape(6.5f / 16.0f, 0.0f, 6.5f / 16.0f, 9.5f / 16.0f, 1.0f, 9.5f / 16.0f);
		t.color(tr, tg, tb); renderFaceUp(tile, 0, 0, 0, 155);
		t.color(tr * 0.5f, tg * 0.5f, tb * 0.5f); renderFaceDown(tile, 0, 0, 0, 155);
		t.color(tr * 0.8f, tg * 0.8f, tb * 0.8f); renderNorth(tile, 0, 0, 0, 155); renderSouth(tile, 0, 0, 0, 155);
		t.color(tr * 0.6f, tg * 0.6f, tb * 0.6f); renderWest(tile, 0, 0, 0, 155); renderEast(tile, 0, 0, 0, 155);

		t.draw();
		t.addOffset(0.5f, 0.5f, 0.5f);
		tile->setShape(0, 0, 0, 1, 1, 1);
	} else if (shape == Tile::SHAPE_SCAFFOLDING) {
		t.addOffset(-0.5f, -0.5f, -0.5f);
		t.begin();

		int cleanTex = tile->getTexture(0, data) & ~Tile::TEXTURE_ALT_FLAG;
		for (int i = 0; i < 5; i++) {
			if (i == 0) tile->setShape(0.0f, 0.0f, 0.0f, 2.0f / 16.0f, 1.0f, 2.0f / 16.0f);
			if (i == 1) tile->setShape(14.0f / 16.0f, 0.0f, 0.0f, 1.0f, 1.0f, 2.0f / 16.0f);
			if (i == 2) tile->setShape(0.0f, 0.0f, 14.0f / 16.0f, 2.0f / 16.0f, 1.0f, 1.0f);
			if (i == 3) tile->setShape(14.0f / 16.0f, 0.0f, 14.0f / 16.0f, 1.0f, 1.0f, 1.0f);
			if (i == 4) tile->setShape(0.0f, 15.0f / 16.0f, 0.0f, 1.0f, 1.0f, 1.0f);

			t.color(tr, tg, tb); renderFaceUp(tile, 0, 0, 0, cleanTex);
			t.color(tr * 0.5f, tg * 0.5f, tb * 0.5f); renderFaceDown(tile, 0, 0, 0, cleanTex);
			t.color(tr * 0.8f, tg * 0.8f, tb * 0.8f); renderNorth(tile, 0, 0, 0, cleanTex); renderSouth(tile, 0, 0, 0, cleanTex);
			t.color(tr * 0.6f, tg * 0.6f, tb * 0.6f); renderWest(tile, 0, 0, 0, cleanTex); renderEast(tile, 0, 0, 0, cleanTex);
		}

		t.draw();
		t.addOffset(0.5f, 0.5f, 0.5f);
		tile->setShape(0, 0, 0, 1, 1, 1);
	} else if (shape == Tile::SHAPE_CANDLE) {
		t.addOffset(-0.5f, -0.5f, -0.5f);
		t.begin();

		int tex = tile->getTexture(0, data) & ~Tile::TEXTURE_ALT_FLAG;
		float tr = ((color >> 16) & 0xff) / 255.0f;
		float tg = ((color >> 8) & 0xff) / 255.0f;
		float tb = (color & 0xff) / 255.0f;

		float colUpR = tr, colUpG = tg, colUpB = tb;
		float colDownR = tr * 0.5f, colDownG = tg * 0.5f, colDownB = tb * 0.5f;
		float colNSR = tr * 0.8f, colNSG = tg * 0.8f, colNSB = tb * 0.8f;
		float colWER = tr * 0.6f, colWEG = tg * 0.6f, colWEB = tb * 0.6f;

		float cx0 = 7.0f / 16.0f, cx1 = 9.0f / 16.0f;
		float cy0 = 0.0f / 16.0f, cy1 = 6.0f / 16.0f;
		float cz0 = 7.0f / 16.0f, cz1 = 9.0f / 16.0f;

		float uTop0 = getAtlasU(tex, 0.0f), uTop1 = getAtlasU(tex, 2.0f);
		float vTop0 = getAtlasV(tex, 6.0f), vTop1 = getAtlasV(tex, 8.0f);

		float uSide0 = getAtlasU(tex, 0.0f), uSide1 = getAtlasU(tex, 2.0f);
		float vSide0 = getAtlasV(tex, 6.0f), vSide1 = getAtlasV(tex, 12.0f);

		// Top
		t.color(colUpR, colUpG, colUpB);
		t.vertexUV(cx0, cy1, cz0, uTop0, vTop0);
		t.vertexUV(cx0, cy1, cz1, uTop0, vTop1);
		t.vertexUV(cx1, cy1, cz1, uTop1, vTop1);
		t.vertexUV(cx1, cy1, cz0, uTop1, vTop0);

		// Down
		t.color(colDownR, colDownG, colDownB);
		t.vertexUV(cx0, cy0, cz1, uTop0, vTop1);
		t.vertexUV(cx0, cy0, cz0, uTop0, vTop0);
		t.vertexUV(cx1, cy0, cz0, uTop1, vTop0);
		t.vertexUV(cx1, cy0, cz1, uTop1, vTop1);

		// North & South
		t.color(colNSR, colNSG, colNSB);
		t.vertexUV(cx0, cy1, cz0, uSide1, vSide0);
		t.vertexUV(cx1, cy1, cz0, uSide0, vSide0);
		t.vertexUV(cx1, cy0, cz0, uSide0, vSide1);
		t.vertexUV(cx0, cy0, cz0, uSide1, vSide1);
		t.vertexUV(cx1, cy1, cz0, uSide0, vSide0);
		t.vertexUV(cx0, cy1, cz0, uSide1, vSide0);
		t.vertexUV(cx0, cy0, cz0, uSide1, vSide1);
		t.vertexUV(cx1, cy0, cz0, uSide0, vSide1);

		t.vertexUV(cx0, cy1, cz1, uSide0, vSide0);
		t.vertexUV(cx0, cy0, cz1, uSide0, vSide1);
		t.vertexUV(cx1, cy0, cz1, uSide1, vSide1);
		t.vertexUV(cx1, cy1, cz1, uSide1, vSide0);
		t.vertexUV(cx1, cy1, cz1, uSide1, vSide0);
		t.vertexUV(cx1, cy0, cz1, uSide1, vSide1);
		t.vertexUV(cx0, cy0, cz1, uSide0, vSide1);
		t.vertexUV(cx0, cy1, cz1, uSide0, vSide0);

		// West & East
		t.color(colWER, colWEG, colWEB);
		t.vertexUV(cx0, cy1, cz1, uSide1, vSide0);
		t.vertexUV(cx0, cy1, cz0, uSide0, vSide0);
		t.vertexUV(cx0, cy0, cz0, uSide0, vSide1);
		t.vertexUV(cx0, cy0, cz1, uSide1, vSide1);
		t.vertexUV(cx0, cy1, cz0, uSide0, vSide0);
		t.vertexUV(cx0, cy1, cz1, uSide1, vSide0);
		t.vertexUV(cx0, cy0, cz1, uSide1, vSide1);
		t.vertexUV(cx0, cy0, cz0, uSide0, vSide1);

		t.vertexUV(cx1, cy0, cz1, uSide0, vSide1);
		t.vertexUV(cx1, cy0, cz0, uSide1, vSide1);
		t.vertexUV(cx1, cy1, cz0, uSide1, vSide0);
		t.vertexUV(cx1, cy1, cz1, uSide0, vSide0);
		t.vertexUV(cx1, cy1, cz0, uSide1, vSide0);
		t.vertexUV(cx1, cy0, cz0, uSide1, vSide1);
		t.vertexUV(cx1, cy0, cz1, uSide0, vSide1);
		t.vertexUV(cx1, cy1, cz1, uSide0, vSide0);

		// Wick
		float wu0 = getAtlasU(tex, 0.0f), wu1 = getAtlasU(tex, 1.0f);
		float wv0 = getAtlasV(tex, 4.5f), wv1 = getAtlasV(tex, 6.0f);
		float wy0 = 6.0f / 16.0f, wy1 = 8.5f / 16.0f;
		float wx0 = 7.5f / 16.0f, wx1 = 8.5f / 16.0f;
		float wz0 = 7.5f / 16.0f, wz1 = 8.5f / 16.0f;
		float wMidX = 8.0f / 16.0f;
		float wMidZ = 8.0f / 16.0f;

		t.color(colUpR, colUpG, colUpB);
		// Quad X (double sided)
		t.vertexUV(wx0, wy1, wMidZ, wu0, wv0);
		t.vertexUV(wx0, wy0, wMidZ, wu0, wv1);
		t.vertexUV(wx1, wy0, wMidZ, wu1, wv1);
		t.vertexUV(wx1, wy1, wMidZ, wu1, wv0);

		t.vertexUV(wx1, wy1, wMidZ, wu1, wv0);
		t.vertexUV(wx1, wy0, wMidZ, wu1, wv1);
		t.vertexUV(wx0, wy0, wMidZ, wu0, wv1);
		t.vertexUV(wx0, wy1, wMidZ, wu0, wv0);

		// Quad Z (double sided)
		t.vertexUV(wMidX, wy1, wz0, wu0, wv0);
		t.vertexUV(wMidX, wy0, wz0, wu0, wv1);
		t.vertexUV(wMidX, wy0, wz1, wu1, wv1);
		t.vertexUV(wMidX, wy1, wz1, wu1, wv0);

		t.vertexUV(wMidX, wy1, wz1, wu1, wv0);
		t.vertexUV(wMidX, wy0, wz1, wu1, wv1);
		t.vertexUV(wMidX, wy0, wz0, wu0, wv1);
		t.vertexUV(wMidX, wy1, wz0, wu0, wv0);

		t.draw();
		t.addOffset(0.5f, 0.5f, 0.5f);
		tile->setShape(0, 0, 0, 1, 1, 1);
	} else if (shape == Tile::SHAPE_CAULDRON) {
		t.addOffset(-0.5f, -0.5f, -0.5f);
		t.begin();

		const int TEX_TOP = 149;
		const int TEX_INNER = 150;
		const int TEX_SIDE = 163;
		const int TEX_BOTTOM = 165;

		// 4 legs
		tile->setShape(0.0f, 0.0f, 0.0f, 2.0f / 16.0f, 3.0f / 16.0f, 2.0f / 16.0f);
		t.color(tr * 0.5f, tg * 0.5f, tb * 0.5f); renderFaceDown(tile, 0, 0, 0, TEX_BOTTOM);
		t.color(tr * 0.8f, tg * 0.8f, tb * 0.8f); renderNorth(tile, 0, 0, 0, TEX_SIDE); renderSouth(tile, 0, 0, 0, TEX_SIDE);
		t.color(tr * 0.6f, tg * 0.6f, tb * 0.6f); renderWest(tile, 0, 0, 0, TEX_SIDE); renderEast(tile, 0, 0, 0, TEX_SIDE);

		tile->setShape(14.0f / 16.0f, 0.0f, 0.0f, 1.0f, 3.0f / 16.0f, 2.0f / 16.0f);
		t.color(tr * 0.5f, tg * 0.5f, tb * 0.5f); renderFaceDown(tile, 0, 0, 0, TEX_BOTTOM);
		t.color(tr * 0.8f, tg * 0.8f, tb * 0.8f); renderNorth(tile, 0, 0, 0, TEX_SIDE); renderSouth(tile, 0, 0, 0, TEX_SIDE);
		t.color(tr * 0.6f, tg * 0.6f, tb * 0.6f); renderWest(tile, 0, 0, 0, TEX_SIDE); renderEast(tile, 0, 0, 0, TEX_SIDE);

		tile->setShape(0.0f, 0.0f, 14.0f / 16.0f, 2.0f / 16.0f, 3.0f / 16.0f, 1.0f);
		t.color(tr * 0.5f, tg * 0.5f, tb * 0.5f); renderFaceDown(tile, 0, 0, 0, TEX_BOTTOM);
		t.color(tr * 0.8f, tg * 0.8f, tb * 0.8f); renderNorth(tile, 0, 0, 0, TEX_SIDE); renderSouth(tile, 0, 0, 0, TEX_SIDE);
		t.color(tr * 0.6f, tg * 0.6f, tb * 0.6f); renderWest(tile, 0, 0, 0, TEX_SIDE); renderEast(tile, 0, 0, 0, TEX_SIDE);

		tile->setShape(14.0f / 16.0f, 0.0f, 14.0f / 16.0f, 1.0f, 3.0f / 16.0f, 1.0f);
		t.color(tr * 0.5f, tg * 0.5f, tb * 0.5f); renderFaceDown(tile, 0, 0, 0, TEX_BOTTOM);
		t.color(tr * 0.8f, tg * 0.8f, tb * 0.8f); renderNorth(tile, 0, 0, 0, TEX_SIDE); renderSouth(tile, 0, 0, 0, TEX_SIDE);
		t.color(tr * 0.6f, tg * 0.6f, tb * 0.6f); renderWest(tile, 0, 0, 0, TEX_SIDE); renderEast(tile, 0, 0, 0, TEX_SIDE);

		// Bottom basin floor
		tile->setShape(2.0f / 16.0f, 3.0f / 16.0f, 2.0f / 16.0f, 14.0f / 16.0f, 5.0f / 16.0f, 14.0f / 16.0f);
		t.color(tr, tg, tb); renderFaceUp(tile, 0, 0, 0, TEX_INNER);
		t.color(tr * 0.5f, tg * 0.5f, tb * 0.5f); renderFaceDown(tile, 0, 0, 0, TEX_BOTTOM);

		// 4 walls
		// North wall
		tile->setShape(0.0f, 3.0f / 16.0f, 0.0f, 1.0f, 1.0f, 2.0f / 16.0f);
		t.color(tr, tg, tb); renderFaceUp(tile, 0, 0, 0, TEX_TOP);
		t.color(tr * 0.5f, tg * 0.5f, tb * 0.5f); renderFaceDown(tile, 0, 0, 0, TEX_BOTTOM);
		t.color(tr * 0.8f, tg * 0.8f, tb * 0.8f); renderNorth(tile, 0, 0, 0, TEX_SIDE); renderSouth(tile, 0, 0, 0, TEX_INNER);
		t.color(tr * 0.6f, tg * 0.6f, tb * 0.6f); renderWest(tile, 0, 0, 0, TEX_SIDE); renderEast(tile, 0, 0, 0, TEX_SIDE);

		// South wall
		tile->setShape(0.0f, 3.0f / 16.0f, 14.0f / 16.0f, 1.0f, 1.0f, 1.0f);
		t.color(tr, tg, tb); renderFaceUp(tile, 0, 0, 0, TEX_TOP);
		t.color(tr * 0.5f, tg * 0.5f, tb * 0.5f); renderFaceDown(tile, 0, 0, 0, TEX_BOTTOM);
		t.color(tr * 0.8f, tg * 0.8f, tb * 0.8f); renderNorth(tile, 0, 0, 0, TEX_INNER); renderSouth(tile, 0, 0, 0, TEX_SIDE);
		t.color(tr * 0.6f, tg * 0.6f, tb * 0.6f); renderWest(tile, 0, 0, 0, TEX_SIDE); renderEast(tile, 0, 0, 0, TEX_SIDE);

		// West wall
		tile->setShape(0.0f, 3.0f / 16.0f, 2.0f / 16.0f, 2.0f / 16.0f, 1.0f, 14.0f / 16.0f);
		t.color(tr, tg, tb); renderFaceUp(tile, 0, 0, 0, TEX_TOP);
		t.color(tr * 0.5f, tg * 0.5f, tb * 0.5f); renderFaceDown(tile, 0, 0, 0, TEX_BOTTOM);
		t.color(tr * 0.6f, tg * 0.6f, tb * 0.6f); renderWest(tile, 0, 0, 0, TEX_SIDE); renderEast(tile, 0, 0, 0, TEX_INNER);

		// East wall
		tile->setShape(14.0f / 16.0f, 3.0f / 16.0f, 2.0f / 16.0f, 1.0f, 1.0f, 14.0f / 16.0f);
		t.color(tr, tg, tb); renderFaceUp(tile, 0, 0, 0, TEX_TOP);
		t.color(tr * 0.5f, tg * 0.5f, tb * 0.5f); renderFaceDown(tile, 0, 0, 0, TEX_BOTTOM);
		t.color(tr * 0.6f, tg * 0.6f, tb * 0.6f); renderWest(tile, 0, 0, 0, TEX_INNER); renderEast(tile, 0, 0, 0, TEX_SIDE);

		t.draw();
		t.addOffset(0.5f, 0.5f, 0.5f);
		tile->setShape(0, 0, 0, 1, 1, 1);
	} else if (shape == Tile::SHAPE_ANVIL) {
		t.addOffset(-0.5f, -0.5f, -0.5f);
		t.begin();

		int dmg = 0;
		AnvilTile* at = dynamic_cast<AnvilTile*>(tile);
		if (at) dmg = at->damageType;
		else if (tile == Tile::chippedAnvil) dmg = 1;
		else if (tile == Tile::damagedAnvil) dmg = 2;

		int topTex = (dmg == 0 ? 170 : (dmg == 1 ? 180 : 185));
		int baseTex = 166;

		// Base: (2..14, 0..4, 2..14)
		tile->setShape(2.0f / 16.0f, 0.0f, 2.0f / 16.0f, 14.0f / 16.0f, 4.0f / 16.0f, 14.0f / 16.0f);
		t.color(tr, tg, tb); renderFaceUp(tile, 0, 0, 0, baseTex);
		t.color(tr * 0.5f, tg * 0.5f, tb * 0.5f); renderFaceDown(tile, 0, 0, 0, baseTex);
		t.color(tr * 0.8f, tg * 0.8f, tb * 0.8f); renderNorth(tile, 0, 0, 0, baseTex); renderSouth(tile, 0, 0, 0, baseTex);
		t.color(tr * 0.6f, tg * 0.6f, tb * 0.6f); renderWest(tile, 0, 0, 0, baseTex); renderEast(tile, 0, 0, 0, baseTex);

		// Lower neck: (4..12, 4..5, 5..11)
		tile->setShape(4.0f / 16.0f, 4.0f / 16.0f, 5.0f / 16.0f, 12.0f / 16.0f, 5.0f / 16.0f, 11.0f / 16.0f);
		t.color(tr, tg, tb); renderFaceUp(tile, 0, 0, 0, baseTex);
		t.color(tr * 0.5f, tg * 0.5f, tb * 0.5f); renderFaceDown(tile, 0, 0, 0, baseTex);
		t.color(tr * 0.8f, tg * 0.8f, tb * 0.8f); renderNorth(tile, 0, 0, 0, baseTex); renderSouth(tile, 0, 0, 0, baseTex);
		t.color(tr * 0.6f, tg * 0.6f, tb * 0.6f); renderWest(tile, 0, 0, 0, baseTex); renderEast(tile, 0, 0, 0, baseTex);

		// Stem: (6..10, 5..10, 6..10)
		tile->setShape(6.0f / 16.0f, 5.0f / 16.0f, 6.0f / 16.0f, 10.0f / 16.0f, 10.0f / 16.0f, 10.0f / 16.0f);
		t.color(tr, tg, tb); renderFaceUp(tile, 0, 0, 0, baseTex);
		t.color(tr * 0.5f, tg * 0.5f, tb * 0.5f); renderFaceDown(tile, 0, 0, 0, baseTex);
		t.color(tr * 0.8f, tg * 0.8f, tb * 0.8f); renderNorth(tile, 0, 0, 0, baseTex); renderSouth(tile, 0, 0, 0, baseTex);
		t.color(tr * 0.6f, tg * 0.6f, tb * 0.6f); renderWest(tile, 0, 0, 0, baseTex); renderEast(tile, 0, 0, 0, baseTex);

		// Head: (3..13, 10..16, 0..16)
		tile->setShape(3.0f / 16.0f, 10.0f / 16.0f, 0.0f, 13.0f / 16.0f, 1.0f, 1.0f);
		t.color(tr, tg, tb); renderFaceUp(tile, 0, 0, 0, topTex);
		t.color(tr * 0.5f, tg * 0.5f, tb * 0.5f); renderFaceDown(tile, 0, 0, 0, baseTex);
		t.color(tr * 0.8f, tg * 0.8f, tb * 0.8f); renderNorth(tile, 0, 0, 0, baseTex); renderSouth(tile, 0, 0, 0, baseTex);
		t.color(tr * 0.6f, tg * 0.6f, tb * 0.6f); renderWest(tile, 0, 0, 0, baseTex); renderEast(tile, 0, 0, 0, baseTex);

		t.draw();
		t.addOffset(0.5f, 0.5f, 0.5f);
		tile->setShape(0, 0, 0, 1, 1, 1);
	} else if (shape == Tile::SHAPE_HOPPER) {
		t.addOffset(-0.5f, -0.5f, -0.5f);
		t.begin();

		const int TEX_TOP = 195;
		const int TEX_INSIDE = 199;
		const int TEX_OUTSIDE = 200;

		// 1. Top basin floor
		tile->setShape(2.0f / 16.0f, 10.0f / 16.0f, 2.0f / 16.0f, 14.0f / 16.0f, 11.0f / 16.0f, 14.0f / 16.0f);
		t.color(tr, tg, tb); renderFaceUp(tile, 0, 0, 0, TEX_INSIDE);
		t.color(tr * 0.5f, tg * 0.5f, tb * 0.5f); renderFaceDown(tile, 0, 0, 0, TEX_OUTSIDE);
		t.color(tr * 0.8f, tg * 0.8f, tb * 0.8f); renderNorth(tile, 0, 0, 0, TEX_OUTSIDE); renderSouth(tile, 0, 0, 0, TEX_OUTSIDE);
		t.color(tr * 0.6f, tg * 0.6f, tb * 0.6f); renderWest(tile, 0, 0, 0, TEX_OUTSIDE); renderEast(tile, 0, 0, 0, TEX_OUTSIDE);

		// 2. Basin 4 walls
		// North wall
		tile->setShape(0.0f, 10.0f / 16.0f, 0.0f, 1.0f, 1.0f, 2.0f / 16.0f);
		t.color(tr, tg, tb); renderFaceUp(tile, 0, 0, 0, TEX_TOP);
		t.color(tr * 0.5f, tg * 0.5f, tb * 0.5f); renderFaceDown(tile, 0, 0, 0, TEX_OUTSIDE);
		t.color(tr * 0.8f, tg * 0.8f, tb * 0.8f); renderNorth(tile, 0, 0, 0, TEX_OUTSIDE); renderSouth(tile, 0, 0, 0, TEX_INSIDE);
		t.color(tr * 0.6f, tg * 0.6f, tb * 0.6f); renderWest(tile, 0, 0, 0, TEX_OUTSIDE); renderEast(tile, 0, 0, 0, TEX_OUTSIDE);

		// South wall
		tile->setShape(0.0f, 10.0f / 16.0f, 14.0f / 16.0f, 1.0f, 1.0f, 1.0f);
		t.color(tr, tg, tb); renderFaceUp(tile, 0, 0, 0, TEX_TOP);
		t.color(tr * 0.5f, tg * 0.5f, tb * 0.5f); renderFaceDown(tile, 0, 0, 0, TEX_OUTSIDE);
		t.color(tr * 0.8f, tg * 0.8f, tb * 0.8f); renderNorth(tile, 0, 0, 0, TEX_INSIDE); renderSouth(tile, 0, 0, 0, TEX_OUTSIDE);
		t.color(tr * 0.6f, tg * 0.6f, tb * 0.6f); renderWest(tile, 0, 0, 0, TEX_OUTSIDE); renderEast(tile, 0, 0, 0, TEX_OUTSIDE);

		// West wall
		tile->setShape(0.0f, 10.0f / 16.0f, 2.0f / 16.0f, 2.0f / 16.0f, 1.0f, 14.0f / 16.0f);
		t.color(tr, tg, tb); renderFaceUp(tile, 0, 0, 0, TEX_TOP);
		t.color(tr * 0.5f, tg * 0.5f, tb * 0.5f); renderFaceDown(tile, 0, 0, 0, TEX_OUTSIDE);
		t.color(tr * 0.6f, tg * 0.6f, tb * 0.6f); renderWest(tile, 0, 0, 0, TEX_OUTSIDE); renderEast(tile, 0, 0, 0, TEX_INSIDE);

		// East wall
		tile->setShape(14.0f / 16.0f, 10.0f / 16.0f, 2.0f / 16.0f, 1.0f, 1.0f, 14.0f / 16.0f);
		t.color(tr, tg, tb); renderFaceUp(tile, 0, 0, 0, TEX_TOP);
		t.color(tr * 0.5f, tg * 0.5f, tb * 0.5f); renderFaceDown(tile, 0, 0, 0, TEX_OUTSIDE);
		t.color(tr * 0.6f, tg * 0.6f, tb * 0.6f); renderWest(tile, 0, 0, 0, TEX_INSIDE); renderEast(tile, 0, 0, 0, TEX_OUTSIDE);

		// 3. Middle funnel box (4..12, 4..10, 4..12)
		tile->setShape(4.0f / 16.0f, 4.0f / 16.0f, 4.0f / 16.0f, 12.0f / 16.0f, 10.0f / 16.0f, 12.0f / 16.0f);
		t.color(tr, tg, tb); renderFaceUp(tile, 0, 0, 0, TEX_OUTSIDE);
		t.color(tr * 0.5f, tg * 0.5f, tb * 0.5f); renderFaceDown(tile, 0, 0, 0, TEX_OUTSIDE);
		t.color(tr * 0.8f, tg * 0.8f, tb * 0.8f); renderNorth(tile, 0, 0, 0, TEX_OUTSIDE); renderSouth(tile, 0, 0, 0, TEX_OUTSIDE);
		t.color(tr * 0.6f, tg * 0.6f, tb * 0.6f); renderWest(tile, 0, 0, 0, TEX_OUTSIDE); renderEast(tile, 0, 0, 0, TEX_OUTSIDE);

		// 4. Bottom spout (6..10, 0..4, 6..10)
		tile->setShape(6.0f / 16.0f, 0.0f, 6.0f / 16.0f, 10.0f / 16.0f, 4.0f / 16.0f, 10.0f / 16.0f);
		t.color(tr, tg, tb); renderFaceUp(tile, 0, 0, 0, TEX_OUTSIDE);
		t.color(tr * 0.5f, tg * 0.5f, tb * 0.5f); renderFaceDown(tile, 0, 0, 0, TEX_OUTSIDE);
		t.color(tr * 0.8f, tg * 0.8f, tb * 0.8f); renderNorth(tile, 0, 0, 0, TEX_OUTSIDE); renderSouth(tile, 0, 0, 0, TEX_OUTSIDE);
		t.color(tr * 0.6f, tg * 0.6f, tb * 0.6f); renderWest(tile, 0, 0, 0, TEX_OUTSIDE); renderEast(tile, 0, 0, 0, TEX_OUTSIDE);

		t.draw();
		t.addOffset(0.5f, 0.5f, 0.5f);
		tile->setShape(0, 0, 0, 1, 1, 1);
	}

}

bool TileRenderer::canRender( int renderShape )
{
	if (renderShape == Tile::SHAPE_BLOCK) return true;
	if (renderShape == Tile::SHAPE_CACTUS) return true;
	if (renderShape == Tile::SHAPE_STAIRS) return true;
	if (renderShape == Tile::SHAPE_FENCE) return true;
	if (renderShape == Tile::SHAPE_FENCE_GATE) return true;
	if (renderShape == Tile::SHAPE_LANTERN) return true;
	if (renderShape == Tile::SHAPE_CAMPFIRE) return true;
	if (renderShape == Tile::SHAPE_GRINDSTONE) return true;
	if (renderShape == Tile::SHAPE_LECTERN) return true;
	if (renderShape == Tile::SHAPE_COMPOSTER) return true;
	if (renderShape == Tile::SHAPE_STONECUTTER) return true;
	if (renderShape == Tile::SHAPE_CHAIN) return true;
	if (renderShape == Tile::SHAPE_SCAFFOLDING) return true;
	if (renderShape == Tile::SHAPE_CANDLE) return true;
	if (renderShape == Tile::SHAPE_CAULDRON) return true;
	if (renderShape == Tile::SHAPE_ANVIL) return true;
	if (renderShape == Tile::SHAPE_HOPPER) return true;

	return false;
}

void TileRenderer::renderGuiTile( Tile* tile, int data )
{
	Tesselator& t = Tesselator::instance;

	int shape = tile->getRenderShape();

	if (shape == Tile::SHAPE_BLOCK) {
		tile->updateDefaultShape();

		t.begin();
		t.addOffset(-0.5f, -0.5f, -0.5f);

		bool isLeaf = (tile == ((Tile*)Tile::leaves)
			|| (Tile::spruceLeaves && tile == (Tile*)Tile::spruceLeaves)
			|| (Tile::birchLeaves && tile == (Tile*)Tile::birchLeaves)
			|| (Tile::jungleLeaves && tile == (Tile*)Tile::jungleLeaves)
			|| (Tile::acaciaLeaves && tile == (Tile*)Tile::acaciaLeaves)
			|| (Tile::darkOakLeaves && tile == (Tile*)Tile::darkOakLeaves));

		int lr = 0x48, lg = 0xb5, lb = 0x18;
		if (isLeaf) {
			int leafColor = 0x48b518;
			if (tile == (Tile*)Tile::spruceLeaves || (tile == (Tile*)Tile::leaves && data == LeafTile::EVERGREEN_LEAF)) {
				leafColor = FoliageColor::getEvergreenColor();
			} else if (tile == (Tile*)Tile::birchLeaves || (tile == (Tile*)Tile::leaves && data == LeafTile::BIRCH_LEAF)) {
				leafColor = FoliageColor::getBirchColor();
			} else if (tile == (Tile*)Tile::jungleLeaves || (tile == (Tile*)Tile::leaves && data == LeafTile::JUNGLE_LEAF)) {
				leafColor = 0x30bb0b;
			} else if (tile == (Tile*)Tile::acaciaLeaves || (tile == (Tile*)Tile::leaves && data == LeafTile::ACACIA_LEAF)) {
				leafColor = 0xaea42a;
			} else if (tile == (Tile*)Tile::darkOakLeaves || (tile == (Tile*)Tile::leaves && data == LeafTile::DARK_OAK_LEAF)) {
				leafColor = 0x3b5919;
			}
			lr = (leafColor >> 16) & 0xff;
			lg = (leafColor >> 8) & 0xff;
			lb = leafColor & 0xff;
		}

		// Up face
		if (tile == Tile::grass) {
			t.color(0x79, 0xc0, 0x5a); // Grass green tint for top face
		} else if (isLeaf) {
			t.color(lr, lg, lb);
		} else {
			t.color(0xff, 0xff, 0xff);
		}
		{
			int texUp = tile->getTexture(1, data);
			bool isAltUp = (texUp & Tile::TEXTURE_ALT_FLAG) != 0;
			if (atlasFilter == -1 || (atlasFilter == 0 && !isAltUp) || (atlasFilter == 1 && isAltUp))
				renderFaceUp(tile, 0, 0, 0, texUp & ~Tile::TEXTURE_ALT_FLAG);
		}

		// Down face
		t.color(0xff, 0xff, 0xff);
		if (isLeaf) {
			t.color(lr, lg, lb);
		}
		{
			int texDown = tile->getTexture(0, data);
			bool isAltDown = (texDown & Tile::TEXTURE_ALT_FLAG) != 0;
			if (atlasFilter == -1 || (atlasFilter == 0 && !isAltDown) || (atlasFilter == 1 && isAltDown))
				renderFaceDown(tile, 0, 0, 0, texDown & ~Tile::TEXTURE_ALT_FLAG);
		}

		// North / South (Front-Left face: Medium-light)
		if (isLeaf) {
			t.color((lr * 200) / 255, (lg * 200) / 255, (lb * 200) / 255);
		} else {
			t.color(0xd0, 0xd0, 0xd0);
		}
		{
			int texNorth = tile->getTexture(2, data);
			bool isAltNorth = (texNorth & Tile::TEXTURE_ALT_FLAG) != 0;
			if (atlasFilter == -1 || (atlasFilter == 0 && !isAltNorth) || (atlasFilter == 1 && isAltNorth))
				renderNorth(tile, 0, 0, 0, texNorth & ~Tile::TEXTURE_ALT_FLAG);

			int texSouth = tile->getTexture(3, data);
			bool isAltSouth = (texSouth & Tile::TEXTURE_ALT_FLAG) != 0;
			if (atlasFilter == -1 || (atlasFilter == 0 && !isAltSouth) || (atlasFilter == 1 && isAltSouth))
				renderSouth(tile, 0, 0, 0, texSouth & ~Tile::TEXTURE_ALT_FLAG);
		}

		// East / West (Front-Right face: Shadow)
		if (isLeaf) {
			t.color((lr * 150) / 255, (lg * 150) / 255, (lb * 150) / 255);
		} else {
			t.color(0x80, 0x80, 0x80);
		}
		{
			int texEast = tile->getTexture(5, data);
			bool isAltEast = (texEast & Tile::TEXTURE_ALT_FLAG) != 0;
			if (atlasFilter == -1 || (atlasFilter == 0 && !isAltEast) || (atlasFilter == 1 && isAltEast))
				renderEast(tile, 0, 0, 0, texEast & ~Tile::TEXTURE_ALT_FLAG);

			int texWest = tile->getTexture(4, data);
			bool isAltWest = (texWest & Tile::TEXTURE_ALT_FLAG) != 0;
			if (atlasFilter == -1 || (atlasFilter == 0 && !isAltWest) || (atlasFilter == 1 && isAltWest))
				renderWest(tile, 0, 0, 0, texWest & ~Tile::TEXTURE_ALT_FLAG);
		}

		bool isJungleGui = (tile == (Tile*)Tile::jungleLeaves || (tile == (Tile*)Tile::leaves && data == LeafTile::JUNGLE_LEAF));
		if (isJungleGui && (atlasFilter == -1 || atlasFilter == 1)) {
			t.color(0xff, 0xff, 0xff);
			renderFaceUp(tile, 0, 0, 0, 55);
			renderFaceDown(tile, 0, 0, 0, 55);
			t.color(0xd0, 0xd0, 0xd0);
			renderNorth(tile, 0, 0, -0.0005f, 55);
			renderSouth(tile, 0, 0, 0.0005f, 55);
			t.color(0x80, 0x80, 0x80);
			renderEast(tile, 0.0005f, 0, 0, 55);
			renderWest(tile, -0.0005f, 0, 0, 55);
		}

		t.draw();
		t.addOffset(0.5f, 0.5f, 0.5f);

	} else if (shape == Tile::SHAPE_CROSS_TEXTURE) {
		// Respect atlas filter for cross-texture tiles too
		int tex = tile->getTexture(0, data);
		bool isAlt = (tex & Tile::TEXTURE_ALT_FLAG) != 0;
		if (atlasFilter == -1 || (atlasFilter == 0 && !isAlt) || (atlasFilter == 1 && isAlt)) {
			t.begin();
			//t.normal(0, -1, 0);
			tesselateCrossTexture(tile, data, -0.5f, -0.5f, -0.5f);
			//t.end();
			t.draw();
		}
	} else if (shape == Tile::SHAPE_CACTUS) {
		tile->updateDefaultShape();
		t.begin();
		t.offset(-0.5f, -0.5f, -0.5f);
		float s = 1 / 16.0f;
		t.color(0xff, 0xff, 0xff);
		renderFaceDown(tile, 0, 0, 0, tile->getTexture(0));
		renderFaceUp(tile, 0, 0, 0, tile->getTexture(1));

		t.color(0xd0, 0xd0, 0xd0);
		t.addOffset(0, 0, s);
		renderNorth(tile, 0, 0, 0, tile->getTexture(2));
		t.addOffset(0, 0, -s-s);
		renderSouth(tile, 0, 0, 0, tile->getTexture(3));

		t.color(0x80, 0x80, 0x80);
		t.addOffset(s, 0, s);
		renderWest(tile, 0, 0, 0, tile->getTexture(4));
		t.addOffset(-s-s, 0, 0);
		renderEast(tile, 0, 0, 0, tile->getTexture(5));

		t.draw();
		t.addOffset(s+0.5f, 0.5f, 0.5f);
	} else if (shape == Tile::SHAPE_STAIRS) {
		t.offset(-0.5f, -0.5f, -0.5f);
		t.begin();
		for (int i = 0; i < 2; i++) {
			if (i == 0) tile->setShape(0, 0, 0, 1, 0.5f, 1);
			if (i == 1) tile->setShape(0, 0.5f, 0.5f, 1, 1, 1);

			t.color(0xff, 0xff, 0xff);
			renderFaceDown(tile, 0, 0, 0, tile->getTexture(0));
			renderFaceUp(tile, 0, 0, 0, tile->getTexture(1));

			t.color(0xd0, 0xd0, 0xd0);
			renderNorth(tile, 0, 0, 0, tile->getTexture(2));
			renderSouth(tile, 0, 0, 0, tile->getTexture(3));

			t.color(0x80, 0x80, 0x80);
			renderWest(tile, 0, 0, 0, tile->getTexture(4));
			renderEast(tile, 0, 0, 0, tile->getTexture(5));
		}
		t.draw();
		tile->setShape(0, 0, 0, 1, 1, 1);
		t.offset(0, 0, 0);
	}
	else if (shape == Tile::SHAPE_FENCE) {
		t.addOffset(-0.5f, -0.5f, -0.5f);
		t.begin();
		for (int i = 0; i < 4; i++) {
			float w = 2 / 16.0f;
			if (i == 0) tile->setShape(0.5f - w, 0, 0, 0.5f + w, 1, w * 2);
			if (i == 1) tile->setShape(0.5f - w, 0, 1 - w * 2, 0.5f + w, 1, 1);
			w = 1 / 16.0f;
			if (i == 2) tile->setShape(0.5f - w, 1 - w * 3, -w * 2, 0.5f + w, 1 - w, 1 + w * 2);
			if (i == 3) tile->setShape(0.5f - w, 0.5f - w * 3, -w * 2, 0.5f + w, 0.5f - w, 1 + w * 2);

			t.color(0xff, 0xff, 0xff);

			renderFaceDown(tile, 0, 0, 0, tile->getTexture(0));
			renderFaceUp(tile, 0, 0, 0, tile->getTexture(1));

			t.color(0xd0, 0xd0, 0xd0);
			renderNorth(tile, 0, 0, 0, tile->getTexture(2));
			renderSouth(tile, 0, 0, 0, tile->getTexture(3));

			t.color(0x80, 0x80, 0x80);
			renderWest(tile, 0, 0, 0, tile->getTexture(4));
			renderEast(tile, 0, 0, 0, tile->getTexture(5));
		}
		t.draw();
		t.addOffset(0.5f, 0.5f, 0.5f);
		tile->setShape(0, 0, 0, 1, 1, 1);
	}
	else if (shape == Tile::SHAPE_FENCE_GATE) {
		t.addOffset(-0.5f, -0.5f, -0.5f);
		t.begin();
		for (int i = 0; i < 3; i++) {
			float w = 1 / 16.0f;
			if (i == 0) tile->setShape(0.5f - w, .3f, 0, 0.5f + w, 1, w * 2);
			if (i == 1) tile->setShape(0.5f - w, .3f, 1 - w * 2, 0.5f + w, 1, 1);
			w = 1 / 16.0f;
			if (i == 2) tile->setShape(0.5f - w, .5f, 0, 0.5f + w, 1 - w, 1);

			t.color(0xff, 0xff, 0xff);
			renderFaceUp(tile, 0, 0, 0, tile->getTexture(0));
			renderFaceDown(tile, 0, 0, 0, tile->getTexture(1));

			t.color(0xd0, 0xd0, 0xd0);
			renderNorth(tile, 0, 0, 0, tile->getTexture(2));
			renderSouth(tile, 0, 0, 0, tile->getTexture(3));

			t.color(0x80, 0x80, 0x80);
			renderWest(tile, 0, 0, 0, tile->getTexture(4));
			renderEast(tile, 0, 0, 0, tile->getTexture(5));
		}
		t.draw();
		tile->setShape(0, 0, 0, 1, 1, 1);
		t.addOffset(0.5f, 0.5f, 0.5f);
	} else if (shape == Tile::SHAPE_LANTERN) {
		int tex = tile->getTexture(0, data);
		bool isAlt = (tex & Tile::TEXTURE_ALT_FLAG) != 0;
		if (atlasFilter == -1 || (atlasFilter == 0 && !isAlt) || (atlasFilter == 1 && isAlt)) {
			t.addOffset(-0.5f, -0.5f, -0.5f);
			t.begin();

			int cleanTex = tex & ~Tile::TEXTURE_ALT_FLAG;
			int xt = (cleanTex & 0xf) << 4;
			int yt = cleanTex & 0xf0;
			const float atlasSize = 256.0f;

			float uSide0 = (xt + 0.0f) / atlasSize;
			float uSide1 = (xt + 6.0f) / atlasSize;
			float vSide0 = (yt + 2.0f) / atlasSize;
			float vSide1 = (yt + 9.0f) / atlasSize;

			float uLid0 = (xt + 0.0f) / atlasSize;
			float uLid1 = (xt + 6.0f) / atlasSize;
			float vLid0 = (yt + 9.0f) / atlasSize;
			float vLid1 = (yt + 15.0f) / atlasSize;

			float uCap0 = (xt + 1.0f) / atlasSize;
			float uCap1 = (xt + 5.0f) / atlasSize;
			float vCap0 = (yt + 0.0f) / atlasSize;
			float vCap1 = (yt + 2.0f) / atlasSize;

			float uCapTop0 = (xt + 1.0f) / atlasSize;
			float uCapTop1 = (xt + 5.0f) / atlasSize;
			float vCapTop0 = (yt + 9.0f) / atlasSize;
			float vCapTop1 = (yt + 13.0f) / atlasSize;

			float uRing0 = (xt + 11.0f) / atlasSize;
			float uRing1 = (xt + 14.0f) / atlasSize;
			float vRing0 = (yt + 1.0f) / atlasSize;
			float vRing1 = (yt + 5.0f) / atlasSize;

			float bx0 = 5.0f / 16.0f, bx1 = 11.0f / 16.0f;
			float bz0 = 5.0f / 16.0f, bz1 = 11.0f / 16.0f;
			float by0 = 0.0f / 16.0f, by1 = 7.0f / 16.0f;

			// Up
			t.color(0xff, 0xff, 0xff);
			t.vertexUV(bx0, by1, bz0, uLid0, vLid0);
			t.vertexUV(bx0, by1, bz1, uLid0, vLid1);
			t.vertexUV(bx1, by1, bz1, uLid1, vLid1);
			t.vertexUV(bx1, by1, bz0, uLid1, vLid0);

			// Down
			t.color(0x80, 0x80, 0x80);
			t.vertexUV(bx0, by0, bz1, uLid0, vLid1);
			t.vertexUV(bx0, by0, bz0, uLid0, vLid0);
			t.vertexUV(bx1, by0, bz0, uLid1, vLid0);
			t.vertexUV(bx1, by0, bz1, uLid1, vLid1);

			// North & South
			t.color(0xd0, 0xd0, 0xd0);
			t.vertexUV(bx0, by1, bz0, uSide1, vSide0);
			t.vertexUV(bx1, by1, bz0, uSide0, vSide0);
			t.vertexUV(bx1, by0, bz0, uSide0, vSide1);
			t.vertexUV(bx0, by0, bz0, uSide1, vSide1);
			t.vertexUV(bx1, by1, bz0, uSide0, vSide0);
			t.vertexUV(bx0, by1, bz0, uSide1, vSide0);
			t.vertexUV(bx0, by0, bz0, uSide1, vSide1);
			t.vertexUV(bx1, by0, bz0, uSide0, vSide1);

			t.vertexUV(bx0, by1, bz1, uSide0, vSide0);
			t.vertexUV(bx0, by0, bz1, uSide0, vSide1);
			t.vertexUV(bx1, by0, bz1, uSide1, vSide1);
			t.vertexUV(bx1, by1, bz1, uSide1, vSide0);
			t.vertexUV(bx1, by1, bz1, uSide1, vSide0);
			t.vertexUV(bx1, by0, bz1, uSide1, vSide1);
			t.vertexUV(bx0, by0, bz1, uSide0, vSide1);
			t.vertexUV(bx0, by1, bz1, uSide0, vSide0);

			// West & East
			t.color(0x90, 0x90, 0x90);
			t.vertexUV(bx0, by1, bz1, uSide1, vSide0);
			t.vertexUV(bx0, by1, bz0, uSide0, vSide0);
			t.vertexUV(bx0, by0, bz0, uSide0, vSide1);
			t.vertexUV(bx0, by0, bz1, uSide1, vSide1);
			t.vertexUV(bx0, by1, bz0, uSide0, vSide0);
			t.vertexUV(bx0, by1, bz1, uSide1, vSide0);
			t.vertexUV(bx0, by0, bz1, uSide1, vSide1);
			t.vertexUV(bx0, by0, bz0, uSide0, vSide1);

			t.vertexUV(bx1, by0, bz1, uSide0, vSide1);
			t.vertexUV(bx1, by0, bz0, uSide1, vSide1);
			t.vertexUV(bx1, by1, bz0, uSide1, vSide0);
			t.vertexUV(bx1, by1, bz1, uSide0, vSide0);
			t.vertexUV(bx1, by0, bz0, uSide1, vSide1);
			t.vertexUV(bx1, by0, bz1, uSide0, vSide1);
			t.vertexUV(bx1, by1, bz1, uSide0, vSide0);
			t.vertexUV(bx1, by1, bz0, uSide1, vSide0);

			// Cap
			float cx0 = 6.0f / 16.0f, cx1 = 10.0f / 16.0f;
			float cz0 = 6.0f / 16.0f, cz1 = 10.0f / 16.0f;
			float cy0 = 7.0f / 16.0f, cy1 = 9.0f / 16.0f;

			t.color(0xff, 0xff, 0xff);
			t.vertexUV(cx0, cy1, cz0, uCapTop0, vCapTop0);
			t.vertexUV(cx0, cy1, cz1, uCapTop0, vCapTop1);
			t.vertexUV(cx1, cy1, cz1, uCapTop1, vCapTop1);
			t.vertexUV(cx1, cy1, cz0, uCapTop1, vCapTop0);

			t.color(0xd0, 0xd0, 0xd0);
			t.vertexUV(cx0, cy1, cz0, uCap1, vCap0);
			t.vertexUV(cx1, cy1, cz0, uCap0, vCap0);
			t.vertexUV(cx1, cy0, cz0, uCap0, vCap1);
			t.vertexUV(cx0, cy0, cz0, uCap1, vCap1);

			t.vertexUV(cx0, cy1, cz1, uCap0, vCap0);
			t.vertexUV(cx0, cy0, cz1, uCap0, vCap1);
			t.vertexUV(cx1, cy0, cz1, uCap1, vCap1);
			t.vertexUV(cx1, cy1, cz1, uCap1, vCap0);

			t.color(0x90, 0x90, 0x90);
			t.vertexUV(cx0, cy1, cz1, uCap1, vCap0);
			t.vertexUV(cx0, cy1, cz0, uCap0, vCap0);
			t.vertexUV(cx0, cy0, cz0, uCap0, vCap1);
			t.vertexUV(cx0, cy0, cz1, uCap1, vCap1);

			t.vertexUV(cx1, cy0, cz1, uCap0, vCap1);
			t.vertexUV(cx1, cy0, cz0, uCap1, vCap1);
			t.vertexUV(cx1, cy1, cz0, uCap1, vCap0);
			t.vertexUV(cx1, cy1, cz1, uCap0, vCap0);

			// Ring
			t.color(0xd0, 0xd0, 0xd0);
			float rx0 = 6.5f / 16.0f, rx1 = 9.5f / 16.0f;
			float rz0 = 6.5f / 16.0f, rz1 = 9.5f / 16.0f;
			float ry0 = 9.0f / 16.0f, ry1 = 13.0f / 16.0f;
			float rxMid = 8.0f / 16.0f, rzMid = 8.0f / 16.0f;

			t.vertexUV(rx0, ry1, rzMid, uRing0, vRing0);
			t.vertexUV(rx1, ry1, rzMid, uRing1, vRing0);
			t.vertexUV(rx1, ry0, rzMid, uRing1, vRing1);
			t.vertexUV(rx0, ry0, rzMid, uRing0, vRing1);

			t.vertexUV(rx1, ry1, rzMid, uRing1, vRing0);
			t.vertexUV(rx0, ry1, rzMid, uRing0, vRing0);
			t.vertexUV(rx0, ry0, rzMid, uRing0, vRing1);
			t.vertexUV(rx1, ry0, rzMid, uRing1, vRing1);

			t.vertexUV(rxMid, ry1, rz0, uRing0, vRing0);
			t.vertexUV(rxMid, ry1, rz1, uRing1, vRing0);
			t.vertexUV(rxMid, ry0, rz1, uRing1, vRing1);
			t.vertexUV(rxMid, ry0, rz0, uRing0, vRing1);

			t.vertexUV(rxMid, ry1, rz1, uRing1, vRing0);
			t.vertexUV(rxMid, ry1, rz0, uRing0, vRing0);
			t.vertexUV(rxMid, ry0, rz0, uRing0, vRing1);
			t.vertexUV(rxMid, ry0, rz1, uRing1, vRing1);

			t.draw();
			t.addOffset(0.5f, 0.5f, 0.5f);
		}
	} else if (shape == Tile::SHAPE_CAMPFIRE) {
		int tex = tile->getTexture(0, data);
		bool isAlt = (tex & Tile::TEXTURE_ALT_FLAG) != 0;
		if (atlasFilter == -1 || (atlasFilter == 0 && !isAlt) || (atlasFilter == 1 && isAlt)) {
			t.addOffset(-0.5f, -0.5f, -0.5f);
			t.begin();

			const int TEX_LOG = 118;
			const int TEX_FIRE = 109;
			const int TEX_LIT = 110;

			// 1. Bottom Left Log (West Log: x: 1..5, y: 0..4, z: 0..16)
			float lx0 = 1.0f / 16.0f, lx1 = 5.0f / 16.0f;
			float ly0 = 0.0f, ly1 = 4.0f / 16.0f;
			float lz0 = 0.0f, lz1 = 1.0f;

			// Up (rot 90)
			t.color(0xff, 0xff, 0xff);
			t.vertexUV(lx0, ly1, lz0, getAtlasU(TEX_LOG, 0), getAtlasV(TEX_LOG, 4));
			t.vertexUV(lx0, ly1, lz1, getAtlasU(TEX_LOG, 16), getAtlasV(TEX_LOG, 4));
			t.vertexUV(lx1, ly1, lz1, getAtlasU(TEX_LOG, 16), getAtlasV(TEX_LOG, 0));
			t.vertexUV(lx1, ly1, lz0, getAtlasU(TEX_LOG, 0), getAtlasV(TEX_LOG, 0));

			// Down (rot 90)
			t.color(0x80, 0x80, 0x80);
			t.vertexUV(lx0, ly0, lz1, getAtlasU(TEX_LOG, 16), getAtlasV(TEX_LOG, 0));
			t.vertexUV(lx0, ly0, lz0, getAtlasU(TEX_LOG, 0), getAtlasV(TEX_LOG, 0));
			t.vertexUV(lx1, ly0, lz0, getAtlasU(TEX_LOG, 0), getAtlasV(TEX_LOG, 4));
			t.vertexUV(lx1, ly0, lz1, getAtlasU(TEX_LOG, 16), getAtlasV(TEX_LOG, 4));

			// North (end cut)
			t.color(0xd0, 0xd0, 0xd0);
			t.vertexUV(lx0, ly1, lz0, getAtlasU(TEX_LOG, 0), getAtlasV(TEX_LOG, 4));
			t.vertexUV(lx1, ly1, lz0, getAtlasU(TEX_LOG, 4), getAtlasV(TEX_LOG, 4));
			t.vertexUV(lx1, ly0, lz0, getAtlasU(TEX_LOG, 4), getAtlasV(TEX_LOG, 8));
			t.vertexUV(lx0, ly0, lz0, getAtlasU(TEX_LOG, 0), getAtlasV(TEX_LOG, 8));

			// South (end cut)
			t.vertexUV(lx0, ly1, lz1, getAtlasU(TEX_LOG, 4), getAtlasV(TEX_LOG, 4));
			t.vertexUV(lx0, ly0, lz1, getAtlasU(TEX_LOG, 4), getAtlasV(TEX_LOG, 8));
			t.vertexUV(lx1, ly0, lz1, getAtlasU(TEX_LOG, 0), getAtlasV(TEX_LOG, 8));
			t.vertexUV(lx1, ly1, lz1, getAtlasU(TEX_LOG, 0), getAtlasV(TEX_LOG, 4));

			// West (bark)
			t.color(0x90, 0x90, 0x90);
			t.vertexUV(lx0, ly1, lz1, getAtlasU(TEX_LOG, 0), getAtlasV(TEX_LOG, 0));
			t.vertexUV(lx0, ly1, lz0, getAtlasU(TEX_LOG, 16), getAtlasV(TEX_LOG, 0));
			t.vertexUV(lx0, ly0, lz0, getAtlasU(TEX_LOG, 16), getAtlasV(TEX_LOG, 4));
			t.vertexUV(lx0, ly0, lz1, getAtlasU(TEX_LOG, 0), getAtlasV(TEX_LOG, 4));

			// East (lit log inner)
			t.vertexUV(lx1, ly1, lz0, getAtlasU(TEX_LIT, 16), getAtlasV(TEX_LIT, 1));
			t.vertexUV(lx1, ly1, lz1, getAtlasU(TEX_LIT, 0), getAtlasV(TEX_LIT, 1));
			t.vertexUV(lx1, ly0, lz1, getAtlasU(TEX_LIT, 0), getAtlasV(TEX_LIT, 5));
			t.vertexUV(lx1, ly0, lz0, getAtlasU(TEX_LIT, 16), getAtlasV(TEX_LIT, 5));

			// 2. Bottom Right Log (East Log: x: 11..15, y: 0..4, z: 0..16)
			float rx0 = 11.0f / 16.0f, rx1 = 15.0f / 16.0f;
			float ry0 = 0.0f, ry1 = 4.0f / 16.0f;
			float rz0 = 0.0f, rz1 = 1.0f;

			// Up (rot 90)
			t.color(0xff, 0xff, 0xff);
			t.vertexUV(rx0, ry1, rz0, getAtlasU(TEX_LOG, 0), getAtlasV(TEX_LOG, 4));
			t.vertexUV(rx0, ry1, rz1, getAtlasU(TEX_LOG, 16), getAtlasV(TEX_LOG, 4));
			t.vertexUV(rx1, ry1, rz1, getAtlasU(TEX_LOG, 16), getAtlasV(TEX_LOG, 0));
			t.vertexUV(rx1, ry1, rz0, getAtlasU(TEX_LOG, 0), getAtlasV(TEX_LOG, 0));

			// Down (rot 90)
			t.color(0x80, 0x80, 0x80);
			t.vertexUV(rx0, ry0, rz1, getAtlasU(TEX_LOG, 16), getAtlasV(TEX_LOG, 0));
			t.vertexUV(rx0, ry0, rz0, getAtlasU(TEX_LOG, 0), getAtlasV(TEX_LOG, 0));
			t.vertexUV(rx1, ry0, rz0, getAtlasU(TEX_LOG, 0), getAtlasV(TEX_LOG, 4));
			t.vertexUV(rx1, ry0, rz1, getAtlasU(TEX_LOG, 16), getAtlasV(TEX_LOG, 4));

			// North (end cut)
			t.color(0xd0, 0xd0, 0xd0);
			t.vertexUV(rx0, ry1, rz0, getAtlasU(TEX_LOG, 0), getAtlasV(TEX_LOG, 4));
			t.vertexUV(rx1, ry1, rz0, getAtlasU(TEX_LOG, 4), getAtlasV(TEX_LOG, 4));
			t.vertexUV(rx1, ry0, rz0, getAtlasU(TEX_LOG, 4), getAtlasV(TEX_LOG, 8));
			t.vertexUV(rx0, ry0, rz0, getAtlasU(TEX_LOG, 0), getAtlasV(TEX_LOG, 8));

			// South (end cut)
			t.vertexUV(rx0, ry1, rz1, getAtlasU(TEX_LOG, 4), getAtlasV(TEX_LOG, 4));
			t.vertexUV(rx0, ry0, rz1, getAtlasU(TEX_LOG, 4), getAtlasV(TEX_LOG, 8));
			t.vertexUV(rx1, ry0, rz1, getAtlasU(TEX_LOG, 0), getAtlasV(TEX_LOG, 8));
			t.vertexUV(rx1, ry1, rz1, getAtlasU(TEX_LOG, 0), getAtlasV(TEX_LOG, 4));

			// West (lit log inner)
			t.color(0x90, 0x90, 0x90);
			t.vertexUV(rx0, ry1, rz1, getAtlasU(TEX_LIT, 0), getAtlasV(TEX_LIT, 1));
			t.vertexUV(rx0, ry1, rz0, getAtlasU(TEX_LIT, 16), getAtlasV(TEX_LIT, 1));
			t.vertexUV(rx0, ry0, rz0, getAtlasU(TEX_LIT, 16), getAtlasV(TEX_LIT, 5));
			t.vertexUV(rx0, ry0, rz1, getAtlasU(TEX_LIT, 0), getAtlasV(TEX_LIT, 5));

			// East (bark)
			t.vertexUV(rx1, ry1, rz0, getAtlasU(TEX_LOG, 16), getAtlasV(TEX_LOG, 0));
			t.vertexUV(rx1, ry1, rz1, getAtlasU(TEX_LOG, 0), getAtlasV(TEX_LOG, 0));
			t.vertexUV(rx1, ry0, rz1, getAtlasU(TEX_LOG, 0), getAtlasV(TEX_LOG, 4));
			t.vertexUV(rx1, ry0, rz0, getAtlasU(TEX_LOG, 16), getAtlasV(TEX_LOG, 4));

			// 3. Top North Log (x: 0..16, y: 3..7, z: 1..5)
			float nx0 = 0.0f, nx1 = 1.0f;
			float ny0 = 3.0f / 16.0f, ny1 = 7.0f / 16.0f;
			float nz0 = 1.0f / 16.0f, nz1 = 5.0f / 16.0f;

			// Up (rot 180)
			t.color(0xff, 0xff, 0xff);
			t.vertexUV(nx0, ny1, nz0, getAtlasU(TEX_LOG, 16), getAtlasV(TEX_LOG, 4));
			t.vertexUV(nx0, ny1, nz1, getAtlasU(TEX_LOG, 16), getAtlasV(TEX_LOG, 0));
			t.vertexUV(nx1, ny1, nz1, getAtlasU(TEX_LOG, 0), getAtlasV(TEX_LOG, 0));
			t.vertexUV(nx1, ny1, nz0, getAtlasU(TEX_LOG, 0), getAtlasV(TEX_LOG, 4));

			// Down
			t.color(0x80, 0x80, 0x80);
			t.vertexUV(nx0, ny0, nz1, getAtlasU(TEX_LIT, 0), getAtlasV(TEX_LIT, 4));
			t.vertexUV(nx0, ny0, nz0, getAtlasU(TEX_LIT, 0), getAtlasV(TEX_LIT, 8));
			t.vertexUV(nx1, ny0, nz0, getAtlasU(TEX_LIT, 16), getAtlasV(TEX_LIT, 8));
			t.vertexUV(nx1, ny0, nz1, getAtlasU(TEX_LIT, 16), getAtlasV(TEX_LIT, 4));

			// West (end cut)
			t.color(0x90, 0x90, 0x90);
			t.vertexUV(nx0, ny1, nz1, getAtlasU(TEX_LOG, 4), getAtlasV(TEX_LOG, 4));
			t.vertexUV(nx0, ny1, nz0, getAtlasU(TEX_LOG, 0), getAtlasV(TEX_LOG, 4));
			t.vertexUV(nx0, ny0, nz0, getAtlasU(TEX_LOG, 0), getAtlasV(TEX_LOG, 8));
			t.vertexUV(nx0, ny0, nz1, getAtlasU(TEX_LOG, 4), getAtlasV(TEX_LOG, 8));

			// East (end cut)
			t.vertexUV(nx1, ny1, nz0, getAtlasU(TEX_LOG, 4), getAtlasV(TEX_LOG, 4));
			t.vertexUV(nx1, ny1, nz1, getAtlasU(TEX_LOG, 0), getAtlasV(TEX_LOG, 4));
			t.vertexUV(nx1, ny0, nz1, getAtlasU(TEX_LOG, 0), getAtlasV(TEX_LOG, 8));
			t.vertexUV(nx1, ny0, nz0, getAtlasU(TEX_LOG, 4), getAtlasV(TEX_LOG, 8));

			// North (outer lit side)
			t.color(0xd0, 0xd0, 0xd0);
			t.vertexUV(nx0, ny1, nz0, getAtlasU(TEX_LIT, 0), getAtlasV(TEX_LIT, 0));
			t.vertexUV(nx1, ny1, nz0, getAtlasU(TEX_LIT, 16), getAtlasV(TEX_LIT, 0));
			t.vertexUV(nx1, ny0, nz0, getAtlasU(TEX_LIT, 16), getAtlasV(TEX_LIT, 4));
			t.vertexUV(nx0, ny0, nz0, getAtlasU(TEX_LIT, 0), getAtlasV(TEX_LIT, 4));

			// South (inner lit side)
			t.vertexUV(nx0, ny1, nz1, getAtlasU(TEX_LIT, 0), getAtlasV(TEX_LIT, 0));
			t.vertexUV(nx0, ny0, nz1, getAtlasU(TEX_LIT, 0), getAtlasV(TEX_LIT, 4));
			t.vertexUV(nx1, ny0, nz1, getAtlasU(TEX_LIT, 16), getAtlasV(TEX_LIT, 4));
			t.vertexUV(nx1, ny1, nz1, getAtlasU(TEX_LIT, 16), getAtlasV(TEX_LIT, 0));

			// 4. Top South Log (x: 0..16, y: 3..7, z: 11..15)
			float sx0 = 0.0f, sx1 = 1.0f;
			float sy0 = 3.0f / 16.0f, sy1 = 7.0f / 16.0f;
			float sz0 = 11.0f / 16.0f, sz1 = 15.0f / 16.0f;

			// Up (rot 180)
			t.color(0xff, 0xff, 0xff);
			t.vertexUV(sx0, sy1, sz0, getAtlasU(TEX_LOG, 16), getAtlasV(TEX_LOG, 4));
			t.vertexUV(sx0, sy1, sz1, getAtlasU(TEX_LOG, 16), getAtlasV(TEX_LOG, 0));
			t.vertexUV(sx1, sy1, sz1, getAtlasU(TEX_LOG, 0), getAtlasV(TEX_LOG, 0));
			t.vertexUV(sx1, sy1, sz0, getAtlasU(TEX_LOG, 0), getAtlasV(TEX_LOG, 4));

			// Down
			t.color(0x80, 0x80, 0x80);
			t.vertexUV(sx0, sy0, sz1, getAtlasU(TEX_LIT, 0), getAtlasV(TEX_LIT, 4));
			t.vertexUV(sx0, sy0, sz0, getAtlasU(TEX_LIT, 0), getAtlasV(TEX_LIT, 8));
			t.vertexUV(sx1, sy0, sz0, getAtlasU(TEX_LIT, 16), getAtlasV(TEX_LIT, 8));
			t.vertexUV(sx1, sy0, sz1, getAtlasU(TEX_LIT, 16), getAtlasV(TEX_LIT, 4));

			// West (end cut)
			t.color(0x90, 0x90, 0x90);
			t.vertexUV(sx0, sy1, sz1, getAtlasU(TEX_LOG, 4), getAtlasV(TEX_LOG, 4));
			t.vertexUV(sx0, sy1, sz0, getAtlasU(TEX_LOG, 0), getAtlasV(TEX_LOG, 4));
			t.vertexUV(sx0, sy0, sz0, getAtlasU(TEX_LOG, 0), getAtlasV(TEX_LOG, 8));
			t.vertexUV(sx0, sy0, sz1, getAtlasU(TEX_LOG, 4), getAtlasV(TEX_LOG, 8));

			// East (end cut)
			t.vertexUV(sx1, sy1, sz0, getAtlasU(TEX_LOG, 4), getAtlasV(TEX_LOG, 4));
			t.vertexUV(sx1, sy1, sz1, getAtlasU(TEX_LOG, 0), getAtlasV(TEX_LOG, 4));
			t.vertexUV(sx1, sy0, sz1, getAtlasU(TEX_LOG, 0), getAtlasV(TEX_LOG, 8));
			t.vertexUV(sx1, sy0, sz0, getAtlasU(TEX_LOG, 4), getAtlasV(TEX_LOG, 8));

			// North (inner lit side)
			t.color(0xd0, 0xd0, 0xd0);
			t.vertexUV(sx0, sy1, sz0, getAtlasU(TEX_LIT, 16), getAtlasV(TEX_LIT, 0));
			t.vertexUV(sx1, sy1, sz0, getAtlasU(TEX_LIT, 0), getAtlasV(TEX_LIT, 0));
			t.vertexUV(sx1, sy0, sz0, getAtlasU(TEX_LIT, 0), getAtlasV(TEX_LIT, 4));
			t.vertexUV(sx0, sy0, sz0, getAtlasU(TEX_LIT, 16), getAtlasV(TEX_LIT, 4));

			// South (outer lit side)
			t.vertexUV(sx0, sy1, sz1, getAtlasU(TEX_LIT, 16), getAtlasV(TEX_LIT, 0));
			t.vertexUV(sx0, sy0, sz1, getAtlasU(TEX_LIT, 16), getAtlasV(TEX_LIT, 4));
			t.vertexUV(sx1, sy0, sz1, getAtlasU(TEX_LIT, 0), getAtlasV(TEX_LIT, 4));
			t.vertexUV(sx1, sy1, sz1, getAtlasU(TEX_LIT, 0), getAtlasV(TEX_LIT, 0));

			// 5. Coals / Ash bed (x: 5..11, y: 0..1, z: 0..16)
			float ax0 = 5.0f / 16.0f, ax1 = 11.0f / 16.0f;
			float ay0 = 0.0f, ay1 = 1.0f / 16.0f;
			float az0 = 0.0f, az1 = 1.0f;

			// Up (ash coals glowing)
			t.color(0xff, 0xff, 0xff);
			t.vertexUV(ax0, ay1, az0, getAtlasU(TEX_LIT, 0), getAtlasV(TEX_LIT, 14));
			t.vertexUV(ax0, ay1, az1, getAtlasU(TEX_LIT, 16), getAtlasV(TEX_LIT, 14));
			t.vertexUV(ax1, ay1, az1, getAtlasU(TEX_LIT, 16), getAtlasV(TEX_LIT, 8));
			t.vertexUV(ax1, ay1, az0, getAtlasU(TEX_LIT, 0), getAtlasV(TEX_LIT, 8));

			// Down
			t.color(0x80, 0x80, 0x80);
			t.vertexUV(ax0, ay0, az1, getAtlasU(TEX_LOG, 16), getAtlasV(TEX_LOG, 8));
			t.vertexUV(ax0, ay0, az0, getAtlasU(TEX_LOG, 0), getAtlasV(TEX_LOG, 8));
			t.vertexUV(ax1, ay0, az0, getAtlasU(TEX_LOG, 0), getAtlasV(TEX_LOG, 14));
			t.vertexUV(ax1, ay0, az1, getAtlasU(TEX_LOG, 16), getAtlasV(TEX_LOG, 14));

			// North ash edge
			t.color(0xd0, 0xd0, 0xd0);
			t.vertexUV(ax0, ay1, az0, getAtlasU(TEX_LOG, 0), getAtlasV(TEX_LOG, 15));
			t.vertexUV(ax1, ay1, az0, getAtlasU(TEX_LOG, 6), getAtlasV(TEX_LOG, 15));
			t.vertexUV(ax1, ay0, az0, getAtlasU(TEX_LOG, 6), getAtlasV(TEX_LOG, 16));
			t.vertexUV(ax0, ay0, az0, getAtlasU(TEX_LOG, 0), getAtlasV(TEX_LOG, 16));

			// South ash edge
			t.vertexUV(ax0, ay1, az1, getAtlasU(TEX_LOG, 16), getAtlasV(TEX_LOG, 15));
			t.vertexUV(ax0, ay0, az1, getAtlasU(TEX_LOG, 16), getAtlasV(TEX_LOG, 16));
			t.vertexUV(ax1, ay0, az1, getAtlasU(TEX_LOG, 10), getAtlasV(TEX_LOG, 16));
			t.vertexUV(ax1, ay1, az1, getAtlasU(TEX_LOG, 10), getAtlasV(TEX_LOG, 15));

			// 6. Fire Cross Planes (Full Brightness)
			t.color(0xff, 0xff, 0xff);
			float fx0 = 0.8f / 16.0f, fx1 = 15.2f / 16.0f;
			float fz0 = 0.8f / 16.0f, fz1 = 15.2f / 16.0f;
			float fy0 = 1.0f / 16.0f, fy1 = 17.0f / 16.0f;

			float fu0 = getAtlasU(TEX_FIRE, 0), fu1 = getAtlasU(TEX_FIRE, 16);
			float fv0 = getAtlasV(TEX_FIRE, 0), fv1 = getAtlasV(TEX_FIRE, 16);

			// Diagonal 1
			t.vertexUV(fx0, fy1, fz0, fu0, fv0);
			t.vertexUV(fx0, fy0, fz0, fu0, fv1);
			t.vertexUV(fx1, fy0, fz1, fu1, fv1);
			t.vertexUV(fx1, fy1, fz1, fu1, fv0);

			t.vertexUV(fx1, fy1, fz1, fu1, fv0);
			t.vertexUV(fx1, fy0, fz1, fu1, fv1);
			t.vertexUV(fx0, fy0, fz0, fu0, fv1);
			t.vertexUV(fx0, fy1, fz0, fu0, fv0);

			// Diagonal 2
			t.vertexUV(fx0, fy1, fz1, fu0, fv0);
			t.vertexUV(fx0, fy0, fz1, fu0, fv1);
			t.vertexUV(fx1, fy0, fz0, fu1, fv1);
			t.vertexUV(fx1, fy1, fz0, fu1, fv0);

			t.vertexUV(fx1, fy1, fz0, fu1, fv0);
			t.vertexUV(fx1, fy0, fz0, fu1, fv1);
			t.vertexUV(fx0, fy0, fz1, fu0, fv1);
			t.vertexUV(fx0, fy1, fz1, fu0, fv0);

			t.draw();
			t.addOffset(0.5f, 0.5f, 0.5f);
			tile->setShape(0, 0, 0, 1, 1, 1);
		}
	} else if (shape == Tile::SHAPE_GRINDSTONE) {
		int tex = tile->getTexture(0, data);
		bool isAlt = (tex & Tile::TEXTURE_ALT_FLAG) != 0;
		if (atlasFilter == -1 || (atlasFilter == 0 && !isAlt) || (atlasFilter == 1 && isAlt)) {
			t.addOffset(-0.5f, -0.5f, -0.5f);
			t.begin();

			// Left Leg (151)
			tile->setShape(2.0f / 16.0f, 0.0f, 6.0f / 16.0f, 4.0f / 16.0f, 7.0f / 16.0f, 10.0f / 16.0f);
			t.color(0xff, 0xff, 0xff); renderFaceUp(tile, 0, 0, 0, 151);
			t.color(0x80, 0x80, 0x80); renderFaceDown(tile, 0, 0, 0, 151);
			t.color(0xd0, 0xd0, 0xd0); renderNorth(tile, 0, 0, 0, 151); renderSouth(tile, 0, 0, 0, 151);
			t.color(0x90, 0x90, 0x90); renderWest(tile, 0, 0, 0, 151); renderEast(tile, 0, 0, 0, 151);

			// Right Leg (151)
			tile->setShape(12.0f / 16.0f, 0.0f, 6.0f / 16.0f, 14.0f / 16.0f, 7.0f / 16.0f, 10.0f / 16.0f);
			t.color(0xff, 0xff, 0xff); renderFaceUp(tile, 0, 0, 0, 151);
			t.color(0x80, 0x80, 0x80); renderFaceDown(tile, 0, 0, 0, 151);
			t.color(0xd0, 0xd0, 0xd0); renderNorth(tile, 0, 0, 0, 151); renderSouth(tile, 0, 0, 0, 151);
			t.color(0x90, 0x90, 0x90); renderWest(tile, 0, 0, 0, 151); renderEast(tile, 0, 0, 0, 151);

			// Left Pivot Bracket (151)
			tile->setShape(2.0f / 16.0f, 7.0f / 16.0f, 5.0f / 16.0f, 4.0f / 16.0f, 13.0f / 16.0f, 11.0f / 16.0f);
			t.color(0xff, 0xff, 0xff); renderFaceUp(tile, 0, 0, 0, 151);
			t.color(0x80, 0x80, 0x80); renderFaceDown(tile, 0, 0, 0, 151);
			t.color(0xd0, 0xd0, 0xd0); renderNorth(tile, 0, 0, 0, 151); renderSouth(tile, 0, 0, 0, 151);
			t.color(0x90, 0x90, 0x90); renderWest(tile, 0, 0, 0, 151); renderEast(tile, 0, 0, 0, 151);

			// Right Pivot Bracket (151)
			tile->setShape(12.0f / 16.0f, 7.0f / 16.0f, 5.0f / 16.0f, 14.0f / 16.0f, 13.0f / 16.0f, 11.0f / 16.0f);
			t.color(0xff, 0xff, 0xff); renderFaceUp(tile, 0, 0, 0, 151);
			t.color(0x80, 0x80, 0x80); renderFaceDown(tile, 0, 0, 0, 151);
			t.color(0xd0, 0xd0, 0xd0); renderNorth(tile, 0, 0, 0, 151); renderSouth(tile, 0, 0, 0, 151);
			t.color(0x90, 0x90, 0x90); renderWest(tile, 0, 0, 0, 151); renderEast(tile, 0, 0, 0, 151);

			// Wheel (Round 17 on Up/Down/North/South, Side 21 on West/East)
			tile->setShape(4.0f / 16.0f, 4.0f / 16.0f, 2.0f / 16.0f, 12.0f / 16.0f, 1.0f, 14.0f / 16.0f);
			t.color(0xff, 0xff, 0xff); renderFaceUp(tile, 0, 0, 0, 17);
			t.color(0x80, 0x80, 0x80); renderFaceDown(tile, 0, 0, 0, 17);
			t.color(0xd0, 0xd0, 0xd0); renderNorth(tile, 0, 0, 0, 17); renderSouth(tile, 0, 0, 0, 17);
			t.color(0x90, 0x90, 0x90); renderWest(tile, 0, 0, 0, 21); renderEast(tile, 0, 0, 0, 21);

			t.draw();
			t.addOffset(0.5f, 0.5f, 0.5f);
			tile->setShape(0, 0, 0, 1, 1, 1);
		}
	} else if (shape == Tile::SHAPE_LECTERN) {
		int tex = tile->getTexture(0, data);
		bool isAlt = (tex & Tile::TEXTURE_ALT_FLAG) != 0;
		if (atlasFilter == -1 || (atlasFilter == 0 && !isAlt) || (atlasFilter == 1 && isAlt)) {
			t.addOffset(-0.5f, -0.5f, -0.5f);
			t.begin();

			const int TEX_BASE = 162;
			const int TEX_FRONT = 22;
			const int TEX_SIDES = 106;
			const int TEX_TOP = 107;

			// 1. Base pedestal (x: 0..1, y: 0..2/16, z: 0..1)
			float bx0 = 0.0f, bx1 = 1.0f;
			float by0 = 0.0f, by1 = 2.0f / 16.0f;
			float bz0 = 0.0f, bz1 = 1.0f;

			// Base Up
			t.color(0xff, 0xff, 0xff);
			t.vertexUV(bx0, by1, bz0, getAtlasU(TEX_BASE, 0), getAtlasV(TEX_BASE, 0));
			t.vertexUV(bx0, by1, bz1, getAtlasU(TEX_BASE, 0), getAtlasV(TEX_BASE, 16));
			t.vertexUV(bx1, by1, bz1, getAtlasU(TEX_BASE, 16), getAtlasV(TEX_BASE, 16));
			t.vertexUV(bx1, by1, bz0, getAtlasU(TEX_BASE, 16), getAtlasV(TEX_BASE, 0));

			// Base Down
			t.color(0x80, 0x80, 0x80);
			t.vertexUV(bx0, by0, bz1, getAtlasU(TEX_BASE, 0), getAtlasV(TEX_BASE, 16));
			t.vertexUV(bx0, by0, bz0, getAtlasU(TEX_BASE, 0), getAtlasV(TEX_BASE, 0));
			t.vertexUV(bx1, by0, bz0, getAtlasU(TEX_BASE, 16), getAtlasV(TEX_BASE, 0));
			t.vertexUV(bx1, by0, bz1, getAtlasU(TEX_BASE, 16), getAtlasV(TEX_BASE, 16));

			// Base North ([0, 14, 16, 16])
			t.color(0xd0, 0xd0, 0xd0);
			t.vertexUV(bx0, by1, bz0, getAtlasU(TEX_BASE, 0), getAtlasV(TEX_BASE, 14));
			t.vertexUV(bx1, by1, bz0, getAtlasU(TEX_BASE, 16), getAtlasV(TEX_BASE, 14));
			t.vertexUV(bx1, by0, bz0, getAtlasU(TEX_BASE, 16), getAtlasV(TEX_BASE, 16));
			t.vertexUV(bx0, by0, bz0, getAtlasU(TEX_BASE, 0), getAtlasV(TEX_BASE, 16));

			// Base South ([0, 6, 16, 8])
			t.vertexUV(bx0, by1, bz1, getAtlasU(TEX_BASE, 0), getAtlasV(TEX_BASE, 6));
			t.vertexUV(bx0, by0, bz1, getAtlasU(TEX_BASE, 0), getAtlasV(TEX_BASE, 8));
			t.vertexUV(bx1, by0, bz1, getAtlasU(TEX_BASE, 16), getAtlasV(TEX_BASE, 8));
			t.vertexUV(bx1, by1, bz1, getAtlasU(TEX_BASE, 16), getAtlasV(TEX_BASE, 6));

			// Base West ([0, 6, 16, 8])
			t.color(0x90, 0x90, 0x90);
			t.vertexUV(bx0, by1, bz1, getAtlasU(TEX_BASE, 16), getAtlasV(TEX_BASE, 6));
			t.vertexUV(bx0, by1, bz0, getAtlasU(TEX_BASE, 0), getAtlasV(TEX_BASE, 6));
			t.vertexUV(bx0, by0, bz0, getAtlasU(TEX_BASE, 0), getAtlasV(TEX_BASE, 8));
			t.vertexUV(bx0, by0, bz1, getAtlasU(TEX_BASE, 16), getAtlasV(TEX_BASE, 8));

			// Base East ([0, 6, 16, 8])
			t.vertexUV(bx1, by1, bz0, getAtlasU(TEX_BASE, 0), getAtlasV(TEX_BASE, 6));
			t.vertexUV(bx1, by1, bz1, getAtlasU(TEX_BASE, 16), getAtlasV(TEX_BASE, 6));
			t.vertexUV(bx1, by0, bz1, getAtlasU(TEX_BASE, 16), getAtlasV(TEX_BASE, 8));
			t.vertexUV(bx1, by0, bz0, getAtlasU(TEX_BASE, 0), getAtlasV(TEX_BASE, 8));

			// 2. Central pillar (x: 4..12, y: 2..14, z: 4..12)
			float px0 = 4.0f / 16.0f, px1 = 12.0f / 16.0f;
			float py0 = 2.0f / 16.0f, py1 = 14.0f / 16.0f;
			float pz0 = 4.0f / 16.0f, pz1 = 12.0f / 16.0f;

			// Pillar North (front open face: [0, 0, 8, 12])
			t.color(0xd0, 0xd0, 0xd0);
			t.vertexUV(px0, py1, pz0, getAtlasU(TEX_FRONT, 0), getAtlasV(TEX_FRONT, 0));
			t.vertexUV(px1, py1, pz0, getAtlasU(TEX_FRONT, 8), getAtlasV(TEX_FRONT, 0));
			t.vertexUV(px1, py0, pz0, getAtlasU(TEX_FRONT, 8), getAtlasV(TEX_FRONT, 12));
			t.vertexUV(px0, py0, pz0, getAtlasU(TEX_FRONT, 0), getAtlasV(TEX_FRONT, 12));

			// Pillar South (back face: [8, 4, 16, 16])
			t.vertexUV(px0, py1, pz1, getAtlasU(TEX_FRONT, 8), getAtlasV(TEX_FRONT, 4));
			t.vertexUV(px0, py0, pz1, getAtlasU(TEX_FRONT, 8), getAtlasV(TEX_FRONT, 16));
			t.vertexUV(px1, py0, pz1, getAtlasU(TEX_FRONT, 16), getAtlasV(TEX_FRONT, 16));
			t.vertexUV(px1, py1, pz1, getAtlasU(TEX_FRONT, 16), getAtlasV(TEX_FRONT, 4));

			// Pillar West (side face: [0, 2, 8, 14])
			t.color(0x90, 0x90, 0x90);
			t.vertexUV(px0, py1, pz1, getAtlasU(TEX_SIDES, 8), getAtlasV(TEX_SIDES, 2));
			t.vertexUV(px0, py1, pz0, getAtlasU(TEX_SIDES, 0), getAtlasV(TEX_SIDES, 2));
			t.vertexUV(px0, py0, pz0, getAtlasU(TEX_SIDES, 0), getAtlasV(TEX_SIDES, 14));
			t.vertexUV(px0, py0, pz1, getAtlasU(TEX_SIDES, 8), getAtlasV(TEX_SIDES, 14));

			// Pillar East (side face: [0, 2, 8, 14])
			t.vertexUV(px1, py1, pz0, getAtlasU(TEX_SIDES, 0), getAtlasV(TEX_SIDES, 2));
			t.vertexUV(px1, py1, pz1, getAtlasU(TEX_SIDES, 8), getAtlasV(TEX_SIDES, 2));
			t.vertexUV(px1, py0, pz1, getAtlasU(TEX_SIDES, 8), getAtlasV(TEX_SIDES, 14));
			t.vertexUV(px1, py0, pz0, getAtlasU(TEX_SIDES, 0), getAtlasV(TEX_SIDES, 14));

			// 3. Top reading desk (x: 0..1, y: 12..15, z: 2..16)
			float dx0 = 0.0f, dx1 = 1.0f;
			float dy0 = 12.0f / 16.0f, dy1 = 15.0f / 16.0f;
			float dz0 = 2.0f / 16.0f, dz1 = 1.0f;

			// Top surface (the book/desk face: [0, 1, 16, 15] on TEX_TOP!)
			t.color(0xff, 0xff, 0xff);
			t.vertexUV(dx0, dy1, dz0, getAtlasU(TEX_TOP, 0), getAtlasV(TEX_TOP, 1));
			t.vertexUV(dx0, dy1, dz1, getAtlasU(TEX_TOP, 0), getAtlasV(TEX_TOP, 15));
			t.vertexUV(dx1, dy1, dz1, getAtlasU(TEX_TOP, 16), getAtlasV(TEX_TOP, 15));
			t.vertexUV(dx1, dy1, dz0, getAtlasU(TEX_TOP, 16), getAtlasV(TEX_TOP, 1));

			// Bottom surface (wood bottom: [0, 0, 16, 14] on TEX_BASE)
			t.color(0x80, 0x80, 0x80);
			t.vertexUV(dx0, dy0, dz1, getAtlasU(TEX_BASE, 0), getAtlasV(TEX_BASE, 14));
			t.vertexUV(dx0, dy0, dz0, getAtlasU(TEX_BASE, 0), getAtlasV(TEX_BASE, 0));
			t.vertexUV(dx1, dy0, dz0, getAtlasU(TEX_BASE, 16), getAtlasV(TEX_BASE, 0));
			t.vertexUV(dx1, dy0, dz1, getAtlasU(TEX_BASE, 16), getAtlasV(TEX_BASE, 14));

			// Desk North edge: [0, 0, 16, 3] on TEX_SIDES
			t.color(0xd0, 0xd0, 0xd0);
			t.vertexUV(dx0, dy1, dz0, getAtlasU(TEX_SIDES, 0), getAtlasV(TEX_SIDES, 0));
			t.vertexUV(dx1, dy1, dz0, getAtlasU(TEX_SIDES, 16), getAtlasV(TEX_SIDES, 0));
			t.vertexUV(dx1, dy0, dz0, getAtlasU(TEX_SIDES, 16), getAtlasV(TEX_SIDES, 3));
			t.vertexUV(dx0, dy0, dz0, getAtlasU(TEX_SIDES, 0), getAtlasV(TEX_SIDES, 3));

			// Desk South edge: [0, 4, 16, 7] on TEX_SIDES
			t.vertexUV(dx0, dy1, dz1, getAtlasU(TEX_SIDES, 0), getAtlasV(TEX_SIDES, 4));
			t.vertexUV(dx0, dy0, dz1, getAtlasU(TEX_SIDES, 0), getAtlasV(TEX_SIDES, 7));
			t.vertexUV(dx1, dy0, dz1, getAtlasU(TEX_SIDES, 16), getAtlasV(TEX_SIDES, 7));
			t.vertexUV(dx1, dy1, dz1, getAtlasU(TEX_SIDES, 16), getAtlasV(TEX_SIDES, 4));

			// Desk West edge: [0, 4, 14, 7] on TEX_SIDES
			t.color(0x90, 0x90, 0x90);
			t.vertexUV(dx0, dy1, dz1, getAtlasU(TEX_SIDES, 14), getAtlasV(TEX_SIDES, 4));
			t.vertexUV(dx0, dy1, dz0, getAtlasU(TEX_SIDES, 0), getAtlasV(TEX_SIDES, 4));
			t.vertexUV(dx0, dy0, dz0, getAtlasU(TEX_SIDES, 0), getAtlasV(TEX_SIDES, 7));
			t.vertexUV(dx0, dy0, dz1, getAtlasU(TEX_SIDES, 14), getAtlasV(TEX_SIDES, 7));

			// Desk East edge: [0, 4, 14, 7] on TEX_SIDES
			t.vertexUV(dx1, dy1, dz0, getAtlasU(TEX_SIDES, 0), getAtlasV(TEX_SIDES, 4));
			t.vertexUV(dx1, dy1, dz1, getAtlasU(TEX_SIDES, 14), getAtlasV(TEX_SIDES, 4));
			t.vertexUV(dx1, dy0, dz1, getAtlasU(TEX_SIDES, 14), getAtlasV(TEX_SIDES, 7));
			t.vertexUV(dx1, dy0, dz0, getAtlasU(TEX_SIDES, 0), getAtlasV(TEX_SIDES, 7));

			t.draw();
			t.addOffset(0.5f, 0.5f, 0.5f);
			tile->setShape(0, 0, 0, 1, 1, 1);
		}
	} else if (shape == Tile::SHAPE_COMPOSTER) {
		int tex = tile->getTexture(0, data);
		bool isAlt = (tex & Tile::TEXTURE_ALT_FLAG) != 0;
		if (atlasFilter == -1 || (atlasFilter == 0 && !isAlt) || (atlasFilter == 1 && isAlt)) {
			t.addOffset(-0.5f, -0.5f, -0.5f);
			t.begin();

			// Bottom
			tile->setShape(2.0f / 16.0f, 0.0f, 2.0f / 16.0f, 14.0f / 16.0f, 2.0f / 16.0f, 14.0f / 16.0f);
			t.color(0xff, 0xff, 0xff); renderFaceUp(tile, 0, 0, 0, 14);
			t.color(0x80, 0x80, 0x80); renderFaceDown(tile, 0, 0, 0, 255);
			t.color(0xd0, 0xd0, 0xd0); renderNorth(tile, 0, 0, 0, 14); renderSouth(tile, 0, 0, 0, 14);
			t.color(0x90, 0x90, 0x90); renderWest(tile, 0, 0, 0, 14); renderEast(tile, 0, 0, 0, 14);

			// North wall
			tile->setShape(0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 2.0f / 16.0f);
			t.color(0xff, 0xff, 0xff); renderFaceUp(tile, 0, 0, 0, 131);
			t.color(0x80, 0x80, 0x80); renderFaceDown(tile, 0, 0, 0, 255);
			t.color(0xd0, 0xd0, 0xd0); renderNorth(tile, 0, 0, 0, 14); renderSouth(tile, 0, 0, 0, 14);
			t.color(0x90, 0x90, 0x90); renderWest(tile, 0, 0, 0, 14); renderEast(tile, 0, 0, 0, 14);

			// South wall
			tile->setShape(0.0f, 0.0f, 14.0f / 16.0f, 1.0f, 1.0f, 1.0f);
			t.color(0xff, 0xff, 0xff); renderFaceUp(tile, 0, 0, 0, 131);
			t.color(0x80, 0x80, 0x80); renderFaceDown(tile, 0, 0, 0, 255);
			t.color(0xd0, 0xd0, 0xd0); renderNorth(tile, 0, 0, 0, 14); renderSouth(tile, 0, 0, 0, 14);
			t.color(0x90, 0x90, 0x90); renderWest(tile, 0, 0, 0, 14); renderEast(tile, 0, 0, 0, 14);

			// West wall
			tile->setShape(0.0f, 0.0f, 2.0f / 16.0f, 2.0f / 16.0f, 1.0f, 14.0f / 16.0f);
			t.color(0xff, 0xff, 0xff); renderFaceUp(tile, 0, 0, 0, 131);
			t.color(0x80, 0x80, 0x80); renderFaceDown(tile, 0, 0, 0, 255);
			t.color(0xd0, 0xd0, 0xd0); renderNorth(tile, 0, 0, 0, 14); renderSouth(tile, 0, 0, 0, 14);
			t.color(0x90, 0x90, 0x90); renderWest(tile, 0, 0, 0, 14); renderEast(tile, 0, 0, 0, 14);

			// East wall
			tile->setShape(14.0f / 16.0f, 0.0f, 2.0f / 16.0f, 1.0f, 1.0f, 14.0f / 16.0f);
			t.color(0xff, 0xff, 0xff); renderFaceUp(tile, 0, 0, 0, 131);
			t.color(0x80, 0x80, 0x80); renderFaceDown(tile, 0, 0, 0, 255);
			t.color(0xd0, 0xd0, 0xd0); renderNorth(tile, 0, 0, 0, 14); renderSouth(tile, 0, 0, 0, 14);
			t.color(0x90, 0x90, 0x90); renderWest(tile, 0, 0, 0, 14); renderEast(tile, 0, 0, 0, 14);

			t.draw();
			t.addOffset(0.5f, 0.5f, 0.5f);
			tile->setShape(0, 0, 0, 1, 1, 1);
		}
	} else if (shape == Tile::SHAPE_STONECUTTER) {
		int tex = tile->getTexture(0, data);
		bool isAlt = (tex & Tile::TEXTURE_ALT_FLAG) != 0;
		if (atlasFilter == -1 || (atlasFilter == 0 && !isAlt) || (atlasFilter == 1 && isAlt)) {
			t.addOffset(-0.5f, -0.5f, -0.5f);
			t.begin();

			// 1. Table base
			tile->setShape(0.0f, 0.0f, 0.0f, 1.0f, 9.0f / 16.0f, 1.0f);
			t.color(0xff, 0xff, 0xff); renderFaceUp(tile, 0, 0, 0, 251);
			t.color(0x80, 0x80, 0x80); renderFaceDown(tile, 0, 0, 0, 252);
			t.color(0xd0, 0xd0, 0xd0); renderNorth(tile, 0, 0, 0, 253); renderSouth(tile, 0, 0, 0, 253);
			t.color(0x90, 0x90, 0x90); renderWest(tile, 0, 0, 0, 253); renderEast(tile, 0, 0, 0, 253);

			// 2. Saw blade
			tile->setShape(1.0f / 16.0f, 9.0f / 16.0f, 7.5f / 16.0f, 15.0f / 16.0f, 1.0f, 8.5f / 16.0f);
			t.color(0xff, 0xff, 0xff); renderFaceUp(tile, 0, 0, 0, 254);
			t.color(0x80, 0x80, 0x80); renderFaceDown(tile, 0, 0, 0, 254);
			t.color(0xd0, 0xd0, 0xd0); renderNorth(tile, 0, 0, 0, 254); renderSouth(tile, 0, 0, 0, 254);
			t.color(0x90, 0x90, 0x90); renderWest(tile, 0, 0, 0, 254); renderEast(tile, 0, 0, 0, 254);

			t.draw();
			t.addOffset(0.5f, 0.5f, 0.5f);
			tile->setShape(0, 0, 0, 1, 1, 1);
		}
	} else if (shape == Tile::SHAPE_CHAIN) {
		int tex = tile->getTexture(0, data);
		bool isAlt = (tex & Tile::TEXTURE_ALT_FLAG) != 0;
		if (atlasFilter == -1 || (atlasFilter == 0 && !isAlt) || (atlasFilter == 1 && isAlt)) {
			t.addOffset(-0.5f, -0.5f, -0.5f);
			t.begin();

			tile->setShape(6.5f / 16.0f, 0.0f, 6.5f / 16.0f, 9.5f / 16.0f, 1.0f, 9.5f / 16.0f);
			t.color(0xff, 0xff, 0xff); renderFaceUp(tile, 0, 0, 0, 155);
			t.color(0x80, 0x80, 0x80); renderFaceDown(tile, 0, 0, 0, 155);
			t.color(0xd0, 0xd0, 0xd0); renderNorth(tile, 0, 0, 0, 155); renderSouth(tile, 0, 0, 0, 155);
			t.color(0x90, 0x90, 0x90); renderWest(tile, 0, 0, 0, 155); renderEast(tile, 0, 0, 0, 155);

			t.draw();
			t.addOffset(0.5f, 0.5f, 0.5f);
			tile->setShape(0, 0, 0, 1, 1, 1);
		}
	} else if (shape == Tile::SHAPE_SCAFFOLDING) {
		int tex = tile->getTexture(0, data);
		bool isAlt = (tex & Tile::TEXTURE_ALT_FLAG) != 0;
		if (atlasFilter == -1 || (atlasFilter == 0 && !isAlt) || (atlasFilter == 1 && isAlt)) {
			t.addOffset(-0.5f, -0.5f, -0.5f);
			t.begin();

			int cleanTex = tex & ~Tile::TEXTURE_ALT_FLAG;
			for (int i = 0; i < 5; i++) {
				if (i == 0) tile->setShape(0.0f, 0.0f, 0.0f, 2.0f / 16.0f, 1.0f, 2.0f / 16.0f);
				if (i == 1) tile->setShape(14.0f / 16.0f, 0.0f, 0.0f, 1.0f, 1.0f, 2.0f / 16.0f);
				if (i == 2) tile->setShape(0.0f, 0.0f, 14.0f / 16.0f, 2.0f / 16.0f, 1.0f, 1.0f);
				if (i == 3) tile->setShape(14.0f / 16.0f, 0.0f, 14.0f / 16.0f, 1.0f, 1.0f, 1.0f);
				if (i == 4) tile->setShape(0.0f, 15.0f / 16.0f, 0.0f, 1.0f, 1.0f, 1.0f);

				t.color(0xff, 0xff, 0xff); renderFaceUp(tile, 0, 0, 0, cleanTex);
				t.color(0x80, 0x80, 0x80); renderFaceDown(tile, 0, 0, 0, cleanTex);
				t.color(0xd0, 0xd0, 0xd0); renderNorth(tile, 0, 0, 0, cleanTex); renderSouth(tile, 0, 0, 0, cleanTex);
				t.color(0x90, 0x90, 0x90); renderWest(tile, 0, 0, 0, cleanTex); renderEast(tile, 0, 0, 0, cleanTex);
			}

			t.draw();
			t.addOffset(0.5f, 0.5f, 0.5f);
			tile->setShape(0, 0, 0, 1, 1, 1);
		}
	} else if (shape == Tile::SHAPE_CANDLE) {
		int tex = tile->getTexture(0, data);
		bool isAlt = (tex & Tile::TEXTURE_ALT_FLAG) != 0;
		if (atlasFilter == -1 || (atlasFilter == 0 && !isAlt) || (atlasFilter == 1 && isAlt)) {
			t.addOffset(-0.5f, -0.5f, -0.5f);
			t.begin();

			int cleanTex = tex & ~Tile::TEXTURE_ALT_FLAG;
			float cx0 = 7.0f / 16.0f, cx1 = 9.0f / 16.0f;
			float cy0 = 0.0f / 16.0f, cy1 = 6.0f / 16.0f;
			float cz0 = 7.0f / 16.0f, cz1 = 9.0f / 16.0f;

			float uTop0 = getAtlasU(cleanTex, 0.0f), uTop1 = getAtlasU(cleanTex, 2.0f);
			float vTop0 = getAtlasV(cleanTex, 6.0f), vTop1 = getAtlasV(cleanTex, 8.0f);

			float uSide0 = getAtlasU(cleanTex, 0.0f), uSide1 = getAtlasU(cleanTex, 2.0f);
			float vSide0 = getAtlasV(cleanTex, 6.0f), vSide1 = getAtlasV(cleanTex, 12.0f);

			// Top
			t.color(0xff, 0xff, 0xff);
			t.vertexUV(cx0, cy1, cz0, uTop0, vTop0);
			t.vertexUV(cx0, cy1, cz1, uTop0, vTop1);
			t.vertexUV(cx1, cy1, cz1, uTop1, vTop1);
			t.vertexUV(cx1, cy1, cz0, uTop1, vTop0);

			// Down
			t.color(0x80, 0x80, 0x80);
			t.vertexUV(cx0, cy0, cz1, uTop0, vTop1);
			t.vertexUV(cx0, cy0, cz0, uTop0, vTop0);
			t.vertexUV(cx1, cy0, cz0, uTop1, vTop0);
			t.vertexUV(cx1, cy0, cz1, uTop1, vTop1);

			// North & South
			t.color(0xd0, 0xd0, 0xd0);
			t.vertexUV(cx0, cy1, cz0, uSide1, vSide0);
			t.vertexUV(cx1, cy1, cz0, uSide0, vSide0);
			t.vertexUV(cx1, cy0, cz0, uSide0, vSide1);
			t.vertexUV(cx0, cy0, cz0, uSide1, vSide1);
			t.vertexUV(cx1, cy1, cz0, uSide0, vSide0);
			t.vertexUV(cx0, cy1, cz0, uSide1, vSide0);
			t.vertexUV(cx0, cy0, cz0, uSide1, vSide1);
			t.vertexUV(cx1, cy0, cz0, uSide0, vSide1);

			t.vertexUV(cx0, cy1, cz1, uSide0, vSide0);
			t.vertexUV(cx0, cy0, cz1, uSide0, vSide1);
			t.vertexUV(cx1, cy0, cz1, uSide1, vSide1);
			t.vertexUV(cx1, cy1, cz1, uSide1, vSide0);
			t.vertexUV(cx1, cy1, cz1, uSide1, vSide0);
			t.vertexUV(cx1, cy0, cz1, uSide1, vSide1);
			t.vertexUV(cx0, cy0, cz1, uSide0, vSide1);
			t.vertexUV(cx0, cy1, cz1, uSide0, vSide0);

			// West & East
			t.color(0x90, 0x90, 0x90);
			t.vertexUV(cx0, cy1, cz1, uSide1, vSide0);
			t.vertexUV(cx0, cy1, cz0, uSide0, vSide0);
			t.vertexUV(cx0, cy0, cz0, uSide0, vSide1);
			t.vertexUV(cx0, cy0, cz1, uSide1, vSide1);
			t.vertexUV(cx0, cy1, cz0, uSide0, vSide0);
			t.vertexUV(cx0, cy1, cz1, uSide1, vSide0);
			t.vertexUV(cx0, cy0, cz1, uSide1, vSide1);
			t.vertexUV(cx0, cy0, cz0, uSide0, vSide1);

			t.vertexUV(cx1, cy0, cz1, uSide0, vSide1);
			t.vertexUV(cx1, cy0, cz0, uSide1, vSide1);
			t.vertexUV(cx1, cy1, cz0, uSide1, vSide0);
			t.vertexUV(cx1, cy1, cz1, uSide0, vSide0);
			t.vertexUV(cx1, cy1, cz0, uSide1, vSide0);
			t.vertexUV(cx1, cy0, cz0, uSide1, vSide1);
			t.vertexUV(cx1, cy0, cz1, uSide0, vSide1);
			t.vertexUV(cx1, cy1, cz1, uSide0, vSide0);

			// Wick
			float wu0 = getAtlasU(cleanTex, 0.0f), wu1 = getAtlasU(cleanTex, 1.0f);
			float wv0 = getAtlasV(cleanTex, 4.5f), wv1 = getAtlasV(cleanTex, 6.0f);
			float wy0 = 6.0f / 16.0f, wy1 = 8.5f / 16.0f;
			float wx0 = 7.5f / 16.0f, wx1 = 8.5f / 16.0f;
			float wz0 = 7.5f / 16.0f, wz1 = 8.5f / 16.0f;
			float wMidX = 8.0f / 16.0f;
			float wMidZ = 8.0f / 16.0f;

			t.color(0xff, 0xff, 0xff);
			// Quad X (double sided)
			t.vertexUV(wx0, wy1, wMidZ, wu0, wv0);
			t.vertexUV(wx0, wy0, wMidZ, wu0, wv1);
			t.vertexUV(wx1, wy0, wMidZ, wu1, wv1);
			t.vertexUV(wx1, wy1, wMidZ, wu1, wv0);

			t.vertexUV(wx1, wy1, wMidZ, wu1, wv0);
			t.vertexUV(wx1, wy0, wMidZ, wu1, wv1);
			t.vertexUV(wx0, wy0, wMidZ, wu0, wv1);
			t.vertexUV(wx0, wy1, wMidZ, wu0, wv0);

			// Quad Z (double sided)
			t.vertexUV(wMidX, wy1, wz0, wu0, wv0);
			t.vertexUV(wMidX, wy0, wz0, wu0, wv1);
			t.vertexUV(wMidX, wy0, wz1, wu1, wv1);
			t.vertexUV(wMidX, wy1, wz1, wu1, wv0);

			t.vertexUV(wMidX, wy1, wz1, wu1, wv0);
			t.vertexUV(wMidX, wy0, wz1, wu1, wv1);
			t.vertexUV(wMidX, wy0, wz0, wu0, wv1);
			t.vertexUV(wMidX, wy1, wz0, wu0, wv0);

			t.draw();
			t.addOffset(0.5f, 0.5f, 0.5f);
			tile->setShape(0, 0, 0, 1, 1, 1);
		}
	} else if (shape == Tile::SHAPE_CAULDRON) {
		int tex = tile->getTexture(0, data);
		bool isAlt = (tex & Tile::TEXTURE_ALT_FLAG) != 0;
		if (atlasFilter == -1 || (atlasFilter == 0 && !isAlt) || (atlasFilter == 1 && isAlt)) {
			t.addOffset(-0.5f, -0.5f, -0.5f);
			t.begin();

			const int TEX_TOP = 149;
			const int TEX_INNER = 150;
			const int TEX_SIDE = 163;
			const int TEX_BOTTOM = 165;

			// 4 legs
			tile->setShape(0.0f, 0.0f, 0.0f, 2.0f / 16.0f, 3.0f / 16.0f, 2.0f / 16.0f);
			t.color(0x80, 0x80, 0x80); renderFaceDown(tile, 0, 0, 0, TEX_BOTTOM);
			t.color(0xd0, 0xd0, 0xd0); renderNorth(tile, 0, 0, 0, TEX_SIDE); renderSouth(tile, 0, 0, 0, TEX_SIDE);
			t.color(0x90, 0x90, 0x90); renderWest(tile, 0, 0, 0, TEX_SIDE); renderEast(tile, 0, 0, 0, TEX_SIDE);

			tile->setShape(14.0f / 16.0f, 0.0f, 0.0f, 1.0f, 3.0f / 16.0f, 2.0f / 16.0f);
			t.color(0x80, 0x80, 0x80); renderFaceDown(tile, 0, 0, 0, TEX_BOTTOM);
			t.color(0xd0, 0xd0, 0xd0); renderNorth(tile, 0, 0, 0, TEX_SIDE); renderSouth(tile, 0, 0, 0, TEX_SIDE);
			t.color(0x90, 0x90, 0x90); renderWest(tile, 0, 0, 0, TEX_SIDE); renderEast(tile, 0, 0, 0, TEX_SIDE);

			tile->setShape(0.0f, 0.0f, 14.0f / 16.0f, 2.0f / 16.0f, 3.0f / 16.0f, 1.0f);
			t.color(0x80, 0x80, 0x80); renderFaceDown(tile, 0, 0, 0, TEX_BOTTOM);
			t.color(0xd0, 0xd0, 0xd0); renderNorth(tile, 0, 0, 0, TEX_SIDE); renderSouth(tile, 0, 0, 0, TEX_SIDE);
			t.color(0x90, 0x90, 0x90); renderWest(tile, 0, 0, 0, TEX_SIDE); renderEast(tile, 0, 0, 0, TEX_SIDE);

			tile->setShape(14.0f / 16.0f, 0.0f, 14.0f / 16.0f, 1.0f, 3.0f / 16.0f, 1.0f);
			t.color(0x80, 0x80, 0x80); renderFaceDown(tile, 0, 0, 0, TEX_BOTTOM);
			t.color(0xd0, 0xd0, 0xd0); renderNorth(tile, 0, 0, 0, TEX_SIDE); renderSouth(tile, 0, 0, 0, TEX_SIDE);
			t.color(0x90, 0x90, 0x90); renderWest(tile, 0, 0, 0, TEX_SIDE); renderEast(tile, 0, 0, 0, TEX_SIDE);

			// Bottom basin floor
			tile->setShape(2.0f / 16.0f, 3.0f / 16.0f, 2.0f / 16.0f, 14.0f / 16.0f, 5.0f / 16.0f, 14.0f / 16.0f);
			t.color(0xff, 0xff, 0xff); renderFaceUp(tile, 0, 0, 0, TEX_INNER);
			t.color(0x80, 0x80, 0x80); renderFaceDown(tile, 0, 0, 0, TEX_BOTTOM);

			// 4 walls
			// North wall
			tile->setShape(0.0f, 3.0f / 16.0f, 0.0f, 1.0f, 1.0f, 2.0f / 16.0f);
			t.color(0xff, 0xff, 0xff); renderFaceUp(tile, 0, 0, 0, TEX_TOP);
			t.color(0x80, 0x80, 0x80); renderFaceDown(tile, 0, 0, 0, TEX_BOTTOM);
			t.color(0xd0, 0xd0, 0xd0); renderNorth(tile, 0, 0, 0, TEX_SIDE); renderSouth(tile, 0, 0, 0, TEX_INNER);
			t.color(0x90, 0x90, 0x90); renderWest(tile, 0, 0, 0, TEX_SIDE); renderEast(tile, 0, 0, 0, TEX_SIDE);

			// South wall
			tile->setShape(0.0f, 3.0f / 16.0f, 14.0f / 16.0f, 1.0f, 1.0f, 1.0f);
			t.color(0xff, 0xff, 0xff); renderFaceUp(tile, 0, 0, 0, TEX_TOP);
			t.color(0x80, 0x80, 0x80); renderFaceDown(tile, 0, 0, 0, TEX_BOTTOM);
			t.color(0xd0, 0xd0, 0xd0); renderNorth(tile, 0, 0, 0, TEX_INNER); renderSouth(tile, 0, 0, 0, TEX_SIDE);
			t.color(0x90, 0x90, 0x90); renderWest(tile, 0, 0, 0, TEX_SIDE); renderEast(tile, 0, 0, 0, TEX_SIDE);

			// West wall
			tile->setShape(0.0f, 3.0f / 16.0f, 2.0f / 16.0f, 2.0f / 16.0f, 1.0f, 14.0f / 16.0f);
			t.color(0xff, 0xff, 0xff); renderFaceUp(tile, 0, 0, 0, TEX_TOP);
			t.color(0x80, 0x80, 0x80); renderFaceDown(tile, 0, 0, 0, TEX_BOTTOM);
			t.color(0x90, 0x90, 0x90); renderWest(tile, 0, 0, 0, TEX_SIDE); renderEast(tile, 0, 0, 0, TEX_INNER);

			// East wall
			tile->setShape(14.0f / 16.0f, 3.0f / 16.0f, 2.0f / 16.0f, 1.0f, 1.0f, 14.0f / 16.0f);
			t.color(0xff, 0xff, 0xff); renderFaceUp(tile, 0, 0, 0, TEX_TOP);
			t.color(0x80, 0x80, 0x80); renderFaceDown(tile, 0, 0, 0, TEX_BOTTOM);
			t.color(0x90, 0x90, 0x90); renderWest(tile, 0, 0, 0, TEX_INNER); renderEast(tile, 0, 0, 0, TEX_SIDE);

			t.draw();
			t.addOffset(0.5f, 0.5f, 0.5f);
			tile->setShape(0, 0, 0, 1, 1, 1);
		}
	} else if (shape == Tile::SHAPE_ANVIL) {
		int tex = tile->getTexture(0, data);
		bool isAlt = (tex & Tile::TEXTURE_ALT_FLAG) != 0;
		if (atlasFilter == -1 || (atlasFilter == 0 && !isAlt) || (atlasFilter == 1 && isAlt)) {
			t.addOffset(-0.5f, -0.5f, -0.5f);
			t.begin();

			int dmg = 0;
			AnvilTile* at = dynamic_cast<AnvilTile*>(tile);
			if (at) dmg = at->damageType;
			else if (tile == Tile::chippedAnvil) dmg = 1;
			else if (tile == Tile::damagedAnvil) dmg = 2;

			int topTex = (dmg == 0 ? 170 : (dmg == 1 ? 180 : 185));
			int baseTex = 166;

			// Base: (2..14, 0..4, 2..14)
			tile->setShape(2.0f / 16.0f, 0.0f, 2.0f / 16.0f, 14.0f / 16.0f, 4.0f / 16.0f, 14.0f / 16.0f);
			t.color(0xff, 0xff, 0xff); renderFaceUp(tile, 0, 0, 0, baseTex);
			t.color(0x80, 0x80, 0x80); renderFaceDown(tile, 0, 0, 0, baseTex);
			t.color(0xd0, 0xd0, 0xd0); renderNorth(tile, 0, 0, 0, baseTex); renderSouth(tile, 0, 0, 0, baseTex);
			t.color(0x90, 0x90, 0x90); renderWest(tile, 0, 0, 0, baseTex); renderEast(tile, 0, 0, 0, baseTex);

			// Lower neck: (4..12, 4..5, 5..11)
			tile->setShape(4.0f / 16.0f, 4.0f / 16.0f, 5.0f / 16.0f, 12.0f / 16.0f, 5.0f / 16.0f, 11.0f / 16.0f);
			t.color(0xff, 0xff, 0xff); renderFaceUp(tile, 0, 0, 0, baseTex);
			t.color(0x80, 0x80, 0x80); renderFaceDown(tile, 0, 0, 0, baseTex);
			t.color(0xd0, 0xd0, 0xd0); renderNorth(tile, 0, 0, 0, baseTex); renderSouth(tile, 0, 0, 0, baseTex);
			t.color(0x90, 0x90, 0x90); renderWest(tile, 0, 0, 0, baseTex); renderEast(tile, 0, 0, 0, baseTex);

			// Stem: (6..10, 5..10, 6..10)
			tile->setShape(6.0f / 16.0f, 5.0f / 16.0f, 6.0f / 16.0f, 10.0f / 16.0f, 10.0f / 16.0f, 10.0f / 16.0f);
			t.color(0xff, 0xff, 0xff); renderFaceUp(tile, 0, 0, 0, baseTex);
			t.color(0x80, 0x80, 0x80); renderFaceDown(tile, 0, 0, 0, baseTex);
			t.color(0xd0, 0xd0, 0xd0); renderNorth(tile, 0, 0, 0, baseTex); renderSouth(tile, 0, 0, 0, baseTex);
			t.color(0x90, 0x90, 0x90); renderWest(tile, 0, 0, 0, baseTex); renderEast(tile, 0, 0, 0, baseTex);

			// Head: (3..13, 10..16, 0..16)
			tile->setShape(3.0f / 16.0f, 10.0f / 16.0f, 0.0f, 13.0f / 16.0f, 1.0f, 1.0f);
			t.color(0xff, 0xff, 0xff); renderFaceUp(tile, 0, 0, 0, topTex);
			t.color(0x80, 0x80, 0x80); renderFaceDown(tile, 0, 0, 0, baseTex);
			t.color(0xd0, 0xd0, 0xd0); renderNorth(tile, 0, 0, 0, baseTex); renderSouth(tile, 0, 0, 0, baseTex);
			t.color(0x90, 0x90, 0x90); renderWest(tile, 0, 0, 0, baseTex); renderEast(tile, 0, 0, 0, baseTex);

			t.draw();
			t.addOffset(0.5f, 0.5f, 0.5f);
			tile->setShape(0, 0, 0, 1, 1, 1);
		}
	} else if (shape == Tile::SHAPE_HOPPER) {
		int tex = tile->getTexture(0, data);
		bool isAlt = (tex & Tile::TEXTURE_ALT_FLAG) != 0;
		if (atlasFilter == -1 || (atlasFilter == 0 && !isAlt) || (atlasFilter == 1 && isAlt)) {
			t.addOffset(-0.5f, -0.5f, -0.5f);
			t.begin();

			const int TEX_TOP = 195;
			const int TEX_INSIDE = 199;
			const int TEX_OUTSIDE = 200;

			// 1. Top basin floor
			tile->setShape(2.0f / 16.0f, 10.0f / 16.0f, 2.0f / 16.0f, 14.0f / 16.0f, 11.0f / 16.0f, 14.0f / 16.0f);
			t.color(0xff, 0xff, 0xff); renderFaceUp(tile, 0, 0, 0, TEX_INSIDE);
			t.color(0x80, 0x80, 0x80); renderFaceDown(tile, 0, 0, 0, TEX_OUTSIDE);
			t.color(0xd0, 0xd0, 0xd0); renderNorth(tile, 0, 0, 0, TEX_OUTSIDE); renderSouth(tile, 0, 0, 0, TEX_OUTSIDE);
			t.color(0x90, 0x90, 0x90); renderWest(tile, 0, 0, 0, TEX_OUTSIDE); renderEast(tile, 0, 0, 0, TEX_OUTSIDE);

			// 2. Basin 4 walls
			// North wall
			tile->setShape(0.0f, 10.0f / 16.0f, 0.0f, 1.0f, 1.0f, 2.0f / 16.0f);
			t.color(0xff, 0xff, 0xff); renderFaceUp(tile, 0, 0, 0, TEX_TOP);
			t.color(0x80, 0x80, 0x80); renderFaceDown(tile, 0, 0, 0, TEX_OUTSIDE);
			t.color(0xd0, 0xd0, 0xd0); renderNorth(tile, 0, 0, 0, TEX_OUTSIDE); renderSouth(tile, 0, 0, 0, TEX_INSIDE);
			t.color(0x90, 0x90, 0x90); renderWest(tile, 0, 0, 0, TEX_OUTSIDE); renderEast(tile, 0, 0, 0, TEX_OUTSIDE);

			// South wall
			tile->setShape(0.0f, 10.0f / 16.0f, 14.0f / 16.0f, 1.0f, 1.0f, 1.0f);
			t.color(0xff, 0xff, 0xff); renderFaceUp(tile, 0, 0, 0, TEX_TOP);
			t.color(0x80, 0x80, 0x80); renderFaceDown(tile, 0, 0, 0, TEX_OUTSIDE);
			t.color(0xd0, 0xd0, 0xd0); renderNorth(tile, 0, 0, 0, TEX_INSIDE); renderSouth(tile, 0, 0, 0, TEX_OUTSIDE);
			t.color(0x90, 0x90, 0x90); renderWest(tile, 0, 0, 0, TEX_OUTSIDE); renderEast(tile, 0, 0, 0, TEX_OUTSIDE);

			// West wall
			tile->setShape(0.0f, 10.0f / 16.0f, 2.0f / 16.0f, 2.0f / 16.0f, 1.0f, 14.0f / 16.0f);
			t.color(0xff, 0xff, 0xff); renderFaceUp(tile, 0, 0, 0, TEX_TOP);
			t.color(0x80, 0x80, 0x80); renderFaceDown(tile, 0, 0, 0, TEX_OUTSIDE);
			t.color(0x90, 0x90, 0x90); renderWest(tile, 0, 0, 0, TEX_OUTSIDE); renderEast(tile, 0, 0, 0, TEX_INSIDE);

			// East wall
			tile->setShape(14.0f / 16.0f, 10.0f / 16.0f, 2.0f / 16.0f, 1.0f, 1.0f, 14.0f / 16.0f);
			t.color(0xff, 0xff, 0xff); renderFaceUp(tile, 0, 0, 0, TEX_TOP);
			t.color(0x80, 0x80, 0x80); renderFaceDown(tile, 0, 0, 0, TEX_OUTSIDE);
			t.color(0x90, 0x90, 0x90); renderWest(tile, 0, 0, 0, TEX_INSIDE); renderEast(tile, 0, 0, 0, TEX_OUTSIDE);

			// 3. Middle Funnel (4..12, 4..10, 4..12)
			tile->setShape(4.0f / 16.0f, 4.0f / 16.0f, 4.0f / 16.0f, 12.0f / 16.0f, 10.0f / 16.0f, 12.0f / 16.0f);
			t.color(0xff, 0xff, 0xff); renderFaceUp(tile, 0, 0, 0, TEX_OUTSIDE);
			t.color(0x80, 0x80, 0x80); renderFaceDown(tile, 0, 0, 0, TEX_OUTSIDE);
			t.color(0xd0, 0xd0, 0xd0); renderNorth(tile, 0, 0, 0, TEX_OUTSIDE); renderSouth(tile, 0, 0, 0, TEX_OUTSIDE);
			t.color(0x90, 0x90, 0x90); renderWest(tile, 0, 0, 0, TEX_OUTSIDE); renderEast(tile, 0, 0, 0, TEX_OUTSIDE);

			// 4. Spout (down) (6..10, 0..4, 6..10)
			tile->setShape(6.0f / 16.0f, 0.0f, 6.0f / 16.0f, 10.0f / 16.0f, 4.0f / 16.0f, 10.0f / 16.0f);
			t.color(0xff, 0xff, 0xff); renderFaceUp(tile, 0, 0, 0, TEX_OUTSIDE);
			t.color(0x80, 0x80, 0x80); renderFaceDown(tile, 0, 0, 0, TEX_OUTSIDE);
			t.color(0xd0, 0xd0, 0xd0); renderNorth(tile, 0, 0, 0, TEX_OUTSIDE); renderSouth(tile, 0, 0, 0, TEX_OUTSIDE);
			t.color(0x90, 0x90, 0x90); renderWest(tile, 0, 0, 0, TEX_OUTSIDE); renderEast(tile, 0, 0, 0, TEX_OUTSIDE);

			t.draw();
			t.addOffset(0.5f, 0.5f, 0.5f);
			tile->setShape(0, 0, 0, 1, 1, 1);
		}
	}
}

bool TileRenderer::tesselateThinFenceInWorld(ThinFenceTile* tt, int x, int y, int z) {
	const int depth = 128;
	Tesselator& t = Tesselator::instance;

	float br = tt->getBrightness(level, x, y, z);
	int col = tt->getColor(level, x, y, z);
	float r = ((col >> 16) & 0xff) / 255.0f;
	float g = ((col >> 8) & 0xff) / 255.0f;
	float b = ((col) & 0xff) / 255.0f;

	//if (GameRenderer::anaglyph3d) {
	//	float cr = (r * 30 + g * 59 + b * 11) / 100;
	//	float cg = (r * 30 + g * 70) / (100);
	//	float cb = (r * 30 + b * 70) / (100);

	//	r = cr;
	//	g = cg;
	//	b = cb;
	//}
	t.color(br * r, br * g, br * b);

	int tex = 0;
	int edgeTex = 0;

	if (fixedTexture >= 0) {
		tex = fixedTexture;
		edgeTex = fixedTexture;
	} else {
		int data = level->getData(x, y, z);
		tex = tt->getTexture(0, data);
		edgeTex = tt->getEdgeTexture();
	}

	const int xt = (tex & 0xf) << 4;
	const int yt = tex & 0xf0;
	float u0 = (xt) / 256.0f;
	const float u1 = (xt + 7.99f) / 256.0f;
	const float u2 = (xt + 15.99f) / 256.0f;
	const float v0 = (yt) / 256.0f;
	const float v2 = (yt + 15.99f) / 256.0f;

	const int xet = (edgeTex & 0xf) << 4;
	const int yet = edgeTex & 0xf0;

	const float iu0 = (xet + 7) / 256.0f;
	const float iu1 = (xet + 8.99f) / 256.0f;
	const float iv0 = (yet) / 256.0f;
	const float iv1 = (yet + 8) / 256.0f;
	const float iv2 = (yet + 15.99f) / 256.0f;

	const float x0 = (float)x;
	const float x1 = x0 + .5f;
	const float x2 = x0 + 1;
	const float y0 = (float)y + 0.001f;
	const float y1 = y0 + 1 - 0.002f;
	const float z0 = (float)z;
	const float z1 = z0 + .5f;
	const float z2 = z0 + 1;
	const float ix0 = x0 + .5f - 1.0f / 16.0f;
	const float ix1 = x0 + .5f + 1.0f / 16.0f;
	const float iz0 = z0 + .5f - 1.0f / 16.0f;
	const float iz1 = z0 + .5f + 1.0f / 16.0f;

	const bool n = tt->attachsTo(level->getTile(x, y, z - 1));
	const bool s = tt->attachsTo(level->getTile(x, y, z + 1));
	const bool w = tt->attachsTo(level->getTile(x - 1, y, z));
	const bool e = tt->attachsTo(level->getTile(x + 1, y, z));

	const bool up = tt->shouldRenderFace(level, x, y + 1, z, Facing::UP);
	const bool down = tt->shouldRenderFace(level, x, y - 1, z, Facing::DOWN);

	const float noZFightingOffset = 0.01f;

	if ((w && e) || (!w && !e && !n && !s)) {
		t.vertexUV(x0, y1, z1, u0, v0);
		t.vertexUV(x0, y0, z1, u0, v2);
		t.vertexUV(x2, y0, z1, u2, v2);
		t.vertexUV(x2, y1, z1, u2, v0);

		t.vertexUV(x2, y1, z1, u0, v0);
		t.vertexUV(x2, y0, z1, u0, v2);
		t.vertexUV(x0, y0, z1, u2, v2);
		t.vertexUV(x0, y1, z1, u2, v0);

		if (up) {
			// small edge texture
			t.vertexUV(x0, y1 + noZFightingOffset, iz1, iu1, iv2);
			t.vertexUV(x2, y1 + noZFightingOffset, iz1, iu1, iv0);
			t.vertexUV(x2, y1 + noZFightingOffset, iz0, iu0, iv0);
			t.vertexUV(x0, y1 + noZFightingOffset, iz0, iu0, iv2);

			t.vertexUV(x2, y1 + noZFightingOffset, iz1, iu1, iv2);
			t.vertexUV(x0, y1 + noZFightingOffset, iz1, iu1, iv0);
			t.vertexUV(x0, y1 + noZFightingOffset, iz0, iu0, iv0);
			t.vertexUV(x2, y1 + noZFightingOffset, iz0, iu0, iv2);
		} else {
			if (y < (depth - 1) && level->isEmptyTile(x - 1, y + 1, z)) {
				t.vertexUV(x0, y1 + noZFightingOffset, iz1, iu1, iv1);
				t.vertexUV(x1, y1 + noZFightingOffset, iz1, iu1, iv2);
				t.vertexUV(x1, y1 + noZFightingOffset, iz0, iu0, iv2);
				t.vertexUV(x0, y1 + noZFightingOffset, iz0, iu0, iv1);

				t.vertexUV(x1, y1 + noZFightingOffset, iz1, iu1, iv1);
				t.vertexUV(x0, y1 + noZFightingOffset, iz1, iu1, iv2);
				t.vertexUV(x0, y1 + noZFightingOffset, iz0, iu0, iv2);
				t.vertexUV(x1, y1 + noZFightingOffset, iz0, iu0, iv1);
			}
			if (y < (depth - 1) && level->isEmptyTile(x + 1, y + 1, z)) {
				t.vertexUV(x1, y1 + noZFightingOffset, iz1, iu1, iv0);
				t.vertexUV(x2, y1 + noZFightingOffset, iz1, iu1, iv1);
				t.vertexUV(x2, y1 + noZFightingOffset, iz0, iu0, iv1);
				t.vertexUV(x1, y1 + noZFightingOffset, iz0, iu0, iv0);

				t.vertexUV(x2, y1 + noZFightingOffset, iz1, iu1, iv0);
				t.vertexUV(x1, y1 + noZFightingOffset, iz1, iu1, iv1);
				t.vertexUV(x1, y1 + noZFightingOffset, iz0, iu0, iv1);
				t.vertexUV(x2, y1 + noZFightingOffset, iz0, iu0, iv0);
			}
		}
		if (down) {
			// small edge texture
			t.vertexUV(x0, y0 - noZFightingOffset, iz1, iu1, iv2);
			t.vertexUV(x2, y0 - noZFightingOffset, iz1, iu1, iv0);
			t.vertexUV(x2, y0 - noZFightingOffset, iz0, iu0, iv0);
			t.vertexUV(x0, y0 - noZFightingOffset, iz0, iu0, iv2);

			t.vertexUV(x2, y0 - noZFightingOffset, iz1, iu1, iv2);
			t.vertexUV(x0, y0 - noZFightingOffset, iz1, iu1, iv0);
			t.vertexUV(x0, y0 - noZFightingOffset, iz0, iu0, iv0);
			t.vertexUV(x2, y0 - noZFightingOffset, iz0, iu0, iv2);
		} else {
			if (y > 1 && level->isEmptyTile(x - 1, y - 1, z)) {
				t.vertexUV(x0, y0 - noZFightingOffset, iz1, iu1, iv1);
				t.vertexUV(x1, y0 - noZFightingOffset, iz1, iu1, iv2);
				t.vertexUV(x1, y0 - noZFightingOffset, iz0, iu0, iv2);
				t.vertexUV(x0, y0 - noZFightingOffset, iz0, iu0, iv1);

				t.vertexUV(x1, y0 - noZFightingOffset, iz1, iu1, iv1);
				t.vertexUV(x0, y0 - noZFightingOffset, iz1, iu1, iv2);
				t.vertexUV(x0, y0 - noZFightingOffset, iz0, iu0, iv2);
				t.vertexUV(x1, y0 - noZFightingOffset, iz0, iu0, iv1);
			}
			if (y > 1 && level->isEmptyTile(x + 1, y - 1, z)) {
				t.vertexUV(x1, y0 - noZFightingOffset, iz1, iu1, iv0);
				t.vertexUV(x2, y0 - noZFightingOffset, iz1, iu1, iv1);
				t.vertexUV(x2, y0 - noZFightingOffset, iz0, iu0, iv1);
				t.vertexUV(x1, y0 - noZFightingOffset, iz0, iu0, iv0);

				t.vertexUV(x2, y0 - noZFightingOffset, iz1, iu1, iv0);
				t.vertexUV(x1, y0 - noZFightingOffset, iz1, iu1, iv1);
				t.vertexUV(x1, y0 - noZFightingOffset, iz0, iu0, iv1);
				t.vertexUV(x2, y0 - noZFightingOffset, iz0, iu0, iv0);
			}
		}

	} else if (w && !e) {
		// half-step towards west
		t.vertexUV(x0, y1, z1, u0, v0);
		t.vertexUV(x0, y0, z1, u0, v2);
		t.vertexUV(x1, y0, z1, u1, v2);
		t.vertexUV(x1, y1, z1, u1, v0);

		t.vertexUV(x1, y1, z1, u0, v0);
		t.vertexUV(x1, y0, z1, u0, v2);
		t.vertexUV(x0, y0, z1, u1, v2);
		t.vertexUV(x0, y1, z1, u1, v0);

		// small edge texture
		if (!s && !n) {
			t.vertexUV(x1, y1, iz1, iu0, iv0);
			t.vertexUV(x1, y0, iz1, iu0, iv2);
			t.vertexUV(x1, y0, iz0, iu1, iv2);
			t.vertexUV(x1, y1, iz0, iu1, iv0);

			t.vertexUV(x1, y1, iz0, iu0, iv0);
			t.vertexUV(x1, y0, iz0, iu0, iv2);
			t.vertexUV(x1, y0, iz1, iu1, iv2);
			t.vertexUV(x1, y1, iz1, iu1, iv0);
		}

		if (up || (y < (depth - 1) && level->isEmptyTile(x - 1, y + 1, z))) {
			// small edge texture
			t.vertexUV(x0, y1 + noZFightingOffset, iz1, iu1, iv1);
			t.vertexUV(x1, y1 + noZFightingOffset, iz1, iu1, iv2);
			t.vertexUV(x1, y1 + noZFightingOffset, iz0, iu0, iv2);
			t.vertexUV(x0, y1 + noZFightingOffset, iz0, iu0, iv1);

			t.vertexUV(x1, y1 + noZFightingOffset, iz1, iu1, iv1);
			t.vertexUV(x0, y1 + noZFightingOffset, iz1, iu1, iv2);
			t.vertexUV(x0, y1 + noZFightingOffset, iz0, iu0, iv2);
			t.vertexUV(x1, y1 + noZFightingOffset, iz0, iu0, iv1);
		}
		if (down || (y > 1 && level->isEmptyTile(x - 1, y - 1, z))) {
			// small edge texture
			t.vertexUV(x0, y0 - noZFightingOffset, iz1, iu1, iv1);
			t.vertexUV(x1, y0 - noZFightingOffset, iz1, iu1, iv2);
			t.vertexUV(x1, y0 - noZFightingOffset, iz0, iu0, iv2);
			t.vertexUV(x0, y0 - noZFightingOffset, iz0, iu0, iv1);

			t.vertexUV(x1, y0 - noZFightingOffset, iz1, iu1, iv1);
			t.vertexUV(x0, y0 - noZFightingOffset, iz1, iu1, iv2);
			t.vertexUV(x0, y0 - noZFightingOffset, iz0, iu0, iv2);
			t.vertexUV(x1, y0 - noZFightingOffset, iz0, iu0, iv1);
		}

	} else if (!w && e) {
		// half-step towards east
		t.vertexUV(x1, y1, z1, u1, v0);
		t.vertexUV(x1, y0, z1, u1, v2);
		t.vertexUV(x2, y0, z1, u2, v2);
		t.vertexUV(x2, y1, z1, u2, v0);

		t.vertexUV(x2, y1, z1, u1, v0);
		t.vertexUV(x2, y0, z1, u1, v2);
		t.vertexUV(x1, y0, z1, u2, v2);
		t.vertexUV(x1, y1, z1, u2, v0);

		// small edge texture
		if (!s && !n) {
			t.vertexUV(x1, y1, iz0, iu0, iv0);
			t.vertexUV(x1, y0, iz0, iu0, iv2);
			t.vertexUV(x1, y0, iz1, iu1, iv2);
			t.vertexUV(x1, y1, iz1, iu1, iv0);

			t.vertexUV(x1, y1, iz1, iu0, iv0);
			t.vertexUV(x1, y0, iz1, iu0, iv2);
			t.vertexUV(x1, y0, iz0, iu1, iv2);
			t.vertexUV(x1, y1, iz0, iu1, iv0);
		}

		if (up || (y < (depth - 1) && level->isEmptyTile(x + 1, y + 1, z))) {
			// small edge texture
			t.vertexUV(x1, y1 + noZFightingOffset, iz1, iu1, iv0);
			t.vertexUV(x2, y1 + noZFightingOffset, iz1, iu1, iv1);
			t.vertexUV(x2, y1 + noZFightingOffset, iz0, iu0, iv1);
			t.vertexUV(x1, y1 + noZFightingOffset, iz0, iu0, iv0);

			t.vertexUV(x2, y1 + noZFightingOffset, iz1, iu1, iv0);
			t.vertexUV(x1, y1 + noZFightingOffset, iz1, iu1, iv1);
			t.vertexUV(x1, y1 + noZFightingOffset, iz0, iu0, iv1);
			t.vertexUV(x2, y1 + noZFightingOffset, iz0, iu0, iv0);
		}
		if (down || (y > 1 && level->isEmptyTile(x + 1, y - 1, z))) {
			// small edge texture
			t.vertexUV(x1, y0 - noZFightingOffset, iz1, iu1, iv0);
			t.vertexUV(x2, y0 - noZFightingOffset, iz1, iu1, iv1);
			t.vertexUV(x2, y0 - noZFightingOffset, iz0, iu0, iv1);
			t.vertexUV(x1, y0 - noZFightingOffset, iz0, iu0, iv0);

			t.vertexUV(x2, y0 - noZFightingOffset, iz1, iu1, iv0);
			t.vertexUV(x1, y0 - noZFightingOffset, iz1, iu1, iv1);
			t.vertexUV(x1, y0 - noZFightingOffset, iz0, iu0, iv1);
			t.vertexUV(x2, y0 - noZFightingOffset, iz0, iu0, iv0);
		}

	}

	if ((n && s) || (!w && !e && !n && !s)) {
		// straight north-south
		t.vertexUV(x1, y1, z2, u0, v0);
		t.vertexUV(x1, y0, z2, u0, v2);
		t.vertexUV(x1, y0, z0, u2, v2);
		t.vertexUV(x1, y1, z0, u2, v0);

		t.vertexUV(x1, y1, z0, u0, v0);
		t.vertexUV(x1, y0, z0, u0, v2);
		t.vertexUV(x1, y0, z2, u2, v2);
		t.vertexUV(x1, y1, z2, u2, v0);

		if (up) {
			// small edge texture
			t.vertexUV(ix1, y1, z2, iu1, iv2);
			t.vertexUV(ix1, y1, z0, iu1, iv0);
			t.vertexUV(ix0, y1, z0, iu0, iv0);
			t.vertexUV(ix0, y1, z2, iu0, iv2);

			t.vertexUV(ix1, y1, z0, iu1, iv2);
			t.vertexUV(ix1, y1, z2, iu1, iv0);
			t.vertexUV(ix0, y1, z2, iu0, iv0);
			t.vertexUV(ix0, y1, z0, iu0, iv2);
		} else {
			if (y < (depth - 1) && level->isEmptyTile(x, y + 1, z - 1)) {
				t.vertexUV(ix0, y1, z0, iu1, iv0);
				t.vertexUV(ix0, y1, z1, iu1, iv1);
				t.vertexUV(ix1, y1, z1, iu0, iv1);
				t.vertexUV(ix1, y1, z0, iu0, iv0);

				t.vertexUV(ix0, y1, z1, iu1, iv0);
				t.vertexUV(ix0, y1, z0, iu1, iv1);
				t.vertexUV(ix1, y1, z0, iu0, iv1);
				t.vertexUV(ix1, y1, z1, iu0, iv0);
			}
			if (y < (depth - 1) && level->isEmptyTile(x, y + 1, z + 1)) {
				t.vertexUV(ix0, y1, z1, iu0, iv1);
				t.vertexUV(ix0, y1, z2, iu0, iv2);
				t.vertexUV(ix1, y1, z2, iu1, iv2);
				t.vertexUV(ix1, y1, z1, iu1, iv1);

				t.vertexUV(ix0, y1, z2, iu0, iv1);
				t.vertexUV(ix0, y1, z1, iu0, iv2);
				t.vertexUV(ix1, y1, z1, iu1, iv2);
				t.vertexUV(ix1, y1, z2, iu1, iv1);
			}
		}
		if (down) {
			// small edge texture
			t.vertexUV(ix1, y0, z2, iu1, iv2);
			t.vertexUV(ix1, y0, z0, iu1, iv0);
			t.vertexUV(ix0, y0, z0, iu0, iv0);
			t.vertexUV(ix0, y0, z2, iu0, iv2);

			t.vertexUV(ix1, y0, z0, iu1, iv2);
			t.vertexUV(ix1, y0, z2, iu1, iv0);
			t.vertexUV(ix0, y0, z2, iu0, iv0);
			t.vertexUV(ix0, y0, z0, iu0, iv2);
		} else {
			if (y > 1 && level->isEmptyTile(x, y - 1, z - 1)) {
				// north half-step
				t.vertexUV(ix0, y0, z0, iu1, iv0);
				t.vertexUV(ix0, y0, z1, iu1, iv1);
				t.vertexUV(ix1, y0, z1, iu0, iv1);
				t.vertexUV(ix1, y0, z0, iu0, iv0);

				t.vertexUV(ix0, y0, z1, iu1, iv0);
				t.vertexUV(ix0, y0, z0, iu1, iv1);
				t.vertexUV(ix1, y0, z0, iu0, iv1);
				t.vertexUV(ix1, y0, z1, iu0, iv0);
			}
			if (y > 1 && level->isEmptyTile(x, y - 1, z + 1)) {
				// south half-step
				t.vertexUV(ix0, y0, z1, iu0, iv1);
				t.vertexUV(ix0, y0, z2, iu0, iv2);
				t.vertexUV(ix1, y0, z2, iu1, iv2);
				t.vertexUV(ix1, y0, z1, iu1, iv1);

				t.vertexUV(ix0, y0, z2, iu0, iv1);
				t.vertexUV(ix0, y0, z1, iu0, iv2);
				t.vertexUV(ix1, y0, z1, iu1, iv2);
				t.vertexUV(ix1, y0, z2, iu1, iv1);
			}
		}

	} else if (n && !s) {
		// half-step towards north
		t.vertexUV(x1, y1, z0, u0, v0);
		t.vertexUV(x1, y0, z0, u0, v2);
		t.vertexUV(x1, y0, z1, u1, v2);
		t.vertexUV(x1, y1, z1, u1, v0);

		t.vertexUV(x1, y1, z1, u0, v0);
		t.vertexUV(x1, y0, z1, u0, v2);
		t.vertexUV(x1, y0, z0, u1, v2);
		t.vertexUV(x1, y1, z0, u1, v0);

		// small edge texture
		if (!e && !w) {
			t.vertexUV(ix0, y1, z1, iu0, iv0);
			t.vertexUV(ix0, y0, z1, iu0, iv2);
			t.vertexUV(ix1, y0, z1, iu1, iv2);
			t.vertexUV(ix1, y1, z1, iu1, iv0);

			t.vertexUV(ix1, y1, z1, iu0, iv0);
			t.vertexUV(ix1, y0, z1, iu0, iv2);
			t.vertexUV(ix0, y0, z1, iu1, iv2);
			t.vertexUV(ix0, y1, z1, iu1, iv0);
		}

		if (up || (y < (depth - 1) && level->isEmptyTile(x, y + 1, z - 1))) {
			// small edge texture
			t.vertexUV(ix0, y1, z0, iu1, iv0);
			t.vertexUV(ix0, y1, z1, iu1, iv1);
			t.vertexUV(ix1, y1, z1, iu0, iv1);
			t.vertexUV(ix1, y1, z0, iu0, iv0);

			t.vertexUV(ix0, y1, z1, iu1, iv0);
			t.vertexUV(ix0, y1, z0, iu1, iv1);
			t.vertexUV(ix1, y1, z0, iu0, iv1);
			t.vertexUV(ix1, y1, z1, iu0, iv0);
		}

		if (down || (y > 1 && level->isEmptyTile(x, y - 1, z - 1))) {
			// small edge texture
			t.vertexUV(ix0, y0, z0, iu1, iv0);
			t.vertexUV(ix0, y0, z1, iu1, iv1);
			t.vertexUV(ix1, y0, z1, iu0, iv1);
			t.vertexUV(ix1, y0, z0, iu0, iv0);

			t.vertexUV(ix0, y0, z1, iu1, iv0);
			t.vertexUV(ix0, y0, z0, iu1, iv1);
			t.vertexUV(ix1, y0, z0, iu0, iv1);
			t.vertexUV(ix1, y0, z1, iu0, iv0);
		}

	} else if (!n && s) {
		// half-step towards south
		t.vertexUV(x1, y1, z1, u1, v0);
		t.vertexUV(x1, y0, z1, u1, v2);
		t.vertexUV(x1, y0, z2, u2, v2);
		t.vertexUV(x1, y1, z2, u2, v0);

		t.vertexUV(x1, y1, z2, u1, v0);
		t.vertexUV(x1, y0, z2, u1, v2);
		t.vertexUV(x1, y0, z1, u2, v2);
		t.vertexUV(x1, y1, z1, u2, v0);

		// small edge texture
		if (!e && !w) {
			t.vertexUV(ix1, y1, z1, iu0, iv0);
			t.vertexUV(ix1, y0, z1, iu0, iv2);
			t.vertexUV(ix0, y0, z1, iu1, iv2);
			t.vertexUV(ix0, y1, z1, iu1, iv0);

			t.vertexUV(ix0, y1, z1, iu0, iv0);
			t.vertexUV(ix0, y0, z1, iu0, iv2);
			t.vertexUV(ix1, y0, z1, iu1, iv2);
			t.vertexUV(ix1, y1, z1, iu1, iv0);
		}

		if (up || (y < (depth - 1) && level->isEmptyTile(x, y + 1, z + 1))) {
			// small edge texture
			t.vertexUV(ix0, y1, z1, iu0, iv1);
			t.vertexUV(ix0, y1, z2, iu0, iv2);
			t.vertexUV(ix1, y1, z2, iu1, iv2);
			t.vertexUV(ix1, y1, z1, iu1, iv1);

			t.vertexUV(ix0, y1, z2, iu0, iv1);
			t.vertexUV(ix0, y1, z1, iu0, iv2);
			t.vertexUV(ix1, y1, z1, iu1, iv2);
			t.vertexUV(ix1, y1, z2, iu1, iv1);
		}
		if (down || (y > 1 && level->isEmptyTile(x, y - 1, z + 1))) {
			// small edge texture
			t.vertexUV(ix0, y0, z1, iu0, iv1);
			t.vertexUV(ix0, y0, z2, iu0, iv2);
			t.vertexUV(ix1, y0, z2, iu1, iv2);
			t.vertexUV(ix1, y0, z1, iu1, iv1);

			t.vertexUV(ix0, y0, z2, iu0, iv1);
			t.vertexUV(ix0, y0, z1, iu0, iv2);
			t.vertexUV(ix1, y0, z1, iu1, iv2);
			t.vertexUV(ix1, y0, z2, iu1, iv1);
		}

	}

	return true;
}

void TileRenderer::tesselateRowTexture( Tile* tt, int data, float x, float y, float z ) {
	Tesselator& t = Tesselator::instance;

	int tex = tt->getTexture(0, data);
	if(fixedTexture >= 0)
		tex = fixedTexture;

	int xt = (tex & 0xf) << 4;
	int yt = tex & 0xf0;
	float u0 = (xt) / 256.0f;
	float u1 = (xt + 15.99f) / 256.f;
	float v0 = (yt) / 256.0f;
	float v1 = (yt + 15.99f) / 256.0f;

	float x0 = x + 0.5f - 0.25f;
	float x1 = x + 0.5f + 0.25f;
	float z0 = z + 0.5f - 0.5f;
	float z1 = z + 0.5f + 0.5f;
	t.vertexUV(x0, y + 1, z0, u0, v0);
	t.vertexUV(x0, y + 0, z0, u0, v1);
	t.vertexUV(x0, y + 0, z1, u1, v1);
	t.vertexUV(x0, y + 1, z1, u1, v0);

	t.vertexUV(x0, y + 1, z1, u0, v0);
	t.vertexUV(x0, y + 0, z1, u0, v1);
	t.vertexUV(x0, y + 0, z0, u1, v1);
	t.vertexUV(x0, y + 1, z0, u1, v0);

	t.vertexUV(x1, y + 1, z1, u0, v0);
	t.vertexUV(x1, y + 0, z1, u0, v1);
	t.vertexUV(x1, y + 0, z0, u1, v1);
	t.vertexUV(x1, y + 1, z0, u1, v0);

	t.vertexUV(x1, y + 1, z0, u0, v0);
	t.vertexUV(x1, y + 0, z0, u0, v1);
	t.vertexUV(x1, y + 0, z1, u1, v1);
	t.vertexUV(x1, y + 1, z1, u1, v0);

	x0 = x + 0.5f - 0.5f;
	x1 = x + 0.5f + 0.5f;
	z0 = z + 0.5f - 0.25f;
	z1 = z + 0.5f + 0.25f;

	t.vertexUV(x0, y + 1, z0, u0, v0);
	t.vertexUV(x0, y + 0, z0, u0, v1);
	t.vertexUV(x1, y + 0, z0, u1, v1);
	t.vertexUV(x1, y + 1, z0, u1, v0);

	t.vertexUV(x1, y + 1, z0, u0, v0);
	t.vertexUV(x1, y + 0, z0, u0, v1);
	t.vertexUV(x0, y + 0, z0, u1, v1);
	t.vertexUV(x0, y + 1, z0, u1, v0);

	t.vertexUV(x1, y + 1, z1, u0, v0);
	t.vertexUV(x1, y + 0, z1, u0, v1);
	t.vertexUV(x0, y + 0, z1, u1, v1);
	t.vertexUV(x0, y + 1, z1, u1, v0);

	t.vertexUV(x0, y + 1, z1, u0, v0);
	t.vertexUV(x0, y + 0, z1, u0, v1);
	t.vertexUV(x1, y + 0, z1, u1, v1);
	t.vertexUV(x1, y + 1, z1, u1, v0);
}

bool TileRenderer::renderWithMaterialInstances(Tile* tile, int x, int y, int z, int data) {
	Tesselator& t = Tesselator::instance;
	if (!textures) {
		return false;
	}

	if (!tile->hasMaterialInstances()) {
		return false;
	}

	bool inChunkPass = t.isTesselating();
	if (!inChunkPass) {
		tile->updateDefaultShape();
		t.addOffset(-0.5f, -0.5f, -0.5f);
		t.begin();
	}

	bool changed = false;
	float c10 = 0.5f;
	float c11 = 1.0f;
	float c2 = 0.8f;
	float c3 = 0.6f;

	int col = level ? tile->getColor(level, x, y, z) : 0xffffff;
	float r = ((col >> 16) & 0xff) / 255.0f;
	float g = ((col >> 8) & 0xff) / 255.0f;
	float b = ((col) & 0xff) / 255.0f;

	float centerBrightness = level ? tile->getBrightness(level, x, y, z) : 1.0f;

	// Renderizar cada cara con su textura y culling correspondiente
	for (int face = 0; face < 6; face++) {
		bool shouldRender = false;
		float br = 1.0f;
		float cr = 1.0f, cg = 1.0f, cb = 1.0f;

		switch (face) {
			case 0: // DOWN
				shouldRender = noCulling || !level || tile->shouldRenderFace(level, x, y - 1, z, 0);
				if (shouldRender) {
					br = level ? tile->getBrightness(level, x, y - 1, z) : 1.0f;
					cr = r * c10 * br; cg = g * c10 * br; cb = b * c10 * br;
				}
				break;
			case 1: // UP
				shouldRender = noCulling || !level || tile->shouldRenderFace(level, x, y + 1, z, 1);
				if (shouldRender) {
					br = level ? tile->getBrightness(level, x, y + 1, z) : 1.0f;
					if (tile->yy1 != 1 && !tile->material->isLiquid() && level) br = centerBrightness;
					cr = r * c11 * br; cg = g * c11 * br; cb = b * c11 * br;
				}
				break;
			case 2: // NORTH
				shouldRender = noCulling || !level || tile->shouldRenderFace(level, x, y, z - 1, 2);
				if (shouldRender) {
					br = level ? tile->getBrightness(level, x, y, z - 1) : 1.0f;
					if (tile->zz0 > 0 && level) br = centerBrightness;
					cr = r * c2 * br; cg = g * c2 * br; cb = b * c2 * br;
				}
				break;
			case 3: // SOUTH
				shouldRender = noCulling || !level || tile->shouldRenderFace(level, x, y, z + 1, 3);
				if (shouldRender) {
					br = level ? tile->getBrightness(level, x, y, z + 1) : 1.0f;
					if (tile->zz1 < 1 && level) br = centerBrightness;
					cr = r * c2 * br; cg = g * c2 * br; cb = b * c2 * br;
				}
				break;
			case 4: // WEST
				shouldRender = noCulling || !level || tile->shouldRenderFace(level, x - 1, y, z, 4);
				if (shouldRender) {
					br = level ? tile->getBrightness(level, x - 1, y, z) : 1.0f;
					if (tile->xx0 > 0 && level) br = centerBrightness;
					cr = r * c3 * br; cg = g * c3 * br; cb = b * c3 * br;
				}
				break;
			case 5: // EAST
				shouldRender = noCulling || !level || tile->shouldRenderFace(level, x + 1, y, z, 5);
				if (shouldRender) {
					br = level ? tile->getBrightness(level, x + 1, y, z) : 1.0f;
					if (tile->xx1 < 1 && level) br = centerBrightness;
					cr = r * c3 * br; cg = g * c3 * br; cb = b * c3 * br;
				}
				break;
		}

		if (!shouldRender) continue;

		t.color(cr, cg, cb);

		Tile::BlockFace blockFace = Tile::renderFaceToBlockFace(face);
		const Tile::MaterialInstance* matInst = tile->getMaterialInstance(blockFace);
		
		int textureIndex = 0;
		bool useSeparateTexture = false;
		
		if (matInst && matInst->usesSeparateTexture()) {
			std::string texturePath = "data/images/blocks/" + matInst->textureName + ".png";
			TextureId texId = textures->loadTexture(texturePath, false);
			if (texId != Textures::InvalidId) {
				if (!inChunkPass) {
					textures->bind(texId);
				}
			}
			textureIndex = matInst->textureIndex > 0 ? matInst->textureIndex : tile->getTexture(face, data);
			useSeparateTexture = true;
		} else if (matInst) {
			if (!inChunkPass) {
				textures->loadAndBindTexture((matInst->textureIndex & Tile::TEXTURE_ALT_FLAG) ? "terrain2.png" : "terrain.png");
			}
			textureIndex = matInst->textureIndex > 0 ? matInst->textureIndex : tile->getTexture(face, data);
			useSeparateTexture = false;
		} else {
			if (!inChunkPass) {
				textures->loadAndBindTexture("terrain.png");
			}
			textureIndex = tile->getTexture(face, data);
			useSeparateTexture = false;
		}
		
		bool isAlt = (textureIndex & Tile::TEXTURE_ALT_FLAG) != 0;
		if (inChunkPass && atlasFilter != -1) {
			if (atlasFilter == 0 && isAlt) continue;
			if (atlasFilter == 1 && !isAlt) continue;
		}

		float x0 = (float)x + tile->xx0;
		float x1 = (float)x + tile->xx1;
		float y0 = (float)y + tile->yy0;
		float y1 = (float)y + tile->yy1;
		float z0 = (float)z + tile->zz0;
		float z1 = (float)z + tile->zz1;

		if (inChunkPass || !useSeparateTexture) {
			int cleanTex = textureIndex & ~Tile::TEXTURE_ALT_FLAG;
			switch(face) {
				case 0: // DOWN
					t.normal(0.0f, -1.0f, 0.0f);
					renderFaceDown(tile, (float)x, (float)y, (float)z, cleanTex);
					break;
				case 1: // UP
					t.normal(0.0f, 1.0f, 0.0f);
					renderFaceUp(tile, (float)x, (float)y, (float)z, cleanTex);
					break;
				case 2: // NORTH
					t.normal(0.0f, 0.0f, -1.0f);
					renderNorth(tile, (float)x, (float)y, (float)z, cleanTex);
					break;
				case 3: // SOUTH
					t.normal(0.0f, 0.0f, 1.0f);
					renderSouth(tile, (float)x, (float)y, (float)z, cleanTex);
					break;
				case 4: // WEST
					t.normal(-1.0f, 0.0f, 0.0f);
					renderWest(tile, (float)x, (float)y, (float)z, cleanTex);
					break;
				case 5: // EAST
					t.normal(1.0f, 0.0f, 0.0f);
					renderEast(tile, (float)x, (float)y, (float)z, cleanTex);
					break;
			}
		} else {
			switch(face) {
				case 0: // DOWN
					t.normal(0.0f, -1.0f, 0.0f);
					t.vertexUV(x0, y0, z0, 0.0f, 0.0f);
					t.vertexUV(x1, y0, z0, 1.0f, 0.0f);
					t.vertexUV(x1, y0, z1, 1.0f, 1.0f);
					t.vertexUV(x0, y0, z1, 0.0f, 1.0f);
					break;
				case 1: // UP
					t.normal(0.0f, 1.0f, 0.0f);
					t.vertexUV(x0, y1, z0, 0.0f, 0.0f);
					t.vertexUV(x1, y1, z0, 1.0f, 0.0f);
					t.vertexUV(x1, y1, z1, 1.0f, 1.0f);
					t.vertexUV(x0, y1, z1, 0.0f, 1.0f);
					break;
				case 2: // NORTH
					t.normal(0.0f, 0.0f, -1.0f);
					t.vertexUV(x0, y0, z0, 0.0f, 0.0f);
					t.vertexUV(x1, y0, z0, 1.0f, 0.0f);
					t.vertexUV(x1, y1, z0, 1.0f, 1.0f);
					t.vertexUV(x0, y1, z0, 0.0f, 1.0f);
					break;
				case 3: // SOUTH
					t.normal(0.0f, 0.0f, 1.0f);
					t.vertexUV(x0, y0, z1, 0.0f, 0.0f);
					t.vertexUV(x1, y0, z1, 1.0f, 0.0f);
					t.vertexUV(x1, y1, z1, 1.0f, 1.0f);
					t.vertexUV(x0, y1, z1, 0.0f, 1.0f);
					break;
				case 4: // WEST
					t.normal(-1.0f, 0.0f, 0.0f);
					t.vertexUV(x0, y0, z0, 0.0f, 0.0f);
					t.vertexUV(x0, y0, z1, 1.0f, 0.0f);
					t.vertexUV(x0, y1, z1, 1.0f, 1.0f);
					t.vertexUV(x0, y1, z0, 0.0f, 1.0f);
					break;
				case 5: // EAST
					t.normal(1.0f, 0.0f, 0.0f);
					t.vertexUV(x1, y0, z0, 0.0f, 0.0f);
					t.vertexUV(x1, y0, z1, 1.0f, 0.0f);
					t.vertexUV(x1, y1, z1, 1.0f, 1.0f);
					t.vertexUV(x1, y1, z0, 0.0f, 1.0f);
					break;
			}
		}
		changed = true;
	}

	if (!inChunkPass) {
		t.draw();
		t.addOffset(0.5f, 0.5f, 0.5f);
	}

	return changed;
}

bool TileRenderer::tesselateLanternInWorld(Tile* tt, int x, int y, int z) {
	bool hanging = level && (level->isSolidBlockingTile(x, y + 1, z) || 
		(Tile::ironChain && level->getTile(x, y + 1, z) == Tile::ironChain->id) || 
		(Tile::fence && level->getTile(x, y + 1, z) == Tile::fence->id) || 
		(Tile::ironBars && level->getTile(x, y + 1, z) == Tile::ironBars->id) || 
		(Tile::lantern && level->getTile(x, y + 1, z) == Tile::lantern->id) || 
		(Tile::soulLantern && level->getTile(x, y + 1, z) == Tile::soulLantern->id) ||
		(level->getTile(x, y + 1, z) != 0 && !level->isSolidRenderTile(x, y - 1, z)));

	Tesselator& t = Tesselator::instance;
	float br = tt->getBrightness(level, x, y, z);
	int tex = tt->getTexture(level, x, y, z, 0) & ~Tile::TEXTURE_ALT_FLAG;
	int xt = (tex & 0xf) << 4;
	int yt = tex & 0xf0;
	const float atlasSize = 256.0f;

	float xf = (float)x;
	float yf = (float)y;
	float zf = (float)z;

	// UVs for lantern parts within the 16x16 tile
	// Body sides: 6 wide, 7 high (x: 0..6, y: 2..9)
	float uSide0 = (xt + 0.0f) / atlasSize;
	float uSide1 = (xt + 6.0f) / atlasSize;
	float vSide0 = (yt + 2.0f) / atlasSize;
	float vSide1 = (yt + 9.0f) / atlasSize;

	// Body top/bottom lid: 6x6 (x: 0..6, y: 9..15)
	float uLid0 = (xt + 0.0f) / atlasSize;
	float uLid1 = (xt + 6.0f) / atlasSize;
	float vLid0 = (yt + 9.0f) / atlasSize;
	float vLid1 = (yt + 15.0f) / atlasSize;

	// Cap sides: 4 wide, 2 high (x: 1..5, y: 0..2)
	float uCap0 = (xt + 1.0f) / atlasSize;
	float uCap1 = (xt + 5.0f) / atlasSize;
	float vCap0 = (yt + 0.0f) / atlasSize;
	float vCap1 = (yt + 2.0f) / atlasSize;

	// Cap top: 4x4 (x: 1..5, y: 9..13)
	float uCapTop0 = (xt + 1.0f) / atlasSize;
	float uCapTop1 = (xt + 5.0f) / atlasSize;
	float vCapTop0 = (yt + 9.0f) / atlasSize;
	float vCapTop1 = (yt + 13.0f) / atlasSize;

	// Handle / Ring: 3 wide, 4 high (x: 11..14, y: 1..5)
	float uRing0 = (xt + 11.0f) / atlasSize;
	float uRing1 = (xt + 14.0f) / atlasSize;
	float vRing0 = (yt + 1.0f) / atlasSize;
	float vRing1 = (yt + 5.0f) / atlasSize;

	// Hanging chain: 3 wide, 6 high (x: 11..14, y: 6..12)
	float uChain0 = (xt + 11.0f) / atlasSize;
	float uChain1 = (xt + 14.0f) / atlasSize;
	float vChain0 = (yt + 6.0f) / atlasSize;
	float vChain1 = (yt + 12.0f) / atlasSize;

	// Colors for directional lighting
	float colUp = 1.0f * br;
	float colDown = 0.5f * br;
	float colNorthSouth = 0.8f * br;
	float colWestEast = 0.6f * br;

	float bx0 = xf + 5.0f / 16.0f;
	float bx1 = xf + 11.0f / 16.0f;
	float bz0 = zf + 5.0f / 16.0f;
	float bz1 = zf + 11.0f / 16.0f;
	float by0 = hanging ? (yf + 1.0f / 16.0f) : (yf + 0.0f / 16.0f);
	float by1 = hanging ? (yf + 8.0f / 16.0f) : (yf + 7.0f / 16.0f);

	// 1. Lantern Main Body (6x7x6)
	// Up
	t.color(colUp, colUp, colUp);
	t.vertexUV(bx0, by1, bz0, uLid0, vLid0);
	t.vertexUV(bx0, by1, bz1, uLid0, vLid1);
	t.vertexUV(bx1, by1, bz1, uLid1, vLid1);
	t.vertexUV(bx1, by1, bz0, uLid1, vLid0);

	// Down
	t.color(colDown, colDown, colDown);
	t.vertexUV(bx0, by0, bz1, uLid0, vLid1);
	t.vertexUV(bx0, by0, bz0, uLid0, vLid0);
	t.vertexUV(bx1, by0, bz0, uLid1, vLid0);
	t.vertexUV(bx1, by0, bz1, uLid1, vLid1);

	// North (Z-) & inside South
	t.color(colNorthSouth, colNorthSouth, colNorthSouth);
	t.vertexUV(bx0, by1, bz0, uSide1, vSide0);
	t.vertexUV(bx1, by1, bz0, uSide0, vSide0);
	t.vertexUV(bx1, by0, bz0, uSide0, vSide1);
	t.vertexUV(bx0, by0, bz0, uSide1, vSide1);
	// Interior face
	t.vertexUV(bx1, by1, bz0, uSide0, vSide0);
	t.vertexUV(bx0, by1, bz0, uSide1, vSide0);
	t.vertexUV(bx0, by0, bz0, uSide1, vSide1);
	t.vertexUV(bx1, by0, bz0, uSide0, vSide1);

	// South (Z+) & inside North
	t.vertexUV(bx0, by1, bz1, uSide0, vSide0);
	t.vertexUV(bx0, by0, bz1, uSide0, vSide1);
	t.vertexUV(bx1, by0, bz1, uSide1, vSide1);
	t.vertexUV(bx1, by1, bz1, uSide1, vSide0);
	// Interior face
	t.vertexUV(bx1, by1, bz1, uSide1, vSide0);
	t.vertexUV(bx1, by0, bz1, uSide1, vSide1);
	t.vertexUV(bx0, by0, bz1, uSide0, vSide1);
	t.vertexUV(bx0, by1, bz1, uSide0, vSide0);

	// West (X-) & inside East
	t.color(colWestEast, colWestEast, colWestEast);
	t.vertexUV(bx0, by1, bz1, uSide1, vSide0);
	t.vertexUV(bx0, by1, bz0, uSide0, vSide0);
	t.vertexUV(bx0, by0, bz0, uSide0, vSide1);
	t.vertexUV(bx0, by0, bz1, uSide1, vSide1);
	// Interior face
	t.vertexUV(bx0, by1, bz0, uSide0, vSide0);
	t.vertexUV(bx0, by1, bz1, uSide1, vSide0);
	t.vertexUV(bx0, by0, bz1, uSide1, vSide1);
	t.vertexUV(bx0, by0, bz0, uSide0, vSide1);

	// East (X+) & inside West
	t.vertexUV(bx1, by0, bz1, uSide0, vSide1);
	t.vertexUV(bx1, by0, bz0, uSide1, vSide1);
	t.vertexUV(bx1, by1, bz0, uSide1, vSide0);
	t.vertexUV(bx1, by1, bz1, uSide0, vSide0);
	// Interior face
	t.vertexUV(bx1, by0, bz0, uSide1, vSide1);
	t.vertexUV(bx1, by0, bz1, uSide0, vSide1);
	t.vertexUV(bx1, by1, bz1, uSide0, vSide0);
	t.vertexUV(bx1, by1, bz0, uSide1, vSide0);

	// 2. Top Cap (4x2x4)
	float cx0 = xf + 6.0f / 16.0f;
	float cx1 = xf + 10.0f / 16.0f;
	float cz0 = zf + 6.0f / 16.0f;
	float cz1 = zf + 10.0f / 16.0f;
	float cy0 = hanging ? (yf + 8.0f / 16.0f) : (yf + 7.0f / 16.0f);
	float cy1 = hanging ? (yf + 10.0f / 16.0f) : (yf + 9.0f / 16.0f);

	// Cap Up
	t.color(colUp, colUp, colUp);
	t.vertexUV(cx0, cy1, cz0, uCapTop0, vCapTop0);
	t.vertexUV(cx0, cy1, cz1, uCapTop0, vCapTop1);
	t.vertexUV(cx1, cy1, cz1, uCapTop1, vCapTop1);
	t.vertexUV(cx1, cy1, cz0, uCapTop1, vCapTop0);

	// Cap North
	t.color(colNorthSouth, colNorthSouth, colNorthSouth);
	t.vertexUV(cx0, cy1, cz0, uCap1, vCap0);
	t.vertexUV(cx1, cy1, cz0, uCap0, vCap0);
	t.vertexUV(cx1, cy0, cz0, uCap0, vCap1);
	t.vertexUV(cx0, cy0, cz0, uCap1, vCap1);

	// Cap South
	t.vertexUV(cx0, cy1, cz1, uCap0, vCap0);
	t.vertexUV(cx0, cy0, cz1, uCap0, vCap1);
	t.vertexUV(cx1, cy0, cz1, uCap1, vCap1);
	t.vertexUV(cx1, cy1, cz1, uCap1, vCap0);

	// Cap West
	t.color(colWestEast, colWestEast, colWestEast);
	t.vertexUV(cx0, cy1, cz1, uCap1, vCap0);
	t.vertexUV(cx0, cy1, cz0, uCap0, vCap0);
	t.vertexUV(cx0, cy0, cz0, uCap0, vCap1);
	t.vertexUV(cx0, cy0, cz1, uCap1, vCap1);

	// Cap East
	t.vertexUV(cx1, cy0, cz1, uCap0, vCap1);
	t.vertexUV(cx1, cy0, cz0, uCap1, vCap1);
	t.vertexUV(cx1, cy1, cz0, uCap1, vCap0);
	t.vertexUV(cx1, cy1, cz1, uCap0, vCap0);

	// 3. Handle / Chain Link
	t.color(colNorthSouth, colNorthSouth, colNorthSouth);
	if (hanging) {
		float hx0 = xf + 6.5f / 16.0f;
		float hx1 = xf + 9.5f / 16.0f;
		float hz0 = zf + 6.5f / 16.0f;
		float hz1 = zf + 9.5f / 16.0f;
		float hy0 = yf + 10.0f / 16.0f;
		float hy1 = yf + 16.0f / 16.0f;
		float hxMid = xf + 8.0f / 16.0f;
		float hzMid = zf + 8.0f / 16.0f;

		// Quad along X at z = hzMid (double sided)
		t.vertexUV(hx0, hy1, hzMid, uChain0, vChain0);
		t.vertexUV(hx1, hy1, hzMid, uChain1, vChain0);
		t.vertexUV(hx1, hy0, hzMid, uChain1, vChain1);
		t.vertexUV(hx0, hy0, hzMid, uChain0, vChain1);

		t.vertexUV(hx1, hy1, hzMid, uChain1, vChain0);
		t.vertexUV(hx0, hy1, hzMid, uChain0, vChain0);
		t.vertexUV(hx0, hy0, hzMid, uChain0, vChain1);
		t.vertexUV(hx1, hy0, hzMid, uChain1, vChain1);

		// Quad along Z at x = hxMid (double sided)
		t.vertexUV(hxMid, hy1, hz0, uChain0, vChain0);
		t.vertexUV(hxMid, hy1, hz1, uChain1, vChain0);
		t.vertexUV(hxMid, hy0, hz1, uChain1, vChain1);
		t.vertexUV(hxMid, hy0, hz0, uChain0, vChain1);

		t.vertexUV(hxMid, hy1, hz1, uChain1, vChain0);
		t.vertexUV(hxMid, hy1, hz0, uChain0, vChain0);
		t.vertexUV(hxMid, hy0, hz0, uChain0, vChain1);
		t.vertexUV(hxMid, hy0, hz1, uChain1, vChain1);
	} else {
		float rx0 = xf + 6.5f / 16.0f;
		float rx1 = xf + 9.5f / 16.0f;
		float rz0 = zf + 6.5f / 16.0f;
		float rz1 = zf + 9.5f / 16.0f;
		float ry0 = yf + 9.0f / 16.0f;
		float ry1 = yf + 13.0f / 16.0f;
		float rxMid = xf + 8.0f / 16.0f;
		float rzMid = zf + 8.0f / 16.0f;

		// Quad along X at z = rzMid (double sided)
		t.vertexUV(rx0, ry1, rzMid, uRing0, vRing0);
		t.vertexUV(rx1, ry1, rzMid, uRing1, vRing0);
		t.vertexUV(rx1, ry0, rzMid, uRing1, vRing1);
		t.vertexUV(rx0, ry0, rzMid, uRing0, vRing1);

		t.vertexUV(rx1, ry1, rzMid, uRing1, vRing0);
		t.vertexUV(rx0, ry1, rzMid, uRing0, vRing0);
		t.vertexUV(rx0, ry0, rzMid, uRing0, vRing1);
		t.vertexUV(rx1, ry0, rzMid, uRing1, vRing1);

		// Quad along Z at x = rxMid (double sided)
		t.vertexUV(rxMid, ry1, rz0, uRing0, vRing0);
		t.vertexUV(rxMid, ry1, rz1, uRing1, vRing0);
		t.vertexUV(rxMid, ry0, rz1, uRing1, vRing1);
		t.vertexUV(rxMid, ry0, rz0, uRing0, vRing1);

		t.vertexUV(rxMid, ry1, rz1, uRing1, vRing0);
		t.vertexUV(rxMid, ry1, rz0, uRing0, vRing0);
		t.vertexUV(rxMid, ry0, rz0, uRing0, vRing1);
		t.vertexUV(rxMid, ry0, rz1, uRing1, vRing1);
	}

	tt->setShape(5.0f / 16.0f, 0.0f, 5.0f / 16.0f, 11.0f / 16.0f, 9.0f / 16.0f, 11.0f / 16.0f);
	return true;
}

bool TileRenderer::tesselateCampfireInWorld(Tile* tt, int x, int y, int z) {
	Tesselator& t = Tesselator::instance;
	float br = tt->getBrightness(level, x, y, z);
	if (Tile::lightEmission[tt->id] > 0) br = 1.0f;

	const int TEX_LOG = 118;
	const int TEX_FIRE = 109;
	const int TEX_LIT = 110;

	float colUp = 1.0f * br;
	float colDown = 0.5f * br;
	float colNS = 0.8f * br;
	float colWE = 0.6f * br;

	float xf = (float)x;
	float yf = (float)y;
	float zf = (float)z;

	// 1. Bottom Left Log (West Log: x: 1..5, y: 0..4, z: 0..16)
	float lx0 = xf + 1.0f / 16.0f, lx1 = xf + 5.0f / 16.0f;
	float ly0 = yf + 0.0f, ly1 = yf + 4.0f / 16.0f;
	float lz0 = zf + 0.0f, lz1 = zf + 1.0f;

	// Up (rot 90)
	t.color(colUp, colUp, colUp);
	t.vertexUV(lx0, ly1, lz0, getAtlasU(TEX_LOG, 0), getAtlasV(TEX_LOG, 4));
	t.vertexUV(lx0, ly1, lz1, getAtlasU(TEX_LOG, 16), getAtlasV(TEX_LOG, 4));
	t.vertexUV(lx1, ly1, lz1, getAtlasU(TEX_LOG, 16), getAtlasV(TEX_LOG, 0));
	t.vertexUV(lx1, ly1, lz0, getAtlasU(TEX_LOG, 0), getAtlasV(TEX_LOG, 0));

	// Down (rot 90)
	t.color(colDown, colDown, colDown);
	t.vertexUV(lx0, ly0, lz1, getAtlasU(TEX_LOG, 16), getAtlasV(TEX_LOG, 0));
	t.vertexUV(lx0, ly0, lz0, getAtlasU(TEX_LOG, 0), getAtlasV(TEX_LOG, 0));
	t.vertexUV(lx1, ly0, lz0, getAtlasU(TEX_LOG, 0), getAtlasV(TEX_LOG, 4));
	t.vertexUV(lx1, ly0, lz1, getAtlasU(TEX_LOG, 16), getAtlasV(TEX_LOG, 4));

	// North (end cut)
	t.color(colNS, colNS, colNS);
	t.vertexUV(lx0, ly1, lz0, getAtlasU(TEX_LOG, 0), getAtlasV(TEX_LOG, 4));
	t.vertexUV(lx1, ly1, lz0, getAtlasU(TEX_LOG, 4), getAtlasV(TEX_LOG, 4));
	t.vertexUV(lx1, ly0, lz0, getAtlasU(TEX_LOG, 4), getAtlasV(TEX_LOG, 8));
	t.vertexUV(lx0, ly0, lz0, getAtlasU(TEX_LOG, 0), getAtlasV(TEX_LOG, 8));

	// South (end cut)
	t.vertexUV(lx0, ly1, lz1, getAtlasU(TEX_LOG, 4), getAtlasV(TEX_LOG, 4));
	t.vertexUV(lx0, ly0, lz1, getAtlasU(TEX_LOG, 4), getAtlasV(TEX_LOG, 8));
	t.vertexUV(lx1, ly0, lz1, getAtlasU(TEX_LOG, 0), getAtlasV(TEX_LOG, 8));
	t.vertexUV(lx1, ly1, lz1, getAtlasU(TEX_LOG, 0), getAtlasV(TEX_LOG, 4));

	// West (bark)
	t.color(colWE, colWE, colWE);
	t.vertexUV(lx0, ly1, lz1, getAtlasU(TEX_LOG, 0), getAtlasV(TEX_LOG, 0));
	t.vertexUV(lx0, ly1, lz0, getAtlasU(TEX_LOG, 16), getAtlasV(TEX_LOG, 0));
	t.vertexUV(lx0, ly0, lz0, getAtlasU(TEX_LOG, 16), getAtlasV(TEX_LOG, 4));
	t.vertexUV(lx0, ly0, lz1, getAtlasU(TEX_LOG, 0), getAtlasV(TEX_LOG, 4));

	// East (lit log inner)
	t.vertexUV(lx1, ly1, lz0, getAtlasU(TEX_LIT, 16), getAtlasV(TEX_LIT, 1));
	t.vertexUV(lx1, ly1, lz1, getAtlasU(TEX_LIT, 0), getAtlasV(TEX_LIT, 1));
	t.vertexUV(lx1, ly0, lz1, getAtlasU(TEX_LIT, 0), getAtlasV(TEX_LIT, 5));
	t.vertexUV(lx1, ly0, lz0, getAtlasU(TEX_LIT, 16), getAtlasV(TEX_LIT, 5));

	// 2. Bottom Right Log (East Log: x: 11..15, y: 0..4, z: 0..16)
	float rx0 = xf + 11.0f / 16.0f, rx1 = xf + 15.0f / 16.0f;
	float ry0 = yf + 0.0f, ry1 = yf + 4.0f / 16.0f;
	float rz0 = zf + 0.0f, rz1 = zf + 1.0f;

	// Up (rot 90)
	t.color(colUp, colUp, colUp);
	t.vertexUV(rx0, ry1, rz0, getAtlasU(TEX_LOG, 0), getAtlasV(TEX_LOG, 4));
	t.vertexUV(rx0, ry1, rz1, getAtlasU(TEX_LOG, 16), getAtlasV(TEX_LOG, 4));
	t.vertexUV(rx1, ry1, rz1, getAtlasU(TEX_LOG, 16), getAtlasV(TEX_LOG, 0));
	t.vertexUV(rx1, ry1, rz0, getAtlasU(TEX_LOG, 0), getAtlasV(TEX_LOG, 0));

	// Down (rot 90)
	t.color(colDown, colDown, colDown);
	t.vertexUV(rx0, ry0, rz1, getAtlasU(TEX_LOG, 16), getAtlasV(TEX_LOG, 0));
	t.vertexUV(rx0, ry0, rz0, getAtlasU(TEX_LOG, 0), getAtlasV(TEX_LOG, 0));
	t.vertexUV(rx1, ry0, rz0, getAtlasU(TEX_LOG, 0), getAtlasV(TEX_LOG, 4));
	t.vertexUV(rx1, ry0, rz1, getAtlasU(TEX_LOG, 16), getAtlasV(TEX_LOG, 4));

	// North (end cut)
	t.color(colNS, colNS, colNS);
	t.vertexUV(rx0, ry1, rz0, getAtlasU(TEX_LOG, 0), getAtlasV(TEX_LOG, 4));
	t.vertexUV(rx1, ry1, rz0, getAtlasU(TEX_LOG, 4), getAtlasV(TEX_LOG, 4));
	t.vertexUV(rx1, ry0, rz0, getAtlasU(TEX_LOG, 4), getAtlasV(TEX_LOG, 8));
	t.vertexUV(rx0, ry0, rz0, getAtlasU(TEX_LOG, 0), getAtlasV(TEX_LOG, 8));

	// South (end cut)
	t.vertexUV(rx0, ry1, rz1, getAtlasU(TEX_LOG, 4), getAtlasV(TEX_LOG, 4));
	t.vertexUV(rx0, ry0, rz1, getAtlasU(TEX_LOG, 4), getAtlasV(TEX_LOG, 8));
	t.vertexUV(rx1, ry0, rz1, getAtlasU(TEX_LOG, 0), getAtlasV(TEX_LOG, 8));
	t.vertexUV(rx1, ry1, rz1, getAtlasU(TEX_LOG, 0), getAtlasV(TEX_LOG, 4));

	// West (lit log inner)
	t.color(colWE, colWE, colWE);
	t.vertexUV(rx0, ry1, rz1, getAtlasU(TEX_LIT, 0), getAtlasV(TEX_LIT, 1));
	t.vertexUV(rx0, ry1, rz0, getAtlasU(TEX_LIT, 16), getAtlasV(TEX_LIT, 1));
	t.vertexUV(rx0, ry0, rz0, getAtlasU(TEX_LIT, 16), getAtlasV(TEX_LIT, 5));
	t.vertexUV(rx0, ry0, rz1, getAtlasU(TEX_LIT, 0), getAtlasV(TEX_LIT, 5));

	// East (bark)
	t.vertexUV(rx1, ry1, rz0, getAtlasU(TEX_LOG, 16), getAtlasV(TEX_LOG, 0));
	t.vertexUV(rx1, ry1, rz1, getAtlasU(TEX_LOG, 0), getAtlasV(TEX_LOG, 0));
	t.vertexUV(rx1, ry0, rz1, getAtlasU(TEX_LOG, 0), getAtlasV(TEX_LOG, 4));
	t.vertexUV(rx1, ry0, rz0, getAtlasU(TEX_LOG, 16), getAtlasV(TEX_LOG, 4));

	// 3. Top North Log (x: 0..16, y: 3..7, z: 1..5)
	float nx0 = xf + 0.0f, nx1 = xf + 1.0f;
	float ny0 = yf + 3.0f / 16.0f, ny1 = yf + 7.0f / 16.0f;
	float nz0 = zf + 1.0f / 16.0f, nz1 = zf + 5.0f / 16.0f;

	// Up (rot 180)
	t.color(colUp, colUp, colUp);
	t.vertexUV(nx0, ny1, nz0, getAtlasU(TEX_LOG, 16), getAtlasV(TEX_LOG, 4));
	t.vertexUV(nx0, ny1, nz1, getAtlasU(TEX_LOG, 16), getAtlasV(TEX_LOG, 0));
	t.vertexUV(nx1, ny1, nz1, getAtlasU(TEX_LOG, 0), getAtlasV(TEX_LOG, 0));
	t.vertexUV(nx1, ny1, nz0, getAtlasU(TEX_LOG, 0), getAtlasV(TEX_LOG, 4));

	// Down
	t.color(colDown, colDown, colDown);
	t.vertexUV(nx0, ny0, nz1, getAtlasU(TEX_LIT, 0), getAtlasV(TEX_LIT, 4));
	t.vertexUV(nx0, ny0, nz0, getAtlasU(TEX_LIT, 0), getAtlasV(TEX_LIT, 8));
	t.vertexUV(nx1, ny0, nz0, getAtlasU(TEX_LIT, 16), getAtlasV(TEX_LIT, 8));
	t.vertexUV(nx1, ny0, nz1, getAtlasU(TEX_LIT, 16), getAtlasV(TEX_LIT, 4));

	// West (end cut)
	t.color(colWE, colWE, colWE);
	t.vertexUV(nx0, ny1, nz1, getAtlasU(TEX_LOG, 4), getAtlasV(TEX_LOG, 4));
	t.vertexUV(nx0, ny1, nz0, getAtlasU(TEX_LOG, 0), getAtlasV(TEX_LOG, 4));
	t.vertexUV(nx0, ny0, nz0, getAtlasU(TEX_LOG, 0), getAtlasV(TEX_LOG, 8));
	t.vertexUV(nx0, ny0, nz1, getAtlasU(TEX_LOG, 4), getAtlasV(TEX_LOG, 8));

	// East (end cut)
	t.vertexUV(nx1, ny1, nz0, getAtlasU(TEX_LOG, 4), getAtlasV(TEX_LOG, 4));
	t.vertexUV(nx1, ny1, nz1, getAtlasU(TEX_LOG, 0), getAtlasV(TEX_LOG, 4));
	t.vertexUV(nx1, ny0, nz1, getAtlasU(TEX_LOG, 0), getAtlasV(TEX_LOG, 8));
	t.vertexUV(nx1, ny0, nz0, getAtlasU(TEX_LOG, 4), getAtlasV(TEX_LOG, 8));

	// North (outer lit side)
	t.color(colNS, colNS, colNS);
	t.vertexUV(nx0, ny1, nz0, getAtlasU(TEX_LIT, 0), getAtlasV(TEX_LIT, 0));
	t.vertexUV(nx1, ny1, nz0, getAtlasU(TEX_LIT, 16), getAtlasV(TEX_LIT, 0));
	t.vertexUV(nx1, ny0, nz0, getAtlasU(TEX_LIT, 16), getAtlasV(TEX_LIT, 4));
	t.vertexUV(nx0, ny0, nz0, getAtlasU(TEX_LIT, 0), getAtlasV(TEX_LIT, 4));

	// South (inner lit side)
	t.vertexUV(nx0, ny1, nz1, getAtlasU(TEX_LIT, 0), getAtlasV(TEX_LIT, 0));
	t.vertexUV(nx0, ny0, nz1, getAtlasU(TEX_LIT, 0), getAtlasV(TEX_LIT, 4));
	t.vertexUV(nx1, ny0, nz1, getAtlasU(TEX_LIT, 16), getAtlasV(TEX_LIT, 4));
	t.vertexUV(nx1, ny1, nz1, getAtlasU(TEX_LIT, 16), getAtlasV(TEX_LIT, 0));

	// 4. Top South Log (x: 0..16, y: 3..7, z: 11..15)
	float sx0 = xf + 0.0f, sx1 = xf + 1.0f;
	float sy0 = yf + 3.0f / 16.0f, sy1 = yf + 7.0f / 16.0f;
	float sz0 = zf + 11.0f / 16.0f, sz1 = zf + 15.0f / 16.0f;

	// Up (rot 180)
	t.color(colUp, colUp, colUp);
	t.vertexUV(sx0, sy1, sz0, getAtlasU(TEX_LOG, 16), getAtlasV(TEX_LOG, 4));
	t.vertexUV(sx0, sy1, sz1, getAtlasU(TEX_LOG, 16), getAtlasV(TEX_LOG, 0));
	t.vertexUV(sx1, sy1, sz1, getAtlasU(TEX_LOG, 0), getAtlasV(TEX_LOG, 0));
	t.vertexUV(sx1, sy1, sz0, getAtlasU(TEX_LOG, 0), getAtlasV(TEX_LOG, 4));

	// Down
	t.color(colDown, colDown, colDown);
	t.vertexUV(sx0, sy0, sz1, getAtlasU(TEX_LIT, 0), getAtlasV(TEX_LIT, 4));
	t.vertexUV(sx0, sy0, sz0, getAtlasU(TEX_LIT, 0), getAtlasV(TEX_LIT, 8));
	t.vertexUV(sx1, sy0, sz0, getAtlasU(TEX_LIT, 16), getAtlasV(TEX_LIT, 8));
	t.vertexUV(sx1, sy0, sz1, getAtlasU(TEX_LIT, 16), getAtlasV(TEX_LIT, 4));

	// West (end cut)
	t.color(colWE, colWE, colWE);
	t.vertexUV(sx0, sy1, sz1, getAtlasU(TEX_LOG, 4), getAtlasV(TEX_LOG, 4));
	t.vertexUV(sx0, sy1, sz0, getAtlasU(TEX_LOG, 0), getAtlasV(TEX_LOG, 4));
	t.vertexUV(sx0, sy0, sz0, getAtlasU(TEX_LOG, 0), getAtlasV(TEX_LOG, 8));
	t.vertexUV(sx0, sy0, sz1, getAtlasU(TEX_LOG, 4), getAtlasV(TEX_LOG, 8));

	// East (end cut)
	t.vertexUV(sx1, sy1, sz0, getAtlasU(TEX_LOG, 4), getAtlasV(TEX_LOG, 4));
	t.vertexUV(sx1, sy1, sz1, getAtlasU(TEX_LOG, 0), getAtlasV(TEX_LOG, 4));
	t.vertexUV(sx1, sy0, sz1, getAtlasU(TEX_LOG, 0), getAtlasV(TEX_LOG, 8));
	t.vertexUV(sx1, sy0, sz0, getAtlasU(TEX_LOG, 4), getAtlasV(TEX_LOG, 8));

	// North (inner lit side)
	t.color(colNS, colNS, colNS);
	t.vertexUV(sx0, sy1, sz0, getAtlasU(TEX_LIT, 16), getAtlasV(TEX_LIT, 0));
	t.vertexUV(sx1, sy1, sz0, getAtlasU(TEX_LIT, 0), getAtlasV(TEX_LIT, 0));
	t.vertexUV(sx1, sy0, sz0, getAtlasU(TEX_LIT, 0), getAtlasV(TEX_LIT, 4));
	t.vertexUV(sx0, sy0, sz0, getAtlasU(TEX_LIT, 16), getAtlasV(TEX_LIT, 4));

	// South (outer lit side)
	t.vertexUV(sx0, sy1, sz1, getAtlasU(TEX_LIT, 16), getAtlasV(TEX_LIT, 0));
	t.vertexUV(sx0, sy0, sz1, getAtlasU(TEX_LIT, 16), getAtlasV(TEX_LIT, 4));
	t.vertexUV(sx1, sy0, sz1, getAtlasU(TEX_LIT, 0), getAtlasV(TEX_LIT, 4));
	t.vertexUV(sx1, sy1, sz1, getAtlasU(TEX_LIT, 0), getAtlasV(TEX_LIT, 0));

	// 5. Coals / Ash bed (x: 5..11, y: 0..1, z: 0..16)
	float ax0 = xf + 5.0f / 16.0f, ax1 = xf + 11.0f / 16.0f;
	float ay0 = yf + 0.0f, ay1 = yf + 1.0f / 16.0f;
	float az0 = zf + 0.0f, az1 = zf + 1.0f;

	// Up (ash coals glowing)
	t.color(colUp, colUp, colUp);
	t.vertexUV(ax0, ay1, az0, getAtlasU(TEX_LIT, 0), getAtlasV(TEX_LIT, 14));
	t.vertexUV(ax0, ay1, az1, getAtlasU(TEX_LIT, 16), getAtlasV(TEX_LIT, 14));
	t.vertexUV(ax1, ay1, az1, getAtlasU(TEX_LIT, 16), getAtlasV(TEX_LIT, 8));
	t.vertexUV(ax1, ay1, az0, getAtlasU(TEX_LIT, 0), getAtlasV(TEX_LIT, 8));

	// Down
	t.color(colDown, colDown, colDown);
	t.vertexUV(ax0, ay0, az1, getAtlasU(TEX_LOG, 16), getAtlasV(TEX_LOG, 8));
	t.vertexUV(ax0, ay0, az0, getAtlasU(TEX_LOG, 0), getAtlasV(TEX_LOG, 8));
	t.vertexUV(ax1, ay0, az0, getAtlasU(TEX_LOG, 0), getAtlasV(TEX_LOG, 14));
	t.vertexUV(ax1, ay0, az1, getAtlasU(TEX_LOG, 16), getAtlasV(TEX_LOG, 14));

	// North ash edge
	t.color(colNS, colNS, colNS);
	t.vertexUV(ax0, ay1, az0, getAtlasU(TEX_LOG, 0), getAtlasV(TEX_LOG, 15));
	t.vertexUV(ax1, ay1, az0, getAtlasU(TEX_LOG, 6), getAtlasV(TEX_LOG, 15));
	t.vertexUV(ax1, ay0, az0, getAtlasU(TEX_LOG, 6), getAtlasV(TEX_LOG, 16));
	t.vertexUV(ax0, ay0, az0, getAtlasU(TEX_LOG, 0), getAtlasV(TEX_LOG, 16));

	// South ash edge
	t.vertexUV(ax0, ay1, az1, getAtlasU(TEX_LOG, 16), getAtlasV(TEX_LOG, 15));
	t.vertexUV(ax0, ay0, az1, getAtlasU(TEX_LOG, 16), getAtlasV(TEX_LOG, 16));
	t.vertexUV(ax1, ay0, az1, getAtlasU(TEX_LOG, 10), getAtlasV(TEX_LOG, 16));
	t.vertexUV(ax1, ay1, az1, getAtlasU(TEX_LOG, 10), getAtlasV(TEX_LOG, 15));

	// 6. Fire Cross Planes (Full Brightness 1.0f)
	t.color(1.0f, 1.0f, 1.0f);
	float fx0 = xf + 0.8f / 16.0f, fx1 = xf + 15.2f / 16.0f;
	float fz0 = zf + 0.8f / 16.0f, fz1 = zf + 15.2f / 16.0f;
	float fy0 = yf + 1.0f / 16.0f, fy1 = yf + 17.0f / 16.0f;

	float fu0 = getAtlasU(TEX_FIRE, 0), fu1 = getAtlasU(TEX_FIRE, 16);
	float fv0 = getAtlasV(TEX_FIRE, 0), fv1 = getAtlasV(TEX_FIRE, 16);

	// Diagonal 1
	t.vertexUV(fx0, fy1, fz0, fu0, fv0);
	t.vertexUV(fx0, fy0, fz0, fu0, fv1);
	t.vertexUV(fx1, fy0, fz1, fu1, fv1);
	t.vertexUV(fx1, fy1, fz1, fu1, fv0);

	t.vertexUV(fx1, fy1, fz1, fu1, fv0);
	t.vertexUV(fx1, fy0, fz1, fu1, fv1);
	t.vertexUV(fx0, fy0, fz0, fu0, fv1);
	t.vertexUV(fx0, fy1, fz0, fu0, fv0);

	// Diagonal 2
	t.vertexUV(fx0, fy1, fz1, fu0, fv0);
	t.vertexUV(fx0, fy0, fz1, fu0, fv1);
	t.vertexUV(fx1, fy0, fz0, fu1, fv1);
	t.vertexUV(fx1, fy1, fz0, fu1, fv0);

	t.vertexUV(fx1, fy1, fz0, fu1, fv0);
	t.vertexUV(fx1, fy0, fz0, fu1, fv1);
	t.vertexUV(fx0, fy0, fz1, fu0, fv1);
	t.vertexUV(fx0, fy1, fz1, fu0, fv0);

	tt->setShape(0.0f, 0.0f, 0.0f, 1.0f, 7.0f / 16.0f, 1.0f);
	return true;
}

bool TileRenderer::tesselateGrindstoneInWorld(Tile* tt, int x, int y, int z) {
	float xf = (float)x;
	float yf = (float)y;
	float zf = (float)z;
	Tesselator& t = Tesselator::instance;
	float br = tt->getBrightness(level, x, y, z);

	// Left and Right legs (wood / pivot 151)
	tt->setShape(2.0f / 16.0f, 0.0f, 6.0f / 16.0f, 4.0f / 16.0f, 7.0f / 16.0f, 10.0f / 16.0f);
	tesselateBlockInWorld(tt, x, y, z);

	tt->setShape(12.0f / 16.0f, 0.0f, 6.0f / 16.0f, 14.0f / 16.0f, 7.0f / 16.0f, 10.0f / 16.0f);
	tesselateBlockInWorld(tt, x, y, z);

	// Left and Right pivot brackets (grindstone_pivot 151)
	tt->setShape(2.0f / 16.0f, 7.0f / 16.0f, 5.0f / 16.0f, 4.0f / 16.0f, 13.0f / 16.0f, 11.0f / 16.0f);
	tesselateBlockInWorld(tt, x, y, z);

	tt->setShape(12.0f / 16.0f, 7.0f / 16.0f, 5.0f / 16.0f, 14.0f / 16.0f, 13.0f / 16.0f, 11.0f / 16.0f);
	tesselateBlockInWorld(tt, x, y, z);

	// Central circular stone wheel:
	// Up, Down, North, South use grindstone_round (17)
	// West, East use grindstone_side (21)
	tt->setShape(4.0f / 16.0f, 4.0f / 16.0f, 2.0f / 16.0f, 12.0f / 16.0f, 1.0f, 14.0f / 16.0f);
	t.color(1.0f * br, 1.0f * br, 1.0f * br);
	renderFaceUp(tt, xf, yf, zf, 17);
	t.color(0.5f * br, 0.5f * br, 0.5f * br);
	renderFaceDown(tt, xf, yf, zf, 17);
	t.color(0.8f * br, 0.8f * br, 0.8f * br);
	renderNorth(tt, xf, yf, zf, 17);
	renderSouth(tt, xf, yf, zf, 17);
	t.color(0.6f * br, 0.6f * br, 0.6f * br);
	renderWest(tt, xf, yf, zf, 21);
	renderEast(tt, xf, yf, zf, 21);

	tt->setShape(2.0f / 16.0f, 0.0f, 2.0f / 16.0f, 14.0f / 16.0f, 1.0f, 14.0f / 16.0f);
	return true;
}

bool TileRenderer::tesselateLecternInWorld(Tile* tt, int x, int y, int z) {
	Tesselator& t = Tesselator::instance;
	float br = tt->getBrightness(level, x, y, z);

	const int TEX_BASE = 162;
	const int TEX_FRONT = 22;
	const int TEX_SIDES = 106;
	const int TEX_TOP = 107;

	float colUp = 1.0f * br;
	float colDown = 0.5f * br;
	float colNS = 0.8f * br;
	float colWE = 0.6f * br;

	float xf = (float)x;
	float yf = (float)y;
	float zf = (float)z;

	// 1. Base pedestal (x: 0..1, y: 0..2/16, z: 0..1)
	float bx0 = xf + 0.0f, bx1 = xf + 1.0f;
	float by0 = yf + 0.0f, by1 = yf + 2.0f / 16.0f;
	float bz0 = zf + 0.0f, bz1 = zf + 1.0f;

	// Base Up
	t.color(colUp, colUp, colUp);
	t.vertexUV(bx0, by1, bz0, getAtlasU(TEX_BASE, 0), getAtlasV(TEX_BASE, 0));
	t.vertexUV(bx0, by1, bz1, getAtlasU(TEX_BASE, 0), getAtlasV(TEX_BASE, 16));
	t.vertexUV(bx1, by1, bz1, getAtlasU(TEX_BASE, 16), getAtlasV(TEX_BASE, 16));
	t.vertexUV(bx1, by1, bz0, getAtlasU(TEX_BASE, 16), getAtlasV(TEX_BASE, 0));

	// Base Down
	t.color(colDown, colDown, colDown);
	t.vertexUV(bx0, by0, bz1, getAtlasU(TEX_BASE, 0), getAtlasV(TEX_BASE, 16));
	t.vertexUV(bx0, by0, bz0, getAtlasU(TEX_BASE, 0), getAtlasV(TEX_BASE, 0));
	t.vertexUV(bx1, by0, bz0, getAtlasU(TEX_BASE, 16), getAtlasV(TEX_BASE, 0));
	t.vertexUV(bx1, by0, bz1, getAtlasU(TEX_BASE, 16), getAtlasV(TEX_BASE, 16));

	// Base North ([0, 14, 16, 16])
	t.color(colNS, colNS, colNS);
	t.vertexUV(bx0, by1, bz0, getAtlasU(TEX_BASE, 0), getAtlasV(TEX_BASE, 14));
	t.vertexUV(bx1, by1, bz0, getAtlasU(TEX_BASE, 16), getAtlasV(TEX_BASE, 14));
	t.vertexUV(bx1, by0, bz0, getAtlasU(TEX_BASE, 16), getAtlasV(TEX_BASE, 16));
	t.vertexUV(bx0, by0, bz0, getAtlasU(TEX_BASE, 0), getAtlasV(TEX_BASE, 16));

	// Base South ([0, 6, 16, 8])
	t.vertexUV(bx0, by1, bz1, getAtlasU(TEX_BASE, 0), getAtlasV(TEX_BASE, 6));
	t.vertexUV(bx0, by0, bz1, getAtlasU(TEX_BASE, 0), getAtlasV(TEX_BASE, 8));
	t.vertexUV(bx1, by0, bz1, getAtlasU(TEX_BASE, 16), getAtlasV(TEX_BASE, 8));
	t.vertexUV(bx1, by1, bz1, getAtlasU(TEX_BASE, 16), getAtlasV(TEX_BASE, 6));

	// Base West ([0, 6, 16, 8])
	t.color(colWE, colWE, colWE);
	t.vertexUV(bx0, by1, bz1, getAtlasU(TEX_BASE, 16), getAtlasV(TEX_BASE, 6));
	t.vertexUV(bx0, by1, bz0, getAtlasU(TEX_BASE, 0), getAtlasV(TEX_BASE, 6));
	t.vertexUV(bx0, by0, bz0, getAtlasU(TEX_BASE, 0), getAtlasV(TEX_BASE, 8));
	t.vertexUV(bx0, by0, bz1, getAtlasU(TEX_BASE, 16), getAtlasV(TEX_BASE, 8));

	// Base East ([0, 6, 16, 8])
	t.vertexUV(bx1, by1, bz0, getAtlasU(TEX_BASE, 0), getAtlasV(TEX_BASE, 6));
	t.vertexUV(bx1, by1, bz1, getAtlasU(TEX_BASE, 16), getAtlasV(TEX_BASE, 6));
	t.vertexUV(bx1, by0, bz1, getAtlasU(TEX_BASE, 16), getAtlasV(TEX_BASE, 8));
	t.vertexUV(bx1, by0, bz0, getAtlasU(TEX_BASE, 0), getAtlasV(TEX_BASE, 8));

	// 2. Central pillar (x: 4..12, y: 2..14, z: 4..12)
	float px0 = xf + 4.0f / 16.0f, px1 = xf + 12.0f / 16.0f;
	float py0 = yf + 2.0f / 16.0f, py1 = yf + 14.0f / 16.0f;
	float pz0 = zf + 4.0f / 16.0f, pz1 = zf + 12.0f / 16.0f;

	// Pillar North (front open face: [0, 0, 8, 12])
	t.color(colNS, colNS, colNS);
	t.vertexUV(px0, py1, pz0, getAtlasU(TEX_FRONT, 0), getAtlasV(TEX_FRONT, 0));
	t.vertexUV(px1, py1, pz0, getAtlasU(TEX_FRONT, 8), getAtlasV(TEX_FRONT, 0));
	t.vertexUV(px1, py0, pz0, getAtlasU(TEX_FRONT, 8), getAtlasV(TEX_FRONT, 12));
	t.vertexUV(px0, py0, pz0, getAtlasU(TEX_FRONT, 0), getAtlasV(TEX_FRONT, 12));

	// Pillar South (back face: [8, 4, 16, 16])
	t.vertexUV(px0, py1, pz1, getAtlasU(TEX_FRONT, 8), getAtlasV(TEX_FRONT, 4));
	t.vertexUV(px0, py0, pz1, getAtlasU(TEX_FRONT, 8), getAtlasV(TEX_FRONT, 16));
	t.vertexUV(px1, py0, pz1, getAtlasU(TEX_FRONT, 16), getAtlasV(TEX_FRONT, 16));
	t.vertexUV(px1, py1, pz1, getAtlasU(TEX_FRONT, 16), getAtlasV(TEX_FRONT, 4));

	// Pillar West (side face: [0, 2, 8, 14])
	t.color(colWE, colWE, colWE);
	t.vertexUV(px0, py1, pz1, getAtlasU(TEX_SIDES, 8), getAtlasV(TEX_SIDES, 2));
	t.vertexUV(px0, py1, pz0, getAtlasU(TEX_SIDES, 0), getAtlasV(TEX_SIDES, 2));
	t.vertexUV(px0, py0, pz0, getAtlasU(TEX_SIDES, 0), getAtlasV(TEX_SIDES, 14));
	t.vertexUV(px0, py0, pz1, getAtlasU(TEX_SIDES, 8), getAtlasV(TEX_SIDES, 14));

	// Pillar East (side face: [0, 2, 8, 14])
	t.vertexUV(px1, py1, pz0, getAtlasU(TEX_SIDES, 0), getAtlasV(TEX_SIDES, 2));
	t.vertexUV(px1, py1, pz1, getAtlasU(TEX_SIDES, 8), getAtlasV(TEX_SIDES, 2));
	t.vertexUV(px1, py0, pz1, getAtlasU(TEX_SIDES, 8), getAtlasV(TEX_SIDES, 14));
	t.vertexUV(px1, py0, pz0, getAtlasU(TEX_SIDES, 0), getAtlasV(TEX_SIDES, 14));

	// 3. Top reading desk (x: 0..1, y: 12..15, z: 2..16)
	float dx0 = xf + 0.0f, dx1 = xf + 1.0f;
	float dy0 = yf + 12.0f / 16.0f, dy1 = yf + 15.0f / 16.0f;
	float dz0 = zf + 2.0f / 16.0f, dz1 = zf + 1.0f;

	// Top surface (the book/desk face: [0, 1, 16, 15] on TEX_TOP)
	t.color(colUp, colUp, colUp);
	t.vertexUV(dx0, dy1, dz0, getAtlasU(TEX_TOP, 0), getAtlasV(TEX_TOP, 1));
	t.vertexUV(dx0, dy1, dz1, getAtlasU(TEX_TOP, 0), getAtlasV(TEX_TOP, 15));
	t.vertexUV(dx1, dy1, dz1, getAtlasU(TEX_TOP, 16), getAtlasV(TEX_TOP, 15));
	t.vertexUV(dx1, dy1, dz0, getAtlasU(TEX_TOP, 16), getAtlasV(TEX_TOP, 1));

	// Bottom surface (wood bottom: [0, 0, 16, 14] on TEX_BASE)
	t.color(colDown, colDown, colDown);
	t.vertexUV(dx0, dy0, dz1, getAtlasU(TEX_BASE, 0), getAtlasV(TEX_BASE, 14));
	t.vertexUV(dx0, dy0, dz0, getAtlasU(TEX_BASE, 0), getAtlasV(TEX_BASE, 0));
	t.vertexUV(dx1, dy0, dz0, getAtlasU(TEX_BASE, 16), getAtlasV(TEX_BASE, 0));
	t.vertexUV(dx1, dy0, dz1, getAtlasU(TEX_BASE, 16), getAtlasV(TEX_BASE, 14));

	// Desk North edge: [0, 0, 16, 3] on TEX_SIDES
	t.color(colNS, colNS, colNS);
	t.vertexUV(dx0, dy1, dz0, getAtlasU(TEX_SIDES, 0), getAtlasV(TEX_SIDES, 0));
	t.vertexUV(dx1, dy1, dz0, getAtlasU(TEX_SIDES, 16), getAtlasV(TEX_SIDES, 0));
	t.vertexUV(dx1, dy0, dz0, getAtlasU(TEX_SIDES, 16), getAtlasV(TEX_SIDES, 3));
	t.vertexUV(dx0, dy0, dz0, getAtlasU(TEX_SIDES, 0), getAtlasV(TEX_SIDES, 3));

	// Desk South edge: [0, 4, 16, 7] on TEX_SIDES
	t.vertexUV(dx0, dy1, dz1, getAtlasU(TEX_SIDES, 0), getAtlasV(TEX_SIDES, 4));
	t.vertexUV(dx0, dy0, dz1, getAtlasU(TEX_SIDES, 0), getAtlasV(TEX_SIDES, 7));
	t.vertexUV(dx1, dy0, dz1, getAtlasU(TEX_SIDES, 16), getAtlasV(TEX_SIDES, 7));
	t.vertexUV(dx1, dy1, dz1, getAtlasU(TEX_SIDES, 16), getAtlasV(TEX_SIDES, 4));

	// Desk West edge: [0, 4, 14, 7] on TEX_SIDES
	t.color(colWE, colWE, colWE);
	t.vertexUV(dx0, dy1, dz1, getAtlasU(TEX_SIDES, 14), getAtlasV(TEX_SIDES, 4));
	t.vertexUV(dx0, dy1, dz0, getAtlasU(TEX_SIDES, 0), getAtlasV(TEX_SIDES, 4));
	t.vertexUV(dx0, dy0, dz0, getAtlasU(TEX_SIDES, 0), getAtlasV(TEX_SIDES, 7));
	t.vertexUV(dx0, dy0, dz1, getAtlasU(TEX_SIDES, 14), getAtlasV(TEX_SIDES, 7));

	// Desk East edge: [0, 4, 14, 7] on TEX_SIDES
	t.vertexUV(dx1, dy1, dz0, getAtlasU(TEX_SIDES, 0), getAtlasV(TEX_SIDES, 4));
	t.vertexUV(dx1, dy1, dz1, getAtlasU(TEX_SIDES, 14), getAtlasV(TEX_SIDES, 4));
	t.vertexUV(dx1, dy0, dz1, getAtlasU(TEX_SIDES, 14), getAtlasV(TEX_SIDES, 7));
	t.vertexUV(dx1, dy0, dz0, getAtlasU(TEX_SIDES, 0), getAtlasV(TEX_SIDES, 7));

	tt->setShape(0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f);
	return true;
}

bool TileRenderer::tesselateComposterInWorld(Tile* tt, int x, int y, int z) {
	bool origNoCulling = noCulling;
	noCulling = true;

	// Bottom floor plate
	tt->setShape(2.0f / 16.0f, 0.0f, 2.0f / 16.0f, 14.0f / 16.0f, 2.0f / 16.0f, 14.0f / 16.0f);
	tesselateBlockInWorld(tt, x, y, z);

	// 4 outer and inner wall boxes
	tt->setShape(0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 2.0f / 16.0f);
	tesselateBlockInWorld(tt, x, y, z);

	tt->setShape(0.0f, 0.0f, 14.0f / 16.0f, 1.0f, 1.0f, 1.0f);
	tesselateBlockInWorld(tt, x, y, z);

	tt->setShape(0.0f, 0.0f, 2.0f / 16.0f, 2.0f / 16.0f, 1.0f, 14.0f / 16.0f);
	tesselateBlockInWorld(tt, x, y, z);

	tt->setShape(14.0f / 16.0f, 0.0f, 2.0f / 16.0f, 1.0f, 1.0f, 14.0f / 16.0f);
	tesselateBlockInWorld(tt, x, y, z);

	noCulling = origNoCulling;
	tt->setShape(0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f);
	return true;
}

bool TileRenderer::tesselateStonecutterInWorld(Tile* tt, int x, int y, int z) {
	float xf = (float)x;
	float yf = (float)y;
	float zf = (float)z;
	Tesselator& t = Tesselator::instance;
	float br = tt->getBrightness(level, x, y, z);

	// Base stone table (height 9/16, top = stonecutter_top 251, bottom = 252, sides = 253)
	tt->setShape(0.0f, 0.0f, 0.0f, 1.0f, 9.0f / 16.0f, 1.0f);
	tesselateBlockInWorld(tt, x, y, z);

	// Saw blade standing vertically in the middle (stonecutter_saw 254)
	bool origNoCulling = noCulling;
	noCulling = true;
	tt->setShape(1.0f / 16.0f, 9.0f / 16.0f, 7.5f / 16.0f, 15.0f / 16.0f, 1.0f, 8.5f / 16.0f);
	t.color(0.9f * br, 0.9f * br, 0.9f * br);
	renderNorth(tt, xf, yf, zf, 254);
	renderSouth(tt, xf, yf, zf, 254);
	renderFaceUp(tt, xf, yf, zf, 254);
	renderWest(tt, xf, yf, zf, 254);
	renderEast(tt, xf, yf, zf, 254);
	noCulling = origNoCulling;

	tt->setShape(0.0f, 0.0f, 0.0f, 1.0f, 9.0f / 16.0f, 1.0f);
	return true;
}

bool TileRenderer::tesselateChainInWorld(Tile* tt, int x, int y, int z) {
	tt->setShape(6.5f / 16.0f, 0.0f, 6.5f / 16.0f, 9.5f / 16.0f, 1.0f, 9.5f / 16.0f);
	tesselateBlockInWorld(tt, x, y, z);

	tt->setShape(6.5f / 16.0f, 0.0f, 6.5f / 16.0f, 9.5f / 16.0f, 1.0f, 9.5f / 16.0f);
	return true;
}

bool TileRenderer::tesselateScaffoldingInWorld(Tile* tt, int x, int y, int z) {
	tt->setShape(0.0f, 0.0f, 0.0f, 2.0f / 16.0f, 1.0f, 2.0f / 16.0f);
	tesselateBlockInWorld(tt, x, y, z);

	tt->setShape(14.0f / 16.0f, 0.0f, 0.0f, 1.0f, 1.0f, 2.0f / 16.0f);
	tesselateBlockInWorld(tt, x, y, z);

	tt->setShape(0.0f, 0.0f, 14.0f / 16.0f, 2.0f / 16.0f, 1.0f, 1.0f);
	tesselateBlockInWorld(tt, x, y, z);

	tt->setShape(14.0f / 16.0f, 0.0f, 14.0f / 16.0f, 1.0f, 1.0f, 1.0f);
	tesselateBlockInWorld(tt, x, y, z);

	tt->setShape(0.0f, 15.0f / 16.0f, 0.0f, 1.0f, 1.0f, 1.0f);
	tesselateBlockInWorld(tt, x, y, z);

	tt->setShape(0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f);
	return true;
}

bool TileRenderer::tesselateCandleInWorld(Tile* tt, int x, int y, int z) {
	Tesselator& t = Tesselator::instance;
	float br = tt->getBrightness(level, x, y, z);
	if (Tile::lightEmission[tt->id] > 0) br = 1.0f;

	int tex = tt->getTexture(level, x, y, z, 0);
	bool isAlt = (tex & Tile::TEXTURE_ALT_FLAG) != 0;
	if (atlasFilter != -1) {
		if (atlasFilter == 0 && isAlt) return false;
		if (atlasFilter == 1 && !isAlt) return false;
	}

	int cleanTex = tex & ~Tile::TEXTURE_ALT_FLAG;

	float colUp = 1.0f * br;
	float colDown = 0.5f * br;
	float colNS = 0.8f * br;
	float colWE = 0.6f * br;

	float xf = (float)x;
	float yf = (float)y;
	float zf = (float)z;

	float cx0 = xf + 7.0f / 16.0f, cx1 = xf + 9.0f / 16.0f;
	float cy0 = yf + 0.0f / 16.0f, cy1 = yf + 6.0f / 16.0f;
	float cz0 = zf + 7.0f / 16.0f, cz1 = zf + 9.0f / 16.0f;

	float uTop0 = getAtlasU(cleanTex, 0.0f), uTop1 = getAtlasU(cleanTex, 2.0f);
	float vTop0 = getAtlasV(cleanTex, 6.0f), vTop1 = getAtlasV(cleanTex, 8.0f);

	float uSide0 = getAtlasU(cleanTex, 0.0f), uSide1 = getAtlasU(cleanTex, 2.0f);
	float vSide0 = getAtlasV(cleanTex, 6.0f), vSide1 = getAtlasV(cleanTex, 12.0f);

	// Top
	t.color(colUp, colUp, colUp);
	t.vertexUV(cx0, cy1, cz0, uTop0, vTop0);
	t.vertexUV(cx0, cy1, cz1, uTop0, vTop1);
	t.vertexUV(cx1, cy1, cz1, uTop1, vTop1);
	t.vertexUV(cx1, cy1, cz0, uTop1, vTop0);

	// Down
	t.color(colDown, colDown, colDown);
	t.vertexUV(cx0, cy0, cz1, uTop0, vTop1);
	t.vertexUV(cx0, cy0, cz0, uTop0, vTop0);
	t.vertexUV(cx1, cy0, cz0, uTop1, vTop0);
	t.vertexUV(cx1, cy0, cz1, uTop1, vTop1);

	// North & South
	t.color(colNS, colNS, colNS);
	t.vertexUV(cx0, cy1, cz0, uSide1, vSide0);
	t.vertexUV(cx1, cy1, cz0, uSide0, vSide0);
	t.vertexUV(cx1, cy0, cz0, uSide0, vSide1);
	t.vertexUV(cx0, cy0, cz0, uSide1, vSide1);
	t.vertexUV(cx1, cy1, cz0, uSide0, vSide0);
	t.vertexUV(cx0, cy1, cz0, uSide1, vSide0);
	t.vertexUV(cx0, cy0, cz0, uSide1, vSide1);
	t.vertexUV(cx1, cy0, cz0, uSide0, vSide1);

	t.vertexUV(cx0, cy1, cz1, uSide0, vSide0);
	t.vertexUV(cx0, cy0, cz1, uSide0, vSide1);
	t.vertexUV(cx1, cy0, cz1, uSide1, vSide1);
	t.vertexUV(cx1, cy1, cz1, uSide1, vSide0);
	t.vertexUV(cx1, cy1, cz1, uSide1, vSide0);
	t.vertexUV(cx1, cy0, cz1, uSide1, vSide1);
	t.vertexUV(cx0, cy0, cz1, uSide0, vSide1);
	t.vertexUV(cx0, cy1, cz1, uSide0, vSide0);

	// West & East
	t.color(colWE, colWE, colWE);
	t.vertexUV(cx0, cy1, cz1, uSide1, vSide0);
	t.vertexUV(cx0, cy1, cz0, uSide0, vSide0);
	t.vertexUV(cx0, cy0, cz0, uSide0, vSide1);
	t.vertexUV(cx0, cy0, cz1, uSide1, vSide1);
	t.vertexUV(cx0, cy1, cz0, uSide0, vSide0);
	t.vertexUV(cx0, cy1, cz1, uSide1, vSide0);
	t.vertexUV(cx0, cy0, cz1, uSide1, vSide1);
	t.vertexUV(cx0, cy0, cz0, uSide0, vSide1);

	t.vertexUV(cx1, cy0, cz1, uSide0, vSide1);
	t.vertexUV(cx1, cy0, cz0, uSide1, vSide1);
	t.vertexUV(cx1, cy1, cz0, uSide1, vSide0);
	t.vertexUV(cx1, cy1, cz1, uSide0, vSide0);
	t.vertexUV(cx1, cy1, cz0, uSide1, vSide0);
	t.vertexUV(cx1, cy0, cz0, uSide1, vSide1);
	t.vertexUV(cx1, cy0, cz1, uSide0, vSide1);
	t.vertexUV(cx1, cy1, cz1, uSide0, vSide0);

	// Wick (Full Brightness)
	float wu0 = getAtlasU(cleanTex, 0.0f), wu1 = getAtlasU(cleanTex, 1.0f);
	float wv0 = getAtlasV(cleanTex, 4.5f), wv1 = getAtlasV(cleanTex, 6.0f);
	float wy0 = yf + 6.0f / 16.0f, wy1 = yf + 8.5f / 16.0f;
	float wx0 = xf + 7.5f / 16.0f, wx1 = xf + 8.5f / 16.0f;
	float wz0 = zf + 7.5f / 16.0f, wz1 = zf + 8.5f / 16.0f;
	float wMidX = xf + 8.0f / 16.0f;
	float wMidZ = zf + 8.0f / 16.0f;

	t.color(1.0f, 1.0f, 1.0f);
	// Quad X (double sided)
	t.vertexUV(wx0, wy1, wMidZ, wu0, wv0);
	t.vertexUV(wx0, wy0, wMidZ, wu0, wv1);
	t.vertexUV(wx1, wy0, wMidZ, wu1, wv1);
	t.vertexUV(wx1, wy1, wMidZ, wu1, wv0);

	t.vertexUV(wx1, wy1, wMidZ, wu1, wv0);
	t.vertexUV(wx1, wy0, wMidZ, wu1, wv1);
	t.vertexUV(wx0, wy0, wMidZ, wu0, wv1);
	t.vertexUV(wx0, wy1, wMidZ, wu0, wv0);

	// Quad Z (double sided)
	t.vertexUV(wMidX, wy1, wz0, wu0, wv0);
	t.vertexUV(wMidX, wy0, wz0, wu0, wv1);
	t.vertexUV(wMidX, wy0, wz1, wu1, wv1);
	t.vertexUV(wMidX, wy1, wz1, wu1, wv0);

	t.vertexUV(wMidX, wy1, wz1, wu1, wv0);
	t.vertexUV(wMidX, wy0, wz1, wu1, wv1);
	t.vertexUV(wMidX, wy0, wz0, wu0, wv1);
	t.vertexUV(wMidX, wy1, wz0, wu0, wv0);

	tt->setShape(7.0f / 16.0f, 0.0f, 7.0f / 16.0f, 9.0f / 16.0f, 6.0f / 16.0f, 9.0f / 16.0f);
	return true;
}

bool TileRenderer::tesselateCauldronInWorld(Tile* tt, int x, int y, int z) {
	Tesselator& t = Tesselator::instance;
	float br = tt->getBrightness(level, x, y, z);

	int tex = tt->getTexture(level, x, y, z, 0);
	bool isAlt = (tex & Tile::TEXTURE_ALT_FLAG) != 0;
	if (atlasFilter != -1) {
		if (atlasFilter == 0 && isAlt) return false;
		if (atlasFilter == 1 && !isAlt) return false;
	}

	const int TEX_TOP = 149;
	const int TEX_INNER = 150;
	const int TEX_SIDE = 163;
	const int TEX_BOTTOM = 165;

	float colUp = 1.0f * br;
	float colDown = 0.5f * br;
	float colNS = 0.8f * br;
	float colWE = 0.6f * br;

	float xf = (float)x;
	float yf = (float)y;
	float zf = (float)z;

	// 1. 4 Corner Legs (y: 0..3/16, size 3x3)
	// Leg NW (x: 0..3, z: 0..3)
	t.color(colDown, colDown, colDown);
	t.vertexUV(xf + 0.0f, yf + 0.0f, zf + 3.0f / 16.0f, getAtlasU(TEX_BOTTOM, 0), getAtlasV(TEX_BOTTOM, 3));
	t.vertexUV(xf + 0.0f, yf + 0.0f, zf + 0.0f, getAtlasU(TEX_BOTTOM, 0), getAtlasV(TEX_BOTTOM, 0));
	t.vertexUV(xf + 3.0f / 16.0f, yf + 0.0f, zf + 0.0f, getAtlasU(TEX_BOTTOM, 3), getAtlasV(TEX_BOTTOM, 0));
	t.vertexUV(xf + 3.0f / 16.0f, yf + 0.0f, zf + 3.0f / 16.0f, getAtlasU(TEX_BOTTOM, 3), getAtlasV(TEX_BOTTOM, 3));

	t.color(colNS, colNS, colNS);
	t.vertexUV(xf + 0.0f, yf + 3.0f / 16.0f, zf + 0.0f, getAtlasU(TEX_SIDE, 0), getAtlasV(TEX_SIDE, 13));
	t.vertexUV(xf + 3.0f / 16.0f, yf + 3.0f / 16.0f, zf + 0.0f, getAtlasU(TEX_SIDE, 3), getAtlasV(TEX_SIDE, 13));
	t.vertexUV(xf + 3.0f / 16.0f, yf + 0.0f, zf + 0.0f, getAtlasU(TEX_SIDE, 3), getAtlasV(TEX_SIDE, 16));
	t.vertexUV(xf + 0.0f, yf + 0.0f, zf + 0.0f, getAtlasU(TEX_SIDE, 0), getAtlasV(TEX_SIDE, 16));

	t.color(colWE, colWE, colWE);
	t.vertexUV(xf + 0.0f, yf + 3.0f / 16.0f, zf + 3.0f / 16.0f, getAtlasU(TEX_SIDE, 3), getAtlasV(TEX_SIDE, 13));
	t.vertexUV(xf + 0.0f, yf + 3.0f / 16.0f, zf + 0.0f, getAtlasU(TEX_SIDE, 0), getAtlasV(TEX_SIDE, 13));
	t.vertexUV(xf + 0.0f, yf + 0.0f, zf + 0.0f, getAtlasU(TEX_SIDE, 0), getAtlasV(TEX_SIDE, 16));
	t.vertexUV(xf + 0.0f, yf + 0.0f, zf + 3.0f / 16.0f, getAtlasU(TEX_SIDE, 3), getAtlasV(TEX_SIDE, 16));

	t.color(colNS, colNS, colNS);
	t.vertexUV(xf + 0.0f, yf + 3.0f / 16.0f, zf + 3.0f / 16.0f, getAtlasU(TEX_SIDE, 0), getAtlasV(TEX_SIDE, 13));
	t.vertexUV(xf + 0.0f, yf + 0.0f, zf + 3.0f / 16.0f, getAtlasU(TEX_SIDE, 0), getAtlasV(TEX_SIDE, 16));
	t.vertexUV(xf + 3.0f / 16.0f, yf + 0.0f, zf + 3.0f / 16.0f, getAtlasU(TEX_SIDE, 3), getAtlasV(TEX_SIDE, 16));
	t.vertexUV(xf + 3.0f / 16.0f, yf + 3.0f / 16.0f, zf + 3.0f / 16.0f, getAtlasU(TEX_SIDE, 3), getAtlasV(TEX_SIDE, 13));

	t.color(colWE, colWE, colWE);
	t.vertexUV(xf + 3.0f / 16.0f, yf + 3.0f / 16.0f, zf + 0.0f, getAtlasU(TEX_SIDE, 0), getAtlasV(TEX_SIDE, 13));
	t.vertexUV(xf + 3.0f / 16.0f, yf + 3.0f / 16.0f, zf + 3.0f / 16.0f, getAtlasU(TEX_SIDE, 3), getAtlasV(TEX_SIDE, 13));
	t.vertexUV(xf + 3.0f / 16.0f, yf + 0.0f, zf + 3.0f / 16.0f, getAtlasU(TEX_SIDE, 3), getAtlasV(TEX_SIDE, 16));
	t.vertexUV(xf + 3.0f / 16.0f, yf + 0.0f, zf + 0.0f, getAtlasU(TEX_SIDE, 0), getAtlasV(TEX_SIDE, 16));

	// Leg NE (x: 13..16, z: 0..3)
	t.color(colDown, colDown, colDown);
	t.vertexUV(xf + 13.0f / 16.0f, yf + 0.0f, zf + 3.0f / 16.0f, getAtlasU(TEX_BOTTOM, 13), getAtlasV(TEX_BOTTOM, 3));
	t.vertexUV(xf + 13.0f / 16.0f, yf + 0.0f, zf + 0.0f, getAtlasU(TEX_BOTTOM, 13), getAtlasV(TEX_BOTTOM, 0));
	t.vertexUV(xf + 1.0f, yf + 0.0f, zf + 0.0f, getAtlasU(TEX_BOTTOM, 16), getAtlasV(TEX_BOTTOM, 0));
	t.vertexUV(xf + 1.0f, yf + 0.0f, zf + 3.0f / 16.0f, getAtlasU(TEX_BOTTOM, 16), getAtlasV(TEX_BOTTOM, 3));

	t.color(colNS, colNS, colNS);
	t.vertexUV(xf + 13.0f / 16.0f, yf + 3.0f / 16.0f, zf + 0.0f, getAtlasU(TEX_SIDE, 13), getAtlasV(TEX_SIDE, 13));
	t.vertexUV(xf + 1.0f, yf + 3.0f / 16.0f, zf + 0.0f, getAtlasU(TEX_SIDE, 16), getAtlasV(TEX_SIDE, 13));
	t.vertexUV(xf + 1.0f, yf + 0.0f, zf + 0.0f, getAtlasU(TEX_SIDE, 16), getAtlasV(TEX_SIDE, 16));
	t.vertexUV(xf + 13.0f / 16.0f, yf + 0.0f, zf + 0.0f, getAtlasU(TEX_SIDE, 13), getAtlasV(TEX_SIDE, 16));

	t.color(colWE, colWE, colWE);
	t.vertexUV(xf + 1.0f, yf + 3.0f / 16.0f, zf + 0.0f, getAtlasU(TEX_SIDE, 0), getAtlasV(TEX_SIDE, 13));
	t.vertexUV(xf + 1.0f, yf + 3.0f / 16.0f, zf + 3.0f / 16.0f, getAtlasU(TEX_SIDE, 3), getAtlasV(TEX_SIDE, 13));
	t.vertexUV(xf + 1.0f, yf + 0.0f, zf + 3.0f / 16.0f, getAtlasU(TEX_SIDE, 3), getAtlasV(TEX_SIDE, 16));
	t.vertexUV(xf + 1.0f, yf + 0.0f, zf + 0.0f, getAtlasU(TEX_SIDE, 0), getAtlasV(TEX_SIDE, 16));

	t.color(colNS, colNS, colNS);
	t.vertexUV(xf + 13.0f / 16.0f, yf + 3.0f / 16.0f, zf + 3.0f / 16.0f, getAtlasU(TEX_SIDE, 13), getAtlasV(TEX_SIDE, 13));
	t.vertexUV(xf + 13.0f / 16.0f, yf + 0.0f, zf + 3.0f / 16.0f, getAtlasU(TEX_SIDE, 13), getAtlasV(TEX_SIDE, 16));
	t.vertexUV(xf + 1.0f, yf + 0.0f, zf + 3.0f / 16.0f, getAtlasU(TEX_SIDE, 16), getAtlasV(TEX_SIDE, 16));
	t.vertexUV(xf + 1.0f, yf + 3.0f / 16.0f, zf + 3.0f / 16.0f, getAtlasU(TEX_SIDE, 16), getAtlasV(TEX_SIDE, 13));

	t.color(colWE, colWE, colWE);
	t.vertexUV(xf + 13.0f / 16.0f, yf + 3.0f / 16.0f, zf + 3.0f / 16.0f, getAtlasU(TEX_SIDE, 3), getAtlasV(TEX_SIDE, 13));
	t.vertexUV(xf + 13.0f / 16.0f, yf + 3.0f / 16.0f, zf + 0.0f, getAtlasU(TEX_SIDE, 0), getAtlasV(TEX_SIDE, 13));
	t.vertexUV(xf + 13.0f / 16.0f, yf + 0.0f, zf + 0.0f, getAtlasU(TEX_SIDE, 0), getAtlasV(TEX_SIDE, 16));
	t.vertexUV(xf + 13.0f / 16.0f, yf + 0.0f, zf + 3.0f / 16.0f, getAtlasU(TEX_SIDE, 3), getAtlasV(TEX_SIDE, 16));

	// Leg SW (x: 0..3, z: 13..16)
	t.color(colDown, colDown, colDown);
	t.vertexUV(xf + 0.0f, yf + 0.0f, zf + 1.0f, getAtlasU(TEX_BOTTOM, 0), getAtlasV(TEX_BOTTOM, 16));
	t.vertexUV(xf + 0.0f, yf + 0.0f, zf + 13.0f / 16.0f, getAtlasU(TEX_BOTTOM, 0), getAtlasV(TEX_BOTTOM, 13));
	t.vertexUV(xf + 3.0f / 16.0f, yf + 0.0f, zf + 13.0f / 16.0f, getAtlasU(TEX_BOTTOM, 3), getAtlasV(TEX_BOTTOM, 13));
	t.vertexUV(xf + 3.0f / 16.0f, yf + 0.0f, zf + 1.0f, getAtlasU(TEX_BOTTOM, 3), getAtlasV(TEX_BOTTOM, 16));

	t.color(colNS, colNS, colNS);
	t.vertexUV(xf + 0.0f, yf + 3.0f / 16.0f, zf + 1.0f, getAtlasU(TEX_SIDE, 0), getAtlasV(TEX_SIDE, 13));
	t.vertexUV(xf + 0.0f, yf + 0.0f, zf + 1.0f, getAtlasU(TEX_SIDE, 0), getAtlasV(TEX_SIDE, 16));
	t.vertexUV(xf + 3.0f / 16.0f, yf + 0.0f, zf + 1.0f, getAtlasU(TEX_SIDE, 3), getAtlasV(TEX_SIDE, 16));
	t.vertexUV(xf + 3.0f / 16.0f, yf + 3.0f / 16.0f, zf + 1.0f, getAtlasU(TEX_SIDE, 3), getAtlasV(TEX_SIDE, 13));

	t.color(colWE, colWE, colWE);
	t.vertexUV(xf + 0.0f, yf + 3.0f / 16.0f, zf + 1.0f, getAtlasU(TEX_SIDE, 16), getAtlasV(TEX_SIDE, 13));
	t.vertexUV(xf + 0.0f, yf + 3.0f / 16.0f, zf + 13.0f / 16.0f, getAtlasU(TEX_SIDE, 13), getAtlasV(TEX_SIDE, 13));
	t.vertexUV(xf + 0.0f, yf + 0.0f, zf + 13.0f / 16.0f, getAtlasU(TEX_SIDE, 13), getAtlasV(TEX_SIDE, 16));
	t.vertexUV(xf + 0.0f, yf + 0.0f, zf + 1.0f, getAtlasU(TEX_SIDE, 16), getAtlasV(TEX_SIDE, 16));

	t.color(colNS, colNS, colNS);
	t.vertexUV(xf + 0.0f, yf + 3.0f / 16.0f, zf + 13.0f / 16.0f, getAtlasU(TEX_SIDE, 0), getAtlasV(TEX_SIDE, 13));
	t.vertexUV(xf + 3.0f / 16.0f, yf + 3.0f / 16.0f, zf + 13.0f / 16.0f, getAtlasU(TEX_SIDE, 3), getAtlasV(TEX_SIDE, 13));
	t.vertexUV(xf + 3.0f / 16.0f, yf + 0.0f, zf + 13.0f / 16.0f, getAtlasU(TEX_SIDE, 3), getAtlasV(TEX_SIDE, 16));
	t.vertexUV(xf + 0.0f, yf + 0.0f, zf + 13.0f / 16.0f, getAtlasU(TEX_SIDE, 0), getAtlasV(TEX_SIDE, 16));

	t.color(colWE, colWE, colWE);
	t.vertexUV(xf + 3.0f / 16.0f, yf + 3.0f / 16.0f, zf + 13.0f / 16.0f, getAtlasU(TEX_SIDE, 13), getAtlasV(TEX_SIDE, 13));
	t.vertexUV(xf + 3.0f / 16.0f, yf + 3.0f / 16.0f, zf + 1.0f, getAtlasU(TEX_SIDE, 16), getAtlasV(TEX_SIDE, 13));
	t.vertexUV(xf + 3.0f / 16.0f, yf + 0.0f, zf + 1.0f, getAtlasU(TEX_SIDE, 16), getAtlasV(TEX_SIDE, 16));
	t.vertexUV(xf + 3.0f / 16.0f, yf + 0.0f, zf + 13.0f / 16.0f, getAtlasU(TEX_SIDE, 13), getAtlasV(TEX_SIDE, 16));

	// Leg SE (x: 13..16, z: 13..16)
	t.color(colDown, colDown, colDown);
	t.vertexUV(xf + 13.0f / 16.0f, yf + 0.0f, zf + 1.0f, getAtlasU(TEX_BOTTOM, 13), getAtlasV(TEX_BOTTOM, 16));
	t.vertexUV(xf + 13.0f / 16.0f, yf + 0.0f, zf + 13.0f / 16.0f, getAtlasU(TEX_BOTTOM, 13), getAtlasV(TEX_BOTTOM, 13));
	t.vertexUV(xf + 1.0f, yf + 0.0f, zf + 13.0f / 16.0f, getAtlasU(TEX_BOTTOM, 16), getAtlasV(TEX_BOTTOM, 13));
	t.vertexUV(xf + 1.0f, yf + 0.0f, zf + 1.0f, getAtlasU(TEX_BOTTOM, 16), getAtlasV(TEX_BOTTOM, 16));

	t.color(colNS, colNS, colNS);
	t.vertexUV(xf + 13.0f / 16.0f, yf + 3.0f / 16.0f, zf + 1.0f, getAtlasU(TEX_SIDE, 13), getAtlasV(TEX_SIDE, 13));
	t.vertexUV(xf + 13.0f / 16.0f, yf + 0.0f, zf + 1.0f, getAtlasU(TEX_SIDE, 13), getAtlasV(TEX_SIDE, 16));
	t.vertexUV(xf + 1.0f, yf + 0.0f, zf + 1.0f, getAtlasU(TEX_SIDE, 16), getAtlasV(TEX_SIDE, 16));
	t.vertexUV(xf + 1.0f, yf + 3.0f / 16.0f, zf + 1.0f, getAtlasU(TEX_SIDE, 16), getAtlasV(TEX_SIDE, 13));

	t.color(colWE, colWE, colWE);
	t.vertexUV(xf + 1.0f, yf + 3.0f / 16.0f, zf + 13.0f / 16.0f, getAtlasU(TEX_SIDE, 13), getAtlasV(TEX_SIDE, 13));
	t.vertexUV(xf + 1.0f, yf + 3.0f / 16.0f, zf + 1.0f, getAtlasU(TEX_SIDE, 16), getAtlasV(TEX_SIDE, 13));
	t.vertexUV(xf + 1.0f, yf + 0.0f, zf + 1.0f, getAtlasU(TEX_SIDE, 16), getAtlasV(TEX_SIDE, 16));
	t.vertexUV(xf + 1.0f, yf + 0.0f, zf + 13.0f / 16.0f, getAtlasU(TEX_SIDE, 13), getAtlasV(TEX_SIDE, 16));

	t.color(colNS, colNS, colNS);
	t.vertexUV(xf + 13.0f / 16.0f, yf + 3.0f / 16.0f, zf + 13.0f / 16.0f, getAtlasU(TEX_SIDE, 13), getAtlasV(TEX_SIDE, 13));
	t.vertexUV(xf + 1.0f, yf + 3.0f / 16.0f, zf + 13.0f / 16.0f, getAtlasU(TEX_SIDE, 16), getAtlasV(TEX_SIDE, 13));
	t.vertexUV(xf + 1.0f, yf + 0.0f, zf + 13.0f / 16.0f, getAtlasU(TEX_SIDE, 16), getAtlasV(TEX_SIDE, 16));
	t.vertexUV(xf + 13.0f / 16.0f, yf + 0.0f, zf + 13.0f / 16.0f, getAtlasU(TEX_SIDE, 13), getAtlasV(TEX_SIDE, 16));

	t.color(colWE, colWE, colWE);
	t.vertexUV(xf + 13.0f / 16.0f, yf + 3.0f / 16.0f, zf + 1.0f, getAtlasU(TEX_SIDE, 16), getAtlasV(TEX_SIDE, 13));
	t.vertexUV(xf + 13.0f / 16.0f, yf + 3.0f / 16.0f, zf + 13.0f / 16.0f, getAtlasU(TEX_SIDE, 13), getAtlasV(TEX_SIDE, 13));
	t.vertexUV(xf + 13.0f / 16.0f, yf + 0.0f, zf + 13.0f / 16.0f, getAtlasU(TEX_SIDE, 13), getAtlasV(TEX_SIDE, 16));
	t.vertexUV(xf + 13.0f / 16.0f, yf + 0.0f, zf + 1.0f, getAtlasU(TEX_SIDE, 16), getAtlasV(TEX_SIDE, 16));

	// 2. Cauldron Bottom Plate (y: 3..4/16)
	t.color(colDown, colDown, colDown);
	t.vertexUV(xf + 0.0f, yf + 3.0f / 16.0f, zf + 1.0f, getAtlasU(TEX_BOTTOM, 0), getAtlasV(TEX_BOTTOM, 16));
	t.vertexUV(xf + 0.0f, yf + 3.0f / 16.0f, zf + 0.0f, getAtlasU(TEX_BOTTOM, 0), getAtlasV(TEX_BOTTOM, 0));
	t.vertexUV(xf + 1.0f, yf + 3.0f / 16.0f, zf + 0.0f, getAtlasU(TEX_BOTTOM, 16), getAtlasV(TEX_BOTTOM, 0));
	t.vertexUV(xf + 1.0f, yf + 3.0f / 16.0f, zf + 1.0f, getAtlasU(TEX_BOTTOM, 16), getAtlasV(TEX_BOTTOM, 16));

	t.color(colUp, colUp, colUp);
	t.vertexUV(xf + 2.0f / 16.0f, yf + 4.0f / 16.0f, zf + 2.0f / 16.0f, getAtlasU(TEX_INNER, 2), getAtlasV(TEX_INNER, 2));
	t.vertexUV(xf + 2.0f / 16.0f, yf + 4.0f / 16.0f, zf + 14.0f / 16.0f, getAtlasU(TEX_INNER, 2), getAtlasV(TEX_INNER, 14));
	t.vertexUV(xf + 14.0f / 16.0f, yf + 4.0f / 16.0f, zf + 14.0f / 16.0f, getAtlasU(TEX_INNER, 14), getAtlasV(TEX_INNER, 14));
	t.vertexUV(xf + 14.0f / 16.0f, yf + 4.0f / 16.0f, zf + 2.0f / 16.0f, getAtlasU(TEX_INNER, 14), getAtlasV(TEX_INNER, 2));

	// 3. 4 Walls (y: 3/16..1.0, height 13/16)
	// North Wall
	t.color(colNS, colNS, colNS);
	t.vertexUV(xf + 0.0f, yf + 1.0f, zf + 0.0f, getAtlasU(TEX_SIDE, 0), getAtlasV(TEX_SIDE, 0));
	t.vertexUV(xf + 1.0f, yf + 1.0f, zf + 0.0f, getAtlasU(TEX_SIDE, 16), getAtlasV(TEX_SIDE, 0));
	t.vertexUV(xf + 1.0f, yf + 3.0f / 16.0f, zf + 0.0f, getAtlasU(TEX_SIDE, 16), getAtlasV(TEX_SIDE, 13));
	t.vertexUV(xf + 0.0f, yf + 3.0f / 16.0f, zf + 0.0f, getAtlasU(TEX_SIDE, 0), getAtlasV(TEX_SIDE, 13));

	t.vertexUV(xf + 2.0f / 16.0f, yf + 1.0f, zf + 2.0f / 16.0f, getAtlasU(TEX_INNER, 2), getAtlasV(TEX_INNER, 0));
	t.vertexUV(xf + 2.0f / 16.0f, yf + 4.0f / 16.0f, zf + 2.0f / 16.0f, getAtlasU(TEX_INNER, 2), getAtlasV(TEX_INNER, 12));
	t.vertexUV(xf + 14.0f / 16.0f, yf + 4.0f / 16.0f, zf + 2.0f / 16.0f, getAtlasU(TEX_INNER, 14), getAtlasV(TEX_INNER, 12));
	t.vertexUV(xf + 14.0f / 16.0f, yf + 1.0f, zf + 2.0f / 16.0f, getAtlasU(TEX_INNER, 14), getAtlasV(TEX_INNER, 0));

	t.color(colUp, colUp, colUp);
	t.vertexUV(xf + 0.0f, yf + 1.0f, zf + 0.0f, getAtlasU(TEX_TOP, 0), getAtlasV(TEX_TOP, 0));
	t.vertexUV(xf + 0.0f, yf + 1.0f, zf + 2.0f / 16.0f, getAtlasU(TEX_TOP, 0), getAtlasV(TEX_TOP, 2));
	t.vertexUV(xf + 1.0f, yf + 1.0f, zf + 2.0f / 16.0f, getAtlasU(TEX_TOP, 16), getAtlasV(TEX_TOP, 2));
	t.vertexUV(xf + 1.0f, yf + 1.0f, zf + 0.0f, getAtlasU(TEX_TOP, 16), getAtlasV(TEX_TOP, 0));

	// South Wall
	t.color(colNS, colNS, colNS);
	t.vertexUV(xf + 0.0f, yf + 1.0f, zf + 1.0f, getAtlasU(TEX_SIDE, 0), getAtlasV(TEX_SIDE, 0));
	t.vertexUV(xf + 0.0f, yf + 3.0f / 16.0f, zf + 1.0f, getAtlasU(TEX_SIDE, 0), getAtlasV(TEX_SIDE, 13));
	t.vertexUV(xf + 1.0f, yf + 3.0f / 16.0f, zf + 1.0f, getAtlasU(TEX_SIDE, 16), getAtlasV(TEX_SIDE, 13));
	t.vertexUV(xf + 1.0f, yf + 1.0f, zf + 1.0f, getAtlasU(TEX_SIDE, 16), getAtlasV(TEX_SIDE, 0));

	t.vertexUV(xf + 2.0f / 16.0f, yf + 1.0f, zf + 14.0f / 16.0f, getAtlasU(TEX_INNER, 2), getAtlasV(TEX_INNER, 0));
	t.vertexUV(xf + 14.0f / 16.0f, yf + 1.0f, zf + 14.0f / 16.0f, getAtlasU(TEX_INNER, 14), getAtlasV(TEX_INNER, 0));
	t.vertexUV(xf + 14.0f / 16.0f, yf + 4.0f / 16.0f, zf + 14.0f / 16.0f, getAtlasU(TEX_INNER, 14), getAtlasV(TEX_INNER, 12));
	t.vertexUV(xf + 2.0f / 16.0f, yf + 4.0f / 16.0f, zf + 14.0f / 16.0f, getAtlasU(TEX_INNER, 2), getAtlasV(TEX_INNER, 12));

	t.color(colUp, colUp, colUp);
	t.vertexUV(xf + 0.0f, yf + 1.0f, zf + 14.0f / 16.0f, getAtlasU(TEX_TOP, 0), getAtlasV(TEX_TOP, 14));
	t.vertexUV(xf + 0.0f, yf + 1.0f, zf + 1.0f, getAtlasU(TEX_TOP, 0), getAtlasV(TEX_TOP, 16));
	t.vertexUV(xf + 1.0f, yf + 1.0f, zf + 1.0f, getAtlasU(TEX_TOP, 16), getAtlasV(TEX_TOP, 16));
	t.vertexUV(xf + 1.0f, yf + 1.0f, zf + 14.0f / 16.0f, getAtlasU(TEX_TOP, 16), getAtlasV(TEX_TOP, 14));

	// West Wall
	t.color(colWE, colWE, colWE);
	t.vertexUV(xf + 0.0f, yf + 1.0f, zf + 1.0f, getAtlasU(TEX_SIDE, 16), getAtlasV(TEX_SIDE, 0));
	t.vertexUV(xf + 0.0f, yf + 1.0f, zf + 0.0f, getAtlasU(TEX_SIDE, 0), getAtlasV(TEX_SIDE, 0));
	t.vertexUV(xf + 0.0f, yf + 3.0f / 16.0f, zf + 0.0f, getAtlasU(TEX_SIDE, 0), getAtlasV(TEX_SIDE, 13));
	t.vertexUV(xf + 0.0f, yf + 3.0f / 16.0f, zf + 1.0f, getAtlasU(TEX_SIDE, 16), getAtlasV(TEX_SIDE, 13));

	t.vertexUV(xf + 2.0f / 16.0f, yf + 1.0f, zf + 2.0f / 16.0f, getAtlasU(TEX_INNER, 2), getAtlasV(TEX_INNER, 0));
	t.vertexUV(xf + 2.0f / 16.0f, yf + 1.0f, zf + 14.0f / 16.0f, getAtlasU(TEX_INNER, 14), getAtlasV(TEX_INNER, 0));
	t.vertexUV(xf + 2.0f / 16.0f, yf + 4.0f / 16.0f, zf + 14.0f / 16.0f, getAtlasU(TEX_INNER, 14), getAtlasV(TEX_INNER, 12));
	t.vertexUV(xf + 2.0f / 16.0f, yf + 4.0f / 16.0f, zf + 2.0f / 16.0f, getAtlasU(TEX_INNER, 2), getAtlasV(TEX_INNER, 12));

	t.color(colUp, colUp, colUp);
	t.vertexUV(xf + 0.0f, yf + 1.0f, zf + 2.0f / 16.0f, getAtlasU(TEX_TOP, 0), getAtlasV(TEX_TOP, 2));
	t.vertexUV(xf + 0.0f, yf + 1.0f, zf + 14.0f / 16.0f, getAtlasU(TEX_TOP, 0), getAtlasV(TEX_TOP, 14));
	t.vertexUV(xf + 2.0f / 16.0f, yf + 1.0f, zf + 14.0f / 16.0f, getAtlasU(TEX_TOP, 2), getAtlasV(TEX_TOP, 14));
	t.vertexUV(xf + 2.0f / 16.0f, yf + 1.0f, zf + 2.0f / 16.0f, getAtlasU(TEX_TOP, 2), getAtlasV(TEX_TOP, 2));

	// East Wall
	t.color(colWE, colWE, colWE);
	t.vertexUV(xf + 1.0f, yf + 1.0f, zf + 0.0f, getAtlasU(TEX_SIDE, 0), getAtlasV(TEX_SIDE, 0));
	t.vertexUV(xf + 1.0f, yf + 1.0f, zf + 1.0f, getAtlasU(TEX_SIDE, 16), getAtlasV(TEX_SIDE, 0));
	t.vertexUV(xf + 1.0f, yf + 3.0f / 16.0f, zf + 1.0f, getAtlasU(TEX_SIDE, 16), getAtlasV(TEX_SIDE, 13));
	t.vertexUV(xf + 1.0f, yf + 3.0f / 16.0f, zf + 0.0f, getAtlasU(TEX_SIDE, 0), getAtlasV(TEX_SIDE, 13));

	t.vertexUV(xf + 14.0f / 16.0f, yf + 1.0f, zf + 14.0f / 16.0f, getAtlasU(TEX_INNER, 14), getAtlasV(TEX_INNER, 0));
	t.vertexUV(xf + 14.0f / 16.0f, yf + 1.0f, zf + 2.0f / 16.0f, getAtlasU(TEX_INNER, 2), getAtlasV(TEX_INNER, 0));
	t.vertexUV(xf + 14.0f / 16.0f, yf + 4.0f / 16.0f, zf + 2.0f / 16.0f, getAtlasU(TEX_INNER, 2), getAtlasV(TEX_INNER, 12));
	t.vertexUV(xf + 14.0f / 16.0f, yf + 4.0f / 16.0f, zf + 14.0f / 16.0f, getAtlasU(TEX_INNER, 14), getAtlasV(TEX_INNER, 12));

	t.color(colUp, colUp, colUp);
	t.vertexUV(xf + 14.0f / 16.0f, yf + 1.0f, zf + 2.0f / 16.0f, getAtlasU(TEX_TOP, 14), getAtlasV(TEX_TOP, 2));
	t.vertexUV(xf + 14.0f / 16.0f, yf + 1.0f, zf + 14.0f / 16.0f, getAtlasU(TEX_TOP, 14), getAtlasV(TEX_TOP, 14));
	t.vertexUV(xf + 1.0f, yf + 1.0f, zf + 14.0f / 16.0f, getAtlasU(TEX_TOP, 16), getAtlasV(TEX_TOP, 14));
	t.vertexUV(xf + 1.0f, yf + 1.0f, zf + 2.0f / 16.0f, getAtlasU(TEX_TOP, 16), getAtlasV(TEX_TOP, 2));

	tt->setShape(0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f);
	return true;
}

bool TileRenderer::tesselateAnvilInWorld(Tile* tt, int x, int y, int z) {
	Tesselator& t = Tesselator::instance;
	float br = tt->getBrightness(level, x, y, z);

	int tex = tt->getTexture(level, x, y, z, 0);
	bool isAlt = (tex & Tile::TEXTURE_ALT_FLAG) != 0;
	if (atlasFilter != -1) {
		if (atlasFilter == 0 && isAlt) return false;
		if (atlasFilter == 1 && !isAlt) return false;
	}

	int dir = level ? (level->getData(x, y, z) & 3) : 0;
	bool alongX = (dir == 1 || dir == 3);

	int dmg = 0;
	AnvilTile* at = dynamic_cast<AnvilTile*>(tt);
	if (at) {
		dmg = at->damageType;
	} else if (tt->id == 511) {
		dmg = 1;
	} else if (tt->id == 559) {
		dmg = 2;
	}

	const int TEX_BASE = 166;
	const int TEX_TOP = (dmg == 0 ? 170 : (dmg == 1 ? 180 : 185));

	float colUp = 1.0f * br;
	float colDown = 0.5f * br;
	float colNS = 0.8f * br;
	float colWE = 0.6f * br;

	float xf = (float)x;
	float yf = (float)y;
	float zf = (float)z;

	auto renderBox = [&](float x0, float y0, float z0, float x1, float y1, float z1,
	                     int tUp, float uUp0, float vUp0, float uUp1, float vUp1,
	                     int tDown, float uD0, float vD0, float uD1, float vD1,
	                     int tNorth, float uN0, float vN0, float uN1, float vN1,
	                     int tSouth, float uS0, float vS0, float uS1, float vS1,
	                     int tWest, float uW0, float vW0, float uW1, float vW1,
	                     int tEast, float uE0, float vE0, float uE1, float vE1) {
		// Up (+Y)
		t.color(colUp, colUp, colUp);
		t.vertexUV(x0, y1, z0, getAtlasU(tUp, uUp0), getAtlasV(tUp, vUp0));
		t.vertexUV(x0, y1, z1, getAtlasU(tUp, uUp0), getAtlasV(tUp, vUp1));
		t.vertexUV(x1, y1, z1, getAtlasU(tUp, uUp1), getAtlasV(tUp, vUp1));
		t.vertexUV(x1, y1, z0, getAtlasU(tUp, uUp1), getAtlasV(tUp, vUp0));

		// Down (-Y)
		t.color(colDown, colDown, colDown);
		t.vertexUV(x0, y0, z1, getAtlasU(tDown, uD0), getAtlasV(tDown, vD1));
		t.vertexUV(x0, y0, z0, getAtlasU(tDown, uD0), getAtlasV(tDown, vD0));
		t.vertexUV(x1, y0, z0, getAtlasU(tDown, uD1), getAtlasV(tDown, vD0));
		t.vertexUV(x1, y0, z1, getAtlasU(tDown, uD1), getAtlasV(tDown, vD1));

		// North (-Z)
		t.color(colNS, colNS, colNS);
		t.vertexUV(x0, y1, z0, getAtlasU(tNorth, uN0), getAtlasV(tNorth, vN0));
		t.vertexUV(x1, y1, z0, getAtlasU(tNorth, uN1), getAtlasV(tNorth, vN0));
		t.vertexUV(x1, y0, z0, getAtlasU(tNorth, uN1), getAtlasV(tNorth, vN1));
		t.vertexUV(x0, y0, z0, getAtlasU(tNorth, uN0), getAtlasV(tNorth, vN1));

		// South (+Z)
		t.vertexUV(x1, y1, z1, getAtlasU(tSouth, uS1), getAtlasV(tSouth, vS0));
		t.vertexUV(x0, y1, z1, getAtlasU(tSouth, uS0), getAtlasV(tSouth, vS0));
		t.vertexUV(x0, y0, z1, getAtlasU(tSouth, uS0), getAtlasV(tSouth, vS1));
		t.vertexUV(x1, y0, z1, getAtlasU(tSouth, uS1), getAtlasV(tSouth, vS1));

		// West (-X)
		t.color(colWE, colWE, colWE);
		t.vertexUV(x0, y1, z1, getAtlasU(tWest, uW1), getAtlasV(tWest, vW0));
		t.vertexUV(x0, y1, z0, getAtlasU(tWest, uW0), getAtlasV(tWest, vW0));
		t.vertexUV(x0, y0, z0, getAtlasU(tWest, uW0), getAtlasV(tWest, vW1));
		t.vertexUV(x0, y0, z1, getAtlasU(tWest, uW1), getAtlasV(tWest, vW1));

		// East (+X)
		t.vertexUV(x1, y1, z0, getAtlasU(tEast, uE0), getAtlasV(tEast, vE0));
		t.vertexUV(x1, y1, z1, getAtlasU(tEast, uE1), getAtlasV(tEast, vE0));
		t.vertexUV(x1, y0, z1, getAtlasU(tEast, uE1), getAtlasV(tEast, vE1));
		t.vertexUV(x1, y0, z0, getAtlasU(tEast, uE0), getAtlasV(tEast, vE1));
	};

	// 1. Base (12x4x12 centered at [2..14, 0..4, 2..14])
	renderBox(xf + 2.0f / 16.0f, yf + 0.0f, zf + 2.0f / 16.0f,
	          xf + 14.0f / 16.0f, yf + 4.0f / 16.0f, zf + 14.0f / 16.0f,
	          TEX_BASE, 2, 2, 14, 14,
	          TEX_BASE, 2, 2, 14, 14,
	          TEX_BASE, 2, 12, 14, 16,
	          TEX_BASE, 2, 12, 14, 16,
	          TEX_BASE, 2, 12, 14, 16,
	          TEX_BASE, 2, 12, 14, 16);

	// 2. Lower Neck (y: 4..5/16)
	float lx0, lx1, lz0, lz1, mx0, mx1, mz0, mz1;
	if (alongX) {
		lx0 = xf + 3.0f / 16.0f; lx1 = xf + 13.0f / 16.0f;
		lz0 = zf + 4.0f / 16.0f; lz1 = zf + 12.0f / 16.0f;

		mx0 = xf + 4.0f / 16.0f; mx1 = xf + 12.0f / 16.0f;
		mz0 = zf + 6.0f / 16.0f; mz1 = zf + 10.0f / 16.0f;
	} else {
		lx0 = xf + 4.0f / 16.0f; lx1 = xf + 12.0f / 16.0f;
		lz0 = zf + 3.0f / 16.0f; lz1 = zf + 13.0f / 16.0f;

		mx0 = xf + 6.0f / 16.0f; mx1 = xf + 10.0f / 16.0f;
		mz0 = zf + 4.0f / 16.0f; mz1 = zf + 12.0f / 16.0f;
	}

	renderBox(lx0, yf + 4.0f / 16.0f, lz0, lx1, yf + 5.0f / 16.0f, lz1,
	          TEX_BASE, 4, 4, 12, 12,
	          TEX_BASE, 4, 4, 12, 12,
	          TEX_BASE, 4, 15, 12, 16,
	          TEX_BASE, 4, 15, 12, 16,
	          TEX_BASE, 4, 15, 12, 16,
	          TEX_BASE, 4, 15, 12, 16);

	// 3. Middle Stem (y: 5..10/16)
	renderBox(mx0, yf + 5.0f / 16.0f, mz0, mx1, yf + 10.0f / 16.0f, mz1,
	          TEX_BASE, 6, 6, 10, 10,
	          TEX_BASE, 6, 6, 10, 10,
	          TEX_BASE, 6, 5, 10, 10,
	          TEX_BASE, 6, 5, 10, 10,
	          TEX_BASE, 6, 5, 10, 10,
	          TEX_BASE, 6, 5, 10, 10);

	// 4. Head (y: 10..16/16)
	if (alongX) {
		float hx0 = xf + 0.0f, hx1 = xf + 1.0f;
		float hz0 = zf + 3.0f / 16.0f, hz1 = zf + 13.0f / 16.0f;
		float hy0 = yf + 10.0f / 16.0f, hy1 = yf + 1.0f;

		// Head Up face (Rotated 90 deg so length along X samples along V, width along Z samples along U)
		t.color(colUp, colUp, colUp);
		t.vertexUV(hx0, hy1, hz0, getAtlasU(TEX_TOP, 3), getAtlasV(TEX_TOP, 0));
		t.vertexUV(hx0, hy1, hz1, getAtlasU(TEX_TOP, 13), getAtlasV(TEX_TOP, 0));
		t.vertexUV(hx1, hy1, hz1, getAtlasU(TEX_TOP, 13), getAtlasV(TEX_TOP, 16));
		t.vertexUV(hx1, hy1, hz0, getAtlasU(TEX_TOP, 3), getAtlasV(TEX_TOP, 16));

		// Down
		t.color(colDown, colDown, colDown);
		t.vertexUV(hx0, hy0, hz1, getAtlasU(TEX_BASE, 0), getAtlasV(TEX_BASE, 13));
		t.vertexUV(hx0, hy0, hz0, getAtlasU(TEX_BASE, 0), getAtlasV(TEX_BASE, 3));
		t.vertexUV(hx1, hy0, hz0, getAtlasU(TEX_BASE, 16), getAtlasV(TEX_BASE, 3));
		t.vertexUV(hx1, hy0, hz1, getAtlasU(TEX_BASE, 16), getAtlasV(TEX_BASE, 13));

		// North (-Z)
		t.color(colNS, colNS, colNS);
		t.vertexUV(hx0, hy1, hz0, getAtlasU(TEX_BASE, 0), getAtlasV(TEX_BASE, 0));
		t.vertexUV(hx1, hy1, hz0, getAtlasU(TEX_BASE, 16), getAtlasV(TEX_BASE, 0));
		t.vertexUV(hx1, hy0, hz0, getAtlasU(TEX_BASE, 16), getAtlasV(TEX_BASE, 6));
		t.vertexUV(hx0, hy0, hz0, getAtlasU(TEX_BASE, 0), getAtlasV(TEX_BASE, 6));

		// South (+Z)
		t.vertexUV(hx1, hy1, hz1, getAtlasU(TEX_BASE, 16), getAtlasV(TEX_BASE, 0));
		t.vertexUV(hx0, hy1, hz1, getAtlasU(TEX_BASE, 0), getAtlasV(TEX_BASE, 0));
		t.vertexUV(hx0, hy0, hz1, getAtlasU(TEX_BASE, 0), getAtlasV(TEX_BASE, 6));
		t.vertexUV(hx1, hy0, hz1, getAtlasU(TEX_BASE, 16), getAtlasV(TEX_BASE, 6));

		// West (-X)
		t.color(colWE, colWE, colWE);
		t.vertexUV(hx0, hy1, hz1, getAtlasU(TEX_BASE, 13), getAtlasV(TEX_BASE, 0));
		t.vertexUV(hx0, hy1, hz0, getAtlasU(TEX_BASE, 3), getAtlasV(TEX_BASE, 0));
		t.vertexUV(hx0, hy0, hz0, getAtlasU(TEX_BASE, 3), getAtlasV(TEX_BASE, 6));
		t.vertexUV(hx0, hy0, hz1, getAtlasU(TEX_BASE, 13), getAtlasV(TEX_BASE, 6));

		// East (+X)
		t.vertexUV(hx1, hy1, hz0, getAtlasU(TEX_BASE, 3), getAtlasV(TEX_BASE, 0));
		t.vertexUV(hx1, hy1, hz1, getAtlasU(TEX_BASE, 13), getAtlasV(TEX_BASE, 0));
		t.vertexUV(hx1, hy0, hz1, getAtlasU(TEX_BASE, 13), getAtlasV(TEX_BASE, 6));
		t.vertexUV(hx1, hy0, hz0, getAtlasU(TEX_BASE, 3), getAtlasV(TEX_BASE, 6));
	} else {
		float hx0 = xf + 3.0f / 16.0f, hx1 = xf + 13.0f / 16.0f;
		float hz0 = zf + 0.0f, hz1 = zf + 1.0f;
		float hy0 = yf + 10.0f / 16.0f, hy1 = yf + 1.0f;

		// Head Up face (Length along Z from 0 to 16, width along X from 3 to 13)
		t.color(colUp, colUp, colUp);
		t.vertexUV(hx0, hy1, hz0, getAtlasU(TEX_TOP, 3), getAtlasV(TEX_TOP, 0));
		t.vertexUV(hx0, hy1, hz1, getAtlasU(TEX_TOP, 3), getAtlasV(TEX_TOP, 16));
		t.vertexUV(hx1, hy1, hz1, getAtlasU(TEX_TOP, 13), getAtlasV(TEX_TOP, 16));
		t.vertexUV(hx1, hy1, hz0, getAtlasU(TEX_TOP, 13), getAtlasV(TEX_TOP, 0));

		// Down
		t.color(colDown, colDown, colDown);
		t.vertexUV(hx0, hy0, hz1, getAtlasU(TEX_BASE, 3), getAtlasV(TEX_BASE, 16));
		t.vertexUV(hx0, hy0, hz0, getAtlasU(TEX_BASE, 3), getAtlasV(TEX_BASE, 0));
		t.vertexUV(hx1, hy0, hz0, getAtlasU(TEX_BASE, 13), getAtlasV(TEX_BASE, 0));
		t.vertexUV(hx1, hy0, hz1, getAtlasU(TEX_BASE, 13), getAtlasV(TEX_BASE, 16));

		// North (-Z)
		t.color(colNS, colNS, colNS);
		t.vertexUV(hx0, hy1, hz0, getAtlasU(TEX_BASE, 3), getAtlasV(TEX_BASE, 0));
		t.vertexUV(hx1, hy1, hz0, getAtlasU(TEX_BASE, 13), getAtlasV(TEX_BASE, 0));
		t.vertexUV(hx1, hy0, hz0, getAtlasU(TEX_BASE, 13), getAtlasV(TEX_BASE, 6));
		t.vertexUV(hx0, hy0, hz0, getAtlasU(TEX_BASE, 3), getAtlasV(TEX_BASE, 6));

		// South (+Z)
		t.vertexUV(hx1, hy1, hz1, getAtlasU(TEX_BASE, 13), getAtlasV(TEX_BASE, 0));
		t.vertexUV(hx0, hy1, hz1, getAtlasU(TEX_BASE, 3), getAtlasV(TEX_BASE, 0));
		t.vertexUV(hx0, hy0, hz1, getAtlasU(TEX_BASE, 3), getAtlasV(TEX_BASE, 6));
		t.vertexUV(hx1, hy0, hz1, getAtlasU(TEX_BASE, 13), getAtlasV(TEX_BASE, 6));

		// West (-X)
		t.color(colWE, colWE, colWE);
		t.vertexUV(hx0, hy1, hz1, getAtlasU(TEX_BASE, 16), getAtlasV(TEX_BASE, 0));
		t.vertexUV(hx0, hy1, hz0, getAtlasU(TEX_BASE, 0), getAtlasV(TEX_BASE, 0));
		t.vertexUV(hx0, hy0, hz0, getAtlasU(TEX_BASE, 0), getAtlasV(TEX_BASE, 6));
		t.vertexUV(hx0, hy0, hz1, getAtlasU(TEX_BASE, 16), getAtlasV(TEX_BASE, 6));

		// East (+X)
		t.vertexUV(hx1, hy1, hz0, getAtlasU(TEX_BASE, 0), getAtlasV(TEX_BASE, 0));
		t.vertexUV(hx1, hy1, hz1, getAtlasU(TEX_BASE, 16), getAtlasV(TEX_BASE, 0));
		t.vertexUV(hx1, hy0, hz1, getAtlasU(TEX_BASE, 16), getAtlasV(TEX_BASE, 6));
		t.vertexUV(hx1, hy0, hz0, getAtlasU(TEX_BASE, 0), getAtlasV(TEX_BASE, 6));
	}

	tt->setShape(0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f);
	return true;
}

bool TileRenderer::tesselateHopperInWorld(Tile* tt, int x, int y, int z) {
	Tesselator& t = Tesselator::instance;
	float br = tt->getBrightness(level, x, y, z);

	int tex = tt->getTexture(level, x, y, z, 0);
	bool isAlt = (tex & Tile::TEXTURE_ALT_FLAG) != 0;
	if (atlasFilter != -1) {
		if (atlasFilter == 0 && isAlt) return false;
		if (atlasFilter == 1 && !isAlt) return false;
	}

	int facing = level ? (level->getData(x, y, z)) : 0;

	const int TEX_TOP = 195;
	const int TEX_INSIDE = 199;
	const int TEX_OUTSIDE = 200;

	float colUp = 1.0f * br;
	float colDown = 0.5f * br;
	float colNS = 0.8f * br;
	float colWE = 0.6f * br;

	float xf = (float)x;
	float yf = (float)y;
	float zf = (float)z;

	auto renderBox = [&](float x0, float y0, float z0, float x1, float y1, float z1,
	                     int tUp, float uUp0, float vUp0, float uUp1, float vUp1,
	                     int tDown, float uD0, float vD0, float uD1, float vD1,
	                     int tNorth, float uN0, float vN0, float uN1, float vN1,
	                     int tSouth, float uS0, float vS0, float uS1, float vS1,
	                     int tWest, float uW0, float vW0, float uW1, float vW1,
	                     int tEast, float uE0, float vE0, float uE1, float vE1) {
		// Up (+Y)
		t.color(colUp, colUp, colUp);
		t.vertexUV(x0, y1, z0, getAtlasU(tUp, uUp0), getAtlasV(tUp, vUp0));
		t.vertexUV(x0, y1, z1, getAtlasU(tUp, uUp0), getAtlasV(tUp, vUp1));
		t.vertexUV(x1, y1, z1, getAtlasU(tUp, uUp1), getAtlasV(tUp, vUp1));
		t.vertexUV(x1, y1, z0, getAtlasU(tUp, uUp1), getAtlasV(tUp, vUp0));

		// Down (-Y)
		t.color(colDown, colDown, colDown);
		t.vertexUV(x0, y0, z1, getAtlasU(tDown, uD0), getAtlasV(tDown, vD1));
		t.vertexUV(x0, y0, z0, getAtlasU(tDown, uD0), getAtlasV(tDown, vD0));
		t.vertexUV(x1, y0, z0, getAtlasU(tDown, uD1), getAtlasV(tDown, vD0));
		t.vertexUV(x1, y0, z1, getAtlasU(tDown, uD1), getAtlasV(tDown, vD1));

		// North (-Z)
		t.color(colNS, colNS, colNS);
		t.vertexUV(x0, y1, z0, getAtlasU(tNorth, uN0), getAtlasV(tNorth, vN0));
		t.vertexUV(x1, y1, z0, getAtlasU(tNorth, uN1), getAtlasV(tNorth, vN0));
		t.vertexUV(x1, y0, z0, getAtlasU(tNorth, uN1), getAtlasV(tNorth, vN1));
		t.vertexUV(x0, y0, z0, getAtlasU(tNorth, uN0), getAtlasV(tNorth, vN1));

		// South (+Z)
		t.vertexUV(x1, y1, z1, getAtlasU(tSouth, uS1), getAtlasV(tSouth, vS0));
		t.vertexUV(x0, y1, z1, getAtlasU(tSouth, uS0), getAtlasV(tSouth, vS0));
		t.vertexUV(x0, y0, z1, getAtlasU(tSouth, uS0), getAtlasV(tSouth, vS1));
		t.vertexUV(x1, y0, z1, getAtlasU(tSouth, uS1), getAtlasV(tSouth, vS1));

		// West (-X)
		t.color(colWE, colWE, colWE);
		t.vertexUV(x0, y1, z1, getAtlasU(tWest, uW1), getAtlasV(tWest, vW0));
		t.vertexUV(x0, y1, z0, getAtlasU(tWest, uW0), getAtlasV(tWest, vW0));
		t.vertexUV(x0, y0, z0, getAtlasU(tWest, uW0), getAtlasV(tWest, vW1));
		t.vertexUV(x0, y0, z1, getAtlasU(tWest, uW1), getAtlasV(tWest, vW1));

		// East (+X)
		t.vertexUV(x1, y1, z0, getAtlasU(tEast, uE0), getAtlasV(tEast, vE0));
		t.vertexUV(x1, y1, z1, getAtlasU(tEast, uE1), getAtlasV(tEast, vE0));
		t.vertexUV(x1, y0, z1, getAtlasU(tEast, uE1), getAtlasV(tEast, vE1));
		t.vertexUV(x1, y0, z0, getAtlasU(tEast, uE0), getAtlasV(tEast, vE1));
	};

	// 1. Top Basin (y: 10/16..1.0)
	// Outer North Wall (Z: 0..2/16)
	t.color(colNS, colNS, colNS);
	t.vertexUV(xf + 0.0f, yf + 1.0f, zf + 0.0f, getAtlasU(TEX_OUTSIDE, 0), getAtlasV(TEX_OUTSIDE, 0));
	t.vertexUV(xf + 1.0f, yf + 1.0f, zf + 0.0f, getAtlasU(TEX_OUTSIDE, 16), getAtlasV(TEX_OUTSIDE, 0));
	t.vertexUV(xf + 1.0f, yf + 10.0f / 16.0f, zf + 0.0f, getAtlasU(TEX_OUTSIDE, 16), getAtlasV(TEX_OUTSIDE, 6));
	t.vertexUV(xf + 0.0f, yf + 10.0f / 16.0f, zf + 0.0f, getAtlasU(TEX_OUTSIDE, 0), getAtlasV(TEX_OUTSIDE, 6));
	// Inner South Wall (inside basin, facing South)
	t.vertexUV(xf + 14.0f / 16.0f, yf + 1.0f, zf + 2.0f / 16.0f, getAtlasU(TEX_INSIDE, 14), getAtlasV(TEX_INSIDE, 0));
	t.vertexUV(xf + 2.0f / 16.0f, yf + 1.0f, zf + 2.0f / 16.0f, getAtlasU(TEX_INSIDE, 2), getAtlasV(TEX_INSIDE, 0));
	t.vertexUV(xf + 2.0f / 16.0f, yf + 10.0f / 16.0f, zf + 2.0f / 16.0f, getAtlasU(TEX_INSIDE, 2), getAtlasV(TEX_INSIDE, 6));
	t.vertexUV(xf + 14.0f / 16.0f, yf + 10.0f / 16.0f, zf + 2.0f / 16.0f, getAtlasU(TEX_INSIDE, 14), getAtlasV(TEX_INSIDE, 6));
	// Top Rim North
	t.color(colUp, colUp, colUp);
	t.vertexUV(xf + 0.0f, yf + 1.0f, zf + 0.0f, getAtlasU(TEX_TOP, 0), getAtlasV(TEX_TOP, 0));
	t.vertexUV(xf + 0.0f, yf + 1.0f, zf + 2.0f / 16.0f, getAtlasU(TEX_TOP, 0), getAtlasV(TEX_TOP, 2));
	t.vertexUV(xf + 1.0f, yf + 1.0f, zf + 2.0f / 16.0f, getAtlasU(TEX_TOP, 16), getAtlasV(TEX_TOP, 2));
	t.vertexUV(xf + 1.0f, yf + 1.0f, zf + 0.0f, getAtlasU(TEX_TOP, 16), getAtlasV(TEX_TOP, 0));

	// Outer South Wall (Z: 14/16..1)
	t.color(colNS, colNS, colNS);
	t.vertexUV(xf + 1.0f, yf + 1.0f, zf + 1.0f, getAtlasU(TEX_OUTSIDE, 16), getAtlasV(TEX_OUTSIDE, 0));
	t.vertexUV(xf + 0.0f, yf + 1.0f, zf + 1.0f, getAtlasU(TEX_OUTSIDE, 0), getAtlasV(TEX_OUTSIDE, 0));
	t.vertexUV(xf + 0.0f, yf + 10.0f / 16.0f, zf + 1.0f, getAtlasU(TEX_OUTSIDE, 0), getAtlasV(TEX_OUTSIDE, 6));
	t.vertexUV(xf + 1.0f, yf + 10.0f / 16.0f, zf + 1.0f, getAtlasU(TEX_OUTSIDE, 16), getAtlasV(TEX_OUTSIDE, 6));
	// Inner North Wall (inside basin, facing North)
	t.vertexUV(xf + 2.0f / 16.0f, yf + 1.0f, zf + 14.0f / 16.0f, getAtlasU(TEX_INSIDE, 2), getAtlasV(TEX_INSIDE, 0));
	t.vertexUV(xf + 14.0f / 16.0f, yf + 1.0f, zf + 14.0f / 16.0f, getAtlasU(TEX_INSIDE, 14), getAtlasV(TEX_INSIDE, 0));
	t.vertexUV(xf + 14.0f / 16.0f, yf + 10.0f / 16.0f, zf + 14.0f / 16.0f, getAtlasU(TEX_INSIDE, 14), getAtlasV(TEX_INSIDE, 6));
	t.vertexUV(xf + 2.0f / 16.0f, yf + 10.0f / 16.0f, zf + 14.0f / 16.0f, getAtlasU(TEX_INSIDE, 2), getAtlasV(TEX_INSIDE, 6));
	// Top Rim South
	t.color(colUp, colUp, colUp);
	t.vertexUV(xf + 0.0f, yf + 1.0f, zf + 14.0f / 16.0f, getAtlasU(TEX_TOP, 0), getAtlasV(TEX_TOP, 14));
	t.vertexUV(xf + 0.0f, yf + 1.0f, zf + 1.0f, getAtlasU(TEX_TOP, 0), getAtlasV(TEX_TOP, 16));
	t.vertexUV(xf + 1.0f, yf + 1.0f, zf + 1.0f, getAtlasU(TEX_TOP, 16), getAtlasV(TEX_TOP, 16));
	t.vertexUV(xf + 1.0f, yf + 1.0f, zf + 14.0f / 16.0f, getAtlasU(TEX_TOP, 16), getAtlasV(TEX_TOP, 14));

	// Outer West Wall (X: 0..2/16)
	t.color(colWE, colWE, colWE);
	t.vertexUV(xf + 0.0f, yf + 1.0f, zf + 1.0f, getAtlasU(TEX_OUTSIDE, 16), getAtlasV(TEX_OUTSIDE, 0));
	t.vertexUV(xf + 0.0f, yf + 1.0f, zf + 0.0f, getAtlasU(TEX_OUTSIDE, 0), getAtlasV(TEX_OUTSIDE, 0));
	t.vertexUV(xf + 0.0f, yf + 10.0f / 16.0f, zf + 0.0f, getAtlasU(TEX_OUTSIDE, 0), getAtlasV(TEX_OUTSIDE, 6));
	t.vertexUV(xf + 0.0f, yf + 10.0f / 16.0f, zf + 1.0f, getAtlasU(TEX_OUTSIDE, 16), getAtlasV(TEX_OUTSIDE, 6));
	// Inner East Wall (inside basin, facing East)
	t.vertexUV(xf + 2.0f / 16.0f, yf + 1.0f, zf + 0.0f + 2.0f / 16.0f, getAtlasU(TEX_INSIDE, 2), getAtlasV(TEX_INSIDE, 0));
	t.vertexUV(xf + 2.0f / 16.0f, yf + 1.0f, zf + 14.0f / 16.0f, getAtlasU(TEX_INSIDE, 14), getAtlasV(TEX_INSIDE, 0));
	t.vertexUV(xf + 2.0f / 16.0f, yf + 10.0f / 16.0f, zf + 14.0f / 16.0f, getAtlasU(TEX_INSIDE, 14), getAtlasV(TEX_INSIDE, 6));
	t.vertexUV(xf + 2.0f / 16.0f, yf + 10.0f / 16.0f, zf + 2.0f / 16.0f, getAtlasU(TEX_INSIDE, 2), getAtlasV(TEX_INSIDE, 6));
	// Top Rim West
	t.color(colUp, colUp, colUp);
	t.vertexUV(xf + 0.0f, yf + 1.0f, zf + 2.0f / 16.0f, getAtlasU(TEX_TOP, 0), getAtlasV(TEX_TOP, 2));
	t.vertexUV(xf + 0.0f, yf + 1.0f, zf + 14.0f / 16.0f, getAtlasU(TEX_TOP, 0), getAtlasV(TEX_TOP, 14));
	t.vertexUV(xf + 2.0f / 16.0f, yf + 1.0f, zf + 14.0f / 16.0f, getAtlasU(TEX_TOP, 2), getAtlasV(TEX_TOP, 14));
	t.vertexUV(xf + 2.0f / 16.0f, yf + 1.0f, zf + 2.0f / 16.0f, getAtlasU(TEX_TOP, 2), getAtlasV(TEX_TOP, 2));

	// Outer East Wall (X: 14/16..1)
	t.color(colWE, colWE, colWE);
	t.vertexUV(xf + 1.0f, yf + 1.0f, zf + 0.0f, getAtlasU(TEX_OUTSIDE, 0), getAtlasV(TEX_OUTSIDE, 0));
	t.vertexUV(xf + 1.0f, yf + 1.0f, zf + 1.0f, getAtlasU(TEX_OUTSIDE, 16), getAtlasV(TEX_OUTSIDE, 0));
	t.vertexUV(xf + 1.0f, yf + 10.0f / 16.0f, zf + 1.0f, getAtlasU(TEX_OUTSIDE, 16), getAtlasV(TEX_OUTSIDE, 6));
	t.vertexUV(xf + 1.0f, yf + 10.0f / 16.0f, zf + 0.0f, getAtlasU(TEX_OUTSIDE, 0), getAtlasV(TEX_OUTSIDE, 6));
	// Inner West Wall (inside basin, facing West)
	t.vertexUV(xf + 14.0f / 16.0f, yf + 1.0f, zf + 14.0f / 16.0f, getAtlasU(TEX_INSIDE, 14), getAtlasV(TEX_INSIDE, 0));
	t.vertexUV(xf + 14.0f / 16.0f, yf + 1.0f, zf + 2.0f / 16.0f, getAtlasU(TEX_INSIDE, 2), getAtlasV(TEX_INSIDE, 0));
	t.vertexUV(xf + 14.0f / 16.0f, yf + 10.0f / 16.0f, zf + 2.0f / 16.0f, getAtlasU(TEX_INSIDE, 2), getAtlasV(TEX_INSIDE, 6));
	t.vertexUV(xf + 14.0f / 16.0f, yf + 10.0f / 16.0f, zf + 14.0f / 16.0f, getAtlasU(TEX_INSIDE, 14), getAtlasV(TEX_INSIDE, 6));
	// Top Rim East
	t.color(colUp, colUp, colUp);
	t.vertexUV(xf + 14.0f / 16.0f, yf + 1.0f, zf + 2.0f / 16.0f, getAtlasU(TEX_TOP, 14), getAtlasV(TEX_TOP, 2));
	t.vertexUV(xf + 14.0f / 16.0f, yf + 1.0f, zf + 14.0f / 16.0f, getAtlasU(TEX_TOP, 14), getAtlasV(TEX_TOP, 14));
	t.vertexUV(xf + 1.0f, yf + 1.0f, zf + 14.0f / 16.0f, getAtlasU(TEX_TOP, 16), getAtlasV(TEX_TOP, 14));
	t.vertexUV(xf + 1.0f, yf + 1.0f, zf + 2.0f / 16.0f, getAtlasU(TEX_TOP, 16), getAtlasV(TEX_TOP, 2));

	// Basin Floor (y = 10/16)
	t.color(colUp, colUp, colUp);
	t.vertexUV(xf + 2.0f / 16.0f, yf + 10.0f / 16.0f, zf + 2.0f / 16.0f, getAtlasU(TEX_INSIDE, 2), getAtlasV(TEX_INSIDE, 2));
	t.vertexUV(xf + 2.0f / 16.0f, yf + 10.0f / 16.0f, zf + 14.0f / 16.0f, getAtlasU(TEX_INSIDE, 2), getAtlasV(TEX_INSIDE, 14));
	t.vertexUV(xf + 14.0f / 16.0f, yf + 10.0f / 16.0f, zf + 14.0f / 16.0f, getAtlasU(TEX_INSIDE, 14), getAtlasV(TEX_INSIDE, 14));
	t.vertexUV(xf + 14.0f / 16.0f, yf + 10.0f / 16.0f, zf + 2.0f / 16.0f, getAtlasU(TEX_INSIDE, 14), getAtlasV(TEX_INSIDE, 2));

	t.color(colDown, colDown, colDown);
	t.vertexUV(xf + 0.0f, yf + 10.0f / 16.0f, zf + 1.0f, getAtlasU(TEX_OUTSIDE, 0), getAtlasV(TEX_OUTSIDE, 16));
	t.vertexUV(xf + 0.0f, yf + 10.0f / 16.0f, zf + 0.0f, getAtlasU(TEX_OUTSIDE, 0), getAtlasV(TEX_OUTSIDE, 0));
	t.vertexUV(xf + 1.0f, yf + 10.0f / 16.0f, zf + 0.0f, getAtlasU(TEX_OUTSIDE, 16), getAtlasV(TEX_OUTSIDE, 0));
	t.vertexUV(xf + 1.0f, yf + 10.0f / 16.0f, zf + 1.0f, getAtlasU(TEX_OUTSIDE, 16), getAtlasV(TEX_OUTSIDE, 16));

	// 2. Middle Funnel (8x6x8 centered at [4..12, 4..10, 4..12])
	renderBox(xf + 4.0f / 16.0f, yf + 4.0f / 16.0f, zf + 4.0f / 16.0f,
	          xf + 12.0f / 16.0f, yf + 10.0f / 16.0f, zf + 12.0f / 16.0f,
	          TEX_OUTSIDE, 4, 4, 12, 12,
	          TEX_OUTSIDE, 4, 4, 12, 12,
	          TEX_OUTSIDE, 4, 6, 12, 12,
	          TEX_OUTSIDE, 4, 6, 12, 12,
	          TEX_OUTSIDE, 4, 6, 12, 12,
	          TEX_OUTSIDE, 4, 6, 12, 12);

	// 3. Spout
	float sx0, sx1, sy0, sy1, sz0, sz1;
	if (facing == 2) { // North
		sx0 = xf + 6.0f / 16.0f; sx1 = xf + 10.0f / 16.0f;
		sy0 = yf + 4.0f / 16.0f; sy1 = yf + 8.0f / 16.0f;
		sz0 = zf + 0.0f;         sz1 = zf + 4.0f / 16.0f;
	} else if (facing == 3) { // South
		sx0 = xf + 6.0f / 16.0f; sx1 = xf + 10.0f / 16.0f;
		sy0 = yf + 4.0f / 16.0f; sy1 = yf + 8.0f / 16.0f;
		sz0 = zf + 12.0f / 16.0f; sz1 = zf + 1.0f;
	} else if (facing == 4) { // West
		sx0 = xf + 0.0f;         sx1 = xf + 4.0f / 16.0f;
		sy0 = yf + 4.0f / 16.0f; sy1 = yf + 8.0f / 16.0f;
		sz0 = zf + 6.0f / 16.0f; sz1 = zf + 10.0f / 16.0f;
	} else if (facing == 5) { // East
		sx0 = xf + 12.0f / 16.0f; sx1 = xf + 1.0f;
		sy0 = yf + 4.0f / 16.0f; sy1 = yf + 8.0f / 16.0f;
		sz0 = zf + 6.0f / 16.0f; sz1 = zf + 10.0f / 16.0f;
	} else { // Down (0 or default)
		sx0 = xf + 6.0f / 16.0f; sx1 = xf + 10.0f / 16.0f;
		sy0 = yf + 0.0f;         sy1 = yf + 4.0f / 16.0f;
		sz0 = zf + 6.0f / 16.0f; sz1 = zf + 10.0f / 16.0f;
	}

	renderBox(sx0, sy0, sz0, sx1, sy1, sz1,
	          TEX_OUTSIDE, 6, 6, 10, 10,
	          TEX_OUTSIDE, 6, 6, 10, 10,
	          TEX_OUTSIDE, 6, 12, 10, 16,
	          TEX_OUTSIDE, 6, 12, 10, 16,
	          TEX_OUTSIDE, 6, 12, 10, 16,
	          TEX_OUTSIDE, 6, 12, 10, 16);

	tt->setShape(0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f);
	return true;
}



