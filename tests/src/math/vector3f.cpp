#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

#include <engine/math/vector3f.hpp>

using namespace core::math;

TEST_CASE("Vector3f constructors", "[Vector3]") {
    SECTION("Default constructor initializes to zero") {
        Vector3f v;
        REQUIRE(v.x == Catch::Approx(0.0f));
        REQUIRE(v.y == Catch::Approx(0.0f));
        REQUIRE(v.z == Catch::Approx(0.0f));
    }

    SECTION("Single-value constructor fills all components") {
        Vector3f v(5.0f);
        REQUIRE(v.x == Catch::Approx(5.0f));
        REQUIRE(v.y == Catch::Approx(5.0f));
        REQUIRE(v.z == Catch::Approx(5.0f));
    }

    SECTION("Full constructor initializes correctly") {
        Vector3f v(1.0f, 2.0f, 3.0f);
        REQUIRE(v.x == Catch::Approx(1.0f));
        REQUIRE(v.y == Catch::Approx(2.0f));
        REQUIRE(v.z == Catch::Approx(3.0f));
    }
}

TEST_CASE("Vector3f comparisons", "[Vector3]") {
    Vector3f a(1.0f, 2.0f, 3.0f);
    Vector3f b(1.0f, 2.0f, 3.0f);
    Vector3f c(4.0f, 5.0f, 6.0f);

    SECTION("Equality operator") {
        REQUIRE(a == b);
        REQUIRE_FALSE(a == c);
    }
}

TEST_CASE("Vector3f unary operators", "[Vector3]") {
    Vector3f a(1.0f, -2.0f, 3.0f);

    SECTION("Negation") {
        auto neg = -a;
        REQUIRE(neg.x == Catch::Approx(-1.0f));
        REQUIRE(neg.y == Catch::Approx(2.0f));
        REQUIRE(neg.z == Catch::Approx(-3.0f));
    }
}

TEST_CASE("Vector3f arithmetic with other vectors", "[Vector3]") {
    Vector3f a(1.0f, 2.0f, 3.0f);
    Vector3f b(4.0f, 5.0f, 6.0f);

    SECTION("Addition") {
        auto r = a + b;
        REQUIRE(r.x == Catch::Approx(5.0f));
        REQUIRE(r.y == Catch::Approx(7.0f));
        REQUIRE(r.z == Catch::Approx(9.0f));
    }

    SECTION("Subtraction") {
        auto r = b - a;
        REQUIRE(r.x == Catch::Approx(3.0f));
        REQUIRE(r.y == Catch::Approx(3.0f));
        REQUIRE(r.z == Catch::Approx(3.0f));
    }

    SECTION("Compound assignment += and -=") {
        Vector3f v = a;
        v += b;
        REQUIRE(v.x == Catch::Approx(5.0f));
        REQUIRE(v.y == Catch::Approx(7.0f));
        REQUIRE(v.z == Catch::Approx(9.0f));

        v -= b;
        REQUIRE(v.x == Catch::Approx(1.0f));
        REQUIRE(v.y == Catch::Approx(2.0f));
        REQUIRE(v.z == Catch::Approx(3.0f));
    }
}

TEST_CASE("Vector3f scalar arithmetic", "[Vector3]") {
    Vector3f v(2.0f, 3.0f, 4.0f);

    SECTION("Multiplication") {
        auto r = v * 2.0f;
        REQUIRE(r.x == Catch::Approx(4.0f));
        REQUIRE(r.y == Catch::Approx(6.0f));
        REQUIRE(r.z == Catch::Approx(8.0f));
    }

    SECTION("Division") {
        auto r = v / 2.0f;
        REQUIRE(r.x == Catch::Approx(1.0f));
        REQUIRE(r.y == Catch::Approx(1.5f));
        REQUIRE(r.z == Catch::Approx(2.0f));
    }

    SECTION("Compound assignment *= and /=") {
        Vector3f a = v;
        a *= 2.0f;
        REQUIRE(a.x == Catch::Approx(4.0f));
        REQUIRE(a.y == Catch::Approx(6.0f));
        REQUIRE(a.z == Catch::Approx(8.0f));

        a /= 2.0f;
        REQUIRE(a.x == Catch::Approx(2.0f));
        REQUIRE(a.y == Catch::Approx(3.0f));
        REQUIRE(a.z == Catch::Approx(4.0f));
    }
}

TEST_CASE("Vector3f length and normalization", "[Vector3]") {
    Vector3f v(1.0f, 2.0f, 2.0f);

    SECTION("Length and squared length") {
        REQUIRE(v.sq_length() == Catch::Approx(9.0f));
        REQUIRE(v.length() == Catch::Approx(3.0f));
    }

    SECTION("Normalization") {
        auto norm = v.normalized();
        REQUIRE(norm.length() == Catch::Approx(1.0f));

        Vector3f copy = v;
        copy.normalize();
        REQUIRE(copy.length() == Catch::Approx(1.0f));
    }
}

TEST_CASE("Vector3f dot and cross products", "[Vector3]") {
    Vector3f a(1.0f, 2.0f, 3.0f);
    Vector3f b(4.0f, 5.0f, 6.0f);

    SECTION("Dot product") {
        float dot = Vector3f::dot(a, b);
        REQUIRE(dot == Catch::Approx(32.0f)); // 1*4 + 2*5 + 3*6
    }

    SECTION("Cross product") {
        auto cross = Vector3f::cross(a, b);
        REQUIRE(cross.x == Catch::Approx(-3.0f));
        REQUIRE(cross.y == Catch::Approx(6.0f));
        REQUIRE(cross.z == Catch::Approx(-3.0f));
    }
}
