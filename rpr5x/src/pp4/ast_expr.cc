/* File: ast_expr.cc
 * -----------------
 * Implementation of expression node classes.
 */
#include "ast_expr.h"
#include "ast_type.h"
#include "ast_decl.h"
#include <string.h>
#include "errors.h"
#include "hashtable.h"

extern Hashtable <Node*> *table;
extern int currentLevel;

void EmptyExpr::checkNode()
{
 // printf("I am an empty expression\n");
}

void EmptyExpr::checkNode2()
{
 // printf("I am an empty expression\n");
}

IntConstant::IntConstant(yyltype loc, int val) : Expr(loc) {
    value = val;
    this->branches = new List<Node *>();
}

void IntConstant::checkNode()
{
  Node * parent = this->GetParent();

  //Increment level for switch statement scope if the test expression is an IntConstant
  SwitchStmt * s = dynamic_cast<SwitchStmt *>(parent);
  if(s)
  {
    Expr * test = s->getTestExpr();
    IntConstant * testInt = dynamic_cast<IntConstant *>(test);
    if(testInt)
    {
      currentLevel++;  //Increment level for switch statement scope
    }
  }

  
  //printf("The value is %d\n", value);   
}

void IntConstant::checkNode2()
{


  //printf("The value is %d\n", value);
}


DoubleConstant::DoubleConstant(yyltype loc, double val) : Expr(loc) {
    value = val;
    this->branches = new List<Node*>();

}

BoolConstant::BoolConstant(yyltype loc, bool val) : Expr(loc) {
    value = val;
    this->branches = new List<Node*>();

}

StringConstant::StringConstant(yyltype loc, const char *val) : Expr(loc) {
    Assert(val != NULL);
    value = strdup(val);
    this->branches = new List<Node*>();
   
}

Operator::Operator(yyltype loc, const char *tok) : Node(loc) {
    Assert(tok != NULL);
    strncpy(tokenString, tok, sizeof(tokenString));
    this->branches = new List<Node*> ();
}

void Operator::checkNode()
{
  //printf("I am an operator\n");

}

void Operator::checkNode2()
{
 // printf("I am an operator\n");

}
CompoundExpr::CompoundExpr(Expr *l, Operator *o, Expr *r) 
  : Expr(Join(l->GetLocation(), r->GetLocation())) {
    Assert(l != NULL && o != NULL && r != NULL);
    (op=o)->SetParent(this);
    (left=l)->SetParent(this); 
    (right=r)->SetParent(this);
    this->branches = new List<Node *>();
    this->branches->Append(left);
    this->branches->Append(op);
    this->branches->Append(right);

}

CompoundExpr::CompoundExpr(Operator *o, Expr *r) 
  : Expr(Join(o->GetLocation(), r->GetLocation())) {
    Assert(o != NULL && r != NULL);
    left = NULL; 
    (op=o)->SetParent(this);
    (right=r)->SetParent(this);
    this->branches = new List<Node *>();
    this->branches->Append(o);
    this->branches->Append(right);


}
   
CompoundExpr::CompoundExpr(Expr *l, Operator *o)
  : Expr(Join(l->GetLocation(), o->GetLocation())) {
    Assert(l != NULL && o != NULL);
    right = NULL;
    (op=o)->SetParent(this);
    (left=l)->SetParent(this);
    this->branches = new List<Node *>();
    this->branches->Append(left);
    this->branches->Append(o);


}   

ArithmeticExpr::ArithmeticExpr(Expr *lhs, Operator *op, Expr *rhs):CompoundExpr(lhs,op,rhs)
{
   this->branches = new List<Node*>();
   this->branches->Append(lhs);
   this->branches->Append(op);
   this->branches->Append(rhs);
}

ArithmeticExpr::ArithmeticExpr(Operator *op, Expr *rhs):CompoundExpr(op,rhs)
{
   this->branches = new List<Node*>();
   this->branches->Append(op);
   this->branches->Append(rhs);

}
void ArithmeticExpr::checkNode()
{

}

void ArithmeticExpr::checkNode2()
{
  //Check to make sure both operand types are valid
  Expr * l = this->left;
  Expr * r = this->right;

  //Binary arithmetic expression
  if(l)
  {
    IntConstant * i1 = dynamic_cast<IntConstant *>(l);
    IntConstant * i2 = dynamic_cast<IntConstant *>(r);
    DoubleConstant * d1 = dynamic_cast<DoubleConstant *>(l);
    DoubleConstant * d2 = dynamic_cast<DoubleConstant *>(r);
    StringConstant * s1 = dynamic_cast<StringConstant *>(l);
    StringConstant * s2 = dynamic_cast<StringConstant *>(r);
    FieldAccess * v1 = dynamic_cast<FieldAccess *>(l);
    FieldAccess * v2 = dynamic_cast<FieldAccess *>(r);
    
    //Check for improper constants
    if(i1 && d2)
         ReportError::IncompatibleOperands(this->op, Type::intType, Type::doubleType);
    else if(i1 && s2)
         ReportError::IncompatibleOperands(this->op, Type::intType, Type::stringType);
    else if(d1 && i2)
         ReportError::IncompatibleOperands(this->op, Type::doubleType, Type::intType);
    else if(d1 && s2)
          ReportError::IncompatibleOperands(this->op, Type::doubleType, Type::stringType);
    else if(s1 && i2)
          ReportError::IncompatibleOperands(this->op, Type::stringType, Type::intType);
    else if(s1 && d2)
          ReportError::IncompatibleOperands(this->op, Type::stringType, Type::doubleType);
    else if(v1)
    {
           Node * n = table->Lookup(v1->getIdentifier()->getName());
           VarDecl * firstVar = dynamic_cast<VarDecl *>(n);
           if(firstVar)
           { 
             char * t1 = firstVar->getType()->getTypeName();
             //Check when the first operand is a variable of some type and the second operand is a constant
             if(!((strcmp(t1, "int") == 0 && i1) || (strcmp(t1, "double") == 0 && d1)))
             {
                if(strcmp(t1, "int") == 0 && d2)
                  ReportError::IncompatibleOperands(this->op, Type::intType, Type::doubleType);
                else if(strcmp(t1, "int") == 0 && s2)
                  ReportError::IncompatibleOperands(this->op, Type::intType, Type::stringType);
                else if(strcmp(t1, "double") == 0 && i2)
                  ReportError::IncompatibleOperands(this->op, Type::doubleType, Type::intType);
                else if(strcmp(t1, "double") == 0 && s2)
                  ReportError::IncompatibleOperands(this->op, Type::doubleType, Type::stringType);       
                else if(strcmp(t1, "string") == 0 && i2)
                  ReportError::IncompatibleOperands(this->op, Type::stringType, Type::intType);
                else if( strcmp(t1, "string") == 0 && d2)
                  ReportError::IncompatibleOperands(this->op, Type::stringType, Type::doubleType);

             }
           }
     }
    else if(v2)
    {
           Node * n = table->Lookup(v2->getIdentifier()->getName());
           VarDecl * secondVar = dynamic_cast<VarDecl *>(n);
           if(secondVar)
           { 
             char * t2 = secondVar->getType()->getTypeName();
             //Check when the first operand is a constant and the second operand is a constant variable of some type.
             if(!((strcmp(t2, "int") == 0 && i2) || (strcmp(t2, "double") == 0 && d2)))
             {
                if(strcmp(t2, "double") == 0 && i1)
                  ReportError::IncompatibleOperands(this->op, Type::intType, Type::doubleType);
                else if(strcmp(t2, "string") == 0 && i1)
                  ReportError::IncompatibleOperands(this->op, Type::intType, Type::stringType);
                else if(strcmp(t2, "int") == 0 && d1)
                  ReportError::IncompatibleOperands(this->op, Type::doubleType, Type::intType);
                else if(strcmp(t2, "string") == 0 && d1)
                  ReportError::IncompatibleOperands(this->op, Type::doubleType, Type::stringType);       
                else if(strcmp(t2, "int") == 0 && s1)
                  ReportError::IncompatibleOperands(this->op, Type::stringType, Type::intType);
                else if( strcmp(t2, "double") == 0 && s1)
                  ReportError::IncompatibleOperands(this->op, Type::stringType, Type::doubleType);

             }
           }
 
    }
    if(v1 && v2)
    {
      Node *n1 = table->Lookup(v1->getIdentifier()->getName());
      Node *n2 = table->Lookup(v2->getIdentifier()->getName());
      VarDecl * var1 = dynamic_cast<VarDecl *>(n1);
      VarDecl * var2 = dynamic_cast<VarDecl *>(n2);
      if(var1 && var2)
      {
        char * t1 = var1->getType()->getTypeName();
        char * t2 = var2->getType()->getTypeName();

        //Check when both operands are variables of some type.  Both are not ints or doubles in this case.
        if(!((strcmp(t1, "int") == 0 && strcmp(t2, "int") == 0) || (strcmp(t1, "double") == 0 && strcmp(t2, "double") == 0)))
        {
           ReportError::IncompatibleOperands(this->op, var1->getType(), var2->getType());
         
        }
     }

    } 
  }
  else  //Unary arithmetic expression
  {
     IntConstant * i1 = dynamic_cast<IntConstant *>(r);
     DoubleConstant * d1 = dynamic_cast<DoubleConstant *>(r);
     StringConstant * s1 = dynamic_cast<StringConstant *>(r);  
     
     FieldAccess * v2 = dynamic_cast<FieldAccess *>(r);

     //Check when the expression is not an IntConstant, DoubleConstant or a variable
     if(!i1 && !d1 && !v2)
     {
        if(s1)
           ReportError::IncompatibleOperand(this->op, Type::stringType);

     }

     //If the variable type is not int or double, report an error.
     if(v2)
     {
       Node * n = table->Lookup(v2->getIdentifier()->getName());   
       VarDecl * var2 = dynamic_cast<VarDecl *>(n);
       if(var2)
       {
         char * t2 = var2->getType()->getTypeName();
         if(t2 == NULL || !(strcmp(t2, "int") == 0 || strcmp(t2, "double") == 0))
         {
           ReportError::IncompatibleOperand(this->op, var2->getType());
         }
       }

     }
  }



} 

RelationalExpr::RelationalExpr(Expr *lhs, Operator *op, Expr *rhs):CompoundExpr(lhs,op,rhs)
{
  this->branches = new List<Node*>();
  this->branches->Append(lhs);
  this->branches->Append(op);
  this->branches->Append(rhs);

}
void RelationalExpr::checkNode()
{
  
}

void RelationalExpr::checkNode2()
{
  //Check to make sure both operand types are valid
   
  Expr * l = this->left;
  Expr * r = this->right;

  //Binary relational expression
  if(l && r)
  {
    IntConstant * i1 = dynamic_cast<IntConstant *>(l);
    IntConstant * i2 = dynamic_cast<IntConstant *>(r);
    DoubleConstant * d1 = dynamic_cast<DoubleConstant *>(l);
    DoubleConstant * d2 = dynamic_cast<DoubleConstant *>(r);
    StringConstant * s1 = dynamic_cast<StringConstant *>(l);
    StringConstant * s2 = dynamic_cast<StringConstant *>(r);
    FieldAccess * v1 = dynamic_cast<FieldAccess *>(l);
    FieldAccess * v2 = dynamic_cast<FieldAccess *>(r);
    
    //Check for improper constants
    if(i1 && d2)
         ReportError::IncompatibleOperands(this->op, Type::intType, Type::doubleType);
    else if(i1 && s2)
         ReportError::IncompatibleOperands(this->op, Type::intType, Type::stringType);
    else if(d1 && i2)
         ReportError::IncompatibleOperands(this->op, Type::doubleType, Type::intType);
    else if(d1 && s2)
          ReportError::IncompatibleOperands(this->op, Type::doubleType, Type::stringType);
    else if(s1 && i2)
          ReportError::IncompatibleOperands(this->op, Type::stringType, Type::intType);
    else if(s1 && d2)
          ReportError::IncompatibleOperands(this->op, Type::stringType, Type::doubleType);
    else if(v1)
    {
           Node * n = table->Lookup(v1->getIdentifier()->getName());
           VarDecl * firstVar = dynamic_cast<VarDecl *>(n);
           if(firstVar)
           { 
             char * t1 = firstVar->getType()->getTypeName();
             //Check when the first operand is a variable of some type and the second operand is a constant
             if(!((strcmp(t1, "int") == 0 && i1) || (strcmp(t1, "double") == 0 && d1)))
             {
                if(strcmp(t1, "int") == 0 && d2)
                  ReportError::IncompatibleOperands(this->op, Type::intType, Type::doubleType);
                else if(strcmp(t1, "int") == 0 && s2)
                  ReportError::IncompatibleOperands(this->op, Type::intType, Type::stringType);
                else if(strcmp(t1, "double") == 0 && i2)
                  ReportError::IncompatibleOperands(this->op, Type::doubleType, Type::intType);
                else if(strcmp(t1, "double") == 0 && s2)
                  ReportError::IncompatibleOperands(this->op, Type::doubleType, Type::stringType);       
                else if(strcmp(t1, "string") == 0 && i2)
                  ReportError::IncompatibleOperands(this->op, Type::stringType, Type::intType);
                else if( strcmp(t1, "string") == 0 && d2)
                  ReportError::IncompatibleOperands(this->op, Type::stringType, Type::doubleType);
                
             }
           }
     }
    else if(v2)
    {
           Node * n = table->Lookup(v2->getIdentifier()->getName());
           VarDecl * secondVar = dynamic_cast<VarDecl *>(n);
           if(secondVar)
           { 
             char * t2 = secondVar->getType()->getTypeName();
             //Check when the first operand is a constant and the second operand is a variable of some type.
             if(!((strcmp(t2, "int") == 0 && i2) || (strcmp(t2, "double") == 0 && d2)))
             {
                if(strcmp(t2, "double") == 0 && i1)
                  ReportError::IncompatibleOperands(this->op, Type::intType, Type::doubleType);
                else if(strcmp(t2, "string") == 0 && i1)
                  ReportError::IncompatibleOperands(this->op, Type::intType, Type::stringType);
                else if(strcmp(t2, "int") == 0 && d1)
                  ReportError::IncompatibleOperands(this->op, Type::doubleType, Type::intType);
                else if(strcmp(t2, "string") == 0 && d1)
                  ReportError::IncompatibleOperands(this->op, Type::doubleType, Type::stringType);       
                else if(strcmp(t2, "int") == 0 && s1)
                  ReportError::IncompatibleOperands(this->op, Type::stringType, Type::intType);
                else if( strcmp(t2, "double") == 0 && s1)
                  ReportError::IncompatibleOperands(this->op, Type::stringType, Type::doubleType);

             }
           }
 
    }
    if(v1 && v2)
    {
      Node *n1 = table->Lookup(v1->getIdentifier()->getName());
      Node *n2 = table->Lookup(v2->getIdentifier()->getName());
      VarDecl * var1 = dynamic_cast<VarDecl *>(n1);
      VarDecl * var2 = dynamic_cast<VarDecl *>(n2);
      if(var1 && var2)
      {
        char * t1 = var1->getType()->getTypeName();
        char * t2 = var2->getType()->getTypeName();

        //Check when both operands are variables of some type.  Both are not ints or doubles in this case.
        if(!((strcmp(t1, "int") == 0 && strcmp(t2, "int") == 0) || (strcmp(t1, "double") == 0 && strcmp(t2, "double") == 0)))
        {
           ReportError::IncompatibleOperands(this->op, var1->getType(), var2->getType());
         
        }
     }

    } 
  }  
}



EqualityExpr::EqualityExpr(Expr *lhs, Operator *op, Expr *rhs):CompoundExpr(lhs, op, rhs)
{
   this->branches = new List<Node*>();
   this->branches->Append(lhs);
   this->branches->Append(op);
   this->branches->Append(rhs);

}

void EqualityExpr::checkNode()
{

}

void EqualityExpr::checkNode2()
{
  //Check to make sure both operand types are valid 
  Expr * l = this->left;
  Expr * r = this->right;

  //Binary equality expression
  if(l && r)
  {
    IntConstant * i1 = dynamic_cast<IntConstant *>(l);
    IntConstant * i2 = dynamic_cast<IntConstant *>(r);
    DoubleConstant * d1 = dynamic_cast<DoubleConstant *>(l);
    DoubleConstant * d2 = dynamic_cast<DoubleConstant *>(r);
    StringConstant * s1 = dynamic_cast<StringConstant *>(l);
    StringConstant * s2 = dynamic_cast<StringConstant *>(r);
    FieldAccess * v1 = dynamic_cast<FieldAccess *>(l);
    FieldAccess * v2 = dynamic_cast<FieldAccess *>(r);
    BoolConstant * b1 = dynamic_cast<BoolConstant *>(l);
    BoolConstant * b2 = dynamic_cast<BoolConstant *>(r);   
 

    //Check for improper constants
    if(i1 && d2)
         ReportError::IncompatibleOperands(this->op, Type::intType, Type::doubleType);
    else if(i1 && s2)
         ReportError::IncompatibleOperands(this->op, Type::intType, Type::stringType);
    else if(d1 && i2)
         ReportError::IncompatibleOperands(this->op, Type::doubleType, Type::intType);
    else if(d1 && s2)
          ReportError::IncompatibleOperands(this->op, Type::doubleType, Type::stringType);
    else if(s1 && i2)
          ReportError::IncompatibleOperands(this->op, Type::stringType, Type::intType);
    else if(s1 && d2)
          ReportError::IncompatibleOperands(this->op, Type::stringType, Type::doubleType);
    else if(b1 && i2)
          ReportError::IncompatibleOperands(this->op, Type::boolType, Type::intType);
    else if(b1 && d2)
          ReportError::IncompatibleOperands(this->op, Type::boolType, Type::doubleType);
    else if(b1 && s2)
          ReportError::IncompatibleOperands(this->op, Type::boolType, Type::stringType);
    else if(i1 && b2)
          ReportError::IncompatibleOperands(this->op, Type::intType, Type::boolType);
    else if(d1 && b2)
          ReportError::IncompatibleOperands(this->op, Type::doubleType, Type::boolType);
    else if(s1 && b2)
          ReportError::IncompatibleOperands(this->op, Type::stringType, Type::boolType);
 
    else if(v1)
    {
           Node * n = table->Lookup(v1->getIdentifier()->getName());
           VarDecl * firstVar = dynamic_cast<VarDecl *>(n);
           if(firstVar)
           { 
             char * t1 = firstVar->getType()->getTypeName();
             //Check when the first operand is a variable of some type and the second operand is a constant
             
             
                if(strcmp(t1, "int") == 0 && d2)
                  ReportError::IncompatibleOperands(this->op, Type::intType, Type::doubleType);
                else if(strcmp(t1, "int") == 0 && s2)
                  ReportError::IncompatibleOperands(this->op, Type::intType, Type::stringType);
                else if(strcmp(t1, "double") == 0 && i2)
                  ReportError::IncompatibleOperands(this->op, Type::doubleType, Type::intType);
                else if(strcmp(t1, "double") == 0 && s2)
                  ReportError::IncompatibleOperands(this->op, Type::doubleType, Type::stringType);       
                else if(strcmp(t1, "string") == 0 && i2)
                  ReportError::IncompatibleOperands(this->op, Type::stringType, Type::intType);
                else if( strcmp(t1, "string") == 0 && d2)
                  ReportError::IncompatibleOperands(this->op, Type::stringType, Type::doubleType);
                else if(strcmp(t1, "int") == 0 && b2)
                  ReportError::IncompatibleOperands(this->op, Type::intType, Type::boolType);
                else if(strcmp(t1, "double") == 0 && b2)
                  ReportError::IncompatibleOperands(this->op, Type::doubleType, Type::boolType); 
                else if(strcmp(t1, "string") == 0 && b2)
                  ReportError::IncompatibleOperands(this->op, Type::stringType, Type::boolType);   
             }
           
     }
    else if(v2)
    {
           Node * n = table->Lookup(v2->getIdentifier()->getName());
           VarDecl * secondVar = dynamic_cast<VarDecl *>(n);
           if(secondVar)
           { 
             char * t2 = secondVar->getType()->getTypeName();
             //Check when the first operand is a constant and the second operand is a variable of some type.
              if(strcmp(t2, "double") == 0 && i1)
                ReportError::IncompatibleOperands(this->op, Type::intType, Type::doubleType);
              else if(strcmp(t2, "string") == 0 && i1)
                ReportError::IncompatibleOperands(this->op, Type::intType, Type::stringType);
              else if(strcmp(t2, "int") == 0 && d1)
                ReportError::IncompatibleOperands(this->op, Type::doubleType, Type::intType);
              else if(strcmp(t2, "string") == 0 && d1)
                ReportError::IncompatibleOperands(this->op, Type::doubleType, Type::stringType);       
              else if(strcmp(t2, "int") == 0 && s1)
                ReportError::IncompatibleOperands(this->op, Type::stringType, Type::intType);
              else if( strcmp(t2, "double") == 0 && s1)
                ReportError::IncompatibleOperands(this->op, Type::stringType, Type::doubleType);
              else if(strcmp(t2, "int") == 0 && b1)
                ReportError::IncompatibleOperands(this->op, Type::boolType, Type::intType);
              else if(strcmp(t2, "double") == 0 && b1)
                ReportError::IncompatibleOperands(this->op, Type::boolType, Type::doubleType); 
              else if(strcmp(t2, "string") == 0 && b1)
                ReportError::IncompatibleOperands(this->op, Type::boolType, Type::stringType);   
              else if(strcmp(t2, "bool") == 0 && i1)
                ReportError::IncompatibleOperands(this->op, Type::intType, Type::boolType);
              else if(strcmp(t2, "bool") == 0 && d1)
                ReportError::IncompatibleOperands(this->op, Type::doubleType, Type::boolType);
              else if(strcmp(t2, "bool") == 0 && s1)
                ReportError::IncompatibleOperands(this->op, Type::stringType, Type::boolType);
                 
           }
           
 
    }
    if(v1 && v2)
    {
      Node *n1 = table->Lookup(v1->getIdentifier()->getName());
      Node *n2 = table->Lookup(v2->getIdentifier()->getName());
      VarDecl * var1 = dynamic_cast<VarDecl *>(n1);
      VarDecl * var2 = dynamic_cast<VarDecl *>(n2);
      if(var1 && var2)
      {
        Type * type1 = var1->getType();
        Type * type2 = var2->getType(); 
        char * t1 = var1->getType()->getTypeName();
        char * t2 = var2->getType()->getTypeName();
        NamedType * nt1 = dynamic_cast<NamedType *>(type1);
        NamedType * nt2 = dynamic_cast<NamedType *>(type2);
        //Report an error when both operands are not of the same type  
        if(t1 && t2 && strcmp(t1, t2) != 0)
        {
           ReportError::IncompatibleOperands(this->op, var1->getType(), var2->getType());
         
        }
        else if(nt1 && nt2 && strcmp(nt1->getId()->getName(), nt2->getId()->getName()) != 0)
        {
               //  A derived type of a class or interface is equal to that class/interface type
               Node * base = table->Lookup(nt1->getId()->getName());
               ClassDecl * baseClass = dynamic_cast<ClassDecl *>(base);
               InterfaceDecl * baseInterface = dynamic_cast<InterfaceDecl *>(base);
               Node * derived = table->Lookup(nt2->getId()->getName());
               ClassDecl * derivedClass = dynamic_cast<ClassDecl *>(derived);

               //A named type variable of a subclass is equal to that of the base class.
               if(baseClass && derivedClass && derivedClass->getExtends())
               {
                 if(!(strcmp(nt2->getId()->getName(), derivedClass->getIdentifier()->getName()) == 0 && strcmp(nt1->getId()->getName(), derivedClass->getExtends()->getId()->getName()) == 0))
                 {
                   ReportError::IncompatibleOperands(this->op, nt1, nt2); 
                   return;
                 } 
               
               }//A named type variable of a class that implementes an interface is equal to a named type variable of that interface
               else if(baseInterface && derivedClass && derivedClass->getImplements()->NumElements() > 0)
               {
                 bool isImplemented = false;
                 List<NamedType *> *implements = derivedClass->getImplements();
                 for(int i = 0; i < implements->NumElements(); i++)
                 {
                   if(strcmp(nt1->getId()->getName(), implements->Nth(i)->getId()->getName()) == 0)
                   {
                     isImplemented = true;
                     if(!(strcmp(nt2->getId()->getName(), derivedClass->getIdentifier()->getName()) == 0))
                     {
                      ReportError::IncompatibleOperands(this->op, nt1, nt2);
                      return;
                     }
                   }
                 }
                 if(!isImplemented)
                 {
                    isImplemented = false;
                    ReportError::IncompatibleOperands(this->op, nt1, nt2);
                    return; 
                
                 }
               }
               else //All other cases of two different named types are errors
               {
                 ReportError::IncompatibleOperands(this->op, nt1, nt2);
               }
             }
           }   
         }
   }
          
}

LogicalExpr::LogicalExpr(Expr *lhs, Operator *op, Expr *rhs):CompoundExpr(lhs, op, rhs)
{
   this->branches = new List<Node*>();
   this->branches->Append(lhs);
   this->branches->Append(op);
   this->branches->Append(rhs);

}

LogicalExpr::LogicalExpr(Operator *op, Expr *rhs): CompoundExpr(op, rhs)
{
   this->branches = new List<Node*>();
   this->branches->Append(op);
   this->branches->Append(rhs);
}

void LogicalExpr::checkNode()
{
  
}

void LogicalExpr::checkNode2()
{
  //Check to make sure all operand types are valid
  Expr * l = this->left;
  Expr * r = this->right;
  
  
  if(l) //Binary logical operators
  {
    //Check to make sure both operand types are valid
    IntConstant * i1 = dynamic_cast<IntConstant *>(l);
    IntConstant * i2 = dynamic_cast<IntConstant *>(r);
    DoubleConstant * d1 = dynamic_cast<DoubleConstant *>(l);
    DoubleConstant * d2 = dynamic_cast<DoubleConstant *>(r);
    StringConstant * s1 = dynamic_cast<StringConstant *>(l);
    StringConstant * s2 = dynamic_cast<StringConstant *>(r);
    FieldAccess * v1 = dynamic_cast<FieldAccess *>(l);
    FieldAccess * v2 = dynamic_cast<FieldAccess *>(r);
    BoolConstant *b1 = dynamic_cast<BoolConstant *>(l);
    BoolConstant *b2 = dynamic_cast<BoolConstant *>(r);    

    //Check for improper constants
    if(i1 && d2)
         ReportError::IncompatibleOperands(this->op, Type::intType, Type::doubleType);
    else if(i1 && s2)
         ReportError::IncompatibleOperands(this->op, Type::intType, Type::stringType);
    else if(d1 && i2)
         ReportError::IncompatibleOperands(this->op, Type::doubleType, Type::intType);
    else if(d1 && s2)
          ReportError::IncompatibleOperands(this->op, Type::doubleType, Type::stringType);
    else if(s1 && i2)
          ReportError::IncompatibleOperands(this->op, Type::stringType, Type::intType);
    else if(s1 && d2)
          ReportError::IncompatibleOperands(this->op, Type::stringType, Type::doubleType);
    else if(b1 && i2)
          ReportError::IncompatibleOperands(this->op, Type::boolType, Type::intType);
    else if(b1 && d2)
          ReportError::IncompatibleOperands(this->op, Type::boolType, Type::doubleType);
    else if(b1 && s2)
          ReportError::IncompatibleOperands(this->op, Type::boolType, Type::stringType);
    else if(i1 && b2)
          ReportError::IncompatibleOperands(this->op, Type::intType, Type::boolType);
    else if(d1 && b2)
          ReportError::IncompatibleOperands(this->op, Type::doubleType, Type::boolType);
    else if(s1 && b2)
          ReportError::IncompatibleOperands(this->op, Type::stringType, Type::boolType);
   
    else if(v1)
    {
           Node * n = table->Lookup(v1->getIdentifier()->getName());
           VarDecl * firstVar = dynamic_cast<VarDecl *>(n);
           if(firstVar)
           { 
             char * t1 = firstVar->getType()->getTypeName();
             //Check when the first operand is a variable of some type and the second operand is a constant
             
             
                if(strcmp(t1, "int") == 0 && d2)
                  ReportError::IncompatibleOperands(this->op, Type::intType, Type::doubleType);
                else if(strcmp(t1, "int") == 0 && s2)
                  ReportError::IncompatibleOperands(this->op, Type::intType, Type::stringType);
                else if(strcmp(t1, "double") == 0 && i2)
                  ReportError::IncompatibleOperands(this->op, Type::doubleType, Type::intType);
                else if(strcmp(t1, "double") == 0 && s2)
                  ReportError::IncompatibleOperands(this->op, Type::doubleType, Type::stringType);       
                else if(strcmp(t1, "string") == 0 && i2)
                  ReportError::IncompatibleOperands(this->op, Type::stringType, Type::intType);
                else if( strcmp(t1, "string") == 0 && d2)
                  ReportError::IncompatibleOperands(this->op, Type::stringType, Type::doubleType);
                else if(strcmp(t1, "int") == 0 && b2)
                  ReportError::IncompatibleOperands(this->op, Type::intType, Type::boolType);
                else if(strcmp(t1, "double") == 0 && b2)
                  ReportError::IncompatibleOperands(this->op, Type::doubleType, Type::boolType); 
                else if(strcmp(t1, "string") == 0 && b2)
                  ReportError::IncompatibleOperands(this->op, Type::stringType, Type::boolType);   
                else if(strcmp(t1, "bool") == 0 && i2)
                  ReportError::IncompatibleOperands(this->op, Type::boolType, Type::intType);
                else if(strcmp(t1, "bool") == 0 && d2)
                  ReportError::IncompatibleOperands(this->op, Type::boolType, Type::doubleType);
                else if(strcmp(t1, "bool") == 0 && s2)
                  ReportError::IncompatibleOperands(this->op, Type::boolType, Type::stringType);

             
          }
  }   
  else if(v2)
    {
      Node * n = table->Lookup(v2->getIdentifier()->getName());
      VarDecl * secondVar = dynamic_cast<VarDecl *>(n);
      if(secondVar)
       { 
         char * t2 = secondVar->getType()->getTypeName();
         //Check when the first operand is a constant and the second operand is a constant variable of some type.
        
           if(strcmp(t2, "double") == 0 && i1)
              ReportError::IncompatibleOperands(this->op, Type::intType, Type::doubleType);
           else if(strcmp(t2, "string") == 0 && i1)
              ReportError::IncompatibleOperands(this->op, Type::intType, Type::stringType);
           else if(strcmp(t2, "int") == 0 && d1)
              ReportError::IncompatibleOperands(this->op, Type::doubleType, Type::intType);
           else if(strcmp(t2, "string") == 0 && d1)
              ReportError::IncompatibleOperands(this->op, Type::doubleType, Type::stringType);       
           else if(strcmp(t2, "int") == 0 && s1)
              ReportError::IncompatibleOperands(this->op, Type::stringType, Type::intType);
           else if( strcmp(t2, "double") == 0 && s1)
              ReportError::IncompatibleOperands(this->op, Type::stringType, Type::doubleType);
           else if(strcmp(t2, "int") == 0 && b1)
              ReportError::IncompatibleOperands(this->op, Type::boolType, Type::intType);
           else if(strcmp(t2, "double") == 0 && b1)
              ReportError::IncompatibleOperands(this->op, Type::boolType, Type::doubleType); 
           else if(strcmp(t2, "string") == 0 && b1)
              ReportError::IncompatibleOperands(this->op, Type::boolType, Type::stringType);   
           else if(strcmp(t2, "bool") == 0 && i1)
              ReportError::IncompatibleOperands(this->op, Type::intType, Type::boolType); 
           else if(strcmp(t2, "bool") == 0 && d1)
              ReportError::IncompatibleOperands(this->op, Type::doubleType, Type::boolType);
           else if(strcmp(t2, "bool") == 0 && s1)
              ReportError::IncompatibleOperands(this->op, Type::stringType, Type::boolType); 
       }
 
    }
    if(v1 && v2)
    {
      Node *n1 = table->Lookup(v1->getIdentifier()->getName());
      Node *n2 = table->Lookup(v2->getIdentifier()->getName());
      VarDecl * var1 = dynamic_cast<VarDecl *>(n1);
      VarDecl * var2 = dynamic_cast<VarDecl *>(n2);
      if(var1 && var2)
      {
        char * t1 = var1->getType()->getTypeName();
        char * t2 = var2->getType()->getTypeName();

        //Check when both operands are variables of some type.  Both are not ints or doubles in this case.
        if(!((strcmp(t1, "bool") == 0 && strcmp(t2, "bool") == 0)))
        {
           ReportError::IncompatibleOperands(this->op, var1->getType(), var2->getType());
         
        }
      }

    } 
  }
  else //Unary logical operators (Can only be applied to boolean constants and boolean variables)
  {
     IntConstant * i1 = dynamic_cast<IntConstant *>(r);
     DoubleConstant * d1 = dynamic_cast<DoubleConstant *>(r);
     StringConstant * s1 = dynamic_cast<StringConstant *>(r);  
     BoolConstant * b1 = dynamic_cast<BoolConstant *>(r);
     FieldAccess * v2 = dynamic_cast<FieldAccess *>(r);

     if(!b1 & !v2)
     {
       if(i1)
         ReportError::IncompatibleOperand(this->op, Type::intType);
       else if(d1)
         ReportError::IncompatibleOperand(this->op, Type::doubleType);
       else if(s1)
         ReportError::IncompatibleOperand(this->op, Type::stringType); 

     }

     if(v2)
     {
       Node * n = table->Lookup(v2->getIdentifier()->getName());   
       VarDecl * var2 = dynamic_cast<VarDecl *>(n);
       if(var2)
       {
         
         char * t2 = var2->getType()->getTypeName();
         if(t2 == NULL || strcmp(t2, "bool") != 0)
         {
           ReportError::IncompatibleOperand(this->op, var2->getType());
         }
         
       } 
     }
  }
}

AssignExpr::AssignExpr(Expr *lhs, Operator *op, Expr *rhs): CompoundExpr(lhs, op, rhs)
{
  this->branches = new List<Node*>();
  this->branches->Append(lhs);
  this->branches->Append(op);
  this->branches->Append(rhs);

}

void AssignExpr::checkNode()
{
  //printf("I am an assignment statement\n");
}

void AssignExpr::checkNode2()
{
  //Check to make sure the rhs expression has a valid type as the lhs expression
  Expr * l = this->left;
  Expr * r = this->right;
  FieldAccess * f1 = dynamic_cast<FieldAccess *>(l);//Used for normal assignment to fields
  ArrayAccess *a1 = dynamic_cast<ArrayAccess *>(l);//Used for assignment to an entry in an array
  if(f1)
  { 
     Node * d1 = table->Lookup(f1->getIdentifier()->getName());
     VarDecl * v1 = dynamic_cast<VarDecl *>(d1);
     FieldAccess *f2 = dynamic_cast<FieldAccess *>(r);     
     Call * c2 = dynamic_cast<Call *>(r); //Function call

     //Handles the case where the lhs is a variable but the rhs is a constant
     if(v1 && !f2)
     {
       char * firstType = v1->getType()->getTypeName();
       Type * t1 = v1->getType();
       NamedType * nt1 = dynamic_cast<NamedType *>(t1);
       IntConstant * i2 = dynamic_cast<IntConstant *>(r);
       DoubleConstant * d2 = dynamic_cast<DoubleConstant *>(r);
       StringConstant * s2 = dynamic_cast<StringConstant *>(r);
       BoolConstant * b2 = dynamic_cast<BoolConstant *>(r);
       NullConstant * n2 = dynamic_cast<NullConstant *>(r);  
       

       if(firstType || nt1)
       {
         if(firstType && strcmp(firstType, "int") == 0 && d2)
           ReportError::IncompatibleOperands(this->op, Type::intType, Type::doubleType);
         else if(firstType && strcmp(firstType, "int") == 0 && s2)
            ReportError::IncompatibleOperands(this->op, Type::intType, Type::stringType);
         else if(firstType && strcmp(firstType, "int") == 0 && b2)
            ReportError::IncompatibleOperands(this->op, Type::intType, Type::boolType);
         else if(firstType && strcmp(firstType, "int") == 0 && n2)
            ReportError::IncompatibleOperands(this->op, Type::intType, Type::nullType);
         else if(firstType && strcmp(firstType, "double") == 0 && i2)
            ReportError::IncompatibleOperands(this->op, Type::doubleType, Type::intType);    
         else if(firstType && strcmp(firstType, "double") == 0 && s2)
            ReportError::IncompatibleOperands(this->op, Type::doubleType, Type::stringType);
         else if(firstType && strcmp(firstType, "double") == 0 && b2)
            ReportError::IncompatibleOperands(this->op, Type::doubleType, Type::boolType);   
         else if(firstType && strcmp(firstType, "double") == 0 && n2)
            ReportError::IncompatibleOperands(this->op, Type::doubleType, Type::nullType);
         else if(firstType && strcmp(firstType, "string") == 0 && i2)
            ReportError::IncompatibleOperands(this->op, Type::stringType, Type::intType);    
         else if(firstType && strcmp(firstType, "string") == 0 && d2)
            ReportError::IncompatibleOperands(this->op, Type::stringType, Type::doubleType);
         else if(firstType && strcmp(firstType, "string") == 0 && b2)
            ReportError::IncompatibleOperands(this->op, Type::stringType, Type::boolType);   
         else if(firstType && strcmp(firstType, "string") == 0 && n2)
            ReportError::IncompatibleOperands(this->op, Type::stringType, Type::nullType);
         else if(firstType && strcmp(firstType, "bool") == 0 && i2)
            ReportError::IncompatibleOperands(this->op, Type::boolType, Type::intType);    
         else if(firstType && strcmp(firstType, "bool") == 0 && d2)
            ReportError::IncompatibleOperands(this->op, Type::boolType, Type::doubleType);
         else if(firstType && strcmp(firstType, "bool") == 0 && s2)
            ReportError::IncompatibleOperands(this->op, Type::boolType, Type::stringType);   
         else if(firstType && strcmp(firstType, "bool") == 0 && n2)
            ReportError::IncompatibleOperands(this->op, Type::boolType, Type::nullType);
         else if(!nt1 && n2)
            ReportError::IncompatibleOperands(this->op, t1, Type::nullType);
         else if(c2)
         {
             Node * fnNode = table->Lookup(c2->getField()->getName());
             FnDecl * fnDecl = dynamic_cast<FnDecl *>(fnNode);
             if(fnDecl) 
             {   
              Type * fnType = fnDecl->getReturnType();
              NamedType * fnNamedType = dynamic_cast<NamedType *>(fnType);
              if(fnType)
              {
                char * fnReturnType = fnType->getTypeName();
                if(firstType && fnReturnType && strcmp(firstType, fnReturnType) != 0)
                  ReportError::IncompatibleOperands(this->op, t1, fnType);
                else if(nt1 && fnReturnType && strcmp(nt1->getId()->getName(), fnReturnType) != 0)
                  ReportError::IncompatibleOperands(this->op, nt1, fnType);               
              }
              else if(fnNamedType)
              {
                char * fnNamedReturnType = fnNamedType->getId()->getName();
                if(firstType && fnNamedType && strcmp(firstType, fnNamedReturnType) != 0)
                  ReportError::IncompatibleOperands(this->op, t1, fnNamedType);
              }   
             }     
         }
       }
     }
     else if(v1 && f2)  //Handles the case where both the lhs and the rhs are variables
     {
         Node * d2 = table->Lookup(f2->getIdentifier()->getName());     
         VarDecl * v2 = dynamic_cast<VarDecl *>(d2);
         if(v2)
         {
           Type * type1 = v1->getType();
           Type * type2 = v2->getType();
           char * firstType = type1->getTypeName(); 
           char * secondType = type2->getTypeName();
           NamedType * nt1 = dynamic_cast<NamedType *>(type1);
           NamedType * nt2 = dynamic_cast<NamedType *>(type2);                     

           if(firstType && secondType)
           {   
             if(strcmp(firstType, secondType) != 0)
               ReportError::IncompatibleOperands(this->op, type1, type2);
           }
           else if(nt1 && secondType)
           {
             
             if(strcmp(nt1->getId()->getName(), secondType) != 0)
               ReportError::IncompatibleOperands(this->op, nt1, type2);

           }
           else if(nt1 && nt2)
           {
             if(strcmp(nt1->getId()->getName(), nt2->getId()->getName()) != 0)
             {
               //  It is legal to assign a derived type (either through extends or implements) to
               //a variable of base type 
               Node * base = table->Lookup(nt1->getId()->getName());
               ClassDecl * baseClass = dynamic_cast<ClassDecl *>(base);
               InterfaceDecl * baseInterface = dynamic_cast<InterfaceDecl *>(base);
               Node * derived = table->Lookup(nt2->getId()->getName());
               ClassDecl * derivedClass = dynamic_cast<ClassDecl *>(derived);

               //A named type variable of a subclass can be assigned to a named type variable of a base class.
               if(baseClass && derivedClass && derivedClass->getExtends())
               {
                 if(!(strcmp(nt2->getId()->getName(), derivedClass->getIdentifier()->getName()) == 0 && strcmp(nt1->getId()->getName(), derivedClass->getExtends()->getId()->getName()) == 0))
                 {
                   ReportError::IncompatibleOperands(this->op, nt1, nt2); 
                   return;
                 } 
               
               }//A named type variable of a class that implementes an interface can be assigned to a named type variable of that interface
               else if(baseInterface && derivedClass && derivedClass->getImplements()->NumElements() > 0)
               {
                 bool isImplemented = false;
                 List<NamedType *> *implements = derivedClass->getImplements();
                 for(int i = 0; i < implements->NumElements(); i++)
                 {
                   if(strcmp(nt1->getId()->getName(), implements->Nth(i)->getId()->getName()) == 0)
                   {
                     isImplemented = true;
                     if(!(strcmp(nt2->getId()->getName(), derivedClass->getIdentifier()->getName()) == 0))
                     {
                      ReportError::IncompatibleOperands(this->op, nt1, nt2);
                      return;
                     }
                   }
                 }
                 if(!isImplemented)
                 {
                    isImplemented = false;
                    ReportError::IncompatibleOperands(this->op, nt1, nt2);
                    return; 
                
                 }
               }
               else //All other cases of two different named types are errors
               {
                 ReportError::IncompatibleOperands(this->op, nt1, nt2);
               }
             }
           }   
         }
     }
  }
  else if(a1)//Handle array subscript assignment case here
  {
    Expr * arrayBase = a1->getBase();
    FieldAccess * arrayBase2 = dynamic_cast<FieldAccess *>(arrayBase);
    
    //Handle array subscript assignment case here
     Node * d1 = table->Lookup(arrayBase2->getIdentifier()->getName());
     VarDecl * v1 = dynamic_cast<VarDecl *>(d1);
     FieldAccess *f2 = dynamic_cast<FieldAccess *>(r);     

     //Handles the case where the lhs is a variable but the rhs is a constant
     if(v1 && !f2)
     {
       char * firstType = v1->getType()->getTypeName();
       Type * t1 = v1->getType();
       NamedType * nt1 = dynamic_cast<NamedType *>(t1);
       IntConstant * i2 = dynamic_cast<IntConstant *>(r);
       DoubleConstant * d2 = dynamic_cast<DoubleConstant *>(r);
       StringConstant * s2 = dynamic_cast<StringConstant *>(r);
       BoolConstant * b2 = dynamic_cast<BoolConstant *>(r);
       NullConstant * n2 = dynamic_cast<NullConstant *>(r);  

       if(firstType)
       {
         if(strcmp(firstType, "int") == 0 && d2)
           ReportError::IncompatibleOperands(this->op, Type::intType, Type::doubleType);
         else if(strcmp(firstType, "int") == 0 && s2)
            ReportError::IncompatibleOperands(this->op, Type::intType, Type::stringType);
         else if(strcmp(firstType, "int") == 0 && b2)
            ReportError::IncompatibleOperands(this->op, Type::intType, Type::boolType);
         else if(strcmp(firstType, "int") == 0 && n2)
            ReportError::IncompatibleOperands(this->op, Type::intType, Type::nullType);
         else if(strcmp(firstType, "double") == 0 && i2)
            ReportError::IncompatibleOperands(this->op, Type::doubleType, Type::intType);    
         else if(strcmp(firstType, "double") == 0 && s2)
            ReportError::IncompatibleOperands(this->op, Type::doubleType, Type::stringType);
         else if(strcmp(firstType, "double") == 0 && b2)
            ReportError::IncompatibleOperands(this->op, Type::doubleType, Type::boolType);   
         else if(strcmp(firstType, "double") == 0 && n2)
            ReportError::IncompatibleOperands(this->op, Type::doubleType, Type::nullType);
         else if(strcmp(firstType, "string") == 0 && i2)
            ReportError::IncompatibleOperands(this->op, Type::stringType, Type::intType);    
         else if(strcmp(firstType, "string") == 0 && d2)
            ReportError::IncompatibleOperands(this->op, Type::stringType, Type::doubleType);
         else if(strcmp(firstType, "string") == 0 && b2)
            ReportError::IncompatibleOperands(this->op, Type::stringType, Type::boolType);   
         else if(strcmp(firstType, "string") == 0 && n2)
            ReportError::IncompatibleOperands(this->op, Type::stringType, Type::nullType);
         else if(strcmp(firstType, "bool") == 0 && i2)
            ReportError::IncompatibleOperands(this->op, Type::boolType, Type::intType);    
         else if(strcmp(firstType, "bool") == 0 && d2)
            ReportError::IncompatibleOperands(this->op, Type::boolType, Type::doubleType);
         else if(strcmp(firstType, "bool") == 0 && s2)
            ReportError::IncompatibleOperands(this->op, Type::boolType, Type::stringType);   
         else if(strcmp(firstType, "bool") == 0 && n2)
            ReportError::IncompatibleOperands(this->op, Type::boolType, Type::nullType);
         else if(!nt1 && n2)
            ReportError::IncompatibleOperands(this->op, t1, Type::nullType);  



       }
     }
     else if(v1 && f2)  //Handles the case where both the lhs and the rhs are variables
     {
         Node * d2 = table->Lookup(f2->getIdentifier()->getName());     
         VarDecl * v2 = dynamic_cast<VarDecl *>(d2);
         if(v2)
         {
           Type * type1 = v1->getType();
           Type * type2 = v2->getType();
           char * firstType = type1->getTypeName(); 
           char * secondType = type2->getTypeName();
           NamedType * nt1 = dynamic_cast<NamedType *>(type1);
           NamedType * nt2 = dynamic_cast<NamedType *>(type2);                     

           if(firstType && secondType)
           {   
             if(strcmp(firstType, secondType) != 0)
               ReportError::IncompatibleOperands(this->op, type1, type2);
           }
           else if(nt1 && secondType)
           {
             
             if(strcmp(nt1->getId()->getName(), secondType) != 0)
               ReportError::IncompatibleOperands(this->op, nt1, type2);

           }
           else if(nt1 && nt2)
           {
             if(strcmp(nt1->getId()->getName(), nt2->getId()->getName()) != 0)
             {
               //  It is legal to assign a derived type (either through extends or implements) to
               //a variable of base type 
               Node * base = table->Lookup(nt1->getId()->getName());
               ClassDecl * baseClass = dynamic_cast<ClassDecl *>(base);
               InterfaceDecl * baseInterface = dynamic_cast<InterfaceDecl *>(base);
               Node * derived = table->Lookup(nt2->getId()->getName());
               ClassDecl * derivedClass = dynamic_cast<ClassDecl *>(derived);

               //A named type variable of a subclass can be assigned to a named type variable of a base class.
               if(baseClass && derivedClass && derivedClass->getExtends())
               {
                 if(!(strcmp(nt2->getId()->getName(), derivedClass->getIdentifier()->getName()) == 0 && strcmp(nt1->getId()->getName(), derivedClass->getExtends()->getId()->getName()) == 0))
                 {
                   ReportError::IncompatibleOperands(this->op, nt1, nt2); 
                   return;
                 } 
               
               }//A named type variable of a class that implementes an interface can be assigned to a named type variable of that interface
               else if(baseInterface && derivedClass && derivedClass->getImplements()->NumElements() > 0)
               {
                 bool isImplemented = false;
                 List<NamedType *> *implements = derivedClass->getImplements();
                 for(int i = 0; i < implements->NumElements(); i++)
                 {
                   if(strcmp(nt1->getId()->getName(), implements->Nth(i)->getId()->getName()) == 0)
                   {
                     isImplemented = true;
                     if(!(strcmp(nt2->getId()->getName(), derivedClass->getIdentifier()->getName()) == 0))
                     {
                      ReportError::IncompatibleOperands(this->op, nt1, nt2);
                      return;
                     }
                   }
                 }
                 if(!isImplemented)
                 {
                    isImplemented = false;
                    ReportError::IncompatibleOperands(this->op, nt1, nt2);
                    return; 
                
                 }
               }
               else //All other cases of two named types with differen
               {
                 ReportError::IncompatibleOperands(this->op, nt1, nt2);
               }
             }
           }   
         }
     } 


  }
  //printf("I am an assignment statement\n");
}

PostfixExpr::PostfixExpr(Expr *lhs, Operator *op): CompoundExpr(lhs, op)
{
   this->branches = new List<Node*>();
   this->branches->Append(lhs);
   this->branches->Append(op);


}
void PostfixExpr::checkNode()
{
  //printf("I am a postfix expression\n");
}

void PostfixExpr::checkNode2()
{
  //Check to make sure the operand is an integer variable
  Expr * l = this->left;
  IntConstant * i1 = dynamic_cast<IntConstant *>(l);
  DoubleConstant * d1 = dynamic_cast<DoubleConstant *>(l);
  StringConstant * s1 = dynamic_cast<StringConstant *>(l);  
  BoolConstant * b1 = dynamic_cast<BoolConstant *>(l);
  FieldAccess * v2 = dynamic_cast<FieldAccess *>(l);
  if(!v2)
  {
    if(i1)
       ReportError::IncompatibleOperand(this->op, Type::intType);
    else if(d1)
       ReportError::IncompatibleOperand(this->op, Type::doubleType);
    else if(s1)
        ReportError::IncompatibleOperand(this->op, Type::stringType);
    else if(b1)
        ReportError::IncompatibleOperand(this->op, Type::boolType);       

  }
  else
  {
    Node * n = table->Lookup(v2->getIdentifier()->getName());   
    VarDecl * var2 = dynamic_cast<VarDecl *>(n);
    if(var2)
    {
      char * t2 = var2->getType()->getTypeName();
      if(t2 == NULL || strcmp(t2, "int") != 0)
      {
        ReportError::IncompatibleOperand(this->op, var2->getType());
      }
         
    }
  } 

}


void This::checkNode()
{

}

void This::checkNode2()
{
  Node * parent = this->GetParent();
  ClassDecl * c = dynamic_cast<ClassDecl *>(parent);
  Program * p = dynamic_cast<Program *>(parent);
 
  while(!c && !p)
  { 
    parent = parent->GetParent();
    c = dynamic_cast<ClassDecl *>(parent);
    p = dynamic_cast<Program *>(parent);
  }

  if(p)
  {
    ReportError::ThisOutsideClassScope(this);
  }
}
 
ArrayAccess::ArrayAccess(yyltype loc, Expr *b, Expr *s) : LValue(loc) {
    (base=b)->SetParent(this); 
    (subscript=s)->SetParent(this);
    this->branches = new List<Node *>();
    this->branches->Append(base);
    this->branches->Append(subscript);
}

void ArrayAccess::checkNode()
{

}

void ArrayAccess::checkNode2()
{
   IntConstant * intSubscript = dynamic_cast<IntConstant *>(this->subscript);
   FieldAccess * field = dynamic_cast<FieldAccess *>(this->subscript);  
   bool isError = false;


   // Check to make sure that we only use the double brackets ([]) when accessing
   //elements of array types
   FieldAccess * baseField = dynamic_cast<FieldAccess *>(this->base);
   if(baseField)
   {
     Node * baseNode = table->Lookup(baseField->getIdentifier()->getName());
     VarDecl * baseVar = dynamic_cast<VarDecl *>(baseNode);
     if(baseVar)
     {
       ArrayType * array = dynamic_cast<ArrayType *>(baseVar->getType());
       if(!array)
       {      
         ReportError::BracketsOnNonArray(this->base);
       }
     }
   }

   //Check to make the array subscript is either a variable of type int or
   //an IntConstant.  If it's not either one of these, report an error.
   if(field) 
   {
     Node * d = table->Lookup(field->getIdentifier()->getName());
     VarDecl * v = dynamic_cast<VarDecl *> (d);
     if(v && strcmp(v->getType()->getTypeName(), "int") != 0)
     {
       ReportError::SubscriptNotInteger(this->subscript);
     }
     isError = true;
   }

   if(!intSubscript && !isError)
   {
      ReportError::SubscriptNotInteger(this->subscript); 
      isError = true;
   }   

  




}
    
FieldAccess::FieldAccess(Expr *b, Identifier *f) 
  : LValue(b? Join(b->GetLocation(), f->GetLocation()) : *f->GetLocation()) {
    Assert(f != NULL); // b can be be NULL (just means no explicit base)
    base = b; 
    if (base) base->SetParent(this); 
    (field=f)->SetParent(this);
    this->branches = new List<Node*>();
    if(base) this->branches->Append(base);
    this->branches->Append(field);


}

void FieldAccess::checkNode()
{
  Node * parent = this->GetParent();
  
  //Increment level for switch statement scope if the test expression is a variable
  SwitchStmt * s = dynamic_cast<SwitchStmt *>(parent);
  if(s)
  {
    Expr * test = s->getTestExpr();
    FieldAccess * testField = dynamic_cast<FieldAccess *>(test);
    if(testField && strcmp(testField->field->getName(), this->field->getName()) == 0)
    {
      currentLevel++;  //Increment level for switch statement scope
    }
  }
  //printf("The field is here\n"); 
}

void FieldAccess::checkNode2()
{
  Node * parent = this->GetParent();
  
  //Increment level for switch statement scope if the test expression is a variable
  SwitchStmt * s = dynamic_cast<SwitchStmt *>(parent);
  if(s)
  {
    Expr * test = s->getTestExpr();
    FieldAccess * testField = dynamic_cast<FieldAccess *>(test);
    if(testField && strcmp(testField->field->getName(), this->field->getName()) == 0)
    {
      currentLevel++;  //Increment level for switch statement scope
    }
  }

  Node * fieldNode = table->Lookup(this->field->getName());
  VarDecl * varField = dynamic_cast<VarDecl *>(fieldNode);
  if(!base && !varField)
  {
    ReportError::IdentifierNotDeclared(this->field, LookingForVariable);
  }

 
  This * thisBase = dynamic_cast<This *>(this->base);

  //  Handles the case where we try to access a field in the current class (using "this.") and
  //it is not present.
  if(this->base && thisBase)
  {
      Node * fieldParent = this->GetParent();
      ClassDecl * parentClass = dynamic_cast<ClassDecl *>(fieldParent);
      Program * parentProgram = dynamic_cast<Program *>(fieldParent);
      while(!parentClass && !parentProgram)
      {
        fieldParent = fieldParent->GetParent();
        parentClass = dynamic_cast<ClassDecl *>(fieldParent);
        parentProgram = dynamic_cast<Program *>(fieldParent);
      }
      if(parentClass)
      {
         bool isClassField = false;
         List<Decl *> *classMembers = parentClass->getMembers();
         for(int i = 0; i < classMembers->NumElements(); i++) 
         {            
            VarDecl * classVar = dynamic_cast<VarDecl *>(classMembers->Nth(i));

            //If we find that the field we want is inside the base class, set the bool to true.
            if(classVar && strcmp(this->field->getName(), classVar->getIdentifier()->getName()) == 0)
            {
              isClassField = true;
              
            }
 
          }//If we don't find the field in the current class, report an error.
          if(!isClassField)
          {
             Type * classType = new Type(parentClass->getIdentifier()->getName());//Need a dummy type to hold the name of the class
             ReportError::FieldNotFoundInBase(this->field, classType);

          }
      }

  }
  //Handles the case where a field is being accessed that is not part of its base class type.  We skip this
  //check if our base is "this." since we have alredy checked that case for nonexistent fields.
  if(this->base && !thisBase)
  {
    FieldAccess *baseField = dynamic_cast<FieldAccess *>(this->base);
    if(baseField)
    {
      Node * baseNode = table->Lookup(baseField->field->getName());//Looks up the field we want in the table
      VarDecl * baseVar = dynamic_cast<VarDecl *>(baseNode);
      if(baseVar)
      {
        Type * baseType = baseVar->getType();//Gets the data type of the field (which should be a named type)
        char * baseTypeName = baseType->getTypeName();
        Node * classNode = table->Lookup(baseTypeName);
        ClassDecl * classNode2 = dynamic_cast<ClassDecl *>(classNode);//Assume that it's a named/class type
        if(classNode2)  
        {
          List<Decl *> *classMembers = classNode2->getMembers();//Get class members
          bool isClassField = false;
          VarDecl * resultClassVar = NULL;
          for(int i = 0; i < classMembers->NumElements(); i++) 
         {            
            VarDecl * classVar = dynamic_cast<VarDecl *>(classMembers->Nth(i));

            //If we find that the field we want is inside the base class, set the bool to true.
            if(classVar && strcmp(this->field->getName(), classVar->getIdentifier()->getName()) == 0)
            {
              isClassField = true;
              resultClassVar = classVar;
              
            }
 
          }
          //If we could not find the field inside the base class, report the error.
          if(!isClassField)
            ReportError::FieldNotFoundInBase(this->field, baseType);
          else
          {
              Node * fieldParent = this->GetParent();
              ClassDecl * parentClass = dynamic_cast<ClassDecl *>(fieldParent);
              Program * parentProgram = dynamic_cast<Program *>(fieldParent);
              while(!parentClass && !parentProgram)
              {
                fieldParent = fieldParent->GetParent();
                parentClass = dynamic_cast<ClassDecl *>(fieldParent);
                parentProgram = dynamic_cast<Program *>(fieldParent);
              }
              if(parentClass && resultClassVar && strcmp(parentClass->getIdentifier()->getName(), classNode2->getIdentifier()->getName()) != 0)
              {
                ReportError::InaccessibleField(resultClassVar->getIdentifier(), baseType);

              }
              else if(parentProgram)
              {
                 ReportError::InaccessibleField(resultClassVar->getIdentifier(), baseType);
              }
          }
        }
      }
    }
  }
  //printf("The field is here\n");  
}


Call::Call(yyltype loc, Expr *b, Identifier *f, List<Expr*> *a) : Expr(loc)  {
    Assert(f != NULL && a != NULL); // b can be be NULL (just means no explicit base)
    base = b;
    if (base) base->SetParent(this);
    (field=f)->SetParent(this);
    (actuals=a)->SetParentAll(this);
    this->branches = new List<Node*>();
    if(base) this->branches->Append(base);
    this->branches->Append(field);
    for(int i = 0; i < actuals->NumElements(); i++)
    {
      this->branches->Append(actuals->Nth(i));
    }
}

void Call::checkNode()
{

}

void Call::checkNode2()
{  
  Node * fieldNode = table->Lookup(this->field->getName());
  FnDecl * fnDecl = dynamic_cast<FnDecl *>(fieldNode);
  if(!fnDecl)
  {
    ReportError::IdentifierNotDeclared(this->field, LookingForFunction);
  }
  else
  {
    List<VarDecl *> *formalParams = fnDecl->getFormals();//Formal parameters in function declaration
    List<Expr *> *actualParams = this->actuals;//Actual parameters in function call
    int numFormals = formalParams->NumElements();
    int numActuals = actualParams->NumElements();
    if(numFormals != numActuals)
    {
      ReportError::NumArgsMismatch(fnDecl->getIdentifier(), numFormals, numActuals);
    }
    else
    {
      bool isWrongType = false;
      int index = 0;//Bad Arg index
      VarDecl * misTypedFormal;
      Expr * misTypedArg; 
      
      if(numFormals > 0 && numActuals > 0)//Both formals and actuals have at least one parameter
      {
        misTypedFormal = formalParams->Nth(0); //Default
        misTypedArg = actualParams->Nth(0);  //Default
      }
      else //Both formals and actuals have no parameters
      {
        misTypedFormal = NULL;
        misTypedArg = NULL;
      }

      for(int i = 0; i < numFormals; i++)
      {
        isWrongType = false;
        VarDecl * formal = formalParams->Nth(i);
        
        for(int j = 0; j < numActuals; j++)
        {
          Expr * actual1 = actualParams->Nth(j);
          FieldAccess * actual2 = dynamic_cast<FieldAccess *>(actual1);//Assuming that an actual param is a field not array           
          if(actual2)
          {
            
            Node * d = table->Lookup(actual2->getIdentifier()->getName());
            VarDecl * v = dynamic_cast<VarDecl *>(d);
            Type *t1 = formal->getType();
            Type *t2 = v->getType();//Assuming v is in fact a variable declaration  
            ArrayType * arrayType1 = dynamic_cast<ArrayType *>(t1);//Checks for a formal parameter that is of array type  
            ArrayType * arrayType2 = dynamic_cast<ArrayType *>(t2);//Checks for an actual  parameter that is of array type       
             
            if(v && ((arrayType1 && !arrayType2) || (!arrayType1 && arrayType2)))
            {
              isWrongType = true;
              index = i;
              misTypedFormal = formal;
              misTypedArg = actual1;
              break;
            }
            else if(v && strcmp(formal->getType()->getTypeName(), v->getType()->getTypeName()) != 0)
            {
              isWrongType = true;
              index = i;
              misTypedFormal = formal;
              misTypedArg = actual1;
              break;
            }

          }
        }
        if(isWrongType)
          break;
      }
       
      if(isWrongType)
      {
           
           FieldAccess * actual2 = dynamic_cast<FieldAccess *>(misTypedArg);
           Node * d = table->Lookup(actual2->getIdentifier()->getName());
           VarDecl * v = dynamic_cast<VarDecl *>(d);
           

           if(v)
             ReportError::ArgMismatch(misTypedArg, index, v->getType(), misTypedFormal->getType());  
      }
  
    }
  }

}
 

NewExpr::NewExpr(yyltype loc, NamedType *c) : Expr(loc) { 
  Assert(c != NULL);
  (cType=c)->SetParent(this);
  this->branches = new List<Node *>();
  this->branches->Append(c);


}

void NewExpr::checkNode()
{

}

void NewExpr::checkNode2()
{
  Node * d = table->Lookup(cType->getId()->getName());
  VarDecl * v = dynamic_cast<VarDecl *>(d);
  if(!v)
  {
    ReportError::IdentifierNotDeclared(cType->getId(), LookingForClass);

  } 
  
}


NewArrayExpr::NewArrayExpr(yyltype loc, Expr *sz, Type *et) : Expr(loc) {
    Assert(sz != NULL && et != NULL);
    (size=sz)->SetParent(this); 
    (elemType=et)->SetParent(this);
    this->branches = new List<Node *>();
    this->branches->Append(sz);
    this->branches->Append(et);
}

void NewArrayExpr::checkNode()
{
  

}

void NewArrayExpr::checkNode2()
{
  IntConstant * ic = dynamic_cast<IntConstant *>(this->size);
  FieldAccess * field = dynamic_cast<FieldAccess *>(this->size);  
  bool isError = false;

  //  Check to make sure that the size of the array passed into the NewArray function
  //is either an IntConstant or a variable of type integer.  If it's not one of these,
  //report an error.
  if(field)
  {
    Node * d = table->Lookup(field->getIdentifier()->getName());
    VarDecl * v = dynamic_cast<VarDecl *>(d);
    if(v && strcmp(v->getType()->getTypeName(), "int") != 0)
    {
      ReportError::NewArraySizeNotInteger(this->size);
    }
    isError = true;
  }

  if(!ic && !isError)
  {
     ReportError::NewArraySizeNotInteger(this->size);
     isError = true;
  }

 
}

void ReadIntegerExpr::checkNode()
{

}

void ReadIntegerExpr::checkNode2()
{

}

void ReadLineExpr::checkNode()
{

}

void ReadLineExpr::checkNode2()
{

}

       
