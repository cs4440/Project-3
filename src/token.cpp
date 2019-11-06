#include "../include/token.h"

Token& Token::operator=(const Token& rhs) {
    _token = rhs._token;
    _type = rhs._type;
    _sub_type = rhs._sub_type;
    return *this;
}

/*******************************************************************************
 * DESCRIPTION:
 *  Returns empty state of Token
 *
 * PRE-CONDITIONS:
 *  none
 *
 * POST-CONDITIONS:
 *  none
 *
 * RETURN:
 *  bool
 ******************************************************************************/
bool Token::empty() const { return _token.empty(); }

/*******************************************************************************
 * DESCRIPTION:
 *  Returns an integer corresponding to the string type, such as the
 *  state_machine's state constants.
 *
 * PRE-CONDITIONS:
 *  none
 *
 * POST-CONDITIONS:
 *  none
 *
 * RETURN:
 *  int of string type (or state constants in state_machine)
 ******************************************************************************/
int Token::type() const { return _type; }

/*******************************************************************************
 * DESCRIPTION:
 *  Returns an integer corresponding to the string sub type, such as the
 *  state_machine's state constants.
 *
 * PRE-CONDITIONS:
 *  none
 *
 * POST-CONDITIONS:
 *  none
 *
 * RETURN:
 *  int of string type (or state constants in state_machine)
 ******************************************************************************/
int Token::sub_type() const { return _sub_type; }

/*******************************************************************************
 * DESCRIPTION:
 *  Returns a readable string corresponding to the string type, such as the
 *  state_machine's state constants, for human interpretation.
 *
 * PRE-CONDITIONS:
 *  none
 *
 * POST-CONDITIONS:
 *  none
 *
 * RETURN:
 *  string type for human readable interpretation
 ******************************************************************************/
std::string Token::type_string() const {
    std::string type_string;

    switch(_type) {
        case state_machine::STATE_ERROR:
            type_string = "ERROR";
            break;
        case state_machine::STATE_ARG:
            type_string = "ARGUMENT";
            break;
        case state_machine::STATE_QUOTE:
            type_string = "QUOTE";
            break;
        case state_machine::STATE_QUOTE_S:
            type_string = "SINGLE QUOTE";
            break;
        case state_machine::STATE_QUOTE_D:
            type_string = "DOUBLE QUOTE";
            break;
        case state_machine::STATE_OP:
            type_string = "OPERATOR";
            break;
        case state_machine::STATE_BSLASH:
            type_string = "BACKSLASH";
            break;
        case state_machine::STATE_SPACE:
            type_string = "SPACE";
            break;
        default:
            type_string = "UNKNOWN";
    }

    return type_string;
}

/*******************************************************************************
 * DESCRIPTION:
 *  Returns the string value, such as the input buffer string from STokenizer.
 *
 * PRE-CONDITIONS:
 *  none
 *
 * POST-CONDITIONS:
 *  none
 *
 * RETURN:
 *  string
 ******************************************************************************/
std::string Token::string() const { return _token; }

/*******************************************************************************
 * DESCRIPTION:
 *  Clear the token
 *
 * PRE-CONDITIONS:
 *  none
 *
 * POST-CONDITIONS:
 *  private variables reset
 *
 * RETURN:
 *  none
 ******************************************************************************/
void Token::clear() {
    _token.clear();
    _type = _sub_type = -1;
}

/*******************************************************************************
 * DESCRIPTION:
 *  Changes string type.
 *
 * PRE-CONDITIONS:
 *  int type: string type
 *
 * POST-CONDITIONS:
 *  int _type: set to new type
 *
 * RETURN:
 *  none
 ******************************************************************************/
void Token::set_type(int type) { _type = type; }

/*******************************************************************************
 * DESCRIPTION:
 *  Changes string sub type.
 *
 * PRE-CONDITIONS:
 *  int type: string type
 *
 * POST-CONDITIONS:
 *  int _type: set to new type
 *
 * RETURN:
 *  none
 ******************************************************************************/
void Token::set_sub_type(int sub_type) { _sub_type = sub_type; }

/*******************************************************************************
 * DESCRIPTION:
 *  Changes tok's string.
 *
 * PRE-CONDITIONS:
 *  std::string str: new string
 *
 * POST-CONDITIONS:
 *  std::string _token: set to new string
 *
 * RETURN:
 *  none
 ******************************************************************************/
void Token::set_string(std::string str) { _token = str; }

/*******************************************************************************
 * DESCRIPTION:
 *  Inserts a formatted Token's string value to the outstream.
 *
 * PRE-CONDITIONS:
 *  ostream& outs : out stream
 *  const Token& t: Token object with string value
 *
 * POST-CONDITIONS:
 *  none
 *
 * RETURN:
 *  ostream by reference
 ******************************************************************************/
std::ostream& operator<<(std::ostream& outs, const Token& t) {
    return outs << "|" + t.string() + "|";
}

/*******************************************************************************
 * DESCRIPTION:
 *  Equality comparison
 *
 * PRE-CONDITIONS:
 *  const Token lhs: left tok
 *  const Token rhs: right tok
 *
 * POST-CONDITIONS:
 *  none
 *
 * RETURN:
 *  bool
 ******************************************************************************/
bool operator==(const Token& lhs, const Token& rhs) {
    return lhs._type == rhs._type;
}

/*******************************************************************************
 * DESCRIPTION:
 *  Inequality comparison
 *
 * PRE-CONDITIONS:
 *  const Token lhs: left tok
 *  const Token rhs: right tok
 *
 * POST-CONDITIONS:
 *  none
 *
 * RETURN:
 *  bool
 ******************************************************************************/
bool operator!=(const Token& lhs, const Token& rhs) {
    return lhs._type != rhs._type;
}

/*******************************************************************************
 * DESCRIPTION:
 *  Add two Tokens together with left hand side concatenating the right hand
 *  side's string.
 *
 * PRE-CONDITIONS:
 *  const Token lhs: left tok
 *  const Token rhs: right tok
 *
 * POST-CONDITIONS:
 *  none
 *
 * RETURN:
 *  bool
 ******************************************************************************/
Token& operator+=(Token& lhs, const Token& rhs) {
    lhs._token += rhs._token;
    lhs._type = rhs._type;
    lhs._sub_type = rhs._sub_type;
    return lhs;
}

/*******************************************************************************
 * DESCRIPTION:
 *  Add two Tokens together with left hand side concatenating the right hand
 *  side's string.
 *
 * PRE-CONDITIONS:
 *  const Token lhs: left tok
 *  const Token rhs: right tok
 *
 * POST-CONDITIONS:
 *  none
 *
 * RETURN:
 *  bool
 ******************************************************************************/
Token& operator+(Token& lhs, const Token& rhs) {
    lhs._token += rhs._token;
    lhs._type = rhs._type;
    lhs._sub_type = rhs._sub_type;
    return lhs;
}
