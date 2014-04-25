// data shared across entire scene
// expected to change up to once per frame
#ifndef Scene_hpp
#define Scene_hpp

#include "Vec.hpp"
#include "MatPair.hpp"

class Marker;
struct GLFWwindow;

class Scene {
// private data
private:
    // GL uniform buffer IDs
    enum {UNIFORM_BUFFER, NUM_BUFFERS};
    unsigned int bufferIDs[NUM_BUFFERS];

// public data
public:
    struct ShaderData {
        MatPair4f viewmat, projection; // viewing matrices
        Vec3f lightpos;		       // light position
        int fog;		       // 1 = display fog, 0 = don't
		int time;
    } sdata;

    int width, height;         // current window dimensions

    Vec3f viewSph;          // view position in spherical coordinates
    Vec3f lightSph;         // light position in spherical coordinates
	Vec3f positionSph;      // POV Position from the origin
	Vec2f alignmentSph;		// POV Alignment with the surface normal
	float orientation;      // POV Forward Orientation in radians

// public methods
public:
    // create with initial window size and orbit location
    Scene(GLFWwindow *win, Marker &lightMarker);

    // set up new window viewport and projection
    void viewport(GLFWwindow *win);

    // set view using orbitAngle
    void view();

    // update light
    void light(Marker &lightMarker);

    // update shader uniform state each frame
    void update() const;
};

#endif
