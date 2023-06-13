#include "Type.h"
#include <sstream>
bool debug=0;
std::vector<Type*> paramsType;
// IntType TypeSystem::commonInt = IntType(4);
IntType TypeSystem::commonInt = IntType(32);
IntType TypeSystem::commonBool = IntType(1);
VoidType TypeSystem::commonVoid = VoidType();
FloatType TypeSystem::commonFloat = FloatType(8);
ArrayType TypeSystem::commonARRAY = ArrayType(nullptr);
FunctionType TypeSystem::commonFunc = FunctionType(nullptr,paramsType);

Type* TypeSystem::intType = &commonInt;
Type* TypeSystem::voidType = &commonVoid;
Type* TypeSystem::floatType = &commonFloat;
Type* TypeSystem::arrayType = &commonARRAY;
Type* TypeSystem::boolType = &commonBool;
Type* TypeSystem::funcType = &commonFunc;

std::string PointerType::toStr()
{
    std::ostringstream buffer;
    buffer << valueType->toStr() << "*";
    return buffer.str();
}

std::string IntType::toStr()
{
    std::ostringstream buffer;
    buffer << "i" << size;
    return buffer.str();
}

std::string VoidType::toStr()
{
    return "void";
}
// Type* Type::getBasicType(){
//     if(isFunc()){
//         return dynamic_cast<FunctionType*>(this)->getRetType();
//     }
//     else if(isArray()){
//         return dynamic_cast<ArrayType*>(this)->getEleType();
//     }
//     else{
//         return this;
//     }
// }
std::string FunctionType::toStr()
{
    std::ostringstream buffer;
    // buffer << returnType->toStr() << "(";
    buffer << "(";
    for (int i=0;i<(int)paramsType.size()-1;i++)
        buffer << paramsType[i]->toStr()<<", ";
    if((int)paramsType.size()-1>=0)
        buffer << paramsType[(int)paramsType.size()-1]->toStr();
    buffer << ")";
    return buffer.str();
} 

// z1213 更新函数参数检查
// z1213 更新funccall隐式类型转换
bool FunctionType::checkParam(std::vector<SymbolEntry*> rParamsSE){
    
    std::vector<Type*> rparams;
    for (size_t i = 0; i < rParamsSE.size(); i++){
        rparams.push_back(rParamsSE[i]->getType());
    }
    /*if need debug type check*/
    // std::cout<<"p = "<<paramsType.size()<<" r = "<<rparams.size()<<"\n";
    if(paramsType.size()!=rparams.size()){
        // std::cout<<"function params number not match!\n";
        assert(paramsType.size()==rparams.size());
        return false;
        
    }
    int retCode=0;
    std::vector<Type*> typeVec;
    for (size_t i = 0; i < paramsType.size(); i++)
    {
        /*if need debug type check*/
        typeVec.clear();
        //printf("<<<<<<<<<<<<<<\n%s\n%s\n<<<<<<<<<<<<<<\n",paramsType[i]->toStr().c_str(),rparams[i]->toStr().c_str());
        typeVec.push_back(paramsType[i]);
        typeVec.push_back(rparams[i]);
        retCode = pairTypeCheck(typeVec);
        switch(retCode){
            case 0:
                return 1;
            case 1:
                return 0;
            /*f to i*/
            case 2:
                rParamsSE[i]->setIsTrans(1);
                rParamsSE[i]->setTransType(paramsType[i]);
                if(rParamsSE[i]->isConstant()){
                    int value=dynamic_cast<ConstantSymbolEntry*>(rParamsSE[i])->getIntValue();
                    dynamic_cast<ConstantSymbolEntry*>(rParamsSE[i])->setFloatValue((float)value);
                }
                break;
            /*i to f*/
            case 3:
                rParamsSE[i]->setIsTrans(1);
                rParamsSE[i]->setTransType(paramsType[i]);
                if(rParamsSE[i]->isConstant()){
                    float value=dynamic_cast<ConstantSymbolEntry*>(rParamsSE[i])->getFloatValue();
                    dynamic_cast<ConstantSymbolEntry*>(rParamsSE[i])->setIntValue((int)value);
                }
                break;
        }
        // if(paramsType[i]!=rparams[i]){
        //     if(rparams[i]->isFunc()){
        //         std::cout<<dynamic_cast<FunctionType *>(rparams[i])->getRetType()->toStr()<<"\n";
        //         if(dynamic_cast<FunctionType *>(rparams[i])->getRetType()==paramsType[i])
        //             continue;
        //     }
        //     if(paramsType[i]->isArray()&&rparams[i]->isArray()){
                
        //         ArrayType *arr1=(ArrayType*)paramsType[i];
        //         ArrayType *arr2=(ArrayType*)rparams[i];
        //         if(arr1->getEleType()!=arr2->getEleType()){
        //         std::cout<<"Array element type not match\n";
        //         assert(arr1->getEleType()==arr2->getEleType());
        //         }
                
        //         if(arr1->getDimen()!=arr2->getDimen()){
        //         std::cout<<"Array dimen not match\n";
        //         assert(arr1->getDimen()==arr2->getDimen());
        //         }
        //         continue;
        //     }
            
        //     // z1209 todo 重载
        //     else if(paramsType[i]->isPointer()&&rparams[i]->isPointer()){
        //         PointerType *ptr1=(PointerType*)paramsType[i];
        //         PointerType *ptr2=(PointerType*)rparams[i];
        //         if(ptr1->getValueType()!=ptr2->getValueType()){
        //         std::cout<<"Pointer type not match\n";
        //         assert(ptr1->getValueType()==ptr2->getValueType());
        //         }
                
        //         continue;
        //     }
        //     else if(paramsType[i]->isArray()&&rparams[i]->isPointer()){
                
        //         ArrayType *arr=(ArrayType*)paramsType[i];
        //         PointerType *ptr=(PointerType*)rparams[i];
        //         if (arr->getDimen() == 1){
        //             if (arr->getEleType() == ptr->getValueType()){
        //                 continue;
        //             }
        //         }
        //         else{
        //             if (arr->getDimen() == dynamic_cast<ArrayType *>(ptr->getValueType())->getDimen() + 1)
        //             continue;
        //             }
                
        //     }
        //     else if(paramsType[i]->isPointer()&&rparams[i]->isArray()){
                
        //         ArrayType *arr=(ArrayType*)rparams[i];
        //         PointerType *ptr=(PointerType*)paramsType[i];
        //         if (arr->getDimen() == 1){
        //             if (arr->getEleType() == ptr->getValueType()){
        //                 continue;
        //             }
        //         }
        //         else{
        //             if (arr->getDimen() == dynamic_cast<ArrayType *>(ptr->getValueType())->getDimen() + 1)
        //             continue;
        //             }
        //     }
        //     else if(paramsType[i]->isArray()&&!rparams[i]->isArray()){
        //         ArrayType *arr=(ArrayType*)paramsType[i];
        //         if(arr->getEleType()!=rparams[i]){
        //         std::cout<<"Array element type not match\n";
        //         assert(arr->getEleType()==rparams[i]);
        //         }
        //         continue;
        //     }
        //     else if(rparams[i]->isArray()&&!paramsType[i]->isArray()){
        //         ArrayType *arr=(ArrayType*)rparams[i];
        //         if(arr->getEleType()!=paramsType[i]){
        //         std::cout<<"Array element type not match\n";
        //         assert(arr->getEleType()==paramsType[i]);
        //         }
        //         continue;
        //     }
        //     //std::cout<<"The param number "<<i<<" not match\n";
        //     assert(paramsType[i]==rparams[i]);
        //     return false;
        // }
    }
    
    return true;
    }
    
PointerType::PointerType(ArrayType* arrtype,int idx) : Type(Type::PTR) {
        Type *type=arrtype;
        if(idx==0){
            this->valueType=arrtype->getEleType();
        }
        if (idx==arrtype->getDimen()){
            this->valueType=arrtype->getEleType();
        }
        else{
            while(idx--){
                type=dynamic_cast<ArrayType*>(type)->getStripEleType();
            }
            this->valueType=type;
        }
    }
std::string FloatType::toStr()
{
    return "float";
}
bool ArrayType::calHighestDimen(){
    if(arrayValue.size()==0||len[0]!=0)
        return 1;
    size_t sum = 1;

    for(size_t i=1;i<len.size();i++){
        sum*=dimen;
    }
    if(arrayValue.size()<sum){
        return 1;
    }
    else if(arrayValue.size()%sum==0){
        len[0]=arrayValue.size()/sum;
    }
    else 
        return 0;

}
void ArrayType::genArr(std::ostringstream &buffer,int dim){
    if(dim>=getDimen()){
        return;
    }
    else if(len[dim]!=0)
        buffer<<"["<<len[dim]<<" x";
    genArr(buffer,dim+1);
    if(dim==getDimen()-1)
        buffer<<" "<<eletype->toStr();
    if(len[dim]!=0)
        buffer<<"]";
    return;
}


std::string ArrayType::toStr() {
    std::ostringstream buffer;
    if(len[0]==0){
        // buffer<<(eletype)->toStr();
        buffer<<(new PointerType(eletype))->toStr();
        return buffer.str();

    }
    // buffer<<eletype->toStr();
    // for (int i=0;i<(int)len.size();i++) 
    //         buffer << '[' << len[i] << ']';
    // if((int)len.size()==0)
    //     buffer<<"[]";

    if(debug)std::cout<<"in arr gen\n";
    genArr(buffer,0);
    //const char* buffer_str = buffer.str().c_str();
    return buffer.str();
}
Type* ArrayType::getStripEleType(){
    Type* strip_arr=nullptr;
    if(!this->len.empty()){
        int new_dimen=this->getDimen()-1;
        if(new_dimen!=-1){
            strip_arr=new ArrayType(this->getEleType(),new_dimen);
            ArrayType *arr=(ArrayType *)strip_arr;
            arr->getLenVec().assign(this->len.begin(), this->len.end());
            arr->getLenVec().erase(arr->getLenVec().begin());
        }
        else{
            strip_arr=this->getEleType();
        }
    }

    return strip_arr;
    
}