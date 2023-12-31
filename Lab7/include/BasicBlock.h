#ifndef __BASIC_BLOCK_H__
#define __BASIC_BLOCK_H__
#include <vector>
#include <set>
#include "Instruction.h"
#include "AsmBuilder.h"

class Function;

class BasicBlock
{
    typedef std::vector<BasicBlock *>::iterator bb_iterator;

private:
    std::vector<BasicBlock *> pred, succ;
    Instruction *head;
    Function *parent;
    int no;
    bool mark;
public:
    BasicBlock(Function *);
    ~BasicBlock();
    std::vector<BasicBlock *>&getPrevbb(){return pred;}
    void deleteInstr(Instruction * instr){}
    void insertFront(Instruction *);
    void insertBack(Instruction *);
    void insertBefore(Instruction *, Instruction *);
    void remove(Instruction *);
    bool empty() const { return head->getNext() == head;}
    void output() const;
    bool succEmpty() const { return succ.empty(); };
    bool predEmpty() const { return pred.empty(); };
    void addSucc(BasicBlock *);
    void removeSucc(BasicBlock *);
    void addPred(BasicBlock *);
    void removePred(BasicBlock *);
    int getNo() { return no; };
    Function *getParent() { return parent; };
    Instruction* begin() { return head->getNext();};
    Instruction* end() { return head;};
    Instruction* rbegin() { return head->getPrev();};
    Instruction* rend() { return head;};
    bb_iterator succ_begin() { return succ.begin(); };
    bb_iterator succ_end() { return succ.end(); };
    bb_iterator pred_begin() { return pred.begin(); };
    bb_iterator pred_end() { return pred.end(); };
    int getNumOfPred() const { return pred.size(); };
    int getNumOfSucc() const { return succ.size(); };
    void genMachineCode(AsmBuilder*);
    void setMark() { mark = true; };
    void unsetMark() { mark = false; };
    bool isMark() { return mark; };
    void cleanMark();
    std::vector<BasicBlock *>& getSuccBlock() { return succ;};
    std::vector<BasicBlock *>& getPredBlock() { return pred;};
    void insertPhiInstruction(Operand* dst);
    void addSucc(BasicBlock* bb, bool first);
};

#endif