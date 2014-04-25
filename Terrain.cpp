// draw a simple terrain height field

#include "Terrain.hpp"
#include "AppContext.hpp"
#include "ImagePPM.hpp"
#include "Vec.inl"
#include "math.h"

// using core modern OpenGL
#include <GL/glew.h>
#include <GLFW/glfw3.h>


//
// load the terrain data
//
Terrain::Terrain(const char *elevationPPM, const char *texturePPM,
                 const char *normalPPM, const char *glossPPM)
{
	// set amount of terrain replication
	repl = 3;

    // buffer objects to be used later
    glGenTextures(NUM_TEXTURES, textureIDs);
    glGenBuffers(NUM_BUFFERS, bufferIDs);
    glGenVertexArrays(NUM_VARRAYS, varrayIDs);

    // load albedo, normal & gloss image into a named textures
    ImagePPM(texturePPM).loadTexture(textureIDs[COLOR_TEXTURE]);
    ImagePPM(normalPPM).loadTexture(textureIDs[NORMAL_TEXTURE]);
    ImagePPM(glossPPM).loadTexture(textureIDs[GLOSS_TEXTURE]);

    // load terrain heights
    ImagePPM elevation(elevationPPM);
    unsigned int w_act = elevation.width, h_act = elevation.height;
	unsigned int w = w_act*repl, h = h_act*repl;
    gridSize = vec3<float>(float(w), float(h), 255.f);

    // world dimensions
	walkableSize = vec2<float>(512, 512);
    mapSize = vec3<float>(walkableSize.x*repl, walkableSize.y*repl, 100);

    // build vertex, normal and texture coordinate arrays
    // * x & y are the position in the terrain grid
    // * idx is the linear array index for each vertex
    numvert = (w + 1) * (h + 1);
    vert = new Vec3f[numvert];
    dPdu = new Vec3f[numvert];
    dPdv = new Vec3f[numvert];
    norm = new Vec3f[numvert];
    texcoord = new Vec2f[numvert];

    for(unsigned int y=0, idx=0;  y <= h;  ++y) {
        for(unsigned int x=0;  x <= w;  ++idx, ++x) {
			// 3d vertex location: x,y from grid location, z from terrain data
			vert[idx] = (vec3<float>(float(x), float(y), elevation(x%w_act, y%h_act).r)
							/ gridSize - 0.5f) * mapSize;

			// compute normal & tangents from partial derivatives:
			//   position =
			//     (u / gridSize.x - .5) * mapSize.x
			//     (v / gridSize.y - .5) * mapSize.y
			//     (elevation / gridSize.z - .5) * mapSize.z
			//   the u-tangent is the per-component partial derivative by u:
			//      mapSize.x / gridSize.x
			//      0
			//      d(elevation(u,v))/du * mapSize.z / gridSize.z
			//   the v-tangent is the partial derivative by v
			//      0
			//      mapSize.y / gridSize.y
			//      d(elevation(u,v))/du * mapSize.z / gridSize.z
			//   the normal is the cross product of these

			// first approximate du = d(elevation(u,v))/du (and dv)
			// be careful to wrap indices to 0 <= x < w and 0 <= y < h
			float du = (elevation((x+1)%w_act, y%h_act).r - elevation((x+w-1)%w_act, y%h_act).r)
				* 0.5f * mapSize.z / gridSize.z;
			float dv = (elevation(x%w_act, (y+1)%h_act).r - elevation(x%w_act, (y+h-1)%h_act).r)
				* 0.5f * mapSize.z / gridSize.z;

			// final tangents and normal using these
			dPdu[idx] = normalize(vec3<float>(mapSize.x/gridSize.x, 0, du));
			dPdv[idx] = normalize(vec3<float>(0, mapSize.y/gridSize.y, dv));
			norm[idx] = normalize(dPdu[idx] ^ dPdv[idx]);

			// 2D texture coordinate for rocks texture, from grid location
			texcoord[idx] = vec2<float>(float(x*repl),float(y*repl)) / gridSize.xy;
        }
    }

    // build index array linking sets of three vertices into triangles
    // two triangles per square in the grid. Each vertex index is
    // essentially its unfolded grid array position. Be careful that
    // each triangle ends up in counter-clockwise order
    numtri = 2*w*h;
    indices = new Vec<unsigned int, 3>[numtri];
    for(unsigned int y=0, idx=0; y<h; ++y) {
        for(unsigned int x=0; x<w; ++x, idx+=2) {
            indices[idx][0] = (w+1)* y    + x;
            indices[idx][1] = (w+1)* y    + x+1;
            indices[idx][2] = (w+1)*(y+1) + x+1;

            indices[idx+1][0] = (w+1)* y    + x;
            indices[idx+1][1] = (w+1)*(y+1) + x+1;
            indices[idx+1][2] = (w+1)*(y+1) + x;
        }
    }

    // load vertex and index array to GPU
    glBindBuffer(GL_ARRAY_BUFFER, bufferIDs[POSITION_BUFFER]);
    glBufferData(GL_ARRAY_BUFFER, numvert*sizeof(Vec3f), vert, GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, bufferIDs[TANGENT_BUFFER]);
    glBufferData(GL_ARRAY_BUFFER, numvert*sizeof(Vec3f), dPdu, GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, bufferIDs[BITANGENT_BUFFER]);
    glBufferData(GL_ARRAY_BUFFER, numvert*sizeof(Vec3f), dPdv, GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, bufferIDs[NORMAL_BUFFER]);
    glBufferData(GL_ARRAY_BUFFER, numvert*sizeof(Vec3f), norm, GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, bufferIDs[UV_BUFFER]);
    glBufferData(GL_ARRAY_BUFFER, numvert*sizeof(Vec2f), texcoord, 
                 GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bufferIDs[INDEX_BUFFER]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, 
                 numtri*sizeof(unsigned int[3]), indices, GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    // initial shader load
    shaderParts[0].id = glCreateShader(GL_VERTEX_SHADER);
    shaderParts[0].file = "terrain.vert";
    shaderParts[1].id = glCreateShader(GL_FRAGMENT_SHADER);
    shaderParts[1].file = "terrain.frag";
    shaderID = glCreateProgram();
    updateShaders();
}

//
// Delete terrain data
//
Terrain::~Terrain()
{
    glDeleteShader(shaderParts[0].id);
    glDeleteShader(shaderParts[1].id);
    glDeleteProgram(shaderID);
    glDeleteTextures(NUM_TEXTURES, textureIDs);
    glDeleteBuffers(NUM_BUFFERS, bufferIDs);

    delete[] indices;
    delete[] texcoord;
    delete[] norm;
    delete[] dPdv;
    delete[] dPdu;
    delete[] vert;
}

//
// load (or replace) texture
//
void Terrain::updateTexture(const char *ppm, unsigned int textureID)
{
    ImagePPM texture(ppm);
    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 
                 texture.width, texture.height, 0,
                 GL_RGB, GL_UNSIGNED_BYTE, texture.image);
    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,
                    GL_LINEAR_MIPMAP_LINEAR);
    glBindTexture(GL_TEXTURE_2D, 0);
}

//
// load (or replace) terrain shaders
//
void Terrain::updateShaders()
{
    loadShaders(shaderID, sizeof(shaderParts)/sizeof(*shaderParts), 
                shaderParts);
    glUseProgram(shaderID);

    // (re)connect view and projection matrices
    glUniformBlockBinding(shaderID, 
                          glGetUniformBlockIndex(shaderID,"SceneData"),
                          AppContext::SCENE_UNIFORMS);

    // map shader name for texture to glActiveTexture number used in draw
    glUniform1i(glGetUniformLocation(shaderID, "colorTexture"), COLOR_TEXTURE);
    glUniform1i(glGetUniformLocation(shaderID, "normalTexture"), NORMAL_TEXTURE);
    glUniform1i(glGetUniformLocation(shaderID, "glossTexture"), GLOSS_TEXTURE);

    // re-connect attribute arrays
    glBindVertexArray(varrayIDs[TERRAIN_VARRAY]);

    GLint positionAttrib = glGetAttribLocation(shaderID, "vPosition");
    glBindBuffer(GL_ARRAY_BUFFER, bufferIDs[POSITION_BUFFER]);
    glVertexAttribPointer(positionAttrib, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(positionAttrib);

    GLint tangentAttrib = glGetAttribLocation(shaderID, "vTangent");
    glBindBuffer(GL_ARRAY_BUFFER, bufferIDs[TANGENT_BUFFER]);
    glVertexAttribPointer(tangentAttrib, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(tangentAttrib);

    GLint bitangentAttrib = glGetAttribLocation(shaderID, "vBitangent");
    glBindBuffer(GL_ARRAY_BUFFER, bufferIDs[BITANGENT_BUFFER]);
    glVertexAttribPointer(bitangentAttrib, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(bitangentAttrib);

    GLint normalAttrib = glGetAttribLocation(shaderID, "vNormal");
    glBindBuffer(GL_ARRAY_BUFFER, bufferIDs[NORMAL_BUFFER]);
    glVertexAttribPointer(normalAttrib, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(normalAttrib);

    GLint uvAttrib = glGetAttribLocation(shaderID, "vUV");
    glBindBuffer(GL_ARRAY_BUFFER, bufferIDs[UV_BUFFER]);
    glVertexAttribPointer(uvAttrib, 2, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(uvAttrib);

    // turn off everything we enabled
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    glUseProgram(0);
}

//
// this is called every time the terrain needs to be redrawn 
//
void Terrain::draw() const
{
    // enable shaders
    glUseProgram(shaderID);

    // enable vertex array and textures
    glBindVertexArray(varrayIDs[TERRAIN_VARRAY]);
    for(int i=0; i<NUM_TEXTURES; ++i) {
        glActiveTexture(GL_TEXTURE0 + i);
        glBindTexture(GL_TEXTURE_2D, textureIDs[i]);
    }

    // draw the triangles for each three indices
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bufferIDs[INDEX_BUFFER]);
    glDrawElements(GL_TRIANGLES, 3*numtri, GL_UNSIGNED_INT, 0);

    // turn of whatever we turned on
    for(int i=0; i<NUM_TEXTURES; ++i) {
        glActiveTexture(GL_TEXTURE0 + i);
        glBindTexture(GL_TEXTURE_2D, 0);
    }
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    glUseProgram(0);
}

//
// update view if necessary based on a/d keys
//
void Terrain::getElevation(float x, float y, float &e, float &t_xz, float &t_yz)
{
	float u0, v0, w0, u1, v1, w1, uvw0, uvw1, theta_xz, theta_yz;
	float elevation = 0.f;
	Vec3f n;
	Vec2f p = vec2<float>(x, y);
	Vec3f p_grid = (vec3<float>(x, y, 0.f) / mapSize + 0.5f) * gridSize;
	
	int xmin = (int) floor(p_grid.x);
	int xmax = (int) ceil(p_grid.x);
	int ymin = (int) floor(p_grid.y);
	int ymax = (int) ceil(p_grid.y);

	int idx_x0y0 = ymin * ((int) gridSize.x + 1) + xmin;
	int idx_x1y0 = idx_x0y0 + 1;
	int idx_x0y1 = idx_x0y0 + ((int) gridSize.x + 1);
	int idx_x1y1 = idx_x0y1 + 1;
	
	float areaTri0 = area(vert[idx_x0y0].xy, vert[idx_x1y0].xy, vert[idx_x1y1].xy);
	u0 = area(p, vert[idx_x1y0].xy, vert[idx_x1y1].xy) / areaTri0;
	v0 = area(vert[idx_x0y0].xy, p, vert[idx_x1y1].xy) / areaTri0;
	w0 = area(vert[idx_x0y0].xy, vert[idx_x1y0].xy, p) / areaTri0;
	uvw0 = u0 + v0 + w0;
	float areaTri1 = area(vert[idx_x0y0].xy, vert[idx_x1y1].xy, vert[idx_x0y1].xy);
	u1 = area(p, vert[idx_x1y1].xy, vert[idx_x0y1].xy) / areaTri1;
	v1 = area(vert[idx_x0y0].xy, p, vert[idx_x0y1].xy) / areaTri1;
	w1 = area(vert[idx_x0y0].xy, vert[idx_x1y1].xy, p) / areaTri1;
	uvw1 = u1 + v1 + w1;

	if (uvw0 < uvw1) {
		elevation += vert[idx_x0y0].z * u0;
		elevation += vert[idx_x1y0].z * v0;
		elevation += vert[idx_x1y1].z * w0;
		//n = normalize((norm[idx_x0y0] * u0) + (norm[idx_x1y0] * v0) + (norm[idx_x1y1] * w0));
	} else {
		elevation += vert[idx_x0y0].z * u1;
		elevation += vert[idx_x1y1].z * v1;
		elevation += vert[idx_x0y1].z * w1;
		//n = normalize((norm[idx_x0y0] * u1) + (norm[idx_x1y1] * v1) + (norm[idx_x0y1] * w1));
	}
	//theta_xz = atan(-n.x / n.z);
	//theta_yz = atan(n.y / n.z);

	e = elevation;
	//t_xz = theta_xz;
	//t_yz = theta_yz;
	t_xz = 0.f;
	t_yz = 0.f;
}

