namespace MathTest;

use std::log::println;
use std::math::PI;
use std::math::E;
use std::math::TAU;
use std::math::INF;
use std::math::INT32_MAX;
use std::math::sqrt;
use std::math::pow;
use std::math::sin;
use std::math::cos;
use std::math::ln;
use std::math::ceil;
use std::math::floor;
use std::math::round;
use std::math::abs;
use std::math::min;
use std::math::max;
use std::math::clamp;
use std::math::toRadians;
use std::math::toDegrees;
use std::math::lerp;
use std::math::isNaN;
use std::math::isInf;
use std::math::isFinite;
use std::math::NAN;

int32 main() {
    // Constants
    println(PI);
    println(E);
    println(TAU);
    println(INF);
    println(INT32_MAX);

    // Basic math functions
    println(sqrt(144.0));
    println(pow(2.0, 10.0));
    println(ln(E));
    println(ceil(3.2));
    println(floor(3.8));
    println(round(3.5));

    // Trig
    float64 angle = toRadians(90.0);
    println(sin(angle));
    println(cos(toRadians(0.0)));
    println(toDegrees(PI));

    // Polymorphic: abs
    println(abs(-42));
    println(abs(-3.14));

    // Polymorphic: min/max
    println(min(10, 20));
    println(max(10, 20));

    // Polymorphic: clamp
    println(clamp(150, 0, 100));
    println(clamp(-5, 0, 100));
    println(clamp(50, 0, 100));

    // Interpolation
    println(lerp(0.0, 100.0, 0.5));

    // Checks
    println(isNaN(NAN));
    println(isInf(INF));
    println(isFinite(42.0));
    println(isFinite(INF));

    ret 0;
}
