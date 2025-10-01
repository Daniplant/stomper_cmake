#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

#include <engine/math/matrix4f.hpp>

using namespace core::math;

TEST_CASE("matrix4f constructors", "[Matrix4]") {
    SECTION("Default constructor initializes to zero") {
        Matrix4f m;
        for (int i = 0; i < 16; i++) {
            REQUIRE(m.data[i] == Catch::Approx(0.0f));
        }
    }

    SECTION("Single-value constructor fills all entries") {
        Matrix4f m(5.0f);
        for (int i = 0; i < 16; i++) {
            REQUIRE(m.data[i] == Catch::Approx(5.0f));
        }
    }

    SECTION("Full constructor initializes correctly") {
        Matrix4f m(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16);

        REQUIRE(m.m00 == Catch::Approx(1));
        REQUIRE(m.m01 == Catch::Approx(2));
        REQUIRE(m.m02 == Catch::Approx(3));
        REQUIRE(m.m03 == Catch::Approx(4));
        REQUIRE(m.m10 == Catch::Approx(5));
        REQUIRE(m.m11 == Catch::Approx(6));
        REQUIRE(m.m12 == Catch::Approx(7));
        REQUIRE(m.m13 == Catch::Approx(8));
        REQUIRE(m.m20 == Catch::Approx(9));
        REQUIRE(m.m21 == Catch::Approx(10));
        REQUIRE(m.m22 == Catch::Approx(11));
        REQUIRE(m.m23 == Catch::Approx(12));
        REQUIRE(m.m30 == Catch::Approx(13));
        REQUIRE(m.m31 == Catch::Approx(14));
        REQUIRE(m.m32 == Catch::Approx(15));
        REQUIRE(m.m33 == Catch::Approx(16));
    }
}

TEST_CASE("matrix4f negation and equality", "[Matrix4]") {
    Matrix4f a(1.0f);
    Matrix4f b(1.0f);

    SECTION("Negation operator") {
        auto neg = -a;
        for (int i = 0; i < 16; i++) {
            REQUIRE(neg.data[i] == Catch::Approx(-1.0f));
        }
    }

    SECTION("Equality operator") {
        REQUIRE(a == b);
        Matrix4f c(2.0f);
        REQUIRE_FALSE(a == c);
    }
}

TEST_CASE("matrix4f arithmetic with scalars", "[Matrix4]") {
    Matrix4f m(2.0f);

    SECTION("Addition with scalar") {
        auto r = m + 3.0f;
        for (int i = 0; i < 16; i++) {
            REQUIRE(r.data[i] == Catch::Approx(5.0f));
        }
    }

    SECTION("Subtraction with scalar") {
        auto r = m - 2.0f;
        for (int i = 0; i < 16; i++) {
            REQUIRE(r.data[i] == Catch::Approx(0.0f));
        }
    }

    SECTION("Multiplication with scalar") {
        auto r = m * 4.0f;
        for (int i = 0; i < 16; i++) {
            REQUIRE(r.data[i] == Catch::Approx(8.0f));
        }
    }

    SECTION("Division with scalar") {
        auto r = m / 2.0f;
        for (int i = 0; i < 16; i++) {
            REQUIRE(r.data[i] == Catch::Approx(1.0f));
        }
    }
}

TEST_CASE("matrix4f arithmetic with other matrices", "[Matrix4]") {
    Matrix4f a(1.0f);
    Matrix4f b(2.0f);

    SECTION("Addition with matrix") {
        auto r = a + b;
        for (int i = 0; i < 16; i++) {
            REQUIRE(r.data[i] == Catch::Approx(3.0f));
        }
    }

    SECTION("Subtraction with matrix") {
        auto r = b - a;
        for (int i = 0; i < 16; i++) {
            REQUIRE(r.data[i] == Catch::Approx(1.0f));
        }
    }

    SECTION("Multiplication with matrix (identity test)") {
        Matrix4f I(1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1);

        auto r = I * b;
        REQUIRE(r.m00 == Catch::Approx(b.m00));
        REQUIRE(r.m11 == Catch::Approx(b.m11));
        REQUIRE(r.m22 == Catch::Approx(b.m22));
        REQUIRE(r.m33 == Catch::Approx(b.m33));
    }
}

TEST_CASE("matrix4f projections", "[Matrix4]") {
    /*
    SECTION("Perspective projection")
    {
        float fovY = 90.0f * 3.14159265f / 180.0f; // 90 degrees in radians
        float aspect = 16.0f / 9.0f;
        float z_near = 0.1f;
        float z_far = 100.0f;
        float f = 1.0f / std::tan(fovY / 2.0f);
        matrix4f m = matrix4f::perspective_vfov(fovY, aspect, z_near, z_far);

        REQUIRE(m.data[0] == Catch::Approx(f / aspect));
        REQUIRE(m.data[5] == Catch::Approx(f));
        REQUIRE(m.data[10] == Catch::Approx(z_far / (z_far - z_near)));
        REQUIRE(m.data[11] == Catch::Approx(1.0f));
        REQUIRE(m.data[14] == Catch::Approx(-z_near * z_far / (z_far - z_near)));
        REQUIRE(m.data[1] == Catch::Approx(0.0f));
        REQUIRE(m.data[2] == Catch::Approx(0.0f));
        REQUIRE(m.data[3] == Catch::Approx(0.0f));
        REQUIRE(m.data[4] == Catch::Approx(0.0f));
        REQUIRE(m.data[6] == Catch::Approx(0.0f));
        REQUIRE(m.data[7] == Catch::Approx(0.0f));
        REQUIRE(m.data[8] == Catch::Approx(0.0f));
        REQUIRE(m.data[9] == Catch::Approx(0.0f));
        REQUIRE(m.data[12] == Catch::Approx(0.0f));
        REQUIRE(m.data[13] == Catch::Approx(0.0f));
        REQUIRE(m.data[15] == Catch::Approx(0.0f)); // bottom-right is 0 for
    perspective division, 1 otherwise
    }

    SECTION("Orthographic projection")
    {
        float left = -10.0f;
        float right = 10.0f;
        float bottom = -5.0f;
        float top = 5.0f;
        float z_near = 0.1f;
        float z_far = 100.0f;

        matrix4f m = matrix4f::orthographic(left, right, bottom, top, z_near,
    z_far);

        REQUIRE(m.data[0] == Catch::Approx(2.0f / (right - left)));
        REQUIRE(m.data[5] == Catch::Approx(2.0f / (top - bottom)));
        REQUIRE(m.data[10] == Catch::Approx(1.0f / (z_far - z_near)));
        REQUIRE(m.data[15] == Catch::Approx(1.0f));
        REQUIRE(m.data[12] == Catch::Approx(-(right + left) / (right - left)));
        REQUIRE(m.data[13] == Catch::Approx(-(top + bottom) / (top - bottom)));
        REQUIRE(m.data[14] == Catch::Approx(-z_near / (z_far - z_near)));
        REQUIRE(m.data[1] == Catch::Approx(0.0f));
        REQUIRE(m.data[2] == Catch::Approx(0.0f));
        REQUIRE(m.data[3] == Catch::Approx(0.0f));
        REQUIRE(m.data[4] == Catch::Approx(0.0f));
        REQUIRE(m.data[6] == Catch::Approx(0.0f));
        REQUIRE(m.data[7] == Catch::Approx(0.0f));
        REQUIRE(m.data[8] == Catch::Approx(0.0f));
        REQUIRE(m.data[9] == Catch::Approx(0.0f));
        REQUIRE(m.data[11] == Catch::Approx(0.0f));
    }
    */
}
