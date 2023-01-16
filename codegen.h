#include <iostream>
#include <iterator>
#include<string.h>
#include "frame.h"
#include "visitor.h"

#define ADD_SPACE(str) str = str + " "
#define NEW_LINE(str) str = str + "\n"


class CodeGenVisitor: public NodeVisitor{
public:

    // override
    void Visit_Assign(AssignNode* node);
    void Visit_Binary(BinOpNode* node);
    void Visit_Variable(VariableNode* node);
    void Visit_FunctionDef(FunctionDefNode* node);
    void visit_FunctionCall(FunctionCallNode* node);
    void Visit_Number(NumberNode* node);
    void Visit_Unary(UnaryOpNode* node);
    void Visit_TreeNode(TreeNode* node);
    void Visit_Declaration(Declaration* node);


    // unique
    void OutPut(OperatorInstruction* op, BaseOperand* reg, RegValType reg_type);
    void OutPut(OperatorInstruction* op, BaseOperand* left, RegValType left_type, BaseOperand* right, RegValType right_type);
    void OutPut(OperatorInstruction* op, BaseOperand* left, RegValType left_type, BaseOperand* right);
    void OutPut(std::string str);

    int CalcFunctionVarAddr(FunctionDefNode* node, int lastFpLoc, Scope*);
    int  CalcFunctionArgAddr(FunctionDefNode* node);
    int CalcFunctionLocalVarAddr(CompoundStmtNode*, int, Scope*);

    void SetGlobalVariableCompile(Scope* s);
    BaseOperand*VisitAndPop(ExprNode*);

    std::string cur_str;

    // symbolVisit 使用scope来保存变量信息 decalNode 会设置 自身scope
    // codegen使用 frame来保存寄存器信息 decl和var Node 会设置 compile_location
    BaseOperand* FrameLookUpVariable(std::string name, Scope* sc);
    bool TryUseRegFrame(BaseOperand*);
    std::pair<BaseOperand*, BaseOperand*> TryPushRegToStack(BaseOperand*, bool);
    FrameManager* curFrame;
    void PushFrame();
    FrameManager* PopFrame();
    std::vector<FrameManager*> frameStack;
};

