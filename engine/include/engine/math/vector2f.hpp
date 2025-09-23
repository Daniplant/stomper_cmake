#pragma once
#include "engine/common.hpp"

#include <cmath>
#include <spdlog/fmt/bundled/format.h>

#ifndef VECTOR2_INLINE_IMPL
#define VECTOR2_INLINE_IMPL
#endif

namespace core::math
{
    struct Vector2f
    {
        constexpr Vector2f();

        constexpr Vector2f(float v);

        constexpr Vector2f(float x, float y);

        constexpr CORE_FORCE_INLINE bool operator==(const Vector2f& v) const;

        constexpr CORE_FORCE_INLINE Vector2f operator-() const;

        constexpr CORE_FORCE_INLINE Vector2f operator+(const Vector2f& rhs) const;

        constexpr CORE_FORCE_INLINE Vector2f operator-(const Vector2f& rhs) const;

        constexpr CORE_FORCE_INLINE Vector2f operator*(float rhs) const;

        constexpr CORE_FORCE_INLINE Vector2f operator/(float rhs) const;

        constexpr CORE_FORCE_INLINE Vector2f& operator+=(const Vector2f& rhs);

        constexpr CORE_FORCE_INLINE Vector2f& operator-=(const Vector2f& rhs);

        constexpr CORE_FORCE_INLINE Vector2f& operator*=(float rhs);

        constexpr CORE_FORCE_INLINE Vector2f& operator/=(float rhs);

        CORE_FORCE_INLINE auto length() const;

        constexpr CORE_FORCE_INLINE float sq_length() const;

        Vector2f& normalize();

        Vector2f normalized() const;

        static constexpr CORE_FORCE_INLINE float dot(const Vector2f& lhs, const Vector2f& rhs);

        union {
            struct
            {
                float x, y;
            };

            struct
            {
                float u, v;
            };

            struct
            {
                float r, g;
            };

            float data[2];
        };

        static const Vector2f Zero, One, Up, Down, Left, Right;

    private:
        static constexpr f32 m_epsilon = 0.00001f;
    };

#include "inl/vector2f.inl"
} // namespace core::math

#undef VECTOR2_INLINE_IMPL

template <> class fmt::formatter<core::math::Vector2f>
{
public:
    constexpr auto parse(format_parse_context& ctx) { return ctx.begin(); }

    template <typename Context> constexpr auto format(core::math::Vector2f const& vector, Context& ctx) const {
        return format_to(ctx.out(), "({}, {})", vector.x, vector.y);
    }
};
