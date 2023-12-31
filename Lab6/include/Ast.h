#ifndef __AST_H__
#define __AST_H__

#include <fstream>
#include "Type.h"
#include "Operand.h"
#include "SymbolTable.h"
# define DEBUG_SWITCH_genCode 0
# if DEBUG_SWITCH_genCode
# define printg(str)\
        std::cout<<str<<"\n";
# else
# define printg(str) //
# endif

# define DEBUG_SWITCH 0
# if DEBUG_SWITCH
# define print(str)\
        printf("%s\n",str);
# else
# define print(str) //
# endif

# define DEBUG_SWITCH_M 0
# if DEBUG_SWITCH_M
# define printinfo(str)\
        printf("%s\n",str);
# else
# define printinfo(str) //
# endif

# define DEBUG_SWITCH_MG 0
# if DEBUG_SWITCH_MG
# define mprintg(str)\
        std::cout<<str<<"\n";
# else
# define mprintg(str) //
# endif

class SymbolEntry;
class Unit;
class Type;
class FunctionType;
class TypeSystem;
class Function;
class BasicBlock;
class Instruction;
class IRBuilder;
extern Unit unit;

class Node
{
private:
    static int counter;
    int seq;
    /* z1102 增加next结点 */
    Node *next =nullptr;

protected:
    std::vector<Instruction*> true_list;
    std::vector<Instruction*> false_list;
    static IRBuilder *builder;
    void backPatch(std::vector<Instruction*> &list, BasicBlock*bb);
    std::vector<Instruction*> merge(std::vector<Instruction*> &list1, std::vector<Instruction*> &list2);

public:
    Node();
    static void setIRBuilder(IRBuilder*ib) {builder = ib;};
    int getSeq() const {return seq;};
    virtual void output(int level) = 0;
    virtual void typeCheck() = 0;
    virtual void genCode() = 0;
    std::vector<Instruction*>& trueList() {return true_list;}
    std::vector<Instruction*>& falseList() {return false_list;}
    void setNext(Node* node) {
        this->next=node;
    }
    void append(Node* node){
        print("in append");
        Node *p=this;
        while(p->getNext()!=nullptr){
            p=p->getNext();
        }
        
        if(p != this)
            p->setNext(node);
        else 
            p->setNext(node);

    }
    Node* getNext() {
        return next;
    }
};

class ExprNode : public Node
{
protected:
    SymbolEntry *symbolEntry;
    Operand *dst;   // The result of the subtree is stored into dst.
    bool isNull=0;
public:
    ExprNode(SymbolEntry *symbolEntry) : symbolEntry(symbolEntry){};
    SymbolEntry *&getSymbolEntry(){return this->symbolEntry;}
    Operand* getOperand() {return dst;};
    void setNull(bool isNull){this->isNull=isNull;}
    bool getNull(){return isNull;}
    void output(int level){}
    void typeCheck();
    void genCode();
    void resetOperand(Operand* newdst){dst = newdst;};
};

// 仅用于int2bool
class ImplictCastExpr : public ExprNode {
   private:
    ExprNode* expr;

   public:
    ImplictCastExpr(ExprNode* expr): ExprNode(nullptr), expr(expr) {
        dst = new Operand(
            new TemporarySymbolEntry(TypeSystem::boolType, SymbolTable::getLabel()));
    };
    void output(int level);
    ExprNode* getExpr() const { return expr; };
    bool typeCheck(Type* retType = nullptr) { return false; };
    void genCode();
};

/*m1123 定义类型转换*/
class CastExpr : public ExprNode
{
private:
    SymbolEntry *old;
    ExprNode *expr1;
public:
    CastExpr(SymbolEntry *se, SymbolEntry *se_old, ExprNode*expr1) : ExprNode(se), old(se_old), expr1(expr1){};
    void output(int level);
    void typeCheck();
    void genCode();
};

/*m1102 添加乘法运算符, 关系运算符等*/
class BinaryExpr : public ExprNode
{
private:
    int op;
    ExprNode *expr1, *expr2;
public:
    enum {ADD, SUB,  MUL, DIV, MOD, AND, OR, LESS, GREATER, LESSEQUAL, GREATEREQUAL, EQUAL, NOTEQUAL};
    BinaryExpr(SymbolEntry *se, int op, ExprNode*expr1, ExprNode*expr2) : ExprNode(se), op(op), expr1(expr1), expr2(expr2){dst = new Operand(se);};
    void output(int level);
    void typeCheck();
    void genCode();
};

/*m1102 定义单目运算符*/
class UnaryExpr : public ExprNode
{
private:
    int op;
    ExprNode *expr1;
public:
//z1203 fix 这里的东西比之前多了记得看看
    enum {PLUS, UMINUS, NOT,FUNC};
    UnaryExpr(SymbolEntry *se, int op, ExprNode*expr1) : ExprNode(se), op(op), expr1(expr1){dst = new Operand(se);};
    void output(int level);
    void typeCheck();
    void genCode();
};

class Constant : public ExprNode
{
public:
    Constant(SymbolEntry *se) : ExprNode(se){dst = new Operand(se);};
    void output(int level);
    void typeCheck();
    void genCode();
};

class Id : public ExprNode
{
private:
    std::string name;
    ExprNode* index=nullptr;
    bool isArray;
    bool isLeft=0;
    bool arrFlag=0;
public:
    Id(SymbolEntry *se,std::string name="",bool isArray=false) : ExprNode(se),name(name),isArray(isArray){
        SymbolEntry *temp = new TemporarySymbolEntry(se->getType(), SymbolTable::getLabel()); 
        dst = new Operand(temp);
    };
    /* for ID ArrayIndices */
    Id(SymbolEntry *se,ExprNode* index,std::string name="",bool isArray=false) 
    : ExprNode(se),name(name),index(index),isArray(isArray){
        SymbolEntry *temp = new TemporarySymbolEntry(se->getType(), SymbolTable::getLabel()); 
        dst = new Operand(temp);
    };
    void setLeft(){isLeft=1;}
    void setRight(){isLeft=0;}
    void output(int level);
    void setName(std::string name){this->name=name;}
    std::string getName(){return this->name;}
    ExprNode* getIndex(){return index;}
    void setIsArray(bool isArray){this->isArray=isArray;}
    bool getIsArray(){return this->isArray;}
    void typeCheck();
    void genCode();
    void setArrPFlag(bool f){arrFlag=f;};
    
    // z1102
    // void setNext(Id* node) {
    //     this->setNext(node);
    // }
    // Id* getNext() {
    //     print("getNext");
    //     return this->getNext();
    // }
    // void append(Id* node){
    //     print("in append");
    //     Node *p=this;
    //     while(p->getNext()!=nullptr){
    //         p=p->getNext();
    //     }
        
    //     if(p != this)
    //         this->setNext(node);
    //     else 
    //         p->setNext(node);

    // }
};

// class ConstNode : public Id
// {
// private:
//     ConstantSymbolEntry *constantSymbolEntry;
// public:
//     ConstNode(SymbolEntry *se,ConstantSymbolEntry *constantSymbolEntry):Id(se), constantSymbolEntry(constantSymbolEntry){};
//     ConstantSymbolEntry *&getSymbolEntry(){return this->constantSymbolEntry;};
// };Id

class ConstNode : public ExprNode
{
protected:
    Id* id;
public:
    ConstNode(Id* id,SymbolEntry *cs):ExprNode(cs), id(id){
        // SymbolEntry *temp = new TemporarySymbolEntry(id->getSymbolEntry()->getType(), SymbolTable::getLabel()); 
        dst = id->getOperand();
    };
    void output(int level);
    void typeCheck();
    void genCode();
};


// z1103
class FuncCall : public ExprNode {
   private:
    ExprNode* param;
    // z1108 或许之后用得上
    Type * retType;
    ExprNode* retExp;
   public:
    FuncCall(SymbolEntry* se, ExprNode* param = nullptr,ExprNode*retExp=nullptr): ExprNode(se), param(param),retExp(retExp){
            FunctionType * funcType=(FunctionType *)se->getType();
            retType=funcType->getRetType();
        };
    ExprNode* getRetExp(){return this->retExp;}
    void output(int level);
    void typeCheck();
    void genCode();
};


class StmtNode : public Node
{
    
};

class FuncStmt : public StmtNode
{
private:
    ExprNode *expr;
public:
    FuncStmt(ExprNode *expr) : expr(expr) {};
    void output(int level);
    void typeCheck();
    void genCode();
};

class CompoundStmt : public StmtNode
{
private:
    StmtNode *stmt;
public:
    CompoundStmt(StmtNode *stmt) : stmt(stmt) {};
    void output(int level);
    void typeCheck();
    void genCode();
};

class SeqNode : public StmtNode
{
private:
    StmtNode *stmt1, *stmt2;
public:
    SeqNode(StmtNode *stmt1, StmtNode *stmt2) : stmt1(stmt1), stmt2(stmt2){};
    void output(int level);
    void typeCheck();
    void genCode();
};

class DeclStmt : public StmtNode
{
private:
    Id *id;
public:
    DeclStmt(Id *id) : id(id){};
    void output(int level);
    Id * &getId(){
        return id;
    }
    void typeCheck();
    void genCode();
};

// z1102
class InitStmt : public StmtNode
{
private:
    Id *id;
    ExprNode *expr;
    bool is_init=0;
public:
    InitStmt(Id *id, ExprNode *expr) : id(id), expr(expr) {is_init=1;};
    InitStmt(Id *id) : id(id) {};
    void output(int level);
    void typeCheck();
    void genCode();
    Id* &getId(){
        return id;
    }
    ExprNode* &getExp(){
        return expr;
    }
    

};
// z1105
class FuncHead : public StmtNode {
   private:
    
    Type* type;
    std::string name; 

   public:
    FuncHead(Type* type, std::string name)
        : type(type), name(name){};
    std::string getName(){return this->name;}
    Type* &getType(){return this->type;}
    void output(int level);
    void typeCheck();
    void genCode();
};

class DeclInitStmt : public StmtNode
{
private:
    InitStmt *initstmt;
public:
    DeclInitStmt(InitStmt *initstmt) : initstmt(initstmt){};
    void output(int level);
    void typeCheck();
    void genCode();
};

class SpaceStmt : public StmtNode
{
public:
    void output(int level);
    void typeCheck();
    void genCode();
};

class IfStmt : public StmtNode
{
private:
    ExprNode *cond;
    StmtNode *thenStmt;
public:
    IfStmt(ExprNode *cond, StmtNode *thenStmt) : cond(cond), thenStmt(thenStmt){
        if (cond->getSymbolEntry()->getType() == TypeSystem::intType) {
            // std::cout<<"这里隐转了\n";
            ImplictCastExpr* temp = new ImplictCastExpr(cond);
            this->cond = temp;
        }else{
            Type* type = cond->getSymbolEntry()->getType();
            if(type!=nullptr&&type->isFunc()){
                /*m1202 处理void情况*/
                FunctionType * funcType=(FunctionType *)cond->getSymbolEntry()->getType();
                if(funcType->getRetType() == TypeSystem::intType){
                    // std::cout<<"这里隐转了\n";
                    ImplictCastExpr* temp = new ImplictCastExpr(cond);
                    this->cond = temp;
                }
            }
        }
    };
    void output(int level);
    void typeCheck();
    void genCode();
};

class IfElseStmt : public StmtNode
{
private:
    ExprNode *cond;
    StmtNode *thenStmt;
    StmtNode *elseStmt;
public:
    IfElseStmt(ExprNode *cond, StmtNode *thenStmt, StmtNode *elseStmt) : cond(cond), thenStmt(thenStmt), elseStmt(elseStmt) {
        // if (cond->getSymbolEntry()->getType() == TypeSystem::intType) {
        //     ImplictCastExpr* temp = new ImplictCastExpr(cond);
        //     this->cond = temp;
        // }
    };
    void output(int level);
    void typeCheck();
    void genCode();
};

class WhileStmt : public StmtNode {
   private:
    ExprNode* cond;
    StmtNode* stmt;
    BasicBlock* cond_bb;
    BasicBlock* stmt_bb;
   public:
    WhileStmt(ExprNode* cond, StmtNode* stmt=nullptr) : cond(cond), stmt(stmt){
        // if (cond->getSymbolEntry()->getType() == TypeSystem::intType) {
        //     ImplictCastExpr* temp = new ImplictCastExpr(cond);
        //     this->cond = temp;
        // }
    };
    void output(int level);
    void typeCheck();
    void genCode();
    void setStmt(StmtNode*stmt){this->stmt=stmt;}
    ExprNode* getCond(){return cond;};
    StmtNode* getStmt(){return stmt;};
    BasicBlock* getCond_bb(){return cond_bb;};
    BasicBlock* getStmt_bb(){return stmt_bb;};
};

class ReturnStmt : public StmtNode
{
private:
    ExprNode *retValue;
public:
    ReturnStmt(ExprNode*retValue) : retValue(retValue) {};
    void output(int level);
    void typeCheck();
    void genCode();
};

class BreakStmt : public StmtNode {
    WhileStmt *whileStmt;
   public:
    BreakStmt(WhileStmt* whileStmt){this->whileStmt=whileStmt;};
    void output(int level);
    void typeCheck();
    void genCode();
};

class ContinueStmt : public StmtNode {
    WhileStmt *whileStmt;
   public:
    ContinueStmt(WhileStmt* whileStmt){this->whileStmt=whileStmt;};
    void output(int level);
    void typeCheck();
    void genCode();
};

class AssignStmt : public StmtNode
{
private:
    Id *lval;
    ExprNode *expr;
public:
    AssignStmt(Id *lval, ExprNode *expr) : lval(lval), expr(expr) {};
    void output(int level);
    void typeCheck();
    void genCode();
};

class ExprStmt : public StmtNode {
   private:
    ExprNode* expr;

   public:
    ExprStmt(ExprNode* expr) : expr(expr){};
    void output(int level);
    void typeCheck();
    void genCode();
};

class FunctionDef : public StmtNode
{
private:
    SymbolEntry *se;
    StmtNode *stmt;
    // Z1103
    DeclStmt* decl;
    int funcno=0;
public:
    FunctionDef(SymbolEntry *se, StmtNode *stmt, DeclStmt* decl,int funcno=0) : se(se), stmt(stmt), decl(decl),funcno(funcno){};
    void output(int level);
    void typeCheck();
    void genCode();
};

class Ast
{
private:
    Node* root;
public:
    Ast() {root = nullptr;}
    void setRoot(Node*n) {root = n;}
    void output();
    void typeCheck();
    void genCode(Unit *unit);
};

#endif
