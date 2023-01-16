#include "visitor.h"
#include<string>
#include "Ast.h"


/*
 * NodeVisitor
 */

void NodeVisitor::Visit(ASTNode* node){
    node->Visit(this); // 动态函数 根据类型调动子函数
}

void NodeVisitor::Visit_TreeNode(TreeNode* node){
    if (node->compBody){
        Visit_CompoundStatement(node->compBody);
    }
}

void NodeVisitor::Visit_CompoundStatement(CompoundStmtNode* node){

    StatementListNode* statements = node->statements;
    DeclarationListNode* declarations = node->declarations;

    for ( auto decl : declarations->nodeList){
        Visit_Declaration(decl);
    }

    for (auto st : statements->nodeList) {
        Visit_Statement(st); // 直接调用 Visit? 因为不确定类型 所以交给自己去处理
    }
}

void NodeVisitor::Visit_Declaration(Declaration* node){
    // body visit
    // 子类应该会自己实现
}

void NodeVisitor::Visit_FunctionDef(FunctionDefNode* node){
    // 子类应该会自己实现
}

void NodeVisitor::visit_FunctionCall(FunctionCallNode* node){
    // 子类应该会自己实现
}

void NodeVisitor::Visit_Assign(AssignNode* node){
    // 子类应该会自己实现
    node->assign_var->Visit(this);
}

void NodeVisitor::Visit_Variable(VariableNode* node){
    // 子类应该会自己实现
}

void NodeVisitor::Visit_Expression(ExprNode* node){
    // 不存在expression 都是子类
    // 如 binary expression
    // variable name expression 等
    node->Visit(this);
};

void NodeVisitor::Visit_Statement(StatementNode* node){
    // 不存在Statement
    // 子类应该会自己实现
    // 理论上要么是 print for 或者 return 以及 expression等
    node->Visit(this);
}

void NodeVisitor::Visit_Binary(BinOpNode* node){
    node->binary_left->Visit(this);
    node->binary_right->Visit(this);
}

void NodeVisitor::Visit_Number(NumberNode* node){
}

void NodeVisitor::Visit_Unary(UnaryOpNode* node){
}

void NodeVisitor::Visit_Return(ReturnNode* node){
}

void NodeVisitor::Visit_DeclarationList(DeclarationListNode* node){
    for (auto decl : node->nodeList){
        Visit_Declaration(decl);
    }
}

/*
 * Scope
 */

void Scope::ScopeAddSymbol(std::string name, Declaration* node){
    if (this->symbolMap.find(name)!= this->symbolMap.end()){
        // 检查类型 并报错
    }
    this->symbolMap.insert(std::pair<std::string, Declaration*>(name, node));
    // int c = this->symbolMap.size();
}

Declaration* Scope::ScopeLookUpSymbol(std::string name){
    auto iter = this->symbolMap.find(name);
    if (iter!= this->symbolMap.end()){
        /*
            其实这里得考虑顺序
            存在获取local a 是在 申明a 之前的 这种情况得判断下
            这其实涉及到语法
            int a = 1;
            {
                c = a + 2; // 这里的a如何处理
                int a = 2;
            }
        */
        return (*iter).second;
    }
    else if(this->parentScope){
        return this->parentScope->ScopeLookUpSymbol(name); //  recursively find
    }
    return NULL;
}

/*
 * SymbolTableVisitor
 */

void SymbolTableVisitor::PushScope(){
    Scope* sc = new Scope();
    if (this->curScope){
        sc->parentScope = this->curScope;
    }
    this->scopeStack.push_back(sc);
    this->curScope = sc;
}

Scope* SymbolTableVisitor::PopScope(){
    if (scopeStack.empty()){
        return NULL;
    }
    Scope* sc = scopeStack.back();
    this->scopeStack.pop_back();
    if (scopeStack.empty()){
        this->curScope = NULL;
        return sc;
    }
    this->curScope = scopeStack.back();
    return sc;
}

void SymbolTableVisitor::Visit_FunctionDef(FunctionDefNode* node){
    this->PushScope();
    node->nodeScope = this->curScope; // 先简单搞一下 后续还是走decl
    Visit_DeclarationList(node->arguments);
    Visit_CompoundStatement(node->compBody);
    this->PopScope();
}

void SymbolTableVisitor::Visit_Declaration(Declaration* node){
    // decl node need set scope to storage declare for variable to use
    node->nodeScope = curScope;
    this->curScope->ScopeAddSymbol(node->varName, node); // 有个类型转换
    // body visit
    if (node->function){
        Visit_FunctionDef(node->function); // 这个逻辑写早 ast中 还是这里?
    }
    if (node->init_node){
        // 表示complete
        node->defineComplete = true; //表示初始化了
        node->init_node->initial_value->Visit(this);
    }
}

void SymbolTableVisitor::Visit_TreeNode(TreeNode* node){
    this->PushScope();
    node->nodeScope = this->curScope; // unique set scope
    if (node->compBody){
        Visit_CompoundStatement(node->compBody);
    }
    this->PopScope();
}

void SymbolTableVisitor::Visit_Variable(VariableNode* node){
    Declaration* decl = this->curScope->ScopeLookUpSymbol(node->variable_name);
    node->targetDeclarationNode = decl;
}

void SymbolTableVisitor::visit_FunctionCall(FunctionCallNode* node){
    Declaration* decl = this->curScope->ScopeLookUpSymbol(node->function->variable_name);
    node->targetDeclarationNode = decl;
    for (auto st : node->argList->nodeList){
        Visit_Statement(st);
    }
}
