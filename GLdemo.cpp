//
// Simple GL example
//

#include "AppContext.hpp"
#include "Input.hpp"
#include "Scene.hpp"
#include "Terrain.hpp"
#include "Marker.hpp"

// using core modern OpenGL
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <math.h>
#include <stdio.h>

///////
// Clean up any context data
AppContext::~AppContext()
{
    // if any are NULL, deleting a NULL pointer is OK
    delete scene;
    delete input;
    delete terrain;
    delete lightmarker;
}

///////
// GLFW callbacks must use extern "C"
extern "C" {

    //
    // called for GLFW error
    //
    void winError(int error, const char *description)
    {
        fprintf(stderr, "GLFW error: %s\n", description);
    }

    //
    // called whenever the window size changes
    //
    void reshape(GLFWwindow *win, int width, int height)
    {
        AppContext *appctx = (AppContext*)glfwGetWindowUserPointer(win);

        appctx->scene->viewport(win);
        appctx->input->redraw = true;
    }

    //
    // called when mouse button is pressed
    //
    void mousePress(GLFWwindow *win, int button, int action, int mods)
    {
        AppContext *appctx = (AppContext*)glfwGetWindowUserPointer(win);

        appctx->input->mousePress(win, button, action);
    }

    //
    // called when mouse is moved
    //
    void mouseMove(GLFWwindow *win, double x, double y)
    {
        AppContext *appctx = (AppContext*)glfwGetWindowUserPointer(win);

        appctx->input->mouseMove(win, appctx->scene, x,y);
    }

    // 
    // called on any keypress
    //
    void keyPress(GLFWwindow *win, int key, int scancode, int action, int mods)
    {
        AppContext *appctx = (AppContext*)glfwGetWindowUserPointer(win);

        if (action == GLFW_PRESS)
            appctx->input->keyPress(win, key, appctx);
        else if (action == GLFW_RELEASE)
            appctx->input->keyRelease(win, key);
    }
}

// initialize GLFW - windows and interaction
GLFWwindow *initGLFW(AppContext *appctx)
{
    // set error callback before init
    glfwSetErrorCallback(winError);
    if (! glfwInit())
        return 0;

    // ask for a window with dimensions 843 x 480 (HD 480p)
    // using core OpenGL
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
    GLFWwindow *win = glfwCreateWindow(843, 480, "OpenGL Demo", 0, 0);
    if (! win) {
        glfwTerminate();
        return 0;
    }
    glfwMakeContextCurrent(win);

    // use GLEW on windows to access modern OpenGL functions
    glewExperimental = true;
    glewInit();

	// store context pointer to access application data
    glfwSetWindowUserPointer(win, appctx);

    // set callback functions to be called by GLFW
    glfwSetFramebufferSizeCallback(win, reshape);
    glfwSetKeyCallback(win, keyPress);
    glfwSetMouseButtonCallback(win, mousePress);
    glfwSetCursorPosCallback(win, mouseMove);

    // set OpenGL state
    glEnable(GL_DEPTH_TEST);      // tell OpenGL to handle overlapping surfaces

    return win;
}

int main(int argc, char *argv[])
{
    // collected data about application for use in callbacks
    AppContext appctx;

    // set up GLUT and OpenGL
    GLFWwindow *win = initGLFW(&appctx);
    if (! win) return 1;

    // initialize context (after GLFW)
    appctx.input = new Input;
    appctx.terrain = new Terrain("terrain.ppm", "pebbles.ppm", 
                                 "pebbles-norm.ppm", "pebbles-gloss.ppm");
    appctx.lightmarker = new Marker();
    appctx.scene = new Scene(win, *appctx.lightmarker);

	glfwSetInputMode(win, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    // loop until GLFW says it's time to quit
    while (!glfwWindowShouldClose(win)) {
        // check for continuous key updates to view
        appctx.input->keyUpdate(&appctx);

		// get current time;
		double t = glfwGetTime() * 4;
		int tpart = floor(t);
		if (appctx.scene->sdata.time != tpart) {
			appctx.scene->sdata.time = tpart;
			appctx.input->redraw = true;
		}

        if (appctx.input->redraw) {
            // we're handing the redraw now
            appctx.input->redraw = false;

            // clear old screen contents
            glClearColor(0.2f, 0.5f, 1.f, 1.f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            // draw something
            appctx.scene->update();
            appctx.terrain->draw();
            appctx.lightmarker->draw();

            // show what we drew
            glfwSwapBuffers(win);
        }

        // wait for user input
        glfwPollEvents();
    }
	glfwSetInputMode(win, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

    glfwDestroyWindow(win);
    glfwTerminate();

    return 0;
}
