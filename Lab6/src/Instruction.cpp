#include "Instruction.h"
#include "BasicBlock.h"
#include <iostream>
#include "Function.h"
#include "Type.h"
#include <sstream>
#include "Ast.h"
extern FILE* yyout;

FuncCallInstruction::FuncCallInstruction(Operand* dst, SymbolEntry* se, std::vector<Operand*> rParams, BasicBlock* insert_bb) : Instruction(CALL, insert_bb)
{
    printg("== FuncCallInstruction ==");
    this -> se = se;
    operands.push_back(dst);
    if(dst)
    {
        dst -> setDef(this);
    }
    for(auto param : rParams)
    {
        operands.push_back(param);
        param -> addUse(this);
    }
}

void FuncCallInstruction::output() const
{
    fprintf(yyout, "  ");
    FunctionType* type = (FunctionType*) (se -> getType());
    if(operands[0] && type -> getRetType() != TypeSystem::voidType)
    {
        fprintf(yyout, "%s = ", operands[0] -> toStr().c_str());
    }
    fprintf(yyout, "call %s %s(", type -> getRetType() -> toStr().c_str(), se -> toStr().c_str());
    for(long unsigned int i = 1; i < operands.size(); i++)
    {
        if(i != 1)
        {
            fprintf(yyout, ", ");
        }
        fprintf(yyout, "%s %s", operands[i] -> getType() -> toStr().c_str(), operands[i] -> toStr().c_str());
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

BasicBlock *Instruction::getParent()
{
    return parent;
}

void Instruction::setParent(BasicBlock *bb)
{
    parent = bb;
}

void Instruction::setNext(Instruction *inst)
{
    next = inst;
}

void Instruction::setPrev(Instruction *inst)
{
    prev = inst;
}

Instruction *Instruction::getNext()
{
    return next;
}

Instruction *Instruction::getPrev()
{
    return prev;
}

XorInstruction::XorInstruction(Operand* dst, Operand* src, BasicBlock* insert_bb): Instruction(XOR, insert_bb){
    printg("== XorInstruction ==");
    operands.push_back(dst);
    operands.push_back(src);
    dst->setDef(this);
    src->addUse(this);
}

XorInstruction::~XorInstruction(){
    operands[0]->setDef(nullptr);
    if(operands[0]->usersNum() == 0)
        delete operands[0];
    operands[1]->removeUse(this);
}

void XorInstruction::output() const{
    Operand* dst = operands[0];
    Operand* src = operands[1];
    fprintf(yyout, "  %s = xor %s %s, true\n", dst->toStr().c_str(), src->getType()->toStr().c_str(), src->toStr().c_str());
}

BinaryInstruction::BinaryInstruction(unsigned opcode, Operand *dst, Operand *src1, Operand *src2, BasicBlock *insert_bb) : Instruction(BINARY, insert_bb)
{
    printg("== BinaryInstruction ==");
    this->opcode = opcode;
    operands.push_back(dst);
    operands.push_back(src1);
    operands.push_back(src2);
    dst->setDef(this);
    src1->addUse(this);
    src2->addUse(this); 
}

BinaryInstruction::~BinaryInstruction()
{
    operands[0]->setDef(nullptr);
    if(operands[0]->usersNum() == 0)
        delete operands[0];
    operands[1]->removeUse(this);
    operands[2]->removeUse(this);
}

void BinaryInstruction::output() const
{
    std::string s1, s2, s3, op, type;
    int v2,v3;
    bool flag1(false), flag2(false);
    printg("+Binary+");
    s1 = operands[0]->toStr();
    if(operands[1]->getSymbolEntry()->isConstant()){
        ConstantSymbolEntry* cs  = dynamic_cast<ConstantSymbolEntry*>(operands[1]->getSymbolEntry());
        v2 = cs->getIntValue();
        flag1 = true;
    }else{
        s2 = operands[1]->toStr();
    }
    if(operands[2]->getSymbolEntry()->isConstant()){
        ConstantSymbolEntry* cs  = dynamic_cast<ConstantSymbolEntry*>(operands[2]->getSymbolEntry());
        v3 = cs->getIntValue();
        flag2 = true;
    }else{
        s3 = operands[2]->toStr();
    }
    type = operands[0]->getType()->toStr();
    switch (opcode)
    {
    case ADD:
        op = "add";
        break;
    case SUB:
        op = "sub";
        break;
    case MOD:
        op = "srem";
        break;
    case DIV:
        op = "sdiv";
        break;
    case MUL:
        op = "mul";
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
    if(flag1 && flag2){
        fprintf(yyout, "  %s = %s %s %d, %d\n", s1.c_str(), op.c_str(), type.c_str(), v2, v3);
    }else if(flag1 && !flag2){
        fprintf(yyout, "  %s = %s %s %d, %s\n", s1.c_str(), op.c_str(), type.c_str(), v2, s3.c_str());
    }else if(!flag1 && flag2){
        fprintf(yyout, "  %s = %s %s %s, %d\n", s1.c_str(), op.c_str(), type.c_str(), s2.c_str(), v3);
    }else{
        fprintf(yyout, "  %s = %s %s %s, %s\n", s1.c_str(), op.c_str(), type.c_str(), s2.c_str(), s3.c_str());
    }
}

CmpInstruction::CmpInstruction(unsigned opcode, Operand *dst, Operand *src1, Operand *src2, BasicBlock *insert_bb): Instruction(CMP, insert_bb){
    printg("== CmpInstruction ==");
    this->opcode = opcode;
    
    operands.push_back(dst);
    operands.push_back(src1);
    operands.push_back(src2);
    dst->setDef(this);
    src1->addUse(this);
    src2->addUse(this);
}

CmpInstruction::~CmpInstruction()
{
    operands[0]->setDef(nullptr);
    if(operands[0]->usersNum() == 0)
        delete operands[0];
    operands[1]->removeUse(this);
    operands[2]->removeUse(this);
}

void CmpInstruction::output() const
{
    std::string s1, s2, s3, op, type;
    printg("+Cmp+");
    s1 = operands[0]->toStr();
    s2 = operands[1]->toStr();
    s3 = operands[2]->toStr();
    type = operands[1]->getType()->toStr();
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

    fprintf(yyout, "  %s = icmp %s %s %s, %s\n", s1.c_str(), op.c_str(), type.c_str(), s2.c_str(), s3.c_str());
}

UncondBrInstruction::UncondBrInstruction(BasicBlock *to, BasicBlock *insert_bb) : Instruction(UNCOND, insert_bb)
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

CondBrInstruction::CondBrInstruction(BasicBlock*true_branch, BasicBlock*false_branch, Operand *cond, BasicBlock *insert_bb) : Instruction(COND, insert_bb){
    printg("== CondBrInstruction ==");
    this->true_branch = true_branch;
    this->false_branch = false_branch;
    cond->addUse(this);
    operands.push_back(cond);
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
    int true_label = true_branch->getNo();
    int false_label = false_branch->getNo();
    type = operands[0]->getType()->toStr();
    if(operands[0]->getSymbolEntry()->isConstant()){
        ConstantSymbolEntry* cs  = dynamic_cast<ConstantSymbolEntry*>(operands[0]->getSymbolEntry());
        value = cs->getIntValue();
        fprintf(yyout, "  br %s %d, label %%B%d, label %%B%d\n", type.c_str(), value, true_label, false_label);
    }else{
        cond = operands[0]->toStr();
        fprintf(yyout, "  br %s %s, label %%B%d, label %%B%d\n", type.c_str(), cond.c_str(), true_label, false_label);
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

RetInstruction::RetInstruction(Operand *src, BasicBlock *insert_bb) : Instruction(RET, insert_bb)
{
    if(src != nullptr)
    {
        operands.push_back(src);
        src->addUse(this);
    }
}

RetInstruction::~RetInstruction()
{
    if(!operands.empty())
        operands[0]->removeUse(this);
}

void RetInstruction::output() const
{
    printg("+RetInstruction+");
    if(operands.empty())
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

AllocaInstruction::AllocaInstruction(Operand *dst, SymbolEntry *se, BasicBlock *insert_bb) : Instruction(ALLOCA, insert_bb)
{
    printg("== AllocaInstruction ==");
    operands.push_back(dst);
    dst->setDef(this);
    this->se = se;
}

AllocaInstruction::~AllocaInstruction()
{
    operands[0]->setDef(nullptr);
    if(operands[0]->usersNum() == 0)
        delete operands[0];
}

void AllocaInstruction::output() const
{
    std::string dst, type;
    printg("+AllocaInstruction+");
    dst = operands[0]->toStr();
    
    type = se->getType()->toStr();
    if(se->getType()->isArray()){
        ArrayType* arrType = dynamic_cast<ArrayType*>(se->getType());
        if(arrType->getLenVec()[0]==0){
            type = (new PointerType(arrType->getEleType()))->toStr();
        }
        else 
            type = arrType->toStr();
    }
    fprintf(yyout, "  %s = alloca %s, align 4\n", dst.c_str(), type.c_str());
}

LoadInstruction::LoadInstruction(Operand *dst, Operand *src_addr, BasicBlock *insert_bb,int align) : Instruction(LOAD, insert_bb)
{
    printg("== LoadInstruction ==");
    this->align=align;
    operands.push_back(dst);
    operands.push_back(src_addr);
    dst->setDef(this);
    src_addr->addUse(this);
    printinfo("load instruction");
}

LoadInstruction::~LoadInstruction()
{
    operands[0]->setDef(nullptr);
    if(operands[0]->usersNum() == 0)
        delete operands[0];
    operands[1]->removeUse(this);
}

void LoadInstruction::output() const
{
    printg("+Load+");
    std::string dst = operands[0]->toStr();
    std::string src = operands[1]->toStr();
    std::string src_type;
    std::string dst_type;
    dst_type = operands[0]->getType()->toStr();
    src_type = operands[1]->getType()->toStr();
    if(operands[1]->getType()->isArray()){
        Type * type = dynamic_cast<ArrayType*>(operands[1]->getType())
                ->getEleType();
        src_type = (new PointerType(type))->toStr();
    }
    
    fprintf(yyout, "  %s = load %s, %s %s, align %d\n", dst.c_str(), dst_type.c_str(), src_type.c_str(), src.c_str(),align);
}

StoreInstruction::StoreInstruction(Operand *dst_addr, Operand *src, BasicBlock *insert_bb,int align) : Instruction(STORE, insert_bb)
{
    printg("== StoreInstruction ==");
    this->align=align;
    operands.push_back(dst_addr);
    operands.push_back(src);
    dst_addr->addUse(this);
    src->addUse(this);
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

    fprintf(yyout, "  store %s %s, %s %s, align %d\n", src_type.c_str(), src.c_str(), dst_type.c_str(), dst.c_str(),align);
}
GepInstruction::~GepInstruction()
{
    // operands[0]->removeUse(this);
    // operands[1]->removeUse(this);
}
GepInstruction::GepInstruction(Operand* dst,Operand* src,BasicBlock* insert_bb,Operand* idx) : Instruction(GEP, insert_bb){
    printg("== GepInstruction ==");
    operands.push_back(dst);
    operands.push_back(src);
    operands.push_back(idx);
    dst->setDef(this);
    src->addUse(this);
    if(idx!=nullptr)
        idx->addUse(this);
}

void GepInstruction::output() const {
    printg("+Gep+");
    Operand* dst = operands[0];
    Operand* src = operands[1];
    Operand* idx = operands[2];
    ArrayType* arrType = (ArrayType*)dst->getType();
    std::string buffer_str1;
    std::string buffer_str2;
    std::string index;
    if(idx!=nullptr){
        index = idx->toStr();
    }else{
        index="0";
    }
    // z1214 todo 或许浮点得改
    bool flag = 0;
    Type* basicType = arrType->getEleType();
    if(arrType->getLenVec().size()==0||arrType->getLenVec()[0]==0){
        buffer_str1=basicType->toStr();
        buffer_str2=(new PointerType(basicType))->toStr();
        flag=1;
    }
    else{
        buffer_str1 = arrType->toStr();
        buffer_str2=(new PointerType(arrType))->toStr();
        // buffer_str2 = buffer_str1;
    }
    if(flag==0){
    //z1204 fix 或许指针基址不一样首个i32 0需要改变
    // %1 = getelementptr [5 x [4 x i32]], [5 x [4 x i32]]* @a, i32 0, i32 2
    fprintf(yyout, "  %s = getelementptr inbounds %s, %s %s, %s 0, %s %s\n",
        dst->toStr().c_str(), buffer_str1.c_str(),
        buffer_str2.c_str(), src->toStr().c_str(),
        basicType->toStr().c_str(),basicType->toStr().c_str(),
        index.c_str());
    }
    else{
    fprintf(yyout, "  %s = getelementptr inbounds %s, %s %s, %s %s\n",
        dst->toStr().c_str(), buffer_str1.c_str(),
        buffer_str2.c_str(), src->toStr().c_str(),
        basicType->toStr().c_str(),
        index.c_str());
    }
}

ZextInstruction::ZextInstruction(Operand* dst,
                                 Operand* src,
                                 BasicBlock* insert_bb)
    : Instruction(ZEXT, insert_bb) {
    operands.push_back(dst);
    operands.push_back(src);
    dst->setDef(this);
    src->addUse(this);
}

void ZextInstruction::output() const {
    Operand* dst = operands[0];
    Operand* src = operands[1];
    int v;
    if(operands[1]->getSymbolEntry()->isConstant()){
        ConstantSymbolEntry* cs  = dynamic_cast<ConstantSymbolEntry*>(operands[1]->getSymbolEntry());
        v = cs->getIntValue();
        fprintf(yyout, "  %s = zext %s %d to i32\n", dst->toStr().c_str(),
            src->getType()->toStr().c_str(), v);
    }else{
        fprintf(yyout, "  %s = zext %s %s to i32\n", dst->toStr().c_str(),
            src->getType()->toStr().c_str(), src->toStr().c_str());
    }
}
