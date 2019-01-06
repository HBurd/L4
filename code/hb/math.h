#ifndef HBMATH_H
#define HBMATH_H

const float MATH_PI  = 3.141592653589793238462643f;

// Vector math
// ==========

// 2D
struct Vec2
{
    float x = 0.0f;
    float y = 0.0f;

    Vec2() = default;
    Vec2(float _x, float _y);

    Vec2& operator+=(const Vec2& rhs);
    Vec2& operator-=(const Vec2& rhs);
    float norm() const;
    Vec2 normalize() const;
};

struct Mat22
{
    float data[4] = {};
    Mat22() = default;
    Mat22(float a, float b,
          float c, float d);

    static Mat22 rotation(float theta);
};

// algebra
Vec2 operator+(const Vec2& lhs, const Vec2& rhs);
Vec2 operator-(const Vec2& lhs, const Vec2& rhs);
Vec2 operator*(float lhs, const Vec2& rhs);
Vec2 operator*(const Vec2& lhs, float rhs);
Vec2 operator-(const Vec2& operand);
bool operator==(const Vec2& lhs, const Vec2& rhs);

float dot(const Vec2& lhs, const Vec2& rhs);

Vec2 operator*(const Mat22& lhs, const Vec2& rhs);

// utility
Vec2 unit_angle(float angle);

// 3D
struct Vec3
{
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;

    Vec3() = default;
    Vec3(float _x, float _y, float _z);

    Vec3& operator+=(const Vec3& rhs);
    Vec3& operator-=(const Vec3& rhs);
    float norm() const;
    Vec3 normalize() const;
};

static_assert(sizeof(Vec3) == 3 * sizeof(float), "There must be no padding between components");

struct Mat33
{
    float data[9] = {};  // row major
    Mat33() = default;
    Mat33(float e11, float e12, float e13,
          float e21, float e22, float e23,
          float e31, float e32, float e33);

    void print() const;
};

static_assert(sizeof(Mat33) == 9 * sizeof(float), "There must be no padding between components");

struct Rotor
{
    float s = 1.0f;    // scalar part

    float xy = 0.0f;   //
    float yz = 0.0f;   // bivector part
    float zx = 0.0f;   //

    Rotor() = default;
    Rotor(const Vec3& v1, const Vec3& lhs);
    Mat33 to_matrix() const;
    Rotor inverse() const;
    Rotor normalize() const;

    static Rotor roll(float angle);
    static Rotor pitch(float angle);
    static Rotor yaw(float angle);

    Rotor exponentiate(float t);
    static Rotor lerp(const Rotor &start, const Rotor &end, float t);
    static Rotor slerp(const Rotor &start, const Rotor &end, float t);
};

// algebra
Vec3 operator+(const Vec3& lhs, const Vec3& rhs);
Vec3 operator-(const Vec3& lhs, const Vec3& rhs);
Vec3 operator*(float lhs, const Vec3& rhs);
Vec3 operator*(const Vec3& lhs, float rhs);
Vec3 operator-(const Vec3& operand);
bool operator==(const Vec3& lhs, const Vec3& rhs);

float dot(const Vec3& lhs, const Vec3& rhs);
Vec3 cross(const Vec3& lhs, const Vec3& rhs);

Mat33 operator*(const Mat33& lhs, const Mat33& rhs);
Mat33 operator+(const Mat33& lhs, const Mat33& rhs);
Mat33 operator-(const Mat33& lhs, const Mat33& rhs);
Vec3 operator*(const Mat33& lhs, const Vec3& rhs);

Rotor operator*(const Rotor& lhs, const Rotor& rhs);

// Misc
// ====
int modulo(int n, int d); // works for negative numbers, unlike %

#ifdef FAST_BUILD
#include "math.cpp"
#endif

#endif // include guard
