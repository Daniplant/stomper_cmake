#pragma once
#include "engine/common.hpp"

#include <cmath>
#include <spdlog/fmt/bundled/format.h>

#ifndef VECTOR3_INLINE_IMPL
#define VECTOR3_INLINE_IMPL
#endif

namespace core::math
{
    struct Vector3f
    {
        constexpr Vector3f();

        explicit constexpr Vector3f(float v);

        constexpr Vector3f(float x, float y, float z);

        constexpr CORE_FORCE_INLINE bool operator==(const Vector3f& v) const;

        constexpr CORE_FORCE_INLINE Vector3f operator-() const;

        constexpr CORE_FORCE_INLINE Vector3f operator+(const Vector3f& rhs) const;

        constexpr CORE_FORCE_INLINE Vector3f operator-(const Vector3f& rhs) const;

        constexpr CORE_FORCE_INLINE Vector3f operator*(float rhs) const;

        constexpr CORE_FORCE_INLINE Vector3f operator/(float rhs) const;

        constexpr CORE_FORCE_INLINE Vector3f& operator+=(const Vector3f& rhs);

        constexpr CORE_FORCE_INLINE Vector3f& operator-=(const Vector3f& rhs);

        constexpr CORE_FORCE_INLINE Vector3f& operator*=(float rhs);

        constexpr CORE_FORCE_INLINE Vector3f& operator/=(float rhs);

        CORE_FORCE_INLINE auto length() const;

        constexpr CORE_FORCE_INLINE float sq_length() const;

        CORE_FORCE_INLINE Vector3f& normalize();

        CORE_FORCE_INLINE Vector3f normalized() const;

        constexpr static CORE_FORCE_INLINE float dot(const Vector3f& lhs, const Vector3f& rhs);

        constexpr static CORE_FORCE_INLINE Vector3f cross(const Vector3f& lhs, const Vector3f& rhs);

        union {
            struct
            {
                float x, y, z;
            };

            struct
            {
                float u, v, w;
            };

            struct
            {
                float r, g, b;
            };

            float data[3];
        };

        static const Vector3f Zero, One, Up, Down, Left, Right, Forward, Backward;

    private:
        static constexpr f32 m_epsilon = 0.00001f;
    };

#include "inl/vector3f.inl"
} // namespace core::math

#undef VECTOR3_INLINE_IMPL

template <> class fmt::formatter<core::math::Vector3f>
{
public:
    constexpr auto parse(format_parse_context& ctx) { return ctx.begin(); }

    template <typename Context> constexpr auto format(core::math::Vector3f const& vector, Context& ctx) const {
        return format_to(ctx.out(), "({}, {}, {})", vector.x, vector.y, vector.z);
    }
};
