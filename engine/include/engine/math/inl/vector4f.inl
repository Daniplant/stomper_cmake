#pragma once

#ifndef VECTOR4_INLINE_IMPL
#error "Do not include vector4f.inl, include engine/math/vector4f.hpp instead"
#else

constexpr Vector4f::Vector4f()
    : x(0.0f)
    , y(0.0f)
    , z(0.0f)
    , w(0.0f) { }

constexpr Vector4f::Vector4f(float v)
    : x(v)
    , y(v)
    , z(v)
    , w(v) { }

constexpr Vector4f::Vector4f(float x, float y, float z, float w)
    : x(x)
    , y(y)
    , z(z)
    , w(w) { }

constexpr bool Vector4f::operator==(const Vector4f& v) const { return x == v.x && y == v.y && z == v.z && w == v.w; }

constexpr Vector4f Vector4f::operator-() const { return Vector4f(-x, -y, -z, -w); }

constexpr Vector4f Vector4f::operator+(const Vector4f& rhs) const { return Vector4f(x + rhs.x, y + rhs.y, z + rhs.z, w + rhs.w); }

constexpr Vector4f Vector4f::operator-(const Vector4f& rhs) const { return Vector4f(x - rhs.x, y - rhs.y, z - rhs.z, w - rhs.w); }

constexpr Vector4f Vector4f::operator*(float rhs) const { return Vector4f(x * rhs, y * rhs, z * rhs, w * rhs); }

constexpr Vector4f operator*(float lhs, const Vector4f& rhs) { return Vector4f(lhs * rhs.x, lhs * rhs.y, lhs * rhs.z, lhs * rhs.w); }

constexpr Vector4f Vector4f::operator/(float rhs) const { return Vector4f(x / rhs, y / rhs, z / rhs, w / rhs); }

constexpr Vector4f& Vector4f::operator+=(const Vector4f& rhs) {
    x += rhs.x;
    y += rhs.y;
    z += rhs.z;
    w += rhs.w;
    return *this;
}

constexpr Vector4f& Vector4f::operator-=(const Vector4f& rhs) {
    x -= rhs.x;
    y -= rhs.y;
    z -= rhs.z;
    w -= rhs.w;
    return *this;
}

constexpr Vector4f& Vector4f::operator*=(float rhs) {
    x *= rhs;
    y *= rhs;
    z *= rhs;
    w *= rhs;
    return *this;
}

constexpr Vector4f& Vector4f::operator/=(float rhs) {
    x /= rhs;
    y /= rhs;
    z /= rhs;
    w /= rhs;
    return *this;
}

auto Vector4f::length() const { return std::sqrt(x * x + y * y + z * z + w * w); }

constexpr float Vector4f::sq_length() const { return x * x + y * y + z * z + w * w; }

Vector4f& Vector4f::normalize() {
    if (auto len = length(); len > m_epsilon) {
        auto inv = 1.0f / len;
        return *this *= inv;
    }
    x = 0.0f;
    y = 0.0f;
    z = 0.0f;
    w = 0.0f;
    return *this;
}

Vector4f Vector4f::normalized() const {
    if (auto len = length(); len > m_epsilon) {
        auto inv = 1.0f / len;
        return Vector4f(x * inv, y * inv, z * inv, w * inv);
    }
    return Vector4f(0.0f, 0.0f, 0.0f, 0.0f);
}

inline constexpr Vector4f Vector4f::Zero     = Vector4f(0.0f, 0.0f, 0.0f, 0.0f);
inline constexpr Vector4f Vector4f::One      = Vector4f(1.0f, 1.0, 1.0f, 1.0f);
inline constexpr Vector4f Vector4f::Up       = Vector4f(0.0f, 1.0f, 0.0f, 0.0f);
inline constexpr Vector4f Vector4f::Down     = Vector4f(0.0f, -1.0f, 0.0f, 0.0f);
inline constexpr Vector4f Vector4f::Left     = Vector4f(-1.0f, 0.0f, 0.0f, 0.0f);
inline constexpr Vector4f Vector4f::Right    = Vector4f(1.0f, 0.0f, 0.0f, 0.0f);
inline constexpr Vector4f Vector4f::Forward  = Vector4f(0.0f, 0.0f, 1.0f, 0.0f);
inline constexpr Vector4f Vector4f::Backward = Vector4f(0.0f, 0.0f, -1.0f, 0.0f);

#endif
