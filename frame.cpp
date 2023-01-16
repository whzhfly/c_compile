#include "frame.h"




/*
    A_R == > argument register
*/

std::string C_RAX = "%rax"; // 返回值等
std::string C_RBX = "%rbx"; // miscellaneous register

std::string C_RCX = "%rcx";  // fourth A_R
std::string C_RDX = "%rdx"; // third A_R


std::string C_RSI = "%rsi"; // second A_R
std::string C_RDI = "%rdi"; // first A_R


std::string C_R8 = "%r8"; // fifth A_R
std::string C_R9 = "%r9"; // sixth A_R
std::string C_R10 = "%r10"; // miscellaneous register
std::string C_R11 = "%r11"; // miscellaneous register

std::string C_RBP = "%rbp"; // frame point 应该是 block——point frame 帧
std::string C_RSP = "%rsp"; // stack point 栈


std::string P_RSP = "(%rsp)"; // stack point 栈
std::string P_RBP = "(%rbp)"; // frame point

std::string POINT_STR = "point"; // 可以用enum去搞定



std::string C_MOVE = "move";
std::string C_ADD = "add";
std::string C_SUB = "sub";
std::string C_IMUL = "imul";
std::string C_DIV = "idiv";

std::string C_PUSH = "push";
std::string C_POP = "pop";



/*
    抽象寄存器
    x86下的 esp(32位)指 stack 指针 => 64 位中就是 rsp(64 位) 一般16位就叫sp 根据位数不同来区分
*/


RegisterOperand* RF_Rax = new RegisterOperand(C_RAX);
RegisterOperand* RF_Rbx = new RegisterOperand(C_RBX);
RegisterOperand* RF_Rcx = new RegisterOperand(C_RCX);
RegisterOperand* RF_Rdx = new RegisterOperand(C_RDX);

RegisterOperand* RF_Rsi = new RegisterOperand(C_RSI);
RegisterOperand* RF_Rdi = new RegisterOperand(C_RDI);

RegisterOperand* RF_R8 = new RegisterOperand(C_R8);
RegisterOperand* RF_R9 = new RegisterOperand(C_R9);
RegisterOperand* RF_R10 = new RegisterOperand(C_R10);
RegisterOperand* RF_R11 = new RegisterOperand(C_R11);

RegisterOperand* RF_Rbp = new RegisterOperand(C_RBP);
RegisterOperand* RF_Rsp = new RegisterOperand(C_RSP);




OperatorInstruction* OP_Move = new OperatorInstruction(C_MOVE);
OperatorInstruction* OP_Add = new OperatorInstruction(C_ADD);
OperatorInstruction* OP_Sub = new OperatorInstruction(C_SUB);
OperatorInstruction* OP_MUL = new OperatorInstruction(C_IMUL);
OperatorInstruction* OP_DIV = new OperatorInstruction(C_DIV);


OperatorInstruction* OP_PUSH = new OperatorInstruction(C_PUSH);
OperatorInstruction* OP_POP = new OperatorInstruction(C_POP);






/*
    需要一个管理寄存器的manager
    用来提供寄存器的设置和恢复

    b 中调用 a 在a 函数中 a.caller = b
    callee返回正在执行的函数本身的引用 如: arguments.callee.length 返回当前形参个数。指当前函数

    caller 上一个调用函数 调用者 开启调用后 要保存他的寄存器的值  pop 使用集合来保存
    callee 当前函数 当前调用对象 结束调用后 要恢复调用者的寄存器内容 push 

    https://zhuanlan.zhihu.com/p/107455887 详看这个
*/

/*
    调用Function 参考python虚拟机
    主要是要reset rbp 和 rsp

    函数A 调动函数B
    stack []

    先 push 当前A函数的 rbp 到stack
    stack [A_rbp] 此时 rsp + 1
    再把当前地址 标记为b_rbp 设置到 rbp中
    stack [a_rbp ,b_rbp] 
    然后加入参数 
    stack [a_rbp ,b_rbp, xxxxxxxxxxxxxx] 

    结束时候 取除rbp 当前 stack 指向a_rbp 并把rbp 重置为 a_rbp 即可

    利用了stack来保存caller的rbp 并在 return后 恢复rbp 此时的rsp 也是恢复了之前位置 并把ret 赋值给了 rax

    这就是call  函数的 整个流程。
    当做表达式来看  经过一系列操作  其结果保存在了 rax 中 

    op1...
    op_call ==> 类似一个虚拟机器指令 只不过拓展了成了n个汇编指令
*/


/*
    callee 被调用者  caller调用者
    rax	累加器，是算术运算的主要寄存器
    rbx	基址寄存器，被调用者保存 存放存储区的起始地址
    rdx	I/O指针
    r10~r15	被调用者保存
    %rbx，%rbp，%r12，%r13，%14，%15 用作数据存储，遵循被调用者(callee)使用规则，调用子函数之前要备份(push?)它，以防他被修改
    %r10，%r11 用作数据存储，遵循调用者(caller)使用规则，简单说就是使用之前要先保存原值

    函数执行前后必须保持原始的寄存器有3个：是rbx、rbp、rsp。rx寄存器中，
    最后4个必须保持原值：r12、r13、r14、r15。保持原值的意义是为了让当前函数有可信任的寄存器，
    减小在函数调用过程中的保存&恢复操作。除了rbp、rsp用于特定用途外，其余5个寄存器可随意使用。
*/



// 有些应该定义到Frame内部去 每个Frame使用的不同

using VecReg = std::vector<RegisterOperand*>;

VecReg Vec_RegCalleeSave{RF_Rbx, RF_Rbp};
VecReg Vec_RegCallerSave{RF_R10, RF_R11}; // 保存作用

VecReg Vec_AllReg{RF_Rax, RF_Rbx, RF_Rcx, RF_Rdx, RF_Rsi, RF_Rdi, RF_R8, RF_R9, RF_R10, RF_R11};
VecReg Vec_RegFree(Vec_AllReg); // 初始化所有Reg
VecReg Vec_RegFreeCache;

VecReg Vec_RegArg{RF_Rdi, RF_Rsi, RF_Rdx, RF_Rcx, RF_R8, RF_R9}; // 参数 1-6 个


bool is_base_reg(std::string cs){
    if ( cs == C_RAX || cs == C_RBX || cs == C_RCX|| cs == C_RDX){
        return true;
    }
    return false;
}

bool is_index_reg(std::string cs){
    if ( cs == C_RSI || cs == C_RDI){
        return true;
    }
    return false;
}

bool is_extend_reg(std::string cs){
    if ( cs == C_R8 || cs == C_R9 || cs == C_R10|| cs == C_R11){
        return true;
    }
    return false;
}

bool is_stack_reg(std::string cs){
    if (cs == C_RBP || cs == C_RSP){
        return true;
    }
    return false;
}


void FrameError(std::string s){
    std::cout<<s<<std::endl;
}

/*
 * Vector Function
 */


bool regCheck_in_vec(std::vector<RegisterOperand*>& vec, RegisterOperand* reg){
    // 判断是否在vec中
    for (auto item : vec){
        if (item == reg){
            return true;
        }
    }
    return false;
}

bool vec_remove_reg(std::vector<RegisterOperand*>& vec, RegisterOperand* reg){
    // vec remove
    for(size_t  i=0; i<vec.size();i++){
        if(vec[i] == reg){
            vec.erase(vec.begin() + i);
            break;
        }
    }
    return true;
}

bool vec_add_reg(std::vector<RegisterOperand*>& vec, RegisterOperand* reg){
    // vec add
    vec.push_back(reg);
    return true;
}

bool vec_add_memory(std::vector<MemoryOperand*>& vec, MemoryOperand* reg){
    // vec add
    vec.push_back(reg);
    return true;
}

void vec_extend(std::vector<RegisterOperand*> src, std::vector<RegisterOperand*> vec){
    auto begin_ = vec.begin(); // R10 和 R11
    auto end_ = vec.end();
    for(auto iter = begin_; iter!=end_; iter++){
        if (!regCheck_in_vec(src, *iter)){
            src.push_back(*iter);
        }
    }
}


/*
 * ConstantOperand
 */
std::string ConstantOperand::To_Str(RegValType type_str){
    return "$"+ constant_value;
}


void ConstantOperand::Negative(){
    if (constant_value.size() > 0){
        if(constant_value[0] == '-'){
            // todo 去除第一个
            // constant_value = constant_value;
        }
        else {
            constant_value = "-" + constant_value;
        }
    }
}
/*
 * RegisterOperand
 */

std::string RegisterOperand::To_Str(RegValType type_str){
        // 根据类型获取真正的寄存器
    std::string output_str;
    if (type_str == RegValType::CHAR){
        output_str = Reg_to_8();
    }
    else if (type_str == RegValType::INT)
    {
        output_str = Reg_to_32();
    }
    else {
        // pointer
        output_str = rName;
    }

    if (output_str[0] != '%'){
        std::string mod = "%";
        output_str = mod + output_str;
    }
    return output_str;
}

std::string RegisterOperand::Reg_to_8(){
    // todo
    std::string res = "";
    // char 8位的寄存器
    if (is_base_reg(this->rName)){
        // %rax ==> al
    }
    else if (is_index_reg(this->rName)){
        // %rsi => sil
        std::string pos_fix = "il";
        char pre_fix = this->rName[2];
        res = std::to_string(pre_fix) + pos_fix;
    }
    else if (is_extend_reg(this->rName)){
        // %r8 ==> r8b
    }
    else{
        // error
    }
    return this->rName;
}

std::string RegisterOperand::Reg_to_32(){
    // 32位
    // %rax ==> %eax
    // %ras ==> %esp

    // '%rax', '%rbx', '%rcx', '%rdx' 操作
    // %rsi', '%rdi' 操作
    // %r8', '%r9', '%r10', '%r11' 操作
    // '%rbp', '%rsp' 操作
    // todo
    return this->rName;
}

/*
 * OperatorInstruction
 */

std::string OperatorInstruction::To_Str(RegValType type_str){
    std::string post = GetPostFix(type_str);
    std::string output_str = this->opName + post;
    return output_str;
}

std::string OperatorInstruction::To_Str(RegValType type_str, RegValType right_type){
    // todo
    std::string post = GetPostFix(type_str);
    std::string output_str = this->opName + post;
    return output_str;
}

std::string OperatorInstruction::GetPostFix(RegValType v){
    /*
    POST_FIX OP 后缀
    char': 'b
    'int': 'l'
    'pointer': 'q'
    */
    if (v == RegValType::CHAR){
        return "b";
    }
    else if(v == RegValType::INT){
        return "l";
    }
    if (v != RegValType::POINT){
        FrameError("OP的后缀错误: ");
    }
    return "q"; // point-type
}

/*
 * MemoryOperand
 */

std::string MemoryOperand::To_Str(RegValType type_str){
    if (this->offset == 0){
        return this->rName;
    }
    // offset  + rname
    std::string of_str = std::to_string(this->offset);
    return of_str + this->rName;
}

/*
 * FrameManager
 */

bool FrameManager::IsStackEmpty(){
    if (FrameOperandStack.size() > 0){
        return false;
    }
    return true;
}

void FrameManager::DoneRecover(){
    vec_extend(Vec_RegFree, Vec_RegFreeCache);
    Vec_RegFreeCache.clear();
}

void FrameManager::SetBaseFp(int fp){
    this->next_temp = fp - WORD_SIZE;
}

BaseOperand* FrameManager::StackPush(BaseOperand* preferred_reg, bool regCheck){
    /*
        regCheck 首先得是寄存器
        而且 如果push是要改变寄存器值的 需要check一下
        如果只是取值 则不需要
    */
    BaseOperand* ret;
    if (regCheck){
        ret = GetFreeReg((RegisterOperand*)preferred_reg);
    }
    else {
        ret = preferred_reg;
    }
    FrameOperandStack.push_back(ret); // 这里stack push
    return ret;
}

BaseOperand* FrameManager::StackPop(){
    if (IsStackEmpty()){
        FrameError("Frame Stack Empty Pop");
        return NULL;
    }

    BaseOperand* pop_loc = FrameOperandStack.back();
    FrameOperandStack.pop_back();
    if (pop_loc->GetOperandType()==OperandType::Register){
        // 记住转换成reg指针
        if (regCheck_in_vec(Vec_AllReg, (RegisterOperand*)pop_loc)){
            vec_add_reg(Vec_RegFreeCache, (RegisterOperand*)pop_loc);
        }
    }
    return pop_loc;
}

bool FrameManager::IsTargetReg(BaseOperand* prefer, std::string tp){
    if (prefer->GetOperandType() == OperandType::Register){
        return ((RegisterOperand*)prefer)->rName == tp;
    }
    return false;
}


bool CheckRegisterInFree(RegisterOperand* tarReg){
    if (Vec_RegFree.size() > 0 ){
        if (tarReg && regCheck_in_vec(Vec_RegFree, tarReg)){
            return true;
        }
    }
    return false;
}

void GetValidReg(){
}

RegisterOperand* FrameManager::GetFreeReg(RegisterOperand* prefer){
/*
    如何获取一个reg
    如果Vec_RegFree有的话:
        [1]首先从Vec_RegFree 中获取一个 f-reg, 这个f-reg可能是你想要的如 rdx也可能是 r11这种
        [2]把f-reg 从 Vec_RegFree 中移除
        [3] 检查f-reg 是否是caller-protocol 通过 RemoveFreeReg 其实就是 R10和R11 需要保存一下caller的值
        [4]直接返回f-reg
    如果Vec_RegFree没有有的话:
        寄存器毕竟是有限的 这个时候就要用stack去动态扩展寄存器
        [1]先从cache中找到一个寄存器 a, push到stack 
        [2]此时可以使用a
        [3]使用后 pop a 恢复寄存器的值
        [4]类似函数call之前需要保存一些寄存器的概念

        [5]当然上述就是程序运行时候 我们编译器要做的就是明确什么时候push 和 pop 我们用一个单独的memory_stack 去搞定
*/
    if (Vec_RegFree.size() > 0 ){
        RegisterOperand* reg;
        if (prefer && regCheck_in_vec(Vec_RegFree, prefer)){
            reg  = prefer;
        }
        else {
            // 选择最后一个
            reg = Vec_RegFree.front();
        };
        if (reg){
            RemoveFreeReg(reg);
            return reg; // 获取前 在stack中记录下
        }
    }
    return CopyRegToTmp(Vec_RegFreeCache.front()); // Vec_RegFreeCache 一定有? why
}

RegisterOperand* FrameManager::CopyRegToTmp(RegisterOperand* validReg){
    /*
        应该是一些reg 要进入 stack 中 毕竟reg数量不够？？ 真的会不够么
        对于调用者负责保存的寄存器而言，我们会将正在使用的寄存器通过copy_reg_to_temp函数将对应的内容先保存到临时地址上
        然后再让寄存器变成可以使用的状态。
    */
    // 保证初始化
    if (Memory_Free.size() == 0){
        MemoryOperand* mem = new MemoryOperand(P_RSP, this->next_temp); // 固定使用当前栈顶创建
        vec_add_memory(Memory_Free, mem);
        this->next_temp = this->next_temp - WORD_SIZE;
    }

    MemoryOperand* popMem = Memory_Free.back();
    Memory_Free.pop_back();

    bool find_reg = false;

    // 比如 我们需要寄存器%rsi 保存一个值a
    // 然后函数分析运行到一半
    // 需要寄存器
    // 我们可以在stack中指明一个地址作为fake_reg ,用当前的C_RSP 和 next_temp 作为该寄存器地址
    // 后面 就使用这个地址去作为临时寄存器处理 把a 的值放在这个fake_reg 中
    // 后续就直接从这里拿?

    //如果我们无法得到一个可用的寄存器，就会将已经使用的某个寄存器中的值放进临时区域，并替换栈中对应的寄存器
    //这里，我们使用next_temp来偏移得到存储寄存器中临时变量的地址，并保存到Memory_Free。
    // 操作数管理器


    // 减少寄存器的使用 直接使用帧指针的相对偏移量进行操作，
    // 为每个用到的局部变量分配地址 具有相对于帧指针具体的偏移量
    // 以直接使用帧指针的相对偏移量进行操作，减少了栈指针的移动
    // 由于汇编语言规定，操作指令的两个操作数不能同时为存储器 如 move (%rcx) (%rdx)这种 都是间接
    // 需要将存储器的内容临时提取到寄存器 move %rcx (%rdx)
    // 如果寄存器满负荷运转
    // 则需要使用栈指针后（往低地址方向）的地址存储寄存器中的值，之后再恢复数据

    /*
        int a = 1; // a 存进了%rsi中  值为1
        ...

        ... 这里用了%rsi 寄存器 我们需要把 a 放在临时地址上
        ...
        b = a + 1; 这里要用a 直接使用临时地址的a 也就是offset 
    */


    // FrameOperandStack 一个寄存器应该只存在一次
    // StackPush 才会进入 FrameOperandStack
    // 明确StackPush 和 FrameOperandStack的作用

    auto begin_ = FrameOperandStack.begin(); // R10 和 R11
    auto end_ = FrameOperandStack.end();
    for(auto iter = begin_; iter!=end_; iter++){
        if (*iter == validReg){ // 找到上一个用该寄存器寄存器%rsi的 frame 比如上一个函数用了这个%rsi 保存了一个值等后续使用之类
            *iter = popMem; // 把那块%rsi的内容换成popMem
            find_reg = true; // 找到了这块内容 后续替换即可。
            break;
        }
    }
    if (!find_reg){
        // error
        FrameError("No free registers inside OR outside of stack!");
    }
    return validReg; // %rsi寄存器可用了 原来的值保存在了popMem 中 而这个popMem 又替换了原来在stack中的%rsi
}

void FrameManager::CallerProtocol(){
    // caller 协议
    // R10 和 R11 两个寄存器
    // 使用 Vec_RegCallerSave 保存
    auto begin_ = Vec_RegCallerSave.begin(); // R10 和 R11
    auto end_ = Vec_RegCallerSave.end(); 
    for(auto iter = begin_; iter!=end_; iter++){
        if (!regCheck_in_vec(Vec_RegFree, *iter)){
            CopyRegToTmp(*iter); //push
            vec_add_reg(Vec_RegFree, *iter);
        }
    }
}

std::string FrameManager::CalleeProtocol(std::string op){
    /*
        callee 协议
        %rbx，%rbp，%r12，%r13，%14，%15 中用过的

        把上一个 caller 的 Vec_RegCalleeOccupy 的保存起来
        for reg in self.callee_save_regs_used:
            return f"push {reg.to_str('pointer')}"

        或者是 callee 的 Vec_RegCalleeOccupy 清楚
        restore
        for reg in self.callee_save_regs_used:
            return f"pop {reg.to_str('pointer')}
        op = push |pop
    */
    auto begin_ = Vec_RegCalleeOccupy.begin(); // R10 和 R11
    auto end_ = Vec_RegCalleeOccupy.end();
    std::string ret = "";
    for(auto iter = begin_; iter!=end_; iter++){
        std::string c_reg = (*iter)->To_Str(RegValType::POINT);
        c_reg = op + c_reg;
        ret += c_reg + "\n";
    };
    return ret;
}

void FrameManager::RemoveFreeReg(RegisterOperand* reg){
    // 此时，如果我们调用函数，通过在前后调用save_callee_saves和restore_callee_saves
    //就能实现寄存器被被调用者保存和恢复的功能。 通过 CalleeProtocol 的op pop 进行恢复
    vec_remove_reg(Vec_RegFree, reg); // free remove

    if (regCheck_in_vec(Vec_RegCalleeSave, reg) && !regCheck_in_vec(Vec_RegCalleeOccupy, reg)){
        vec_add_reg(Vec_RegCalleeOccupy, reg);
    }
};
