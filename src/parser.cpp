#include "../include/parser.h"

// STATIC VARIABLES
bool Parser::_need_init = true;          // static class init flag
int Parser::_table[MAX_ROWS][MAX_COLS];  // adjacency table

// constructor with class init check
Parser::Parser(char* buf, std::size_t buf_size)
    : _max_buf(buf_size), _tokenizer(buf, _max_buf) {
    assert(_max_buf > 0);
    if(buf) assert(_max_buf >= strlen(buf));

    if(_need_init) _need_init = _init();
}

// returns tokens list
const std::vector<std::string>& Parser::get_tokens() const { return _tokens; }

// clear all private states
void Parser::clear() {
    _tokenizer.set_string("");
    _tokens.clear();
}

// set new buffer string to tokenizer
void Parser::set_string(char str[]) { _tokenizer.set_string(str); }
void Parser::set_string(const char str[]) { _tokenizer.set_string(str); }

// parses buffer to internal tokens list
bool Parser::parse() {
    using namespace state_machine;

    bool get_newline = false;
    int state = START;
    std::string concat;
    Token token;

    while(_tokenizer >> token) {
        state = _table[state][token.type()];

        if(state == PUSH) {
            if(!concat.empty()) _tokens.emplace_back(concat);
            concat.clear();
        } else if(state == PUSH_BOTH) {
            if(!concat.empty()) _tokens.emplace_back(concat);
            _tokens.emplace_back(token.string());
            concat.clear();
        } else if(state == CONCAT) {
            concat += token;
            get_newline = false;
        } else if(state == BSLASH) {
            get_newline = true;
        }
    }
    if(!concat.empty()) _tokens.emplace_back(concat);

    return get_newline;
}

// init class
bool Parser::_init() {
    init_table(_table);
    mark_table(_table);

    return false;  // false to disable further init
}

// fill adjaency table with -1's
void Parser::init_table(int _table[][MAX_COLS]) {
    for(int row = 0; row < MAX_ROWS; ++row)
        for(int col = 0; col < MAX_COLS; ++col) _table[row][col] = -1;
}

// mark this state (row) with a 1 (success)
void Parser::mark_success(int _table[][MAX_COLS], int state) {
    assert(state < MAX_ROWS);
    _table[state][0] = 1;
}

// mark this state (row) with a 0 (fail)
void Parser::mark_fail(int _table[][MAX_COLS], int state) {
    assert(state < MAX_ROWS);
    _table[state][0] = 0;
}

// true if state is a success state
bool Parser::is_success(const int _table[][MAX_COLS], int state) {
    return _table[state][0];
}

// mark a range of cells in the array.
void Parser::mark_cells(int row, int _table[][MAX_COLS], int from, int to,
                        int state) {
    for(int col = from; col <= to; ++col) _table[row][col] = state;
}

// mark columns represented by the string columns[] for this row
void Parser::mark_cells(int row, int _table[][MAX_COLS], const char columns[],
                        int state) {
    for(int i = 0; columns[i] != '\0'; ++i)
        _table[row][(int)columns[i]] = state;
}

// mark this row and column
void Parser::mark_cell(int row, int _table[][MAX_COLS], int column, int state) {
    _table[row][column] = state;
}

// mark adjacency table rules for parser
void Parser::mark_table(int _table[][MAX_COLS]) {
    using namespace state_machine;

    // mark start states
    mark_cell(START, _table, STATE_ARG, CONCAT);
    mark_cell(START, _table, STATE_QUOTE, CONCAT);
    mark_cell(START, _table, STATE_OP, CONCAT);
    mark_cell(START, _table, STATE_BSLASH, BSLASH);
    mark_cell(START, _table, STATE_SPACE, START);

    mark_cell(CONCAT, _table, STATE_ARG, CONCAT);
    mark_cell(CONCAT, _table, STATE_QUOTE, CONCAT);
    mark_cell(CONCAT, _table, STATE_OP, PUSH_BOTH);
    mark_cell(CONCAT, _table, STATE_BSLASH, BSLASH);
    mark_cell(CONCAT, _table, STATE_SPACE, PUSH);

    mark_cell(PUSH, _table, STATE_ARG, CONCAT);
    mark_cell(PUSH, _table, STATE_QUOTE, CONCAT);
    mark_cell(PUSH, _table, STATE_OP, PUSH_BOTH);
    mark_cell(PUSH, _table, STATE_BSLASH, BSLASH);
    mark_cell(PUSH, _table, STATE_SPACE, START);

    mark_cell(PUSH_BOTH, _table, STATE_ARG, CONCAT);
    mark_cell(PUSH_BOTH, _table, STATE_QUOTE, CONCAT);
    mark_cell(PUSH_BOTH, _table, STATE_OP, PUSH_BOTH);
    mark_cell(PUSH_BOTH, _table, STATE_BSLASH, BSLASH);
    mark_cell(PUSH_BOTH, _table, STATE_SPACE, START);

    mark_cell(BSLASH, _table, STATE_ARG, CONCAT);
    mark_cell(BSLASH, _table, STATE_QUOTE, CONCAT);
    mark_cell(BSLASH, _table, STATE_OP, CONCAT);
    mark_cell(BSLASH, _table, STATE_BSLASH, CONCAT);
    mark_cell(BSLASH, _table, STATE_SPACE, CONCAT);
}
// print table for debug
void Parser::print_table(const int _table[][MAX_COLS]) {
    int cols_per_row = 11, count = 1, value_len;

    while(count < MAX_COLS) {
        // print header
        std::cout << " S ";
        std::cout << "|  " << 0 << "  ";
        for(int col = count; col < count + cols_per_row; ++col) {
            if(col < MAX_COLS && (col < 32 || col > 126)) {
                value_len = std::to_string(col).length();
                if(value_len == 1)
                    std::cout << "|  " << col << "  ";
                else if(value_len == 2)
                    std::cout << "|  " << col << " ";
                else if(value_len == 3)
                    std::cout << "| " << col << " ";

            } else if(col < MAX_COLS && col > 31 && col < 127)
                std::cout << "| '" << static_cast<char>(col) << "' ";
        }
        std::cout << "|" << std::endl;

        // print bar
        std::cout << "--- -----";
        for(int col = count; col < count + cols_per_row; ++col)
            if(col < MAX_COLS) std::cout << " -----";

        std::cout << std::endl;

        // print values in array
        for(int row = 0; row < MAX_ROWS; ++row) {
            // print row number
            std::cout << std::setw(2) << std::right << row << " ";

            // print column 0
            value_len = std::to_string(_table[row][0]).length();
            if(value_len == 1)
                std::cout << "|  " << _table[row][0] << "  ";
            else if(value_len == 2)
                std::cout << "|  " << _table[row][0] << " ";
            else if(value_len == 3)
                std::cout << "| " << _table[row][0] << " ";

            for(int col = count; col < count + cols_per_row; ++col) {
                if(col < MAX_COLS) {
                    value_len = std::to_string(_table[row][col]).length();
                    if(value_len == 1)
                        std::cout << "|  " << _table[row][col] << "  ";
                    else if(value_len == 2)
                        std::cout << "|  " << _table[row][col] << " ";
                    else if(value_len == 3)
                        std::cout << "| " << _table[row][col] << " ";
                }
            }
            std::cout << "|" << std::endl;
        }
        std::cout << std::endl;

        count += cols_per_row;  // udpate count
    }
}
