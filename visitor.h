
#pragma once

#include "map"
#include "Ast.h"
#include "vector"



/*
    [1]declare check 检查 看变量是否申明 且{ 和 }开始结束
    [2]flow_control check 检查流程 如返回值 return break嵌套等 遍历语句
    [3]type check 检查赋值和初始化等是否正确 左值右值等 还有binary 和 unary的操作是否合乎规范
*/



class Scope{
public:
    void ScopeAddSymbol(std::string name, Declaration* node);
    Declaration* ScopeLookUpSymbol(std::string name);
    Scope* parentScope;
    std::map<std::string, Declaration*> symbolMap;
};


class NodeVisitor{
public:
    virtual void Visit(ASTNode* node);
    virtual void Visit_TreeNode(TreeNode* node);
    virtual void Visit_CompoundStatement(CompoundStmtNode* node);
    virtual void Visit_Expression(ExprNode* node);
    virtual void Visit_Declaration(Declaration* node);
    virtual void Visit_FunctionDef(FunctionDefNode* node);
    virtual void visit_FunctionCall(FunctionCallNode* node);
    virtual void Visit_Statement(StatementNode* node);

    virtual void Visit_Binary(BinOpNode* node);
    virtual void Visit_Assign(AssignNode* node);
    virtual void Visit_Variable(VariableNode* node);
    virtual void Visit_Number(NumberNode* node);
    virtual void Visit_Unary(UnaryOpNode* node);
    virtual void Visit_Return(ReturnNode* node);
    virtual void Visit_DeclarationList(DeclarationListNode* node);
};

class SymbolTableVisitor: public NodeVisitor{
    /*
        检测declare的 变量
        收集scope 并把 scope的内容放到 vec 中
        后续的node 则会有一个指向这个scope的变量

        基本就是只管定义和一些variable的重定向 variable的类型等信息确定
        把一些delcare的node进行处理 得出具体的信息给后面codegen使用
    */
public:
    // override
    void Visit_TreeNode(TreeNode* node);
    void Visit_Declaration(Declaration* node);
    void Visit_FunctionDef(FunctionDefNode* node);
    // void Visit_DeclarationList(DeclarationListNode* node);
    void Visit_Variable(VariableNode* node);
    void visit_FunctionCall(FunctionCallNode* node);

    void PushScope();
    Scope* PopScope();
    std::vector<Scope*> scopeStack;
    Scope* curScope;
};


class FlowControlVisitor: public NodeVisitor{
    /*
        检测流程的
    */
};


class TypeVisitor: public NodeVisitor{
    /*
        检测类型的
        assign declare的init 等几种类型即可
    */
};


