#include "SymbolTable.h"
#include "Type.h"
#include <iostream>
#include <sstream>
#include <vector>
extern bool check_on;
extern bool assert_on;
extern int yylineno;
extern int offsets;
// z1214 计算基本类型
SymbolEntry::SymbolEntry(Type *type, int kind) 
{
    this->type = type;
    this->kind = kind;
    // if(type!=nullptr){
    //     if(type->isFunc()){
    //         this->basicType=dynamic_cast<FunctionType*>(type)->getRetType();
    //     }
    //     else if(type->isArray()){
    //         this->basicType=dynamic_cast<ArrayType*>(type)->getEleType();
    //     }
    //     else{
    //         this->basicType=type;
    //     }
    // }
}
void SymbolEntry::setType(Type *type) 
{
    this->type = type;
    
    // if(type!=nullptr){
    //     if(type->isFunc()){
    //         this->basicType=dynamic_cast<FunctionType*>(type)->getRetType();
    //     }
    //     else if(type->isArray()){
    //         this->basicType=dynamic_cast<ArrayType*>(type)->getEleType();
    //     }
    //     else{
    //         this->basicType=type;
    //     }
    // }
}
ConstantSymbolEntry::ConstantSymbolEntry(Type *type, int value) : SymbolEntry(type, SymbolEntry::CONSTANT)
{
    this->value = value;
}

ConstantSymbolEntry::ConstantSymbolEntry(Type *type, float fvalue) : SymbolEntry(type, SymbolEntry::CONSTANT)
{
    //printf("%f\n",fvalue);
    this->fvalue = fvalue;
}

std::string ConstantSymbolEntry::toStr()
{
    std::ostringstream buffer;
    if(this->getType() == TypeSystem::intType)
        buffer << value;
    if(this->getType() == TypeSystem::floatType)
        buffer << fvalue;
    return buffer.str();
}


IdentifierSymbolEntry::IdentifierSymbolEntry(Type *type, std::string name, int scope,int ident_type) : SymbolEntry(type, ident_type), name(name),scope(scope)
{
    //this->scope = 0;
    this->idtype = ident_type;
}

IdentifierSymbolEntry::IdentifierSymbolEntry(std::string name, int scope,int ident_type) : SymbolEntry(type, ident_type), name(name),scope(scope)
{
    //this->scope = 0;
    this->idtype = ident_type;
}

std::string IdentifierSymbolEntry::toStr()
{
    //z1214 fix
    std::ostringstream buffer;
        buffer << '@' << name;
    return buffer.str();
}

TemporarySymbolEntry::TemporarySymbolEntry(Type *type, int label) : SymbolEntry(type, SymbolEntry::TEMPORARY)
{
    this->label = label;
}

std::string TemporarySymbolEntry::toStr()
{
    std::ostringstream buffer;
    buffer << "%t" << label;
    return buffer.str();
}

SymbolTable::SymbolTable()
{
    prev = nullptr;
    level = 0;
}

SymbolTable::SymbolTable(SymbolTable *prev)
{
    this->prev = prev;
    this->level = prev->level + 1;
}

/*
    Description: lookup the symbol entry of an identifier in the symbol table
    Parameters: 
        name: identifier name
    Return: pointer to the symbol entry of the identifier

    hint:
    1. The symbol table is a stack. The top of the stack contains symbol entries in the current scope.
    2. Search the entry in the current symbol table at first.
    3. If it's not in the current table, search it in previous ones(along the 'prev' link).
    4. If you find the entry, return it.
    5. If you can't find it in all symbol tables, return nullptr.
*/
SymbolEntry* SymbolTable::lookup(std::string name)
{
    SymbolTable *symTab = this;
    while(symTab!=nullptr){
        if(symTab->symbolTable.find(name)!=symTab->symbolTable.end()){
            return symTab->symbolTable[name];
        }
        symTab = symTab->prev;
    }
    return nullptr;
}

// install the entry into current symbol table.
void SymbolTable::install(std::string name, SymbolEntry* entry)
{
    if (this->symbolTable.find(name) != this->symbolTable.end()) {
        if(check_on){
            fprintf(stderr,"repetitive define\n");
            fprintf(stderr, "error occurs in <line: %d, col: %d>\n",yylineno,offsets);
            if(assert_on) assert(this->symbolTable.find(name) == this->symbolTable.end());
        }
    } else {
        symbolTable[name] = entry;
    }
}

int SymbolTable::counter = 0;
static SymbolTable t;
SymbolTable *identifiers = &t;
SymbolTable *globals = &t;

int IdentSystem::constant=CONSTANT;
int IdentSystem::variable=VARIABLE;
int IdentSystem::temporary=TEMPORARY;

