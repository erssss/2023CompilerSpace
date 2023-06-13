#include "Instruction.h"
#include "BasicBlock.h"
#include <iostream>
#include "Function.h"
#include <sstream>
#include "Ast.h"
extern FILE *yyout;

#define DEBUG_SWITCH_genMaCode 0
#if DEBUG_SWITCH_genMaCode
#define printma(str) \
    std::cout << str << "\n";
#else
#define printma(str) //
#endif

FuncCallInstruction::FuncCallInstruction(
    Operand *dst, SymbolEntry *se, 
    std::vector<Operand *> rParams, BasicBlock *insert_bb) 
: Instruction(CALL, insert_bb)
{
    printg("== FuncCallInstruction ==");
    this->se = se;
    this->dst = dst;
    operands.emplace_back(dst);
    if (dst!=nullptr)
    {
        dst->setDef(this);
        dst->addUse(this);
    }
    for (auto param : rParams)
    {
        operands.emplace_back(param);
        param->addUse(this);
    }
}

void FuncCallInstruction::output() const
{
    fprintf(yyout, "  ");
    FunctionType *type = (FunctionType *)(se->getType());
    if (operands[0] && type->getRetType() != TypeSystem::voidType)
    {
        fprintf(yyout, "%s = ", operands[0]->toStr().c_str());
    }
    fprintf(yyout, "call %s %s(", 
            type->getRetType()->toStr().c_str(), 
            se->toStr().c_str());
    for (long unsigned int i = 1; i < operands.size(); i++)
    {
        if (i != 1)
        {
            fprintf(yyout, ", ");
        }
        fprintf(yyout, "%s %s", 
            operands[i]->getType()->toStr().c_str(), 
            operands[i]->toStr().c_str());
    }
    fprintf(yyout, ")\n");
}

Instruction::Instruction(unsigned instType, BasicBlock *insert_bb)
{
    prev = next = this;
    opcode = -1;
    this->instType = instType;
    if (insert_bb != nullptr)
    {
        insert_bb->insertBack(this);
        parent = insert_bb;
    }
}

Instruction::~Instruction()
{
    parent->remove(this);
}


void Instruction::setParent(BasicBlock *bb)
{
    parent = bb;
}

void Instruction::setNext(Instruction *instr)
{
    next = instr;
}

void Instruction::setPrev(Instruction *instr)
{
    prev = instr;
}

Instruction *Instruction::getNext()
{
    return next;
}

Instruction *Instruction::getPrev()
{
    return prev;
}

bool Instruction::isEssential()
{
    // return value
    // if (isRet()) {
    //     if (getUse().empty())
    //         return true;
    //     auto preds = parent->getParent()->getPreds();
    //     if (preds.empty())
    //         return true;
    //     // 只要有接收ret值的就要返回true
    //     for (auto it : preds)
    //         for (auto in : it.second)
    //             if (in->getDef()->usersNum())
    //                 return true;

    //     return false;
    // }
    // // input/output
    // if (isCall()) {
    //     IdentifierSymbolEntry* funcSE =
    //         (IdentifierSymbolEntry*)(((FuncCallInstruction*)this)->getSymbolEmpty());
    //     if (funcSE->getName() == "getint" || funcSE->getName() == "memset") {
    //         return true;
    //     } else {
    //         auto func = funcSE->getFunction();
    //         if (func->getEssential() == 1) {
    //             return true;
    //         }
    //     }
    // }
    // if (isStore()) {
    //     return true;
    // }
    return false;
}

XorInstruction::XorInstruction(
        Operand *dst, Operand *src, 
        BasicBlock *insert_bb) 
: Instruction(XOR, insert_bb)
{
    printg("== XorInstruction ==");
    operands.emplace_back(dst);
    operands.emplace_back(src);
    dst->setDef(this);
    src->addUse(this);

    this->push_back_opse(src->getSymbolEntry());
}

XorInstruction::~XorInstruction()
{
    operands[0]->setDef(nullptr);
    if (operands[0]->usersNum() == 0)
        delete operands[0];
    operands[1]->removeUse(this);
}

void XorInstruction::output() const
{
    Operand *dst = operands[0];
    Operand *src = operands[1];
    fprintf(yyout, "  %s = xor %s %s, true\n", 
            dst->toStr().c_str(), src->getType()->toStr().c_str(), 
            src->toStr().c_str());
}

BinaryInstruction::BinaryInstruction(
        unsigned opcode, 
        Operand *dst, Operand *src1, Operand *src2, 
        BasicBlock *insert_bb) 
: Instruction(BINARY, insert_bb)
{
    printg("== BinaryInstruction ==");
    this->opcode = opcode;
    operands.emplace_back(dst);
    dst->setDef(this);
    operands.emplace_back(src1);
    dst->addUse(this);
    src1->addUse(this);
    operands.emplace_back(src2);
    src2->addUse(this);

    // CSE 废弃
    // this->push_back_opse(src1->getSymbolEntry());
    // this->push_back_opse(src2->getSymbolEntry());
}

BinaryInstruction::~BinaryInstruction()
{
    operands[0]->setDef(nullptr);
    if (operands[0]->usersNum() == 0)
        delete operands[0];
    operands[1]->removeUse(this);
    operands[2]->removeUse(this);
}

void BinaryInstruction::output() const
{
    std::string s1, s2, s3, op;
    int v2, v3;
    bool flag1(false), flag2(false);
    printg("+Binary+");
    s1 = operands[0]->toStr();
    if (operands[1]->isConstant())
    {
        v2 = operands[1]->getValue();
        flag1 = true;
    }
    else
    {
        s2 = operands[1]->toStr();
    }
    if (operands[2]->isConstant())
    {
        v3 = operands[2]->getValue();
        flag2 = true;
    }
    else
    {
        s3 = operands[2]->toStr();
    }
    Type *type = operands[0]->getType();
    switch (opcode)
    {
    case ADD:
        if (type->isFloat())
        {
            op = "fadd";
        }
        else
        {
            op = "add";
        }
        break;
    case SUB:
        if (type->isFloat())
        {
            op = "fsub";
        }
        else
        {
            op = "sub";
        }
        break;
    case MUL:
        if (type->isFloat())
        {
            op = "fmul";
        }
        else
        {
            op = "mul";
        }
        break;
    case DIV:
        if (type->isFloat())
        {
            op = "fdiv";
        }
        else
        {
            op = "sdiv";
        }
        break;
    case MOD:
        op = "srem";
        break;
    case AND:
        op = "and";
        break;
    case OR:
        op = "or";
        break;
    default:
        break;
    }
    if (flag1 && flag2)
    {
        fprintf(yyout, "  %s = %s %s %d, %d\n", 
                s1.c_str(), op.c_str(), type->toStr().c_str(), 
                v2, v3);
    }
    else if (flag1 && !flag2)
    {
        fprintf(yyout, "  %s = %s %s %d, %s\n", 
                s1.c_str(), op.c_str(), type->toStr().c_str(), 
                v2, s3.c_str());
    }
    else if (!flag1 && flag2)
    {
        fprintf(yyout, "  %s = %s %s %s, %d\n", 
                s1.c_str(), op.c_str(), type->toStr().c_str(), 
                s2.c_str(), v3);
    }
    else
    {
        fprintf(yyout, "  %s = %s %s %s, %s\n", 
                s1.c_str(), op.c_str(), type->toStr().c_str(), 
                s2.c_str(), s3.c_str());
    }
}

CmpInstruction::CmpInstruction(unsigned opcode, Operand *dst, Operand *src1, Operand *src2, BasicBlock *insert_bb) : Instruction(CMP, insert_bb)
{
    printg("== CmpInstruction ==");
    this->opcode = opcode;
    operands.emplace_back(dst);
    dst->setDef(this);
    operands.emplace_back(src1);
    dst->addUse(this);
    src1->addUse(this);
    operands.emplace_back(src2);
    src2->addUse(this);
}

CmpInstruction::~CmpInstruction()
{
    operands[0]->setDef(nullptr);
    if (operands[0]->usersNum() == 0)
        delete operands[0];
    operands[1]->removeUse(this);
    operands[2]->removeUse(this);
}

void CmpInstruction::output() const
{
    std::string s1, s2, s3, op;
    printg("+Cmp+");
    Type *type = operands[1]->getType();
    s1 = operands[0]->toStr();
    s2 = operands[1]->toStr();
    s3 = operands[2]->toStr();
    switch (opcode)
    {
    case E:
        op = "eq";
        break;
    case NE:
        op = "ne";
        break;
    case L:
        op = "slt";
        break;
    case LE:
        op = "sle";
        break;
    case G:
        op = "sgt";
        break;
    case GE:
        op = "sge";
        break;
    default:
        op = "";
        break;
    }

    fprintf(yyout, "  %s = icmp %s %s %s, %s\n", 
            s1.c_str(), op.c_str(), 
            type->toStr().c_str(), 
            s2.c_str(), s3.c_str());
}

UncondBrInstruction::UncondBrInstruction(
        BasicBlock *to, BasicBlock *insert_bb) 
: Instruction(UNCOND, insert_bb)
{
    printg("== UncondBrInstruction ==");
    branch = to;
}

void UncondBrInstruction::output() const
{
    printg("+UncondBrInstruction+");
    fprintf(yyout, "  br label %%B%d\n", branch->getNo());
}

void UncondBrInstruction::setBranch(BasicBlock *bb)
{
    branch = bb;
}

BasicBlock *UncondBrInstruction::getBranch()
{
    return branch;
}

CondBrInstruction::CondBrInstruction(
        BasicBlock *true_branch, BasicBlock *false_branch, 
        Operand *cond, BasicBlock *insert_bb) 
: Instruction(COND, insert_bb)
{
    printg("== CondBrInstruction ==");
    this->true_branch = true_branch;
    this->false_branch = false_branch;
    this->cond = cond;
    cond->addUse(this);
    operands.emplace_back(cond);
}

CondBrInstruction::~CondBrInstruction()
{
    operands[0]->removeUse(this);
}

void CondBrInstruction::output() const
{
    std::string cond, type;
    int value;
    printg("+CondBrInstruction+");
    int truebb_label = true_branch->getNo();
    int falsebb_label = false_branch->getNo();
    type = operands[0]->getType()->toStr();
    if (operands[0]->getSymbolEntry()->isConstant())
    {
        ConstantSymbolEntry *cs = dynamic_cast<ConstantSymbolEntry *>(operands[0]->getSymbolEntry());
        value = cs->getIntValue();
        fprintf(yyout, "  br %s %d, label %%B%d, label %%B%d\n", 
                type.c_str(), value, truebb_label, falsebb_label);
    }
    else
    {
        cond = operands[0]->toStr();
        fprintf(yyout, "  br %s %s, label %%B%d, label %%B%d\n", 
                type.c_str(), cond.c_str(), truebb_label, falsebb_label);
    }
}

void CondBrInstruction::setFalseBranch(BasicBlock *bb)
{
    false_branch = bb;
}

BasicBlock *CondBrInstruction::getFalseBranch()
{
    return false_branch;
}

void CondBrInstruction::setTrueBranch(BasicBlock *bb)
{
    true_branch = bb;
}

BasicBlock *CondBrInstruction::getTrueBranch()
{
    return true_branch;
}

RetInstruction::RetInstruction(Operand *src, BasicBlock *insert_bb) 
: Instruction(RET, insert_bb)
{
    if (src != nullptr)
    {
        operands.emplace_back(src);
        src->addUse(this);
    }
}

RetInstruction::~RetInstruction()
{
    if (!operands.empty())
        operands[0]->removeUse(this);
}

void RetInstruction::output() const
{
    printg("+RetInstruction+");
    if (operands.empty())
    {
        fprintf(yyout, "  ret void\n");
    }
    else
    {
        std::string ret, type;
        ret = operands[0]->toStr();
        type = operands[0]->getType()->toStr();
        fprintf(yyout, "  ret %s %s\n", type.c_str(), ret.c_str());
    }
}

AllocaInstruction::AllocaInstruction(
    Operand *dst, SymbolEntry *se, 
    BasicBlock *insert_bb) 
: Instruction(ALLOCA, insert_bb)
{
    printg("== AllocaInstruction ==");
    operands.emplace_back(dst);
    dst->setDef(this);
    this->se = se;
}

AllocaInstruction::~AllocaInstruction()
{
    operands[0]->setDef(nullptr);
    if (operands[0]->usersNum() == 0)
        delete operands[0];
}

void AllocaInstruction::output() const
{
    std::string dst, type;
    printg("+AllocaInstruction+");
    dst = operands[0]->toStr();

    type = se->getType()->toStr();
    if (se->getType()->isArray())
    {
        ArrayType *arrType = dynamic_cast<ArrayType *>(se->getType());
        if (arrType->getLenVec()[0] == 0)
        {
            type = (new PointerType(arrType->getEleType()))->toStr();
        }
        else
            type = arrType->toStr();
    }
    fprintf(yyout, "  %s = alloca %s, align 4\n", dst.c_str(), type.c_str());
}

LoadInstruction::LoadInstruction(
        Operand *dst, Operand *src_addr, 
        BasicBlock *insert_bb, int align) 
: Instruction(LOAD, insert_bb)
{
    printg("== LoadInstruction ==");
    this->align = align;
    operands.emplace_back(dst);
    operands.emplace_back(src_addr);
    dst->setDef(this);
    // dst->addUse(this);
    src_addr->addUse(this);
    printinfo("load instruction");
}

LoadInstruction::~LoadInstruction()
{
    operands[0]->setDef(nullptr);
    if (operands[0]->usersNum() == 0)
        delete operands[0];
    operands[1]->removeUse(this);
}

void LoadInstruction::output() const
{
    std::string dst = operands[0]->toStr();
    std::string src = operands[1]->toStr();
    std::string src_type;
    std::string dst_type;
    dst_type = operands[0]->getType()->toStr();
    printg("+Load+");
    src_type = operands[1]->getType()->toStr();
    if (operands[1]->getType()->isArray())
    {
        Type *type = dynamic_cast<ArrayType *>
                (operands[1]->getType())->getEleType();
        src_type = (new PointerType(type))->toStr();
    }

    fprintf(yyout, "  %s = load %s, %s %s, align %d\n", 
            dst.c_str(), dst_type.c_str(), 
            src_type.c_str(), src.c_str(), 
            align);
}

StoreInstruction::StoreInstruction(
        Operand *dst_addr, Operand *src, 
        BasicBlock *insert_bb, int align) 
: Instruction(STORE, insert_bb)
{
    printg("== StoreInstruction ==");

    this->align = align;
    operands.emplace_back(dst_addr);
    operands.emplace_back(src);
    dst_addr->addUse(this);
    src->addUse(this);

    // this->push_back_opse(dst_addr->getSymbolEntry());
}

StoreInstruction::~StoreInstruction()
{
    operands[0]->removeUse(this);
    operands[1]->removeUse(this);
}

void StoreInstruction::output() const
{
    printg("+Store+");

    std::string dst = operands[0]->toStr();
    std::string src = operands[1]->toStr();
    std::string dst_type = operands[0]->getType()->toStr();
    std::string src_type = operands[1]->getType()->toStr();

    fprintf(yyout, "  store %s %s, %s %s, align %d\n", 
            src_type.c_str(), src.c_str(), 
            dst_type.c_str(), dst.c_str(), align);
}
GepInstruction::~GepInstruction()
{
    operands[0]->setDef(nullptr);
    if (operands[0]->usersNum() == 0)
        delete operands[0];
    operands[1]->removeUse(this);
    operands[2]->removeUse(this);
}
GepInstruction::GepInstruction(
        Operand *dst, Operand *src,
        BasicBlock *insert_bb, Operand *idx, 
        bool param, SymbolEntry *se) 
        : Instruction(GEP, insert_bb)
{
    printg("== GepInstruction ==");
    this->param = param;
    operands.emplace_back(dst);
    operands.emplace_back(src);
    if (idx == nullptr)
    {
        idxFlag = 0;
        idx = new Operand(new TemporarySymbolEntry
                (TypeSystem::intType, SymbolTable::getLabel()));
    }
    operands.emplace_back(idx);
    dst->setDef(this);
    dst->addUse(this);
    src->addUse(this);
    if (idx != nullptr)
        idx->addUse(this);

    if (!param)
    {
        this->push_back_opse(src->getSymbolEntry());
    }
    else
    {
        this->push_back_opse(se);
    }
    if (idx != nullptr)
        this->push_back_opse(idx->getSymbolEntry());
}

void GepInstruction::output() const
{
    printg("+Gep+");
    Operand *dst = operands[0];
    Operand *src = operands[1];
    Operand *idx = operands[2];
    ArrayType *arrType = (ArrayType *)dst->getType();
    std::string buffer_str1;
    std::string buffer_str2;
    std::string index;
    if (idx != nullptr)
    {
        index = idx->toStr();
    }
    else
    {
        index = "0";
    }
    // z1214 todo 或许浮点得改
    bool flag = 0;
    Type *basicType = arrType->getEleType();
    if (arrType->getLenVec().size() == 0 || arrType->getLenVec()[0] == 0)
    {
        buffer_str1 = basicType->toStr();
        buffer_str2 = (new PointerType(basicType))->toStr();
        flag = 1;
    }
    else
    {
        buffer_str1 = arrType->toStr();
        buffer_str2 = (new PointerType(arrType))->toStr();
        // buffer_str2 = buffer_str1;
    }
    if (flag == 0)
    {
        // z1204 fix 或许指针基址不一样首个i32 0需要改变
        //  %1 = getelementptr [5 x [4 x i32]], [5 x [4 x i32]]* @a, i32 0, i32 2
        fprintf(yyout, "  %s = getelementptr inbounds %s, %s %s, %s 0, %s %s\n",
                dst->toStr().c_str(), buffer_str1.c_str(),
                buffer_str2.c_str(), src->toStr().c_str(),
                basicType->toStr().c_str(), basicType->toStr().c_str(),
                index.c_str());
    }
    else
    {
        fprintf(yyout, "  %s = getelementptr inbounds %s, %s %s, %s %s\n",
                dst->toStr().c_str(), buffer_str1.c_str(),
                buffer_str2.c_str(), src->toStr().c_str(),
                basicType->toStr().c_str(),
                index.c_str());
    }
}

ZextInstruction::ZextInstruction(Operand *dst,
                                 Operand *src,
                                 BasicBlock *insert_bb)
    : Instruction(ZEXT, insert_bb)
{
    operands.emplace_back(dst);
    operands.emplace_back(src);
    dst->setDef(this);
    dst->addUse(this);
    src->addUse(this);
}

void ZextInstruction::output() const
{
    Operand *dst = operands[0];
    Operand *src = operands[1];
    int v;
    if (operands[1]->getSymbolEntry()->isConstant())
    {
        ConstantSymbolEntry *cs = dynamic_cast<ConstantSymbolEntry *>(operands[1]->getSymbolEntry());
        v = cs->getIntValue();
        fprintf(yyout, "  %s = zext %s %d to i32\n", dst->toStr().c_str(),
                src->getType()->toStr().c_str(), v);
    }
    else
    {
        fprintf(yyout, "  %s = zext %s %s to i32\n", dst->toStr().c_str(),
                src->getType()->toStr().c_str(), src->toStr().c_str());
    }
}

MachineOperand *Instruction::genMachineOperand(Operand *op)
{
    SymbolEntry *se = op->getSymbolEntry();
    MachineOperand *newop = nullptr;
    if (op->isConstant())
    {
        newop = new MachineOperand(MachineOperand::IMM, op->getValue());
    }
    else if (op->isTemporary())
    {
        newop = new MachineOperand(MachineOperand::VREG, dynamic_cast<TemporarySymbolEntry *>(se)->getLabel());
    }
    else if (op->isVariable())
    {
        IdentifierSymbolEntry *id = dynamic_cast<IdentifierSymbolEntry *>(se);
        if (id->isGlobal())
        {
            newop = new MachineOperand(id->toStr().c_str());
        }
        else if (id->isParam())
        {
            if (id->getParamNo() < 4)
            {
                int temp = id->getParamNo();
                newop = new MachineOperand(MachineOperand::REG, temp);
            }
            else
            {
                newop = new MachineOperand(MachineOperand::REG, 3);
            }
        }
    }
    return newop;
}

MachineOperand *Instruction::genMachineFReg(int freg)
{
    return new MachineOperand(MachineOperand::REG, freg + 16, true);
}

MachineOperand *Instruction::genMachineReg(int reg)
{
    return new MachineOperand(MachineOperand::REG, reg);
}

MachineOperand *Instruction::genMachineVReg(bool fpu)
{
    return new MachineOperand(MachineOperand::VREG, SymbolTable::getLabel(), fpu);
}

MachineOperand *Instruction::genMachineImm(int val)
{
    return new MachineOperand(MachineOperand::IMM, val);
}

MachineOperand *Instruction::genMachineLabel(int block_no)
{
    std::ostringstream buffer;
    buffer << ".L" << block_no;
    std::string label = buffer.str();
    auto ret = new MachineOperand(label);
    return ret;
}

void AllocaInstruction::genMachineCode(AsmBuilder *builder)
{
    /* HINT:
     * Allocate stack space for local variabel
     * Store frame offset in symbol entry */
    MachineFunction *nowfunc = builder->getFunction();
    if (se->getType()->isArray())
    {
        dynamic_cast<ArrayType *>(se->getType())->setSize();
    }
    int size = se->getType()->getSize() / 8;
    if (size <= 0)size = 4;
    int offset = nowfunc->AllocSpace(size);
    dynamic_cast<TemporarySymbolEntry *>(operands[0]->getSymbolEntry())->setOffset(-offset);
}

void LoadInstruction::genMachineCode(AsmBuilder *builder)
{
    MachineBlock *nowblock = builder->getBlock();
    MachineInstruction *newinst = nullptr;
    // 局部变量
    if (operands[1]->isTemporary() && operands[1]->getDef() && operands[1]->getDef()->isAlloc())
    {
        // load r1, [r0, #4]
        if (operands[0]->getType()->isFloat())
        {
            MachineOperand *dst = genMachineFloatOperand(operands[0]);
            MachineOperand *src1 = genMachineReg(11);
            int offset = dynamic_cast<TemporarySymbolEntry *>
                    (operands[1]->getSymbolEntry())->getOffset();
            MachineOperand *src2 = genMachineImm(offset);
            if (offset < -255 || offset > 255  )
            {
                MachineOperand *vreg = genMachineVReg();
                nowblock->InsertInst((new LoadMInstruction(nowblock, 
                        LoadMInstruction::LDR, vreg, src2)));
                src2 = new MachineOperand(*vreg);
                vreg = genMachineVReg();
                nowblock->InsertInst((new BinaryMInstruction(nowblock, 
                        BinaryMInstruction::ADD, vreg, src1, src2)));
                newinst = new LoadMInstruction(nowblock, 
                        LoadMInstruction::VLDR, dst, new MachineOperand(*vreg));
            }
            else
            {
                newinst = new LoadMInstruction(
                    nowblock, LoadMInstruction::VLDR, dst, src1, src2);
            }
            nowblock->InsertInst(newinst);
        }
        else
        {
            MachineOperand *dst = genMachineOperand(operands[0]);
            MachineOperand *src1 = genMachineReg(11);
            int offset = dynamic_cast<TemporarySymbolEntry *>(operands[1]->getSymbolEntry())->getOffset();
            MachineOperand *src2 = genMachineImm(offset);
            if (offset > 255 || offset < -255)
            {
                MachineOperand *operand = genMachineVReg();
                nowblock->InsertInst((new LoadMInstruction(nowblock, 
                        LoadMInstruction::LDR, operand, src2)));
                src2 = operand;
            }
            newinst = new LoadMInstruction(nowblock, 
                    LoadMInstruction::LDR, dst, src1, src2);
            nowblock->InsertInst(newinst);
        }
    }
    // 全局变量
    else if (operands[1]->isVariable() && operands[1]->isGobal())
    {
        if (operands[0]->getType()->isFloat())
        {
            MachineOperand *dst = genMachineFloatOperand(operands[0]);
            MachineOperand *vreg1 = genMachineVReg();
            MachineOperand *vreg_ = new MachineOperand(*vreg1);
            MachineOperand *src = genMachineOperand(operands[1]);
            newinst = new LoadMInstruction(nowblock, 
                    LoadMInstruction::LDR, vreg1, src);
            nowblock->InsertInst(newinst);
            newinst = new LoadMInstruction(nowblock, 
                    LoadMInstruction::VLDR, dst, vreg_);
            nowblock->InsertInst(newinst);
        }
        else
        {
            MachineOperand *dst = genMachineOperand(operands[0]);
            MachineOperand *vreg1 = genMachineVReg();
            MachineOperand *vreg_ = new MachineOperand(*vreg1);
            MachineOperand *src = genMachineOperand(operands[1]);
            // load r0, addr_a
            newinst = new LoadMInstruction(nowblock, 
                    LoadMInstruction::LDR, vreg1, src);
            nowblock->InsertInst(newinst);
            // load r1, [r0]
            newinst = new LoadMInstruction(nowblock, 
                    LoadMInstruction::LDR, dst, vreg_);
            nowblock->InsertInst(newinst);
        }
    }
    else
    {
        // load r1, [r0]
        MachineOperand *dst = nullptr;
        if (operands[0]->getType()->isFloat())
        {
            dst = genMachineFloatOperand(operands[0]);
        }
        else
        {
            dst = genMachineOperand(operands[0]);
        }

        MachineOperand *src = genMachineOperand(operands[1]);
        if (operands[0]->getType()->isFloat() ||
            operands[1]->getType()->isFloat())
        {
            newinst = new LoadMInstruction(nowblock, 
                    LoadMInstruction::VLDR, dst, src);
            nowblock->InsertInst(newinst);
        }
        else
        {
            newinst = new LoadMInstruction(nowblock, 
                    LoadMInstruction::LDR, dst, src);
            nowblock->InsertInst(newinst);
        }
    }
}

void StoreInstruction::genMachineCode(AsmBuilder *builder)
{
    MachineBlock *nowblock = builder->getBlock();
    MachineInstruction *newinst = nullptr;
    MachineOperand *dst = nullptr;
    MachineOperand *src = nullptr;

    dst = genMachineOperand(operands[0]);

    if (operands[1]->getType()->isFloat())
    {
        src = genMachineFloatOperand(operands[1]);
    }
    else
    {
        src = genMachineOperand(operands[1]);
    }
    bool flag = operands[1]->getType()->isFloat();
    // 存储立即数
    if (operands[1]->isConstant())
    {
        MachineOperand *vreg = genMachineVReg(flag);
        if (flag)
        {
            MachineOperand *vreg1 = genMachineVReg();
            newinst = new LoadMInstruction(nowblock, 
                    LoadMInstruction::LDR, vreg1, src);
            nowblock->InsertInst(newinst);
            vreg1 = new MachineOperand(*vreg1);
            newinst = new MovMInstruction(nowblock, 
                    MovMInstruction::VMOV, vreg, vreg1);
        }
        else
        {
            newinst = new LoadMInstruction(nowblock, 
                    LoadMInstruction::LDR, vreg, src);
        }
        nowblock->InsertInst(newinst);
        src = new MachineOperand(*vreg);
    }
    // 局部变量
    if (operands[0]->isTemporary() && 
        operands[0]->getDef() && 
        operands[0]->getDef()->isAlloc())
    {
        MachineOperand *src1 = genMachineReg(11);
        int offset = dynamic_cast<TemporarySymbolEntry *>(operands[0]->getSymbolEntry())->getOffset();
        MachineOperand *src2 = genMachineImm(offset);
        if (offset > 255 || offset < -255)
        {
            MachineOperand *operand = genMachineVReg();
            nowblock->InsertInst((new LoadMInstruction(
                nowblock, LoadMInstruction::LDR, operand, src2)));
            src2 = operand;
        }
        if (flag)
        {
            if (offset > 255 || offset < -255)
            {
                MachineOperand *vreg = genMachineVReg();
                newinst = new BinaryMInstruction(
                    nowblock, BinaryMInstruction::ADD, vreg, src1, src2);
                nowblock->InsertInst(newinst);
                newinst = new StoreMInstruction(nowblock, 
                        StoreMInstruction::VSTR, src, new MachineOperand(*vreg));
            }
            else
            {
                newinst = new StoreMInstruction(nowblock, 
                        StoreMInstruction::VSTR, src, src1, src2);
            }
        }
        else
        {
            newinst = new StoreMInstruction(nowblock, 
                    StoreMInstruction::STR, src, src1, src2);
        }
        nowblock->InsertInst(newinst);
    }
    // 全局变量
    else if (operands[0]->isVariable() 
            && operands[0]->isGobal())
    {
        MachineOperand *vreg1 = genMachineVReg();
        // load r0, addr_a
        if (flag)
        {
            newinst = new LoadMInstruction(nowblock, 
                    LoadMInstruction::LDR, vreg1, dst);
            nowblock->InsertInst(newinst);
            newinst = new StoreMInstruction(nowblock, 
                    StoreMInstruction::VSTR, src, vreg1);
            nowblock->InsertInst(newinst);
        }
        else
        {
            newinst = new LoadMInstruction(nowblock, 
                    LoadMInstruction::LDR, vreg1, dst);
            nowblock->InsertInst(newinst);
            // store r1, [r0]
            newinst = new StoreMInstruction(nowblock, 
                    StoreMInstruction::STR, src, vreg1);
            nowblock->InsertInst(newinst);
        }
    }
    // 存储地址
    else if (operands[0]->getType()->isPointer())
    {
        if (flag)
        {
            newinst = new StoreMInstruction(nowblock, 
                    StoreMInstruction::VSTR, src, dst);
            nowblock->InsertInst(newinst);
        }
        else
        {
            // std::cout<<"operands[1]->getSymbolEntry() "
            // <<operands[1]->getSymbolEntry()->toStr()<<"\n";
            newinst = new StoreMInstruction(nowblock, 
                    StoreMInstruction::STR, src, dst);
            nowblock->InsertInst(newinst);
        }
    }
    else
    {
        if (flag)
        {
            newinst = new StoreMInstruction(nowblock, 
                    StoreMInstruction::VSTR, src, dst);
            nowblock->InsertInst(newinst);
        }
        else
        {
            newinst = new StoreMInstruction(nowblock, 
                    StoreMInstruction::STR, src, dst);
            nowblock->InsertInst(newinst);
        }
    }
}

void BinaryInstruction::genMachineCode(AsmBuilder *builder)
{
    MachineBlock *nowblock = builder->getBlock();
    /* HINT:
     * The source operands of ADD instruction in ir code both can be immediate num.
     * However, it's not allowed in assembly code.
     * So you need to insert LOAD/MOV instrucrion to load immediate num into register.
     * As to other instructions, such as MUL, CMP, you need to deal with this situation, too.*/
    MachineInstruction *newinst = nullptr;
    MachineOperand *dst = nullptr;
    MachineOperand *src1 = nullptr;
    MachineOperand *src2 = nullptr;

    bool flag = operands[0]->getType()->isFloat();

    if (flag)
    {
        dst = genMachineFloatOperand(operands[0]);
        src1 = genMachineFloatOperand(operands[1]);
        src2 = genMachineFloatOperand(operands[2]);
    }
    else
    {
        dst = genMachineOperand(operands[0]);
        src1 = genMachineOperand(operands[1]);
        src2 = genMachineOperand(operands[2]);
    }

    if (operands[0]->getType()->isFloat())
    {
        if (src1->isImm())
        {
            MachineOperand *vreg1 = genMachineVReg(true);
            MachineOperand *vreg_ = genMachineVReg();
            newinst = new LoadMInstruction(nowblock, 
                    LoadMInstruction::LDR, vreg_, src1);
            nowblock->InsertInst(newinst);
            vreg_ = new MachineOperand(*vreg_);
            newinst = new MovMInstruction(nowblock, 
                    MovMInstruction::VMOV,vreg1, vreg_);
            nowblock->InsertInst(newinst);
            src1 = new MachineOperand(*vreg1);
        }
        if (src2->isImm())
        {
            if (src2->getFVal() == 0 && opcode == ADD)
            {
                newinst = new MovMInstruction(nowblock, MovMInstruction::VMOVF32, dst, src1);
                return;
            }
            else
            {
                MachineOperand *vreg1 = genMachineVReg(true);
                MachineOperand *vreg_ = genMachineVReg();
                newinst = new LoadMInstruction(nowblock, 
                        LoadMInstruction::LDR, vreg_, src2);
                nowblock->InsertInst(newinst);
                vreg_ = new MachineOperand(*vreg_);
                newinst = new MovMInstruction(nowblock, 
                        MovMInstruction::VMOV, vreg1, vreg_);
                nowblock->InsertInst(newinst);
                src2 = new MachineOperand(*vreg1);
            }
        }
        switch (opcode)
        {
        case ADD:
            newinst = new BinaryMInstruction(nowblock, 
                    BinaryMInstruction::VADD, dst, src1, src2);
            break;
        case SUB:
            newinst = new BinaryMInstruction(nowblock, 
                    BinaryMInstruction::VSUB, dst, src1, src2);
            break;
        case MUL:
            newinst = new BinaryMInstruction(nowblock, 
                    BinaryMInstruction::VMUL, dst, src1, src2);
            break;
        case DIV:
            newinst = new BinaryMInstruction(nowblock, 
                    BinaryMInstruction::VDIV, dst, src1, src2);
            break;
        default:
            break;
        }
        nowblock->InsertInst(newinst);
    }
    else
    {
        if (src1->isImm() && src2->isImm() && src2->getVal() == 0 &&
            opcode == ADD)
        {
            if (!(src1->getVal() < 256 && src1->getVal() > -255))
            {
                MachineOperand *vreg = genMachineVReg();
                newinst = new LoadMInstruction(
                    nowblock, LoadMInstruction::LDR, vreg, src1);
                nowblock->InsertInst(newinst);
                src1 = new MachineOperand(*vreg);
            }
            newinst = new MovMInstruction(nowblock, MovMInstruction::MOV, dst, src1);
            nowblock->InsertInst(newinst);
            return;
        }
        if (src1->isImm())
        {
            MachineOperand *vreg = genMachineVReg();
            newinst = new LoadMInstruction(nowblock, LoadMInstruction::LDR, vreg, src1);
            nowblock->InsertInst(newinst);
            src1 = new MachineOperand(*vreg);
        }
        if (src2->isImm())
        {
            if ((opcode <= BinaryInstruction::OR && operands[2]->getValue() > 255) ||
                opcode >= BinaryInstruction::MUL)
            {
                MachineOperand *vreg = genMachineVReg();
                newinst = new LoadMInstruction(nowblock, LoadMInstruction::LDR, vreg, src2);
                nowblock->InsertInst(newinst);
                src2 = new MachineOperand(*vreg);
            }
        }
        switch (opcode)
        {
        case ADD:
            newinst = new BinaryMInstruction(nowblock, 
                    BinaryMInstruction::ADD, dst, src1, src2);
            break;
        case SUB:
            newinst = new BinaryMInstruction(nowblock, 
                    BinaryMInstruction::SUB, dst, src1, src2);
            break;
        case MUL:
            newinst = new BinaryMInstruction(nowblock, 
                    BinaryMInstruction::MUL, dst, src1, src2);
            break;
        case DIV:
            newinst = new BinaryMInstruction(nowblock, 
                    BinaryMInstruction::DIV, dst, src1, src2);
            break;
        case AND:
            newinst = new BinaryMInstruction(nowblock, 
                    BinaryMInstruction::AND, dst, src1, src2);
            break;
        case OR:
            newinst = new BinaryMInstruction(nowblock, 
                    BinaryMInstruction::OR, dst, src1, src2);
            break;
        case MOD:
            newinst = new BinaryMInstruction(nowblock, 
                    BinaryMInstruction::DIV, dst, src1, src2);
            MachineOperand *temp = new MachineOperand(*dst);
            nowblock->InsertInst(newinst);
            newinst = new BinaryMInstruction(nowblock, 
                    BinaryMInstruction::MUL, temp, dst, new MachineOperand(*src2));
            nowblock->InsertInst(newinst);
            dst = new MachineOperand(*temp);
            newinst = new BinaryMInstruction(nowblock, 
                    BinaryMInstruction::SUB, dst, new MachineOperand(*src1), temp);
            break;
        }
        nowblock->InsertInst(newinst);
    }
}

void CmpInstruction::genMachineCode(AsmBuilder *builder)
{
    printma("CmpInstruction");
    MachineBlock *nowblock = builder->getBlock();
    if (operands[1]->getType()->isFloat())
    {
        MachineOperand *src1 = genMachineFloatOperand(operands[1]);
        MachineOperand *src2 = genMachineFloatOperand(operands[2]);
        MachineInstruction *newinst = nullptr;
        if (src1->isImm())
        {
            MachineOperand *vreg = genMachineVReg(true);
            MachineOperand *vreg_ = genMachineVReg();
            newinst = new LoadMInstruction(nowblock, 
                    LoadMInstruction::LDR, vreg_, src1);
            nowblock->InsertInst(newinst);
            vreg_ = new MachineOperand(*vreg_);
            newinst = new MovMInstruction(nowblock, 
                    MovMInstruction::VMOV, vreg, vreg_);
            nowblock->InsertInst(newinst);
            src1 = new MachineOperand(*vreg);
        }
        if (src2->isImm())
        {
            MachineOperand *vreg = genMachineVReg(true);
            MachineOperand *vreg_ = genMachineVReg();
            newinst = new LoadMInstruction(nowblock, 
                    LoadMInstruction::LDR, vreg_, src2);
            nowblock->InsertInst(newinst);
            vreg_ = new MachineOperand(*vreg_);
            newinst = new MovMInstruction(nowblock, 
                    MovMInstruction::VMOV, vreg, vreg_);
            nowblock->InsertInst(newinst);
            src2 = new MachineOperand(*vreg);
        }
        newinst = new CmpMInstruction(nowblock, 
                CmpMInstruction::VCMP, src1, src2, opcode);
        nowblock->InsertInst(newinst);
        newinst = new VmrsMInstruction(nowblock);
        nowblock->InsertInst(newinst);

        if (opcode >= CmpInstruction::L && opcode <= CmpInstruction::GE)
        {
            MachineOperand *dst = genMachineOperand(operands[0]);
            MachineOperand *trueOperand = genMachineImm(1);
            MachineOperand *falseOperand = genMachineImm(0);
            newinst = new MovMInstruction(nowblock, 
                    MovMInstruction::MOV, dst, trueOperand, opcode);
            nowblock->InsertInst(newinst);
            newinst = new MovMInstruction(nowblock, 
                    MovMInstruction::MOV, dst, falseOperand, 7 - opcode);
            nowblock->InsertInst(newinst);
        }
        else if (opcode == CmpInstruction::E)
        {
            MachineOperand *dst = genMachineOperand(operands[0]);
            MachineOperand *trueOperand = genMachineImm(1);
            MachineOperand *falseOperand = genMachineImm(0);
            newinst = new MovMInstruction(nowblock, 
                    MovMInstruction::MOV, dst, trueOperand, E);
            nowblock->InsertInst(newinst);
            newinst = new MovMInstruction(nowblock, 
                    MovMInstruction::MOV, dst, falseOperand, NE);
            nowblock->InsertInst(newinst);
        }
        else if (opcode == CmpInstruction::NE)
        {
            MachineOperand *dst = genMachineOperand(operands[0]);
            MachineOperand *trueOperand = genMachineImm(1);
            MachineOperand *falseOperand = genMachineImm(0);
            newinst = new MovMInstruction(nowblock, 
                    MovMInstruction::MOV, dst, trueOperand, NE);
            nowblock->InsertInst(newinst);
            newinst = new MovMInstruction(nowblock, 
                    MovMInstruction::MOV, dst, falseOperand, E);
            nowblock->InsertInst(newinst);
        }
    }
    else
    {
        MachineOperand *src1 = genMachineOperand(operands[1]);
        MachineOperand *src2 = genMachineOperand(operands[2]);
        MachineInstruction *newinst = nullptr;
        if (src1->isImm())
        {
            MachineOperand *vreg = genMachineVReg();
            newinst = new LoadMInstruction(nowblock, 
                    LoadMInstruction::LDR, vreg, src1);
            nowblock->InsertInst(newinst);
            src1 = new MachineOperand(*vreg);
        }
        if (src2->isImm() &&
            ((ConstantSymbolEntry *)(operands[2]->getSymbolEntry()))->getIntValue() > 255)
        {
            MachineOperand *vreg = genMachineVReg();
            newinst = new LoadMInstruction(nowblock, 
                    LoadMInstruction::LDR, vreg, src2);
            nowblock->InsertInst(newinst);
            src2 = new MachineOperand(*vreg);
        }
        newinst = new CmpMInstruction(nowblock, CmpMInstruction::CMP, src1, src2, opcode);
        nowblock->InsertInst(newinst);
        if (opcode >= CmpInstruction::L && opcode <= CmpInstruction::GE)
        {
            MachineOperand *dst = genMachineOperand(operands[0]);
            MachineOperand *trueOperand = genMachineImm(1);
            MachineOperand *falseOperand = genMachineImm(0);
            newinst = new MovMInstruction(nowblock, 
                    MovMInstruction::MOV, dst, trueOperand, opcode);
            nowblock->InsertInst(newinst);
            newinst = new MovMInstruction(nowblock, 
                    MovMInstruction::MOV, dst, falseOperand, 7 - opcode);
            nowblock->InsertInst(newinst);
        }
        else if (opcode == CmpInstruction::NE)
        {
            MachineOperand *dst = genMachineOperand(operands[0]);
            MachineOperand *trueOperand = genMachineImm(1);
            MachineOperand *falseOperand = genMachineImm(0);
            newinst = new MovMInstruction(nowblock, 
                    MovMInstruction::MOV, dst, trueOperand, NE);
            nowblock->InsertInst(newinst);
            newinst = new MovMInstruction(nowblock, 
                    MovMInstruction::MOV, dst, falseOperand, E);
            nowblock->InsertInst(newinst);
        }
        else if (opcode == CmpInstruction::E)
        {
            MachineOperand *dst = genMachineOperand(operands[0]);
            MachineOperand *trueOperand = genMachineImm(1);
            MachineOperand *falseOperand = genMachineImm(0);
            newinst = new MovMInstruction(nowblock, 
                    MovMInstruction::MOV, dst, trueOperand, E);
            nowblock->InsertInst(newinst);
            newinst = new MovMInstruction(nowblock, 
                    MovMInstruction::MOV, dst, falseOperand, NE);
            nowblock->InsertInst(newinst);
        }
    }
}

void UncondBrInstruction::genMachineCode(AsmBuilder *builder)
{
    printma("UncondBrInstruction");
    MachineBlock *nowblock = builder->getBlock();
    std::stringstream output;
    output << ".L" << branch->getNo();
    MachineOperand *dst = new MachineOperand(output.str());
    auto newinst = new BranchMInstruction(nowblock, 
            BranchMInstruction::B, dst);
    nowblock->InsertInst(newinst);
}

void CondBrInstruction::genMachineCode(AsmBuilder *builder)
{
    printma("CondBrInstruction");
    std::stringstream output;
    MachineBlock *nowblock = builder->getBlock();
    
    // 增加条件判断为常量的情况
    if (cond->getSymbolEntry()->isConstant())
    {
        ConstantSymbolEntry *cs = dynamic_cast<ConstantSymbolEntry *>
                (cond->getSymbolEntry());
        if (cs->getIntValue() == 1)
        {
            output << ".L" << true_branch->getNo();
            MachineOperand *dst = new MachineOperand(output.str());
            auto newinst = new BranchMInstruction(nowblock, BranchMInstruction::B, dst);
            nowblock->InsertInst(newinst);
        }
        else
        {
            output << ".L" << false_branch->getNo();
            MachineOperand *dst = new MachineOperand(output.str());
            auto newinst = new BranchMInstruction(nowblock, BranchMInstruction::B, dst);
            nowblock->InsertInst(newinst);
        }
    }
    else
    {
        output << ".L" << true_branch->getNo();
        MachineOperand *dst = new MachineOperand(output.str());
        auto newinst = new BranchMInstruction(nowblock, 
                BranchMInstruction::B, dst, nowblock->getCmpCond());
        nowblock->InsertInst(newinst);

        output.str("");
        output << ".L" << false_branch->getNo();
        dst = new MachineOperand(output.str());
        newinst = new BranchMInstruction(nowblock, BranchMInstruction::B, dst);
        nowblock->InsertInst(newinst);
    }
}

void RetInstruction::genMachineCode(AsmBuilder *builder)
{
    /* HINT:
     * 1. Generate mov instruction to save return value in r0
     * 2. Restore callee saved registers and sp, fp
     * 3. Generate bx instruction */
    printma("RetInstruction");
    MachineOperand *dst;
    MachineOperand *src;
    MachineInstruction *newinst;
    MachineBlock *nowblock = builder->getBlock();
    // r0 or s0
    if (!operands.empty())
    {
        if (operands[0]->getType()->isFloat())
        {
            dst = new MachineOperand(MachineOperand::REG, 16, true);
            src = genMachineFloatOperand(operands[0]);
            if (src->isImm())
            {
                MachineOperand *vreg = genMachineVReg();
                newinst = new LoadMInstruction(nowblock, 
                        LoadMInstruction::LDR, vreg, src);
                nowblock->InsertInst(newinst);
                src = vreg;
            }
            newinst = new MovMInstruction(nowblock, MovMInstruction::VMOV, dst, src);
        }
        else
        {
            dst = new MachineOperand(MachineOperand::REG, 0);
            src = genMachineOperand(operands[0]);
            if (operands[0]->getSymbolEntry()->isConstant())
            {
                auto val = dynamic_cast<ConstantSymbolEntry *>
                        (operands[0]->getSymbolEntry())->getIntValue();
                if (val > 255 || val <= -255)
                {
                    MachineOperand *r0 = new MachineOperand(MachineOperand::REG, 0);
                    nowblock->InsertInst(new LoadMInstruction(nowblock, 
                            LoadMInstruction::LDR, r0, src));
                    src = r0;
                }
            }
            newinst = new MovMInstruction(nowblock, MovMInstruction::MOV, dst, src);
        }
        nowblock->InsertInst(newinst);
    }

    // 恢复现场
    MachineOperand *sp = new MachineOperand(MachineOperand::REG, 13);
    MachineOperand *size = new MachineOperand(MachineOperand::IMM, 
                builder->getFunction()->AllocSpace(0));
    nowblock->InsertInst(new BinaryMInstruction(nowblock, 
                BinaryMInstruction::ADD, sp, sp, size));
    // 返回地址
    MachineOperand *lr = new MachineOperand(MachineOperand::REG, 14);
    nowblock->InsertInst(new BranchMInstruction(nowblock, 
                BranchMInstruction::BX, lr));
}

void XorInstruction::genMachineCode(AsmBuilder *builder)
{
    printma("XorInstruction");
    MachineBlock *nowblock = builder->getBlock();
    MachineOperand *dst = genMachineOperand(operands[0]);
    MachineOperand *falseOperand = genMachineImm(0);
    MachineOperand *trueOperand = genMachineImm(1);
    auto newinst = new MovMInstruction(nowblock, 
            MovMInstruction::MOV, dst, trueOperand, MachineInstruction::EQ);
    nowblock->InsertInst(newinst);
    newinst = new MovMInstruction(nowblock, 
            MovMInstruction::MOV, dst, falseOperand, MachineInstruction::NE);
    nowblock->InsertInst(newinst);
}

void ZextInstruction::genMachineCode(AsmBuilder *builder)
{
    printma("ZextInstruction");
    MachineBlock *nowblock = builder->getBlock();
    MachineOperand *dst = genMachineOperand(operands[0]);
    MachineOperand *src = genMachineOperand(operands[1]);
    auto newinst = new MovMInstruction(nowblock, 
            MovMInstruction::MOV, dst, src);
    nowblock->InsertInst(newinst);
}

void GepInstruction::genMachineCode(AsmBuilder *builder)
{
    printma("GepInstruction");
    MachineBlock *nowblock = builder->getBlock();
    MachineInstruction *newinst;
    MachineOperand *dst;
    MachineOperand *idx;
    dst = genMachineOperand(operands[0]);
    // if(idxFlag)
    idx = genMachineOperand(operands[2]);

    int size;
    MachineOperand *base = nullptr;
    MachineOperand *ld_dst_op = genMachineVReg();
    // 无idx
    if (!idxFlag)
    {
        idx = genMachineImm(0);
        // std::cout<<"@@ idx = "<<idx->getVal()<<"\n";
    }

    if (idx->isImm())
    {
        if (idx->getVal() < 255)
        {
            newinst = new MovMInstruction(nowblock, 
                    MovMInstruction::MOV, ld_dst_op, idx);
        }
        else
        {
            newinst = new LoadMInstruction(nowblock, 
                    LoadMInstruction::LDR, ld_dst_op, idx);
        }
        idx = new MachineOperand(*ld_dst_op);
        nowblock->InsertInst(newinst);
    }

    ArrayType *type;
    if (operands[1]->getType()->isPointer())
    {
        type = (ArrayType *)(dynamic_cast<PointerType *>
                (operands[1]->getType()))->getValueType();
        size = type->getEleType()->getSize() / 8;
    }
    else if (operands[1]->getType()->isArray())
    {
        type = (ArrayType *)(operands[1]->getType());
        type->setSize();
        size = type->getSize() / 8;
    }
    else
    {
        size = operands[1]->getType()->getSize() / 8;
    }

    if (!param && idxFirst)
    {
        base = genMachineVReg();
        if (operands[1]->getSymbolEntry()->isVariable() &&
            ((IdentifierSymbolEntry *)(operands[1]->getSymbolEntry()))->isGlobal())
        {
            MachineOperand *src = genMachineOperand(operands[1]);
            newinst = new LoadMInstruction(nowblock, 
                    LoadMInstruction::LDR, base, src);
        }
        else
        {
            int offset = ((TemporarySymbolEntry *)(operands[1]->getSymbolEntry()))->getOffset();
            // std::cout<<"offset = "<<offset<<" \n";
            // 偏移大于255重新load
            if (offset > -255 && offset < 255)
            {
                newinst = new MovMInstruction(nowblock, 
                        MovMInstruction::MOV, base, genMachineImm(offset));
            }
            else
            {
                newinst = new LoadMInstruction(nowblock, 
                        LoadMInstruction::LDR, base, genMachineImm(offset));
            }
        }
        nowblock->InsertInst(newinst);
    
        MachineOperand *size_ = genMachineVReg();
        if (size > -255 && size < 255)
        {
            newinst = new MovMInstruction(nowblock, 
                    MovMInstruction::MOV, size_, genMachineImm(size));
        }
        else
        {
            newinst = new LoadMInstruction(nowblock, 
                    LoadMInstruction::LDR, size_, genMachineImm(size));
        }
    
        nowblock->InsertInst(newinst);
        MachineOperand *off = genMachineVReg();
        newinst = new BinaryMInstruction(nowblock, 
                BinaryMInstruction::MUL, off, idx, size_);
        off = new MachineOperand(*off);
        nowblock->InsertInst(newinst);

        MachineOperand *addr = genMachineVReg();
        MachineOperand *base_ = new MachineOperand(*base);
        newinst = new BinaryMInstruction(nowblock, 
                BinaryMInstruction::ADD, addr, base_, off);
        nowblock->InsertInst(newinst);
        addr = new MachineOperand(*addr);
        if (operands[1]->getSymbolEntry()->isVariable() &&
            dynamic_cast<IdentifierSymbolEntry*>
            (operands[1]->getSymbolEntry())->isGlobal())
        {
            newinst =
                new MovMInstruction(nowblock, 
                        MovMInstruction::MOV, dst, addr);
        }
        else
        {
            MachineOperand *fp = genMachineReg(11);
            newinst = new BinaryMInstruction(nowblock, 
                    BinaryMInstruction::ADD, dst, fp, addr);
        }
        nowblock->InsertInst(newinst);
    }
    else
    {    
        MachineOperand *size_ = genMachineVReg();
        if (size > -255 && size < 255)
        {
            newinst = new MovMInstruction(nowblock, 
                    MovMInstruction::MOV, size_, genMachineImm(size));
        }
        else
        {
            newinst = new LoadMInstruction(nowblock, 
                    LoadMInstruction::LDR, size_, genMachineImm(size));
        }
    
        nowblock->InsertInst(newinst);
        MachineOperand *off = genMachineVReg();
        newinst = new BinaryMInstruction(nowblock, 
                BinaryMInstruction::MUL, off, idx, size_);
        off = new MachineOperand(*off);
        nowblock->InsertInst(newinst);
        MachineOperand *arr = genMachineOperand(operands[1]);
        newinst = new BinaryMInstruction(nowblock, 
                BinaryMInstruction::ADD, dst, arr, off);
        nowblock->InsertInst(newinst);

    }
}

void FuncCallInstruction::genMachineCode(AsmBuilder *builder)
{
    printma("FuncCallInstruction");
    MachineBlock *nowblock = builder->getBlock();
    MachineOperand *operand;
    MachineInstruction *newinst;
    int idx = 0;
    int push_num = 0;

    // 数组赋值优化
    if (dynamic_cast<IdentifierSymbolEntry *>(se)->getName() == "memset")
    {
        MachineOperand *r0 = genMachineReg(0);
        MachineOperand *r1 = genMachineReg(1);
        MachineOperand *r2 = genMachineReg(2);
        auto tmp = operands[1];
        auto bitcast = (BitcastInstruction *)(tmp->getDef());
        if (!bitcast->getFlag())
        {
            MachineOperand *fp = genMachineReg(11);
            TemporarySymbolEntry* tmp =(TemporarySymbolEntry *)
                    (bitcast->getUse()[0]->getSymbolEntry());
            int offset = tmp->getOffset();
            operand = genMachineVReg();
            if (offset > -255 && offset < 255)
            {
                nowblock->InsertInst(new BinaryMInstruction(nowblock, 
                        BinaryMInstruction::ADD, r0, fp, genMachineImm(offset)));
            }
            else
            {
                newinst = new LoadMInstruction(nowblock, 
                        LoadMInstruction::LDR, operand, genMachineImm(offset));
                operand = new MachineOperand(*operand);
                nowblock->InsertInst(newinst);
                nowblock->InsertInst(new BinaryMInstruction(nowblock, 
                        BinaryMInstruction::ADD, r0, fp, operand));
            }
        }
        else
        {
            nowblock->InsertInst(new MovMInstruction(nowblock, 
                        MovMInstruction::MOV, r0,genMachineOperand(bitcast->getUse()[0])));
        }
        nowblock->InsertInst(new MovMInstruction(nowblock, 
                MovMInstruction::MOV, r1, genMachineImm(0)));
        MachineOperand *len = genMachineOperand(operands[3]);
        if (len->isImm() && len->getVal() > 255)
        {
            operand = genMachineVReg();
            newinst = new LoadMInstruction(nowblock, 
                    LoadMInstruction::LDR, operand, len);
            operand = new MachineOperand(*operand);
            nowblock->InsertInst(newinst);
        }
        else{
            operand = len;
        }
            nowblock->InsertInst(new MovMInstruction(nowblock, 
                        MovMInstruction::MOV, r2, operand));
            nowblock->InsertInst(new BranchMInstruction(nowblock, 
                        BranchMInstruction::BL, new MachineOperand("@memset")));

        return;
    }

    // 一般函数调用

    // 计算对齐
    bool align_flag = false;
    std::vector<MachineOperand *> vec;

    // int 跟 float 分开算！！！！
    int intCnt = 0, floatCnt = 0 ,stackCnt=0;
    for (size_t i = 1; i < operands.size(); i++)
    {
        if (operands[i]->getType()->isFloat())
        {
            floatCnt++;
        }
        else
        {
            intCnt++;
        }
    }

    if (floatCnt > 4)
        push_num += floatCnt - 4;
    if (intCnt > 4)
        push_num += intCnt - 4;
    if (push_num % 2 != 0) 
        align_flag = true;

    // int
    int regCnt = 1;
    int op_size = (int)operands.size();
    for (idx = 1; idx < op_size; idx++)
    {
        if (regCnt == 5) 
            break;
        if (operands[idx]->getType()->isFloat())
            continue;
        operand = genMachineReg(regCnt - 1);
        MachineOperand *src = genMachineOperand(operands[idx]);
        if (src->isImm() && src->getVal() > 255)
        {
            newinst = new LoadMInstruction(nowblock, 
                    LoadMInstruction::LDR, operand, src);
        }
        else
        {
            newinst = new MovMInstruction(nowblock, 
                    MovMInstruction::MOV, operand, src);
        }
        nowblock->InsertInst(newinst);
        regCnt++;
    }

    // float
    int int_idx = idx;

    int fpregCnt = 1;
    for (idx = 1; idx < op_size; idx++)
    {
        if (fpregCnt == 5 && !align_flag)
            break;
        if (fpregCnt == 6 && align_flag) {
            break;
        }
        if (!operands[idx]->getType()->isFloat())continue;
        operand = genMachineFReg(fpregCnt - 1);
        MachineOperand *src = genMachineFloatOperand(operands[idx]);
        if (src->isImm())
        {
            MachineOperand *vreg = genMachineVReg();
            newinst = new LoadMInstruction(nowblock, 
                    LoadMInstruction::LDR, vreg, src);
            nowblock->InsertInst(newinst);
            vreg = new MachineOperand(*vreg);
            newinst = new MovMInstruction(nowblock, 
                    MovMInstruction::VMOV, operand, vreg);
        }
        else
        {
            newinst = new MovMInstruction(nowblock, 
                    MovMInstruction::VMOV, operand, src);
        }
        nowblock->InsertInst(newinst);
        fpregCnt++;
    }

    int float_idx = idx;

    idx = std::min(float_idx, int_idx);

    // 参数数目>4
    for (int i = (int)operands.size() - 1; i >= idx; i--)
    {
        if (operands[i]->getType()->isFloat() && i >= float_idx)
        {
            operand = genMachineFloatOperand(operands[i]);
            if (operand->isImm())
            {
                MachineOperand *dst = genMachineVReg(true);
                MachineOperand *vreg = genMachineVReg();
                newinst = new LoadMInstruction(nowblock, 
                        LoadMInstruction::LDR, vreg, operand);
                nowblock->InsertInst(newinst);
                vreg = new MachineOperand(*vreg);
                newinst = new MovMInstruction(nowblock, 
                        MovMInstruction::VMOV, dst, vreg);
                nowblock->InsertInst(newinst);
                operand = new MachineOperand(*dst);
            }
            newinst = new StackMInstruction(nowblock, 
                        StackMInstruction::VPUSH, vec, operand);
            nowblock->InsertInst(newinst);
            stackCnt++;
        }
        else if (!operands[i]->getType()->isFloat() && i >= int_idx)
        {
            operand = genMachineOperand(operands[i]);
            if (operand->isImm())
            {
                MachineOperand *dst = genMachineVReg();
                if (operand->getVal() < 256)
                {
                    newinst = new MovMInstruction(nowblock, 
                            MovMInstruction::MOV, dst, operand);
                }
                else
                {
                    newinst = new LoadMInstruction(nowblock, 
                            LoadMInstruction::LDR, dst, operand);
                }
                nowblock->InsertInst(newinst);
                operand = new MachineOperand(*dst);
            }
            newinst = new StackMInstruction(nowblock, 
                    StackMInstruction::PUSH, vec, operand);
            nowblock->InsertInst(newinst);
            stackCnt++;
        }
    }

    // 切换现场
    MachineOperand* label = new MachineOperand(se->toStr().c_str());
    newinst = new BranchMInstruction(nowblock, 
            BranchMInstruction::BL, label);
    nowblock->InsertInst(newinst);
    // 调整栈帧
    if (stackCnt != 0 &&(regCnt >= 5 || fpregCnt >= 5))
    {
        auto off = genMachineImm(stackCnt * 4);
        MachineOperand *sp = new MachineOperand(MachineOperand::REG, 13);
        newinst = new BinaryMInstruction(nowblock, 
                BinaryMInstruction::ADD, sp, sp, off);
        nowblock->InsertInst(newinst);
    }

    // 获取函数返回值
    if (dst!=nullptr)
    {
        if (dst->getType()->isFloat())
        {
            operand = genMachineFloatOperand(dst);
            MachineOperand *s0 = new MachineOperand(MachineOperand::REG, 16, true);
            newinst = new MovMInstruction(nowblock, 
                    MovMInstruction::VMOV, operand, s0);
            nowblock->InsertInst(newinst);
        }
        else
        {
            operand = genMachineOperand(dst);
            MachineOperand *r0 = new MachineOperand(MachineOperand::REG, 0);
            newinst = new MovMInstruction(nowblock, 
                    MovMInstruction::MOV, operand, r0);
            nowblock->InsertInst(newinst);
        }
    }
}

BitcastInstruction::BitcastInstruction(
    Operand *dst, Operand *src,BasicBlock *insert_bb)
    : Instruction(BITCAST, insert_bb), dst(dst), src(src)
{
    operands.emplace_back(dst);
    operands.emplace_back(src);
    dst->setDef(this);
    dst->addUse(this);
    src->addUse(this);
    flag = false;
}

void BitcastInstruction::output() const
{
    Operand *dst = operands[0];
    Operand *src = operands[1];
    fprintf(yyout, "  %s = bitcast %s %s to %s\n", 
            dst->toStr().c_str(),
            src->getType()->toStr().c_str(), 
            src->toStr().c_str(),
            dst->getType()->toStr().c_str());
}

BitcastInstruction::~BitcastInstruction()
{
    operands[0]->setDef(nullptr);
    if (operands[0]->usersNum() == 0)
        delete operands[0];
    operands[1]->removeUse(this);
}

void BitcastInstruction::genMachineCode(AsmBuilder *builder)
{
    printma("BitcastInstruction");
    // auto ptr = (PointerType*)(dst->getType());
    // auto type = ptr->getValueType();
}

MachineOperand *Instruction::genMachineFloatOperand(Operand *op)
{
    SymbolEntry *se = op->getSymbolEntry();
    if (!op->getType()->isFloat())
    {
        return genMachineOperand(op);
    }
    MachineOperand *newop = nullptr;
    if (op->isConstant())
    {
        newop = new MachineOperand(MachineOperand::IMM,
            (float)dynamic_cast<ConstantSymbolEntry *>(se)->getFloatValue());
    }
    else if (op->isTemporary())
    {
        newop = new MachineOperand(MachineOperand::VREG,
            dynamic_cast<TemporarySymbolEntry *>(se)->getLabel(), true);
    }
    else if (op->isVariable())
    {
        IdentifierSymbolEntry *id = dynamic_cast<IdentifierSymbolEntry *>(se);
        if (id->isGlobal())
        {
            newop = new MachineOperand(id->toStr().c_str());
        }
        else if (id->isParam())
        {
            auto no = id->getParamNo();
            if (id->getParamNo() < 4)
            {
                newop = new MachineOperand(MachineOperand::REG, id->getParamNo() + 16, true);
            }
            else
            {
                newop = new MachineOperand(MachineOperand::REG, 20, true);
                newop->setParam();
                newop->setParamNo(no);
            }
            newop->setAllParamNo(id->getStackParamNo());
        }
    }
    return newop;
}
PhiInstruction::PhiInstruction(Operand* dst, BasicBlock* insert_bb)
    : Instruction(PHI, insert_bb) {
    operands.emplace_back(dst);
    this->dst = dst;
    this->originDef = dst;
    dst->setDef(this);
}

PhiInstruction::~PhiInstruction() {
    dst->setDef(nullptr);
    if (dst->usersNum() == 0)
        delete dst;
    for (auto it : srcs)
        it.second->removeUse(this);
}

void PhiInstruction::output() const {
    fprintf(yyout, "  %s = phi %s", dst->toStr().c_str(),
            dst->getType()->toStr().c_str());
    bool first = true;
    for (auto it = srcs.begin(); it != srcs.end(); it++) {
        if (!first)
            fprintf(yyout, ", ");
        else
            first = false;
        fprintf(yyout, "[ %s , %%B%d ]", it->second->toStr().c_str(),
                it->first->getNo());
    }
    fprintf(yyout, "\n");
}


FptosiInstruction::FptosiInstruction(
    Operand *dst, Operand *src, BasicBlock *insert_bb)
    : Instruction(FPTOSI, insert_bb), dst(dst), src(src)
{
    operands.emplace_back(dst);
    operands.emplace_back(src);
    dst->setDef(this);
    // dst->addUse(this);
    src->addUse(this);
}

void FptosiInstruction::genMachineCode(AsmBuilder *builder)
{
    MachineInstruction *newinst;
    MachineBlock *nowblock = builder->getBlock();

    MachineOperand *src_op = genMachineFloatOperand(src);
    MachineOperand *dst_op = genMachineOperand(dst);

    if (src_op->isImm())
    {
        MachineOperand *tmp = genMachineVReg(true);
        MachineOperand *vreg_ = genMachineVReg();
        newinst = new LoadMInstruction(nowblock, 
                LoadMInstruction::LDR, vreg_, src_op);
        nowblock->InsertInst(newinst);
        vreg_ = new MachineOperand(*vreg_);
        newinst = new MovMInstruction(nowblock, 
                MovMInstruction::VMOV, tmp, vreg_);
        nowblock->InsertInst(newinst);
        src_op = tmp;
    }
    MachineOperand *vcvtDst = genMachineVReg(true);
    newinst = new VcvtMInstruction(nowblock, 
            VcvtMInstruction::F2S, vcvtDst, src_op);
    nowblock->InsertInst(newinst);
    MachineOperand *movUse = new MachineOperand(*vcvtDst);
    newinst = new MovMInstruction(nowblock, 
            MovMInstruction::VMOV, dst_op, movUse);

    nowblock->InsertInst(newinst);
}

void FptosiInstruction::output() const
{
    Operand *dst = operands[0];
    Operand *src = operands[1];
    fprintf(yyout, "  %s = fptosi %s %s to %s\n", 
            dst->toStr().c_str(),
            src->getType()->toStr().c_str(), 
            src->toStr().c_str(),
            dst->getType()->toStr().c_str());
}

FptosiInstruction::~FptosiInstruction()
{
    operands[0]->setDef(nullptr);
    if (operands[0]->usersNum() == 0)
        delete operands[0];
    operands[1]->removeUse(this);
}

SitofpInstruction::SitofpInstruction(
    Operand *dst, Operand *src, BasicBlock *insert_bb)
    : Instruction(FPTOSI, insert_bb), dst(dst), src(src)
{
    operands.emplace_back(dst);
    operands.emplace_back(src);
    dst->setDef(this);
    // dst->addUse(this);
    src->addUse(this);
}

void SitofpInstruction::genMachineCode(AsmBuilder *builder)
{
    MachineInstruction *newinst;
    MachineBlock *nowblock = builder->getBlock();

    MachineOperand *src_op = genMachineOperand(src);

    if (src_op->isImm())
    {
        MachineOperand *tmp = genMachineVReg();
        newinst = new LoadMInstruction(nowblock, 
                LoadMInstruction::LDR, tmp, src_op);
        nowblock->InsertInst(newinst);
        src_op = new MachineOperand(*tmp);
    }
    MachineOperand *movDst = genMachineVReg(true);
    newinst = new MovMInstruction(nowblock, 
                MovMInstruction::VMOV, movDst, src_op);
    nowblock->InsertInst(newinst);
    MachineOperand *vcvtUse = new MachineOperand(*movDst);
    MachineOperand *dst_op = genMachineFloatOperand(dst);
    newinst = new VcvtMInstruction(nowblock, 
                VcvtMInstruction::S2F, dst_op, vcvtUse);
    nowblock->InsertInst(newinst);
}

void SitofpInstruction::output() const
{
    Operand *dst = operands[0];
    Operand *src = operands[1];
    fprintf(yyout, "  %s = sitofp %s %s to %s\n", 
            dst->toStr().c_str(),
            src->getType()->toStr().c_str(), 
            src->toStr().c_str(),
            dst->getType()->toStr().c_str());
}

SitofpInstruction::~SitofpInstruction()
{
    operands[0]->setDef(nullptr);
    if (operands[0]->usersNum() == 0)
        delete operands[0];
    operands[1]->removeUse(this);
}

void BinaryInstruction::replaceUse(Operand* old, Operand* new_) {
    if (operands[1] == old) {
        operands[1]->removeUse(this);
        operands[1] = new_;
        new_->addUse(this);
    } else if (operands[2] == old) {
        operands[2]->removeUse(this);
        operands[2] = new_;
        new_->addUse(this);
    }
}

void BinaryInstruction::replaceDef(Operand* new_) {
    operands[0]->removeDef(this);
    operands[0] = new_;
    new_->setDef(this);
}

void CmpInstruction::replaceUse(Operand* old, Operand* new_) {
    if (operands[1] == old) {
        operands[1]->removeUse(this);
        operands[1] = new_;
        new_->addUse(this);
    } else if (operands[2] == old) {
        operands[2]->removeUse(this);
        operands[2] = new_;
        new_->addUse(this);
    }
}

void CmpInstruction::replaceDef(Operand* new_) {
    operands[0]->removeDef(this);
    operands[0] = new_;
    new_->setDef(this);
}

void CondBrInstruction::replaceUse(Operand* old, Operand* new_) {
    if (operands[0] == old) {
        operands[0]->removeUse(this);
        operands[0] = new_;
        new_->addUse(this);
    }
}

void RetInstruction::replaceDef(Operand* new_) {
    if (operands.size()) {
        operands[0]->removeDef(this);
        operands[0] = new_;
        new_->setDef(this);
    }
}

void RetInstruction::replaceUse(Operand* old, Operand* new_) {
    if (operands.size() && operands[0] == old) {
        operands[0]->removeUse(this);
        operands[0] = new_;
        new_->addUse(this);
    }
}

void AllocaInstruction::replaceDef(Operand* new_) {
    operands[0]->removeDef(this);
    operands[0] = new_;
    new_->setDef(this);
}

void LoadInstruction::replaceDef(Operand* new_) {
    operands[0]->removeDef(this);
    operands[0] = new_;
    new_->setDef(this);
}

void LoadInstruction::replaceUse(Operand* old, Operand* new_) {
    if (operands[1] == old) {
        operands[1]->removeUse(this);
        operands[1] = new_;
        new_->addUse(this);
    }
}

void StoreInstruction::replaceUse(Operand* old, Operand* new_) {
    if (operands[0] == old) {
        operands[0]->removeUse(this);
        operands[0] = new_;
        new_->addUse(this);
    } else if (operands[1] == old) {
        operands[1]->removeUse(this);
        operands[1] = new_;
        new_->addUse(this);
    }
}

void FuncCallInstruction::replaceDef(Operand* new_) {
    if (dst) {
        operands[0]->removeDef(this);
        operands[0] = new_;
        new_->setDef(this);
        dst = new_;
    }
}

void FuncCallInstruction::replaceUse(Operand* old, Operand* new_) {
    for (int i = 1; i < (int)operands.size(); i++)
        if (operands[i] == old) {
            operands[i]->removeUse(this);
            operands[i] = new_;
            new_->addUse(this);
        }
}

void ZextInstruction::replaceDef(Operand* new_) {
    operands[0]->removeDef(this);
    operands[0] = new_;
    new_->setDef(this);
}

void ZextInstruction::replaceUse(Operand* old, Operand* new_) {
    if (operands[1] == old) {
        operands[1]->removeUse(this);
        operands[1] = new_;
        new_->addUse(this);
    }
}

void XorInstruction::replaceDef(Operand* new_) {
    operands[0]->removeDef(this);
    operands[0] = new_;
    new_->setDef(this);
}

void XorInstruction::replaceUse(Operand* old, Operand* new_) {
    if (operands[1] == old) {
        operands[1]->removeUse(this);
        operands[1] = new_;
        new_->addUse(this);
    }
}

void GepInstruction::replaceDef(Operand* new_) {
    operands[0]->removeDef(this);
    operands[0] = new_;
    new_->setDef(this);
}

void GepInstruction::replaceUse(Operand* old, Operand* new_) {
    if (operands[1] == old) {
        operands[1]->removeUse(this);
        operands[1] = new_;
        new_->addUse(this);
    } else if (operands[2] == old) {
        operands[2]->removeUse(this);
        operands[2] = new_;
        new_->addUse(this);
    }
}

void PhiInstruction::replaceUse(Operand* old, Operand* new_) {
    for (auto& it : srcs) {
        if (it.second == old) {
            it.second->removeUse(this);
            it.second = new_;
            new_->addUse(this);
        }
    }
    for (auto it = operands.begin() + 1; it != operands.end(); it++)
        if (*it == old)
            *it = new_;
}

void PhiInstruction::replaceDef(Operand* new_) {
    dst->removeDef(this);
    dst = new_;
    new_->setDef(this);
}

void PhiInstruction::addSrc(BasicBlock* block, Operand* src) {
    operands.emplace_back(src);
    srcs.insert(std::make_pair(block, src));
    src->addUse(this);
}

void SitofpInstruction::replaceUse(Operand* old, Operand* new_) {
    if (operands[1] == old) {
        operands[1]->removeUse(this);
        operands[1] = new_;
        new_->addUse(this);
        src = new_;
    }
}

void FptosiInstruction::replaceUse(Operand* old, Operand* new_) {
    if (operands[1] == old) {
        operands[1]->removeUse(this);
        operands[1] = new_;
        new_->addUse(this);
        src = new_;
    }
}

void FptosiInstruction::replaceDef(Operand* new_) {
    operands[0]->removeDef(this);
    operands[0] = new_;
    new_->setDef(this);
}

void SitofpInstruction::replaceDef(Operand* new_) {
    operands[0]->removeDef(this);
    operands[0] = new_;
    new_->setDef(this);
}

void BitcastInstruction::replaceUse(Operand* old, Operand* new_) {
    if (operands[1] == old) {
        operands[1]->removeUse(this);
        operands[1] = new_;
        new_->addUse(this);
    }
}
