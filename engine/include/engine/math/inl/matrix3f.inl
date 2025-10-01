#pragma once

#ifndef MATRIX3_INLINE_IMPL
#error "Do not include matrix3f.inl, include engine/math/matrix3f.hpp instead"
#else

constexpr Matrix3f::Matrix3f() {
	rows[0] = Vector3f();
	rows[1] = Vector3f();
	rows[2] = Vector3f();
}

constexpr Matrix3f::Matrix3f(float v) {
	rows[0] = Vector3f(v);
	rows[1] = Vector3f(v);
	rows[2] = Vector3f(v);
}

constexpr Matrix3f::Matrix3f(Vector3f row0, Vector3f row1, Vector3f row2) {
	rows[0] = row0;
	rows[1] = row1;
	rows[2] = row2;
}

constexpr Matrix3f::Matrix3f(float m00, float m01, float m02, float m10, float m11, float m12, float m20, float m21,
    float m22)
    : m00(m00)
    , m01(m01)
    , m02(m02)
    , m10(m10)
    , m11(m11)
    , m12(m12)
    , m20(m20)
    , m21(m21)
    , m22(m22)
    { }

constexpr Matrix3f Matrix3f::operator-() const { return { -rows[0], -rows[1], -rows[2] }; }

constexpr bool Matrix3f::operator==(const Matrix3f& rhs) const {
    return rows[0] == rhs.rows[0] && rows[1] == rhs.rows[1] && rows[2] == rhs.rows[2];
}

constexpr Matrix3f Matrix3f::operator+(const Matrix3f& rhs) const {
    return { rows[0] + rhs.rows[0], rows[1] + rhs.rows[1], rows[2] + rhs.rows[2]};
}

constexpr Matrix3f Matrix3f::operator-(const Matrix3f& rhs) const {
    return { rows[0] - rhs.rows[0], rows[1] - rhs.rows[1], rows[2] - rhs.rows[2] };
}

constexpr Matrix3f Matrix3f::operator*(const Matrix3f& rhs) const {
    return {data[0] * rhs.data[0] + data[1] * rhs.data[3] + data[2] * rhs.data[6],
              data[0] * rhs.data[1] + data[1] * rhs.data[4] +  data[2] * rhs.data[7],
              data[0] * rhs.data[2] + data[1] * rhs.data[5] + data[2] * rhs.data[8],
                
              data[3] * rhs.data[0] + data[4] * rhs.data[3] + data[5] * rhs.data[6],
              data[3] * rhs.data[1] + data[4] * rhs.data[4] + data[5] * rhs.data[7],
              data[3] * rhs.data[2] + data[4] * rhs.data[5] + data[5] * rhs.data[8],

              data[6] * rhs.data[0] + data[7] * rhs.data[3] + data[8] * rhs.data[6],
              data[6] * rhs.data[1] + data[7] * rhs.data[4] + data[8] * rhs.data[7],
              data[6] * rhs.data[2] + data[7] * rhs.data[5] + data[8] * rhs.data[8]};
}

constexpr Matrix3f Matrix3f::operator+(float rhs) const { return { rows[0] + Vector3f(rhs), rows[1] + Vector3f(rhs), rows[2] + Vector3f(rhs) }; }

constexpr Matrix3f Matrix3f::operator-(float rhs) const { return { rows[0] - Vector3f(rhs), rows[1] - Vector3f(rhs), rows[2] - Vector3f(rhs) }; }

constexpr Matrix3f Matrix3f::operator*(float rhs) const { return { rows[0] * rhs, rows[1] * rhs, rows[2] * rhs }; }

constexpr Matrix3f Matrix3f::operator/(float rhs) const { return { rows[0] / rhs, rows[1] / rhs, rows[2] / rhs }; }

inline constexpr Matrix3f Matrix3f::Zero(0, 0, 0, 0, 0, 0, 0, 0, 0);
inline constexpr Matrix3f Matrix3f::Identity(1, 0, 0, 0, 1, 0, 0, 0, 1);

#endif