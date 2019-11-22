#ifndef ANSI_STYLE_H
#define ANSI_STYLE_H

#include <ostream>  // output stream
#include <string>   // to_string()

namespace style {

enum COLOR {
    RESET,
    BOLD,
    FAINT,
    ITALIC,
    UNDERLINE,
    REVERSE = 7,
    PRIMARY = 10,
    NORMAL = 22,
    BLACK = 30,
    RED,
    GREEN,
    YELLOW,
    BLUE,
    MAGENTA,
    CYAN,
    WHITE,
    DEFAULT_FG = 39,
    DEFAULT_BG = 49,
    BRIGHT_BLACK = 90,
    BRIGHT_RED,
    BRIGHT_GREEN,
    BRIGHT_YELLOW,
    BRIGHT_BLUE,
    BRIGHT_MAGENTA,
    BRIGHT_CYAN,
    BRIGHT_WHITE
};

struct Ansi {
    int _code;

    Ansi(int c) : _code(c) {}

    friend std::ostream& operator<<(std::ostream& outs, const Ansi& l) {
        return outs << "\033[" << std::to_string(l._code) << "m";
    }
};

}  // namespace style

#endif  // ANSI_STYLE_H
