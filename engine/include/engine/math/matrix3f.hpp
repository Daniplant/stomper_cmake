#pragma once

#include "engine/common.hpp"
#include "vector3f.hpp"

#include <cmath>
#include <spdlog/fmt/bundled/format.h>

#ifndef MATRIX3_INLINE_IMPL
#define MATRIX3_INLINE_IMPL
#endif

namespace core::math
{
    struct matrix3f
    {
        constexpr matrix3f();

        constexpr matrix3f(Vector3f row0, Vector3f row1, Vector3f row2);

        constexpr matrix3f(float m00, float m01, float m02, float m10, float m11, float m12, float m20, float m21, float m22);

        constexpr matrix3f(float v);

        constexpr CORE_FORCE_INLINE matrix3f operator-() const;

        constexpr CORE_FORCE_INLINE matrix3f operator==(const matrix3f& rhs) const;

        constexpr CORE_FORCE_INLINE matrix3f operator+(const matrix3f& rhs) const;

        constexpr CORE_FORCE_INLINE matrix3f operator-(const matrix3f& rhs) const;

        constexpr CORE_FORCE_INLINE matrix3f operator*(const matrix3f& rhs) const;

        constexpr CORE_FORCE_INLINE matrix3f operator+(float rhs) const;

        constexpr CORE_FORCE_INLINE matrix3f operator-(float rhs) const;

        constexpr CORE_FORCE_INLINE matrix3f operator*(float rhs) const;

        constexpr CORE_FORCE_INLINE matrix3f operator/(float rhs) const;

        union {
            struct
            {
                float m00, m01, m02;
                float m10, m11, m12;
                float m20, m21, m22;
            };

            Vector3f rows[3];
            float data[9];
        };

        static const matrix3f Zero, Identity;
    };
#include "inl/matrix3f.inl"
}

#undef MATRIX3_INLINE_IMPL