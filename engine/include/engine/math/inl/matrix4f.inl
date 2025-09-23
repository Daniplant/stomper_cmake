#pragma once

#ifndef MATRIX4_INLINE_IMPL
#error "Do not include matrix4f.inl, include engine/math/matrix4f.hpp instead"
#else

constexpr matrix4f::matrix4f() {
    rows[0] = Vector4f();
    rows[1] = Vector4f();
    rows[2] = Vector4f();
    rows[3] = Vector4f();
}

constexpr matrix4f::matrix4f(float v) {
    rows[0] = Vector4f(v);
    rows[1] = Vector4f(v);
    rows[2] = Vector4f(v);
    rows[3] = Vector4f(v);
}

constexpr matrix4f::matrix4f(Vector4f row0, Vector4f row1, Vector4f row2, Vector4f row3) {
    rows[0] = row0;
    rows[1] = row1;
    rows[2] = row2;
    rows[3] = row3;
}

constexpr matrix4f::matrix4f(float m00, float m01, float m02, float m03, float m10, float m11, float m12, float m13, float m20, float m21,
    float m22, float m23, float m30, float m31, float m32, float m33)
    : m00(m00)
    , m01(m01)
    , m02(m02)
    , m03(m03)
    , m10(m10)
    , m11(m11)
    , m12(m12)
    , m13(m13)
    , m20(m20)
    , m21(m21)
    , m22(m22)
    , m23(m23)
    , m30(m30)
    , m31(m31)
    , m32(m32)
    , m33(m33) { }

constexpr matrix4f matrix4f::operator-() const { return { -rows[0], -rows[1], -rows[2], -rows[3] }; }

constexpr bool matrix4f::operator==(const matrix4f& rhs) const {
    return rows[0] == rhs.rows[0] && rows[1] == rhs.rows[1] && rows[2] == rhs.rows[2] && rows[3] == rhs.rows[3];
}

constexpr matrix4f matrix4f::operator+(const matrix4f& rhs) const {
    return { rows[0] + rhs.rows[0], rows[1] + rhs.rows[1], rows[2] + rhs.rows[2], rows[3] + rhs.rows[3] };
}

constexpr matrix4f matrix4f::operator-(const matrix4f& rhs) const {
    return { rows[0] - rhs.rows[0], rows[1] - rhs.rows[1], rows[2] - rhs.rows[2], rows[3] - rhs.rows[3] };
}

constexpr matrix4f matrix4f::operator*(const matrix4f& rhs) const {
    return { data[0] * rhs.data[0] + data[1] * rhs.data[4] + data[2] * rhs.data[8] + data[3] * rhs.data[12],
        data[0] * rhs.data[1] + data[1] * rhs.data[5] + data[2] * rhs.data[9] + data[3] * rhs.data[13],
        data[0] * rhs.data[2] + data[1] * rhs.data[6] + data[2] * rhs.data[10] + data[3] * rhs.data[14],
        data[0] * rhs.data[3] + data[1] * rhs.data[7] + data[2] * rhs.data[11] + data[3] * rhs.data[15],
        data[4] * rhs.data[0] + data[5] * rhs.data[4] + data[6] * rhs.data[8] + data[7] * rhs.data[12],
        data[4] * rhs.data[1] + data[5] * rhs.data[5] + data[6] * rhs.data[9] + data[7] * rhs.data[13],
        data[4] * rhs.data[2] + data[5] * rhs.data[6] + data[6] * rhs.data[10] + data[7] * rhs.data[14],
        data[4] * rhs.data[3] + data[5] * rhs.data[7] + data[6] * rhs.data[11] + data[7] * rhs.data[15],
        data[8] * rhs.data[0] + data[9] * rhs.data[4] + data[10] * rhs.data[8] + data[11] * rhs.data[12],
        data[8] * rhs.data[1] + data[9] * rhs.data[5] + data[10] * rhs.data[9] + data[11] * rhs.data[13],
        data[8] * rhs.data[2] + data[9] * rhs.data[6] + data[10] * rhs.data[10] + data[11] * rhs.data[14],
        data[8] * rhs.data[3] + data[9] * rhs.data[7] + data[10] * rhs.data[11] + data[11] * rhs.data[15],
        data[12] * rhs.data[0] + data[13] * rhs.data[4] + data[14] * rhs.data[8] + data[15] * rhs.data[12],
        data[12] * rhs.data[1] + data[13] * rhs.data[5] + data[14] * rhs.data[9] + data[15] * rhs.data[13],
        data[12] * rhs.data[2] + data[13] * rhs.data[6] + data[14] * rhs.data[10] + data[15] * rhs.data[14],
        data[12] * rhs.data[3] + data[13] * rhs.data[7] + data[14] * rhs.data[11] + data[15] * rhs.data[15] };
}

constexpr matrix4f matrix4f::operator+(float rhs) const { return { rows[0] + rhs, rows[1] + rhs, rows[2] + rhs, rows[3] + rhs }; }

constexpr matrix4f matrix4f::operator-(float rhs) const { return { rows[0] - rhs, rows[1] - rhs, rows[2] - rhs, rows[3] - rhs }; }

constexpr matrix4f matrix4f::operator*(float rhs) const { return { rows[0] * rhs, rows[1] * rhs, rows[2] * rhs, rows[3] * rhs }; }

constexpr matrix4f matrix4f::operator/(float rhs) const { return { rows[0] / rhs, rows[1] / rhs, rows[2] / rhs, rows[3] / rhs }; }

matrix4f matrix4f::perspective_fov(float fov, float aspect, float z_near, float z_far) {
    float sin_fov = std::sin(0.5f * fov);
    float cos_fov = std::cos(0.5f * fov);
    float h       = cos_fov / sin_fov;
    float w       = h / aspect;
    float f_range = z_far / (z_far - z_near);

    return { w, 0.f, 0.f, 0.f, 0.0f, h, 0.f, 0.f, 0.0f, 0.f, f_range, 0.0f, 0.0f, 0.0f, -f_range * z_near, 0.0f };
}

matrix4f matrix4f::perspective_offcenter(float left, float right, float bottom, float top, float z_near, float z_far) {
    float inv_width  = 1.0f / (right - left);
    float inv_height = 1.0f / (top - bottom);
    float f_range    = z_far / (z_far - z_near);

    return { 2 * z_near * inv_width, 0.0f, 0.0f, 0.0f, 0.0f, 2 * z_near * inv_height, 0.0f, 0.0f, -(left + right) * inv_width,
        -(top + bottom) * inv_height, f_range, 1.0f, 0.0f, 0.0f, -f_range * z_near, 0.0f };
}

inline constexpr matrix4f matrix4f::Zero(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
inline constexpr matrix4f matrix4f::Identity(1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1);

#endif
