// set up and maintain view as window sizes change

#include "Input.hpp"
#include "AppContext.hpp"
#include "Scene.hpp"
#include "Terrain.hpp"
#include "Marker.hpp"
#include "Vec.inl"

// using core modern OpenGL
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <math.h>

#ifndef F_PI
#define F_PI 3.1415926f
#endif

//
// called when a mouse button is pressed. 
// Remember where we were, and what mouse button it was.
//
void Input::mousePress(GLFWwindow *win, int b, int action)
{
    if (action == GLFW_PRESS) {
        // hide cursor, record button
        button = b;
    }
    else {
        // display cursor, update button state
        button = -1;       // no button
    }
}

//
// called when the mouse moves
// use difference between oldX,oldY and x,y to define a rotation
//
void Input::mouseMove(GLFWwindow *win, Scene *scene, double x, double y)
{
    // record differences & update last position
    float dx = float(x - oldX);
    float dy = float(y - oldY);

	// Update forward vector and right vector
	if (isJumping) {
		orientationQ -= F_PI * dx / float(scene->width);
	} else {
		scene->orientation -= F_PI * dx / float(scene->width);
	}

    // rotation angle, scaled so across the window = one rotation
    scene->viewSph.x += F_PI * dx / float(scene->width);
    scene->viewSph.y += 0.5f*F_PI * dy / float(scene->height);
	
	if (scene->viewSph.y > 0.45f * F_PI) { scene->viewSph.y = 0.45f * F_PI; }
	if (scene->viewSph.y < -0.45f * F_PI) { scene->viewSph.y = -0.45f * F_PI; }

    scene->view();

    // tell GLFW that something has changed and we must redraw
    redraw = true;

    // update prior mouse state
    oldButton = button;
    oldX = x;
    oldY = y;
}

//
// called when any key is pressed
//
void Input::keyPress(GLFWwindow *win, int key, AppContext *appctx)
{
    switch (key) {
    case 'A':                   // rotate left
		if (isJumping) {
			sideRateQ -= 100.f;
		} else {
			sideRate -= 100.f;
		}
        updateTime = glfwGetTime();
        redraw = true;          // need to redraw
        break;

    case 'D':                   // rotate right
		if (isJumping) {
			sideRateQ += 100.f;
		} else {
			sideRate += 100.f;
		}
        updateTime = glfwGetTime();
        redraw = true;          // need to redraw
        break;

    case 'W':                   // rotate up
		if (isJumping) {
			forwardRateQ += 100.f;
		} else {
			forwardRate += 100.f;
		}
        updateTime = glfwGetTime();
        redraw = true;          // need to redraw
        break;

    case 'S':                   // rotate down
		if (isJumping) {
			forwardRateQ -= 100.f;
		} else {
			forwardRate -= 100.f;
		}
        updateTime = glfwGetTime();
        redraw = true;          // need to redraw
        break;

    case 'F':                   // toggle fog on or off
        appctx->scene->sdata.fog = 1 - appctx->scene->sdata.fog;
        redraw = true;          // need to redraw
        break;

    case 'R':                   // reload shaders
        appctx->terrain->updateShaders();
        appctx->lightmarker->updateShaders();
        redraw = true;          // need to redraw
        break;

    case GLFW_KEY_SPACE:        // Jump
		if (!isJumping) {
			isJumping = true;
			initJump = true;
			jumpTime = glfwGetTime();
			redraw = true;          // need to redraw
		}
        break;

    case GLFW_KEY_ESCAPE:       // Escape: exit
        glfwSetWindowShouldClose(win, true);
        break;
    }
}

//
// called when any key is released
//
void Input::keyRelease(GLFWwindow *win, int key)
{
    switch (key) {
    case 'A':         // stop moving left
		if (isJumping) {
			sideRateQ += 100.f;
		} else {
			sideRate += 100.f;
		}
        break;
    case 'D':         // stop moving right
		if (isJumping) {
			sideRateQ -= 100.f;
		} else {
			sideRate -= 100.f;
		}
        break;
    case 'W':         // stop moving forward
		if (isJumping) {
			forwardRateQ -= 100.f;
		} else {
			forwardRate -= 100.f;
		}
		break;
    case 'S':         // stop moving backward
		if (isJumping) {
			forwardRateQ += 100.f;
		} else {
			forwardRate += 100.f;
		}
        break;
    }
}

//
// update view if necessary based on a/d keys
//
void Input::keyUpdate(AppContext *appctx)
{
	float elevation, theta_xz, theta_yz;
    double now = glfwGetTime();

	appctx->terrain->getElevation(appctx->scene->positionSph.x, appctx->scene->positionSph.y, elevation, theta_xz, theta_yz);

    if (sideRate != 0 || forwardRate != 0) {
		float sRate = sideRate, fRate = forwardRate;
		if(sideRate != 0 && forwardRate != 0) {
			float sqrt2 = sqrt(2.f);
			sRate = sRate * sqrt2 / 2;
			fRate = fRate * sqrt2 / 2;
		}

        double dt = (now - updateTime);
		
		Vec2f forwardXY = normalize(vec2<float>(cosf(appctx->scene->orientation), sinf(appctx->scene->orientation)));

        // update pan based on time elapsed since last update
        // ensures uniform rate of change
        appctx->scene->positionSph.x += float(fRate * forwardXY.x * dt);
        appctx->scene->positionSph.y += float(fRate * forwardXY.y * dt);
        appctx->scene->positionSph.x += float(sRate * forwardXY.y * dt);
        appctx->scene->positionSph.y += float(sRate * -forwardXY.x * dt);
		
		// move view to opposite side of traversal area if POV moves beyond the edge in either direction
		if (appctx->scene->positionSph.x >= 256.f) { appctx->scene->positionSph.x -= 512.f; }
		if (appctx->scene->positionSph.x <= -256.f) { appctx->scene->positionSph.x += 512.f; }
		if (appctx->scene->positionSph.y >= 256.f) { appctx->scene->positionSph.y -= 512.f; }
		if (appctx->scene->positionSph.y <= -256.f) { appctx->scene->positionSph.y += 512.f; }

		// Set height based on elevation at position x, y
		appctx->terrain->getElevation(appctx->scene->positionSph.x, appctx->scene->positionSph.y, elevation, theta_xz, theta_yz);

        // changing, so will need to start another draw
        redraw = true;
    }

	if (isJumping) {
		if (initJump) {
			initialElevation = elevation;
			initJump = false;
		}

		double jump_dt = (now - jumpTime);

		float jumpElevation = initialElevation + 75.f*jump_dt - 125.f*jump_dt*jump_dt/2;

		if (jumpElevation > elevation) {
			float de = jumpElevation - elevation;
			elevation = jumpElevation;
			if (de < 1.f) {
				theta_xz = (1.f - de) * theta_xz;
				theta_yz = (1.f - de) * theta_yz;
			} else {
				theta_xz = 0;
				theta_yz = 0;
			}
		} else {
			isJumping = false;
			appctx->scene->orientation += orientationQ;
			forwardRate += forwardRateQ;
			sideRate += sideRateQ;
			orientationQ = 0.f;
			forwardRateQ = 0.f;
			sideRateQ = 0.f;
		}

		redraw = true;
	}

	if (redraw) {
		appctx->scene->positionSph.z = elevation;
		appctx->scene->alignmentSph.x = theta_xz;
		appctx->scene->alignmentSph.y = theta_yz;

		// Update View
        appctx->scene->view();
	}

    // remember time for next update
    updateTime = now;
}
