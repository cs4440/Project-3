/*******************************************************************************
 * AUTHOR      : Thuan Tang
 * ID          : 00991588
 * CLASS       : CS008
 * HEADER      : tok
 * DESCRIPTION : Token holds std::string and type associated with it. Types
 *      are defined by state_machine.h
 *
 *      Specialized for command line arguments.
 ******************************************************************************/
#ifndef TOKEN_H
#define TOKEN_H

#include <iostream>         // stream
#include <string>           // string
#include "state_machine.h"  // state_machine functions

class Token {
public:
    // CONSTRUCTORS
    Token(std::string str = "", int type = -1)
        : _token(str), _type(type), _sub_type(_type) {}
    Token(std::string str, int type, int sub_type)
        : _token(str), _type(type), _sub_type(sub_type) {}
    Token(const Token& t)
        : _token(t._token), _type(t._type), _sub_type(t._sub_type) {}
    Token& operator=(const Token& rhs);

    // ACCESSORS
    bool empty() const;                       // if Token is empty
    int type() const;                         // return type in INT
    int sub_type() const;                     // return sub type in INT
    virtual std::string type_string() const;  // return type of token in string
    std::string string() const;               // return string of token

    // MUTATORS
    void clear();
    void set_type(int type);
    void set_sub_type(int sub_type);
    void set_string(std::string str);

    // FRIENDS
    friend std::ostream& operator<<(std::ostream& outs, const Token& t);
    friend bool operator==(const Token& lhs, const Token& rhs);
    friend bool operator==(const Token& lhs, const std::string& rhs);
    friend bool operator!=(const Token& lhs, const Token& rhs);
    friend bool operator!=(const Token& lhs, const std::string& rhs);
    friend Token& operator+=(Token& lhs, const Token& rhs);
    friend Token& operator+=(Token& lhs, const std::string& rhs);
    friend std::string& operator+=(std::string& lhs, const Token& rhs);
    friend Token& operator+(Token& lhs, const Token& rhs);
    friend Token& operator+(Token& lhs, const std::string& rhs);
    friend std::string& operator+(std::string& lhs, const Token& rhs);

protected:
    std::string _token;  // token string
    int _type;           // type of token
    int _sub_type;       // sub type
};

#endif  // TOKEN_H
