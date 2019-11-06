#include "../include/tokenizer.h"  // sql_tokenizer declarations

// STATIC VARIABLES
int Tokenizer::_table[state_machine::MAX_ROWS][state_machine::MAX_COLS];
bool Tokenizer::_need_init = true;

/*******************************************************************************
 * DESCRIPTION:
 *  Default constructor initializes _buffer and _pos  to 0 and calls make_table
 *  to initialize all values in table to -1.
 *
 * PRE-CONDITIONS:
 *  none
 *
 * POST-CONDITIONS:
 *  initialized member variables
 *
 * RETURN:
 *  none
 ******************************************************************************/
Tokenizer::Tokenizer(std::size_t max_buf)
    : _buffer(nullptr), _max_buf(max_buf), _buffer_size(0), _pos(0) {
    _buffer = new char[_max_buf];

    _buffer[0] = '\0';
    if(_need_init) make_table(_table);
}

/*******************************************************************************
 * DESCRIPTION:
 *  Default constructor initializes calls set_string to copy cstring into
 *  _buffer, set _buffer_size to cstring len, reset _pos to 0 and calls
 *  make_table to initialize all values in table to -1.
 *
 * PRE-CONDITIONS:
 *  char str[]: cstring for input buffer
 *
 * POST-CONDITIONS:
 *  initialized member variables via set_string() and make_table()
 *
 * RETURN:
 *  none
 ******************************************************************************/
Tokenizer::Tokenizer(char str[], std::size_t max_buf)
    : _buffer(nullptr), _max_buf(max_buf), _buffer_size(0), _pos(0) {
    _buffer = new char[_max_buf];

    set_string(str);
    if(_need_init) make_table(_table);
}

/*******************************************************************************
 * DESCRIPTION:
 *  Default constructor initializes calls set_string to copy const cstring into
 *  _buffer, set _buffer_size to cstring len, reset _pos to 0 and calls
 *  make_table to initialize all values in table to -1.
 *
 * PRE-CONDITIONS:
 *  const char str[]: cstring for input buffer
 *
 * POST-CONDITIONS:
 *  initialized member variables via set_string() and make_table()
 *
 * RETURN:
 *  none
 ******************************************************************************/
Tokenizer::Tokenizer(const char str[], std::size_t max_buf)
    : _buffer(nullptr), _max_buf(max_buf), _buffer_size(0), _pos(0) {
    _buffer = new char[_max_buf];

    set_string(str);
    if(_need_init) make_table(_table);
}

/*******************************************************************************
 * DESCRIPTION:
 *  Destructor. Deallocate _buffer.
 *
 * PRE-CONDITIONS:
 *  none
 *
 * POST-CONDITIONS:
 *  char* _buffer: deallocated
 *
 * RETURN:
 *  none
 ******************************************************************************/
Tokenizer::~Tokenizer() { delete[] _buffer; }

/*******************************************************************************
 * DESCRIPTION:
 *  Checks if there no tokens left in buffer cstring from current _pos.
 *  Condition: a) if cstring's len is 0, then true.
 *             b) if cstring's len > 0, evaluate prev pos is NUL terminate
 *                to ensure done() only returns true once past NUL position.
 *
 * PRE-CONDITIONS:
 *  char _buffer[_max_buf]: cstring
 *
 * POST-CONDITIONS:
 *  none
 *
 * RETURN:
 *  boolean
 ******************************************************************************/
bool Tokenizer::done() const {
    return _buffer_size > 0 ? _pos > _buffer_size : true;
}

/*******************************************************************************
 * DESCRIPTION:
 *  Checks if there more tokens left in buffer cstring from current _pos.
 *  Condition: a) if cstring's len is 0, then false.
 *             b) if cstring's len > 0, evaluate prev pos is not NUL terminate
 *                to ensure more() only returns false once past NUL position.
 *
 * PRE-CONDITIONS:
 *  none
 *
 * POST-CONDITIONS:
 *  none
 *
 * RETURN:
 *  boolean
 ******************************************************************************/
bool Tokenizer::more() const {
    return _buffer_size > 0 ? _pos <= _buffer_size : false;
}

/*******************************************************************************
 * DESCRIPTION:
 *  Explicit operator bool() overloading, such as in the case of using
 *  extraction operators. Ex: while(sql_tokenizer >> token) statements.
 *
 * PRE-CONDITIONS:
 *  none
 *
 * POST-CONDITIONS:
 *  none
 *
 * RETURN:
 *  boolean
 ******************************************************************************/
Tokenizer::operator bool() const { return more(); }

/*******************************************************************************
 * DESCRIPTION:
 *  Assign cstring param to buffer and reset _pos to 0.
 *
 * PRE-CONDITIONS:
 *  char str[]: cstring
 *
 * POST-CONDITIONS:
 *  char _buffer[]: assigned to str[]
 *  _buffer_size  : assigned to cstring size
 *  int _pos      : assigned to 0
 *
 * RETURN:
 *  none
 ******************************************************************************/
void Tokenizer::set_string(char str[]) {
    if(str) {
        std::size_t index = 0;
        while(str[index] != '\0') {  // copy string into buffer
            assert(index < _max_buf - 1);
            _buffer[index] = str[index];
            ++index;
        }

        _buffer[index] = '\0';  // NUL terminate cstring
        _buffer_size = index;   // set buffer size
        _pos = 0;               // reset cstring pos
    }
}

/*******************************************************************************
 * DESCRIPTION:
 *  Assign const cstring param to buffer and reset _pos to 0.
 *
 * PRE-CONDITIONS:
 *  const char str[]: cstring
 *
 * POST-CONDITIONS:
 *  char _buffer[]: assigned to str[]
 *  _buffer_size  : assigned to cstring size
 *  int _pos      : assigned to 0
 *
 * RETURN:
 *  none
 ******************************************************************************/
void Tokenizer::set_string(const char str[]) {
    if(str) {
        std::size_t index = 0;
        while(str[index] != '\0') {  // copy string into buffer
            assert(index < _max_buf - 1);
            _buffer[index] = str[index];
            ++index;
        }

        _buffer[index] = '\0';  // NUL terminate cstring
        _buffer_size = index;   // set buffer size
        _pos = 0;               // reset cstring pos
    }
}

/*******************************************************************************
 * DESCRIPTION:
 *  Initialize all values in table to -1.
 *
 * PRE-CONDITIONS:
 *  int _table[][state_machine::MAX_COLS]: integer array
 *
 * POST-CONDITIONS:
 *  all cells in table initialized to -1
 *
 * RETURN:
 *  none
 ******************************************************************************/
void Tokenizer::make_table(int _table[][state_machine::MAX_COLS]) {
    using namespace state_machine;

    init_table(_table);  // initialize table with -1

    // mark table with adjacency values
    // mark quote states
    mark_table_enclosed_delim(_table, STATE_QUOTE_S, '\'');
    mark_table_enclosed_delim(_table, STATE_QUOTE_D, '\"');

    // mark argument states
    // includde alpha, digit, and punct except for operators/quotes
    mark_table_generic(_table, STATE_ARG, ALPHA);
    mark_table_generic(_table, STATE_ARG, DIGIT);
    mark_table_generic(_table, STATE_ARG, PUNCT);
    unmark_table_generic(_table, STATE_ARG, OPS);     // rm ops from arg
    unmark_table_generic(_table, STATE_ARG, QUOTES);  // rm quotes from arg
    unmark_table_generic(_table, STATE_ARG, BSLASH);  // rm back slash from arg

    // mark single charater operator states
    mark_table_l_ops(_table, STATE_OP_L);

    // mark single charater bslash states
    mark_table_single_char(_table, STATE_BSLASH, BSLASH);

    // mark any white spaces
    mark_table_generic(_table, STATE_SPACE, SPACE);

    _need_init = false;  // disable further make_table() calls in CTOR
}

/*******************************************************************************
 * DESCRIPTION:
 *  Calls get_token version from state_machine with params and return success
 *  state of get_token.
 *
 * PRE-CONDITIONS:
 *  int start_state: 0 to MAX_ROWS - 1
 *  string& token  : token string via reference
 *
 * POST-CONDITIONS:
 *  int _pos    : old pos if fail, else new pos after token changed, refernce
 *  string token: string unchanged if fail, else change to valid token
 *
 * RETURN:
 *  boolean when state_machine's get_token success/fail
 ******************************************************************************/
bool Tokenizer::get_token(int start_state, std::string& token) {
    return state_machine::get_token(_table, _buffer, _pos, start_state, token);
}

/*******************************************************************************
 * DESCRIPTION:
 *  Extraction operator calls get_token() on various states until a valid token
 *  is found. If none is found, returns an empty Token with ID STATE_ERROR.
 *
 * PRE-CONDITIONS:
 *  Tokenizer& s: tokenizer
 *  Token& t: placeholder by ref
 *
 * POST-CONDITIONS:
 *  Tokenizer& s: advances tokenizer's internal buffer position
 *  Token& t: set to valid token/invalid empty token
 *
 * RETURN:
 *  Tokenizer& s
 ******************************************************************************/
Tokenizer& operator>>(Tokenizer& s, Token& t) {
    using namespace state_machine;

    std::string token;

    // process tokens one state at a time
    if(s._pos > s._buffer_size)  // bound check if call w/o more() or done()
        t = Token();
    else if(s.get_token(STATE_QUOTE_S, token)) {
        token.pop_back();  // remove quote delimeters
        token.erase(0, 1);
        t = Token(token, STATE_QUOTE, STATE_QUOTE_S);
    } else if(s.get_token(STATE_QUOTE_D, token)) {
        token.pop_back();  // remove quote delimeters
        token.erase(0, 1);
        t = Token(token, STATE_QUOTE, STATE_QUOTE_D);
    } else if(s.get_token(STATE_OP_L, token))
        t = Token(token, STATE_OP, STATE_OP_L);
    else if(s.get_token(STATE_OP_R, token))
        t = Token(token, STATE_OP, STATE_OP_R);
    else if(s.get_token(STATE_BSLASH, token))
        t = Token(token, STATE_BSLASH, STATE_BSLASH);
    else if(s.get_token(STATE_ARG, token))
        t = Token(token, STATE_ARG, STATE_ARG);
    else if(s.get_token(STATE_SPACE, token))
        t = Token(token, STATE_SPACE);
    else {
        if(s._pos == s._buffer_size)  // create empty token on NUL char
            t = Token();
        else  // create token for UNKNOWN char
            t = Token(std::string(1, s._buffer[s._pos]));

        ++s._pos;  // when fail to get token, go to next position
    }

    return s;
}
