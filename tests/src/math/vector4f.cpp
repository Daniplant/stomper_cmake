#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

#include <engine/math/vector4f.hpp>

using namespace core::math;

TEST_CASE("Vector4f constructors", "[Vector4]") {
    SECTION("Default constructor initializes to zero") {
        Vector4f v;
        REQUIRE(v.x == Catch::Approx(0.0f));
        REQUIRE(v.y == Catch::Approx(0.0f));
        REQUIRE(v.z == Catch::Approx(0.0f));
        REQUIRE(v.w == Catch::Approx(0.0f));
    }

    SECTION("Single-value constructor fills all components") {
        Vector4f v(5.0f);
        REQUIRE(v.x == Catch::Approx(5.0f));
        REQUIRE(v.y == Catch::Approx(5.0f));
        REQUIRE(v.z == Catch::Approx(5.0f));
        REQUIRE(v.w == Catch::Approx(5.0f));
    }

    SECTION("Full constructor initializes correctly") {
        Vector4f v(1.0f, 2.0f, 3.0f, 4.0f);
        REQUIRE(v.x == Catch::Approx(1.0f));
        REQUIRE(v.y == Catch::Approx(2.0f));
        REQUIRE(v.z == Catch::Approx(3.0f));
        REQUIRE(v.w == Catch::Approx(4.0f));
    }
}

TEST_CASE("Vector4f comparisons", "[Vector4]") {
    Vector4f a(1.0f, 2.0f, 3.0f, 4.0f);
    Vector4f b(1.0f, 2.0f, 3.0f, 4.0f);
    Vector4f c(5.0f, 6.0f, 7.0f, 8.0f);

    SECTION("Equality operator") {
        REQUIRE(a == b);
        REQUIRE_FALSE(a == c);
    }
}

TEST_CASE("Vector4f unary operators", "[Vector4]") {
    Vector4f a(1.0f, -2.0f, 3.0f, -4.0f);

    SECTION("Negation") {
        auto neg = -a;
        REQUIRE(neg.x == Catch::Approx(-1.0f));
        REQUIRE(neg.y == Catch::Approx(2.0f));
        REQUIRE(neg.z == Catch::Approx(-3.0f));
        REQUIRE(neg.w == Catch::Approx(4.0f));
    }
}

TEST_CASE("Vector4f arithmetic with other vectors", "[Vector4]") {
    Vector4f a(1.0f, 2.0f, 3.0f, 4.0f);
    Vector4f b(5.0f, 6.0f, 7.0f, 8.0f);

    SECTION("Addition") {
        auto r = a + b;
        REQUIRE(r.x == Catch::Approx(6.0f));
        REQUIRE(r.y == Catch::Approx(8.0f));
        REQUIRE(r.z == Catch::Approx(10.0f));
        REQUIRE(r.w == Catch::Approx(12.0f));
    }

    SECTION("Subtraction") {
        auto r = b - a;
        REQUIRE(r.x == Catch::Approx(4.0f));
        REQUIRE(r.y == Catch::Approx(4.0f));
        REQUIRE(r.z == Catch::Approx(4.0f));
        REQUIRE(r.w == Catch::Approx(4.0f));
    }

    SECTION("Compound assignment += and -=") {
        Vector4f v = a;
        v += b;
        REQUIRE(v.x == Catch::Approx(6.0f));
        REQUIRE(v.y == Catch::Approx(8.0f));
        REQUIRE(v.z == Catch::Approx(10.0f));
        REQUIRE(v.w == Catch::Approx(12.0f));

        v -= b;
        REQUIRE(v.x == Catch::Approx(1.0f));
        REQUIRE(v.y == Catch::Approx(2.0f));
        REQUIRE(v.z == Catch::Approx(3.0f));
        REQUIRE(v.w == Catch::Approx(4.0f));
    }
}

TEST_CASE("Vector4f scalar arithmetic", "[Vector4]") {
    Vector4f v(2.0f, 3.0f, 4.0f, 5.0f);

    SECTION("Multiplication") {
        auto r = v * 2.0f;
        REQUIRE(r.x == Catch::Approx(4.0f));
        REQUIRE(r.y == Catch::Approx(6.0f));
        REQUIRE(r.z == Catch::Approx(8.0f));
        REQUIRE(r.w == Catch::Approx(10.0f));
    }

    SECTION("Division") {
        auto r = v / 2.0f;
        REQUIRE(r.x == Catch::Approx(1.0f));
        REQUIRE(r.y == Catch::Approx(1.5f));
        REQUIRE(r.z == Catch::Approx(2.0f));
        REQUIRE(r.w == Catch::Approx(2.5f));
    }

    SECTION("Compound assignment *= and /=") {
        Vector4f a = v;
        a *= 2.0f;
        REQUIRE(a.x == Catch::Approx(4.0f));
        REQUIRE(a.y == Catch::Approx(6.0f));
        REQUIRE(a.z == Catch::Approx(8.0f));
        REQUIRE(a.w == Catch::Approx(10.0f));

        a /= 2.0f;
        REQUIRE(a.x == Catch::Approx(2.0f));
        REQUIRE(a.y == Catch::Approx(3.0f));
        REQUIRE(a.z == Catch::Approx(4.0f));
        REQUIRE(a.w == Catch::Approx(5.0f));
    }
}

TEST_CASE("Vector4f length and normalization", "[Vector4]") {
    Vector4f v(1.0f, 2.0f, 3.0f, 4.0f);

    SECTION("Length and squared length") {
        REQUIRE(v.sq_length() == Catch::Approx(30.0f)); // 1^2 + 2^2 + 3^2 + 4^2
        REQUIRE(v.length() == Catch::Approx(std::sqrt(30.0f)));
    }

    SECTION("Normalization") {
        auto norm = v.normalized();
        REQUIRE(norm.length() == Catch::Approx(1.0f));

        Vector4f copy = v;
        copy.normalize();
        REQUIRE(copy.length() == Catch::Approx(1.0f));
    }
}
