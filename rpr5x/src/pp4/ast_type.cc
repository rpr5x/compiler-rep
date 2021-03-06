/* File: ast_type.cc
 * -----------------
 * Implementation of type node classes.
 */
#include "ast_type.h"
#include "ast_decl.h"
#include <string.h>
#include "hashtable.h"
#include <sstream>

extern int currentLevel;
 
/* Class constants
 * ---------------
 * These are public constants for the built-in base types (int, double, etc.)
 * They can be accessed with the syntax Type::intType. This allows you to
 * directly access them and share the built-in types where needed rather that
 * creates lots of copies.
 */

Type *Type::intType    = new Type("int");
Type *Type::doubleType = new Type("double");
Type *Type::voidType   = new Type("void");
Type *Type::boolType   = new Type("bool");
Type *Type::nullType   = new Type("null");
Type *Type::stringType = new Type("string");
Type *Type::errorType  = new Type("error"); 

Type::Type(const char *n) {
    Assert(n);
    typeName = strdup(n);
    branches = new List<Node *>();

}

char * Type::getTypeName() {
    return typeName;
}


void Type::checkNode()
{
      
}
	
void Type::checkNode2()
{


}

NamedType::NamedType(Identifier *i) : Type(*i->GetLocation()) {
    Assert(i != NULL);
    (id=i)->SetParent(this);
    this->branches = new List<Node *>();
    this->branches->Append(id);
} 

Identifier * NamedType::getId()
{
   return id;
}

char * NamedType::getTypeName()
{
  return id->getName();
}

ArrayType::ArrayType(yyltype loc, Type *et) : Type(loc) {
    Assert(et != NULL);
    (elemType=et)->SetParent(this);
    this->branches = new List<Node *>();
    this->branches->Append(elemType);

}

void ArrayType::checkNode()
{
  //printf("I am ArrayType\n");
}

void ArrayType::checkNode2()
{
  //printf("I am ArrayType\n");
}

char * ArrayType::getTypeName()
{
      Type * arrayType = elemType;
      std::stringstream ss;
      ss << arrayType;
      std::string name = ss.str();
      return (char *) name.c_str();
}



