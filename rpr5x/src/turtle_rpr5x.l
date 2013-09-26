
%{
#include "symtab.h"
#include "token-rpr5x.h"
%}

INTEGER [0-9]+
FLOAT [0-9]+\.[0-9]+
%%

\/\/.*           ;
[[:space:]]      ;

MOVE            return MOVE;
SHIFT           return SHIFT;
VAR             return VAR;
JUMP            return JUMP;
FOR             return FOR;
STEP            return STEP;
TO              return TO;
DO              return DO;
BEGIN           return COPEN;
END             return CCLOSE;
WHILE           return WHILE;
IF              return IF;
THEN            return THEN;
ELSE            return ELSE;
CALL            return CALL;
PARAM           return PARAM;


sin             return SIN;
cos             return COS;
sqrt            return SQRT;

;               return SEMICOLON;
\+              return PLUS;
-               return MINUS;
\*              return TIMES;
\/              return DIV;
\(              return OPEN;
\)              return CLOSE;
=               return ASSIGN;
\==             return EQUAL;
\<              return LESS;
\>              return GREATER;
\>=             return GREATEROREQUAL;
\<=             return LESSOREQUAL;
\,              return COMMA;
\{              return LEFTBRACE;
\}              return RIGHTBRACE;
PROCEDURE       return PROCEDURE;


[a-zA-Z]+       { yylval.n = lookup(yytext);
	          if(yylval.n == NULL)
                    yylval.n = insert(yytext);
                  return ID;
                }

{FLOAT}         yylval.d = atof(yytext);  return FLOAT;
{INTEGER}       yylval.i = atoi(yytext);  return INTEGER;

.               return ERROR;/*fprintf(stderr, "Unexpected character %c (%2X)\n", yytext[0],yytext[0]);*/

%%

/*main(){
  int token;
  while((token = yylex()) != 0)
  {
    switch(token){
      case PROCEDURE:      printf("PROCEDURE\n"); break;
      case JUMP:           printf("JUMP\n"); break;
      case MOVE:           printf("MOVE\n"); break;
      case SHIFT:          printf("SHIFT\n"); break;
      case VAR:            printf("VAR\n"); break;
      case FOR:            printf("FOR\n"); break;
      case STEP:           printf("STEP\n"); break;
      case TO:             printf("TO\n"); break;
      case DO:             printf("DO\n"); break;
      case COPEN:          printf("BEGIN\n"); break;
      case CCLOSE:         printf("END\n"); break;
      case SIN:            printf("SIN\n"); break;
      case COS:            printf("COS\n"); break;
      case SQRT:           printf("SQRT\n"); break;
      case SEMICOLON:      printf("SEMICOLON\n"); break;
      case PLUS:           printf("PLUS\n"); break;
      case MINUS:          printf("MINUS\n"); break;
      case TIMES:          printf("TIMES\n"); break;
      case DIV:            printf("DIVIDE\n"); break;
      case OPEN:           printf("OPEN\n"); break;
      case CLOSE:          printf("CLOSE\n"); break;
      case ASSIGN:         printf("ASSIGN\n"); break;
      case ID:             printf("ID %s\n", yylval.n->symbol); break;
      case FLOAT:          printf("FLOAT %f\n", yylval.d); break;
      case INTEGER:        printf("INTEGER %d\n", yylval.i); break;
      case IF:             printf("IF\n"); break;
      case THEN:           printf("THEN\n"); break;
      case ELSE:           printf("ELSE\n"); break;
      case WHILE:          printf("WHILE\n"); break;
      case EQUAL:          printf("EQUAL\n"); break;
      case LESS:           printf("LESS\n"); break;
      case GREATER:        printf("GREATER\n"); break;
      case COMMA:          printf("COMMA\n"); break;
      case LEFTBRACE:      printf("LEFTBRACE\n"); break;
      case RIGHTBRACE:     printf("RIGHTBRACE\n"); break;

      default:             printf("Illegal Character\n"); break;
    }
  }
}*/












