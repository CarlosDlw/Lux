#ifndef STRUCTS_H
#define STRUCTS_H

/* Small struct: 8 bytes — fits in one register (rax) */
typedef struct {
    int x;
    int y;
} Point;

/* Medium struct: 16 bytes — fits in two registers (rax + rdx) */
typedef struct {
    long a;
    long b;
} Pair64;

/* Large struct: 32 bytes — must use sret (hidden pointer) */
typedef struct {
    long w;
    long x;
    long y;
    long z;
} Vec4;

/* Nested struct: contains another struct */
typedef struct {
    Point origin;
    Point size;
} Rect;

/* Functions that RETURN structs by value */
Point make_point(int x, int y);
Pair64 make_pair64(long a, long b);
Vec4 make_vec4(long w, long x, long y, long z);
Rect make_rect(int ox, int oy, int sx, int sy);

/* Functions that TAKE structs by value */
int point_sum(Point p);
long pair64_sum(Pair64 p);
long vec4_sum(Vec4 v);
int rect_area(Rect r);

#endif
