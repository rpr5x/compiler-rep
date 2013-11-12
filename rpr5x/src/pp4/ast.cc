/* File: ast.cc
 * ------------
 */

#include "ast.h"
#include "ast_type.h"
#include "ast_decl.h"
#include <string.h> // strdup
#include <stdio.h>  // printf
#include "hashtable.h"
#include "errors.h"
#include "ast_expr.h"

extern Hashtable<Decl*> *table;
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
    branches = new List<Node *>();
}

Identifier::Identifier(const char *n):Node()
{
    name = strdup(n); 
    branches = new List<Node *>();
}


void Identifier::checkNode()
{

  //Increment for various header scopes
  Node * parentNode = this->GetParent();
  ClassDecl * parentClass = dynamic_cast<ClassDecl *>(parentNode);
  InterfaceDecl * parentInterface = dynamic_cast<InterfaceDecl *>(parentNode);
  FnDecl * parentFunction = dynamic_cast<FnDecl *>(parentNode);

  if(parentClass)//Enter class header scope
  {
    currentLevel++;
    parentClass->level = currentLevel;    
    Decl * oldDecl = table->Lookup(this->getName());
    ClassDecl * oldClassDecl = dynamic_cast<ClassDecl *>(oldDecl);
         

    //Cannot have two class declarations with the same name in the same scope (Handle shadowing later)
    if(oldClassDecl) 
    {
      
      if(currentLevel == oldClassDecl->level)
      {
       ReportError::DeclConflict(parentClass, oldClassDecl);
       //currentLevel--;
      }
    }
    else
    {
      table->Enter(parentClass->getIdentifier()->getName(), parentClass);
  
    }
    currentLevel++; //Increment for class local scope
  }   
  else if(parentInterface)//Enter interface header scope
  {
    currentLevel++;
    parentInterface->level = currentLevel;
    Decl * oldDecl = table->Lookup(this->getName());
    InterfaceDecl * oldInterfaceDecl = dynamic_cast<InterfaceDecl *>(oldDecl);
    
    if(oldInterfaceDecl)
    {
      if(currentLevel == oldInterfaceDecl->level)
      {
        ReportError::DeclConflict(parentInterface, oldInterfaceDecl);
      }
    }
    else
    {
      //printf("The interface is at level %d\n", parentInterface->level);
      table->Enter(parentInterface->getIdentifier()->getName(), parentInterface);
    }
    currentLevel++;//Increment for interface local scope
  }
  else if(parentFunction) //Enter function header scope
  {
    currentLevel++;
    parentFunction->level = currentLevel;
    Decl * oldDecl = table->Lookup(this->getName());
    FnDecl * oldFnDecl = dynamic_cast<FnDecl *>(oldDecl);
    VarDecl * oldVarDecl = dynamic_cast<VarDecl *>(oldDecl);
    
    if(oldFnDecl)
    { 
      //Parent of the old function (if it's a class or interface)
      Node * parentDecl1 = oldFnDecl->GetParent();
      ClassDecl * cd1 = dynamic_cast<ClassDecl *>(parentDecl1);
      InterfaceDecl * id1 = dynamic_cast<InterfaceDecl *>(parentDecl1);

      //Parent of the current function (if it's a class or interface)
      Node * parentDecl2 = parentFunction->GetParent();
      ClassDecl * cd2 = dynamic_cast<ClassDecl *>(parentDecl2);
      InterfaceDecl * id2 = dynamic_cast<InterfaceDecl *>(parentDecl2);
      

      if(cd1 && cd2)
      { 
        //Report a declaration conflict only if two functions exist in the same class scope and their levels are the same.
        if(currentLevel == oldFnDecl->level && strcmp(cd1->getIdentifier()->getName(), cd2->getIdentifier()->getName()) == 0 )
        {
          ReportError::DeclConflict(parentFunction, oldFnDecl);

        }
      }
      else if(id1 && id2)
      {
                 
        //Same as above except for interfaces
        if(currentLevel == oldFnDecl->level && strcmp(id1->getIdentifier()->getName(), id2->getIdentifier()->getName()) == 0)
         ReportError::DeclConflict(parentFunction, oldFnDecl);
      }
      else if(cd1 && id2) 
      {
          //Do nothing.  We need this case to avoid printing a declaration conflict if the same funciton name occurs in both
       //an old class and the current interface. 


      }
      else if(id1 && cd2)
      {
          //Same as the previous case except the interface is old and the class is current.

      }
      else if(cd2)
      {
         //Accounts for the case where a function is declared before the current class declaration with both having the same name
      }
      else if(cd1)
      {
         //Same as the previous one except that the class is declared first
      }
      else if(id2)
      {
         //Accounts for the case where a function is declared before the current interface declaration with both having the same name
      }
      else if(id1)
      {
         //Same as before except the interface is declared first
      } 
      else
      {
        
        ReportError::DeclConflict(parentFunction, oldFnDecl);

      }
    }
    else if(oldVarDecl)
    {
        
      if((currentLevel == oldVarDecl->level + 1) && strcmp(this->getName(), oldVarDecl->getIdentifier()->getName()) == 0)
      {
        ReportError::DeclConflict(parentFunction, oldVarDecl);
      }

    }
  
    else
    {
      table->Enter(parentFunction->getIdentifier()->getName(), parentFunction);
    }

    currentLevel++;//Increment for function parameter scope
    
  }
  }

void Identifier::checkNode2()
{
  Node * parentNode = this->GetParent();
  ClassDecl * parentClass = dynamic_cast<ClassDecl *>(parentNode);
  InterfaceDecl * parentInterface = dynamic_cast<InterfaceDecl *>(parentNode);
  FnDecl * parentFunction = dynamic_cast<FnDecl *>(parentNode);
  LValue * lVal = dynamic_cast<LValue *>(parentNode);
  



  if(parentClass)//Enter class header scope
  {
    currentLevel++;
    parentClass->level = currentLevel;     

    currentLevel++; //Increment for class local scope
  }   
  else if(parentInterface)//Enter interface header scope
  {
    currentLevel++;
    parentInterface->level = currentLevel;
    currentLevel++;//Increment for interface local scope
  }
  else if(parentFunction) //Enter function header scope
  {
    currentLevel++;
    parentFunction->level = currentLevel;
    currentLevel++;//Increment for function parameter scope
    
    
  }
  else if(lVal) //Enter if statement scope (Check if if statement scope is correct)
  {
    /*currentLevel++;
    Node * parentLval = lVal->GetParent();
    Stmt * parentStmt = dynamic_cast<Stmt *>(parentLval);   
    //Loop until we reach a parent node that is in the Stmt class; be careful here when dealing with only StmtBlocks
    while(parentStmt == NULL)
    {
      parentLval = parentLval->GetParent();
      parentStmt = dynamic_cast<Stmt *>(parentLval);
    }
    IfStmt * ifStmt = dynamic_cast<IfStmt *>(parentStmt);
    if(ifStmt)  printf("I am an if statement\n");*/  

  }
}

char * Identifier::getName() const
{
  return name;
} 

