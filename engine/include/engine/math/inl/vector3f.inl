#pragma once

#ifndef VECTOR3_INLINE_IMPL
#error "Do not include vector3f.inl, include engine/math/vector3f.hpp instead"
#else

constexpr Vector3f::Vector3f()
    : x(0.0f)
    , y(0.0f)
    , z(0.0f) { }

constexpr Vector3f::Vector3f(float v)
    : x(v)
    , y(v)
    , z(v) { }

constexpr Vector3f::Vector3f(float x, float y, float z)
    : x(x)
    , y(y)
    , z(z) { }

constexpr bool Vector3f::operator==(const Vector3f& v) const { return x == v.x && y == v.y && z == v.z; }

constexpr Vector3f Vector3f::operator-() const { return Vector3f(-x, -y, -z); }

constexpr Vector3f Vector3f::operator+(const Vector3f& rhs) const { return Vector3f(x + rhs.x, y + rhs.y, z + rhs.z); }

constexpr Vector3f Vector3f::operator-(const Vector3f& rhs) const { return Vector3f(x - rhs.x, y - rhs.y, z - rhs.z); }

constexpr Vector3f Vector3f::operator*(float rhs) const { return Vector3f(x * rhs, y * rhs, z * rhs); }

constexpr Vector3f operator*(float lhs, const Vector3f& rhs) { return Vector3f(lhs * rhs.x, lhs * rhs.y, lhs * rhs.z); }

constexpr Vector3f Vector3f::operator/(float rhs) const { return Vector3f(x / rhs, y / rhs, z / rhs); }

constexpr Vector3f& Vector3f::operator+=(const Vector3f& rhs) {
    x += rhs.x;
    y += rhs.y;
    z += rhs.z;
    return *this;
}

constexpr Vector3f& Vector3f::operator-=(const Vector3f& rhs) {
    x -= rhs.x;
    y -= rhs.y;
    z -= rhs.z;
    return *this;
}

constexpr Vector3f& Vector3f::operator*=(float rhs) {
    x *= rhs;
    y *= rhs;
    z *= rhs;
    return *this;
}

constexpr Vector3f& Vector3f::operator/=(float rhs) {
    x /= rhs;
    y /= rhs;
    z /= rhs;
    return *this;
}

auto Vector3f::length() const { return std::sqrt(x * x + y * y + z * z); }

constexpr float Vector3f::sq_length() const { return x * x + y * y + z * z; }

Vector3f& Vector3f::normalize() {
    if (auto len = length(); len > m_epsilon) {
        auto inv = 1.0f / len;
        return *this *= inv;
    }
    x = 0.0f;
    y = 0.0f;
    z = 0.0f;
    return *this;
}

Vector3f Vector3f::normalized() const {
    if (auto len = length(); len > m_epsilon) {
        auto inv = 1.0f / len;
        return Vector3f(x * inv, y * inv, z * inv);
    }
    return Vector3f(0.0f, 0.0f, 0.0f);
}

constexpr float Vector3f::dot(const Vector3f& lhs, const Vector3f& rhs) { return lhs.x * rhs.x + lhs.y * rhs.y + lhs.z * rhs.z; }

constexpr Vector3f Vector3f::cross(const Vector3f& lhs, const Vector3f& rhs) {
    return Vector3f(lhs.y * rhs.z - lhs.z * rhs.y, lhs.z * rhs.x - lhs.x * rhs.z, lhs.x * rhs.y - lhs.y * rhs.x);
}

inline constexpr Vector3f Vector3f::Zero    = Vector3f(0.0f, 0.0f, 0.0f);
inline constexpr Vector3f Vector3f::One     = Vector3f(1.0f, 1.0, 1.0f);
inline constexpr Vector3f Vector3f::Up      = Vector3f(0.0f, 1.0f, 0.0f);
inline constexpr Vector3f Vector3f::Down    = Vector3f(0.0f, -1.0f, 0.0f);
inline constexpr Vector3f Vector3f::Left    = Vector3f(-1.0f, 0.0f, 0.0f);
inline constexpr Vector3f Vector3f::Right   = Vector3f(1.0f, 0.0f, 0.0f);
inline constexpr Vector3f Vector3f::Forward = Vector3f(0.0f, 0.0f, 1.0f);

#endif
