namespace ExtendStd;

use std::log::println;

// ── Enums ────────────────────────────────────────────────────────────────────

enum Color {
    Red,
    Green,
    Blue,
}

enum Direction {
    North,
    South,
    East,
    West,
}

// ── Structs ──────────────────────────────────────────────────────────────────

struct Vec2 {
    int32 x;
    int32 y;
}

struct Rect {
    Vec2 origin;
    Vec2 size;
}

struct Particle {
    Vec2 pos;
    int32 speed;
    Color color;
    bool active;
}

// ── Extend: Vec2 ─────────────────────────────────────────────────────────────

extend Vec2 {
    // Static: create from values
    Vec2 create(int32 x, int32 y) {
        Vec2 v = Vec2 { x: x, y: y };
        ret v;
    }

    // Static: return a zero vector
    Vec2 zero() {
        Vec2 v = Vec2 { x: 0, y: 0 };
        ret v;
    }

    // Instance: return a number
    int32 manhattanLength(&self) {
        int32 ax = self->x;
        int32 ay = self->y;
        if (ax < 0) { ax = 0 - ax; }
        if (ay < 0) { ay = 0 - ay; }
        ret ax + ay;
    }

    // Instance: return a bool
    bool isZero(&self) {
        ret self->x == 0 && self->y == 0;
    }

    // Instance: return a new struct (Vec2)
    Vec2 add(&self, Vec2 other) {
        Vec2 result = Vec2 { x: self->x + other.x, y: self->y + other.y };
        ret result;
    }

    // Instance: mutate self
    void translate(&self, int32 dx, int32 dy) {
        self->x += dx;
        self->y += dy;
    }
}

// ── Extend: Rect ─────────────────────────────────────────────────────────────

extend Rect {
    // Static: factory
    Rect create(int32 x, int32 y, int32 w, int32 h) {
        Vec2 o = Vec2 { x: x, y: y };
        Vec2 s = Vec2 { x: w, y: h };
        Rect r = Rect { origin: o, size: s };
        ret r;
    }

    // Instance: return computed int32
    int32 area(&self) {
        Vec2 s = self->size;
        ret s.x * s.y;
    }

    // Instance: return bool
    bool isSquare(&self) {
        Vec2 s = self->size;
        ret s.x == s.y;
    }
}

// ── Extend: Particle ────────────────────────────────────────────────────────

extend Particle {
    // Static: factory with defaults
    Particle create(int32 x, int32 y) {
        Vec2 p = Vec2 { x: x, y: y };
        Particle part = Particle { pos: p, speed: 1, color: Color::Red, active: true };
        ret part;
    }

    // Instance: return enum
    Color getColor(&self) {
        ret self->color;
    }

    // Instance: return bool
    bool isActive(&self) {
        ret self->active;
    }

    // Instance: return int32
    int32 getSpeed(&self) {
        ret self->speed;
    }

    // Instance: return another struct
    Vec2 getPosition(&self) {
        ret self->pos;
    }

    // Instance: mutate
    void deactivate(&self) {
        self->active = false;
    }
}

// ── Main ─────────────────────────────────────────────────────────────────────

int32 main() {
    // --- Vec2 tests ---
    Vec2 a = Vec2::create(3, 4);
    println(a.x);               // 3
    println(a.y);               // 4

    Vec2 z = Vec2::zero();
    println(z.x);               // 0
    println(z.y);               // 0

    println(a.manhattanLength());   // 7
    println(z.isZero());            // true
    println(a.isZero());            // false

    Vec2 b = Vec2::create(10, 20);
    Vec2 c = a.add(b);
    println(c.x);               // 13
    println(c.y);               // 24

    a.translate(100, 200);
    println(a.x);               // 103
    println(a.y);               // 204

    // --- Rect tests ---
    Rect r = Rect::create(5, 10, 30, 30);
    println(r.area());          // 900
    println(r.isSquare());      // true

    Rect r2 = Rect::create(0, 0, 10, 5);
    println(r2.area());         // 50
    println(r2.isSquare());     // false

    // --- Particle tests ---
    Particle p = Particle::create(7, 8);
    println(p.getSpeed());      // 1
    println(p.getColor());      // 0 (Red)
    println(p.isActive());      // true

    Vec2 pos = p.getPosition();
    println(pos.x);             // 7
    println(pos.y);             // 8

    p.deactivate();
    println(p.isActive());      // false

    ret 0;
}
