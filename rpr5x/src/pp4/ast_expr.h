/* File: ast_expr.h
 * ----------------
 * The Expr class and its subclasses are used to represent
 * expressions in the parse tree.  For each expression in the
 * language (add, call, New, etc.) there is a corresponding
 * node class for that construct. 
 *
 * pp3: You will need to extend the Expr classes to implement 
 * semantic analysis for rules pertaining to expressions.
 */


#ifndef _H_ast_expr
#define _H_ast_expr

#include "ast.h"
#include "ast_stmt.h"
#include "list.h"

class NamedType; // for new
class Type; // for NewArray


class Expr : public Stmt 
{
  protected:
    List<Node *> *branches;

  public:
    Expr(yyltype loc) : Stmt(loc) {this->branches = new List<Node *>();}
    Expr() : Stmt() {this->branches = new List<Node*>();}
    virtual void checkNode() =0;
    virtual void checkNode2() =0;
    List<Node*> * GetBranchList() {return branches;}
};

/* This node type is used for those places where an expression is optional.
 * We could use a NULL pointer, but then it adds a lot of checking for
 * NULL. By using a valid, but no-op, node, we save that trouble */
class EmptyExpr : public Expr
{
 protected:
    List<Node *> *branches;

  public:
    EmptyExpr() {this->branches = new List<Node*>();}
    void checkNode();
    void checkNode2();
    List<Node*> *GetBranches() {return branches;}
};

class IntConstant : public Expr 
{
  protected:
    int value;
    List<Node*> *branches;

  public:
    IntConstant(yyltype loc, int val);
    void checkNode(); 
    void checkNode2();
    List<Node*> *GetBranches() {return branches;}
};

class DoubleConstant : public Expr 
{
  protected:
    double value;
    List<Node*> *branches;

  public:
    DoubleConstant(yyltype loc, double val);
    void checkNode() {}
    void checkNode2() {}
};

class BoolConstant : public Expr 
{
  protected:
    bool value;
    List<Node*> *branches;

  public:
    BoolConstant(yyltype loc, bool val);
    void checkNode() {}
    void checkNode2() {}
};

class StringConstant : public Expr 
{ 
  protected:
    char *value;
    List<Node*> *branches;

  public:
    StringConstant(yyltype loc, const char *val);
    void checkNode() {}
    void checkNode2() {}
};

class NullConstant: public Expr 
{
  protected:
    List<Node*> *branches;

  public: 
    NullConstant(yyltype loc) : Expr(loc) {this->branches = new List<Node*>();}
    void checkNode() {}
    void checkNode2() {}
};

class Operator : public Node 
{
  protected:
    char tokenString[4];
    List<Node *> *branches;
 
  public:
    Operator(yyltype loc, const char *tok);
    friend std::ostream& operator<<(std::ostream& out, Operator *o) { return out << o->tokenString; }
    void checkNode();
    void checkNode2();
    List<Node*> * GetBranchList() {return branches;}
 };
 
class CompoundExpr : public Expr
{
  protected:
    Operator *op;
    Expr *left, *right; // left will be NULL if unary
    List<Node*> *branches;    

  public:
    CompoundExpr(Expr *lhs, Operator *op, Expr *rhs); // for binary
    CompoundExpr(Operator *op, Expr *rhs);             // for unary
    CompoundExpr(Expr *lhs, Operator *op); //For postfix expressions
    virtual void checkNode() {};
    virtual void checkNode2() {};
    List<Node*> * GetBranchList() {return branches;}
    Expr * returnLeftExpr() {return this->left;}
    Expr * returnRightExpr() {return this->right;}
};

class ArithmeticExpr : public CompoundExpr 
{
  protected:
    List<Node*> *branches;  
 
  public:
    ArithmeticExpr(Expr *lhs, Operator *op, Expr *rhs);
    ArithmeticExpr(Operator *op, Expr *rhs);
    void checkNode();
    void checkNode2();
    List<Node*> * GetBranchList() {return branches;}
};

class RelationalExpr : public CompoundExpr 
{ 
  protected:
    List<Node*> *branches;  

  public:
    RelationalExpr(Expr *lhs, Operator *op, Expr *rhs); 
    void checkNode();
    void checkNode2();
    List<Node*> * GetBranchList() {return branches;}
};

class EqualityExpr : public CompoundExpr 
{
  protected:
    List<Node*> *branches;  

  public:
    EqualityExpr(Expr *lhs, Operator *op, Expr *rhs);
    const char *GetPrintNameForNode() { return "EqualityExpr"; }
    void checkNode();
    void checkNode2();
    List<Node*> * GetBranchList() {return branches;}
};

class LogicalExpr : public CompoundExpr 
{
  protected:
    List<Node*> *branches;  

  public:
    LogicalExpr(Expr *lhs, Operator *op, Expr *rhs);
    LogicalExpr(Operator *op, Expr *rhs);
    const char *GetPrintNameForNode() { return "LogicalExpr"; }
    void checkNode();
    void checkNode2();
    List<Node*> * GetBranchList() {return branches;}
};

class AssignExpr : public CompoundExpr 
{
  protected:
    List<Node*> *branches;  

  public:
    AssignExpr(Expr *lhs, Operator *op, Expr *rhs);
    const char *GetPrintNameForNode() { return "AssignExpr"; }
    void checkNode();
    void checkNode2();
    List<Node*> * GetBranchList() {return branches;} 
};

class PostfixExpr : public CompoundExpr
{
  protected:
    List<Node*> *branches;  

  public:
    PostfixExpr(Expr *lhs, Operator *op);
    const char *GetPrintNameForNode() { return "PostfixExpr"; }
    void checkNode();
    void checkNode2();
    List<Node*> * GetBranchList() {return branches;}
};

class LValue : public Expr 
{
  protected:
    List<Node*> *branches;  

  public:
    LValue(yyltype loc) : Expr(loc) {this->branches = new List<Node *>();}
    virtual void checkNode() {printf("Hello\n");}; 
    virtual void checkNode2() {} ;
    List<Node*> * GetBranchList() {return branches;}
};

class This : public Expr 
{
  protected:
    List<Node*> *branches;  

  public:
    This(yyltype loc) : Expr(loc) {this->branches = new List<Node*>();}
    void checkNode();
    void checkNode2();
    List<Node*> * GetBranchList() {return branches;} 
};

class ArrayAccess : public LValue 
{
  protected:
    Expr *base, *subscript;
    List<Node*> *branches;
  
  public:
    ArrayAccess(yyltype loc, Expr *base, Expr *subscript);
    void checkNode();
    void checkNode2();
    List<Node*> * GetBranchList() {return branches;}
    Expr * getBase() {return base;}
};

/* Note that field access is used both for qualified names
 * base.field and just field without qualification. We don't
 * know for sure whether there is an implicit "this." in
 * front until later on, so we use one node type for either
 * and sort it out later. */
class FieldAccess : public LValue 
{
  protected:
    Expr *base;	// will be NULL if no explicit base
    Identifier *field;
    List<Node*> *branches; 
   
  public:
    FieldAccess(Expr *base, Identifier *field); //ok to pass NULL base
    void checkNode();
    void checkNode2();
    List<Node*> * GetBranchList() {return branches;}
    Identifier * getIdentifier() {return this->field;}
};

/* Like field access, call is used both for qualified base.field()
 * and unqualified field().  We won't figure out until later
 * whether we need implicit "this." so we use one node type for either
 * and sort it out later. */
class Call : public Expr 
{
  protected:
    Expr *base;	// will be NULL if no explicit base
    Identifier *field;
    List<Expr*> *actuals;
    List<Node*> *branches;
    
  public:
    Call(yyltype loc, Expr *base, Identifier *field, List<Expr*> *args);
    void checkNode();
    void checkNode2();
    List<Node*> * GetBranchList() {return branches;}
    Identifier * getField() {return this->field;}
    List<Expr *> * getActuals() {return this->actuals;}
};

class NewExpr : public Expr
{
  protected:
    NamedType *cType;
    List<Node*> *branches;
    
  public:
    NewExpr(yyltype loc, NamedType *clsType);
    void checkNode();
    void checkNode2();
    List<Node*> * GetBranchList() {return branches;}
};

class NewArrayExpr : public Expr
{
  protected:
    Expr *size;
    Type *elemType;
    List<Node *> *branches;

  public:
    NewArrayExpr(yyltype loc, Expr *sizeExpr, Type *elemType);
    void checkNode();
    void checkNode2();
    List<Node*> * GetBranchList() {return branches;}
};

class ReadIntegerExpr : public Expr
{
  protected:
    List<Node*> *branches;  

  public:
    ReadIntegerExpr(yyltype loc) : Expr(loc) {this->branches = new List<Node*>();}
    void checkNode();
    void checkNode2();
    List<Node*> * GetBranchList() {return branches;}
};

class ReadLineExpr : public Expr
{
  protected:
    List<Node*> *branches;  

  public:
    ReadLineExpr(yyltype loc) : Expr (loc) {this->branches = new List<Node*>();}
    void checkNode();
    void checkNode2();
    List<Node*> * GetBranchList() {return branches;}
};

    
#endif
