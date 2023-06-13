#ifndef __OPERAND_H__
#define __OPERAND_H__

#include "SymbolTable.h"
#include "Type.h"
#include <vector>

class Instruction;
class Function;
class Operand;
static std::vector<Operand*>recent_op;

// class Operand - The operand of an instruction.
class Operand
{
typedef std::vector<Instruction *>::iterator use_iterator;

private:
    Instruction *def;                // The instruction where this operand is defined.
    std::vector<Instruction *> uses; // Intructions that use this operand.
    SymbolEntry *se;                 // The symbol entry of this operand.
    std::vector<SymbolEntry*>idx_se;
    // std::vector<int>rangeCurve;
public:
    Operand(SymbolEntry*se) :se(se){
        def = nullptr;
        if(recent_op.size()<5){
            recent_op.emplace_back(this);
        }else{
            recent_op.erase(recent_op.begin());
            recent_op.emplace_back(this);
        }
        };
    void setDef(Instruction *inst) {def = inst;};
    void addUse(Instruction *inst,int no=-1) { 
        uses.emplace_back(inst);
        // if(rangeCurve.empty()&&no!=-1){
        //     rangeCurve.emplace_back(no);
        // }
        };
    void removeUse(Instruction *inst);
    void removeDef(Instruction* inst);
    void setIdxSe(std::vector<SymbolEntry*> v){
        
        idx_se.assign(v.begin(),v.end());
        
        }
    std::vector<SymbolEntry*>& getIdxSe(){return idx_se;}
    int usersNum() const {return uses.size();};
    std::vector<Instruction *> getUses() {return uses;};
    use_iterator use_begin() {return uses.begin();};
    use_iterator use_end() {return uses.end();};
    Type* getType() {return se->getType();};
    void setType(Type * type) {return se->setType(type);};
    std::string toStr() const;
    SymbolEntry*& getSymbolEntry() {return se;};
    Instruction* getDef() { return def; };
    bool isConstant(){ return se->isConstant();};
    bool isTemporary(){ return se->isTemporary();};
    bool isVariable(){ return se->isVariable();};
    bool isGobal() { return this->isVariable() && dynamic_cast<IdentifierSymbolEntry*>(se)->isGlobal();};
    int32_t getValue();
};

#endif