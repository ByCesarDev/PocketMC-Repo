#ifndef NET_MINECRAFT_CLIENT_RENDERER__Tesselator_H__
#define NET_MINECRAFT_CLIENT_RENDERER__Tesselator_H__

//package net.minecraft.client.renderer;

#include <map>
#include "RenderChunk.h"
#include "gles.h"
#include "VertecDecl.h"

extern const int VertexSizeBytes;

typedef VertexDeclPTCN VERTEX;
typedef std::map<GLuint, GLsizei> IntGLMap;


class Tesselator
{
    static const int MAX_MEMORY_USE = 16 * 1024 * 1024;
    static const int MAX_FLOATS = MAX_MEMORY_USE / 4 / 2;

	Tesselator(int size);

public:
	static const int ACCESS_DYNAMIC = 1;
	static const int ACCESS_STATIC = 2;

	static Tesselator instance;

	~Tesselator();

	void init();
    void clear();

    void begin();
    void begin(int mode);
	void draw();
	RenderChunk end(bool useMine, int bufferId);

	void color(int c);
	void color(int c, int alpha);
    void color(float r, float g, float b);
    void color(float r, float g, float b, float a);
    void color(int r, int g, int b);
    void color(int r, int g, int b, int a);
    void color(char r, char g, char b);
	void colorABGR( int c );

	void normal(float x, float y, float z);
	void voidBeginAndEndCalls(bool doVoid);
	
	void tex(float u, float v);
    
	void vertex(float x, float y, float z);
	void vertexUV(float x, float y, float z, float u, float v);
	
	void scale2d(float x, float y);
	void resetScale();

    void noColor();
	void enableColor();
private:
	void setAccessMode(int mode);
public:

    void offset(float xo, float yo, float zo);
	void offset(const Vec3& v);
    void addOffset(float x, float y, float z);
	void addOffset(const Vec3& v);

	int getVboCount();

	int getColor();

	__inline void beginOverride() {
		begin();
		voidBeginAndEndCalls(true);
	}
	__inline void endOverrideAndDraw() {
		voidBeginAndEndCalls(false);
		draw();
	}
	__inline bool isOverridden() {
		return _voidBeginEnd;
	}
	__inline bool isTesselating() const {
		return tesselating;
	}
	__inline RenderChunk endOverride(int bufferId) {
		voidBeginAndEndCalls(false);
		return end(true, bufferId);
	}

private:
	Tesselator(const Tesselator& rhs);
	Tesselator& operator=(const Tesselator& rhs);
	VERTEX* _varray = nullptr;
	int vertices = 0;
	float xo = 0, yo = 0, zo = 0;
	float u = 0, v = 0;
	unsigned int _color = 0;
	float _nx = 0, _ny = 0, _nz = 0; 
	float _sx = 1.0f, _sy = 1.0f;
	bool hasColor = false;
	bool hasTexture = false;
	bool hasNormal = false;
	bool _noColor = false;
	bool _voidBeginEnd = false;
	int p = 0;
	int count = 0;
	bool tesselating = false;
	bool vboMode = false;
	int vboCounts = 0;
	int vboId = -1;
	GLuint* vboIds = nullptr;
	GLuint defaultVao = 0;
	GLuint* vaoIds = nullptr;
	int size = 0;
	int totalSize = 0;
	int maxVertices = 0;
	int mode = 0;
	int accessMode = 0;
	IntGLMap map;

	void setupVertexAttributes();
	void disableVertexAttributes();
};

#endif /*NET_MINECRAFT_CLIENT_RENDERER__Tesselator_H__*/
