#pragma once

#ifndef VECTOR2_INLINE_IMPL
#error "Do not include vector2f.inl, include engine/math/vector2f.hpp instead"
#else

constexpr Vector2f::Vector2f()
    : x(0.0f)
    , y(0.0f) { }

constexpr Vector2f::Vector2f(float v)
    : x(v)
    , y(v) { }

constexpr Vector2f::Vector2f(float x, float y)
    : x(x)
    , y(y) { }

constexpr bool Vector2f::operator==(const Vector2f& v) const { return x == v.x && y == v.y; }

constexpr Vector2f Vector2f::operator-() const {
    static_assert(std::is_signed_v<float>, "You can't negate an unsigned vector");
    return Vector2f(-x, -y);
}

constexpr Vector2f Vector2f::operator+(const Vector2f& rhs) const { return Vector2f(x + rhs.x, y + rhs.y); }

constexpr Vector2f Vector2f::operator-(const Vector2f& rhs) const { return Vector2f(x - rhs.x, y - rhs.y); }

constexpr Vector2f Vector2f::operator*(float rhs) const { return Vector2f(x * rhs, y * rhs); }

constexpr Vector2f operator*(float lhs, const Vector2f& rhs) { return Vector2f(lhs * rhs.x, lhs * rhs.y); }

constexpr Vector2f Vector2f::operator/(float rhs) const { return Vector2f(x / rhs, y / rhs); }

constexpr Vector2f& Vector2f::operator+=(const Vector2f& rhs) {
    x += rhs.x;
    y += rhs.y;
    return *this;
}

constexpr Vector2f& Vector2f::operator-=(const Vector2f& rhs) {
    x -= rhs.x;
    y -= rhs.y;
    return *this;
}

constexpr Vector2f& Vector2f::operator*=(float rhs) {
    x *= rhs;
    y *= rhs;
    return *this;
}

constexpr Vector2f& Vector2f::operator/=(float rhs) {
    x /= rhs;
    y /= rhs;
    return *this;
}

auto Vector2f::length() const { return std::sqrt(x * x + y * y); }

constexpr float Vector2f::sq_length() const { return x * x + y * y; }

Vector2f& Vector2f::normalize() {
    if (auto len = length(); len > m_epsilon) {
        return *this /= len;
    }
    x = 0.0f;
    y = 0.0f;
    return *this;
}

Vector2f Vector2f::normalized() const {
    if (auto len = length(); len > m_epsilon) {
        return Vector2f(x / len, y / len);
    }
    return Vector2f(0.0f, 0.0f);
}

constexpr float Vector2f::dot(const Vector2f& lhs, const Vector2f& rhs) { return lhs.x * rhs.x + lhs.y * rhs.y; }

inline constexpr Vector2f Vector2f::Zero  = Vector2f(0.0f, 0.0f);
inline constexpr Vector2f Vector2f::One   = Vector2f(1.0f, 1.0f);
inline constexpr Vector2f Vector2f::Up    = Vector2f(0.0f, 1.0f);
inline constexpr Vector2f Vector2f::Down  = Vector2f(0.0f, -1.0f);
inline constexpr Vector2f Vector2f::Left  = Vector2f(-1.0f, 0.0f);
inline constexpr Vector2f Vector2f::Right = Vector2f(1.0f, 0.0f);

#endif
