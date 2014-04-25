// vertex shader for light marker in terrain demo
#version 400 core

// per-frame data
layout(std140)                  // use standard layout
uniform SceneData {             // uniform struct name
    mat4 viewMatrix, viewInverse;
    mat4 projectionMatrix, projectionInverse;
    vec3 lightpos;
    int fog;
};

// model data
layout(std140)                  // use standard layout
uniform ModelData {             // uniform struct name
    mat4 modelMatrix, modelInverse;
};

// per-vertex input
in vec3 vPosition;

void main() {
    gl_Position = projectionMatrix * viewMatrix * modelMatrix 
        * vec4(vPosition,1);
}
