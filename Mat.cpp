// NxN-dimensional matrix class template
// based on ideas from Nathan Reed
//    http://www.reedbeta.com/blog/2013/12/28/on-vector-math-libraries/
// non-inline functions

#include "Mat.inl"
#include "Vec.inl"
#include <math.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846264338327950288
#endif

// build 4x4 perspective from field of view (in radians), 
// x/y aspect ratio, and near/far clipping planes
Mat4f perspective4f(float fov, float aspect, float near, float far) {
    float y = 1/tanf(fov/2), x = y/aspect;
    float z = (near+far)/(near-far), w = 2*near*far/(near-far);
    return mat4<float>(x, 0, 0, 0,
                       0, y, 0, 0,
                       0, 0, z, -1,
                       0, 0, w, 0);
}

// build x, y, or z axis rotation for angle (in radians)
Mat4f xrotate4f(float angle) {
    float c = cosf(angle), s = sinf(angle);
    return mat4<float>(1, 0, 0, 0,
                       0, c, s, 0,
                       0,-s, c, 0,
                       0, 0, 0, 1);
}
Mat4f yrotate4f(float angle) {
    float c = cosf(angle), s = sinf(angle);
    return mat4<float>(c, 0,-s, 0,
                       0, 1, 0, 0,
                       s, 0, c, 0,
                       0, 0, 0, 1);
}
Mat4f zrotate4f(float angle) {
    float c = cosf(angle), s = sinf(angle);
    return mat4<float>( c, s, 0, 0,
                       -s, c, 0, 0,
                        0, 0, 1, 0,
                        0, 0, 0, 1);
}

// build 4x4 scale matrix
Mat4f scale4f(Vec3f s) {
    return mat4<float>(s.x, 0, 0, 0,
                       0, s.y, 0, 0,
                       0, 0, s.z, 0,
                       0, 0, 0, 1);
}

// build 4x4 translation matrix
Mat4f translate4f(Vec3f t) {
    return mat4<float>(1, 0, 0, 0,
                       0, 1, 0, 0,
                       0, 0, 1, 0,
                       t.x, t.y, t.z, 1);
}
