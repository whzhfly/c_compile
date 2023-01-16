#include "codegen.h"
#include <string>
#include <vector>


//fp == > frame point


void CodeGenVisitor::OutPut(OperatorInstruction* op, BaseOperand* reg, RegValType val_type){
    std::string cur_code = "";
    cur_code = cur_code + op->To_Str(val_type);
    ADD_SPACE(cur_code);
    cur_code = cur_code + reg->To_Str(val_type);
    NEW_LINE(cur_code);

    this->cur_str = this->cur_str + cur_code; // add to source
};

void CodeGenVisitor::OutPut(OperatorInstruction* op, BaseOperand* left, RegValType left_type, BaseOperand* right){
    std::string cur_code = "";
    cur_code = cur_code + op->To_Str(left_type);
    ADD_SPACE(cur_code);
    cur_code = cur_code + left->To_Str(left_type);
    ADD_SPACE(cur_code);
    cur_code = cur_code + right->To_Str(left_type);
    NEW_LINE(cur_code);

    this->cur_str = this->cur_str + cur_code; // add to source
};

void CodeGenVisitor::OutPut(OperatorInstruction* op, BaseOperand* left, RegValType left_type, BaseOperand* right, RegValType right_type){
    std::string cur_code = "";
    cur_code = cur_code + op->To_Str(left_type, right_type);
    ADD_SPACE(cur_code);
    cur_code = cur_code + left->To_Str(left_type);
    ADD_SPACE(cur_code);
    cur_code = cur_code + right->To_Str(right_type);
    NEW_LINE(cur_code);
    this->cur_str = this->cur_str + cur_code; // add to source
};

void CodeGenVisitor::OutPut(std::string str){
    this->cur_str = this->cur_str + str; // add to source
};

void CodeGenVisitor::PushFrame(){
    FrameManager* frame = new FrameManager();
    if (this->curFrame){
        frame->parentFrame = this->curFrame; //是否有必要？？ todo
    }
    this->frameStack.push_back(frame);
    this->curFrame = frame;
}

FrameManager* CodeGenVisitor::PopFrame(){
    if (frameStack.empty()){
        return NULL;
    }
    FrameManager* frame = frameStack.back();
    this->frameStack.pop_back();
    if (frameStack.empty()){
        this->curFrame = NULL;
        return frame;
    }
    this->curFrame = frameStack.back();
    return frame;
}



void CodeGenVisitor::Visit_FunctionDef(FunctionDefNode* node){

    /*
        这是得出函数内部的汇编代码;
        如 foo(a, 2);
            对于一个函数定义来说
            涉及到的内存如下:
            [1] argument-decl 内存 ==> 以及保存在了寄存器或者stack中
            [2] variable-decl 内存 ==> 需要我们自己存在stack中 移动rsp即可
            [3] 临时使用的内存 ==> stack中

            在函数内部frame 先假设所有寄存器空闲
            得出函数内部机器码后 在查看用到了那些寄存器 这些寄存器需要在用之前保存值

            一个函数的机器码如下:

                [1] rsp rbp 使用
                [2] 用过的其他寄存器得值 先保存下来
                [3] 函数内部的机器码
                [4] 恢复用过的其他寄存器的值
                [5] rsp rbp 恢复
    */
    PushFrame();
    // rest Register 应该 或者每个frame用自己的 都行

    // 这里应该把frame设置到node上作为一个属性 方便后续查找 todo  且函数的指令应该直接能从node获取方便很多
    // 应该是frame 关联node 较好
    // node 只需要关注scope 无需frame?
    this->curFrame->frame_name = node->functionName;
    node->nodeFrame = this->curFrame; // 设置node的 frame 一般只有函数需要这样处理
    std::string pre_str = this->cur_str;
    this->cur_str = "";

    // head
    // [1] push   %rbp
    this->OutPut(OP_PUSH, RF_Rbp, RegValType::POINT);
    // [2] mov   %rsp, %rbp
    this->OutPut(OP_Move, RF_Rsp, RegValType::POINT, RF_Rbp);
    std::string function_begin = this->cur_str; // 就是decl和 之前操作
    this->cur_str = "";

    // parameters and local variables 从rbp开始的偏移量 从0开始 下一个位置是 -8 + %rbp
    // 后续都使用相对位置来搞定
    int stack_frame_size = CalcFunctionVarAddr(node, 0, node->nodeScope); // 看看函数中总decl的size大小 从-8开始
    this->curFrame->SetBaseFp(stack_frame_size); // 类似计算出后续临时内存的地址

    //[3] body 记录
    this->Visit_CompoundStatement(node->compBody);
    std::string function_body_str = this->cur_str; // 就是decl和 之前操作
    this->cur_str = function_begin;
    if (stack_frame_size != 0){
        std::string con_str = "(" + std::to_string(stack_frame_size) + ")";
        ConstantOperand* constant = new ConstantOperand(con_str);
        this->OutPut(OP_Sub, constant, RegValType::POINT, RF_Rsp);
    }

    // callee saves registers
    std::string calleePushStr = this->curFrame->CalleeProtocol(C_PUSH); // 把函数内部使用过的寄存器的value先保存到stack上起来 每个函数都对应自己的frame
    this->cur_str += calleePushStr;
    this->cur_str += function_body_str; // 函数内部运作指令 放在这里是因为 比如先算出函数内部指令 才能知道会用到那些寄存器 而这些需要提前保存
    std::string calleePopStr = this->curFrame->CalleeProtocol(C_POP); // 恢复上面之前保存过的数据到对应寄存器上
    this->cur_str += calleePopStr;

    if (this->curFrame->GetMaxFp() != 0){
        //[4] mov    %rbp, %rsp   把当前frame 恢复 GetMaxFp 看来表示偏移 这里是想对位置
        // 如果存在函数call的话 函数的形参超过6个的话会保存在stack中 会导致内存多出来
        // 所以需要move 
        this->OutPut(OP_Move, RF_Rbp, RegValType::POINT, RF_Rsp);
    };
    // [5]pop    %rbp
    this->OutPut(OP_POP, RF_Rbp, RegValType::POINT);
    this->curFrame->frame_code = this->cur_str; // set func code
    std::cout<< this->curFrame->frame_name <<" : \n"<< std::endl;
    std::cout<<  this->curFrame->frame_code<<std::endl;;
    this->cur_str = pre_str + this->cur_str; // set global code
    PopFrame();
};


int CodeGenVisitor::CalcFunctionVarAddr(FunctionDefNode* node, int lastFpLoc, Scope* funcScop){
    /*
        参考Python或者动态语言的stack 这里存放的就是local变量
        local 包括 参数 和 自定义变量
        也就是说 stack的size最大应该是确定的 虽然一直在动态push pop 但是最终
        里面应该只存在local 变量内容了
        Vec_RegArg
    */
    // Vec_RegArg
    int argSize = CalcFunctionArgAddr(node);
    return argSize + CalcFunctionLocalVarAddr(node->compBody, argSize , funcScop);
};

int CodeGenVisitor::CalcFunctionArgAddr(FunctionDefNode* node){
    // node 的 arg 参数
    // 把arg的node 确定offset地址 或者寄存器地址
    // 这里只是为argument的node 设置对应的compileLocation
    int argIndex = 0, argSize = 0;
    int finalSize = 0;
    for (auto declNode : node->arguments->nodeList){ // todo 应该是针对每一个参数
        int tarSize = declNode->declarationType->GetTypeSize();
        if ((size_t)argIndex >= Vec_RegArg.size()){
            // 开辟存储
            argSize  += FrameManager::WORD_SIZE; // 64位地址 所以是8字节 WORD_SIZE=8
            declNode->compileLocation = new MemoryOperand(P_RBP, argSize); //高地址到低地址 向下生长 第一个参数在最下面
        }
        else {
            // 使用了寄存器就得 注意申请
            // 寄存器移除free中 把真实的参数rbp上的值复制move到参数寄存器上 这里就能看出形参和实参了
            RegisterOperand* reg = Vec_RegArg[argIndex];
            this->curFrame->RemoveFreeReg(reg);
            // 注意这里的remove 这些寄存器会在在FrameOperandStack中 不会pop
            MemoryOperand* argRealAddr = new MemoryOperand(P_RBP, -finalSize);// 参数真实在 rbp的上面内存位置
            declNode->compileLocation = argRealAddr; // 使用内存偏移 函数内部如果存在call操作 rdi rsi会重置 所以 rdi只是初始化的时候才有效
            this->OutPut(OP_Move, reg, declNode->declarationType->GetRegValType(), argRealAddr);
        }
        argIndex += 1;
        finalSize += tarSize;
    }
    return finalSize;
}

int GetNodeSize(Declaration* node){
    return node->declarationType->GetTypeSize();
}

int AlignAddr(int nextFpLoc, int alignSize){
    // alignSize 一般是 2 4 8 的倍数
    int remainder = (-nextFpLoc) % alignSize;
    if (remainder !=0 ){
        nextFpLoc = nextFpLoc - (alignSize - remainder); // 相当于补上多余的
    }
    return nextFpLoc;
}

int CodeGenVisitor::CalcFunctionLocalVarAddr(CompoundStmtNode* node, int lastFpLoc, Scope* funcScop){
    /*
        calculate local variable address in function body
        可以改成取scope的 body相关???
        这里只是为decl的node 设置对应的compileLocation
        并不会产生汇编代码 只是标记函数所持有的的decl-node 的compileLocation在哪
    */
    Scope* curScope = funcScop;
    int finalFpLoc = lastFpLoc;
    auto iter = curScope->symbolMap.begin();
    while (iter != curScope->symbolMap.end())
    {
        if (!(*iter).second->compileLocation){
            // 如果是argument 前面已经存在 compileLocation 了
            int size = GetNodeSize((*iter).second);
            int nextFpLoc = finalFpLoc - size; // 从高地址向低地址发展
            finalFpLoc = AlignAddr(nextFpLoc, size);
            (*iter).second->compileLocation = new MemoryOperand(P_RBP, -finalFpLoc); //高地址到低地址 向下生长 第一个参数在最下面
        }
        iter++;
    }
    return finalFpLoc;
}


void CodeGenVisitor::visit_FunctionCall(FunctionCallNode* node){
    /*
        https://godbolt.org/ 查看
        call的s时候没有scope
        push rbp rsp的操作在函数内部 而不是在call中 call只管参数 和 jump
        操作为
        如 foo(a, 2);
            mov   %eax, %edi
            mov   %0x2, %esi
            callq  _foo
            add rsp // 如果参数大于6的话

        [1] 参数操作 ==》 寄存器
        [2] callq 操作 ==》 jump
        [3] RSP移动 操作

        调用结束 移动stack指针 rsp move 为什么不是移动到rbp位置的方式??? 在内部函数搞定了
        靠 rbp只是真实的调用函数
        这里这是翻译 a() 这个的指令 并没有任何内容

        这个指令就是 存入arg   code指令集指针移动(代码区的指针)
        最后指令集指针回复 这个是底层实现应该call操作

        然后把几个参数的占用地址给回复即可(如果占用了) 函数后结束后自己的local变量会被清除
    */

    this->curFrame->CallerProtocol();
    // 反转

    node->argList->nodeList.reverse();
    int arg_num = node->argList->nodeList.size();
    for (auto arg : node->argList->nodeList){
        BaseOperand* argReg = VisitAndPop((ExprNode*)arg); // 算出器到reg 并pop出去
        BaseOperand* right_reg;
        if ((size_t)arg_num > Vec_RegArg.size()){
            // int offset = (8 - arg_num) * FrameManager::WORD_SIZE;
            // std::string warp = "-" + std::to_string(offset);
            // BaseOperand* right_reg = new ConstantOperand(warp + P_RSP);
            this->OutPut(OP_PUSH, argReg, RegValType::POINT); // 看是否是数还是其他 计算rsp并push
        }
        else {
            right_reg = Vec_RegArg[arg_num-1];
            this->OutPut(OP_Move, argReg, RegValType::POINT, right_reg);
        }
        arg_num -= 1;
    }
    node->argList->nodeList.reverse(); // 再次反转
    // todo 或许改成直接地址也行
    // 如果是隐式的 name ne? a->func()
    std::string funcName = node->function->variable_name;
    std::string call_str = "callq " + funcName + "\n";
    this->OutPut(call_str);
    this->curFrame->DoneRecover(); //调用结束 返回值肯定再rax中
    // 把返回值入栈
    // push的是rcx 所以我如果push成错误的 只需要movel即可 这个属于 先用正确得到操作 最后使用rax返回
    // 但是算术是必须要rax操作的 
    // 也就是两种 一种是非必须 可以用正确操作 最后move正确
    // todo
    /*
        下面这个导致 我们push的rcx寄存器 提供给返回值使用 也就是后续的FrameOperandStack 里面存的是A 
        如 s =  a*3 + func(1,2,3)
        在这个+的 binary_visit 中 right 结束后
        left是一个rax right是一个 rcx ！！！！
        所以这个binary 是 addl rcx rax

        但是在运行时候func的返回值必须用的是rax 所以这里存在问题 rax 会被覆盖
        right 只能是rax这个寄存器 除非不是寄存器
    */
    // 但是最后是 move rax 到 A
    // 
    BaseOperand* retReg = this->curFrame->StackPush(RF_Rax, false); 
    if ( retReg != RF_Rax){
        this->OutPut(OP_Move, RF_Rax, node->targetDeclarationNode->declarationType->GetRegValType() , retReg);
    }


    // 向上 高地址移动即可
    int argStackSize = (arg_num - Vec_RegArg.size()) * FrameManager::WORD_SIZE;
    if (argStackSize > 0){
        std::string num = "$" + std::to_string(argStackSize);
        BaseOperand* leftReg = new ConstantOperand(num);
        this->OutPut(OP_Add, leftReg, RegValType::INT, RF_Rsp);
    }
}

BaseOperand* CodeGenVisitor::VisitAndPop(ExprNode* node){
    node->Visit(this);
    return this->curFrame->StackPop();
}

void CodeGenVisitor::Visit_Assign(AssignNode* node){
    // a.c = xx; a.c 会是rax么?? 要判断下 凡是两个的 都得高一次
    node->assign_var->Visit(this);
    BaseOperand* right = VisitAndPop(node->assign_value);
    BaseOperand* left = this->curFrame->StackPop();
    this->OutPut(OP_Move, right, RegValType::INT, left);
}

void CodeGenVisitor::Visit_Unary(UnaryOpNode* node){
    BaseOperand* right = VisitAndPop(node->unary_node);
    switch (node->unary_op)
    {
    case Token::TOKEN_MINUS:
        // 取负数
        right->Negative();
        break;
    case Token::TOKEN_BANG:
        /* code */
        right->Negative();
        break;
    default:
        break;
    }
    if (!this->curFrame->IsTargetReg(right, C_RAX)){
        this->OutPut(OP_Move, right, RegValType::INT, RF_Rax);
    }
    this->curFrame->StackPush(RF_Rax, false);
}

std::pair<BaseOperand*, BaseOperand*> CodeGenVisitor::TryPushRegToStack(BaseOperand* prefer, bool force){
    // force 表示强制 如果被使用 就需要return 把 原来的返回 如 rax必须强制
    // 但是有时候就不会有这些 如 rdi 在参数时候就可以覆盖
    // 如何使用寄存器 move 操作 以及 算术操作
    // move $8 %rsi 这里的 rsi寄存器存值的时候才算被占用 这时候就得push
    if (TryUseRegFrame(prefer)){
        return std::pair<BaseOperand*, BaseOperand*>(NULL, prefer);
    }
    else{
        if (force){
            // 需要把前面一个存起来
            // 但是这一句得放使用prefer的 后前面 当前可能只是其他的const todo 后面可以字符串check一下
            // this->OutPut(OP_Move, right, RegValType::INT, RF_Rax);
            this->curFrame->StackPush(RF_Rax, false);
        }
    }
}

bool CodeGenVisitor::TryUseRegFrame(BaseOperand* operand){
    // 首先得明确
    // [1] 如果不在FrameOperandStack的reg 是否应该就是可用的??
    // [2] FrameOperandStack 和freeVec区别
    if (operand->GetOperandType() != OperandType::Register){
        return true;
    }
    RegisterOperand* reg = (RegisterOperand*)operand;
    if (reg->cur_state == RegisterState::WORK){
        // 在FrameOperandStack 中
        return false;
    }
}

void CodeGenVisitor::Visit_Binary(BinOpNode* node){
    /*
        对于一个exprNode的visit的 方法来说 无论是 一元二元这种visit
        我们都要假设当前rax都是空闲
        每一个exprNode 都会返回一个值存在stack中
        如果这个值不是常数 那么一定是寄存器 那这个寄存器一定得是rax寄存器 如果不是 就得先转化为rax
        如 add src dst 这个dst 必定是rax寄存器

        rax就是临时寄存器 类似stack_back 只保存当前一个值
        如果rax的值永远是临时值 且马上使用 不过不使用的话 就会保存
        所以每次连续操作完rax 必定会把rax 保存 如  a +b+c==》 rax 表示a， rax 表示a+b rax 表示a+b+c等
        寄存器 就是在 rax上 addl 后 再把rax的值 move 到对应地址即可。 move完后 rax 就失效了

        如 a.c.d 第一次rax表示a 下一次rax表示a.c 在下一次表示a.c.d 无线覆盖

        参考crafting compile中的 OP 操作
        如 OP_ADD 也就是取stack的最后一个和倒数第二个 他这里没有寄存器概念
        只不过我们有 我们需要的是吧stack中的元素替换为寄存器代表的元素
        OP操作只有两个寄存器 我们应该也是如此

        a*2 +b*2 +c*2 
        这个存在 a*2 可以用rax保存
    */

    std::string pre_str = this->cur_str;
    std::string left_str = "";
    std::string right_str = "";
    this->cur_str = "";
    node->binary_left->Visit(this); // 这里没有pop 表示rax还在使用中 所以后续的visit不能用rax了!
    left_str = this->cur_str;
    this->cur_str = "";
    BaseOperand* right = VisitAndPop(node->binary_right); // 这里会pop
    right_str = this->cur_str;
    this->cur_str = pre_str;

    BaseOperand* left = this->curFrame->StackPop(); 
    BaseOperand* final_reg;

    // 如果都用到了rax
    this->cur_str += left_str;
    if (this->curFrame->IsTargetReg(left, C_RAX)){
        if (this->curFrame->IsTargetReg(right, C_RAX)){
            // 关于寄存器的利用率图论后续可以优化优化 todo
            RegisterOperand* prefer_reg= this->curFrame->GetFreeReg(RF_Rbx);
            this->OutPut(OP_Move, left, RegValType::INT, RF_Rbx); // 放入新的寄存器中
            this->cur_str += right_str;
            final_reg = RF_Rbx;
        }
        else{
            this->cur_str += right_str; // 先把left 放进rax 再加入 right的代码
            final_reg = right;
        }
    }
    else{
        // 两个都不是rax
        if (!this->curFrame->IsTargetReg(right, C_RAX)){
            // 两个都不是rax
            this->OutPut(OP_Move, left, RegValType::INT, RF_Rax); // 放入新的寄存器中
            this->cur_str += right_str;
            final_reg = right;
        }
        else{
            this->cur_str += right_str;
            final_reg = left;
        }
    }
    switch (node->binary_op) {
        case Token::TOKEN_PLUS:
            this->OutPut(OP_Add, final_reg, RegValType::INT, RF_Rax);
            break;
        case Token::TOKEN_MINUS:
            this->OutPut(OP_Sub, final_reg, RegValType::INT, RF_Rax);
            break;
        case Token::TOKEN_STAR:
            this->OutPut(OP_MUL, final_reg, RegValType::INT, RF_Rax);
            break;
        case Token::TOKEN_SLASH:
            this->OutPut(OP_DIV, final_reg, RegValType::INT, RF_Rax);
            break;
        default:
            // this->OutPut(OP_Sub, right, RegValType::INT, left);
            break;
    }
    this->curFrame->StackPush(RF_Rax, false);
}

void CodeGenVisitor::Visit_Number(NumberNode* node){
    std::string val =  std::to_string(node->value);
    ConstantOperand* num = new ConstantOperand(val);
    this->curFrame->StackPush(num, false);
}

void CodeGenVisitor::Visit_Variable(VariableNode* node){
    /*
        curFrame 上一个如果是内存偏移的存储器
        因为binary等操作会用到连续两个curFrame的item
        而不允许这两个item都是内存偏移 至少需要一个寄存器
        所以就先把一个存在寄存器 再使用这个寄存器间接去操作


        首先Variable 实际上对应的是 declare的node
        variable 没有任何意义 只是指向declare
        variable 只需要找到对应declare的 compileLocation
        也就是 node->targetDeclarationNode->compileLocation
    */

    if (!this->curFrame->IsStackEmpty()){
        if (!this->curFrame->FrameOperandStack.empty()){
            BaseOperand* last = this->curFrame->FrameOperandStack.back();
            if (last->GetOperandType() == OperandType::Memory){
                BaseOperand* nodeLocation = node->targetDeclarationNode->compileLocation;
                BaseOperand* reg = this->curFrame->StackPop(); // 拿到寄存器
                this->OutPut(OP_Move, nodeLocation, node->targetDeclarationNode->declarationType->GetRegValType(), reg);
            }
        }
    }
    this->curFrame->StackPush(node->targetDeclarationNode->compileLocation, false);
}

void CodeGenVisitor::Visit_TreeNode(TreeNode* node){
    // 本质上一个file就是一个函数 函数的参数就是外部参
    PushFrame();
    this->curFrame->frame_name = "FILE_MAIN";
    // set global 如果是global 就不能用偏移了 要用地址 这里用符号代替
    SetGlobalVariableCompile(node->nodeScope);
    if (node->compBody){
        Visit_CompoundStatement(node->compBody);
    }
    // PopFrame(); 最后一个就不pop了
}

void CodeGenVisitor::Visit_Declaration(Declaration* node) {
    /*
        对于codegen来说
        declare都是已经被函数定义好了 也就是在之前decl的compileLocation 已经确定了
        如同：
        foo(){
            int a = 1;
        }
        这里的a 已经在函数中定义好了其compileLocation 

        后续的compBody 也会使得a调用到这里 但是a是已经被申明的
        codegen只关于赋值操作

        所以 如果是 int a; 没有任何output
        如果是 int a = 1; 需要取出a的地址的值 进行assign操作 
        所以 记住 codegen只关注于赋值
        一切都是在函数内发生的(整体就是main函数) 所有node的decl在函数之前已经通过Visit_FunctionDef被定义了compileLocation 
    */
    if (node->function) {
        // 这里也得push一个
        Visit_FunctionDef(node->function);
    }
    else {
        if (node->init_node){
            BaseOperand* tarReg = node->compileLocation; // 一般是decl位置 或者其他
            node->init_node->initial_value->Visit(this);
            BaseOperand* reg = this->curFrame->StackPop(); // 这里都是rax
            this->OutPut(OP_Move, reg, RegValType::INT, tarReg);
        }
    }
}

BaseOperand* CodeGenVisitor::FrameLookUpVariable(std::string name, Scope* scope){
    Declaration* decl = scope->ScopeLookUpSymbol(name);
    while(!decl){
        if (scope->parentScope){
            decl = scope->parentScope->ScopeLookUpSymbol(name);
        }
        else{
            break;
        }
    }
    if (decl){
        return decl->compileLocation;
    }
    return NULL;
}

void CodeGenVisitor::SetGlobalVariableCompile(Scope* funcScop){
    Scope* curScope = funcScop;
    auto iter = curScope->symbolMap.begin();
    while (iter != curScope->symbolMap.end())
    {
        if (!(*iter).second->compileLocation){
            (*iter).second->compileLocation = new VariableAddrOperand((*iter).second->varName); //高地址到低地址 向下生长 第一个参数在最下面
        }
        iter++;
    }
}
