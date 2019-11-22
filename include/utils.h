#ifndef UTILS_H
#define UTILS_H

namespace utils {

inline void int_to_char(int n, char* str);
inline void char_to_int(char* str, int& n);

void int_to_char(int n, char* str) {
    str[0] = n >> 24 & 0xFF;
    str[1] = n >> 16 & 0xFF;
    str[2] = n >> 8 & 0xFF;
    str[3] = n & 0xFF;
}

void char_to_int(char* str, int& n) {
    n = ((unsigned char)str[0] << 24) | ((unsigned char)str[1] << 16) |
        ((unsigned char)str[2] << 8) | (unsigned char)str[3];
}

}  // namespace utils

#endif  // UTILS_H
