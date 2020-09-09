#ifndef MATRIX_H
#define MATRIX_H

#include "vector.h"

class Matrix
{
public:
    static Matrix from_rotation (const Vector& r);
    static Matrix from_rotation_position (const Vector& r, const Vector& p);
    static Matrix from_perspective (float fov, float aspect);
public:
    Vector x, y, z, w;
public:
    Matrix ()
    : x (1, 0, 0, 0)
    , y (0, 1, 0, 0)
    , z (0, 0, 1, 0)
    , w (0, 0, 0, 1)
    {
    }
    Matrix (const Vector& X, const Vector& Y, const Vector& Z, const Vector& W)
    {
        x = X;
        y = Y;
        z = Z;
        w = W;
    }

    Vector& operator[] (size_t index)
    {
        assert (index < 4 && "Bad matrix index");
        return *(&x + index);
    }
    const Vector& operator[] (size_t index) const
    {
        assert (index < 4 && "Bad matrix index");
        return *(&x + index);
    }

    const float *as_ptr () {return &x[0];}

    const Vector col (size_t index) const
    {
        assert (index < 4 && "Bad column index");
        switch (index)
        {
        case 0: return Vector (x[0], y[0], z[0], w[0]);
        case 1: return Vector (x[1], y[1], z[1], w[1]);
        case 2: return Vector (x[2], y[2], z[2], w[2]);
        case 3: return Vector (x[3], y[3], z[3], w[3]);
        }
        /*Shouldn't happen*/
        return Vector ();
    }
};

inline Matrix operator- (Matrix& m)
{
    return Matrix (-m.x, -m.y, -m.z, -m.w);
}

inline Matrix operator+ (const Matrix& a, const Matrix& b)
{
    return Matrix (a.x + b.x, a.y + b.y, a.z + b.z, a.w + b.w);
}
inline Matrix operator- (const Matrix& a, const Matrix& b)
{
    return Matrix (a.x - b.x, a.y - b.y, a.z - b.z, a.w - b.w);
}
inline Matrix operator* (const Matrix& a, const Matrix& b)
{
    Vector x = b.col (0);
    Vector y = b.col (1);
    Vector z = b.col (2);
    Vector w = b.col (3);
    return Matrix (
        Vector (dot (a.x, x), dot (a.x, y), dot (a.x, z), dot (a.x, w)),
        Vector (dot (a.y, x), dot (a.y, y), dot (a.y, z), dot (a.y, w)),
        Vector (dot (a.z, x), dot (a.z, y), dot (a.z, z), dot (a.z, w)),
        Vector (dot (a.w, x), dot (a.w, y), dot (a.w, z), dot (a.w, w))
    );
}
inline Matrix operator* (const Matrix& m, const float k)
{
    return Matrix (k*m.x, k*m.y, k*m.z, k*m.w);
}
inline Matrix operator* (const float k, const Matrix& m)
{
    return Matrix (k*m.x, k*m.y, k*m.z, k*m.w);
}

inline Matrix& operator+= (Matrix& a, const Matrix& b)
{
    for (auto i = 0; i < 4; i++) a[i] += b[i];
    return a;
}
inline Matrix& operator-= (Matrix& a, const Matrix& b)
{
    for (auto i = 0; i < 4; i++) a[i] -= b[i];
    return a;
}
inline Matrix& operator*= (Matrix& a, const Matrix& b)
{
    for (auto i = 0; i < 4; i++) a[i] *= b[i];
    return a;
}
inline Matrix& operator*= (Matrix& a, const float k)
{
    for (auto i = 0; i < 4; i++) a[i] *= k;
    return a;
}

inline bool operator== (const Matrix& a, const Matrix& b)
{
    return a.x == b.x && a.y == b.y && a.z == b.z && a.w == b.w;
}
inline bool operator!= (const Matrix& a, const Matrix& b)
{
    return a.x != b.x || a.y != b.y || a.z != b.z || a.w != b.w;
}

inline Matrix Matrix::from_perspective (float fov, float aspect)
{
    float y = 1.0/tanf (3.141592*fov/360.0);
    float x = y/aspect;
    return Matrix (
                Vector (x, 0, 0, 0),
                Vector (0, y, 0, 0),
                Vector (0, 0,-1,-1),
                Vector (0, 0,-1, 0));
}
inline Matrix Matrix::from_rotation_position (const Vector& r, const Vector& p)
{
    const float xx = r.x + r.x;
    const float yy = r.y + r.y;
    const float zz = r.z + r.z;
    const float xy = r.x*yy;
    const float yz = r.y*zz;
    const float wx = r.w*xx;
    const float xz = r.x*zz;
    const float wy = r.w*yy;
    const float wz = r.w*zz;
    const float x2x = r.x*xx;
    const float y2y = r.y*yy;
    const float z2z = r.z*zz;
    return Matrix (
            Vector (1 - y2y - z2z, xy - wz, xz + wy, p.x),
            Vector (xy + wz, 1 - x2x - z2z, yz - wx, p.y),
            Vector (xz - wy, yz + wx, 1 - x2x - y2y, p.z),
            Vector (0, 0, 0, p.w));
}
inline Matrix Matrix::from_rotation (const Vector& r)
{
    return Matrix::from_rotation_position (r, Vector (0, 0, 0, 1));
}

#endif // MATRIX_H
