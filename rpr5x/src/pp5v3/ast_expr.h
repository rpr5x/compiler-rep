/* File: ast_expr.h
 * ----------------
 * The Expr class and its subclasses are used to represent
 * expressions in the parse tree.  For each expression in the
 * language (add, call, New, etc.) there is a corresponding
 * node class for that construct. 
 *
 * pp5: You will need to extend the Expr classes to implement 
 * code generation for expressions.
 */


#ifndef _H_ast_expr
#define _H_ast_expr

#include "ast.h"
#include "ast_stmt.h"
#include "list.h"
#include "tac.h"

class NamedType; // for new
class Type; // for NewArray


class Expr : public Stmt 
{
  public:
    Expr(yyltype loc) : Stmt(loc) {}
    Expr() : Stmt() {}
    virtual Location *GetLocationNode() {return NULL;}
    virtual void Emit(){};
};

/* This node type is used for those places where an expression is optional.
 * We could use a NULL pointer, but then it adds a lot of checking for
 * NULL. By using a valid, but no-op, node, we save that trouble */
class EmptyExpr : public Expr
{
  public:
};

class IntConstant : public Expr 
{
  protected:
    int value;
    Location * intLoc;
  
  public:
    IntConstant(yyltype loc, int val);
    void Emit();
    int GetValue() {return value;}
    Location * GetLocationNode() {return intLoc;}
};

class DoubleConstant : public Expr 
{
  protected:
    double value;
    
  public:
    DoubleConstant(yyltype loc, double val);
    void Emit();
    double GetValue() {return value;}
};

class BoolConstant : public Expr 
{
  protected:
    bool value;
    Location * boolLoc;    
  public:
    BoolConstant(yyltype loc, bool val);
    void Emit();
    bool GetValue() {return value;}
    Location * GetLocationNode() {return boolLoc;}
};

class StringConstant : public Expr 
{ 
  protected:
    char *value;
    Location * strLoc;
    
  public:
    StringConstant(yyltype loc, const char *val);
    void Emit();
    char * GetValue() {return value;}
    Location * GetLocationNode() {return strLoc;}
};

class NullConstant: public Expr 
{
  public: 
    NullConstant(yyltype loc) : Expr(loc) {}
    void Emit() {}
};

class Operator : public Node 
{
  protected:
    char tokenString[4];
    
  public:
    Operator(yyltype loc, const char *tok);
    friend std::ostream& operator<<(std::ostream& out, Operator *o) { return out << o->tokenString; }
    const char *str() { return tokenString; }
 };
 
class CompoundExpr : public Expr
{
  protected:
    Operator *op;
    Expr *left, *right; // left will be NULL if unary
    
  public:
    CompoundExpr(Expr *lhs, Operator *op, Expr *rhs); // for binary
    CompoundExpr(Operator *op, Expr *rhs);             // for unary
    virtual void Emit() {};
    virtual Location * GetLocationNode() {return NULL;}
    Expr * GetLeft() {return left; }
    Expr * GetRight() {return right; }

};

class ArithmeticExpr : public CompoundExpr 
{
  protected:
    Location * arithLoc;

  public:
    ArithmeticExpr(Expr *lhs, Operator *op, Expr *rhs) : CompoundExpr(lhs,op,rhs) {}
    ArithmeticExpr(Operator *op, Expr *rhs) : CompoundExpr(op,rhs) {}
    void Emit();
    Location * GetLocationNode() {return arithLoc;}
};

class RelationalExpr : public CompoundExpr 
{
  protected:
    Location * relLoc;

  public:
    RelationalExpr(Expr *lhs, Operator *op, Expr *rhs) : CompoundExpr(lhs,op,rhs) {}
    void Emit();  
    Location * GetLocationNode() {return relLoc;}
};

class EqualityExpr : public CompoundExpr 
{
  protected:
    Location * eqLoc; 

  public:
    EqualityExpr(Expr *lhs, Operator *op, Expr *rhs) : CompoundExpr(lhs,op,rhs) {}
    const char *GetPrintNameForNode() { return "EqualityExpr"; }
    void Emit();
    Location * GetLocationNode() {return eqLoc;}
};

class LogicalExpr : public CompoundExpr 
{
  protected:
    Location * logLoc;

  public:
    LogicalExpr(Expr *lhs, Operator *op, Expr *rhs) : CompoundExpr(lhs,op,rhs) {}
    LogicalExpr(Operator *op, Expr *rhs) : CompoundExpr(op,rhs) {}
    const char *GetPrintNameForNode() { return "LogicalExpr"; }
    void Emit();
    Location * GetLocationNode() {return logLoc;}

};

class AssignExpr : public CompoundExpr 
{
  public:
    AssignExpr(Expr *lhs, Operator *op, Expr *rhs) : CompoundExpr(lhs,op,rhs) {}
    const char *GetPrintNameForNode() { return "AssignExpr"; }
    void Emit();
};

class LValue : public Expr 
{     
  public:
    LValue(yyltype loc) : Expr(loc) {}
    virtual Location * GetLocationNode() {return NULL;}
};

class This : public Expr 
{
  public:
    This(yyltype loc) : Expr(loc) {}
};

class ArrayAccess : public LValue 
{
  protected:
    Expr *base, *subscript;
    
  public:
    ArrayAccess(yyltype loc, Expr *base, Expr *subscript);
    Expr * GetBase(){return base;}
    Expr * GetSubscript(){return subscript;}
    void Emit();
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
    Location * fieldLoc;
  
  public:
    FieldAccess(Expr *base, Identifier *field); //ok to pass NULL base
    void Emit();
    Location *GetLocationNode() {return fieldLoc;}
    Identifier * GetId() {return field;}
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
    
  public:
    Call(yyltype loc, Expr *base, Identifier *field, List<Expr*> *args);
    void Emit();
    Expr * GetBase() {return base;}
    Identifier * GetId() {return field;}
    List<Expr*> *GetActuals() {return actuals;}
  
};

class NewExpr : public Expr
{
  protected:
    NamedType *cType;
    
  public:
    NewExpr(yyltype loc, NamedType *clsType);
    void Emit();
    Location * newLoc;
    Location * GetLocationNode() {return newLoc;}
};

class NewArrayExpr : public Expr
{
  protected:
    Expr *size;
    Type *elemType;
        
  public:
    NewArrayExpr(yyltype loc, Expr *sizeExpr, Type *elemType);
    void Emit();
    Location * arrayLoc;
    Location * GetLocationNode() {return arrayLoc;}
    Location * arrayLengthLoc;
    Location * GetArrayLengthLoc() {return arrayLengthLoc; }
 
};

class ReadIntegerExpr : public Expr
{
  public:
    ReadIntegerExpr(yyltype loc) : Expr(loc) {}
    void Emit();
    Location * readIntegerLoc;
    Location * GetLocationNode() {return readIntegerLoc;}
};

class ReadLineExpr : public Expr
{
  public:
    ReadLineExpr(yyltype loc) : Expr (loc) {}
    void Emit();
    Location * readLineLoc;
    Location * GetLocationNode() {return readLineLoc;}
};

    
#endif
