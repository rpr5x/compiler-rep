/* File: ast_stmt.h
 * ----------------
 * The Stmt class and its subclasses are used to represent
 * statements in the parse tree.  For each statment in the
 * language (for, if, return, etc.) there is a corresponding
 * node class for that construct. 
 *
 * pp3: You will need to extend the Stmt classes to implement
 * semantic analysis for rules pertaining to statements.
 */


#ifndef _H_ast_stmt
#define _H_ast_stmt

#include "list.h"
#include "ast.h"


class Decl;
class VarDecl;
class Expr;
class IntConstant;

extern int currentLevel;
  
class Program : public Node
{
  protected:
     List<Decl*> *decls;
     //List<Node *> *branches;  
  public:
     Program(List<Decl*> *declList);
     void Check();
     void inOrderTraversal(Node * root);
     void secondPass(Node * root);
     void checkNode() {}
     void checkNode2() {}
     void secondCheck();
};

class Stmt : public Node
{
  protected:
    int level;
    List<Node*> *branches;
  public:
     Stmt() : Node() {this->branches = new List<Node*>(); }
     Stmt(yyltype loc) : Node(loc) {this->branches = new List<Node*>();}
     List <Node*> *GetBranchList() {return this->branches;}
     virtual ~Stmt() {};
     virtual void checkNode() {};
     virtual void checkNode2() {};
};

class StmtBlock : public Stmt 
{
  protected:
    List<VarDecl*> *decls;
    List<Stmt*> *stmts;
    List<Node*> *branches; 
  public:
    StmtBlock(List<VarDecl*> *variableDeclarations, List<Stmt*> *statements);
    int getLevel();
    List<VarDecl*> * getDeclList();
    List<Stmt*> * getStmtList();
    void checkNode();
    void checkNode2();
    List<Node *> *GetBranchList() {return branches;}
};

  
class ConditionalStmt : public Stmt
{
  protected:
    Expr *test;
    Stmt *body;
    List<Node *> *branches;    

  public:
    ConditionalStmt(Expr *testExpr, Stmt *body);
    virtual void checkNode() {}
    virtual void checkNode2() {}
    List<Node *> *GetBranchList() {return branches;}
};

class LoopStmt : public ConditionalStmt 
{
  protected:
    List<Node*> *branches;

  public:
    LoopStmt(Expr *testExpr, Stmt *body)
            : ConditionalStmt(testExpr, body) {this->branches = new List<Node*>();}
    void checkNode() {}
    void checkNode2() {}
};

class ForStmt : public LoopStmt 
{
  protected:
    Expr *init, *step;
    List<Node *> *branches;
 
  public:
    ForStmt(Expr *init, Expr *test, Expr *step, Stmt *body);
    void checkNode(); 
    void checkNode2(); 
    List<Node *> *GetBranchList() {return branches;}
};

class WhileStmt : public LoopStmt 
{
  protected:
    List<Node *> *branches;

  public:
    WhileStmt(Expr *test, Stmt *body);
    void checkNode(); 
    void checkNode2(); 
    List<Node *> *GetBranchList() {return branches;}
};

class IfStmt : public ConditionalStmt 
{
  protected:
    Stmt *elseBody;
    List<Node *> *branches;  
 
  public:
    IfStmt(Expr *test, Stmt *thenBody, Stmt *elseBody);
    void checkNode(); 
    void checkNode2(); 
    List<Node *> *GetBranchList() {return branches;}
};

class SwitchStmt : public Stmt
{
  protected:
    Expr *test;
    List<Stmt*> *caseBody;
    Stmt * defaultStmt;
    List<Node *> *branches;
 
  public:
    SwitchStmt(Expr *test, List<Stmt*> *caseBody, Stmt* defaultStmt);
    const char *GetPrintNameForNode() { return "SwitchStmt"; }
    void checkNode(); 
    void checkNode2(); 
    List<Node *> *GetBranchList() {return branches;}
    List<Stmt *> *getCaseBody() {return caseBody;}
    Expr * getTestExpr() {return test;}
  
};

class CaseStmt : public Stmt
{
  protected:
    IntConstant *caseInt;
    List<Stmt*> *stmts;    
    List<Node*> *branches; 

  public:
     CaseStmt(IntConstant * caseInt, List<Stmt*> *stmts);
     const char *GetPrintNameForNode() { return "Case"; }
     void checkNode(); 
     void checkNode2(); 
     List<Node *> *GetBranchList() {return branches;}

};

class DefaultStmt : public Stmt
{
    protected:
      List<Stmt*> *stmts;
      List<Node*> *branches;
    public:
      DefaultStmt(List<Stmt*> *stmts);
      const char *GetPrintNameForNode() { return "Default"; }
      void checkNode(); 
      void checkNode2(); 
      List<Node *> *GetBranchList() {return branches;}
};


class BreakStmt : public Stmt 
{
  protected:
    List<Node*> *branches;

  public:
    BreakStmt(yyltype loc) : Stmt(loc) {this->branches = new List<Node *>();}
    void checkNode(); 
    void checkNode2();  
    List<Node *> *GetBranchList() {return branches;}
};

class ReturnStmt : public Stmt  
{
  protected:
    Expr *expr;
    List<Node *> *branches;
   
  public:
    ReturnStmt(yyltype loc, Expr *expr);
    void checkNode(); 
    void checkNode2(); 
    List<Node *> *GetBranchList() {return branches;}
};

class PrintStmt : public Stmt
{
  protected:
    List<Expr*> *args;
    List<Node*> *branches;
    
  public:
    PrintStmt(List<Expr*> *arguments);
    void checkNode();
    void checkNode2();
    List<Node*> *GetBranchList() {return branches;}
};


#endif
