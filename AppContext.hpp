// collected state for access in callbacks
#ifndef AppContext_hpp
#define AppContext_hpp

struct AppContext {
    class Scene *scene;         // viewing data
    class Input *input;         // user interface data
    class Terrain *terrain;     // terrain geometry
    class Marker *lightmarker;  // light marker geometry

    // uniform (aka shader parameter) block indices
    enum { SCENE_UNIFORMS, MODEL_UNIFORMS };

    // initialize all pointers to NULL to allow delete in destructor
    AppContext() : scene(0), input(0), terrain(0), lightmarker(0) {}

    // clean up any context data
    ~AppContext();
};

// load set of shaders
#endif
