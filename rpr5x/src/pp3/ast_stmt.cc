/* File: ast_stmt.cc
 * -----------------
 * Implementation of statement node classes.
 */
#include "ast_stmt.h"
#include "ast_type.h"
#include "ast_decl.h"
#include "ast_expr.h"
#include "hashtable.h"

extern Hashtable<Decl *> *table;
void inOrderTraversal(Node * root);
 
Program::Program(List<Decl*> *d) {
    Assert(d != NULL);
    (decls=d)->SetParentAll(this);
    this->branches = new List<Node *>(); 
    for(int i = 0; i < d->NumElements(); i++)
      this->branches->Append(d->Nth(i)); 
}

void Program::Check() {
     
    
    inOrderTraversal(this);    
    
     
    /* pp3: here is where the semantic analyzer is kicked off.
     *      The general idea is perform a tree traversal of the
     *      entire program, examining all constructs for compliance
     *      with the semantic rules.  Each node can have its own way of
     *      checking itself, which makes for a great use of inheritance
     *      and polymorphism in the node classes.
     */
}

void Program::inOrderTraversal(Node *  root)
{
  if(root == NULL)
    return;
  
  List<Node *> *d = root->GetBranchList();  
  for(int i = 0; i < d->NumElements(); i++)
  {
    inOrderTraversal(d->Nth(i));
    root->checkNode();  
  }
  /*inOrderTraversal(root->GetLeftChild());  
  root->checkNode();     
  inOrderTraversal(root->GetRightChild()); 
  */

}

StmtBlock::StmtBlock(List<VarDecl*> *d, List<Stmt*> *s) {
    Assert(d != NULL && s != NULL);
    (decls=d)->SetParentAll(this);
    (stmts=s)->SetParentAll(this);
}

List<VarDecl*> * StmtBlock::getDeclList()
{
  return  decls;
}


ConditionalStmt::ConditionalStmt(Expr *t, Stmt *b) { 
    Assert(t != NULL && b != NULL);
    (test=t)->SetParent(this); 
    (body=b)->SetParent(this);
}

ForStmt::ForStmt(Expr *i, Expr *t, Expr *s, Stmt *b): LoopStmt(t, b) { 
    Assert(i != NULL && t != NULL && s != NULL && b != NULL);
    (init=i)->SetParent(this);
    (step=s)->SetParent(this);
}

IfStmt::IfStmt(Expr *t, Stmt *tb, Stmt *eb): ConditionalStmt(t, tb) { 
    Assert(t != NULL && tb != NULL); // else can be NULL
    elseBody = eb;
    if (elseBody) elseBody->SetParent(this);
}

SwitchStmt::SwitchStmt(Expr * t, List<Stmt*> *cb, Stmt * ds) : Stmt() {
   Assert(t != NULL && cb != NULL);//ds can be NULL
   test = t;
   caseBody = cb;
   test->SetParent(this);
   caseBody->SetParentAll(this);
   if(ds) (defaultStmt = ds)->SetParent(this);
}

CaseStmt::CaseStmt(IntConstant * intConstant, List<Stmt*> *statements): Stmt() {
     Assert(intConstant != NULL && statements != NULL);
     caseInt = intConstant;
     stmts = statements;
     caseInt->SetParent(this);
     stmts->SetParentAll(this); 
}

DefaultStmt::DefaultStmt(List<Stmt *> *statements): Stmt() {
    stmts = statements; //statements can be NULL 
    if(stmts) stmts->SetParentAll(this);

}

ReturnStmt::ReturnStmt(yyltype loc, Expr *e) : Stmt(loc) { 
    Assert(e != NULL);
    (expr=e)->SetParent(this);
}
  
PrintStmt::PrintStmt(List<Expr*> *a) {    
    Assert(a != NULL);
    (args=a)->SetParentAll(this);
}


