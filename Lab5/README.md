# 语法分析器简要说明

> Author: Zヾ(≧▽≦*)oM
> Date: 22/11/1——22/11/8

## 实现语法分析器功能

本次实验的实验要求是利用 SysY 语言的上下文无关文法，借助 Yacc 工具实现语法分析器：

+ 语法树数据结构的设计：结点类型的设计，不同类型的节点应保存的信息。
+ 扩展上下文无关文法，设计翻译模式。
+ 设计 Yacc 程序，实现能构造语法树的分析器。
+ 以文本方式输出语法树结构，验证语法分析器实现的正确性。

根据实验要求, 我们实现了如下SysY语言特性:

1. 数据类型：int, float, int[], float[]，函数返回值类型：int, float, int[], float[]；
2. 变量、常量的声明和初始化，并区分其作用域；
3. 语句：赋值（=）、表达式语句、语句块、if、while、return，break 和 continue 语句。
4. 表达式：算术表达式，关系表达式，逻辑表达式；
5. 注释（行注释、块注释）；
6. 输入输出函数；
7. 函数的声明定义和调用；
8. 数组的声明和初始化；
9. 浮点数的声明、识别、运算等；

本次实验中，我们额外实现了一些类型检查：

1. 函数实参形参类型检查；

## 小组分工和项目进度管理

本实验由朱、孟二人合作完成，如下是我们的小组分工和进度管理：

朱璟钰负责部分：

1. 实现变量、常量的连续赋值与初始化，以及相应的检查：
   * [X] 使用性检查
   * [X] 类型检查
2. 实现函数声明、定义、调用：
   * [X] 函数调用时的实参、形参的类型检查；
     * [X] 数组参数的维度匹配
3. 实现数组的声明、初始化，元素赋值及相应的检查：
   1. [X] 数组维度声明整型、常量检查；
   2. [X] 左值数组的维度检查；
   3. [X] 数组使用的下标整型检查；
4. 实现浮点数，及相应的类型检查：
   * [X] 赋值时标识符的类型检查；
     * [ ] 隐式类型转换


孟笑朵负责部分：

1. 词法处理lex的部分合并上次作业代码；
2. 实现不同类型的算术逻辑关系表达式以及表达式的优先级的调整；
3. 实现输入输出函数的识别；
4. 实现常量表达式的计算；
5. 实现输入输出函数的参数类型的检查；

22/11/1进度:

- [X] 实现lookup;
- [X] 合并lex;

22/11/2进度:

- [X] 实现单目运算符, 乘法运算符, 补充关系运算符, 逻辑运算符;

22/11/3进度:

- [X] 实现输入输出;

22/11/4进度:

- [X] 实现常量表达式的int类型的计算;
- [X] 添加输入输出函数的参数类型和返回类型;

22/11/5进度:

- [X] 实现常量表达式的float类型的计算;
- [X] 修改ConstExp的实现;

22/11/6进度:

- [X] 修复了一些bug, 比如return const;
- [X] 添加了输入输出函数的参数类型和返回类型检查;

还未实现的部分:

- [ ] 常量数组的计算;
- [ ] 常量计算的Type类型返回;

## 编译器命令

```
Usage：build/compiler [options] infile
Options:
    -o <file>   Place the output into <file>.
    -t          Print tokens.
    -a          Print abstract syntax tree.
```

## Makefile使用

* 修改测试路径：

默认测试路径为test，你可以修改为任意要测试的路径。我们已将最终所有测试样例分级上传。

如：要测试level1-1下所有sy文件，可以将makefile中的

```
TEST_PATH ?= test
```

修改为

```
TEST_PATH ?= test/level1-1
```

* 编译：

```
    make
```

编译出我们的编译器。

* 运行：

```
    make run
```

以example.sy文件为输入，输出相应的语法树到example.ast文件中。

* 测试：

```
    make testlab5
```

该命令会默认搜索test目录下所有的.sy文件，逐个输入到编译器中，生成相应的抽象语法树.ast文件到test目录中。你还可以指定测试目录：

```
    make testlab5 TEST_PATH=dirpath
```

* 清理:

```
    make clean
```

清除所有可执行文件和测试输出。