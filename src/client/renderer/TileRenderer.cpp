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
#include "EntityTileRenderer.h"

bool TileRenderer::sideTinting = true;

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
	t.color( br, br, br );

	int xt = ((tex & 0xf) << 4);
	int yt = tex & 0xf0;

	float u0 = (xt) / 256.0f;
	float u1 = (xt + 15.99f) / 256.0f;
	float v0 = (yt) / 256.0f;
	float v1 = (yt + 15.99f) / 256.0f;
	float h = 1.4f;

	if ( level->isSolidBlockingTile( x, y - 1, z ) || Tile::fire->canBurn( level, x, y - 1, z ) )
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
	const float epsilon = 0.5f / atlasSize;

	float u0 = (xt + tt->xx0 * 16.0f) / atlasSize + epsilon;
	float u1 = (xt + tt->xx1 * 16.0f) / atlasSize - epsilon;
	float v0 = (yt + tt->zz0 * 16.0f) / atlasSize + epsilon;
	float v1 = (yt + tt->zz1 * 16.0f) / atlasSize - epsilon;

	if (tt->xx0 < 0 || tt->xx1 > 1) {
		u0 = (xt + 0.5f) / atlasSize;
		u1 = (xt + 15.5f) / atlasSize;
	}
	if (tt->zz0 < 0 || tt->zz1 > 1) {
		v0 = (yt + 0.5f) / atlasSize;
		v1 = (yt + 15.5f) / atlasSize;
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
	const float epsilon = 0.5f / atlasSize;

	float u0 = (xt + tt->xx0 * 16.0f) / atlasSize + epsilon;
	float u1 = (xt + tt->xx1 * 16.0f) / atlasSize - epsilon;
	float v0 = (yt + tt->zz0 * 16.0f) / atlasSize + epsilon;
	float v1 = (yt + tt->zz1 * 16.0f) / atlasSize - epsilon;

	if (tt->xx0 < 0 || tt->xx1 > 1) {
		u0 = (xt + 0.5f) / atlasSize;
		u1 = (xt + 15.5f) / atlasSize;
	}
	if (tt->zz0 < 0 || tt->zz1 > 1) {
		v0 = (yt + 0.5f) / atlasSize;
		v1 = (yt + 15.5f) / atlasSize;
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
	const float epsilon = 0.5f / atlasSize;

	float u0 = (xt + tt->xx0 * 16.0f) / atlasSize + epsilon;
	float u1 = (xt + tt->xx1 * 16.0f) / atlasSize - epsilon;
	float v0 = (yt + 16.0f - tt->yy1 * 16.0f) / atlasSize + epsilon;
	float v1 = (yt + 16.0f - tt->yy0 * 16.0f) / atlasSize - epsilon;
	if (xFlipTexture) {
		float tmp = u0;
		u0 = u1;
		u1 = tmp;
	}

	if (tt->xx0 < 0 || tt->xx1 > 1) {
		u0 = (xt + 0.5f) / atlasSize;
		u1 = (xt + 15.5f) / atlasSize;
	}
	if (tt->yy0 < 0 || tt->yy1 > 1) {
		v0 = (yt + 0.5f) / atlasSize;
		v1 = (yt + 15.5f) / atlasSize;
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
	const float epsilon = 0.5f / atlasSize;

	float u0 = (xt + tt->xx0 * 16.0f) / atlasSize + epsilon;
	float u1 = (xt + tt->xx1 * 16.0f) / atlasSize - epsilon;
	float v0 = (yt + 16.0f - tt->yy1 * 16.0f) / atlasSize + epsilon;
	float v1 = (yt + 16.0f - tt->yy0 * 16.0f) / atlasSize - epsilon;
	if (xFlipTexture) {
		float tmp = u0;
		u0 = u1;
		u1 = tmp;
	}

	if (tt->xx0 < 0 || tt->xx1 > 1) {
		u0 = (xt + 0.5f) / atlasSize;
		u1 = (xt + 15.5f) / atlasSize;
	}
	if (tt->yy0 < 0 || tt->yy1 > 1) {
		v0 = (yt + 0.5f) / atlasSize;
		v1 = (yt + 15.5f) / atlasSize;
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
	const float epsilon = 0.5f / atlasSize;

	float u0 = (xt + tt->zz0 * 16.0f) / atlasSize + epsilon;
	float u1 = (xt + tt->zz1 * 16.0f) / atlasSize - epsilon;
	float v0 = (yt + 16.0f - tt->yy1 * 16.0f) / atlasSize + epsilon;
	float v1 = (yt + 16.0f - tt->yy0 * 16.0f) / atlasSize - epsilon;
	if (xFlipTexture) {
		float tmp = u0;
		u0 = u1;
		u1 = tmp;
	}

	if (tt->zz0 < 0 || tt->zz1 > 1) {
		u0 = (xt + 0.5f) / atlasSize;
		u1 = (xt + 15.5f) / atlasSize;
	}
	if (tt->yy0 < 0 || tt->yy1 > 1) {
		v0 = (yt + 0.5f) / atlasSize;
		v1 = (yt + 15.5f) / atlasSize;
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
	const float epsilon = 0.5f / atlasSize;

	float u0 = (xt + tt->zz0 * 16.0f) / atlasSize + epsilon;
	float u1 = (xt + tt->zz1 * 16.0f) / atlasSize - epsilon;
	float v0 = (yt + 16.0f - tt->yy1 * 16.0f) / atlasSize + epsilon;
	float v1 = (yt + 16.0f - tt->yy0 * 16.0f) / atlasSize - epsilon;
	if (xFlipTexture) {
		float tmp = u0;
		u0 = u1;
		u1 = tmp;
	}

	if (tt->zz0 < 0 || tt->zz1 > 1) {
		u0 = (xt + 0.5f) / atlasSize;
		u1 = (xt + 15.5f) / atlasSize;
	}
	if (tt->yy0 < 0 || tt->yy1 > 1) {
		v0 = (yt + 0.5f) / atlasSize;
		v1 = (yt + 15.5f) / atlasSize;
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
	}

}

bool TileRenderer::canRender( int renderShape )
{
	if (renderShape == Tile::SHAPE_BLOCK) return true;
	if (renderShape == Tile::SHAPE_CACTUS) return true;
	if (renderShape == Tile::SHAPE_STAIRS) return true;
	if (renderShape == Tile::SHAPE_FENCE) return true;
	if (renderShape == Tile::SHAPE_FENCE_GATE) return true;
	//if (renderShape == Tile::SHAPE_CROSS_TEXTURE) return true;
	//if (renderShape == Tile::SHAPE_ENTITYTILE_ANIMATED) return true;

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
