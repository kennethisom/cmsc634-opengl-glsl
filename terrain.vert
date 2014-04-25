// vertex shader for simple terrain demo
#version 400 core

// per-frame data
layout(std140)                  // use standard layout
uniform SceneData {             // uniform struct name
    mat4 viewMatrix, viewInverse;
    mat4 projectionMatrix, projectionInverse;
    vec3 lightpos;
    int fog;
	int time;
};

// per-vertex input
in vec3 vPosition;
in vec3 vTangent, vBitangent, vNormal;
in vec2 vUV;

// output to fragment shader
out vec4 position, light, vertpos;
out vec3 tangent, bitangent, normal;
out vec2 texcoord;

void main() {
    // surface and light position in view space
    position = viewMatrix * vec4(vPosition, 1);
    light = viewMatrix * vec4(lightpos, 1);
	vertpos = vec4(vPosition, 1);

    // transform tangents and normal
    tangent = normalize(mat3(viewMatrix) * vTangent);
    bitangent = normalize(mat3(viewMatrix) * vBitangent);
    normal = normalize(vNormal * mat3(viewInverse));

    // pass through texture coordinate
    texcoord = vUV;

    // rendering position
    gl_Position = projectionMatrix * position;
}
