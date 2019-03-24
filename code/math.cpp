#include "hb/math.h"
#include <cmath>
#include <iostream>
#include <cassert>

using std::cout;
using std::endl;

Vec2::Vec2(float _x, float _y): x(_x), y(_y)
{}

Vec2 operator+(const Vec2& lhs, const Vec2& rhs)
{
    Vec2 result = lhs;
    result += rhs;
    return result;
}

Vec2 operator-(const Vec2& lhs, const Vec2& rhs)
{
    Vec2 result = lhs;
    result -= rhs;
    return result;
}

Vec2 operator*(float lhs, const Vec2& rhs)
{
    Vec2 result = rhs;
    result.x *= lhs;
    result.y *= lhs;
    return result;
}

Vec2 operator*(const Vec2& lhs, float rhs)
{
    return rhs * lhs;
}

Vec2& Vec2::operator+=(const Vec2& rhs)
{
    x += rhs.x;
    y += rhs.y;
    return *this;
}

Vec2& Vec2::operator-=(const Vec2& rhs)
{
    x -= rhs.x;
    y -= rhs.y;
    return *this;
}

bool operator==(const Vec2& lhs, const Vec2& rhs)
{
    return (lhs.x == rhs.x) && (lhs.y == rhs.y);
}

float Vec2::norm() const
{
    return sqrtf(x * x + y * y);
}

Vec2 Vec2::normalize() const
{
    float magnitude = norm();
    return Vec2(x / magnitude, y / magnitude);
}

Vec2 operator-(const Vec2& operand)
{
    Vec2 result;
    result.x = -operand.x;
    result.y = -operand.y;
    return result;
}

Vec2 unit_angle(float angle)
{
    Vec2 result;
    result.x = cosf(angle);
    result.y = sinf(angle);
    return result;
}

Mat22::Mat22(float a, float b, float c, float d)
    :data{a, b, c, d}
{}

Mat22 Mat22::rotation(float theta)
{
    float costheta = cosf(theta);
    float sintheta = sinf(theta);
    return Mat22(costheta, -sintheta, sintheta, costheta);
}

Vec2 operator*(const Mat22& lhs, const Vec2& rhs)
{
    return Vec2(lhs.data[0] * rhs.x + lhs.data[1] * rhs.y,
                lhs.data[2] * rhs.x + lhs.data[3] * rhs.y);
}

float dot(const Vec2& lhs, const Vec2& rhs)
{
    return lhs.x * rhs.x + lhs.y * rhs.y;
}
// 3D

Vec3::Vec3(float _x, float _y, float _z)
:x(_x), y(_y), z(_z) {}

Vec3& Vec3::operator+=(const Vec3& rhs)
{
    x += rhs.x;
    y += rhs.y;
    z += rhs.z;
    return *this;
}

Vec3& Vec3::operator-=(const Vec3& rhs)
{
    x -= rhs.x;
    y -= rhs.y;
    z -= rhs.z;
    return *this;
}

float Vec3::norm() const
{
    return sqrtf(x * x + y * y + z * z);
}

Vec3 Vec3::normalize() const
{
    float magnitude = norm();
    return Vec3(x / magnitude, y / magnitude, z / magnitude);
}

Vec3 operator-(const Vec3& operand)
{
    return Vec3(-operand.x, -operand.y, -operand.z);
}

Vec3 operator+(const Vec3& lhs, const Vec3& rhs)
{
    Vec3 result = lhs;
    result += rhs;
    return result;
}

Vec3 operator-(const Vec3& lhs, const Vec3& rhs)
{
    Vec3 result = lhs;
    result -= rhs;
    return result;
}

Vec3 operator*(float lhs, const Vec3& rhs)
{
    Vec3 result = rhs;
    result.x *= lhs;
    result.y *= lhs;
    result.z *= lhs;
    return result;
}

Vec3 operator*(const Vec3& lhs, float rhs)
{
    Vec3 result = lhs;
    result.x *= rhs;
    result.y *= rhs;
    result.z *= rhs;
    return result;
}

float dot(const Vec3& lhs, const Vec3& rhs)
{
    return lhs.x * rhs.x + lhs.y * rhs.y + lhs.z * rhs.z;
}

Vec3 cross(const Vec3& lhs, const Vec3& rhs)
{
    return Vec3(lhs.y * rhs.z - lhs.z * rhs.y,
                lhs.z * rhs.x - lhs.x * rhs.z,
                lhs.x * rhs.y - lhs.y * rhs.z);
}

Mat33::Mat33(float e11, float e12, float e13,
             float e21, float e22, float e23,
             float e31, float e32, float e33)
{
    data[0] = e11;
    data[1] = e12;
    data[2] = e13;
    data[3] = e21;
    data[4] = e22;
    data[5] = e23;
    data[6] = e31;
    data[7] = e32;
    data[8] = e33;
}

void Mat33::print() const
{
    cout << "| " << data[0] << "\t" << data[1] << "\t" << data[2] << "\t|" << endl;
    cout << "| " << data[0] << "\t" << data[1] << "\t" << data[2] << "\t|" << endl;
    cout << "| " << data[0] << "\t" << data[1] << "\t" << data[2] << "\t|" << endl;
    cout << endl;
}

Mat33 operator*(const Mat33& lhs, const Mat33& rhs)
{
    Mat33 result;
    result.data[0] = lhs.data[0] * rhs.data[0] + lhs.data[1] * rhs.data[3] + lhs.data[2] * rhs.data[6];
    result.data[1] = lhs.data[0] * rhs.data[1] + lhs.data[1] * rhs.data[4] + lhs.data[2] * rhs.data[7];
    result.data[2] = lhs.data[0] * rhs.data[2] + lhs.data[1] * rhs.data[5] + lhs.data[2] * rhs.data[8];
    result.data[3] = lhs.data[3] * rhs.data[0] + lhs.data[4] * rhs.data[3] + lhs.data[5] * rhs.data[6];
    result.data[4] = lhs.data[3] * rhs.data[1] + lhs.data[4] * rhs.data[4] + lhs.data[5] * rhs.data[7];
    result.data[5] = lhs.data[3] * rhs.data[2] + lhs.data[4] * rhs.data[5] + lhs.data[5] * rhs.data[8];
    result.data[6] = lhs.data[6] * rhs.data[0] + lhs.data[7] * rhs.data[3] + lhs.data[8] * rhs.data[6];
    result.data[7] = lhs.data[6] * rhs.data[1] + lhs.data[7] * rhs.data[4] + lhs.data[8] * rhs.data[7];
    result.data[8] = lhs.data[6] * rhs.data[2] + lhs.data[7] * rhs.data[5] + lhs.data[8] * rhs.data[8];
    return result;
}

Mat33 operator+(const Mat33& lhs, const Mat33& rhs)
{
    Mat33 result;
    result.data[0] = lhs.data[0] + rhs.data[0];
    result.data[1] = lhs.data[1] + rhs.data[1];
    result.data[2] = lhs.data[2] + rhs.data[2];
    result.data[3] = lhs.data[3] + rhs.data[3];
    result.data[4] = lhs.data[4] + rhs.data[4];
    result.data[5] = lhs.data[5] + rhs.data[5];
    result.data[6] = lhs.data[6] + rhs.data[6];
    result.data[7] = lhs.data[7] + rhs.data[7];
    result.data[8] = lhs.data[8] + rhs.data[8];
    return result;
}

Mat33 operator-(const Mat33& lhs, const Mat33& rhs)
{
    Mat33 result;
    result.data[0] = lhs.data[0] - rhs.data[0];
    result.data[1] = lhs.data[1] - rhs.data[1];
    result.data[2] = lhs.data[2] - rhs.data[2];
    result.data[3] = lhs.data[3] - rhs.data[3];
    result.data[4] = lhs.data[4] - rhs.data[4];
    result.data[5] = lhs.data[5] - rhs.data[5];
    result.data[6] = lhs.data[6] - rhs.data[6];
    result.data[7] = lhs.data[7] - rhs.data[7];
    result.data[8] = lhs.data[8] - rhs.data[8];
    return result;
}

Mat44::Mat44(float *mat_data)
{
    memcpy(data, mat_data, sizeof(data));
}

Mat44 Mat44::perspective(float near, float far, float aspect_ratio)
{
    float scale = 0.07f;
    float data[16] = 
    {
        near * aspect_ratio / scale, 0.0f, 0.0f, 0.0f,
        0.0f, near / scale, 0.0f, 0.0f,
        0.0f, 0.0f, -(near + far) / (far - near), -2 * near * far / (far - near),
        0.0f, 0.0f, -1.0f, 0.0f
    };

    return Mat44(data);
}

Vec3 operator*(const Mat33& lhs, const Vec3& rhs)
{
    return Vec3(
        rhs.x * lhs.data[0] + rhs.y * lhs.data[1] + rhs.z * lhs.data[2],
        rhs.x * lhs.data[3] + rhs.y * lhs.data[4] + rhs.z * lhs.data[5],
        rhs.x * lhs.data[6] + rhs.y * lhs.data[7] + rhs.z * lhs.data[8]
    );
}

Rotor::Rotor(const Vec3& v1, const Vec3& v2)
{
    s = dot(v1, v2);
    xy = v1.x * v2.y - v1.y * v2.x;
    yz = v1.y * v2.z - v1.z * v2.y;
    zx = v1.z * v2.x - v1.x * v2.z;
}

Rotor operator*(const Rotor& lhs, const Rotor& rhs)
{
    Rotor result;
    result.s  = lhs.s * rhs.s  - lhs.xy * rhs.xy - lhs.yz * rhs.yz - lhs.zx * rhs.zx;
    result.xy = lhs.s * rhs.xy + lhs.xy * rhs.s  - lhs.yz * rhs.zx + lhs.zx * rhs.yz;
    result.yz = lhs.s * rhs.yz + lhs.xy * rhs.zx + lhs.yz * rhs.s  - lhs.zx * rhs.xy;
    result.zx = lhs.s * rhs.zx - lhs.xy * rhs.yz + lhs.yz * rhs.xy + lhs.zx * rhs.s;
    return result;
}

Mat33 Rotor::to_matrix() const
{
    float m11 = 1.0f - 2.0f * (xy * xy + zx * zx);
    float m12 = 2.0f * (yz * zx + s * xy);
    float m13 = 2.0f * (xy * yz - s * zx);

    float m21 = 2.0f * (yz * zx - s * xy);
    float m22 = 1.0f - 2.0f * (xy * xy + yz * yz);
    float m23 = 2.0f * (xy * zx + s * yz);

    float m31 = 2.0f * (xy * yz + s * zx);
    float m32 = 2.0f * (xy * zx - s * yz);
    float m33 = 1.0f - 2.0f * (yz * yz + zx * zx);

    return Mat33(m11, m12, m13,
                 m21, m22, m23,
                 m31, m32, m33);
}

Rotor Rotor::inverse() const
{
    Rotor result;
    result.s = s;
    result.xy = -xy;
    result.yz = -yz;
    result.zx = -zx;
    return result;
}

Rotor Rotor::normalize() const
{
    float magnitude = sqrtf(s*s + xy*xy + yz*yz + zx*zx);
    Rotor result;
    result.s = s / magnitude;
    result.xy = xy / magnitude;
    result.yz = yz / magnitude;
    result.zx = zx / magnitude;
    return result;
}

// rotation in xy plane
Rotor Rotor::roll(float angle)
{
    float cosine = cosf(0.5f * angle);
    float sine = sinf(0.5f * angle);
    
    Vec3 start = Vec3(1.0f, 0.0f, 0.0f);
    Vec3 end = Vec3(cosine, sine, 0.0f);

    return Rotor(start, end);
}

// rotation in yz plane
Rotor Rotor::pitch(float angle)
{
    float cosine = cosf(0.5f * angle);
    float sine = sinf(0.5f * angle);

    Vec3 start = Vec3(0.0f, 1.0f, 0.0f);
    Vec3 end = Vec3(0.0f, cosine, sine);

    return Rotor(start, end);
}

// rotation in zx plane
Rotor Rotor::yaw(float angle)
{

    float cosine = cosf(0.5f * angle);
    float sine = sinf(0.5f * angle);
    
    Vec3 start = Vec3(0.0f, 0.0f, 1.0f);
    Vec3 end = Vec3(sine, 0.0f, cosine);

    return Rotor(start, end);
}

Rotor Rotor::angle_axis(Vec3 bivector)
{
    float angle = bivector.norm();
    if (angle == 0.0f)
    {
        return Rotor(); // no rotation
    }
    else
    {
        Rotor result;
        result.s  = cosf(0.5f * angle);
        result.xy = (sinf(0.5f * angle) / angle) * bivector.x;
        result.yz = (sinf(0.5f * angle) / angle) * bivector.y;
        result.zx = (sinf(0.5f * angle) / angle) * bivector.z;
        return result;
    }
}

Rotor Rotor::exponentiate(float t)
{
    Vec3 bivector = Vec3(xy, yz, zx);
    float angle = acosf(s);
    angle *= t;

    Rotor result;
    result.s = cosf(angle);
    if (bivector.norm() != 0)
    {
        bivector = bivector.normalize() * sinf(angle);
        result.xy = bivector.x;
        result.yz = bivector.y;
        result.zx = bivector.z;
    }
    else
    {
        result.xy = 0;
        result.yz = 0;
        result.zx = 0;
    }
    return result;
    // (I barely understand this)
}

void Rotor::to_angle_axis(float *angle, Vec3 *axis) const
{
    // rotor has form: cos(a) + sin(a)v
    // where v is a unit bivector
    *angle = acosf(s);

    float sin_angle = sin(*angle);
    if (sin_angle != 0.0f)
    {
        *axis = Vec3(xy, yz, zx);
        *axis = *axis * (1.0f / sin_angle);
    }
    else
    {
        *axis = Vec3(1.0f, 0.0f, 0.0f);
    }
}

Rotor Rotor::lerp(const Rotor &start, const Rotor &end, float t)
{
    Rotor result;
    result.s  = start.s  + t * (end.s  - start.s);
    result.xy = start.xy + t * (end.xy - start.xy);
    result.yz = start.yz + t * (end.yz - start.yz);
    result.zx = start.zx + t * (end.zx - start.zx);
    result = result.normalize();
    return result;
}

Rotor Rotor::slerp(const Rotor &start, const Rotor &end, float t)
{
    return start * ((start.inverse() * end).exponentiate(t));
}

// misc

int modulo(int a, int b) {
    if (a < 0) {
        return a%b + b;
    }
    else {
        return a%b;
    }
}
