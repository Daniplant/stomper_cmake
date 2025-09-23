#pragma once

#include "engine/common.hpp"
#include "vector4f.hpp"

#include <cmath>
#include <spdlog/fmt/bundled/format.h>

#ifndef MATRIX4_INLINE_IMPL
#define MATRIX4_INLINE_IMPL
#endif

namespace core::math
{
    struct matrix4f
    {
        constexpr matrix4f();

        constexpr matrix4f(float v);

        constexpr matrix4f(Vector4f row0, Vector4f row1, Vector4f row2, Vector4f row3);

        constexpr matrix4f(float m00, float m01, float m02, float m03, float m10, float m11, float m12, float m13, float m20, float m21,
            float m22, float m23, float m30, float m31, float m32, float m33);

        constexpr CORE_FORCE_INLINE matrix4f operator-() const;

        constexpr CORE_FORCE_INLINE bool operator==(const matrix4f& rhs) const;

        constexpr CORE_FORCE_INLINE matrix4f operator+(const matrix4f& rhs) const;

        constexpr CORE_FORCE_INLINE matrix4f operator-(const matrix4f& rhs) const;

        constexpr CORE_FORCE_INLINE matrix4f operator*(const matrix4f& rhs) const;

        constexpr CORE_FORCE_INLINE matrix4f operator+(float rhs) const;

        constexpr CORE_FORCE_INLINE matrix4f operator-(float rhs) const;

        constexpr CORE_FORCE_INLINE matrix4f operator*(float rhs) const;

        constexpr CORE_FORCE_INLINE matrix4f operator/(float rhs) const;

        static CORE_FORCE_INLINE matrix4f perspective_fov(float fov, float aspect, float z_near, float z_far);

        static CORE_FORCE_INLINE matrix4f perspective_offcenter(
            float left, float right, float bottom, float top, float z_near, float z_far);

        union {
            struct
            {
                float m00, m01, m02, m03;
                float m10, m11, m12, m13;
                float m20, m21, m22, m23;
                float m30, m31, m32, m33;
            };

            Vector4f rows[4];
            float data[16];
        };

        static const matrix4f Zero, Identity;
    };

#include "inl/matrix4f.inl"
} // namespace core::math

#undef MATRIX4_INLINE_IMPL
