#pragma once
#include "engine/common.hpp"

#include <cmath>
#include <spdlog/fmt/bundled/format.h>

#ifndef VECTOR4_INLINE_IMPL
#define VECTOR4_INLINE_IMPL
#endif

namespace core::math
{
    struct Vector4f
    {
        constexpr Vector4f();

        constexpr Vector4f(float v);

        constexpr Vector4f(float x, float y, float z, float w);

        constexpr CORE_FORCE_INLINE bool operator==(const Vector4f& v) const;

        constexpr CORE_FORCE_INLINE Vector4f operator-() const;

        constexpr CORE_FORCE_INLINE Vector4f operator+(const Vector4f& rhs) const;

        constexpr CORE_FORCE_INLINE Vector4f operator-(const Vector4f& rhs) const;

        constexpr CORE_FORCE_INLINE Vector4f operator*(float rhs) const;

        constexpr CORE_FORCE_INLINE Vector4f operator/(float rhs) const;

        constexpr CORE_FORCE_INLINE Vector4f& operator+=(const Vector4f& rhs);

        constexpr CORE_FORCE_INLINE Vector4f& operator-=(const Vector4f& rhs);

        constexpr CORE_FORCE_INLINE Vector4f& operator*=(float rhs);

        constexpr CORE_FORCE_INLINE Vector4f& operator/=(float rhs);

        CORE_FORCE_INLINE auto length() const;

        constexpr CORE_FORCE_INLINE float sq_length() const;

        CORE_FORCE_INLINE Vector4f& normalize();

        CORE_FORCE_INLINE Vector4f normalized() const;

        union {
            struct
            {
                float x, y, z, w;
            };

            struct
            {
                float r, g, b, a;
            };

            float data[4];
        };

        static const Vector4f Zero, One, Up, Down, Left, Right, Forward, Backward;

    private:
        static constexpr f32 m_epsilon = 0.00001f;
    };

#include "inl/vector4f.inl"
} // namespace core::math

#undef VECTOR4_INLINE_IMPL

template <> class fmt::formatter<core::math::Vector4f>
{
public:
    constexpr auto parse(format_parse_context& ctx) { return ctx.begin(); }

    template <typename Context> constexpr auto format(core::math::Vector4f const& vector, Context& ctx) const {
        return format_to(ctx.out(), "({}, {}, {}, {})", vector.x, vector.y, vector.z, vector.w);
    }
};
