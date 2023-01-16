#include "scanner.h"
#include "string.h"
#include <iterator>
#include <stdio.h>

bool Scanner::IsAlpha(char c){
    return (c >= 'a' && c <= 'z') ||
                (c >= 'A' && c <= 'Z') ||
                c == '_';
}

bool Scanner::IsDigit(char c){
        return c >= '0' && c <= '9';
}

bool Scanner::IsEndLine() {
    return *current == '\0';
}

char Scanner::Advance() {
    current++;
    return current[-1];
}

char Scanner::Peek() {
    return *current;
}

char Scanner::PeekNext() {
    if (IsEndLine()) return '\0';
    return current[1];
}

bool Scanner::Match(char expected) {
    if (IsEndLine()) return false;
    if (*current != expected) return false;
    current++;
    return true;
}

void Scanner::SkipWhiteSpace(){
    while (char c = Peek())
    {
        switch (c)
        {
        case ' ':
        // case 'r':
        case '\t':
            Advance();
            break;
        // new line
        case '\n':
            line++;
            Advance();
            break;
        // comment
        case '/':
            if (PeekNext() == '/'){
                while (Peek() != '\n' && !IsEndLine()) {Advance();}
            }
            else { return; }

        default:
            return;
        }
    }

}

Token* Scanner::ErrorToken(const char* message) {
    // 手动构成 记录原因
    Token* token = new Token();
    token->type = Token::TOKEN_ERROR;
    token->start = message;
    token->length = (int)strlen(message);
    token->line = line;
    return token;
}

Token* Scanner::ParseNumber() {
    while (IsDigit(Peek())) Advance();

  // Look for a fractional part.
    if (Peek() == '.' && IsDigit(PeekNext())) {
    // Consume the ".".
        Advance();

    while (IsDigit(Peek())) Advance();
    }

    return MakeToken(Token::TOKEN_NUMBER);
}

Token* Scanner::String() {
    while (Peek() != '"' && !IsEndLine()) {
        if (Peek() == '\n') line++;
            Advance();
    }

    if (IsEndLine()) return ErrorToken("Unterminated string.");

  // The closing quote.
    Advance();
    return MakeToken(Token::TOKEN_STRING);
}

Token::TokenType Scanner::IdentifierType(){
    // hashTable search
    int length = (int)(current - start);
    std::string identify_str = std::string(start, length);
    Token::TokenType map_token = Token::GetIdentifyToken(identify_str);
    // std::cout<< "  ==> "<< identify_str  <<  "<== "<< map_token<< std::endl;
    if (map_token == Token::TOKEN_ERROR){
        return Token::TOKEN_IDENTIFIER;
    }
    else {
        return map_token;
    }
}

Token* Scanner::ScanIdentifier() {
    while (IsAlpha(Peek()) || IsDigit(Peek())){
        Advance();
    }
    return MakeToken(IdentifierType());
}

Token* Scanner::MakeToken(Token::TokenType type) {
    Token* token = new Token();
    token->type = type;
    token->start = this->start;
    token->length = (int)(this->current - this->start);
    token->line = line;
    token->str_ = std::string(this->start, token->length);
    return token;
}

Token* Scanner::Scan(){
    //> call-skip-whitespace
    SkipWhiteSpace();
//< call-skip-whitespace
    start = current;

    if (IsEndLine()) return MakeToken(Token::TOKEN_EOF);
//> scan-char

    char c = Advance();
//> scan-identifier
    if (IsAlpha(c)) return ScanIdentifier();
//< scan-identifier
//> scan-number
    if (IsDigit(c)) return ParseNumber();
//< scan-number

    switch (c) {
        case '(': return MakeToken(Token::TOKEN_LEFT_PAREN);
        case ')': return MakeToken(Token::TOKEN_RIGHT_PAREN);
        case '{': return MakeToken(Token::TOKEN_LEFT_BRACE);
        case '}': return MakeToken(Token::TOKEN_RIGHT_BRACE);
        case ';': return MakeToken(Token::TOKEN_SEMICOLON);
        case ',': return MakeToken(Token::TOKEN_COMMA);
        case '.': return MakeToken(Token::TOKEN_DOT);
        case '-': return MakeToken(Token::TOKEN_MINUS);
        case '+': return MakeToken(Token::TOKEN_PLUS);
        case '/': return MakeToken(Token::TOKEN_SLASH);
        case '*': return MakeToken(Token::TOKEN_STAR);
    //> two-char
        case '!':
        return MakeToken(
            Match('=') ? Token::TOKEN_BANG_EQUAL : Token::TOKEN_BANG);
        case '=':
        return MakeToken(
            Match('=') ? Token::TOKEN_EQUAL_EQUAL : Token::TOKEN_EQUAL);
        case '<':
        return MakeToken(
            Match('=') ? Token::TOKEN_LESS_EQUAL : Token::TOKEN_LESS);
        case '>':
        return MakeToken(
            Match('=') ? Token::TOKEN_GREATER_EQUAL : Token::TOKEN_GREATER);
    //< two-char
    //> scan-string
        case '"': return String();
//< scan-string
    }
//< scan-char

    return ErrorToken("Unexpected character.");
}

