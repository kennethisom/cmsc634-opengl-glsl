// NxN-dimensional matrix/inverse pair operations
// inline functions and function declarations
#ifndef MatPair_inl
#define MatPair_inl

#include "MatPair.hpp"
#include "Mat.inl"
#include <math.h>

//////////////////////////////////////////////////////////////////////
// matrix size
template <typename T, int N>
inline int dimensions(const MatPair<T,N> &p) {
    return N;
}

//////////////////////////////////////////////////////////////////////
// addition and subtraction
template <typename T, int N>
inline MatPair<T,N> operator+(const MatPair<T,N> &m1, const MatPair<T,N> &m2) {
    MatPair<T,N> result;
    result.matrix = m1.matrix + m2.matrix;
    result.inverse = m1.inverse + m2.inverse;
}

template <typename T, int N>
inline MatPair<T,N> operator-(const MatPair<T,N> &m1, const MatPair<T,N> &m2) {
    MatPair<T,N> result;
    result.matrix = m1.matrix - m2.matrix;
    result.inverse = m1.inverse - m2.inverse;
}

//////////////////////////////////////////////////////////////////////
// scalar multiplication and division
template <typename T, int N>
inline MatPair<T,N> operator*(T s, const MatPair<T,N> &m) {
    MatPair<T,N> result;
    result.matrix = s*m.matrix;
    result.inverse = m.inverse / s;
    return result;
}

template <typename T, int N>
inline MatPair<T,N> operator*(const MatPair<T,N> &m, T s) {
    MatPair<T,N> result;
    result.matrix = m.matrix * s;
    result.inverse = m.inverse / s;
    return result;
}

template <typename T, int N>
inline MatPair<T,N> operator/(const MatPair<T,N> &m, T s) {
    MatPair<T,N> result;
    result.matrix = m.matrix / s;
    result.inverse = m.inverse * s;
    return result;
}

//////////////////////////////////////////////////////////////////////
// matrix*matrix
template <typename T, int N>
inline MatPair<T,N> operator*(const MatPair<T,N> &m1, const MatPair<T,N> &m2) {
    MatPair<T,N> result;
    result.matrix = m1.matrix * m2.matrix;
    result.inverse = m2.inverse * m1.inverse;
    return result;
}


//////////////////////////////////////////////////////////////////////
// Build functions. Not done as constructors, so the matrix will be a
// POD (plain old data) type for pre-C++11. Among other things,
// guarantees the type will be exactly equivalent to an array in
// memory.

// note that OpenGL uses matrices in column-major format, so each
// row in the source code is one COLUMN in the matrix

// build 4x4 perspective from field of view (in radians), 
// x/y aspect ratio, and near/far clipping planes
MatPair4f perspective4fp(float fov, float aspect, float near, float far);

// build x, y, or z axis rotation for angle (in radians)
MatPair4f xrotate4fp(float angle);
MatPair4f yrotate4fp(float angle);
MatPair4f zrotate4fp(float angle);

// build 4x4 scale matrix
MatPair4f scale4fp(Vec3f s);

// build 4x4 translation matrix
MatPair4f translate4fp(Vec3f t);

#endif
