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
     void checkNode() {}
};

class Stmt : public Node
{

  public:
     Stmt() : Node() {}
     Stmt(yyltype loc) : Node(loc) {}
     virtual ~Stmt() {};
     void checkNode() =0;
};

class StmtBlock : public Stmt 
{
  protected:
    List<VarDecl*> *decls;
    List<Stmt*> *stmts;
 
  public:
    StmtBlock(List<VarDecl*> *variableDeclarations, List<Stmt*> *statements);
    int getLevel();
    List<VarDecl*> * getDeclList();
    void checkNode() {};
};

  
class ConditionalStmt : public Stmt
{
  protected:
    Expr *test;
    Stmt *body;
  
  public:
    ConditionalStmt(Expr *testExpr, Stmt *body);
    void checkNode() {}
};

class LoopStmt : public ConditionalStmt 
{
  public:
    LoopStmt(Expr *testExpr, Stmt *body)
            : ConditionalStmt(testExpr, body) {}
    void checkNode() {}
};

class ForStmt : public LoopStmt 
{
  protected:
    Expr *init, *step;
  
  public:
    ForStmt(Expr *init, Expr *test, Expr *step, Stmt *body);
    void checkNode() {}
};

class WhileStmt : public LoopStmt 
{
  public:
    WhileStmt(Expr *test, Stmt *body) : LoopStmt(test, body) {}
    void checkNode() {}
};

class IfStmt : public ConditionalStmt 
{
  protected:
    Stmt *elseBody;
  
  public:
    IfStmt(Expr *test, Stmt *thenBody, Stmt *elseBody);
    void checkNode() {}
};

class SwitchStmt : public Stmt
{
  protected:
    Expr *test;
    List<Stmt*> *caseBody;
    Stmt * defaultStmt;
 
  public:
    SwitchStmt(Expr *test, List<Stmt*> *caseBody, Stmt* defaultStmt);
    const char *GetPrintNameForNode() { return "SwitchStmt"; }
    void checkNode() {}
};

class CaseStmt : public Stmt
{
  protected:
    IntConstant *caseInt;
    List<Stmt*> *stmts;    

  public:
     CaseStmt(IntConstant * caseInt, List<Stmt*> *stmts);
     const char *GetPrintNameForNode() { return "Case"; }
     void checkNode() {}
};

class DefaultStmt : public Stmt
{
    protected:
      List<Stmt*> *stmts;

    public:
      DefaultStmt(List<Stmt*> *stmts);
      const char *GetPrintNameForNode() { return "Default"; }
      void checkNode() {}
};


class BreakStmt : public Stmt 
{
  public:
    BreakStmt(yyltype loc) : Stmt(loc) {}
    void checkNode() {}
};

class ReturnStmt : public Stmt  
{
  protected:
    Expr *expr;
  
  public:
    ReturnStmt(yyltype loc, Expr *expr);
    void checkNode() {}
};

class PrintStmt : public Stmt
{
  protected:
    List<Expr*> *args;
    
  public:
    PrintStmt(List<Expr*> *arguments);
    void checkNode() {}
};


#endif
