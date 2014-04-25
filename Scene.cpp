// data shared across entire scene
// expected to change up to once per frame
// primarily view information

#include "Scene.hpp"
#include "Terrain.hpp"
#include "AppContext.hpp"
#include "Marker.hpp"

#include "MatPair.inl"
#include "Vec.inl"

// using core modern OpenGL
#include <GL/glew.h>
#include <GLFW/glfw3.h>

// for offsetof
#include <cstddef>

#ifndef F_PI
#define F_PI 3.1415926f
#endif

//
// create and initialize view
//
Scene::Scene(GLFWwindow *win, Marker &lightmarker) : 
    viewSph(vec3<float>(0.f, 0.f, 10.f)),
    lightSph(vec3<float>(0.5f * F_PI, 0.5f * F_PI, 300.f)),
	positionSph(vec3<float>(0.f, 0.f, 0.f)),
	alignmentSph(vec2<float>(0.f, 0.f)),
	orientation(F_PI / 2)
{
    // create uniform buffer objects
    glGenBuffers(NUM_BUFFERS, bufferIDs);
    glBindBuffer(GL_UNIFORM_BUFFER, bufferIDs[UNIFORM_BUFFER]);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(ShaderData), 0, GL_STREAM_DRAW);
    glBindBufferBase(GL_UNIFORM_BUFFER, AppContext::SCENE_UNIFORMS,
                     bufferIDs[UNIFORM_BUFFER]);
	
	AppContext *appctx = (AppContext*)glfwGetWindowUserPointer(win);

    // initialize scene data
	appctx->terrain->getElevation(positionSph.x, positionSph.y, positionSph.z, alignmentSph.x, alignmentSph.y);
    viewport(win);
    view();
    light(lightmarker);
    sdata.fog = 0;                    // fog off
	sdata.time = 0;
}

//
// New view, pointing to origin, at specified angle
//
void Scene::view()
{
    // update view matrix
    sdata.viewmat =  xrotate4fp(-F_PI/2)
		* xrotate4fp(viewSph.y)
		* zrotate4fp(viewSph.x)
        * yrotate4fp(alignmentSph.x)
        * xrotate4fp(alignmentSph.y)
		* translate4fp(vec3<float>(0, 0,-viewSph.z))
		* translate4fp(-1.f * positionSph);
}

//
// This is called when window is created or resized
// Adjust projection accordingly.
//
void Scene::viewport(GLFWwindow *win)
{
    // get window dimensions
    glfwGetFramebufferSize(win, &width, &height);

    // this viewport makes a 1 to 1 mapping of physical pixels to GL
    // "logical" pixels
    glViewport(0, 0, width, height);

    // adjust 3D projection into this window
    sdata.projection = perspective4fp(F_PI/4, (float)width/height, 1, 10000);
}


//
// Call to update light position
//
void Scene::light(Marker &lightmarker)
{
    // update position from spherical coordinates
    float cx = cos(lightSph.x), sx = sin(lightSph.x);
    float cy = cos(lightSph.y), sy = sin(lightSph.y);
    sdata.lightpos = lightSph.z * vec3(cx*cy, sx*cy, sy);

    // update marker position
    lightmarker.updatePosition(sdata.lightpos);
}


//
// call before drawing each frame to update per-frame scene state
//
void Scene::update() const
{
    // update uniform block
    glBindBuffer(GL_UNIFORM_BUFFER, bufferIDs[UNIFORM_BUFFER]);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(ShaderData), &sdata);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
}
