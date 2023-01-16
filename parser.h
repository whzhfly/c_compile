
#include "Ast.h"
#include <stdio.h>
#include <unordered_map>
#include <iostream>
#include <list>


// Assembly worker

enum Precedence{
    /*
    expression 就是赋值
    用来区分expression中应该进行到哪一步为止 3+15*2 这种数学表达式之间如何通过优先级来组合
    [3 + 15 * 2] ==》 [num add num mul num] 5个token
    从左向右执行一直执行到 当前等级小于传入等级
    也就是说 只要一次调用的优先级为n 这个表达式一次解释到直到优先级小于n的为止
    比如 5*7-2 这个表达式 
    如果以* 这个优先级进入 会执行到5*7 后续的-2  不会执行
    如果以= 优先级进入 则会全部执行
    如过于是 !5*7-2    在!这个等级进入的话 只会执行到 !5
    */
    PREC_NONE,
    PREC_ASSIGNMENT,  // =
    PREC_OR,          // ||
    PREC_AND,         // &&
    PREC_EQUALITY,    // == !=
    PREC_COMPARISON,  // < > <= >=
    PREC_TERM,        // + -
    PREC_FACTOR,      // * /
    PREC_UNARY,       // ! -
    PREC_CALL,        // . ()
    PREC_PRIMARY
};



class TokenParse{
    /*
    token 打包机
    相当于机器 打包token成商品
    scanner是消耗字符 这是消耗一系列token
    ast 相当于 标签 二维码 分类 是token包装后的产物 
    这里可以用ast 也可以直接用虚拟的机器码去表示 这样就省略了ast这一步 所以可以抽象这一加工层
    */

public:

    void Advance();
    void Consume(Token::TokenType type, char* msg);
    bool Check(Token::TokenType expected);
    bool Match(Token::TokenType expected);
    bool IsAtEnd();
    void Next();

    // ====>>>>  Expression
    ParseNode* ParseExpression();
    ParseNode* PrecedenceParse(Precedence precedence);
    ParseNode* Map_Group(bool canAssign);
    ParseNode* Map_ParseCall(bool canAssign);
    ParseNode* Map_Dot(bool canAssign);
    ParseNode* Map_Unary(bool canAssign);
    ParseNode* Map_Binary(bool canAssign);
    ParseNode* Map_Variable(bool canAssign);
    ParseNode* Map_String(bool canAssign);
    ParseNode* Map_Number(bool canAssign);
    ParseNode* Map_And(bool canAssign);
    ParseNode* Map_Or(bool canAssign);
    ParseNode* Map_Literal(bool canAssign);


    CompoundStmtNode* ParseCompoundStmt();

    StatementNode* ParseStatement();
    Type* Decl_TypeSpecific();
    DeclarationListNode* ParseDeclare();
    DeclarationListNode* ParseArgument();
    // InitializerNode* Def_InitializerList();
    InitializerNode* Def_InitializerList();
    VariableListNode* Declarator_List();
    Declaration* Decl_Declarator();
    Declaration* Decl_Direct_Declarator();
    const char* VarDeclaration();
    FunctionDefNode* Decl_FunctionDef();
    void Decl_StrcutDef();

    // expressionStack helper
    void PushExpression(ParseNode*);
    ParseNode* PopExpression();


    // 整个文件就是一个function的概念
    TreeNode* TokenToTree();

    const Token* current;
    const Token* previous;
    TokenList* tokList_;
    TokenList::iterator cur_ ;
    std::list<ParseNode*> expressionStack;
};

// function point
typedef ParseNode* (TokenParse::* ParseFn)(bool);

class ParseRule {
public:
    ParseFn prefix;
    ParseFn infix;
    ParseFn postfix; // todo
    Precedence precedence;
    static ParseRule GetParseRule(Token::TokenType);
};
