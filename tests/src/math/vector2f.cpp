#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

#include <engine/math/vector2f.hpp>

using namespace core::math;

TEST_CASE("Vector2f constructors", "[Vector2]") {
    SECTION("Default constructor initializes to zero") {
        Vector2f v;
        REQUIRE(v.x == Catch::Approx(0.0f));
        REQUIRE(v.y == Catch::Approx(0.0f));
    }

    SECTION("Single-value constructor fills both components") {
        Vector2f v(5.0f);
        REQUIRE(v.x == Catch::Approx(5.0f));
        REQUIRE(v.y == Catch::Approx(5.0f));
    }

    SECTION("Full constructor initializes correctly") {
        Vector2f v(1.0f, 2.0f);
        REQUIRE(v.x == Catch::Approx(1.0f));
        REQUIRE(v.y == Catch::Approx(2.0f));
    }
}

TEST_CASE("Vector2f comparisons", "[Vector2]") {
    Vector2f a(1.0f, 2.0f);
    Vector2f b(1.0f, 2.0f);
    Vector2f c(2.0f, 3.0f);

    SECTION("Equality operator") {
        REQUIRE(a == b);
        REQUIRE_FALSE(a == c);
    }
}

TEST_CASE("Vector2f unary operators", "[Vector2]") {
    Vector2f a(1.0f, -2.0f);

    SECTION("Negation") {
        auto neg = -a;
        REQUIRE(neg.x == Catch::Approx(-1.0f));
        REQUIRE(neg.y == Catch::Approx(2.0f));
    }
}

TEST_CASE("Vector2f arithmetic with other vectors", "[Vector2]") {
    Vector2f a(1.0f, 2.0f);
    Vector2f b(3.0f, 4.0f);

    SECTION("Addition") {
        auto r = a + b;
        REQUIRE(r.x == Catch::Approx(4.0f));
        REQUIRE(r.y == Catch::Approx(6.0f));
    }

    SECTION("Subtraction") {
        auto r = b - a;
        REQUIRE(r.x == Catch::Approx(2.0f));
        REQUIRE(r.y == Catch::Approx(2.0f));
    }

    SECTION("Compound assignment += and -=") {
        Vector2f v = a;
        v += b;
        REQUIRE(v.x == Catch::Approx(4.0f));
        REQUIRE(v.y == Catch::Approx(6.0f));

        v -= b;
        REQUIRE(v.x == Catch::Approx(1.0f));
        REQUIRE(v.y == Catch::Approx(2.0f));
    }
}

TEST_CASE("Vector2f scalar arithmetic", "[Vector2]") {
    Vector2f v(2.0f, 3.0f);

    SECTION("Multiplication") {
        auto r = v * 2.0f;
        REQUIRE(r.x == Catch::Approx(4.0f));
        REQUIRE(r.y == Catch::Approx(6.0f));
    }

    SECTION("Division") {
        auto r = v / 2.0f;
        REQUIRE(r.x == Catch::Approx(1.0f));
        REQUIRE(r.y == Catch::Approx(1.5f));
    }

    SECTION("Compound assignment *= and /=") {
        Vector2f a = v;
        a *= 2.0f;
        REQUIRE(a.x == Catch::Approx(4.0f));
        REQUIRE(a.y == Catch::Approx(6.0f));

        a /= 2.0f;
        REQUIRE(a.x == Catch::Approx(2.0f));
        REQUIRE(a.y == Catch::Approx(3.0f));
    }
}

TEST_CASE("Vector2f length and normalization", "[Vector2]") {
    Vector2f v(3.0f, 4.0f);

    SECTION("Length and squared length") {
        REQUIRE(v.sq_length() == Catch::Approx(25.0f));
        REQUIRE(v.length() == Catch::Approx(5.0f));
    }

    SECTION("Normalization") {
        auto norm = v.normalized();
        REQUIRE(norm.length() == Catch::Approx(1.0f));

        Vector2f copy = v;
        copy.normalize();
        REQUIRE(copy.length() == Catch::Approx(1.0f));
    }
}

TEST_CASE("Vector2f dot product", "[Vector2]") {
    Vector2f a(1.0f, 2.0f);
    Vector2f b(3.0f, 4.0f);

    SECTION("Dot product static function") {
        float dot = Vector2f::dot(a, b);
        REQUIRE(dot == Catch::Approx(11.0f)); // 1*3 + 2*4
    }
}
