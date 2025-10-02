#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

#include <engine/math/matrix3f.hpp>

using namespace core::math;

TEST_CASE("Matrix3f constructors", "[Matrix3]") {
    SECTION("Default constructor initializes to zero") {
        Matrix3f m;
        for (int i = 0; i < 9; i++) {
            REQUIRE(m.data[i] == Catch::Approx(0.0f));
        }
    }

    SECTION("Single-value constructor fills all entries") {
        Matrix3f m(5.0f);
        for (int i = 0; i < 9; i++) {
            REQUIRE(m.data[i] == Catch::Approx(5.0f));
        }
    }

    SECTION("Full constructor initializes correctly") {
        Matrix3f m(1, 2, 3, 4, 5, 6, 7, 8, 9);

        REQUIRE(m.m00 == Catch::Approx(1));
        REQUIRE(m.m01 == Catch::Approx(2));
        REQUIRE(m.m02 == Catch::Approx(3));
        REQUIRE(m.m10 == Catch::Approx(4));
        REQUIRE(m.m11 == Catch::Approx(5));
        REQUIRE(m.m12 == Catch::Approx(6));
        REQUIRE(m.m20 == Catch::Approx(7));
        REQUIRE(m.m21 == Catch::Approx(8));
        REQUIRE(m.m22 == Catch::Approx(9));
    }
}

TEST_CASE("Matrix3f negation and equality", "[Matrix3]") {
    Matrix3f a(1.0f);
    Matrix3f b(1.0f);

    SECTION("Negation operator") {
        auto neg = -a;
        for (int i = 0; i < 9; i++) {
            REQUIRE(neg.data[i] == Catch::Approx(-1.0f));
        }
    }

    SECTION("Equality operator") {
        REQUIRE(a == b);
        Matrix3f c(2.0f);
        REQUIRE_FALSE(a == c);
    }
}

TEST_CASE("Matrix3f arithmetic with scalars", "[Matrix3]") {
    Matrix3f m(2.0f);

    SECTION("Addition with scalar") {
        auto r = m + 3.0f;
        for (int i = 0; i < 9; i++) {
            REQUIRE(r.data[i] == Catch::Approx(5.0f));
        }
    }

    SECTION("Subtraction with scalar") {
        auto r = m - 2.0f;
        for (int i = 0; i < 9; i++) {
            REQUIRE(r.data[i] == Catch::Approx(0.0f));
        }
    }

    SECTION("Multiplication with scalar") {
        auto r = m * 4.0f;
        for (int i = 0; i < 9; i++) {
            REQUIRE(r.data[i] == Catch::Approx(8.0f));
        }
    }

    SECTION("Division with scalar") {
        auto r = m / 2.0f;
        for (int i = 0; i < 9; i++) {
            REQUIRE(r.data[i] == Catch::Approx(1.0f));
        }
    }
}

TEST_CASE("Matrix3f arithmetic with other matrices", "[Matrix3]") {
    Matrix3f a(1.0f);
    Matrix3f b(2.0f);

    SECTION("Addition with matrix") {
        auto r = a + b;
        for (int i = 0; i < 9; i++) {
            REQUIRE(r.data[i] == Catch::Approx(3.0f));
        }
    }

    SECTION("Subtraction with matrix") {
        auto r = b - a;
        for (int i = 0; i < 9; i++) {
            REQUIRE(r.data[i] == Catch::Approx(1.0f));
        }
    }

    SECTION("Multiplication with matrix (identity test)") {
        Matrix3f I(1, 0, 0, 1, 0, 0, 0, 0, 1);

        auto r = I * b;
        REQUIRE(r.m00 == Catch::Approx(b.m00));
        REQUIRE(r.m11 == Catch::Approx(b.m11));
        REQUIRE(r.m22 == Catch::Approx(b.m22));
    }
}
