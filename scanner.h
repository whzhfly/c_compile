
#include "token.h"






class Scanner {
    // word eater

public:
    explicit Scanner(const char* source){
            start = source;
            current = source;
            sourceName = source;
            line = 1;
        };

    static bool IsAlpha(char c);
    static bool IsDigit(char c);
    bool IsEndLine();
    char Advance();
    char Peek();
    char PeekNext();
    bool Match(char expected);
    void SkipWhiteSpace();

    Token* Scan();
    Token* MakeToken(Token::TokenType type);

    Token::TokenType IdentifierType();
    Token* ScanIdentifier();
    Token* ParseNumber();
    Token* String();
    Token* ErrorToken(const char* message);


    const char* sourceName;
    const char* start;
    const char* current;
    int line;
};
