// NxN-dimensional matrix/inverse pair operations
// non-inlined functions

#include "MatPair.inl"

#ifndef M_PI
#define M_PI 3.14159265358979323846264338327950288
#endif

//////////////////////////////////////////////////////////////////////
// Build functions. Not done as constructors, so the matrix will be a
// POD (plain old data) type for pre-C++11. Among other things,
// guarantees the type will be exactly equivalent to an array in
// memory.

// note that OpenGL uses matrices in column-major format, so each
// row in the source code is one COLUMN in the matrix

// build 4x4 perspective from field of view (in radians), 
// x/y aspect ratio, and near/far clipping planes
MatPair4f perspective4fp(float fov, float aspect, float near, float far) {
    MatPair4f result;
    result.matrix = perspective4f(fov, aspect, near, far);
    result.inverse = mat4<float>(
        1/result.matrix[0][0], 0, 0, 0,
        0, 1/result.matrix[1][1], 0, 0,
        0, 0, 0, 1/result.matrix[3][2],
        0, 0, -1, result.matrix[2][2]/result.matrix[3][2]);
    return result;
}

// build x, y, or z axis rotation for angle (in radians)
MatPair4f xrotate4fp(float angle) {
    MatPair4f result;
    result.matrix = xrotate4f(angle);
    result.inverse = transpose(result.matrix);
    return result;
}
MatPair4f yrotate4fp(float angle) {
    MatPair4f result;
    result.matrix = yrotate4f(angle);
    result.inverse = transpose(result.matrix);
    return result;
}
MatPair4f zrotate4fp(float angle) {
    MatPair4f result;
    result.matrix = zrotate4f(angle);
    result.inverse = transpose(result.matrix);
    return result;
}

// build 4x4 scale matrix
MatPair4f scale4fp(Vec3f s) {
    MatPair4f result;
    result.matrix = scale4f(s);
    result.inverse = mat4<float>(
        1/s.x, 0, 0, 0,
        0, 1/s.y, 0, 0,
        0, 0, 1/s.z, 0,
        0, 0, 0, 1);
    return result;
}

// build 4x4 translation matrix
MatPair4f translate4fp(Vec3f t) {
    MatPair4f result;
    result.matrix = translate4f(t);
    result.inverse = mat4<float>(
        1, 0, 0, 0,
        0, 1, 0, 0,
        0, 0, 1, 0,
        -t.x, -t.y, -t.z, 1);
    return result;
}
