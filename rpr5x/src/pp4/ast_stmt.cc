/* File: ast_stmt.cc
 * -----------------
 * Implementation of statement node classes.
 */
#include "ast_stmt.h"
#include "ast_type.h"
#include "ast_decl.h"
#include "ast_expr.h"
#include "hashtable.h"
#include "ast.h"
#include "errors.h"

extern Hashtable<Decl *> *table;
void inOrderTraversal(Node * root);
void secondPass(Node * root);

 
Program::Program(List<Decl*> *d) {
    Assert(d != NULL);
    (decls=d)->SetParentAll(this);
    this->branches = new List<Node *>(); 
    for(int i = 0; i < d->NumElements(); i++)
      this->branches->Append(d->Nth(i));
    
}

//First pass
void Program::Check() {
     
    
    inOrderTraversal(this);    
    currentLevel = 0;  //Just to be safe, we reset the currentLevel here for the second pass.    
     
    /* pp3: here is where the semantic analyzer is kicked off.
     *      The general idea is perform a tree traversal of the
     *      entire program, examining all constructs for compliance
     *      with the semantic rules.  Each node can have its own way of
     *      checking itself, which makes for a great use of inheritance
     *      and polymorphism in the node classes.
     */
}

//Second pass
void Program::secondCheck()
{
    secondPass(this);    

}

void Program::inOrderTraversal(Node *  root)
{
  /*if(root == NULL)
    return;*/
  
  
  
  List<Node *> *d = root->GetBranchList();
     
  if(d->NumElements() == 0)
  {
    root->checkNode();
    return;
  }   


  //printf("The number of elements is %d\n", d->NumElements());
 
  int listSize = d->NumElements();

  for(int i = 0; i < listSize; i++)
  { 
    inOrderTraversal(d->Nth(i));
     
    if((i == listSize/2 - 1) || ((i == 0) && (listSize == 1)))
    {
      root->checkNode(); 
    }
  }
   

   
  /*inOrderTraversal(root->GetLeftChild());  
  root->checkNode();     
  inOrderTraversal(root->GetRightChild()); 
  */

}

void Program::secondPass(Node * root)
{
   List<Node *> *d = root->GetBranchList();
   //  printf("The size of d's branch list is %d\n", d->NumElements());


    if(d->NumElements() == 0)
    {
      root->checkNode2();
      return;
    }   

    //printf("The number of elements is %d\n", d->NumElements());
 
    int listSize = d->NumElements();

    for(int i = 0; i < listSize; i++)
    { 
      secondPass(d->Nth(i));

      if((i == listSize/2 - 1) || ((i == 0) && (listSize == 1))) 
        root->checkNode2();
   
    }
  
   
}


  /*Node * body = this->GetParent();
  StmtBlock *trueBody = dynamic_cast<StmtBlock *>(body);
  List<Stmt*> *bodyStmts = trueBody->getStmtList();
  //Stmt * lastStmtDecl = bodyStmts->Nth(bodyStmts->NumElements()-1);
  printf("Hey there\n");
  if(bodyStmts->NumElements() > 0)
  {
     printf("Hi\n");
     for(int i = 0; i < bodyStmts->NumElements(); i++) 
     {
       if(this == bodyStmts->Nth(i))  //Attempt to compare two pointers by comparing their addresses       
       {
         printf("Hello\n"); 
 
       }
     }
  } */   


StmtBlock::StmtBlock(List<VarDecl*> *d, List<Stmt*> *s) {
    Assert(d != NULL && s != NULL);
    (decls=d)->SetParentAll(this);
    (stmts=s)->SetParentAll(this);
    this->branches = new List<Node*>();

    for(int i = 0; i < decls->NumElements(); i++)
      this->branches->Append(decls->Nth(i));
    for(int i = 0; i < s->NumElements(); i++)
      this->branches->Append(stmts->Nth(i));

    //printf("The number of decls is %d and the number of stmts is %d\n", decls->NumElements(), stmts->NumElements()); 
}

void StmtBlock::checkNode()
{ 

 
  //Exit function body scope if the number of variable declarations/statements is either
  //zero or one
  FnDecl * fnDecl = dynamic_cast<FnDecl *>(this->GetParent());
  if(fnDecl && this->branches->NumElements() < 2)
  { 
    currentLevel--; //Exit function body scope 
    currentLevel--;//Exit function parameter scope
    currentLevel--; //Exit function header scope
   
  }

    
    //Exit class scope for functions that have less than 2 statements inside them.  The distinction
    //is necessary because of how in-order traversal is conducted.
    FnDecl * parentFunc = dynamic_cast<FnDecl*>(this->GetParent());
    ClassDecl * parentClass2 = dynamic_cast<ClassDecl *>(this->GetParent());  //Used in the "else if" case below 

    if(parentFunc && this->branches->NumElements() < 2)
    {
      
      ClassDecl * parentClass = dynamic_cast<ClassDecl *>(parentFunc->GetParent());
      if(parentClass)
      { 
        List<Decl *> *classFields = parentClass->getMembers();
        int fieldListSize = classFields->NumElements();
        Decl * lastField = classFields->Nth(fieldListSize-1);
        FnDecl * lastFn = dynamic_cast<FnDecl *>(lastField);
        VarDecl * lastVar = dynamic_cast<VarDecl *>(lastField);
        if(lastFn && strcmp(parentFunc->getIdentifier()->getName(), lastFn->getIdentifier()->getName()) == 0)
        {
          currentLevel--; //Exit class local scope
          currentLevel--; //Exit class header scope     
        } 
      }
    }

    //Exit class scope if you have one or less variables/statements in the class declaration (and no functions)
    else if(parentClass2 && this->branches->NumElements() < 2)
    {
      currentLevel--;

    }
    //printf("I am a statement block\n"); 
      
}

void StmtBlock::checkNode2()
{
  //Exit function body scope if the number of variable declarations/statements is either
  //zero or one
  if(this->branches->NumElements() < 2)
  {
     
    currentLevel--; //Exit function body scope 
    currentLevel--;//Exit function parameter scope
    currentLevel--; //Exit function header scope

  }
    //Exit class scope for functions that have less than 2 statements inside them.  The distinction
    //is necessary because of how in-order traversal is conducted.
    FnDecl * parentFunc = dynamic_cast<FnDecl*>(this->GetParent());
    ClassDecl * parentClass2 = dynamic_cast<ClassDecl *>(this->GetParent());  //Used in the "else if" case below
    if(parentFunc && this->branches->NumElements() < 2)
    {
      ClassDecl * parentClass = dynamic_cast<ClassDecl *>(parentFunc->GetParent());
      if(parentClass)
      { 
        List<Decl *> *classFields = parentClass->getMembers();
        int fieldListSize = classFields->NumElements();
        Decl * lastField = classFields->Nth(fieldListSize-1);
        FnDecl * lastFn = dynamic_cast<FnDecl *>(lastField);
        VarDecl * lastVar = dynamic_cast<VarDecl *>(lastField);
        if(lastFn && strcmp(parentFunc->getIdentifier()->getName(), lastFn->getIdentifier()->getName()) == 0)
        {
         // table->Remove(parentClass->getIdentifier()->getName(), parentClass);
          currentLevel--;//Exit class local scope
          currentLevel--;//Exit class header scope      
          //printf("I am function %s and I am exiting the class scope\n", lastFn->getIdentifier()->getName());
        } 
      }
    }
    //Exit class scope if you have one or less variables/statements in the class declaration (and no functions)
    else if(parentClass2 && this->branches->NumElements() < 2)
    {
      currentLevel--;

    } 

     //printf("I am a statement block\n"); 

}



List<VarDecl*> * StmtBlock::getDeclList()
{
  return  decls;
}

List<Stmt*> * StmtBlock::getStmtList()
{
  return stmts;
} 

//Populate the branches for each of the Stmt classes
ConditionalStmt::ConditionalStmt(Expr *t, Stmt *b) { 
    Assert(t != NULL && b != NULL);
    (test=t)->SetParent(this); 
    (body=b)->SetParent(this);
    this->branches = new List<Node *>();

}

ForStmt::ForStmt(Expr *i, Expr *t, Expr *s, Stmt *b): LoopStmt(t, b) { 
    Assert(i != NULL && t != NULL && s != NULL && b != NULL);
    (init=i)->SetParent(this);
    (step=s)->SetParent(this);
    this->branches = new List<Node *>();
    this->branches->Append(init);
    this->branches->Append(test);
    this->branches->Append(step);
    this->branches->Append(body);


}

void ForStmt::checkNode()
{
  currentLevel++;
  //printf("I am a for statement and I am level %d\n", currentLevel);

  




}

void ForStmt::checkNode2()
{
 currentLevel++;
 //printf("I am a for statement and I am level %d\n", currentLevel);

 Expr * testExpr = this->test;
 LogicalExpr * l = dynamic_cast<LogicalExpr *>(testExpr);
 RelationalExpr * r = dynamic_cast<RelationalExpr *>(testExpr);   
 EqualityExpr * e = dynamic_cast<EqualityExpr *>(testExpr);

 if(!l && !r && !e)
 {
   ReportError::TestNotBoolean(testExpr);
 }
}

WhileStmt::WhileStmt(Expr *test, Stmt *body):LoopStmt(test, body)
{
  this->branches = new List<Node *>();
  this->branches->Append(test);
  this->branches->Append(body);

}


void WhileStmt::checkNode()
{
  currentLevel++;
  
  //printf("This is a while statement and the level is %d\n", currentLevel);
}

void WhileStmt::checkNode2()
{
  currentLevel++;
  //printf("This is a while statement and the level is %d\n", currentLevel);

  Expr * testExpr = this->test;
  LogicalExpr * l = dynamic_cast<LogicalExpr *>(testExpr);
  RelationalExpr * r = dynamic_cast<RelationalExpr *>(testExpr);   
  EqualityExpr * e = dynamic_cast<EqualityExpr *>(testExpr);
  BoolConstant * b = dynamic_cast<BoolConstant *>(testExpr);

  if(!l && !r && !e && !b)
  {
    ReportError::TestNotBoolean(testExpr);
  }


}

IfStmt::IfStmt(Expr *t, Stmt *tb, Stmt *eb): ConditionalStmt(t, tb) { 
    Assert(t != NULL && tb != NULL); // else can be NULL
    elseBody = eb;
    if (elseBody) elseBody->SetParent(this);
    this->branches = new List<Node *>();
    this->branches->Append(test);
    this->branches->Append(body);
    if(elseBody) this->branches->Append(elseBody);
}

void IfStmt::checkNode()
{
  currentLevel++; //Accounts for scope for the body of the if statement
  //printf("I am an if statement and I am level %d\n", currentLevel);
}

void IfStmt::checkNode2()
{
  currentLevel++; //Accounts for scope for the body of the if statement
  //printf("I am an if statement and I am level %d\n", currentLevel);

  Expr * testExpr = this->test;
  LogicalExpr * l = dynamic_cast<LogicalExpr *>(testExpr);
  RelationalExpr * r = dynamic_cast<RelationalExpr *>(testExpr);   
  EqualityExpr * e = dynamic_cast<EqualityExpr *>(testExpr);
  BoolConstant * b = dynamic_cast<BoolConstant *>(testExpr);

  if(!l && !r && !e && !b)
  {
    ReportError::TestNotBoolean(testExpr);
  }
}

SwitchStmt::SwitchStmt(Expr * t, List<Stmt*> *cb, Stmt * ds) : Stmt() {
   Assert(t != NULL && cb != NULL);//ds can be NULL
   test = t;
   caseBody = cb;
   test->SetParent(this);
   caseBody->SetParentAll(this);
   if(ds) (defaultStmt = ds)->SetParent(this);
   this->branches = new List<Node *>();
   this->branches->Append(test);
   for(int i = 0; i < caseBody->NumElements(); i++)
     this->branches->Append(caseBody->Nth(i));
   if(ds) this->branches->Append(defaultStmt);


}

void SwitchStmt::checkNode()
{
 
  //printf("I am a switch statement and I am level %d\n", currentLevel);
}

void SwitchStmt::checkNode2()
{
  //printf("I am a switch statement and I am level %d\n", currentLevel);
}



CaseStmt::CaseStmt(IntConstant * intConstant, List<Stmt*> *statements): Stmt() {
     Assert(intConstant != NULL && statements != NULL);
     caseInt = intConstant;
     stmts = statements;
     caseInt->SetParent(this);
     stmts->SetParentAll(this);
     this->branches = new List<Node*>();
     this->branches->Append(caseInt);
     

     for(int i = 0; i < stmts->NumElements(); i++)
       this->branches->Append(stmts->Nth(i));
     
}

void CaseStmt::checkNode()
{
  //printf("I am a Case statement\n");
}

void CaseStmt::checkNode2()
{
 //printf("I am a Case statement\n");

}

DefaultStmt::DefaultStmt(List<Stmt *> *statements): Stmt() {
    stmts = statements; //statements can be NULL 
    if(stmts) stmts->SetParentAll(this);
    this->branches = new List<Node*>();
    if(stmts)
    {
      for(int i = 0; i < stmts->NumElements(); i++)
        this->branches->Append(stmts->Nth(i));
    }

}

void DefaultStmt::checkNode()
{
  //printf("I am a default statement\n");
}

void DefaultStmt::checkNode2()
{
 //printf("I am a default statement\n");

}

ReturnStmt::ReturnStmt(yyltype loc, Expr *e) : Stmt(loc) { 
    Assert(e != NULL);
    (expr=e)->SetParent(this);
    this->branches = new List<Node *>();
    this->branches->Append(e);


}

void ReturnStmt::checkNode()
{
  //printf("I am a return statement\n");
}

void ReturnStmt::checkNode2()
{
 Expr * returnExpr = this->expr;
 FieldAccess * f = dynamic_cast<FieldAccess *>(returnExpr);  //Assume that only variables can be returned
 IntConstant * i = dynamic_cast<IntConstant *>(returnExpr); 
 DoubleConstant * d = dynamic_cast<DoubleConstant *>(returnExpr); 
 BoolConstant * b = dynamic_cast<BoolConstant *>(returnExpr);
 StringConstant * s = dynamic_cast<StringConstant *>(returnExpr); 
 NamedType * ntReturn = dynamic_cast<NamedType *>(returnExpr);
 
 Node * firstParent = this->GetParent();  //Should be StmtBlock
 Node * secondParent = firstParent->GetParent();  //Should be FnDecl
 FnDecl * parentFn = dynamic_cast<FnDecl *>(secondParent);

 //  Check if the return type of the variable matches with what the function says can be returned.
 //If there is no match, report an error.
 if(f && parentFn)
 {  
   Node * field = table->Lookup(f->getIdentifier()->getName());
   VarDecl * varField = dynamic_cast<VarDecl *>(field);
   //printf("The variable is type %s and the function is type %s\n", varField->getType()->getTypeName(), parentFn->getReturnType()->getTypeName());

   if(varField && strcmp(varField->getType()->getTypeName(), parentFn->getReturnType()->getTypeName()) != 0)
     ReportError::ReturnMismatch(this, varField->getType(), parentFn->getReturnType());
 }
 else if(i && parentFn && strcmp("int" , parentFn->getReturnType()->getTypeName()) != 0)
 {  
   ReportError::ReturnMismatch(this, Type::intType, parentFn->getReturnType());
 }
 else if(d && parentFn && strcmp("double" , parentFn->getReturnType()->getTypeName()) != 0)
 {  
   ReportError::ReturnMismatch(this, Type::doubleType, parentFn->getReturnType());
 }
 else if(b && parentFn && strcmp("bool" , parentFn->getReturnType()->getTypeName()) != 0)
 {  
   ReportError::ReturnMismatch(this, Type::boolType, parentFn->getReturnType());
 }
 else if(s && parentFn && strcmp("string" , parentFn->getReturnType()->getTypeName()) != 0)
 {  
   ReportError::ReturnMismatch(this, Type::stringType, parentFn->getReturnType());
 }
 else if(ntReturn && parentFn && strcmp(ntReturn->getId()->getName(), parentFn->getReturnType()->getTypeName()) != 0)
 {
   ReportError::ReturnMismatch(this, ntReturn, parentFn->getReturnType());
 }
 
 //printf("I am a return statement\n");

}

void BreakStmt::checkNode()
{
 //printf("I am a break statement\n");
}
 

void BreakStmt::checkNode2()
{
  Node * parent = this->GetParent();
  LoopStmt * l = dynamic_cast<LoopStmt *>(parent);
  Program * p = dynamic_cast<Program *>(parent);
  CaseStmt * c = dynamic_cast<CaseStmt *>(parent);
  DefaultStmt * d = dynamic_cast<DefaultStmt *>(parent);

  //  Checks to make sure a break statement is used only inside a loop or
  //a case/default statement inside of a switch statement.  We walk upwards
  // through the tree until we either hit a LoopStmt or the Program root node
  // in which we case we exit.  If we hit the Program root node, it means we 
  //didn't find an enclosing loop so we report the error.
  while(!l && !c && !d && !p)
  {
    parent = parent->GetParent();
    l = dynamic_cast<LoopStmt *>(parent);
    p = dynamic_cast<Program *>(parent);
    c = dynamic_cast<CaseStmt *>(parent);
    d = dynamic_cast<DefaultStmt *>(parent);
  }

  if(p)
  {
    ReportError::BreakOutsideLoop(this);

  }

  

 //printf("I am a break statement\n");



}
 
PrintStmt::PrintStmt(List<Expr*> *a) {    
    Assert(a != NULL);
    (args=a)->SetParentAll(this);
    this->branches = new List<Node *>();
    for(int i = 0; i < args->NumElements(); i++)
    {
      this->branches->Append(args->Nth(i)); 
    } 
}

void PrintStmt::checkNode()
{
  //printf("I am a print statement\n"); 
  
}

void PrintStmt::checkNode2()
{
  //printf("I am a print statement\n");
  //Do some error checking here(Might put in things for function calls later)
  List<Expr*> *arguments = this->args;
  IntConstant * ic;
  StringConstant *sc;
  BoolConstant *bc;
  FieldAccess *fa;
  ArrayAccess *aa;
  DoubleConstant *dc;
  NullConstant *nc;

  for(int i = 0; i < arguments->NumElements(); i++)
  {
     Expr * arg = arguments->Nth(i);
     ic = dynamic_cast<IntConstant*>(arg); 
     sc = dynamic_cast<StringConstant*>(arg);
     bc = dynamic_cast<BoolConstant*>(arg);
     fa = dynamic_cast<FieldAccess*>(arg);
     aa = dynamic_cast<ArrayAccess*>(arg);
     dc = dynamic_cast<DoubleConstant*>(arg);
     nc = dynamic_cast<NullConstant*>(arg); 

     if(!ic && !sc && !bc && !fa && !aa)
     {
       if(dc)
        ReportError::PrintArgMismatch(arg, i, Type::doubleType);
       else if(nc)
        ReportError::PrintArgMismatch(arg, i, Type::nullType);
     }
     else if(fa)
     {
       Node * d = table->Lookup(fa->getIdentifier()->getName());
       VarDecl * v = dynamic_cast<VarDecl *>(d);
       if(v)
       {
         Type * t1 = v->getType();
         char * t1Name = t1->getTypeName();
         if(strcmp(t1Name, "int") != 0 && strcmp(t1Name, "string") != 0 && strcmp(t1Name, "bool") != 0)
           ReportError::PrintArgMismatch(arg, i, t1);
       }
     }
     /*else if(aa)
     {
       
       Expr * arrayBase = aa->getBase();
       FieldAccess * baseField = dynamic_cast<FieldAccess *>(arrayBase);
       if(baseField)
       { 
         Node * d = table->Lookup(baseField->getIdentifier()->getName());
         VarDecl * v = dynamic_cast<VarDecl *>(d);
         
         if(v)
         {
            Type * baseType = v->getType();  //Figure out a way to get the name of the data type from the Type pointer
             
                                    
            

            //if(strcmp(baseType, "int") != 0 && strcmp(baseType, "string") != 0 && strcmp(baseType, "bool") != 0)
             //  ReportError::PrintArgMismatch(arg, i, baseType);
   
 
         }                      
       }
     }*/
  }
}
