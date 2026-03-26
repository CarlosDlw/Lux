#include "structs.h"

/* Return structs by value */
Point make_point(int x, int y) {
    Point p;
    p.x = x;
    p.y = y;
    return p;
}

Pair64 make_pair64(long a, long b) {
    Pair64 p;
    p.a = a;
    p.b = b;
    return p;
}

Vec4 make_vec4(long w, long x, long y, long z) {
    Vec4 v;
    v.w = w;
    v.x = x;
    v.y = y;
    v.z = z;
    return v;
}

Rect make_rect(int ox, int oy, int sx, int sy) {
    Rect r;
    r.origin.x = ox;
    r.origin.y = oy;
    r.size.x = sx;
    r.size.y = sy;
    return r;
}

/* Take structs by value */
int point_sum(Point p) {
    return p.x + p.y;
}

long pair64_sum(Pair64 p) {
    return p.a + p.b;
}

long vec4_sum(Vec4 v) {
    return v.w + v.x + v.y + v.z;
}

int rect_area(Rect r) {
    return r.size.x * r.size.y;
}
