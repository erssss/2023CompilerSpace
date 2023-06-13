#include "Function.h"
#include "Unit.h"
#include "Type.h"
#include "Ast.h"
#include <list>

extern FILE* yyout;

Function::Function(Unit *u, SymbolEntry *s)
{
    u->insertFunc(this);
    entry = new BasicBlock(this);
    sym_ptr = s;
    parent = u;
    // printf("====== init function =======\n");
    // std::cout<<block_list.size()<<std::endl;
}

Function::~Function()
{
    // auto delete_list = block_list;
    // printinfo("======start delete=======\n");
    // for (auto &i : delete_list){
    //     delete i;
    // }
    // printinfo("=======end delete=======\n");
    // parent->removeFunc(this);
}

// remove the basicblock bb from its block_list.
void Function::remove(BasicBlock *bb)
{
    block_list.erase(std::find(block_list.begin(), block_list.end(), bb));
    // printf("block_list的长度 %d\n", block_list.size());
}

void Function::output() const
{
    FunctionType* funcType = dynamic_cast<FunctionType*>(sym_ptr->getType());
    Type *retType = funcType->getRetType();
    std::ostringstream buffer;
    // buffer << returnType->toStr() << "(";
    buffer << "(";
    std::string tmp;
    int size = (int)funcType->getParamType().size();
    for (int i=0;i<size;i++){
        if(funcType->getParamType()[i]->isArray()){
            ArrayType*arr = dynamic_cast<ArrayType*>(funcType->getParamType()[i]);
            if(arr->getLenVec()[0]==0){
                tmp = (new PointerType (arr->getEleType()))->toStr();
            }
        }
        else{
                tmp = funcType->getParamType()[i]->toStr();
        }

        buffer << tmp<<" "<<funcType->getParamSe()[i]->toStr();
    if(i<size-1)
        buffer << ", ";
    }
    buffer << ")";
    fprintf(yyout, "define %s %s %s {\n", retType->toStr().c_str(), sym_ptr->toStr().c_str(),buffer.str().c_str());
    std::set<BasicBlock *> v;
    std::list<BasicBlock *> q;
    q.push_back(entry);
    v.insert(entry);
    printg("func output now\n");
    while (!q.empty())
    {
        
        auto bb = q.front();
        q.pop_front();
        printg("----------------------bb START;");
        // if(!bb->empty())
        bb->output();
        printg("----------------------bb END;");
        for (auto succ = bb->succ_begin(); succ != bb->succ_end(); succ++)
        {
            // printg("========= block chain ===========");
            if (v.find(*succ) == v.end())
            {
                v.insert(*succ);
                q.push_back(*succ);
            }
            // printg("========= block chain end ===========");
        }
    }
    printg("=====================END;\n");
    fprintf(yyout, "}\n");
}
