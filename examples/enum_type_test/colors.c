#include "colors.h"

const char* color_name(Color c) {
    switch (c) {
        case COLOR_RED:   return "red";
        case COLOR_GREEN: return "green";
        case COLOR_BLUE:  return "blue";
        default:          return "unknown";
    }
}

Color next_color(Color c) {
    return (Color)((c + 1) % 3);
}
