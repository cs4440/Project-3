/*******************************************************************************
 * CLASS       : CS4440
 * HEADER      : parser
 * DESCRIPTION : Parses the grammar of the command line arguments, such as
 *      commands, quotes and operators (ex: '|'), which mimics the unix-like
 *      shell.
 ******************************************************************************/
#ifndef PARSER_H
#define PARSER_H

#include <cstring>                 // strlen()
#include <string>                  // string
#include <vector>                  // vector
#include "../include/token.h"      // Token class
#include "../include/tokenizer.h"  // Tokenizer class

class Parser {
public:
    enum States { START, CONCAT, PUSH, PUSH_BOTH, BSLASH, SIZE };
    enum Size { MAX_ROWS = SIZE, MAX_COLS = state_machine::STATE_SIZE };

    // CONSTRUCTORS
    Parser(char* buf = nullptr, std::size_t buf_size = Tokenizer::MAX_BUF);

    // ACCESSORS
    const std::vector<Token>& get_tokens() const;  // list of tokens

    // MUTATORS
    void clear();                       // reset all private states
    bool parse();                       // parse buffer into tokens
    void set_string(char str[]);        // set a new string as the input string
    void set_string(const char str[]);  // set a new string as the input string

private:
    // CLASS VARIABLES
    static bool _need_init;                 // need class initializations?
    static int _table[MAX_ROWS][MAX_COLS];  // adjacency table

    std::size_t _max_buf;        // max buffer size for tokenizer
    Tokenizer _tokenizer;        // tokenizes buffer
    std::vector<Token> _tokens;  // list of tokens

    bool _init();  // init class

    // Helper functions for adjacency table
    // fill all cells of the array with -1
    void init_table(int _table[][MAX_COLS]);

    // mark this state (row) with a 1 (success)
    void mark_success(int _table[][MAX_COLS], int state);

    // mark this state (row) with a 0 (fail)
    void mark_fail(int _table[][MAX_COLS], int state);

    // true if state is a success state
    bool is_success(const int _table[][MAX_COLS], int state);

    // mark a range of cells in the array.
    void mark_cells(int row, int _table[][MAX_COLS], int from, int to,
                    int state);

    // mark columns represented by the string columns[] for this row
    void mark_cells(int row, int _table[][MAX_COLS], const char columns[],
                    int state);

    // mark this row and column
    void mark_cell(int row, int _table[][MAX_COLS], int column, int state);

    // mark adjacency table rules for parser
    void mark_table(int _table[][MAX_COLS]);

    void print_table(const int _table[][MAX_COLS]);  // print table for debug
};

#endif  // PARSER_H
