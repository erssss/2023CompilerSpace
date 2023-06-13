#include <iostream>
#include <string.h>
#include <unistd.h>
#include "Ast.h"
#include "Unit.h"
#include "MachineCode.h"
#include "LinearScan.h"
#include "ComSubExprEli.h"
#include "dominatorTree.h"
#include "mem2reg.h"
using namespace std;

Ast ast;
Unit unit;
MachineUnit mUnit;
extern FILE *yyin;
extern FILE *yyout;
extern bool *fcheck;

int yyparse();

char outfile[256] = "a.out";
bool dump_tokens;
bool dump_ast;
bool dump_ir;
bool dump_asm;
bool optimize;
int main(int argc, char *argv[])
{
    int opt;
    while ((opt = getopt(argc, argv, "Siato:O::")) != -1) 
    {
        switch (opt)
        {
        case 'o':
            strcpy(outfile, optarg);
            break;
        case 'a':
            dump_ast = true;
            break;
        case 't':
            dump_tokens = true;
            break;
        case 'i':
            dump_ir = true;
            break;
        case 'S':
            dump_asm = true;
            break;
        case 'O':
            optimize=true;
            break;
        default:
            fprintf(stderr, "Usage: %s [-o outfile] infile\n", argv[0]);
            exit(EXIT_FAILURE);
            break;
        }
    }
    if (optind >= argc)
    {
        fprintf(stderr, "no input file\n");
        exit(EXIT_FAILURE);
    }
    if (!(yyin = fopen(argv[optind], "r")))
    {
        fprintf(stderr, "%s: No such file or directory\nno input file\n", argv[optind]);
        exit(EXIT_FAILURE);
    }
    if (!(yyout = fopen(outfile, "w")))
    {
        fprintf(stderr, "%s: fail to open output file\n", outfile);
        exit(EXIT_FAILURE);
    }
    yyparse();
    if(dump_ast)
        ast.output();
    ast.genCode(&unit);
    // CSE
    if(optimize){
        ComSubExprEli em(&unit);
        em.execute();
        // dominatorTree dt(&unit);
        // dt.execute();
        // Mem2Reg mr(&unit);
        // mr.execute();
    }
    if(dump_ir)
        unit.output();
    unit.genMachineCode(&mUnit);
    if(*fcheck){
        LinearScan linearScan(&mUnit,1);
        linearScan.allocateRegisters_f();
    }
    else{
        LinearScan linearScan(&mUnit,0);
        linearScan.allocateRegisters();
    }
    if(dump_asm)
        mUnit.output();
    return 0;
}
