// handle changes due to mouse motion, or keys
#ifndef Input_hpp
#define Input_hpp

class Scene;
struct AppContext;
struct GLFWwindow;

class Input {
// private data
private:
    int button, oldButton;      // which mouse button was pressed?
    double oldX, oldY;          // location of mouse at last event

    double updateTime;          // time (in seconds) of last update
    float sideRate, forwardRate;    // for key change, orbiting rate in radians/sec
	float sideRateQ, forwardRateQ;  // for tracking movement change while jumping
	float orientationQ;				// for tracking orientation change while jumping

	double jumpTime;			// time (in seconds) of start of jump
	bool isJumping, initJump;	// tracking jump and jump prep
	float initialElevation;		// elevation at the start of the jump


// public data
public:
    bool redraw;                // true if we need to redraw

// public methods
public:
    // initialize
    Input() : button(-1), oldButton(-1), oldX(0), oldY(0), orientationQ(0),
              sideRate(0), forwardRate(0), sideRateQ(0), forwardRateQ(0),
			  redraw(true), isJumping(false), initJump(false) {}

    // handle mouse press / release
    void mousePress(GLFWwindow *win, int button, int action);

    // handle mouse motion
    void mouseMove(GLFWwindow *win, Scene *scene, double x, double y);

    // handle key press
    void keyPress(GLFWwindow *win, int key, AppContext *ctx);

    // handle key release
    void keyRelease(GLFWwindow *win, int key);

    // update view (if necessary) based on key input
    void keyUpdate(AppContext *ctx);
};

#endif
