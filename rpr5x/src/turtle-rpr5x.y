
%{
#include <stdio.h>
#include "symtab.h"
%}

%union { int i; node *n; double d;}

%token MOVE SHIFT VAR JUMP
%token FOR STEP TO DO
%token COPEN CCLOSE
%token SIN COS SQRT
%token <d> FLOAT
%token <n> ID               
%token <i> INTEGER       
%token SEMICOLON PLUS MINUS TIMES DIV OPEN CLOSE ASSIGN
%token LEFTBRACE RIGHTBRACE
%token LESS GREATER LESSOREQUAL GREATEROREQUAL EQUAL
%token PROCEDURE PARAM CALL
%token IF THEN ELSE
%token WHILE
%token COMMA
%token ERROR

%type <n> decl
%type <n> decllist

%%
program: head decllist stmtlist pretail tail;

head: { printf("%%!PS Adobe\n"
               "\n"
	       "newpath\n0 0 moveto\n\n"
	       );
      };

tail: { printf("stroke\n"); };
pretail: {printf("closepath\n");};

decllist: ;
decllist: decllist decl;

decl: VAR ID SEMICOLON { printf("/tlt%s 0 def\n",$2->symbol);} ;


stmtlist: ;
stmtlist: stmtlist stmt;


stmt: ID ASSIGN expr SEMICOLON {printf("/tlt%s exch store\n",$1->symbol);};
stmt: MOVE expr SEMICOLON {printf("0 rlineto\n");};
stmt: JUMP expr SEMICOLON {printf("0 rmoveto\n");};
stmt: SHIFT expr SEMICOLON {printf("rotate\n");};

stmt: FOR ID ASSIGN expr 
          STEP expr
	  TO expr
	  DO {printf("{ /tlt%s exch store\n",$2->symbol);} 
	     stmt {printf("} for\n");};


stmt: IF expr stmtblock {printf("} { ");} ELSE stmtblock {printf("} ifelse\n");};
stmt: IF expr stmtblock {printf("} if\n");};

stmt: WHILE expr {printf("{exit} ifelse\n");} stmtblock {printf("} loop\n");};


stmt: PROCEDURE ID {printf("/proc%s { ",$2->symbol);} stmtblock {printf("} def\n");};

 
stmt: CALL ID atomic atomic atomic atomic SEMICOLON {printf("proc%s\n", $2->symbol);};


stmtblock: LEFTBRACE stmtlist RIGHTBRACE;

stmt: COPEN stmtlist CCLOSE;

 
expr: expr PLUS term { printf("add ");};
expr: expr MINUS term { printf("sub ");};
expr: term;

term: term TIMES factor { printf("mul ");};
term: term DIV factor { printf("div ");};
term: factor;

factor: MINUS atomic { printf("neg ");};
factor: PLUS atomic;
factor: SIN factor { printf("sin ");};
factor: COS factor { printf("cos ");};
factor: SQRT factor { printf("sqrt ");};
factor: atomic;

atomic: OPEN {printf("{ ");} cond CLOSE {printf("} ");}
atomic: INTEGER {printf("%d ",$1);};
atomic: FLOAT {printf("%f ",$1);};
atomic: ID {printf("tlt%s ", $1->symbol);};
atomic: PARAM;

cond: expr LESS expr {printf("lt\n{");};
cond: expr GREATER expr {printf("gt\n{ ");};
cond: expr LESSOREQUAL expr {printf("le\n{ ");};
cond: expr GREATEROREQUAL expr {printf("ge\n{ ");};
cond: expr EQUAL expr {printf("eq\n{ ");}; 
cond: expr;
%%
int yyerror(char *msg)
{  fprintf(stderr,"Error: %s\n",msg);
   return 0;
}

int main(void)
{   yyparse();
    return 0;
}

