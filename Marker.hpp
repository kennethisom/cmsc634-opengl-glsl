// octahedron signaling light location
#ifndef Marker_hpp
#define Marker_hpp

#include "Vec.hpp"
#include "MatPair.hpp"
#include "Shader.hpp"

// tetrahedron data and rendering methods
class Marker {
// private data
private:
    unsigned int numvert;       // total vertices
    Vec3f vert[6];              // per-vertex position
    
    unsigned int numtri;             // total triangles
    Vec<unsigned int, 3> indices[8]; // 3 vertex indices per triangle

    // GL vertex array object IDs
    enum {TERRAIN_VARRAY, NUM_VARRAYS};
    unsigned int varrayIDs[NUM_VARRAYS];

    // GL buffer object IDs
    enum {POSITION_BUFFER, INDEX_BUFFER, UNIFORM_BUFFER, NUM_BUFFERS};
    unsigned int bufferIDs[NUM_BUFFERS];

    // GL shaders
    unsigned int shaderID;      // ID for shader program
    ShaderInfo shaderParts[2];  // vertex & fragment shader info

// public data
public:
    struct ModelData {
        MatPair4f modelmat;     // model to view matrix
    } mdata;

// public methods
public:
    // create tetrahedron data
    Marker();

    // clean up allocated memory
    ~Marker();

    // load/reload shaders
    void updateShaders();

    // update model matrix with new position
    void updatePosition(const Vec3f &center);

    // draw this tetrahedron object
    void draw() const;
};

#endif
