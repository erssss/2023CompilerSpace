
#ifndef __TYPE_H__
#define __TYPE_H__

#include <vector>
#include <list>
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
    IntType(int size=32) : Type(Type::INT, 32){this->size=32;};
    std::string toStr();
};

class FloatType : public Type
{
private:
    // int size;
public:
    FloatType(int size=32) : Type(Type::FLOAT, 32){};
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
    bool zeroInit=0;
    // int size;
    // 1215 为了arr的赋值指令
    std::vector<int> arrayValue;
    std::vector<float> arrayFloatValue;

public:
    void setZeroInit(bool b){zeroInit=b;}
    bool getZeroInit(){return zeroInit;}
    void setFlag(bool f){flag=f;}
    bool getFlag(){return flag;}
    ArrayType(Type *eletype) : Type(Type::ARRAY), eletype(eletype){};
    ArrayType(Type *eletype, int dimen)
        : Type(Type::ARRAY), eletype(eletype), dimen(dimen){
            // int sum=1;
            // for(size_t i=0;i<len.size();i++){
            //     sum*=len[i];
            // }
            // size = sum * eletype->getSize();
                                               };
    void setSize();
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
    std::vector<float> &getFloatValueVec() { return arrayFloatValue; }
    int getDimen() { return this->dimen; }
    bool calHighestDimen();
    void setDimen(int dimen) { this->dimen = dimen; }
    // z1209 为了指针寻址添加
    Type *getStripEleType();
    int getSize() { this->setSize(); return size; };
};

class FunctionType : public Type
{
private:
    std::string func_name;
    Type *returnType;
    std::vector<Type *> paramsType;
    std::vector<Operand *> paramsSe;
    
    std::vector<FunctionType *> reloadFunc;

public:
    FunctionType(std::string func_name,Type *returnType, std::vector<Type *> paramsType) : Type(Type::FUNC),func_name(func_name),returnType(returnType), paramsType(paramsType){};
    void setParamSe(std::vector<Operand *> paramsSe){this->paramsSe=paramsSe;}
    std::vector<Operand *> &getParamSe(){return paramsSe;}
    std::vector<Type *> &getParamType(){return paramsType;}
    std::string toStr();
    std::vector<FunctionType *> &getReloadFunc() { return reloadFunc; }
    void appendReloadFunc(FunctionType *func)
    {
        reloadFunc.emplace_back(func);
    }
    Type *getRetType() { return returnType; };
    bool checkParam(std::vector<SymbolEntry *> rparams);
};

class TypeSystem
{
private:
    static IntType commonInt;
    static IntType commonInt8;
    static IntType commonBool;
    static VoidType commonVoid;
    static FloatType commonFloat;
    static ArrayType commonARRAY;
    static FunctionType commonFunc;

public:
    static Type *intType;
    static Type* int8Type;
    static Type *voidType;
    static Type *floatType;
    static Type *arrayType;
    static Type *funcType;
    static Type *boolType;
};

inline int pairTypeCheck(std::vector<Type *> typeVec)
{
    Type *basicType1=nullptr;
    Type *basicType2=nullptr;
    std::vector<Type *> basicTypeVec;
    basicTypeVec.emplace_back(basicType1);
    basicTypeVec.emplace_back(basicType2);
    // 计算基本类型
    for (int i = 0; i < 2; i++)
    {
        if (typeVec[i]->isArray()){
            basicTypeVec[i] = dynamic_cast<ArrayType *>(typeVec[i])->getEleType();
        }
        else if (typeVec[i]->isFunc()){
            basicTypeVec[i] = dynamic_cast<FunctionType *>(typeVec[i])->getRetType();
        }
        else if (typeVec[i]->isPointer()){
            basicTypeVec[i] = dynamic_cast<PointerType *>(typeVec[i])->getValueType();
        }else{
            basicTypeVec[i] = typeVec[i];
        }
    }
    if (basicTypeVec[0] == basicTypeVec[1]){// 如果相等
        if (typeVec[0]->isArray()){ // 如果为数组则需进一步新检查
            auto arr1 = dynamic_cast<ArrayType *>(typeVec[0]);
            auto arr2 = dynamic_cast<ArrayType *>(typeVec[1]);
            if (arr1->getDimen() == arr2->getDimen()){
                auto len1 = arr1->getLenVec();
                auto len2 = arr2->getLenVec();
                for(int i=0;i<arr1->getDimen();i++){// 检查维度大小
                    if(len1[i]!=len2[i])
                        return 1;
                }
                return 0;
            }
            else
                return 1;
        }else if(typeVec[0]->isPointer()){ // 确认基本类型相同后检查是否均为指针
            return typeVec[0]!=typeVec[1];
        }
        return 0;
    }
    else{ // 如果不等
        /* f to i */
        if (basicTypeVec[0]->isInt() && basicTypeVec[1]->isFloat()){
            return 2;
        }
        /* i to f */
        else if (basicTypeVec[0]->isFloat() && basicTypeVec[1]->isInt()){
            return 3;
        }
        /* 处理void */
        else if((basicTypeVec[0]->isVoid() && !basicTypeVec[1]->isVoid())||
                (basicTypeVec[1]->isVoid() && !basicTypeVec[0]->isVoid())){
            return 4;
        }
    }
    return 0;
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
// class Value;

// struct Use
// {
//     Value *val_;
//     unsigned arg_no_;
//     Use(Value *val, unsigned no) : val_(val), arg_no_(no) {}

//     friend bool operator==(const Use & lhs, const Use & rhs){
//         return lhs.val_ == rhs.val_ && lhs.arg_no_ == rhs.arg_no_;
//     }
// };

// class Value
// {
// public:
//     explicit Value(Type *ty, const std::string &name = ""){};
//     ~Value() = default;


//     Type *get_type() const { return type_; }

//     std::list<Use> &get_use_list() { return use_list_; }

//     void add_use(Value *val, unsigned arg_no = 0){
//         use_list_.emplace_back(Use(val, arg_no));
//     };

//     bool set_name(std::string name) { 
//         if (name_ == "")
//         {
//             name_=name;
//             return true;
//         }   
//         return false; 
//     }
//     std::string get_name() {return name_;};

//     void replace_all_use_with(Value *new_val){
//          for (auto use : use_list_) {
//         auto val = dynamic_cast<User *>(use.val_);
//         val->set_operand(use.arg_no_, new_val);
//     }
//     };
//     void remove_use(Value *val){
//         auto is_val = [val] (const Use &use) { return use.val_ == val; };
//     use_list_.remove_if(is_val);
//     };

//     virtual std::string print() { return ""; }
// protected:
//     Type *type_;
//     std::list<Use> use_list_;
//     std::string name_;
// };

#endif
