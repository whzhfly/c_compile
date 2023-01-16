#include "parser.h"
#include "token.h"


// typedef enum {
//     TYPE_FUNCTION,
// //> Methods and Initializers initializer-type-enum
//     TYPE_INITIALIZER,
// //< Methods and Initializers initializer-type-enum
// //> Methods and Initializers method-type-enum
//     TYPE_METHOD,
// //< Methods and Initializers method-type-enum
//     TYPE_SCRIPT
// } FunctionType;


static ParseNode* parseLeftNode;


static void ResetLeft(ParseNode* node){
    parseLeftNode = node;
}

static ParseNode* GetLeftNode(){
    return parseLeftNode;
}

void ParseErrorToken(const char* message) {
    // errorAt(&parser.current, message);
    // todo
    std::cout<<"error token ==> "<< message << std::endl;
}

// TokenParse* machine = new TokenParse();

static const std::unordered_map<Token::TokenType, ParseRule> parseMap{

    {Token::TOKEN_LEFT_PAREN,  ParseRule{&TokenParse::Map_Group, &TokenParse::Map_ParseCall, NULL, Precedence::PREC_CALL}},

//> Classes and Instances table-dot
    {Token::TOKEN_DOT,  ParseRule{NULL, &TokenParse::Map_Dot, NULL, Precedence::PREC_CALL}},
//< Classes and Instances table-dot

//> ex
    {Token::TOKEN_MINUS,  ParseRule{&TokenParse::Map_Unary, &TokenParse::Map_Binary, NULL, Precedence::PREC_TERM}},
    {Token::TOKEN_BANG,  ParseRule{&TokenParse::Map_Unary, NULL, NULL, Precedence::PREC_NONE}},

    {Token::TOKEN_PLUS,  ParseRule{NULL, &TokenParse::Map_Binary, NULL, Precedence::PREC_TERM}},
    {Token::TOKEN_STAR,  ParseRule{NULL, &TokenParse::Map_Binary, NULL, Precedence::PREC_FACTOR}},
    {Token::TOKEN_SLASH,  ParseRule{NULL, &TokenParse::Map_Binary, NULL, Precedence::PREC_FACTOR}},

    {Token::TOKEN_BANG_EQUAL,  ParseRule{NULL, &TokenParse::Map_Binary, NULL, Precedence::PREC_EQUALITY}},
    {Token::TOKEN_EQUAL_EQUAL,  ParseRule{NULL, &TokenParse::Map_Binary, NULL, Precedence::PREC_EQUALITY}},

    {Token::TOKEN_GREATER,  ParseRule{NULL, &TokenParse::Map_Binary, NULL, Precedence::PREC_COMPARISON}},
    {Token::TOKEN_GREATER_EQUAL,  ParseRule{NULL, &TokenParse::Map_Binary, NULL, Precedence::PREC_COMPARISON}},
    {Token::TOKEN_LESS,  ParseRule{NULL, NULL, &TokenParse::Map_Binary, Precedence::PREC_COMPARISON}},
    {Token::TOKEN_LESS_EQUAL,  ParseRule{NULL, &TokenParse::Map_Binary, NULL, Precedence::PREC_COMPARISON}},
    // TOKEN_EQUAL 交给identity去实现了
//> ex


//> Identity
    {Token::TOKEN_IDENTIFIER,  ParseRule{&TokenParse::Map_Variable, NULL, NULL, Precedence::PREC_NONE}},
//< Identity

    {Token::TOKEN_STRING,  ParseRule{&TokenParse::Map_String, NULL, NULL, Precedence::PREC_NONE}},
    {Token::TOKEN_NUMBER,  ParseRule{&TokenParse::Map_Number, NULL, NULL, Precedence::PREC_NONE}},

    {Token::TOKEN_AND,  ParseRule{NULL, &TokenParse::Map_And, NULL, Precedence::PREC_AND}},
    {Token::TOKEN_OR,  ParseRule{NULL, &TokenParse::Map_Or, NULL, Precedence::PREC_OR}},

    {Token::TOKEN_FALSE,  ParseRule{&TokenParse::Map_Literal, NULL, NULL, Precedence::PREC_NONE}},
    {Token::TOKEN_TRUE,  ParseRule{&TokenParse::Map_Literal, NULL, NULL, Precedence::PREC_NONE}},
    {Token::TOKEN_NIL,  ParseRule{&TokenParse::Map_Literal, NULL, NULL, Precedence::PREC_NONE}},
};


ParseRule ParseRule::GetParseRule(Token::TokenType key) {
    auto kwIter = parseMap.find(key);
    if (parseMap.end() == kwIter)
        return ParseRule{NULL, NULL, NULL, Precedence::PREC_NONE}; // 后面可以改成唯一对象引用
    return kwIter->second;
}

/*
 * TokenParse 相关
*/

/*
 * 过程操作 大多是针对Token进行处理
 */

bool TokenParse::Check(Token::TokenType type) {
    return current->type == type;
}

void TokenParse::Advance() {
    // 指针前进一步
    previous = current;
    while (true)
    {
        if (current->type == Token::TOKEN_EOF) {
            break;
        };

        this->cur_++; // 防止最后
        current = *(this->cur_);
        if (current->type != Token::TOKEN_ERROR) 
            break;
        ParseErrorToken(current->start);
    }
}

bool TokenParse::Match(Token::TokenType type) {
    // 类型正确则前进一步
    if (!Check(type)) return false;

    Advance(); // 消耗当前token
    return true;
}

void TokenParse::Consume(Token::TokenType type, char* msg) {
    // 类型正确则前进一步
    if (!Check(type)) {
        ParseErrorToken(msg);
        return;
    }
    Advance(); // 消耗当前token
    return;
}

bool TokenParse::IsAtEnd() {
    if (this->current->type == Token::TOKEN_EOF){
        return true;
    }
    return false;
};

/*
 * Expression-Stack 表达式的控制
 */

void TokenParse::PushExpression(ParseNode* node){
    this->expressionStack.push_back(node);
}

ParseNode* TokenParse::PopExpression(){
    ParseNode* node = this->expressionStack.back();
    this->expressionStack.pop_back();
    return node;
}


/*
 * Expression-Map 函数解析
 */

ParseNode* TokenParse::Map_Group(bool canAssign) {
    // Group 解析
    // a+(c*1)
    ParseNode* exp =  ParseExpression();
    char* msg = (char*)"m";
    Consume(Token::TOKEN_RIGHT_PAREN, msg);
    return exp;
}

ParseNode* TokenParse::Map_ParseCall(bool canAssign) {
    if (canAssign){
        printf("\nf\n");
    }
    // uint8_t argCount = argumentList();
    // emitBytes(OP_CALL, argCount);
    // 把callNode 放进容器 callNode 包括解析 args 和 进行call函数操作。 c++ 还会存在 this_call这种
    // 一个整call 包括arg 直到 ) 结束。
    // Consume();

    StatementListNode* list = new StatementListNode();
    while (!Match(Token::TOKEN_RIGHT_PAREN))
    {
        ParseNode* exp =  ParseExpression();
        if (exp){
            list->AddNode(exp);
        }
        else {
            printf("!!!!!!!Map_ParseCall Error");
        }
        Match(Token::TOKEN_COMMA); // 消耗","
    }

    FunctionCallNode* call = new FunctionCallNode();
    call->argList = list;
    call->function = PopExpression(); // 可以后面去搞定分析?? 没有class 所以要么是point 要么是直接
    return call;
}

ParseNode*  TokenParse::Map_Dot(bool canAssign) {
    // Group 解析
    // a.c =1
    // a.c + c
    // a.c;
    // b + a.c + c 中的 a.c
    // a.c(bb) call
    Consume(Token::TOKEN_IDENTIFIER, (char*)"Expect property name after '.'.");
    std::string name = this->previous->str_;
    if (Match(Token::TOKEN_EQUAL)){
        //
    }
    else if(Match(Token::Token::TOKEN_LEFT_PAREN)){
        // method call
    }
    else {
        // 获取属性 值
        // valueNode
        // AttributeNode
    };
    ParseNode* no = new ParseNode();
    return no;
};

ParseNode* TokenParse::Map_Unary(bool canAssign) {
    /*
        -a
        +a
        !a
    */
    const Token* token = this->previous;
    UnaryOpNode* op = new UnaryOpNode();
    op->binary_op = token->type;

    ParseNode* nextNode = PrecedenceParse(Precedence::PREC_UNARY);
    op->OpAdd(nextNode);
    return op;
};

ParseNode* TokenParse::Map_Binary(bool canAssign) {


    BinOpNode* binOP = new BinOpNode(); // 有type
    binOP->binary_op = previous->type; // 设置bin type

    ParseRule rule = ParseRule::GetParseRule(previous->type);
    ParseNode* leftNode = PopExpression(); // 把旧的保存下来
    ParseNode* rightNode = PrecedenceParse(Precedence(rule.precedence + 1)); // 弹出新的node

//     switch (operatorType) {
//         case Token::TOKEN_BANG_EQUAL:    emitBytes(OP_EQUAL, OP_NOT); break;
//         case Token::TOKEN_EQUAL_EQUAL:   emitByte(OP_EQUAL); break;
//         case Token::TOKEN_GREATER:       emitByte(OP_GREATER); break;
//         case Token::TOKEN_GREATER_EQUAL: emitBytes(OP_LESS, OP_NOT); break;
//         case Token::TOKEN_LESS:          emitByte(OP_LESS); break;
//         case Token::TOKEN_LESS_EQUAL:    emitBytes(OP_GREATER, OP_NOT); break;
//         case Token::TOKEN_PLUS:          emitByte(OP_ADD); break;
//         case Token::TOKEN_MINUS:         emitByte(OP_SUBTRACT); break;
//         case Token::TOKEN_STAR:          emitByte(OP_MULTIPLY); break;
//         case Token::TOKEN_SLASH:         emitByte(OP_DIVIDE); break;
//         default: return; // Unreachable.
//   }

    binOP->BinAdd(leftNode, rightNode);
    return binOP;
}

ParseNode* TokenParse::Map_Variable(bool canAssign){
    // NamedVariable(parser.previous, canAssign); // 判断这个variable 来自那一层 可以后面判断
    VariableNode* var = new VariableNode();
    var->variable_name = (char*)this->previous->str_.data();
    if (Match(Token::TOKEN_EQUAL)){
        // 属于是 set 赋值了
        // AssignNode* assign = new AssignNode();
        // assign->var = var;
        // // assign->value = GetLeftNode(); // fuck 类型已经完全无法搞定了
        // ParseNode* right = ParseExpression();
        // // assign->value = PopExpression(); // 把旧的保存下来
        // return assign;
        // assign Expression
        AssignNode* assign = new AssignNode();
        assign->assign_var = var;
        assign->assign_value = ParseExpression();
        return assign;
    }
    return var;
};

ParseNode* TokenParse::Map_String(bool canAssign) {
    ParseNode* no = new ParseNode();
    return no;
}

ParseNode* TokenParse::Map_Number(bool canAssign) {
    // double value = strtod(this->previous->start, NULL);
    int value = atoi(this->previous->str_.c_str());
    NumberNode* node = new NumberNode(value);
    return node;
}

ParseNode* TokenParse::Map_And(bool canAssign) {
    // 理论上最好独立出来
    return Map_Binary(canAssign);
}

ParseNode* TokenParse::Map_Or(bool canAssign) {
    return Map_Binary(canAssign);
}
ParseNode* TokenParse::Map_Literal(bool canAssign) {
    LiteralNode* lit = new LiteralNode();
    lit->litType = this->previous->type;
    return lit;
}

/*
 * Expression
 */

ParseNode* TokenParse::ParseExpression() {
    return PrecedenceParse(Precedence::PREC_ASSIGNMENT);
}

ParseNode* TokenParse::PrecedenceParse(Precedence precedence) {
    /*
        (a.c + b) cur在 '('

        expression 都会返回一个值
        前缀会pushExpression 中缀会Pop 并处理
    */

    //ExprNode* leftNode = new ExprNode;
    Advance(); // 先前进前处理  cur 在 a

    // prefix handle
    ParseRule rule = ParseRule::GetParseRule(previous->type);
    if (rule.prefix == NULL && rule.postfix == NULL)
        return NULL;
    bool canAssign = precedence <= Precedence::PREC_ASSIGNMENT;

    // 前缀分析 一定push一个值
    if (rule.prefix != NULL){
        ParseNode* leftNode = (this->*rule.prefix)(canAssign);
        // ResetLeft(leftNode);
        PushExpression(leftNode);
    }

    // if (rule.postfix != NULL){
    //     (this->*rule.postfix)(canAssign); // 确实恶心啊 这种函数指针this call的方式
    // }

    // 中缀 token 会进行计算
    while(precedence <= ParseRule::GetParseRule(current->type).precedence)
    {
        Advance(); // cur 在 '.' 后变成 '+' 即使是中缀 也会再前进一格 保证消耗当前token
        ParseRule newRule = ParseRule::GetParseRule(previous->type);
        ParseNode* inNode = (this->*newRule.infix)(canAssign); // 中缀判断 这里就不是前缀了 前缀就是prefix操作的
        // ResetLeft(inNode);
        PushExpression(inNode);
    }

    if (canAssign && Match(Token::TOKEN_EQUAL)) {
        ParseErrorToken("error");
    }
    // return GetLeftNode(); // 永远返回后缀的最后一个 也就是left
    return PopExpression();
}

/*
 * Statements
 */

bool CheckDeclType(const Token* tk){
    std::string INT = "int";
    std::string CHAR = "char";
    std::string VOID = "void";
    if (tk->str_ == INT || tk->str_ == CHAR){
        return true;
    }
    return false;
};

CompoundStmtNode* TokenParse::ParseCompoundStmt(){
    /*
        compound_statement : { declaration_list }
                            | { declaration_list statement_list }
        解析 {} 中的内容 以 '{' 开始 结束于'}' 有时候结束于结束符 但是需要匹配下 { 和结束符匹配了
    */
    // EnterScope() // 还需要进入 Scope 么？
    bool brace = false;
    if (this->previous->type == Token::TOKEN_LEFT_BRACE){
        brace = true;
    }
    CompoundStmtNode* comp = new CompoundStmtNode();
    StatementListNode* stmtList = new StatementListNode();
    DeclarationListNode* declList = new DeclarationListNode();
    while (true){
        while (CheckDeclType(this->current)){
            DeclarationListNode* decls = ParseDeclare();
            if (decls== NULL) break;
            auto begin_ = decls->nodeList.begin();
            auto end_ = decls->nodeList.end();
            for(auto iter = begin_; iter!=end_; iter++){
                declList->AddNode(*iter);
            }
            if (IsAtEnd()) break;
        }
        if (Match(Token::TOKEN_RIGHT_BRACE) || IsAtEnd()) {
            break;
        }
        StatementNode* stmt = ParseStatement();
        Match(Token::TOKEN_SEMICOLON);
        if (stmt !=NULL){
            stmtList->AddNode(stmt);
        }
    }
    comp->declarations = declList;
    comp->statements = stmtList;
    if (this->previous->type == Token::TOKEN_EOF && brace){
        // 从 '{' 进来 却没有 '}' 退出
        ParseErrorToken("miss '}' in end");
    }
    return comp;
}

StatementNode* TokenParse::ParseStatement() {
    /*
        statement : expression_statement
                    | jump_statement
                    | iteration_statement
                    | selection_statement
                    | compound_statement
                    | empty_statement
    */
    StatementNode* node;
    if (Match(Token::TOKEN_PRINT)){
        node = new PrintNode();
    }
    else if (Match(Token::TOKEN_FOR)) {
        node = new ForNode();
    }
    else if (Match(Token::TOKEN_RETURN)) {
        ReturnNode* ret = new ReturnNode();
        ret->retExpr = ParseExpression();
        Consume(Token::TOKEN_SEMICOLON, (char*)"return need ;");
        return ret;
    }
    else{
        node = ParseExpression();
    }
    return node;
}

/*
 * Declare
 */


void Def_Initializer(){
    /*
        initializer : expression
                    | { initializer_list }
        基本就是expression多个a =  {ex1,ex2} 或者a = ex, 也会出现这种 Struct As a{"ex", "ex"};属于初始化这种
    */
}

InitializerNode* TokenParse::Def_InitializerList (){
    /*
        initializer_list : initializer
                        | initializer_list, initializer
    */

    InitializerNode* node = new InitializerNode();
    while (true)
    {
        // todo
        if (Check(Token::Token::TOKEN_SEMICOLON)){
            break;
        }
        ParseNode* expr = ParseExpression();
        node->initial_value = expr;
    };
    return node;
}

void TokenParse::Decl_StrcutDef(){
    /*
        生成struct内部内容等

    */
}

FunctionDefNode* TokenParse::Decl_FunctionDef(){
    /*
        生成Function内部内容等
    */

    FunctionDefNode* node = new FunctionDefNode();
    CompoundStmtNode* comp = ParseCompoundStmt();
    node->SetBody(comp); // 想这种的 明确body只有一种 那就是CompoundStmtNode？ 要不然怎么搞
    return node;
}
// todo 改成直接返回token即可
const char* TokenParse::VarDeclaration(){
    const Token* to = this->current;
    if (Match(Token::TOKEN_IDENTIFIER)){
        const char* cc = to->str_.data();
        return cc;
    }
    Advance();
    return (char*)to->str_.data();
};

Type* TokenParse::Decl_TypeSpecific() {
    // type_spec : INT
    //           | CHAR
    //           | STRUCT
    Type* ty = new IntTypeNode;
    std::string valueType = this->current->str_;
    if (valueType == "int"){
        ty->SetDefineType(RegValType::INT, 4);
    }
    else if(valueType == "int"){
        ty->SetDefineType(RegValType::CHAR, 1);
    }
    else {
        ty->SetDefineType(RegValType::POINT, 8);
    }
    Advance();
    return ty;
};

Declaration* TokenParse::Decl_Declarator(){
    /*
    declarator : * direct_declarator
                    |direct_declarator
    */
    Declaration* node;
    // 指针类型判读
    if (Match(Token::Token::TOKEN_STAR)){
        // 指针类型
        node  = Decl_Declarator(); // 递归去实现
        PointTypeNode* p = new PointTypeNode();
        node->SetType(p); // 最后设置type
    }
    else {
        node =  Decl_Direct_Declarator();
    }

    return node;
};

Declaration* TokenParse::Decl_Direct_Declarator(){
    /*direct_declarator : ID
                        | direct_declarator [ const_expr ]
                        | direct_declarator [ ]
                        | direct_declarator ( )
                        | direct_declarator ( parameter_type_list )
                        | direct_declarator = initializer  这也是define, define 和 declare是嵌在一起的
    */
    // 真正的 new Declaration 生产对象
    // Declaration* node = new Declaration(self.variable());
    const char* varName = VarDeclaration();
    Declaration* node = new Declaration();
    node->varName = varName; // 设置name
    if (Match(Token::TOKEN_LEFT_PAREN)){
        // function type
        // function_definition : type_specifier declarator compound_statement
        FuncTypeNode* fun = new FuncTypeNode(); // 后续用函数去生生参数列表等信息
        node->AddType(fun);
        // 可以先从Declaration 复制到 FunctionDefNode;
        // 函数负责生成func_node 并内嵌 的内嵌信息
        // node = func_node 替换node
        // 先不用 参数
        DeclarationListNode* argDecls = new DeclarationListNode();
        while (CheckDeclType(this->current)){
            Type* argType = Decl_TypeSpecific();
            Declaration* node = new Declaration();
            const char* varName = VarDeclaration();
            node->varName = varName; // 设置name
            node->AddType(argType);
            argDecls->AddNode(node);
            Match(Token::TOKEN_COMMA); // 消耗 ;
        }
        Match(Token::TOKEN_RIGHT_PAREN);

        Consume(Token::TOKEN_LEFT_BRACE, (char*)"ss1");

        FunctionDefNode* funcNode = Decl_FunctionDef();
        funcNode->arguments = argDecls; // setArg
        funcNode->functionName = node->varName;
        node->function = funcNode;
        // todo 实现子类copy父类的构造函数
    }
    else if (Match(Token::TOKEN_LEFT_SQ_BRACE)){
        //  int a[] 等 属于数组操作
        // node->AddType(Array);
    }
    else if (Match(Token::TOKEN_EQUAL)){
        // Define判断
        // 初始化 函数的初始化就是 {}
        // 内置类型的初始化 就是 = 开头
        // Initializer
        // declare node 带 Initializer 的
        // 不过有些自动初始化了 如 int a;或者指针  一般有默认值
        node->init_node = Def_InitializerList(); // 返回的是 binary node
        // 再 set 一下
    }
    return node;

};


VariableListNode* TokenParse::Declarator_List(){
    // declarator_list : declarator
    //                  | declarator_list, declarator
    VariableListNode* nodeList = new VariableListNode;
    // 在这里检查type
    while (!TokenParse::Check(Token::TOKEN_SEMICOLON)) // todo match改成 check
    {
        if (TokenParse::Check(Token::TOKEN_RIGHT_BRACE)){
            return nodeList;
        }
        // 有时候不是以分号结束 或者之前消耗了分号
        // 如何如处理？？
        // todo
        Declaration* childNode = Decl_Declarator();
        // 理应判读下childNode 的存在性 后面也要做保证
        nodeList->AddNode(childNode);
        if (Check(Token::TOKEN_SEMICOLON) || IsAtEnd()){
            break;
        }
        Match(Token::TOKEN_COMMA); // 逗号处理 在函数参数中不能有这个
    };
    return nodeList;
};


DeclarationListNode* TokenParse::ParseDeclare() {
    /*
        declarations :   type_specifier declarator_list SEMI
                        | type_specifier declarator_list

        除了几个基本类型外就是statement了
        int a=1, b;
        struct myStruct;
        struct myStruct a; 等等
        b = c 就是statement 所以申明的都是以类型开始 可以是自己定义的类型 也可以是内置类型
    */
    // 需要一个获取当前identity 并返回类型的处理函数
    // 类型包括 自定义类型如 struct A  以及定义好的类型 int b;

    DeclarationListNode* declaration = new DeclarationListNode;
    Type* tyNode = Decl_TypeSpecific();
    if (tyNode != NULL){
        VariableListNode* declaratorList = Declarator_List();
        auto begin_ = declaratorList->nodeList.begin();
        auto end_ = declaratorList->nodeList.end();
        for (auto iter = begin_; iter != end_; ++iter){
            // 这里表示了VariableListNode 和 DeclarationListNode的区别
            // DeclarationListNode的区别的元素会多SetType 一次
            (*iter)->SetType(tyNode);
            declaration->AddNode(*iter);
        };
        Consume(Token::TOKEN_SEMICOLON, (char*)"st"); //
        return declaration;
    }
    return NULL;
    // 理论上不应该有statement
    // 因为是check过Type进来的
}

DeclarationListNode*  TokenParse::ParseArgument(){
    return NULL;
};


/*
 * ParseEntry
 */

TreeNode* TokenParse::TokenToTree() {
    // 如何把tokens 转化为ast 树
    TreeNode* file = new TreeNode();
    while (!IsAtEnd()) {
        CompoundStmtNode* comp = ParseCompoundStmt();
        file->SetBody(comp);
        file->EmitNode();
        return file;
    }
    return file;
};

