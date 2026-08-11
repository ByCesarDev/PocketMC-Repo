#include "Tesselator.h"
#include <cstdio>
#include <cstring>
#include <algorithm>

Tesselator Tesselator::instance(sizeof(GLfloat) * MAX_FLOATS); // max size in bytes

const int VertexSizeBytes = sizeof(VERTEX);

Tesselator::Tesselator( int size )
	:	size(size),
	vertices(0),
	u(0), v(0),
	_color(0),
	hasColor(false),
	hasTexture(false),
	hasNormal(false),
	p(0),
	count(0),
	_noColor(false),
	mode(0),
	xo(0), yo(0), zo(0),
	_nx(0), _ny(0), _nz(0),
	_sx(1), _sy(1),

	tesselating(false),
	vboMode(false),
	vboId(-1),
	vboCounts(128),
	totalSize(0),
	accessMode(ACCESS_STATIC),
	maxVertices(size / sizeof(VERTEX)),
	_voidBeginEnd(false)
{
	vboIds = new GLuint[vboCounts];
	vaoIds = new GLuint[vboCounts];

	_varray = new VERTEX[maxVertices];

	char* a = (char*)&_varray[0];
	char* b = (char*)&_varray[1];
	LOGI("Vsize: %lu, %d\n", (unsigned long)sizeof(VERTEX), (int)(b - a));
}

Tesselator::~Tesselator()
{
	delete[] vboIds;
	delete[] vaoIds;
	delete[] _varray;
}

void Tesselator::init()
{
#ifndef STANDALONE_SERVER
	glGenBuffers2(vboCounts, vboIds);
#endif
}

void Tesselator::clear()
{
	accessMode = ACCESS_STATIC;
	vertices = 0;
	count = 0;
	p = 0;
	_voidBeginEnd = false;
}

int Tesselator::getVboCount() {
	return vboCounts;
}

void Tesselator::setupVertexAttributes()
{
#ifndef STANDALONE_SERVER
	const int stride = VertexSizeBytes;

	// Location 0: a_Position (vec3 float)
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)0);
	glEnableVertexAttribArray(0);

	// Location 1: a_TexCoord (vec2 float)
	if (hasTexture) {
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, stride, (void*)(3 * sizeof(float)));
		glEnableVertexAttribArray(1);
	} else {
		glDisableVertexAttribArray(1);
		glVertexAttrib2f(1, 0.0f, 0.0f);
	}

	// Location 2: a_Color (vec4 ubyte normalized)
	if (hasColor) {
		glVertexAttribPointer(2, 4, GL_UNSIGNED_BYTE, GL_TRUE, stride, (void*)(5 * sizeof(float)));
		glEnableVertexAttribArray(2);
	} else {
		glDisableVertexAttribArray(2);
		glVertexAttrib4f(2, 1.0f, 1.0f, 1.0f, 1.0f);
	}

	// Location 3: a_Normal (vec3 float)
	if (hasNormal) {
		glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, stride, (void*)(6 * sizeof(float)));
		glEnableVertexAttribArray(3);
	} else {
		glDisableVertexAttribArray(3);
		glVertexAttrib3f(3, 0.0f, 1.0f, 0.0f);
	}

	// Sync fixed-function pointer arrays for backwards compatibility
	if (hasTexture) {
		glTexCoordPointer2(2, GL_FLOAT, stride, (GLvoid*)(3 * sizeof(float)));
		glEnableClientState2(GL_TEXTURE_COORD_ARRAY);
	}
	if (hasColor) {
		glColorPointer2(4, GL_UNSIGNED_BYTE, stride, (GLvoid*)(5 * sizeof(float)));
		glEnableClientState2(GL_COLOR_ARRAY);
	}
	if (hasNormal) {
		glNormalPointer(GL_FLOAT, stride, (GLvoid*)(6 * sizeof(float)));
		glEnableClientState2(GL_NORMAL_ARRAY);
	}
	glVertexPointer2(3, GL_FLOAT, stride, 0);
	glEnableClientState2(GL_VERTEX_ARRAY);
#endif
}

void Tesselator::disableVertexAttributes()
{
#ifndef STANDALONE_SERVER
	glDisableVertexAttribArray(0);
	if (hasTexture) glDisableVertexAttribArray(1);
	if (hasColor) glDisableVertexAttribArray(2);
	if (hasNormal) glDisableVertexAttribArray(3);

	glDisableClientState2(GL_VERTEX_ARRAY);
	if (hasTexture) glDisableClientState2(GL_TEXTURE_COORD_ARRAY);
	if (hasColor) glDisableClientState2(GL_COLOR_ARRAY);
	if (hasNormal) glDisableClientState2(GL_NORMAL_ARRAY);
#endif
}

RenderChunk Tesselator::end( bool useMine, int bufferId )
{
#ifndef STANDALONE_SERVER
	if (!tesselating)
		LOGI("not tesselating!\n");

	if (!tesselating || _voidBeginEnd) return RenderChunk();

	tesselating = false;
	const int o_vertices = vertices;

	if (vertices > 0) {
		if (p <= 0 || p > maxVertices) { clear(); return RenderChunk(); }
		int bytes = p * sizeof(VERTEX);
		if (bytes <= 0) return RenderChunk();
		if (++vboId >= vboCounts)
			vboId = 0;

#ifdef USE_VBO
		if (!useMine) {
			bufferId = vboIds[vboId];
		}
#else
		bufferId = vboIds[vboId];
#endif
		int access = GL_STATIC_DRAW;
		glBindBuffer2(GL_ARRAY_BUFFER, bufferId);
		glBufferData2(GL_ARRAY_BUFFER, bytes, _varray, access);

		totalSize += bytes;

#ifndef USE_VBO
		setupVertexAttributes();

		if (mode == GL_QUADS) {
			glDrawArrays2(GL_TRIANGLES, 0, vertices);
		} else {
			glDrawArrays2(mode, 0, vertices);
		}

		disableVertexAttributes();
#endif /*!USE_VBO*/
	}

	clear();
	RenderChunk out(bufferId, o_vertices);
	return out;
#else
	return RenderChunk();
#endif
}

void Tesselator::begin( int mode )
{
	if (tesselating || _voidBeginEnd) {
		if (tesselating && !_voidBeginEnd)
			LOGI("already tesselating!\n");
		return;
	}
	tesselating = true;

	clear();
	this->mode = mode;
	hasNormal = false;
	hasColor = false;
	hasTexture = false;
	_noColor = false;
}

void Tesselator::begin()
{
	begin(GL_QUADS);
}

void Tesselator::tex( float u, float v )
{
	hasTexture = true;
	this->u = u;
	this->v = v;
}

int Tesselator::getColor() {
	return _color;
}

void Tesselator::color( float r, float g, float b )
{
	color((int) (r * 255), (int) (g * 255), (int) (b * 255));
}

void Tesselator::color( float r, float g, float b, float a )
{
	color((int) (r * 255), (int) (g * 255), (int) (b * 255), (int) (a * 255));
}

void Tesselator::color( int r, int g, int b )
{
	color(r, g, b, 255);
}

void Tesselator::color( int r, int g, int b, int a )
{
	if (_noColor) return;

	if (r > 255) r = 255;
	if (g > 255) g = 255;
	if (b > 255) b = 255;
	if (a > 255) a = 255;
	if (r < 0) r = 0;
	if (g < 0) g = 0;
	if (b < 0) b = 0;
	if (a < 0) a = 0;

	hasColor = true;
	_color = (a << 24) | (b << 16) | (g << 8) | (r);
}

void Tesselator::color( char r, char g, char b )
{
	color(r & 0xff, g & 0xff, b & 0xff);
}

void Tesselator::color( int c )
{
	int r = ((c >> 16) & 255);
	int g = ((c >> 8) & 255);
	int b = ((c) & 255);
	color(r, g, b);
}

void Tesselator::colorABGR( int c )
{
	if (_noColor) return;
	hasColor = true;
	_color = c;
}

void Tesselator::color( int c, int alpha )
{
	int r = ((c >> 16) & 255);
	int g = ((c >> 8) & 255);
	int b = ((c) & 255);
	color(r, g, b, alpha);
}

void Tesselator::vertexUV( float x, float y, float z, float u, float v )
{
	tex(u, v);
	vertex(x, y, z);
}

void Tesselator::scale2d(float sx, float sy) {
	_sx *= sx;
	_sy *= sy;
}

void Tesselator::resetScale() {
	_sx = _sy = 1;
}

void Tesselator::vertex( float x, float y, float z )
{
#ifndef STANDALONE_SERVER
	count++;

	if (mode == GL_QUADS && (count & 3) == 0) {
		for (int i = 0; i < 2; i++) {

			const int offs = 3 - i;
			if (p - offs < 0 || p >= maxVertices) { clear(); return; }
			VERTEX& src = _varray[p - offs];
			VERTEX& dst = _varray[p];

			if (hasTexture) {
				dst.u = src.u;
				dst.v = src.v;
			}
			if (hasColor) {
				dst.color = src.color;
			}
			if (hasNormal) {
				dst.nx = src.nx;
				dst.ny = src.ny;
				dst.nz = src.nz;
			}

			dst.x = src.x;
			dst.y = src.y;
			dst.z = src.z;

			++vertices;
			++p;
		}
	}

	if (p < 0 || p >= maxVertices) { clear(); return; }
	VERTEX& vertex = _varray[p];

	if (hasTexture) {
		vertex.u = u;
		vertex.v = v;
	}
	if (hasColor) {
		vertex.color = _color;
	}
	if (hasNormal) {
		vertex.nx = _nx;
		vertex.ny = _ny;
		vertex.nz = _nz;
	}

	vertex.x = _sx * (x + xo);
	vertex.y = _sy * (y + yo);
	vertex.z = z + zo;

	++p;
	++vertices;

	if ((vertices & 3) == 0 && p >= maxVertices-1) {
		for (int i = 0; i < 3; ++i)
			printf("Overwriting the vertex buffer! This chunk/entity won't show up\n");
		clear();
	}
#endif
}

void Tesselator::noColor()
{
	_noColor = true;
}

void Tesselator::setAccessMode(int mode)
{
	accessMode = mode;
}

void Tesselator::normal( float x, float y, float z )
{
	if (!tesselating) printf("But..");
	hasNormal = true;
	this->_nx = x;
	this->_ny = y;
	this->_nz = z;
}

void Tesselator::offset( float xo, float yo, float zo ) {
	this->xo = xo;
	this->yo = yo;
	this->zo = zo;
}

void Tesselator::addOffset( float x, float y, float z ) {
	xo += x;
	yo += y;
	zo += z;
}

void Tesselator::offset( const Vec3& v ) {
	xo = v.x;
	yo = v.y;
	zo = v.z;
}

void Tesselator::addOffset( const Vec3& v ) {
	xo += v.x;
	yo += v.y;
	zo += v.z;
}

void Tesselator::draw()
{
#ifndef STANDALONE_SERVER
	if (!tesselating)
		LOGI("not (draw) tesselating!\n");

	if (!tesselating || _voidBeginEnd)
		return;

	tesselating = false;

	if (vertices > 0) {
		if (p <= 0 || p > maxVertices) { clear(); return; }
		int bytes = p * sizeof(VERTEX);
		if (bytes <= 0) { clear(); return; }
		if (++vboId >= vboCounts)
			vboId = 0;

		int bufferId = vboIds[vboId];

		int access = GL_DYNAMIC_DRAW;
		glBindBuffer2(GL_ARRAY_BUFFER, bufferId);
		glBufferData2(GL_ARRAY_BUFFER, bytes, _varray, access);

		setupVertexAttributes();

		if (mode == GL_QUADS) {
			glDrawArrays2(GL_TRIANGLES, 0, vertices);
		} else {
			glDrawArrays2(mode, 0, vertices);
		}

		disableVertexAttributes();
	}

	clear();
#endif
}

void Tesselator::voidBeginAndEndCalls(bool doVoid) {
	_voidBeginEnd = doVoid;
}

void Tesselator::enableColor() {
	_noColor = false;
}
