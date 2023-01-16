
#include "scanner.h"
#include "parser.h" // 处理tree
#include "visitor.h"
#include "codegen.h"

// 除了指针，其他了类成员必须在构造函数前初始化，最迟是在初始化列表

class TokenCollector {
public:
    TokenCollector(const char* source) {
        scan = new Scanner(source);
        tokList_ = new TokenList();
    }

    void GenTokenList();
    void Advance();
    void AddToken();
    void ShowTokens();

    Scanner* scan;
    TokenList* tokList_;

    Token* previous; // 改成指针
    Token* current;
};



class WalkerTree{
public:
    TokenCollector* collect;
    TokenParse* parse;
    char* source;
    void walkerRun(char* source);

    void LexicalAnalysis();
    void SetWalkerParse();
    void Do_Trans();
};



