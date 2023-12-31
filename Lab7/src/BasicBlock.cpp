#include "BasicBlock.h"
#include "Function.h"
#include "Ast.h"
#include <algorithm>

extern FILE* yyout;

// insert the instruction to the front of the basicblock.
void BasicBlock::insertFront(Instruction *inst)
{
    printg("--------------insertFront");
    insertBefore(inst, head->getNext());
}

// insert the instruction to the back of the basicblock.
void BasicBlock::insertBack(Instruction *inst) 
{
    insertBefore(inst, head);
}

// insert the instruction dst before src.
void BasicBlock::insertBefore(Instruction *dst, Instruction *src)
{
    dst->setNext(src);
    src->getPrev()->setNext(dst);
    dst->setPrev(src->getPrev());
    src->setPrev(dst);

    dst->setParent(this);
}

void BasicBlock::remove(Instruction* inst) {
    inst->getPrev()->setNext(inst->getNext());
    inst->getNext()->setPrev(inst->getPrev());
}

void BasicBlock::output() const
{
    fprintf(yyout, "B%d:", no);

    if (!pred.empty())
    {

        fprintf(yyout, "%*c; preds = %%B%d", 32, '\t', pred[0]->getNo());
        for (auto i = pred.begin() + 1; i != pred.end(); i++)
            fprintf(yyout, ", %%B%d", (*i)->getNo());
    }
    fprintf(yyout, "\n");
    // std::cout<<head->getParent()->getParent()->getSymPtr()->toStr().c_str();
    for (auto i = head->getNext(); i != head; i = i->getNext()){
        i->output();
    }
}

void BasicBlock::addSucc(BasicBlock *bb)
{
    succ.emplace_back(bb);
}

void BasicBlock::addSucc(BasicBlock* bb, bool first) {
    if (first)
        succ.insert(succ.begin(), bb);
    else
        succ.emplace_back(bb);
}

// remove the successor basicclock bb.
void BasicBlock::removeSucc(BasicBlock *bb)
{
    succ.erase(std::find(succ.begin(), succ.end(), bb));
}

void BasicBlock::addPred(BasicBlock *bb)
{
    pred.emplace_back(bb);
}

// remove the predecessor basicblock bb.
void BasicBlock::removePred(BasicBlock *bb)
{
    pred.erase(std::find(pred.begin(), pred.end(), bb));
}

void BasicBlock::genMachineCode(AsmBuilder* builder) 
{
    auto cur_func = builder->getFunction();
    auto cur_block = new MachineBlock(cur_func, no);
    builder->setBlock(cur_block);
    for (auto i = head->getNext(); i != head; i = i->getNext())
    {
        i->genMachineCode(builder);
    }
    cur_func->InsertBlock(cur_block);
}

BasicBlock::BasicBlock(Function *f)
{
    this->no = SymbolTable::getLabel();
    f->insertBlock(this);
    parent = f;
    head = new DummyInstruction();
    head->setParent(this);
}

void BasicBlock::cleanMark() {
    auto inst = head->getNext();
    while (inst != head) {
        inst->unsetMark();
        inst = inst->getNext();
    }
}

BasicBlock::~BasicBlock()
{
    Instruction *inst;
    inst = head->getNext();
    while (inst != head)
    {
        Instruction *t;
        t = inst;
        inst = inst->getNext();
        delete t;
    }
    for(auto &bb:pred)
        bb->removeSucc(this);
    for(auto &bb:succ)
        bb->removePred(this);
    parent->remove(this);
}

void BasicBlock::insertPhiInstruction(Operand* dst) {
    Instruction* i = new PhiInstruction(dst);
    insertFront(i);
}