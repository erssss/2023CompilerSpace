#ifndef __SYMBOLTABLE_H__
#define __SYMBOLTABLE_H__
#include <iostream>
#include <string>
#include <map>
#include <vector>
class Type;
class Operand;

// static bool param_on=0;

class IdentSystem
{
private:
    enum {CONSTANT, VARIABLE, TEMPORARY};
public:
    static int constant;
    static int variable;
    static int temporary;

};

class SymbolEntry
{
private:
    int kind;
protected:
    enum {CONSTANT, VARIABLE, TEMPORARY};
    Type *type;
    //Type *basicType;
    bool isTrans=0;
    Type *transType;

public:
    SymbolEntry(Type *type, int kind);
    virtual ~SymbolEntry() {};
    bool isConstant() const {return kind == CONSTANT;};
    bool isTemporary() const {return kind == TEMPORARY;};
    bool isVariable() const {return kind == VARIABLE;};
    Type* getType() {return type;};
    //Type* getBasicType() {return basicType;};
     void setIsTrans(bool trans){this->isTrans=trans;}
    bool getIsTrans(){return isTrans;}
    Type* getTransType() {return transType;};
    void setType(Type *type);
    void setTransType(Type *type) {this->transType=type;};
    virtual std::string toStr() = 0;
    // You can add any function you need here.
};


/*  
    Symbol entry for literal constant. Example:

    int a = 1;

    Compiler should create constant symbol entry for literal constant '1'.
*/
class ConstantSymbolEntry : public SymbolEntry
{
private:
    int value;
    float fvalue;

public:
    ConstantSymbolEntry(Type *type, int value);
    ConstantSymbolEntry(Type *type, float fvalue);
    ConstantSymbolEntry():SymbolEntry(nullptr, SymbolEntry::CONSTANT){};
    ConstantSymbolEntry(Type* type):SymbolEntry(type, SymbolEntry::CONSTANT){};
    virtual ~ConstantSymbolEntry() {};
    int getIntValue() const {return value;};
    void setIntValue(int v) {value=v;};
    float getFloatValue() const {return fvalue;};
    void setFloatValue(float v) {fvalue=v;};
    std::string toStr();
    // You can add any function you need here.
};


/* 
    Symbol entry for identifier. Example:

    int a;
    int b;
    void f(int c)
    {
        int d;
        {
            int e;
        }
    }

    Compiler should create identifier symbol entries for variables a, b, c, d and e:

    | variable | scope    |
    | a        | GLOBAL   |
    | b        | GLOBAL   |
    | c        | PARAM    |
    | d        | LOCAL    |
    | e        | LOCAL +1 |
*/
class IdentifierSymbolEntry : public SymbolEntry
{
private:
    enum {GLOBAL, PARAM, LOCAL};
    std::string name;
    int scope;
    int value;
    float fvalue;
    
    // You can add any field you need here.
    Operand *addr;  // The address of the identifier.
    int idtype;

public:
    IdentifierSymbolEntry(Type *type, std::string name, int scope,int ident_type=IdentSystem::variable);
    IdentifierSymbolEntry(std::string name, int scope,int ident_type=IdentSystem::variable);
    //void setType(Type *type){this->type=type;};
    void setIdtype(int idtype){this->idtype=idtype;};
    virtual ~IdentifierSymbolEntry() {};
    std::string toStr();
    int getScope() const {return scope;};
    int getIntValue() const { return value;};
    void setIntValue(int v) {this->value = v;};
    void setFloatValue(float f) {this->fvalue = f;};
    float getFloatValue() const { return fvalue;};  
    bool isConstant() const {return idtype == IdentSystem::constant;};
    bool isGlobal() const {return scope == GLOBAL;};
    bool isParam() const {return scope == PARAM;};
    bool isLocal() const {return scope >= LOCAL;};
    void setAddr(Operand *addr) {this->addr = addr;};
    Operand* getAddr() {return addr;};
    // void changeType(Type *type){this->type == type;};
    // You can add any function you need here.
};


/* 
    Symbol entry for temporary variable created by compiler. Example:

    int a;
    a = 1 + 2 + 3;

    The compiler would generate intermediate code like:

    t1 = 1 + 2
    t2 = t1 + 3
    a = t2

    So compiler should create temporary symbol entries for t1 and t2:

    | temporary variable | label |
    | t1                 | 1     |
    | t2                 | 2     |
*/
class TemporarySymbolEntry : public SymbolEntry
{
private:
    int label;
    int ivalue;
    float fvalue;
public:
    TemporarySymbolEntry(Type *type, int label);
    int getLabel(){return label; };
    void setIntValue(int ivalue){this->ivalue = ivalue; };
    void setFloatValue(float fvalue){this->fvalue = fvalue; };
    int getIntValue(){return ivalue; };
    float getFloatValue(){return fvalue; };
    virtual ~TemporarySymbolEntry() {};
    std::string toStr();
    // You can add any function you need here.
};

// symbol table managing identifier symbol entries
class SymbolTable
{
private:
    std::map<std::string, SymbolEntry*> symbolTable;
    SymbolTable *prev;
    int level;
    static int counter;
public:
    SymbolTable();
    SymbolTable(SymbolTable *prev);
    void install(std::string name, SymbolEntry* entry);
    SymbolEntry* lookup(std::string name);
    SymbolTable* getPrev() {return prev;};
    int getLevel() {return level;};
    static int getStillLabel() {return counter;}
    static int getLabel() {
        // std::cout<<"===== "<<counter<<" =====\n";
        return counter++;};
    static void resetLabel() { counter=1; };
};


extern SymbolTable *identifiers;
extern SymbolTable *globals;

#endif
