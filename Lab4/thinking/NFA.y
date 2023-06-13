%{
/****************************************************************************
Prework2
Date: 2022/10/10
****************************************************************************/
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int yylex();
extern int yyparse();
FILE* yyin;
void yyerror(const char* s);

char c[10];
char before_last_node[10];
char last_node[10];
char cur_node[10];
char next_node[10];

int sel=0;

typedef struct{
    int nodeCnt;
    char source[10];
    char tail[10];
    char nodes[20];
    char edge[200];
}nodeGroup;

#ifndef YYSTYPE
#define YYSTYPE nodeGroup
#endif

nodeGroup cur_exp;
char cur_source[10];
%}


%token ADD
%token SUB
%token MUL
%token DIV
%token OR
%token LBRACKET
%token RBRACKET

%token NODE
%token ID
%token LB
%token RB
%left ADD SUB
%left MUL DIV

%%

lines	: lines expr ';'	{ printf("%s\n", $2.edge); }
		| lines ';'
		|
		;

expr	: expr NODE
            {strcpy($$.tail,cur_node);
             strcpy($$.source,$1.source);
             strcpy($$.edge,$1.edge);
             strcat($$.edge,"\n");
             strcat($$.edge,$1.tail);
             strcat($$.edge," --> ");
             strcat($$.edge,cur_node);
             strcpy($$.nodes,$1.nodes);
             strcat($$.nodes,cur_node);
             $$.nodeCnt=strlen($$.nodes);
             cur_exp=$$;
             //printf("%s %d ++\n",$$.nodes,$$.nodeCnt);
            }   

        | expr ADD expr
            {           
            strcpy($$.edge,$1.edge);
            strcat($$.edge,"\n");
            for(int i=0;i<strlen($1.tail);i++){
                char tmp[2]={$1.tail[i]};
                strcat($$.edge,tmp);
                strcat($$.edge," --> ");
                strcat($$.edge,$3.source);
                strcat($$.edge,"\n");
                strcat($$.edge,tmp);
                strcat($$.edge," --> ");
                strcat($$.edge,tmp);
                strcat($$.edge,"\n");
                }                
            strcat($$.edge,$3.edge);
            strcat($$.nodes,$1.nodes);
            strcat($$.nodes,$3.nodes);
            $$.nodeCnt=strlen($$.nodes);
            cur_exp=$$;
            }           

        | expr MUL expr
            {
             strcpy($$.source,$1.source);
             strcpy($$.tail,$3.tail);  
             strcpy($$.edge,$1.edge);
             strcat($$.edge,"\n");
            for(int i=0;i<strlen($1.tail);i++){
                char tmp[2]={$1.tail[i]};
                strcat($$.edge,tmp);
                strcat($$.edge," --> ");
                strcat($$.edge,$3.source);
                strcat($$.edge,"\n");
                strcat($$.edge,tmp);
                strcat($$.edge," --> ");
                strcat($$.edge,tmp);
                strcat($$.edge,"\n");
            }
             if($1.nodeCnt-2>=0){
                char tmp[2]={$1.nodes[$1.nodeCnt-2]};
                strcat($$.edge,tmp);
                strcat($$.edge," --> ");
                strcat($$.edge,$3.source);
             }
            strcat($$.nodes,$1.nodes);
            strcat($$.nodes,$3.nodes);
            $$.nodeCnt=strlen($$.nodes);
            cur_exp=$$;
            }   	

		| expr LB STRING RB	    
            {strcpy($$.edge,$1.edge);
             strcat($$.edge,"\n");
             strcat($$.edge,$1.tail);
             for(int i=0;i<$3.nodeCnt;i++){
                char tmp[2]={$3.nodes[i]};
                strcat($$.edge," --> ");
                strcat($$.edge,tmp);
                }
             strcpy($$.source,$1.source);
             char tmp[2]={$3.nodes[$3.nodeCnt-1]};
             strcpy($$.tail,tmp);
             cur_exp=$$;
             char tmp_[2]={$3.nodes[0]};
             strcpy(cur_source,tmp_);
             }
             
		| expr LBRACKET STRING RBRACKET	    
            {strcpy($$.source,$1.source);
             strcpy($$.tail,$3.nodes);
             strcpy($$.nodes,$1.nodes);
             strcat($$.nodes,$3.nodes);
             strcpy($$.edge,$1.edge);
             strcat($$.edge,"\n");
             for(int i=0;i<$3.nodeCnt;i++){
                char tmp[2]={$3.nodes[i]};
                strcat($$.edge,$1.tail);
                strcat($$.edge," --> ");
                strcat($$.edge,tmp);
                strcat($$.edge,"\n");
                }
                cur_exp=$$;
             }
             
        | NODE
            {strcpy($$.edge,"\0");
             strcpy($$.source,cur_node);
             strcpy($$.tail,cur_node);
             strcpy($$.nodes,cur_node);
             $$.nodeCnt=1;
             }
        |
        ;
STRING  : STRING NODE 
            {strcpy($$.edge,$1.edge);
             strcpy($$.source,$1.source);
             strcpy($$.tail,cur_node);
             strcpy($$.nodes,$1.nodes);
             strcat($$.nodes,cur_node);
             $$.nodeCnt=strlen($$.nodes);
             }
        | NODE
            {strcpy($$.edge,"\0");
             strcpy($$.source,cur_node);
             strcpy($$.tail,cur_node);
             strcpy($$.nodes,cur_node);
             $$.nodeCnt=1;
             }
        ;


		
%%

// programs section

int yylex()
{
    // place your token retrieving code here
    // before_last_node[0]=' ';
    // last_node[0]=' ';
    // cur_node[0]=' ';
    // next_node[0]=' ';
    int t;
    while (1) {
        t = getchar();
        if (t == ' ' || t == '\t' || t == '\n') {
            //do nothing
        }
        else if (isdigit(t)||isalpha(t)){
            before_last_node[0]=last_node[0];
            last_node[0]=cur_node[0];
            cur_node[0]=t;
            return NODE;
        }
        else if (t == '+')
            return ADD;
        else if (t == '-')
            return SUB;
        else if (t == '*')
            return MUL;
        else if (t == '/')
            return DIV;
        else if (t == '(')
            return LB;
        else if (t == ')')
            return RB;    
        else if (t == '[')
            return LBRACKET;
        else if (t == ']')
            return RBRACKET;    
        else if (t == '|')
            return OR;
        else {
            return t;
        }
    }
}

int main(void)
{
    yyin = stdin;
    do {
        yyparse();
    } while (!feof(yyin));
    return 0;
}
void yyerror(const char* s) {
    fprintf(stderr,"Parse error:%s\n",s);
    exit(1);
}
