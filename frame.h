#pragma once
#include <iostream>
#include <iterator>
#include<string.h>
#include <list>
#include<vector>
#include "Ast.h"



#ifndef Frame_H
#define Frame_H
/*
    [label:] mnemonic [operands][ ;comment ]


    mnemonic:
        instruction mnemonic 如 move add
    operands:
        %rax %8 等

    comment:
        注释

    lea是“load effective address”的缩写，简单的说，lea指令可以用来将一个内存地址直接赋给目的操作数，
    例如：lea eax,[ebx+8]就是将ebx+8这个值直接赋给eax，而不是把ebx+8处的内存地址里的数据赋给eax。
    而mov指令则恰恰相反，例如：mov eax,[ebx+8]则是把内存地址为ebx+8处的数据赋给eax。


    格式:
    https://www.cnblogs.com/hdk1993/p/4820353.html

    AT&T格式 汇编
        pushl %eax   ==>  寄存器 要加%
        push $1      ==>  '$' 前缀表示一个立即操作数
        addl $1, %eax     [op src dst]
        movb val, %al     [op src dst]
        movl -4(%ebp), %eax
        movb $4, %fs:(%eax)

    Intel格式 汇编
        add eax, 1

*/

/*
    OperatorInstruction
    movb
    movl
    moveq
*/
class OperatorInstruction{
public:
    OperatorInstruction(std::string op):opName(op) {};
    std::string opName;
    std::string To_Str(RegValType v);
    std::string To_Str(RegValType v, RegValType right_type);
    std::string GetPostFix(RegValType v);
};



enum OperandType{
    BASE_NULL,
    Register,
    Memory,
    Constant,
};

class BaseOperand{
public:
    virtual OperandType GetOperandType(){
        return OperandType::BASE_NULL;
    };
    virtual std::string To_Str(RegValType val_type){
        return "";
    };
    virtual std::string To_Str(RegValType left_type, RegValType right_type){
        return "";
    };
    virtual void Negative(){
        // test
    };
};

/*
    RegisterOperand
    %rax
*/

enum RegisterState{
    READY, // 未工作共
    WORK,
    RELAX, // work 后
};

class RegisterOperand: public BaseOperand{
    /*
        从一个新的frame的角度来说
Note-1:
        使用的register有几种状态 针对Vec_AllReg的类型的寄存器 除了 rsp和rbp外的
        [1] 未使用 ==> Vec_RegFree 中
        [2] 使用中的 ==> 在FrameOperandStack中 (部分特殊寄存器会在Vec_RegCalleeOccupy中记录下)
        [3] 使用后被释放的 Vec_RegFreeCache 中

        外部使用寄存器存在两种方式 stackPush 和 stackPop

        stackPush 需要参数 如果是寄存器类型==> 比如所有的返回值都会是在eax寄存器中

        需要寄存器的时候优先从Vec_RegFree 获取, 直到获取完了
Note-2:
        比如寄存器A
        [1] stackPush调用GetFreeReg 得到A, A会从Vec_RegFree移出, 如果是callee有关的寄存器, 会保存在Vec_RegCalleeOccupy中
        [2] 被StackPop 后, A不会进入GetFreeReg, A会进入Vec_RegFreeCache, 表示被释放的寄存器
        由于按照运算 目前只支持2元寄存器运算 也就是说 同一指令最多使用2个寄存器 如 ExprNode只会产生一个值
        所以push一个寄存器后,

        [3] 如果 Vec_RegFree为空, 表示要么是都释放了 要么

Note-3:
        函数调用参数会使用寄存器可以参考==>CalcFunctionArgAddr
        这些寄存器会在在FrameOperandStack中 且不会pop 参数得一直保存在函数返回
        所以如果超过6个参数 那么正常能够使用的寄存器就只有RF_Rax, RF_Rbx, RF_Rcx, RF_Rdx 这四个了
        这四个寄存器基本就是交替使用

        按照二元运算的规则, Push超过2寄存器后一定会Pop 所以4个是够用的
        大致就是 函数内的变量 只存在3中
        [1] 参数 ==> register + memory
        [2] decl ==> memory
        [3] 临时 ==> reg 
    */
public:
    RegisterOperand() = default;
    RegisterOperand(std::string r): rName(r) {};
    std::string To_Str(RegValType v);
    std::string Reg_to_8();
    std::string Reg_to_32();
    std::string rName;
    RegisterState cur_state;
    OperandType GetOperandType(){
        return OperandType::Register;
    };
};

/*
    ConstantOperand
    立即数就是可以直接放在指令后面进行操作的整数
    立即数在指令中 执行的过程中无寻址操作
    如movl $8, %eax中的$8。同样地，我们也可以定义一个立即数类来管理
*/

class ConstantOperand : public BaseOperand{
public:
    ConstantOperand(std::string val): constant_value(val){};
    std::string constant_value;
    std::string To_Str(RegValType v);
    OperandType GetOperandType(){
        return OperandType::Constant;
    };

    void Negative();
};

class VariableAddrOperand : public BaseOperand{
    // 变量地址
    // 运行时候直接调用的

public:
    VariableAddrOperand(std::string val): constant_value(val){};
    std::string constant_value;
    std::string To_Str(RegValType v){
        return constant_value;
    }
    OperandType GetOperandType(){
        return OperandType::Constant;
    };

    // void Negative();
};

/*
    MemoryOperand
    4(%eax) 取eax的地址 + offset
*/
class MemoryOperand: public BaseOperand{
public:
    MemoryOperand(std::string r, int o): rName(r), offset(o) {};
    std::string To_Str(RegValType v);
    std::string rName;
    int offset;
    OperandType GetOperandType(){
        return OperandType::Memory;
    };
};


using VecMem = std::vector<MemoryOperand*>;

using VecReg = std::vector<RegisterOperand*>;

class FrameManager{
public:
    FrameManager(){
        next_temp = 0;
    }
    BaseOperand* StackPush(BaseOperand* reg, bool is_reg); // 碰到变量
    BaseOperand* StackPop();

    bool IsStackEmpty();
    void DoneRecover(); // 把缓存在cache的放进free
    void SetBaseFp(int fp);
    int GetMaxFp() {return this->next_temp + WORD_SIZE;};

    RegisterOperand* GetFreeReg(RegisterOperand* prefer);

    // protect
    RegisterOperand* CopyRegToTmp(RegisterOperand* validReg);
    void CallerProtocol();
    std::string CalleeProtocol(std::string op);
    void RemoveFreeReg(RegisterOperand* reg);
    bool IsTargetReg(BaseOperand* prefer, std::string s);

    const static int WORD_SIZE = 8;
    std::string frame_code;
    std::string frame_name;
    int next_temp; // 初始为0
    std::vector<BaseOperand*> FrameOperandStack;
    VecMem Memory_Free;
    FrameManager* parentFrame;

    VecReg Vec_RegCalleeOccupy;
};

/*
    为了减少寄存器的使用，防止可能存在的寄存器缺乏引起的数据迁移，我们会为每个用到的局部变量分配地址，
    这些地址具有相对于帧指针具体的偏移量。因此，可以直接使用帧指针的相对偏移量进行操作，减少了栈指针的移动。
    但是，由于汇编语言规定，操作指令的两个操作数不能同时为存储器。
    这时候，就需要将存储器的内容临时提取到寄存器，再进行操作。
    如果寄存器满负荷运转，则需要使用栈指针后（往低地址方向）的地址存储寄存器中的值，之后再恢复数据。
*/

extern std::string C_MOVE;
extern std::string C_ADD;
extern std::string C_SUB;
extern std::string C_IMUL;
extern std::string C_DIV;

extern std::string C_PUSH;
extern std::string C_POP;




extern std::string C_RAX;
extern std::string C_RBX;

extern std::string C_RCX;
extern std::string C_RDX;
extern std::string C_RSI;
extern std::string C_RDI;

extern std::string C_RBP;
extern std::string C_RSP;
extern std::string P_RSP;
extern std::string P_RBP;

extern std::string POINT_STR;

extern RegisterOperand* RF_Rax;
extern RegisterOperand* RF_Rbx;
extern RegisterOperand* RF_Rcx;
extern RegisterOperand* RF_Rdx;

extern RegisterOperand* RF_Rsi;
extern RegisterOperand* RF_Rdi;
extern RegisterOperand* RF_R8;
extern RegisterOperand* RF_R9;
extern RegisterOperand* RF_R10;
extern RegisterOperand* RF_R11;
extern RegisterOperand* RF_Rbp;
extern RegisterOperand* RF_Rsp;



extern OperatorInstruction* OP_Move;
extern OperatorInstruction* OP_Add;
extern OperatorInstruction* OP_Sub;
extern OperatorInstruction* OP_MUL;
extern OperatorInstruction* OP_DIV;
extern OperatorInstruction* OP_PUSH;
extern OperatorInstruction* OP_POP;



extern VecReg Vec_RegArg;
extern VecReg Vec_AllReg;
extern VecReg Vec_RegFree;

#endif
