

#include "token.h"



static const std::unordered_map<std::string, Token::TokenType> IdentifyMap{
    { "auto", Token::TOKEN_AUTO },
    { "break", Token::TOKEN_BREAK },
    { "return", Token::TOKEN_RETURN },
    {"\0", Token::TOKEN_EOF},
    // { "case", Token::CASE },
    // { "continue", Token::CONTINUE },
    // { "default", Token::DEFAULT },
    // { "do", Token::DO },
    // { "double", Token::DOUBLE },
    // { "else", Token::ELSE },
    // { "enum", Token::ENUM },
    // { "extern", Token::EXTERN },
    { "int", Token::TOKEN_INT},
    { "char", Token::TOKEN_CHAR },
    { "struct", Token::TOKEN_STRUCT },
    { "float", Token::TOKEN_FLOAT },
    { "void", Token::TOKEN_VOID },
    // { "for", Token::FOR },
    // { "goto", Token::GOTO },
    // { "if", Token::IF },
    // { "inline", Token::INLINE },
    // { "long", Token::LONG },
    // { "signed", Token::SIGNED },
    // { "unsigned", Token::UNSIGNED },
    // { "register", Token::REGISTER },
    // { "restrict", Token::RESTRICT },
    // { "short", Token::SHORT },
    // { "sizeof", Token::SIZEOF },
    { "static", Token::TOKEN_STATIC },
    { "const", Token::TOKEN_CONST },
    // { "switch", Token::SWITCH },
    // { "typedef", Token::TYPEDEF },
    // { "union", Token::UNION },
    // { "void", Token::VOID },
    // { "volatile", Token::VOLATILE },
    // { "while", Token::WHILE },
    // { "_Alignas", Token::ALIGNAS },
    // { "_Alignof", Token::ALIGNOF },
    // { "_Atomic", Token::ATOMIC },
    // { "__attribute__", Token::ATTRIBUTE },
    // { "_Bool", Token::BOOL },
    // { "_Complex", Token::COMPLEX },
    // { "_Generic", Token::GENERIC },
    // { "_Imaginary", Token::IMAGINARY },
    // { "_Noreturn", Token::NORETURN },
    // { "_Static_assert", Token::STATIC_ASSERT },
    // { "_Thread_local", Token::THREAD },
};

Token::TokenType Token::GetIdentifyToken(const std::string& key) {
    auto kwIter = IdentifyMap.find(key);
    if (IdentifyMap.end() == kwIter)
        return Token::TOKEN_ERROR;	// Not a key word type
    return kwIter->second;
}

