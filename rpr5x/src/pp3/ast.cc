/* File: ast.cc
 * ------------
 */

#include "ast.h"
#include "ast_type.h"
#include "ast_decl.h"
#include <string.h> // strdup
#include <stdio.h>  // printf
#include "hashtable.h"

extern int currentLevel;

Node::Node(yyltype loc) {
    location = new yyltype(loc);
    parent = NULL;
    branches = new List<Node *>();        
}

Node::Node() {
    location = NULL;
    parent = NULL;
    branches = new List<Node *>();
}
	 
Identifier::Identifier(yyltype loc, const char *n) : Node(loc) {
    name = strdup(n);
}

void Identifier::checkNode()
{
   
}

char * Identifier::getName() const
{
  return name;
} 

