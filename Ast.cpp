#include "Ast.h"
#include "visitor.h"



/*
 * Type
 */

void Type::SetBaseType(Type* ty){
    if (this->child == NULL){
        this->child = ty;
    }
    else{
        this->child->SetBaseType(ty);
    }
}

void Type::EmitNode(){
    EMIT(" BASE-TYPE ");
}

void Type::SetDefineType(RegValType va, int size){
    this->valueType = va;
    this->typeSize = size;
}

int Type::GetTypeSize(){
    if (this->child){
        // 默认最里面是int类型
        return this->child->GetTypeSize();
    }
    return this->typeSize;
}

RegValType Type::GetRegValType(){
    return valueType;
}

void IntTypeNode::EmitNode(){
    EMIT(" INT-TYPE ");
}

void CharTypeNode::EmitNode(){
    EMIT(" CHAR-TYPE ");
}

void FuncTypeNode::EmitNode(){
    EMIT(" FUNCTION-TYPE ");
}

/*
 * Node
 */
void StatementNode::EmitNode(){
    // 不能调用这里的方法 子类override
    EMIT(" StatementNode ");
}

void ExprNode::EmitNode(){
    // 不能调用这里的方法 子类override
    EMIT(" ExprNode ");
};

void ExprNode::Visit(NodeVisitor* visitor){
    visitor->Visit_Expression(this);
};


void Declaration::EmitNode(){
    EMIT(" Declaration : ");
    EMIT(varName);
    EMIT(" DECL-type: ");
    declarationType->EmitNode();
    // function
    if (function != NULL){
        EMIT("\n");
        function->EmitNode();
        EMIT("\n");
    }
    if (init_node != NULL){
        init_node->EmitNode();
    }
}

void Declaration::SetType(Type* ty) {
    if (this->declarationType == NULL){
        this->declarationType = ty;
    }
    else {
        this->declarationType->SetBaseType(ty);
    }
}

void Declaration::AddType(Type* ty){
    // int a; 和 int aa(); 的区别
    ty->SetBaseType(this->declarationType);
    this->declarationType = ty; // type 更新 如 函数类型 child 是int类型
}


void StatementListNode::EmitNode(){
    auto begin_ = nodeList.begin();
    auto end_ = nodeList.end();
    for(auto iter = begin_; iter!=end_; iter++){
        (*iter)->EmitNode();
        // (*iter)->EmitNode(); 这种指针无法确定！
        EMIT("\n");
    };
}

void StatementListNode::AddNode(StatementNode* st){
    nodeList.push_back(st);
};

void DeclarationListNode::AddNode(Declaration* st){
    nodeList.push_back(st);
};


void DeclarationListNode::EmitNode(){
    auto begin_ = nodeList.begin();
    auto end_ = nodeList.end();
    for(auto iter = begin_; iter!=end_; iter++){
        (*iter)->EmitNode();
        EMIT("\n");
    };
}

void CompoundStmtNode::EmitNode(){
    EMIT(" { ");
    if (declarations != NULL){
        declarations->EmitNode();
    }
    if (statements != NULL){
        statements->EmitNode();
    }
    EMIT(" } ");
}

void TreeNode::Visit(NodeVisitor* visitor){
    visitor->Visit_TreeNode(this);
};

void TreeNode::SetBody(CompoundStmtNode* body){
    this->compBody = body;
};

void TreeNode::EmitNode(){
    if (compBody != NULL){
        compBody->EmitNode();
    }
};

void FunctionDefNode::SetBody(CompoundStmtNode* body){
    this->compBody = body;
}
void FunctionDefNode::EmitNode(){
    arguments->EmitNode();
    compBody->EmitNode();
};

void AssignNode::Visit(NodeVisitor* visitor){
    visitor->Visit_Assign(this);
};

void AssignNode::EmitNode(){
    assign_var->EmitNode();
    assign_value->EmitNode();
    EMIT(" OP_ASSIGN ");
}

void UnaryOpNode::EmitNode(){
    EMIT(unary_op);
    EMIT(" , ");
    EMIT(unary_node);
}

void UnaryOpNode::OpAdd(ParseNode* node){
    unary_node = node;
};

void UnaryOpNode::Visit(NodeVisitor* node){
    node->Visit_Unary(this);
};

void BinOpNode::Visit(NodeVisitor* visitor){
    visitor->Visit_Binary(this);
};

void BinOpNode::BinAdd(ParseNode* left, ParseNode* right){
    binary_left = left;
    binary_right = right;
};

void BinOpNode::EmitNode(){
    EMIT(" ( ");
    binary_left->EmitNode();
    EMIT(" , ");
    binary_right->EmitNode();
    EMIT(" , ");
    EMIT(binary_op); // 后面转化为str 利用map
    EMIT(" ) ");
}

void VariableNode::EmitNode(){
    EMIT(variable_name);
}

void VariableNode::Visit(NodeVisitor* visitor){
    visitor->Visit_Variable(this);
}

void NumberNode::EmitNode(){
    EMIT(value);
}

void NumberNode::Visit(NodeVisitor* visitor){
    visitor->Visit_Number(this);
}

void InitializerNode::EmitNode(){
    EMIT(" OP_INIT ");
    initial_value->EmitNode();
}
void ReturnNode::EmitNode(){
    EMIT(" OP_RET ");
    this->retExpr->EmitNode();
}

void ReturnNode::Visit(NodeVisitor* visitor){
    visitor->Visit_Return(this);
};

void InitializerNode::Visit(NodeVisitor* visitor){
    this->initial_value->Visit(visitor); // 自己child去实现
};

void FunctionCallNode::Visit(NodeVisitor* visitor){
    // for (auto list : this->argList->nodeList){
    //     list->Visit(visitor);
    // }
    visitor->visit_FunctionCall(this);
};

void FunctionCallNode::EmitNode(){
    EMIT(" OP_Call ");
}
