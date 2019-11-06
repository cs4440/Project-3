/*******************************************************************************
 * CLASS       : CS4440
 * HEADER      : compression
 * DESCRIPTION : Defines simple compression/decompression functions dealing
 *      0's and 1's.
 ******************************************************************************/
#ifndef COMPRESSION_H
#define COMPRESSION_H

#include <stdio.h>  // FILE*
#include <cstring>  // memcpy
#include <string>   // string

// compress ascii 0 and 1 text file
// @param src - input file stream
// @param dest - output file stream
void compress(FILE *src, FILE *dest);

// compress ascii 0 and 1 text file and output to array, with nul terminate
// @param src - source array
// @param dest - destination array
// return bytes written to array like a cstring strlen
std::size_t compress_arr(char *src, char *dest, std::size_t bytes);

// decompress a compressed file of 0/1
// @param src - input file stream
// @param dest - output file stream
void decompress(FILE *src, FILE *dest);

// write n number of bits and compress if over limit
// @param dest - output file stream to write
// @param c - character to write
// @param size - number of characers to write
// @param limit - integer limit to switch to compression symbol; otherwise
//                just write n number of bits to file
void write_compression(FILE *dest, char c, int count, int limit);

// write n number of bits and compress if over limit to array w/o nul terminate
// @param dest - destination array to write
// @param c - character to write
// @param size - number of characers to write
// @param limit - integer limit to switch to compression symbol; otherwise
//                just write n number of bits to file
// return bytes written to array like a cstring strlen
std::size_t write_compression_arr(char *dest, char c, int count, int limit);

void compress(FILE *src, FILE *dest) {
    int chr,         // character to get from src
        count = 0,   // count the number of same characters
        limit = 16,  // limit for compression
        peek;        // next character

    while((chr = fgetc(src)) != EOF) {
        ++count;            // count number of characters read
        peek = fgetc(src);  // peek at next char
        ungetc(peek, src);  // return peek character

        if(chr != peek) {  // if peek char is diff from current, write
            write_compression(dest, chr, count, limit);
            count = 0;
        }
    }
}

std::size_t compress_arr(char *src, char *dest, std::size_t bytes) {
    int count = 0,          // count the number of same characters
        limit = 16;         // limit for compression
    std::size_t index = 0;  // index of array

    for(std::size_t i = 0; i < bytes; ++i) {
        ++count;  // count number of characters read

        // if peek char is diff from current, write
        if(src[i] != src[i + 1] || i == bytes - 1) {
            index += write_compression_arr(dest + index, src[i], count, limit);
            count = 0;
        }
    }
    dest[index] = '\0';
    return index;
}

void write_compression(FILE *dest, char c, int count, int limit) {
    if(count < limit) {
        std::string str(count, c);
        fputs(str.c_str(), dest);
    } else {
        if(c == '0')
            c = '-';
        else if(c == '1')
            c = '+';

        std::string str = (char)c + std::to_string(count) + (char)c;
        fputs(str.c_str(), dest);
    }
}

std::size_t write_compression_arr(char *dest, char c, int count, int limit) {
    std::string str;

    if(count < limit) {
        str = std::string(count, c);
        memcpy(dest, str.c_str(), str.size());
    } else {
        if(c == '0')
            c = '-';
        else if(c == '1')
            c = '+';

        str = (char)c + std::to_string(count) + (char)c;
        memcpy(dest, str.c_str(), str.size());
    }
    return str.size();
}

void decompress(FILE *src, FILE *dest) {
    int chr = 0,  // character to get from src
        n = 0;    // sign count
    std::string sign;

    while((chr = fgetc(src)) != EOF) {
        if(chr == '-' || chr == '+') {
            sign.clear();
            while((chr = fgetc(src)) != '-' && chr != '+') sign += chr;
            n = atoi(sign.c_str());

            chr = chr == '-' ? '0' : '1';
            for(int i = 0; i < n; ++i) fputc(chr, dest);
        } else
            fputc(chr, dest);
    }
}

#endif  // COMPRESSION_H
