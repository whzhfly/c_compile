#include "walker.h"


void TokenCollector::Advance() {
    previous = current;
    while(true) {
        //current = &(scan->Scan()); 这种不行？？？ 可能是 c的return 在返回对象的时候地址会存在一些问题
        current = scan->Scan();
        if (current->type != Token::TOKEN_ERROR) break;
        // 忽略错误的清楚的Token
        // Token::ErrorAtCurrent(parser.current.start);
    }
}

void TokenCollector::GenTokenList() {
    while (true)
    {
        Advance();
        switch (current->type)
        {
            case Token::TOKEN_ERROR:
                // 是否报错??
                break;
            case Token::TOKEN_EOF:
                AddToken();
                return;
            default:
                AddToken();
                break;
        }
    }
}

void TokenCollector::AddToken() {
    // add token to list
    tokList_->push_back(current);
}

void TokenCollector::ShowTokens() {
    auto c = tokList_;
    auto begin_ = tokList_->begin();
    auto end_ = tokList_->end();
    for (auto iter = begin_; iter != end_; ++iter){
        std::cout<< (**iter).str_<< std::endl;
    }
}

/*
 * WalkerTree
*/

void WalkerTree::walkerRun(char* source){
    this->source = source;
    LexicalAnalysis();
    SetWalkerParse();
    Do_Trans();
}

void WalkerTree::LexicalAnalysis(){
    // 词法分析
    // 把一系列的字符拆成Token
    TokenCollector* collect = new TokenCollector(source);
    collect->GenTokenList();
    collect->ShowTokens();
    this->collect = collect;
}

void WalkerTree::SetWalkerParse(){
    // 语法分析
    // Syntax Analysis
    TokenParse* machine = new TokenParse();
    machine->tokList_ = this->collect->tokList_;
    TokenList::iterator beg = this->collect->tokList_->begin();
    machine->current = *beg;
    machine->cur_ = beg; // 设置
    machine->previous = *beg;
    this->parse = machine;
}

void WalkerTree::Do_Trans(){
    // 预处理第一步实现
    std::cout<< "[2]PARSE_BEGIN"<< std::endl;
    TreeNode* f = this->parse->TokenToTree();
    std::cout<< "[2]PARSE_END"<< std::endl;

    std::cout<< "[3]Symbol_VISIT_BEGIN"<< std::endl;
    SymbolTableVisitor* symVisit = new SymbolTableVisitor();
    symVisit->Visit(f);
    std::cout<< "[3]Symbol_VISIT_END"<< std::endl;

    std::cout<< "[6]Codegen_VISIT_BEGIN"<< std::endl;
    CodeGenVisitor* codegen = new CodeGenVisitor();
    codegen->Visit(f);
    std::cout<< "[6]Codegen_VISIT_BEGIN"<< std::endl;
    // std::cout<<std::endl <<codegen->cur_str;
}







