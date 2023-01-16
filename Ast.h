#pragma once

#include <iostream>
#include <unordered_map>
#include <list>
#include <stdio.h>
#include "token.h"

#define EMIT(x) std::cout<<x
#define END_EMIT(x) std::cout<<x<<std::endl

#define WARP_EMIT(x) std::cout<<x
#define PTR_EXIT_EMIT(ptr, xx) if( ptr!= NULL) {xx->EmitNode();} //

enum ExpType{
    CONSTANT, // 常亮
    VARIABLE, // 变量
    UNARY_OP,
    BINARY_OP, //
    ASSIGN_OP, // 赋值
    INITIAL, // 初始化
};

// 保持一致
enum RegValType{
    POINT,
    INT,
    CHAR,
};

// 前置declare
extern class Scope;
extern class FrameManager;
class NodeVisitor;
class BaseOperand;
class VariableNode;
class ExprNode;
class FunctionDefNode;
class InitializerNode;
class Declaration;
using ParseNode = ExprNode;

// 参考python ast的一些内容
class ASTNode {
public:
    Scope* nodeScope;
    RegValType valueType;
    virtual void EmitNode(){};
    virtual void Visit(NodeVisitor* visitor){
    };
};

class StatementNode: public ASTNode {
public:
    Declaration* targetDeclarationNode; // todo
    void EmitNode();
};

class ExprNode : public StatementNode {
public:
    Token::TokenType binary_op;
    Token::TokenType unary_op;
    char* variable_name;
    VariableNode* assign_var;
    ExprNode* assign_value; //
    ExprNode* initial_value;
    int constant_value;
    ExprNode* binary_left;
    ExprNode* binary_right;
    ExprNode* unary_node;
    void EmitNode();
    void Visit(NodeVisitor* visitor);
};

/*
 * Type 类型 char int 等
 */

class Type: public ASTNode {
public:
    Type* child;
    int typeSize;
    void SetBaseType(Type* ty);
    void EmitNode();
    void SetDefineType(RegValType va, int size);
    int GetTypeSize();
    RegValType GetRegValType();
};

class BaseType: public Type {};

class IntTypeNode: public BaseType{
    void EmitNode();
};

class CharTypeNode: public BaseType{
    void EmitNode();
};

class BoolTypeNode: public BaseType{};
class PointTypeNode: public BaseType{};



class CustomizeType: public Type {};
class StructTypeNode: public CustomizeType{};
class EnumTypeNode: public CustomizeType{};
class FuncTypeNode: public CustomizeType{
    void EmitNode();
};


/*
 * Declare: type + declarator
 */

class Declaration: public ASTNode{
    // A node representing a declaration of a function or variable
public:
    Type* declarationType; // 类型
    const char* varName;
    InitializerNode* init_node;
    FunctionDefNode* function; // 先放这里吧。。。 后续统一用function表示
    bool defineComplete; // 默认false
    BaseOperand* compileLocation;
    void SetType(Type* ty);
    void AddType(Type* ty);
    void EmitNode();
};


/*
 * ListNode 复合Node
 */

class ListNode: public ASTNode {};

class StatementListNode: public ListNode{
public:
    std::list<StatementNode*> nodeList;
    void AddNode(StatementNode* st);
    void EmitNode();
}; // 一系列statement

class DeclarationListNode: public ListNode{ // 有type的 declare
public:
    std::list<Declaration*> nodeList; // 一系列declare
    void AddNode(Declaration* st);
    void EmitNode();
};

class CompoundStmtNode: public StatementNode{
public:
    StatementListNode* statements;
    DeclarationListNode* declarations;
    Scope* comScope;
     // 复合的statement 含有body 包括函数就是里面有个这种node
     // 这里面有顺序问题 到时候可以用函数 或者另一个记录改以何种顺序来搞定
     // 不过这里我们先用把申明放前面
    void EmitNode();
};

class TreeNode: public ListNode {
    // 文件
    // 本质应该也是一个函数
public:
    CompoundStmtNode* compBody;
    void SetBody(CompoundStmtNode* body);
    void EmitNode();
    void Visit(NodeVisitor* visitor);
};


class VariableListNode: public ListNode{ 
// 区别是 var 比较简单 这个节点没有设置type
// 如 int a; a的 type 是int 是在最后确定的
public:
    std::list<Declaration*> nodeList;
    void AddNode(Declaration* st){
        nodeList.push_back(st);
    };
};

class FunctionDefNode: public ListNode{
public:
    CompoundStmtNode* compBody;
    std::string functionName;
    DeclarationListNode* arguments;
    void SetBody(CompoundStmtNode* body);
    void EmitNode();
    FrameManager* nodeFrame;
};

/*
 * Expression
 */

class UnaryOpNode : public ExprNode{
public:
    void OpAdd(ParseNode* node);
    void EmitNode();
    void Visit(NodeVisitor* visitor);
};

class BinOpNode : public ExprNode{
public:
    void Visit(NodeVisitor* visitor);
    void BinAdd(ParseNode* left, ParseNode* right);
    void EmitNode();
};

class VariableNode: public ExprNode {
    /*
        变量 存在值类型 这种就是expression
        还有一些struct的 这就就是define
        都在这个自己对应的Declaration 中
    */
public:
    void EmitNode();
    void Visit(NodeVisitor* visitor);
};

class AssignNode : public ExprNode {
    void Visit(NodeVisitor* visitor);
    void EmitNode();
};

class NumberNode : public ExprNode {
public:
    NumberNode(int va): value(va){}
    void Visit(NodeVisitor* visitor);
    void EmitNode();
    int value;
};


class LiteralNode : public ExprNode {
public:
    Token::TokenType litType;
};


class InitializerNode: public ExprNode{
/*
* InitNode 初始化的 辅助declare
*/
public:
    void EmitNode();
    void Visit(NodeVisitor* visitor);
};

class FunctionCallNode : public ExprNode {
public:
    ExprNode* function;
    StatementListNode* argList;
    void Visit(NodeVisitor* visitor);
    void EmitNode();
};

/*
 * Statement 语句
 */

class ReturnNode : public StatementNode {
public:
    ExprNode* retExpr;
    void Visit(NodeVisitor* visitor);
    void EmitNode();
};
class ForNode : public StatementNode {};
class WhileNode : public StatementNode {};
class PrintNode : public StatementNode {};
