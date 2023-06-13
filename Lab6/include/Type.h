
#ifndef __TYPE_H__
#define __TYPE_H__

#include <vector>
#include <string>
#include <iostream>
#include <assert.h>
#include "Operand.h"
#include "SymbolTable.h"
class Node;
class ExprNode;
class Id;
class Type
{
private:
    int kind;

protected:
    enum
    {
        INT,
        VOID,
        FUNC,
        FLOAT,
        ARRAY,
        PTR,
        BOOL
    };
    int size; /*z1104 整合子类size*/
public:
    Type(int kind, int size = 0) : kind(kind), size(size){};
    virtual ~Type(){};
    virtual std::string toStr() = 0;
    bool isInt() const { return kind == INT; };
    bool isFloat() const { return kind == FLOAT; };
    bool isVoid() const { return kind == VOID; };
    bool isFunc() const { return kind == FUNC; };
    bool isArray() const { return kind == ARRAY; };
    bool isPointer() const { return kind == PTR; };
    bool isBool() const { return kind == BOOL;}
    int getSize() const { return size; };
};

class IntType : public Type
{
private:
    // int size;
public:
    IntType(int size) : Type(Type::INT, size){};
    std::string toStr();
};

class FloatType : public Type
{
private:
    // int size;
public:
    FloatType(int size) : Type(Type::FLOAT, size){};
    std::string toStr();
};

class VoidType : public Type
{
public:
    VoidType() : Type(Type::VOID){};
    std::string toStr();
};
class ArrayType;
class PointerType : public Type
{
private:
    Type *valueType;

public:
    PointerType(Type *valueType) : Type(Type::PTR) { this->valueType = valueType; };
    PointerType(ArrayType *arrtype, int idx);
    Type *getValueType() { return valueType; }
    std::string toStr();
};

/*z1104 数组实现*/
class ArrayType : public Type
{
private:
    Type *eletype;
    int dimen;
    bool flag=0;
    std::vector<int> len; // 不同维度大小
    // int size;
    // 1215 为了arr的赋值指令
    std::vector<int> arrayValue;

public:
    void setFlag(bool f){flag=f;}
    bool getFlag(){return flag;}
    ArrayType(Type *eletype) : Type(Type::ARRAY), eletype(eletype){};
    ArrayType(Type *eletype, int dimen)
        : Type(Type::ARRAY), eletype(eletype), dimen(dimen){
                                                   // size = len * eletype->getSize();
                                               };
    bool operator==(ArrayType *arr)
    {
        if (this->eletype == arr->getEleType())
        {
            if (this->dimen == arr->getDimen())
                return true;
        }
        return false;
    };
    bool operator==(PointerType *ptr)
    {
        if (dimen == 1)
        {
            if (this->eletype == ptr->getValueType())
            {
                return true;
            }
        }
        else
        {
            if (dimen == dynamic_cast<ArrayType *>(ptr->getValueType())->getDimen() + 1)
                return true;
        }
        return false;
    };
    void genArr(std::ostringstream &buffer,int dim);
    std::string toStr();
    int getLenNum(int n) const { return len[n]; };
    Type *getEleType() const { return eletype; };
    void setEleType(Type *eletype) { this->eletype = eletype; };
    std::vector<int> &getLenVec() { return len; }
    std::vector<int> &getValueVec() { return arrayValue; }
    int getDimen() { return this->dimen; }
    bool calHighestDimen();
    void setDimen(int dimen) { this->dimen = dimen; }
    // z1209 为了指针寻址添加
    Type *getStripEleType();
};

class FunctionType : public Type
{
private:
    Type *returnType;
    std::vector<Type *> paramsType;
    std::vector<Operand *> paramsSe;
    
    std::vector<FunctionType *> reloadFunc;

public:
    FunctionType(Type *returnType, std::vector<Type *> paramsType) : Type(Type::FUNC), returnType(returnType), paramsType(paramsType){};
    void setParamSe(std::vector<Operand *> paramsSe){this->paramsSe=paramsSe;}
    std::vector<Operand *> &getParamSe(){return paramsSe;}
    std::vector<Type *> &getParamType(){return paramsType;}
    std::string toStr();
    std::vector<FunctionType *> &getReloadFunc() { return reloadFunc; }
    void appendReloadFunc(FunctionType *func)
    {
        reloadFunc.push_back(func);
    }
    Type *getRetType() { return returnType; };
    bool checkParam(std::vector<SymbolEntry *> rparams);
};

class TypeSystem
{
private:
    static IntType commonInt;
    static IntType commonBool;
    static VoidType commonVoid;
    static FloatType commonFloat;
    static ArrayType commonARRAY;
    static FunctionType commonFunc;

public:
    static Type *intType;
    static Type *voidType;
    static Type *floatType;
    static Type *arrayType;
    static Type *funcType;
    static Type *boolType;
};

inline int pairTypeCheck(std::vector<Type *> typeVec)
{
    Type *basicType1;
    Type *basicType2;
    std::vector<Type *> basicTypeVec;
    basicTypeVec.push_back(basicType1);
    basicTypeVec.push_back(basicType2);
    // std::vector<Type*> typeVec;
    // typeVec.push_back(t1);
    // typeVec.push_back(t2);
    if(((basicTypeVec[0]==TypeSystem::voidType)&&!(basicTypeVec[1]==TypeSystem::voidType))||
        ((basicTypeVec[1]==TypeSystem::voidType)&&!(basicTypeVec[0]==TypeSystem::voidType)))
        return 1;
    for (int i = 0; i < 2; i++)
    {

        if (typeVec[i]->isArray())
        {
            basicTypeVec[i] = dynamic_cast<ArrayType *>(typeVec[i])->getEleType();
        }
        else if (typeVec[i]->isFunc())
        {
            basicTypeVec[i] = dynamic_cast<FunctionType *>(typeVec[i])->getRetType();
        }
        else if (typeVec[i]->isPointer())
        {
            basicTypeVec[i] = dynamic_cast<PointerType *>(typeVec[i])->getValueType();
        }
    }
    if (basicTypeVec[0] == basicTypeVec[1])
    {
        if (basicTypeVec[0]->isArray())
        {
            if (dynamic_cast<ArrayType *>(basicTypeVec[0])->getDimen() == dynamic_cast<ArrayType *>(basicTypeVec[1])->getDimen())
                return 0;
            else
                return 1;
        }
        return 0;
    }
    else
    {
        if (basicTypeVec[0]->isInt() && basicTypeVec[0]->isFloat())
            return 2;
        else if (basicTypeVec[0]->isFloat() && basicTypeVec[0]->isInt())
            return 3;
    }

    return 0;
}
inline bool typeHandle(std::vector<Type *> typeVec){
    // int retCode = pairTypeCheck(typeVec);
    // std::cout<<"code = "<<retCode<<"\n";
    // switch(retCode){
    //         case 0:
    //             return 1;
    //         case 1:
    //             return 0;
    //         /*f to i*/
    //         case 2:
    //             se->setIsTrans(1);
    //             se->setTransType(typeVec[0]);
    //             if(se->isConstant()){
    //                 int value=dynamic_cast<ConstantSymbolEntry*>(se)->getIntValue();
    //                 dynamic_cast<ConstantSymbolEntry*>(se)->setFloatValue((float)value);
    //             }
    //             break;
    //         /*i to f*/
    //         case 3:
    //             se->setIsTrans(1);
    //             se->setTransType(typeVec[0]);
    //             if(se->isConstant()){
    //                 float value=dynamic_cast<ConstantSymbolEntry*>(se)->getFloatValue();
    //                 dynamic_cast<ConstantSymbolEntry*>(se)->setIntValue((int)value);
    //             }
    //             break;
    //     }
    return 1;
}
inline Type *getBasicType(SymbolEntry *se)
{
    Type *type = se->getType();
    Type *basicType;
    if (type != nullptr)
    {
        if (type->isFunc())
        {
            basicType = dynamic_cast<FunctionType *>(type)->getRetType();
        }
        else if (type->isArray())
        {
            basicType = dynamic_cast<ArrayType *>(type)->getEleType();
        }
        else
        {
            basicType = type;
        }
    }
    return basicType;
}

#endif
