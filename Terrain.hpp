// terrain data and drawing
#ifndef Terrain_hpp
#define Terrain_hpp

#include "Vec.hpp"
#include "Shader.hpp"

// terrain data and rendering methods
class Terrain {
// private data
private:
	unsigned int repl;			// ammount of replication in both x and y directions
    Vec3f gridSize;             // elevation grid size
    Vec3f mapSize;              // size of terrain in world space
    Vec2f walkableSize;         // size of walkable terrain in world space

    unsigned int numvert;       // total vertices
    Vec3f *vert;                // per-vertex position
    Vec3f *dPdu, *dPdv;         // per-vertex tangents
    Vec3f *norm;                // per-vertex normal
    Vec2f *texcoord;            // per-vertex texture coordinate
    
    unsigned int numtri;        // total triangles
    Vec<unsigned int, 3> *indices; // 3 vertex indices per triangle

    // GL vertex array object IDs
    enum {TERRAIN_VARRAY, NUM_VARRAYS};
    unsigned int varrayIDs[NUM_VARRAYS];

    // GL texture IDs
    enum {COLOR_TEXTURE, NORMAL_TEXTURE, GLOSS_TEXTURE, NUM_TEXTURES};
    unsigned int textureIDs[NUM_TEXTURES];

    // GL buffer object IDs
    enum {POSITION_BUFFER, TANGENT_BUFFER, BITANGENT_BUFFER, NORMAL_BUFFER, 
          UV_BUFFER, INDEX_BUFFER, NUM_BUFFERS};
    unsigned int bufferIDs[NUM_BUFFERS];

    // GL shaders
    unsigned int shaderID;      // ID for shader program
    ShaderInfo shaderParts[2];  // vertex & fragment shader info

// public methods
public:
    // load terrain, given elevation image and surface texture
    Terrain(const char *elevationPPM, const char *texturePPM,
            const char *normalPPM, const char *glossPPM);

    // clean up allocated memory
    ~Terrain();

    // load/reload a texture
    void updateTexture(const char *ppm, unsigned int textureID);

    // load/reload shaders
    void updateShaders();

    // draw this terrain object
    void draw() const;

	// determine elevation at point x, y
	void getElevation(float x, float y, float &e, float &t_xz, float &t_yz);
};

#endif
