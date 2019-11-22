/*******************************************************************************
 * AUTHOR      : Thuan Tang
 * ID          : 00991588
 * CLASS       : CS008
 * HEADER      : tokenizer
 * NAMESPACE   : tok
 * DESCRIPTION : Tokenizer will tokenizes strings into various Tokens
 *      of various states defined by state_machine.h
 *
 *      Specialized for command line arguments.
 ******************************************************************************/
#ifndef TOKENIZER_H
#define TOKENIZER_H

#include <cassert>          // assert()
#include <iostream>         // stream
#include <string>           // string
#include "state_machine.h"  // state_machine functions
#include "token.h"          // Token class

class Tokenizer {
public:
    enum Size { MAX_BUF = 5120 };

    // CONSTRUCTORS
    Tokenizer(std::size_t max_buf = MAX_BUF);
    Tokenizer(char str[], std::size_t max_buf = MAX_BUF);
    Tokenizer(const char str[], std::size_t max_buf = MAX_BUF);

    ~Tokenizer();

    // ACCESSORS
    bool done() const;               // true: there are no more tokens
    bool more() const;               // true: there are more tokens
    explicit operator bool() const;  // boolean conversion for extractor

    // MUTATORS
    void set_string(char str[]);        // set a new string as the input string
    void set_string(const char str[]);  // set a new string as the input string

    // FRIENDS
    friend Tokenizer& operator>>(Tokenizer& s, Token& t);

private:
    static int _table[state_machine::MAX_ROWS][state_machine::MAX_COLS];
    static bool _need_init;  // check if _table is initialized

    char* _buffer;         // input string
    std::size_t _max_buf;  // max buffer size
    int _buffer_size;      // input string size
    int _pos;              // current position in the string

    void make_table(int _table[][state_machine::MAX_COLS]);
    bool get_token(int start_state, std::string& token);
};

#endif  // TOKENIZER_H
