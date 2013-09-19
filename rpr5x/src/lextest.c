
#include <stdio.h>
#include "symtab.h"
#include "token.h"

extern int yylex();

int main(int argc, char** argv) {
int token;

while ((token = yylex()) != 0) {

switch (token) {
  case JUMP : printf("JUMP\n"); break;

  default:
    printf("ILLEGAL CHARACTER\n"); break;


}
}
}
