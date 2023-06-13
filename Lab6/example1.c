// clang -E -Xclang -ast-dump example1.c
// clang -emit-llvm -S example1.c -o example1.ll
//test const gloal var define
// 汇编	llc example.bc -o example.s
//	执行 gcc example.s -o example
// 中间代码输入输出执行 clang -o build/compiler example.ll sysyruntimelibrary/sylib.c
// 可执行程序 clang example.ll sysyruntimelibrary/sylib.c -o example
// 执行 build/compiler example.ll
//clang 中间代码执行文件生成 clang -emit-llvm -S example1.c -o example1.ll
// Use complex expression in assign structure
int main() {
    int a = 2 % 2.1;
}

// int main(){
//     int a;
//     if(a > 1){
//         a =1;
//     }else{
//         a = 2;
//     }
//     return 0;
// }