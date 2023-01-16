

#include <iostream>
#include <string>
#include <unordered_map>
#include <list>
#include "string.h"


#ifndef token_H
#define token_H



class Token {
public:
    Token() {
        length = 0;
        line = 0;
        type = TOKEN_ERROR;
        str_ = "";
    };
    enum TokenType{
        // Single-character tokens
        // http://c.biancheng.net/c/ascii/
        TOKEN_SPACE = ' ', // 00100000 十进制32
        TOKEN_BANG = '!',
        TOKEN_SHARP = '#', // 35
        TOKEN_D = '$',
        TOKEN_AND = '&', // 38
        TOKEN_MOD = '%',
        TOKEN_LEFT_PAREN = '(', // 十进制是40
        TOKEN_RIGHT_PAREN = ')',
        TOKEN_LEFT_BRACE = '{',
        TOKEN_RIGHT_BRACE = '}',
        TOKEN_LEFT_SQ_BRACE = '[',
        TOKEN_RIGHT_SQ_BRACE = ']',
        TOKEN_COMMA = ',',
        TOKEN_DOT = '.',
        TOKEN_MINUS = '-',
        TOKEN_PLUS = '+',
        TOKEN_SLASH = '/',
        TOKEN_SEMICOLON = ';',
        TOKEN_COLON = ':',
        TOKEN_STAR = '*',

        // One or two character tokens.
        TOKEN_BANG_EQUAL,
        TOKEN_EQUAL, TOKEN_EQUAL_EQUAL,
        TOKEN_GREATER, TOKEN_GREATER_EQUAL,
        TOKEN_LESS, TOKEN_LESS_EQUAL,

        // Literals.
        TOKEN_IDENTIFIER, TOKEN_STRING, TOKEN_NUMBER,


        // IDENTIFIER,
        // CONSTANT,
        // I_CONSTANT,
        // C_CONSTANT,
        // F_CONSTANT,
        // LITERAL,

        // Identify Key word
        TOKEN_CLASS, TOKEN_ELSE, TOKEN_FALSE,
        TOKEN_FOR, TOKEN_FUN, TOKEN_IF, TOKEN_NIL, TOKEN_OR,
        TOKEN_PRINT, TOKEN_RETURN, TOKEN_SUPER, TOKEN_THIS,
        TOKEN_TRUE, TOKEN_VAR, TOKEN_WHILE,
        TOKEN_AUTO,
        TOKEN_BREAK,
        TOKEN_ERROR, TOKEN_EOF,

        // type_specifier 类型说明符
        TOKEN_INT,
        TOKEN_FLOAT,
        TOKEN_CHAR,
        TOKEN_STRUCT,
        TOKEN_VOID,

        TOKEN_STATIC,

        // type_qualifier 类型限定符
        TOKEN_CONST,

        //对齐说明符 alignment_specifier
        // storage
    };

    // type for token
    TokenType type;
    const char* start;
    int length;
    int line;
    std::string str_;

    static TokenType GetIdentifyToken(const std::string& key);
};



using TokenList = std::list<const Token*>; // 理论上用指针好 但是得管理

#endif //


// declare
//declaration-specifiers包括存储类别说明符(storage class specifier)
//类型说明符(type specifier) 
//类型限定符(type qualifier),函数说明符(function specifier)
//从C11起又增加了一个对齐说明符(alignment specifier)。

// c++ 等
// ( declarator )
//      direct-declarator [ type-qualifier-list assignment-expressionopt ]
//      direct-declarator [ static type-qualifier-list assignment-expression ]
//      direct-declarator [ type-qualifier-list static assignment-expression ]
//      direct-declarator [ type-qualifier-list * ]
//      direct-declarator ( parameter-type-list )
//      direct-declarator ( identifier-list )
