#ifndef COLORS_H
#define COLORS_H

typedef enum {
    COLOR_RED   = 0,
    COLOR_GREEN = 1,
    COLOR_BLUE  = 2
} Color;

const char* color_name(Color c);
Color next_color(Color c);

#endif
