#ifndef VECTOR_H
#define VECTOR_H

#include <cassert>
#include <cmath>

class Vector
{
public:
    float x, y, z, w;
public:
    Vector () {x = 0; y = 0; z = 0; w = 0;}
    Vector (float X) {x = X; y = 0; z = 0; w = 0;}
    Vector (float X, float Y) {x = X; y = Y; z = 0; w = 0;}
    Vector (float X, float Y, float Z) {x = X; y = Y; z = Z; w = 0;}
    Vector (float X, float Y, float Z, float W) {x = X; y = Y; z = Z; w = W;}

    float& operator[] (size_t index)
    {
        assert (index < 4 && "Bad vector index");
        return *(&x + index);
    }
    const float& operator[] (size_t index) const
    {
        assert (index < 4 && "Bad vector index");
        return *(&x + index);
    }
};

/*Operators*/
inline Vector operator- (Vector& a)
{
    return Vector (-a.x, -a.y, -a.z, -a.w);
}

inline Vector operator+ (const Vector& a, const Vector& b)
{
    return Vector (a.x + b.x, a.y + b.y, a.z + b.z, a.w + b.w);
}
inline Vector operator- (Vector a, const Vector& b)
{
    return Vector (a.x - b.x, a.y - b.y, a.z - b.z, a.w - b.w);
}
inline Vector operator* (Vector a, const Vector& b)
{
    return Vector (a.x*b.x, a.y*b.y, a.z*b.z, a.w*b.w);
}
inline Vector operator* (const float k, const Vector& v)
{
    return Vector (k*v.x, k*v.y, k*v.z, k*v.w);
}
inline Vector operator* (const Vector& v, const float k)
{
    return Vector (k*v.x, k*v.y, k*v.z, k*v.w);
}

inline Vector& operator+= (Vector& a, const Vector b)
{
    for (auto i = 0; i < 4; i++) a[i] += b[i];
    return a;
}
inline Vector& operator-= (Vector& a, const Vector b)
{
    for (auto i = 0; i < 4; i++) a[i] -= b[i];
    return a;
}
inline Vector& operator*= (Vector& a, const Vector b)
{
    for (auto i = 0; i < 4; i++) a[i] *= b[i];
    return a;
}
inline Vector& operator*= (Vector& a, const float k)
{
    for (auto i = 0; i < 4; i++) a[i] *= k;
    return a;
}

inline bool operator== (const Vector& a, const Vector& b)
{
    return a.x == b.x && a.y == b.y && a.z == b.z && a.w == b.w;
}
inline bool operator!= (const Vector& a, const Vector& b)
{
    return a.x != b.x || a.y != b.y || a.z != b.z || a.w != b.w;
}

/*Functions*/
inline bool compare (const Vector &a, const Vector &b, const float epsilon)
{
    return std::abs (a.x - b.x) <= epsilon &&
           std::abs (a.y - b.y) <= epsilon &&
           std::abs (a.z - b.z) <= epsilon &&
           std::abs (a.w - b.w) <= epsilon;
}

inline Vector lerp (const Vector& a, const float k, const Vector& b)
{
    const float g = 1.0 - k;
    return Vector (g*a.x + k*b.x,
                   g*a.y + k*b.y,
                   g*a.z + k*b.z,
                   g*a.w + k*b.w);
}
inline Vector cross (const Vector& a, const Vector& b)
{
    return Vector (a.y*b.z - b.y*a.z,
                   a.z*b.x - b.z*a.x,
                   a.x*b.y - b.x*a.y);
}
inline float dot (const Vector& a, const Vector& b)
{
    return a.x*b.x + a.y*b.y + a.z*b.z + a.w*b.w;
}

#endif
