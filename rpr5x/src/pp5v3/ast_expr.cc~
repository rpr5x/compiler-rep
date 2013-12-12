/* File: ast_expr.cc
 * -----------------
 * Implementation of expression node classes.
 */
#include "ast_expr.h"
#include "ast_type.h"
#include "ast_decl.h"
#include <string.h>
#include "codegen.h"
#include "errors.h"
#include <sstream>
#include <string>

extern CodeGenerator *codeGen;


IntConstant::IntConstant(yyltype loc, int val) : Expr(loc) {
    value = val;
}

void IntConstant::Emit()
{
   
  this->intLoc = codeGen->GenLoadConstant(this->value);
  FnDecl::numBytes += 4;  
}


DoubleConstant::DoubleConstant(yyltype loc, double val) : Expr(loc) {
    value = val;
}

//Not necessary since doubles are not used but it's good practice to include this
void DoubleConstant::Emit()
{
  
}

BoolConstant::BoolConstant(yyltype loc, bool val) : Expr(loc) {
    value = val;
}

void BoolConstant::Emit()
{
  this->boolLoc = codeGen->GenLoadConstant(this->value);
  FnDecl::numBytes += 4;
}

StringConstant::StringConstant(yyltype loc, const char *val) : Expr(loc) {
    Assert(val != NULL);
    value = strdup(val);
}

void StringConstant::Emit()
{
  this->strLoc = codeGen->GenLoadConstant(this->value);
  FnDecl::numBytes += 4;
}

//Not sure if this is needed
/*void NullConstant::Emit()
{
 
}*/


Operator::Operator(yyltype loc, const char *tok) : Node(loc) {
    Assert(tok != NULL);
    strncpy(tokenString, tok, sizeof(tokenString));
}
CompoundExpr::CompoundExpr(Expr *l, Operator *o, Expr *r) 
  : Expr(Join(l->GetLocation(), r->GetLocation())) {
    Assert(l != NULL && o != NULL && r != NULL);
    (op=o)->SetParent(this);
    (left=l)->SetParent(this); 
    (right=r)->SetParent(this);
}

CompoundExpr::CompoundExpr(Operator *o, Expr *r) 
  : Expr(Join(o->GetLocation(), r->GetLocation())) {
    Assert(o != NULL && r != NULL);
    left = NULL; 
    (op=o)->SetParent(this);
    (right=r)->SetParent(this);
}
   
void ArithmeticExpr::Emit()
{
  if(left) left->Emit();
  op->Emit();
  right->Emit();

  const char * opName;
  opName = op->str();
  FnDecl::numBytes+=4; //Update BeginFunc size

  FieldAccess * var1 = dynamic_cast<FieldAccess *>(left);
  FieldAccess * var2 = dynamic_cast<FieldAccess *>(right);
 
  IntConstant * ic1 = dynamic_cast<IntConstant *>(left); 
  Call * fn2 = dynamic_cast<Call *>(right);  


  //Account for unary negation operator by creating a zero constant and doing 0-x = -x
  if(strcmp(opName,"-") == 0 && !left)
  {
     Location * leftZero = codeGen->GenLoadConstant(0);
     FnDecl::numBytes+=4;
     this->arithLoc = codeGen->GenBinaryOp(opName, leftZero, right->GetLocationNode());

  }
  else //Normal binary arithmetic
  {
     if(var1 && var2)
     {
       Decl * d1 = FindDecl(var1->GetId());
       Decl * d2 = FindDecl(var2->GetId());
       VarDecl * v1 = dynamic_cast<VarDecl *>(d1);
       VarDecl * v2 = dynamic_cast<VarDecl *>(d2);
       Location * varLoc1 = v1->GetLocationNode();
       Location * varLoc2 = v2->GetLocationNode();
       this->arithLoc = codeGen->GenBinaryOp(opName, varLoc1, varLoc2);
        
     }
     else if(var1 && fn2)
     {
         Decl * d1 = FindDecl(var1->GetId());
         VarDecl * v1 = dynamic_cast<VarDecl *>(d1);
         Identifier * fnIdent = fn2->GetId();
         Decl * d2 = FindDecl(fnIdent);
         FnDecl * fnDecl = dynamic_cast<FnDecl *>(d2);   
         Location * varLoc1 = v1->GetLocationNode();
         Location * fnLoc2 = fnDecl->GetLocationNode();
         this->arithLoc = codeGen->GenBinaryOp(opName, varLoc1, fnLoc2);
  
     }
     else if(var1)
     {
      
       Decl * d1 = FindDecl(var1->GetId());
       VarDecl * v1 = dynamic_cast<VarDecl *>(d1);
       Location * varLoc1 = v1->GetLocationNode();
       Location * rightLoc = right->GetLocationNode();

       //Check for instance variable assignment.  This needs to be done sightly differenly than a normal
       //variable assignment
       Node * parent = v1->GetParent();
       ClassDecl * classParent = dynamic_cast<ClassDecl *>(parent);
       Program * prog = dynamic_cast<Program *>(parent);

       while(!classParent && !prog)
       {
          parent = parent->GetParent();
          classParent = dynamic_cast<ClassDecl *>(parent);
          prog = dynamic_cast<Program *>(parent);
       }

       if(classParent)
       {
          List<Decl *> *members = classParent->GetMembers();
          for(int i = 0; i < members->NumElements(); i++)
          {    
            Decl * member = members->Nth(i);
            VarDecl * var = dynamic_cast<VarDecl *>(member);
            if(var && strcmp(var->GetId()->GetName(), v1->GetId()->GetName()) == 0)
            {
               int offset = 4 *(i+1);     
               Location *instanceLoc1 = codeGen->GenLoad(codeGen->ThisPtr, offset);
               this->arithLoc = codeGen->GenBinaryOp(opName, instanceLoc1, rightLoc);
               
            }
          }
 
      }
      else
      {
          rightLoc = right->GetLocationNode();
          this->arithLoc = codeGen->GenBinaryOp(opName, varLoc1, rightLoc);
      }
     }
     else if(var2)
     {
       
       Decl * d2 = FindDecl(var2->GetId());
       VarDecl * v2 = dynamic_cast<VarDecl *>(d2);
       Location * leftLoc = left->GetLocationNode();
       Location * varLoc2 = v2->GetLocationNode();
       this->arithLoc = codeGen->GenBinaryOp(opName, leftLoc, varLoc2);
     }
     else if(ic1 && fn2)
     {
       Identifier * fnIdent = fn2->GetId();
       Decl * d2 = FindDecl(fnIdent);
       FnDecl * fnDecl = dynamic_cast<FnDecl *>(d2);   
       Location * intLoc1 = ic1->GetLocationNode();
       Location * fnLoc2 = fnDecl->GetLocationNode();
       this->arithLoc = codeGen->GenBinaryOp(opName, intLoc1, fnLoc2);
     }
     else 
     {
       Location * leftLoc = left->GetLocationNode();
       Location * rightLoc = right->GetLocationNode();
       this->arithLoc = codeGen->GenBinaryOp(opName, leftLoc, rightLoc);
       
     }

  }


  
}

void RelationalExpr::Emit()
{
  //Handle op signs of >, >= and <= slightly different here since MIPS does not support these


  left->Emit();
  op->Emit();
  right->Emit();

  const char * opName;
  opName = op->str();
  FnDecl::numBytes+=4; //Update BeginFunc size


  FieldAccess * var1 = dynamic_cast<FieldAccess *>(left);
  FieldAccess * var2 = dynamic_cast<FieldAccess *>(right);
  Call * c2 = dynamic_cast<Call *>(right);
  IntConstant * i2 = dynamic_cast<IntConstant *>(right);


  //If the operand is a greater than symbol, make it a less than and switch the two other arguments.
  if(strcmp(opName, ">") == 0)
  {    
     opName = "<";
     this->relLoc = codeGen->GenBinaryOp(opName, right->GetLocationNode(), left->GetLocationNode());
  }
  else if(strcmp(opName, ">=") == 0)
  {  
     FnDecl::numBytes+=8;
     const char * opName1 = "<";
     const char * opName2 = "==";
     const char * opName3 = "||";
     Location * loc1 = codeGen->GenBinaryOp(opName1, right->GetLocationNode(), left->GetLocationNode());//Reverse for less than
     Location * loc2 = codeGen->GenBinaryOp(opName2, left->GetLocationNode(), right->GetLocationNode());
     this->relLoc = codeGen->GenBinaryOp(opName3, loc1, loc2);  

  }
  else if(strcmp(opName, "<=") == 0)
  {
     FnDecl::numBytes+=8;
     const char * opName1 = "<";  
     const char * opName2 = "==";
     const char * opName3 = "||";
     
    
     if(var1 && var2)
     {
       Decl * d1 = FindDecl(var1->GetId());
       Decl * d2 = FindDecl(var2->GetId());
       VarDecl * v1 = dynamic_cast<VarDecl *>(d1);
       VarDecl * v2 = dynamic_cast<VarDecl *>(d2);
    
       Location * varLoc = v1->GetLocationNode();  
       Location * varLoc2 = v2->GetLocationNode();
       Location * loc1= codeGen->GenBinaryOp(opName1, varLoc, varLoc2);    
       Location * loc2 = codeGen->GenBinaryOp(opName2, varLoc, varLoc2);//Equal to
       this->relLoc = codeGen->GenBinaryOp(opName3, loc1, loc2);//Logical OR
   
     } 
     else if(var1)
     {
       Decl * d1 = FindDecl(var1->GetId());
       
       VarDecl * v1 = dynamic_cast<VarDecl *>(d1);
       Location * varLoc = v1->GetLocationNode();
       Location * exLoc2 = right->GetLocationNode();
       Location * loc1 = codeGen->GenBinaryOp(opName1, varLoc, exLoc2);//Logical OR
       Location * loc2 = codeGen->GenBinaryOp(opName2, varLoc, exLoc2);//Equal to
       this->relLoc = codeGen->GenBinaryOp(opName3, loc1, loc2);//Logical OR

     }
     else if(var2)
     {
       
       Decl * d2 = FindDecl(var2->GetId());
       
       VarDecl * v2 = dynamic_cast<VarDecl *>(d2);
    

       FnDecl::numBytes+=8;

       Location * exLoc1 = left->GetLocationNode();
       Location * varLoc = v2->GetLocationNode();
       Location * loc1 = codeGen->GenBinaryOp(opName1, exLoc1, varLoc);
       Location * loc2 = codeGen->GenBinaryOp(opName2, exLoc1, varLoc);
       this->relLoc = codeGen->GenBinaryOp(opName3, loc1, loc2); 

     }
     else
     {
       FnDecl::numBytes+=8;
       Location * loc1 = codeGen->GenBinaryOp(opName1, left->GetLocationNode(), right->GetLocationNode());//Less than
       Location * loc2 = codeGen->GenBinaryOp(opName2, left->GetLocationNode(), right->GetLocationNode());//Equal to
       this->relLoc = codeGen->GenBinaryOp(opName3, loc1, loc2);//Logical OR
     }
  }
  else if (strcmp(opName, "<") == 0)
  {
    FnDecl::numBytes+=4;
    if(var1 && var2)
    {
      Decl * d1 = FindDecl(var1->GetId());
      Decl * d2 = FindDecl(var2->GetId());
      VarDecl * v1 = dynamic_cast<VarDecl *>(d1);
      VarDecl * v2 = dynamic_cast<VarDecl *>(d2);
      Location * varLoc = v1->GetLocationNode();  
      Location * varLoc2 = v2->GetLocationNode();
      this->relLoc = codeGen->GenBinaryOp(opName, varLoc, varLoc2);   
      
    }
    else if(var1 && c2)
    {
      Decl * d1 = FindDecl(var1->GetId());
      VarDecl * v1 = dynamic_cast<VarDecl *>(d1);
      Decl * d2;
      Location * varLoc = v1->GetLocationNode();
      Location * fnLoc;
      FnDecl * f2;
      Expr * baseCall = c2->GetBase();
      FieldAccess * baseField = dynamic_cast<FieldAccess *>(baseCall);

      //  We need a special case for the .length() method for arrays here because that method
      //is never declared as a function.
      if(baseField)
      {
        Decl * baseDecl = FindDecl(baseField->GetId());
        VarDecl * baseVar = dynamic_cast<VarDecl *>(baseDecl);

        if(baseVar)
        {
          Type * baseType = baseVar->GetDeclaredType();
          ArrayType * arrayBase = dynamic_cast<ArrayType *>(baseType);
 
         if(arrayBase && strcmp(c2->GetId()->GetName(), "length") == 0)
         { 
          fnLoc = baseVar->GetArrayLength();   
         }
        }
      }
      else
      {
         d2 = FindDecl(c2->GetId());       
         f2 = dynamic_cast<FnDecl *>(d2);
         fnLoc = f2->GetLocationNode(); 
      } 
             
      this->relLoc = codeGen->GenBinaryOp(opName, varLoc, fnLoc);
    }
    else if(var1 && i2)
    {
      Decl * d1 = FindDecl(var1->GetId());
      VarDecl * v1 = dynamic_cast<VarDecl *>(d1);
      Location * varLoc = v1->GetLocationNode();
      Location * intLoc2 = i2->GetLocationNode();
      this->relLoc = codeGen->GenBinaryOp(opName, varLoc, intLoc2);
    }   
    
  }
}

void EqualityExpr::Emit()//Revise this for array accesses later
{
  left->Emit();
  op->Emit();
  right->Emit();

  const char * opName;
  const char * eqName = "==";

  opName = op->str();
  FnDecl::numBytes+=4; //Update BeginFunc size


  FieldAccess * var1 = dynamic_cast<FieldAccess *>(left);
  FieldAccess * var2 = dynamic_cast<FieldAccess *>(right);
  ArrayAccess * ar1 = dynamic_cast<ArrayAccess *>(left);
  ArrayAccess * ar2 = dynamic_cast<ArrayAccess *>(right);


  //Handle Not Equal To 
  if(strcmp(opName, "!=") == 0)
  {
    FnDecl::numBytes+=8;

    //Handles the case where the operator is == and both operands are variables 
    if(var1 && var2)
    {
      Decl * d1 = FindDecl(var1->GetId());
      Decl * d2 = FindDecl(var2->GetId());
      VarDecl * v1 = dynamic_cast<VarDecl *>(d1);
      VarDecl * v2 = dynamic_cast<VarDecl *>(d2);
      if(v1 && v2)
      {
        Location * varLoc = v1->GetLocationNode();  
        //FnDecl::numBytes+=4; //Update BeginFunc size 
        Location * varLoc2 = v2->GetLocationNode();
        
        char * t1 = v1->GetDeclaredType()->GetName();
        char * t2 = v2->GetDeclaredType()->GetName();

        if(strcmp(t1, "string") == 0 && strcmp(t2, "string") == 0)
        {
          FnDecl::numBytes+=8;
          Location * tempLoc = codeGen->GenBuiltInCall(StringEqual, varLoc, varLoc2);
          Location * zeroLoc = codeGen->GenLoadConstant(0);
          Location * eqZeroLoc = codeGen->GenBinaryOp("==", tempLoc, zeroLoc);
          this->eqLoc = eqZeroLoc;
        }
        else
        { 
          FnDecl::numBytes+=8;
          Location * tempLoc = codeGen->GenBinaryOp(eqName, varLoc, varLoc2); 
          Location * zeroLoc = codeGen->GenLoadConstant(0);
          Location * eqZeroLoc = codeGen->GenBinaryOp("==", tempLoc, zeroLoc);
          this->eqLoc = eqZeroLoc;
        }
      }
    } 
    else if(var1)
    {
      Decl * d1 = FindDecl(var1->GetId());
      VarDecl * v1 = dynamic_cast<VarDecl *>(d1);
     
      if(v1)
      {
        Location * varLoc = v1->GetLocationNode();  
        //FnDecl::numBytes+=4; //Update BeginFunc size 
        Location * rightLoc = right->GetLocationNode();

        FnDecl::numBytes+=8;
        Location * tempLoc = codeGen->GenBinaryOp(eqName, varLoc, rightLoc); 
        Location * zeroLoc = codeGen->GenLoadConstant(0);
        Location * eqZeroLoc = codeGen->GenBinaryOp("==", tempLoc, zeroLoc);
        this->eqLoc = eqZeroLoc;

        //this->eqLoc = codeGen->GenBinaryOp(opName, varLoc, rightLoc); 
      }

   }
     else //Default case of two integers, strings, or bools (this should be revised for array accesses later)
     {
        Location * tempLoc = codeGen->GenBinaryOp(eqName, left->GetLocationNode(), right->GetLocationNode()); 
        Location * zeroLoc = codeGen->GenLoadConstant(0);
        Location * eqZeroLoc = codeGen->GenBinaryOp("==", tempLoc, zeroLoc);
        FnDecl::numBytes+=8;
        this->eqLoc = eqZeroLoc;
 
        //this->eqLoc = codeGen->GenBinaryOp(opName, left->GetLocationNode(), right->GetLocationNode());

     }   
  } 
  else
  {
    //Handles the case where the operator is == and both operands are variables 
    if(var1 && var2)
    {
      Decl * d1 = FindDecl(var1->GetId());
      Decl * d2 = FindDecl(var2->GetId());
      VarDecl * v1 = dynamic_cast<VarDecl *>(d1);
      VarDecl * v2 = dynamic_cast<VarDecl *>(d2);
      if(v1 && v2)
      {
        Location * varLoc = v1->GetLocationNode();  
        //FnDecl::numBytes+=4; //Update BeginFunc size 
        Location * varLoc2 = v2->GetLocationNode();
        
        char * t1 = v1->GetDeclaredType()->GetName();
        char * t2 = v2->GetDeclaredType()->GetName();

        if(strcmp(t1, "string") == 0 && strcmp(t2, "string") == 0)
          this->eqLoc = codeGen->GenBuiltInCall(StringEqual, varLoc, varLoc2);

        else
          this->eqLoc = codeGen->GenBinaryOp(opName, varLoc, varLoc2); 
      }
    } 
    else if(var1)
    {
      Decl * d1 = FindDecl(var1->GetId());
      VarDecl * v1 = dynamic_cast<VarDecl *>(d1);
     
      if(v1)
      {
        Location * varLoc = v1->GetLocationNode();  
        //FnDecl::numBytes+=4; //Update BeginFunc size 
        Location * rightLoc = right->GetLocationNode();
        this->eqLoc = codeGen->GenBinaryOp(opName, varLoc, rightLoc); 
      }

     }
     else if(ar1 && var2)
     {
        Expr * base = ar1->GetBase();
        Expr * subscript = ar1->GetSubscript();
        FieldAccess * baseField = dynamic_cast<FieldAccess *>(base);
        FieldAccess * subscript2 = dynamic_cast<FieldAccess *>(subscript); 
        IntConstant * subscript3 = dynamic_cast<IntConstant *>(subscript);
        
               


        Decl * var = FindDecl(var2->GetId());
        VarDecl * var2 = dynamic_cast<VarDecl *>(var);
        Location * varLoc = var2->GetLocationNode();
        char * t2 = var2->GetDeclaredType()->GetName();       



        //Handles the cases for array access 
        if(baseField)
        {
          Decl * d2 = FindDecl(baseField->GetId());
          VarDecl * a2 = dynamic_cast<VarDecl *>(d2);    

         if(a2 && subscript)
         {
           Location * arrayLoc = a2->GetLocationNode();
           char * firstLabel = codeGen->NewLabel();
        
            const char * badSubscript = err_arr_out_of_bounds;

           //Check for illegal array subscript here
           Location * subscriptLoc;   

         
           if(subscript3)
           {
              subscriptLoc = codeGen->GenLoadConstant(subscript3->GetValue());
              FnDecl::numBytes+=4; 
           }
           else if(subscript2)
           {           
             Decl * d = FindDecl(subscript2->GetId());
             VarDecl * v = dynamic_cast<VarDecl *>(d);
             if(v)
             {              
              subscriptLoc = v->GetLocationNode();
             }
            }
          
         //Location * subscriptLoc = GenLoadConstant(arraySubscript);
         Location * zeroLoc = codeGen->GenLoadConstant(0);
         Location * firstCheck = codeGen->GenBinaryOp("<", subscriptLoc, zeroLoc);
         
         Location * arrayLength = codeGen->GenLoad(arrayLoc, -4);
         Location * secondCheck = codeGen->GenBinaryOp("<", subscriptLoc, arrayLength);

         Location * equalCheck = codeGen->GenBinaryOp("==", secondCheck, zeroLoc);
 
         Location * orCheck = codeGen->GenBinaryOp("||", firstCheck, equalCheck);

         codeGen->GenIfZ(orCheck, firstLabel);

         //Executes if the subscript is bad
         Location * badSubscriptError = codeGen->GenLoadConstant(badSubscript); 

         codeGen->GenBuiltInCall(PrintString, badSubscriptError, NULL);

         codeGen->GenBuiltInCall(Halt, NULL, NULL);

         //At this point, the subscript is valid.
         codeGen->GenLabel(firstLabel);
         Location * fourLoc = codeGen->GenLoadConstant(4);
         Location * fourBytes = codeGen->GenBinaryOp("*", fourLoc, subscriptLoc);         
         Location * shiftLoc = codeGen->GenBinaryOp("+", arrayLoc, fourBytes);
         Location * finalLoc = codeGen->GenLoad(shiftLoc, 0);
         FnDecl::numBytes+=44;

         Type * type1;
         Decl * d = FindDecl(baseField->GetId());
         VarDecl * v = dynamic_cast<VarDecl *>(d);
         type1 = v->GetDeclaredType();
         ArrayType *typeArray = dynamic_cast<ArrayType *>(type1);
         const char * t1 = typeArray->GetName();
         

        if(strcmp(t1, "string[]") == 0 && strcmp(t2, "string") == 0)
          this->eqLoc = codeGen->GenBuiltInCall(StringEqual, finalLoc, varLoc);

        else
          this->eqLoc = codeGen->GenBinaryOp(opName, finalLoc, varLoc);







         //this->eqLoc = codeGen->GenBinaryOp(opName, finalLoc, varLoc); 
       }
    }




     }
     else //Default case of two integers, strings, or bools (this should be revised for array accesses later)
     {
        this->eqLoc = codeGen->GenBinaryOp(opName, left->GetLocationNode(), right->GetLocationNode());
        
     }   
   }   

}

void LogicalExpr::Emit()
{
  if(left) left->Emit();
  op->Emit();
  right->Emit();

  const char * opName;
  opName = op->str();
  FnDecl::numBytes+=4; //Update BeginFunc size

  //Handle "not" logical operator
  if(!left && strcmp(opName, "!") == 0)
  {
   Location * zeroLoc = codeGen->GenLoadConstant(0);
   FnDecl::numBytes+=4; 
   Location * rightLoc = right->GetLocationNode();
   const char * opName = "==";
   this->logLoc = codeGen->GenBinaryOp(opName, rightLoc, zeroLoc); 
   
  }
  else 
  {
    this->logLoc = codeGen->GenBinaryOp(opName, left->GetLocationNode(), right->GetLocationNode()); 
  }
  
}

void AssignExpr::Emit()
{
  left->Emit();
  op->Emit();
  right->Emit();   

  Node * parent = this->GetParent();
  ClassDecl * classParent = dynamic_cast<ClassDecl *>(parent);
  Program * prog = dynamic_cast<Program *>(parent);
  

  FieldAccess * left2 = dynamic_cast<FieldAccess *>(left);
  ArrayAccess * left3 = dynamic_cast<ArrayAccess *>(left);
  FieldAccess * right2 = dynamic_cast<FieldAccess *>(right);
  Call * right3 = dynamic_cast<Call *>(right);
  NewArrayExpr * right4 = dynamic_cast<NewArrayExpr *>(right);
  ArrayAccess * right5 = dynamic_cast<ArrayAccess *>(right); 
  NewExpr * right6 = dynamic_cast<NewExpr *>(right); 
  ArithmeticExpr * right7 = dynamic_cast<ArithmeticExpr *>(right);



  //Case 1:  Both left and right are variables
  if(left2 && right2)
  {
    Decl * d1 = FindDecl(left2->GetId());
    Decl * d2 = FindDecl(right2->GetId());
    VarDecl * v1 = dynamic_cast<VarDecl *>(d1);
    VarDecl * v2 = dynamic_cast<VarDecl *>(d2);

    if(v1 && v2)
    {
      Location * varLoc = v1->GetLocationNode();  
      //FnDecl::numBytes+=4; //Update BeginFunc size
       
      //Loop until we find a parent class or reach the program root node
      while(!classParent && !prog)
      {     
         parent = parent->GetParent(); 
         classParent = dynamic_cast<ClassDecl *>(parent);
         prog = dynamic_cast<Program *>(parent);
      }

      
      //If the function is inside a class, check to see if the instance variables are being assigned
      if(classParent)
      {
         List<Decl *> *members = classParent->GetMembers();
         bool isClassMember = false;         
        
         int varCount = 0;
         //Counts the number of instance variables in the class
         for(int i = 0; i < members->NumElements(); i++)
         {
           Decl * member = members->Nth(i);
           VarDecl * varMember = dynamic_cast<VarDecl *>(member);
           if(varMember)
              varCount++;
         }

         varCount *= 4; //Multiply varCount by 4 to account for byte offset

         List<VarDecl *> *varMembers = new List<VarDecl *>();

         int offset = 0; //Offset into list of instance variables
         for(int i = 0; i < members->NumElements(); i++)
         {
           Decl * member = members->Nth(i);
           VarDecl * varMember = dynamic_cast<VarDecl *>(member);
           if(varMember) 
               varMembers->Append(varMember);
         }
         //  Generates the appropriate code if an instance variable is accessed (on the lhs of an assignment expression).
         //Note that the GenStore() instruction only works if all instance variables are declared before the methods.


         for(int i = 0; i < varMembers->NumElements(); i++)
         {
              VarDecl * varMember = varMembers->Nth(i);
              if(strcmp(varMember->GetId()->GetName(), v1->GetId()->GetName()) == 0)
                offset = (4 * i) + 4;
         }

         for(int i = 0; i < members->NumElements(); i++)
         {         
           Decl * member = members->Nth(i);
           VarDecl * varMember = dynamic_cast<VarDecl *>(member);
           if(varMember && strcmp(v1->GetId()->GetName(), varMember->GetId()->GetName()) == 0)
           { 
             Location * thisLoc = codeGen->ThisPtr;
             codeGen->GenStore(thisLoc, v2->GetLocationNode(), offset);
             isClassMember = true;
             
           }
         }

         //If the variable on the left is not an instance variable, do normal assignment
         if(!isClassMember)
         {
            Location * varLoc2 = v2->GetLocationNode();
            codeGen->GenAssign(varLoc, varLoc2);
            
         }

      }
      else
      {
        Location * varLoc2 = v2->GetLocationNode();
        codeGen->GenAssign(varLoc, varLoc2);
        
      }
    }
  }
  else if(left2 && right3)//Case 2:  Left side is a variable and right side is a function
  {
    

     Decl * d1 = FindDecl(left2->GetId());
     VarDecl * v1 = dynamic_cast<VarDecl *>(d1);
     Decl * d2 = FindDecl(right3->GetId());
     FnDecl * f2 = dynamic_cast<FnDecl *>(d2);
     if(v1 && f2)
     {
       Location * varLoc = v1->GetLocationNode();
       Location * rightLoc;
       rightLoc = f2->GetLocationNode();
       codeGen->GenAssign(varLoc, rightLoc);
       
     }

  }
  else if(left2 && right4)//Case 3: The left side is a variable and the right side is a NewArrayExpr
  {
     Decl * d1 = FindDecl(left2->GetId());
     VarDecl * v1 = dynamic_cast<VarDecl *>(d1);
     if(v1)
     {
       Location * varLoc;
       Location * rightLoc = right4->GetLocationNode();

       //Check for instance variable assignment.  This needs to be done sightly differenly than a normal
       //variable assignment
       Node * parent = v1->GetParent();
       ClassDecl * classParent = dynamic_cast<ClassDecl *>(parent);
       Program * prog = dynamic_cast<Program *>(parent);

       while(!classParent && !prog)
       {
          parent = parent->GetParent();
          classParent = dynamic_cast<ClassDecl *>(parent);
          prog = dynamic_cast<Program *>(parent);
       }

       if(classParent)
       {
          List<Decl *> *members = classParent->GetMembers();
          for(int i = 0; i < members->NumElements(); i++)
          {    
            Decl * member = members->Nth(i);
            VarDecl * var = dynamic_cast<VarDecl *>(member);
            if(var && strcmp(var->GetId()->GetName(), v1->GetId()->GetName()) == 0)
            {
               int offset = 4 *(i+1);     
               codeGen->GenStore(codeGen->ThisPtr, rightLoc, offset);
            }
          }
 
       }
       else
       {
        varLoc = v1->GetLocationNode();
        codeGen->GenAssign(varLoc, rightLoc); 
       }
      
       
              
     }

  }
  else if(left2 && right5) //Case 4:  The left is a variable and the right side is an array access
  {
     Decl * d1 = FindDecl(left2->GetId());
     VarDecl * v1 = dynamic_cast<VarDecl *>(d1);
     

     Expr * arrayBase = right5->GetBase();
     Expr * arraySubscript = right5->GetSubscript();

     //Array subscript can be either an integer or an arithmetic expression
     IntConstant * arraySubscript2 = dynamic_cast<IntConstant *>(arraySubscript);
     ArithmeticExpr * arraySubscript3 = dynamic_cast<ArithmeticExpr *>(arraySubscript);

     FieldAccess * arrayBase2 = dynamic_cast<FieldAccess *>(arrayBase);

     //Handles the cases for array access 
     if(arrayBase2)
     {
       Decl * d2 = FindDecl(arrayBase2->GetId());
       VarDecl * a2 = dynamic_cast<VarDecl *>(d2);    

       if(v1 && a2 && arraySubscript)
       {
         
         Location * varLoc = v1->GetLocationNode();
         
         Location * arrayLoc = a2->GetLocationNode();
         char * firstLabel = codeGen->NewLabel();
         
         const char * badSubscript = err_arr_bad_size;

         //Check for illegal array subscript here
         Location * subscriptLoc;   

         FieldAccess * f1 = dynamic_cast<FieldAccess *>(arraySubscript);    
         

         //Case 1 is where the array subscript is an integer, Case 2 is when it is a variable and Case 3
         //is when it is an arithmetic expression
         if(arraySubscript2)
         {
            subscriptLoc = codeGen->GenLoadConstant(arraySubscript2->GetValue());
            FnDecl::numBytes+=4;
         }
         else if(f1)
         {        
           Decl * d = FindDecl(f1->GetId());
           VarDecl * v = dynamic_cast<VarDecl *>(d);
           if(v)
           {              
              subscriptLoc = v->GetLocationNode();
           }
         }
         else if(arraySubscript3)
         {
            Expr * left = arraySubscript3->GetLeft();
            FieldAccess * leftField = dynamic_cast<FieldAccess *>(left);

            //Loop until we find a parent class or reach the program root node
            while(!classParent && !prog)
            {     
              parent = parent->GetParent(); 
              classParent = dynamic_cast<ClassDecl *>(parent);
              prog = dynamic_cast<Program *>(parent);
            }

            //If the array is inside a class, check to see if the instance variables are being assigned
            if(classParent)
            {
              List<Decl *> *members = classParent->GetMembers();

              int offset1 = 0; //Offset into left instance variable (if present)
              int offset2 = 0; //Offset into right instance variable (if present)
              int offset3 = 0;
             //  Generates the appropriate code if an instance variable is accessed (on the lhs of an assignment expression).
             //Note that the GenStore() instruction only works if all instance variables are declared before the methods.

            for(int i = 0; i < members->NumElements(); i++)
            {
               Decl * member = members->Nth(i);
               VarDecl * varMember = dynamic_cast<VarDecl *>(member);
               if(varMember && strcmp(varMember->GetId()->GetName(), v1->GetId()->GetName()) == 0)
                 offset1 = (4 * i) + 4;
               else if(varMember && leftField && strcmp(varMember->GetId()->GetName(), leftField->GetId()->GetName()) == 0)
                 offset2 = (4 * i) + 4;
              else if(varMember && a2 && strcmp(varMember->GetId()->GetName(), a2->GetId()->GetName()) == 0)
                 offset3 = (4 * i) + 4;
            }

            
            for(int i = 0; i < members->NumElements(); i++)
            {         
              Decl * member = members->Nth(i);
              VarDecl * varMember = dynamic_cast<VarDecl *>(member);
              if(varMember && strcmp(v1->GetId()->GetName(), varMember->GetId()->GetName()) == 0)
              { 
               Location * thisLoc = codeGen->ThisPtr;
               codeGen->GenStore(thisLoc, a2->GetLocationNode(), offset1);
                           
              }
              else if(varMember && leftField && strcmp(leftField->GetId()->GetName(), varMember->GetId()->GetName()) == 0)
              {
                Location * thisLoc = codeGen->ThisPtr;
                subscriptLoc = arraySubscript3->GetLocationNode();
              }
              else if(varMember && a2 && strcmp(varMember->GetId()->GetName(), a2->GetId()->GetName()) == 0)
              {
                Location * thisLoc = codeGen->ThisPtr;
                arrayLoc = codeGen->GenLoad(thisLoc, offset3);
              } 
           }


         }
           

        }

         
 
         //Location * subscriptLoc = GenLoadConstant(arraySubscript);
         Location * zeroLoc = codeGen->GenLoadConstant(0);
         FnDecl::numBytes+=4;

         Location * firstCheck = codeGen->GenBinaryOp("<", subscriptLoc, zeroLoc);
         FnDecl::numBytes+=4;  

         Location * arrayLength = codeGen->GenLoad(arrayLoc, -4);
         FnDecl::numBytes+=4;

         Location * secondCheck = codeGen->GenBinaryOp("<", subscriptLoc, arrayLength);
         FnDecl::numBytes+=4; 

         Location * equalCheck = codeGen->GenBinaryOp("==", secondCheck, zeroLoc);
         FnDecl::numBytes+=4;

         Location * orCheck = codeGen->GenBinaryOp("||", firstCheck, equalCheck);
         FnDecl::numBytes+=4;

         codeGen->GenIfZ(orCheck, firstLabel);

         //Executes if the subscript is bad
         Location * badSubscriptError = codeGen->GenLoadConstant(badSubscript); 
         FnDecl::numBytes+=4;

         codeGen->GenBuiltInCall(PrintString, badSubscriptError, NULL);
         
         codeGen->GenBuiltInCall(Halt, NULL, NULL);

         //At this point, the subscript is valid.
         codeGen->GenLabel(firstLabel);
         Location * fourLoc = codeGen->GenLoadConstant(4);
         Location * fourBytes = codeGen->GenBinaryOp("*", fourLoc, subscriptLoc);         
         Location * shiftLoc = codeGen->GenBinaryOp("+", arrayLoc, fourBytes);
         Location * finalLoc = codeGen->GenLoad(shiftLoc, 0);
         FnDecl::numBytes+=16;

         codeGen->GenAssign(varLoc, finalLoc);
       }
    }
  }
  else if (left2 && right6)//Case 5: The left side is a variable and the right side is a NewExpr
  {
     Decl * d1 = FindDecl(left2->GetId());
     VarDecl * v1 = dynamic_cast<VarDecl *>(d1);
     if(v1)
     {
       Location * varLoc = v1->GetLocationNode();
       Location * rightLoc;
       rightLoc = right6->GetLocationNode();
       codeGen->GenAssign(varLoc, rightLoc);
     }
  }
  else if(left2 && right7) //Case 6:  The left side is a variable and the right side is an arithmetic expression
  {
     Decl * d1 = FindDecl(left2->GetId());
     VarDecl * v1 = dynamic_cast<VarDecl *>(d1);
 

     if(v1)
     {
       Location * varLoc;
       Location * rightLoc = right->GetLocationNode();

       //Check for instance variable assignment.  This needs to be done sightly differenly than a normal
       //variable assignment
       Node * parent = v1->GetParent();
       ClassDecl * classParent = dynamic_cast<ClassDecl *>(parent);
       Program * prog = dynamic_cast<Program *>(parent);

       while(!classParent && !prog)
       {
          parent = parent->GetParent();
          classParent = dynamic_cast<ClassDecl *>(parent);
          prog = dynamic_cast<Program *>(parent);
       }

       if(classParent)
       {
          List<Decl *> *members = classParent->GetMembers();
          for(int i = 0; i < members->NumElements(); i++)
          {    
            Decl * member = members->Nth(i);
            VarDecl * var = dynamic_cast<VarDecl *>(member);
            if(var && strcmp(var->GetId()->GetName(), v1->GetId()->GetName()) == 0)
            {
               int offset = 4 *(i+1);    
               codeGen->GenStore(codeGen->ThisPtr, rightLoc, offset);
            }
          }
       }
       else
       {
         varLoc = v1->GetLocationNode();
         rightLoc = right7->GetLocationNode();
         codeGen->GenAssign(varLoc, rightLoc);
       }
   }
  }
  else if (left2)//Case 6: The left side is a variable and the right side is (probably) a constant
  {
     Decl * d1 = FindDecl(left2->GetId());
     VarDecl * v1 = dynamic_cast<VarDecl *>(d1);
 
     if(v1)
     {
       Location * varLoc;
       Location * rightLoc = right->GetLocationNode();


       //Check for instance variable assignment.  This needs to be done sightly differenly than a normal
       //variable assignment
       Node * parent = v1->GetParent();
       ClassDecl * classParent = dynamic_cast<ClassDecl *>(parent);
       Program * prog = dynamic_cast<Program *>(parent);

       while(!classParent && !prog)
       {
          parent = parent->GetParent();
          classParent = dynamic_cast<ClassDecl *>(parent);
          prog = dynamic_cast<Program *>(parent);
       }

       if(classParent)
       {
          List<Decl *> *members = classParent->GetMembers();
          for(int i = 0; i < members->NumElements(); i++)
          {    
            Decl * member = members->Nth(i);
            VarDecl * var = dynamic_cast<VarDecl *>(member);
            if(var && strcmp(var->GetId()->GetName(), v1->GetId()->GetName()) == 0)
            {
               int offset = 4 *(i+1);     
               codeGen->GenStore(codeGen->ThisPtr, rightLoc, offset);
            }
          }
 
       }
       else
       {
        varLoc = v1->GetLocationNode();
        codeGen->GenAssign(varLoc, rightLoc); 
       }
      
     
     }
  }
  else if(left3 && right2) //Left side is array access and right side is field
  {
     Expr * arrayBase = left3->GetBase();
     Expr * arraySubscript = left3->GetSubscript();
     IntConstant * arraySubscript2 = dynamic_cast<IntConstant *>(arraySubscript);
     FieldAccess * arraySubscript3 = dynamic_cast<FieldAccess *>(arraySubscript);

     FieldAccess * arrayBase2 = dynamic_cast<FieldAccess *>(arrayBase);
     

     //Handles the cases for array access 
     if(arrayBase2)
     {
       Decl * d2 = FindDecl(arrayBase2->GetId());
       VarDecl * a2 = dynamic_cast<VarDecl *>(d2);    

       if(a2 && arraySubscript)
       {
         Location * instanceArrayLoc;
         Location * instanceSubscriptLoc;

         Decl * subDecl = FindDecl(arraySubscript3->GetId());
         VarDecl * subVar = dynamic_cast<VarDecl *>(subDecl);

         //Check for instance variable assignment.  This needs to be done sightly differenly than a normal
         //variable assignment
         Node * parent = a2->GetParent();
         ClassDecl * classParent = dynamic_cast<ClassDecl *>(parent);
         Program * prog = dynamic_cast<Program *>(parent);
         
         while(!classParent && !prog)
         {
            parent = parent->GetParent();
            classParent = dynamic_cast<ClassDecl *>(parent);
            prog = dynamic_cast<Program *>(parent);
         }

         if(classParent)
         {
            List<Decl *> *members = classParent->GetMembers();
            for(int i = 0; i < members->NumElements(); i++)
            {     
              Decl * member = members->Nth(i);
              VarDecl * var = dynamic_cast<VarDecl *>(member);
              
              if(var && strcmp(var->GetId()->GetName(), a2->GetId()->GetName()) == 0)
              {    
                 int offset1 = 4 *(i+1);
                 instanceArrayLoc = codeGen->GenLoad(codeGen->ThisPtr, offset1);
              }
              if(var && arraySubscript3 && strcmp(var->GetId()->GetName(), arraySubscript3->GetId()->GetName()) == 0) 
              {
                 instanceSubscriptLoc = codeGen->GenLoad(codeGen->ThisPtr, 4*(i+1));
              
              } 
            }
 
         }          

         //Location * arrayLoc = a2->GetLocationNode();
         char * firstLabel = codeGen->NewLabel();
         
         const char * badSubscript = err_arr_out_of_bounds;

         Location * arrayLoc;

         if(classParent && instanceArrayLoc)
           arrayLoc = instanceArrayLoc;

         else
           arrayLoc = a2->GetLocationNode();

          
         //Check for illegal array subscript here
         Location * subscriptLoc;

         FieldAccess * f1 = dynamic_cast<FieldAccess *>(arraySubscript);  
 


         if(classParent && instanceSubscriptLoc)
         {
          subscriptLoc = instanceSubscriptLoc; 
         }
         else
         {
           //Array Subscript is either a constant or a variable.  Handle other expressions later
           if(arraySubscript2)
              subscriptLoc = codeGen->GenLoadConstant(arraySubscript2->GetValue());
           else if(f1)
           {          
             Decl * d = FindDecl(f1->GetId());
             VarDecl * v = dynamic_cast<VarDecl *>(d);
             if(v)
             {              
                subscriptLoc = v->GetLocationNode();
                
             }
           }
         }
        
          if(classParent && instanceArrayLoc)
           arrayLoc = instanceArrayLoc;

         else
           arrayLoc = a2->GetLocationNode();


         //Location * subscriptLoc = GenLoadConstant(arraySubscript);
         Location * zeroLoc = codeGen->GenLoadConstant(0);
         
         Location * firstCheck = codeGen->GenBinaryOp("<", subscriptLoc, zeroLoc); //SegFault here
          
         Location * arrayLength = codeGen->GenLoad(arrayLoc, -4);
         Location * secondCheck = codeGen->GenBinaryOp("<", subscriptLoc, arrayLength);
         
         Location * equalCheck = codeGen->GenBinaryOp("==", secondCheck, zeroLoc);
 
         Location * orCheck = codeGen->GenBinaryOp("||", firstCheck, equalCheck);

         codeGen->GenIfZ(orCheck, firstLabel);
         
         //Executes if the subscript is bad
         Location * badSubscriptError = codeGen->GenLoadConstant(badSubscript); 

         codeGen->GenBuiltInCall(PrintString, badSubscriptError, NULL);

         codeGen->GenBuiltInCall(Halt, NULL, NULL);

         //At this point, the subscript is valid.
         codeGen->GenLabel(firstLabel);
         Location * fourLoc = codeGen->GenLoadConstant(4);
         Location * fourBytes = codeGen->GenBinaryOp("*", fourLoc, subscriptLoc);         
         Location * shiftLoc = codeGen->GenBinaryOp("+", arrayLoc, fourBytes);
         
         Decl * d2 = FindDecl(right2->GetId());
   
         VarDecl * v2 = dynamic_cast<VarDecl *>(d2);
         
         Location * rightLoc = v2->GetLocationNode();
                
         codeGen->GenStore(shiftLoc, rightLoc, 0);
         
         //codeGen->GenAssign(finalLoc, rightLoc);
         FnDecl::numBytes+=44; 

       }
     }


  }
  else if(left3 && right3) //Left side is array access and right side is function.
  {
    Expr * arrayBase = left3->GetBase();
    Expr * arraySubscript = left3->GetSubscript();
    

  }
  else if(left3 && right4) //Left side is array access and right side is NewArrayExpr() for multi-dimensional arrays
  {
    
     Expr * arrayBase = left3->GetBase();
     Expr * arraySubscript = left3->GetSubscript();
     IntConstant * arraySubscript2 = dynamic_cast<IntConstant *>(arraySubscript);
     
     FieldAccess * arrayBase2 = dynamic_cast<FieldAccess *>(arrayBase);

     Location * rightLoc = right4->GetLocationNode(); 

   
     //Handles the cases for array access 
     if(arrayBase2)
     {
       Decl * d2 = FindDecl(arrayBase2->GetId());
       VarDecl * a2 = dynamic_cast<VarDecl *>(d2);    

       if(a2 && arraySubscript)
       {
         
         Location * arrayLoc = a2->GetLocationNode();
         char * firstLabel = codeGen->NewLabel();
        
          
         const char * badSubscript = err_arr_out_of_bounds;

         //Check for illegal array subscript here
         Location * subscriptLoc;   

         FieldAccess * f1 = dynamic_cast<FieldAccess *>(arraySubscript);    
         
         if(arraySubscript2)
            subscriptLoc = codeGen->GenLoadConstant(arraySubscript2->GetValue());
         else if(f1)
         {        
           Decl * d = FindDecl(f1->GetId());
           VarDecl * v = dynamic_cast<VarDecl *>(d);
           if(v)
           {              
              subscriptLoc = v->GetLocationNode();
           }
         }
         
         //Location * subscriptLoc = GenLoadConstant(arraySubscript);
         Location * zeroLoc = codeGen->GenLoadConstant(0);
         Location * firstCheck = codeGen->GenBinaryOp("<", subscriptLoc, zeroLoc);
         
         Location * arrayLength = codeGen->GenLoad(arrayLoc, -4);
         Location * secondCheck = codeGen->GenBinaryOp("<", subscriptLoc, arrayLength);

         Location * equalCheck = codeGen->GenBinaryOp("==", secondCheck, zeroLoc);
 
         Location * orCheck = codeGen->GenBinaryOp("||", firstCheck, equalCheck);

         codeGen->GenIfZ(orCheck, firstLabel);

         //Executes if the subscript is bad
         Location * badSubscriptError = codeGen->GenLoadConstant(badSubscript); 

         codeGen->GenBuiltInCall(PrintString, badSubscriptError, NULL);

         codeGen->GenBuiltInCall(Halt, NULL, NULL);

         //At this point, the subscript is valid.
         codeGen->GenLabel(firstLabel);
         Location * fourLoc = codeGen->GenLoadConstant(4);
         Location * fourBytes = codeGen->GenBinaryOp("*", fourLoc, subscriptLoc);         
         Location * shiftLoc = codeGen->GenBinaryOp("+", arrayLoc, fourBytes);
         //Location * finalLoc = codeGen->GenLoad(shiftLoc, 0);
         FnDecl::numBytes+=40;


         codeGen->GenStore(shiftLoc,rightLoc, 0);
         //codeGen->GenAssign(finalLoc, rightLoc);
       }
    }
   

  }
  else if(left3 && right5) //Left side is array access and right side is also an array access 
  {
     Expr * arrayBase = left3->GetBase();
     Expr * secondArrayBase = right5->GetBase();
     Expr * arraySubscript = left3->GetSubscript();
     Expr * secondArraySubscript = right5->GetSubscript();

     IntConstant * arraySubscript2 = dynamic_cast<IntConstant *>(arraySubscript);
     IntConstant * arraySubscript3 = dynamic_cast<IntConstant *>(secondArraySubscript);

     FieldAccess * arrayBase2 = dynamic_cast<FieldAccess *>(arrayBase);
     FieldAccess * arrayBase3 = dynamic_cast<FieldAccess *>(secondArrayBase);

     char * firstLabel = codeGen->NewLabel();
     char * secondLabel = codeGen->NewLabel();

     //Handles the cases for array access 
     if(arrayBase2 && arrayBase3)
     {
       Decl * d2 = FindDecl(arrayBase2->GetId());
       VarDecl * a2 = dynamic_cast<VarDecl *>(d2);    

         
         Location * arrayLoc = a2->GetLocationNode();
         //char * firstLabel = codeGen->NewLabel();
         
         const char * badSubscript = err_arr_out_of_bounds;

         //Check for illegal array subscript here
         Location * subscriptLoc;   

         FieldAccess * f1 = dynamic_cast<FieldAccess *>(arraySubscript);    

         //Array Subscript is either a constant or a variable.  Handle other expressions later
         if(arraySubscript2)
         {
            subscriptLoc = codeGen->GenLoadConstant(arraySubscript2->GetValue());
            FnDecl::numBytes+=4;
         }
         else if(f1)
         {        
           Decl * d = FindDecl(f1->GetId());
           VarDecl * v = dynamic_cast<VarDecl *>(d);
           if(v)
           {              
              subscriptLoc = v->GetLocationNode();
           }
         }
         
         //Location * subscriptLoc = GenLoadConstant(arraySubscript);
         Location * zeroLoc = codeGen->GenLoadConstant(0);
         Location * firstCheck = codeGen->GenBinaryOp("<", subscriptLoc, zeroLoc);
         
         Location * arrayLength = codeGen->GenLoad(arrayLoc, -4);
         Location * secondCheck = codeGen->GenBinaryOp("<", subscriptLoc, arrayLength);

         Location * equalCheck = codeGen->GenBinaryOp("==", secondCheck, zeroLoc);
 
         Location * orCheck = codeGen->GenBinaryOp("||", firstCheck, equalCheck);

         codeGen->GenIfZ(orCheck, firstLabel);

         //Executes if the subscript is bad
         Location * badSubscriptError = codeGen->GenLoadConstant(badSubscript); 

         codeGen->GenBuiltInCall(PrintString, badSubscriptError, NULL);

         codeGen->GenBuiltInCall(Halt, NULL, NULL);

         codeGen->GenLabel(firstLabel);    

                  //codeGen->GenAssign(finalLoc, rightLoc);
         

         Location * fourLoc = codeGen->GenLoadConstant(4);

         Location * fourLoc2 = codeGen->GenBinaryOp("*", fourLoc, subscriptLoc);
    
         Location * firstElemLoc = codeGen->GenBinaryOp("+", a2->GetLocationNode(), fourLoc2);


    
         Decl * d3 = FindDecl(arrayBase2->GetId());
         VarDecl * a3 = dynamic_cast<VarDecl *>(d3);    

       
       
         
         Location * arrayLoc2 = a3->GetLocationNode();
         //char * firstLabel = codeGen->NewLabel();
         
         

         //Check for illegal array subscript here
         Location * subscriptLoc2;   

         FieldAccess * f2 = dynamic_cast<FieldAccess *>(secondArraySubscript);    

         //Array Subscript is either a constant or a variable.  Handle other expressions later
         if(arraySubscript3)
         {
            subscriptLoc2 = codeGen->GenLoadConstant(arraySubscript3->GetValue());
            FnDecl::numBytes+=4;
         }
         else if(f2)
         {        
           Decl * d = FindDecl(f2->GetId());
           VarDecl * v = dynamic_cast<VarDecl *>(d);
           if(v)
           {              
              subscriptLoc2 = v->GetLocationNode();
           }
         }
         
         //Location * subscriptLoc = GenLoadConstant(arraySubscript);
         Location * zeroLoc2 = codeGen->GenLoadConstant(0);
         Location * firstCheck2 = codeGen->GenBinaryOp("<", subscriptLoc2, zeroLoc2);
         
         Location * arrayLength2 = codeGen->GenLoad(arrayLoc, -4);
         Location * secondCheck2 = codeGen->GenBinaryOp("<", subscriptLoc2, arrayLength2);

         Location * equalCheck2 = codeGen->GenBinaryOp("==", secondCheck2, zeroLoc2);
 
         Location * orCheck2 = codeGen->GenBinaryOp("||", firstCheck2, equalCheck2);

         codeGen->GenIfZ(orCheck2, secondLabel);

         //Executes if the subscript is bad
         Location * badSubscriptError2 = codeGen->GenLoadConstant(badSubscript); 

         codeGen->GenBuiltInCall(PrintString, badSubscriptError2, NULL);

         codeGen->GenBuiltInCall(Halt, NULL, NULL);

         codeGen->GenLabel(secondLabel);

                  //codeGen->GenAssign(finalLoc, rightLoc);
         
        //At this point, the subscript is valid.;
         Location * fourLoc3 = codeGen->GenLoadConstant(4);
         Location * fourBytes = codeGen->GenBinaryOp("*", fourLoc3, subscriptLoc2);         
         Location * shiftLoc = codeGen->GenBinaryOp("+", arrayLoc, fourBytes);


         Location * loadLoc = codeGen->GenLoad(shiftLoc, 0);

         codeGen->GenStore(firstElemLoc, loadLoc, 0);         

         /*Decl * d2 = FindDecl(right2->GetId());
   
         VarDecl * v2 = dynamic_cast<VarDecl *>(d2);

         
         Location * rightLoc = v2->GetLocationNode();
            
                 
         codeGen->GenStore(shiftLoc, rightLoc, 0);*/
         FnDecl::numBytes+=88;

      



    }
  
  }
  else if(left3) //Left side is array access and right side is (probably) a constant
  {
     Expr * arrayBase = left3->GetBase();
     Expr * arraySubscript = left3->GetSubscript();
     IntConstant * arraySubscript2 = dynamic_cast<IntConstant *>(arraySubscript);
      
     FieldAccess * arrayBase2 = dynamic_cast<FieldAccess *>(arrayBase);
     
     //Used for multi-dimensional arrays.  This is for the first array index (i.e. d[i] in d[i][j])
     ArrayAccess * firstArrayBase = dynamic_cast<ArrayAccess *>(arrayBase);
     
 
     //Handles the cases for normal array access 
     if(arrayBase2)
     {
       Decl * d2 = FindDecl(arrayBase2->GetId());
       VarDecl * a2 = dynamic_cast<VarDecl *>(d2);    

       if(a2 && arraySubscript)
       {
         
         Location * arrayLoc = a2->GetLocationNode();
         char * firstLabel = codeGen->NewLabel();
         
         const char * badSubscript = err_arr_bad_size;

         //Check for illegal array subscript here
         Location * subscriptLoc;   

         FieldAccess * f1 = dynamic_cast<FieldAccess *>(arraySubscript);    

         //Array Subscript is either a constant or a variable.  Handle other expressions later
         if(arraySubscript2)
            subscriptLoc = codeGen->GenLoadConstant(arraySubscript2->GetValue());
         else if(f1)
         {        
           Decl * d = FindDecl(f1->GetId());
           VarDecl * v = dynamic_cast<VarDecl *>(d);
           if(v)
           {              
              subscriptLoc = v->GetLocationNode();
           }
         }
         
         //Location * subscriptLoc = GenLoadConstant(arraySubscript);
         Location * zeroLoc = codeGen->GenLoadConstant(0);
         Location * firstCheck = codeGen->GenBinaryOp("<", subscriptLoc, zeroLoc);
         
         Location * arrayLength = codeGen->GenLoad(arrayLoc, -4);
         Location * secondCheck = codeGen->GenBinaryOp("<", subscriptLoc, arrayLength);

         Location * equalCheck = codeGen->GenBinaryOp("==", secondCheck, zeroLoc);
 
         Location * orCheck = codeGen->GenBinaryOp("||", firstCheck, equalCheck);

         codeGen->GenIfZ(orCheck, firstLabel);

         //Executes if the subscript is bad
         Location * badSubscriptError = codeGen->GenLoadConstant(badSubscript); 

         codeGen->GenBuiltInCall(PrintString, badSubscriptError, NULL);

         codeGen->GenBuiltInCall(Halt, NULL, NULL);

         //At this point, the subscript is valid.
         codeGen->GenLabel(firstLabel);
         Location * fourLoc = codeGen->GenLoadConstant(4);
         Location * fourBytes = codeGen->GenBinaryOp("*", fourLoc, subscriptLoc);         
         Location * shiftLoc = codeGen->GenBinaryOp("+", arrayLoc, fourBytes);
 
         Location * rightLoc = right->GetLocationNode(); 
 
                   
         codeGen->GenStore(shiftLoc, rightLoc, 0);

         FnDecl::numBytes+=44;       


         //codeGen->GenAssign(finalLoc, rightLoc);
       }
      }
       else if(firstArrayBase) //Used for multi-dimensional arrays
       {
         Expr * baseExpr = firstArrayBase->GetBase();
         Expr * subscript = firstArrayBase->GetSubscript(); //The first of the two subscripts
         FieldAccess * baseField = dynamic_cast<FieldAccess *>(baseExpr);
         Decl * d2 = FindDecl(baseField->GetId());
         VarDecl * a2 = dynamic_cast<VarDecl *>(d2);    

         if(a2 && subscript)
         {
         
           Location * arrayLoc = a2->GetLocationNode();
           char * firstLabel = codeGen->NewLabel();
           char * secondLabel = codeGen->NewLabel();           

           const char * badSubscript = err_arr_bad_size;

           //Check for illegal array subscript here
           Location * subscriptLoc;   

           IntConstant * intSub = dynamic_cast<IntConstant *>(subscript); 
           FieldAccess * f1 = dynamic_cast<FieldAccess *>(subscript);    

           //Array Subscript is either a constant or a variable.  Handle other expressions later
           if(intSub)
           {   
             subscriptLoc = codeGen->GenLoadConstant(intSub->GetValue());  
           }
           else if(f1)
           {        
             Decl * d = FindDecl(f1->GetId());
             VarDecl * v = dynamic_cast<VarDecl *>(d);
             if(v)
             {              
                subscriptLoc = v->GetLocationNode();
             }
           }
         
         //Location * subscriptLoc = GenLoadConstant(arraySubscript);
         Location * zeroLoc = codeGen->GenLoadConstant(0);
         Location * firstCheck = codeGen->GenBinaryOp("<", subscriptLoc, zeroLoc);
         
         Location * arrayLength = codeGen->GenLoad(arrayLoc, -4);
         Location * secondCheck = codeGen->GenBinaryOp("<", subscriptLoc, arrayLength);

         Location * equalCheck = codeGen->GenBinaryOp("==", secondCheck, zeroLoc);
 
         Location * orCheck = codeGen->GenBinaryOp("||", firstCheck, equalCheck);

         codeGen->GenIfZ(orCheck, firstLabel);

         //Executes if the subscript is bad
         Location * badSubscriptError = codeGen->GenLoadConstant(badSubscript); 

         codeGen->GenBuiltInCall(PrintString, badSubscriptError, NULL);

         codeGen->GenBuiltInCall(Halt, NULL, NULL);

         //At this point, the subscript is valid.
         codeGen->GenLabel(firstLabel);
         Location * fourLoc = codeGen->GenLoadConstant(4);
         Location * fourBytes = codeGen->GenBinaryOp("*", fourLoc, subscriptLoc);         
         Location * shiftLoc = codeGen->GenBinaryOp("+", arrayLoc, fourBytes);
 
         //Location * rightLoc = right->GetLocationNode(); 
 
         //Location for first array index                   
         Location * firstSubLoc = codeGen->GenLoad(shiftLoc, 0);

         FnDecl::numBytes+=44;

         //From this point, we get the Location for the second array index

         intSub = dynamic_cast<IntConstant *>(arraySubscript);
         f1 = dynamic_cast<FieldAccess *>(arraySubscript);         
      
         //Array Subscript is either a constant or a variable.  Handle other expressions later
         if(intSub)
         {   
           subscriptLoc = codeGen->GenLoadConstant(intSub->GetValue());  
           FnDecl::numBytes+=4;
         }
         else if(f1)
         {        
           Decl * d = FindDecl(f1->GetId());
           VarDecl * v = dynamic_cast<VarDecl *>(d);
           if(v)
           {              
             subscriptLoc = v->GetLocationNode();
           }
         }
         

         zeroLoc = codeGen->GenLoadConstant(0);
         firstCheck = codeGen->GenBinaryOp("<", subscriptLoc, zeroLoc);
         
         arrayLength = codeGen->GenLoad(arrayLoc, -4);
         secondCheck = codeGen->GenBinaryOp("<", subscriptLoc, arrayLength);

         equalCheck = codeGen->GenBinaryOp("==", secondCheck, zeroLoc);
 
         orCheck = codeGen->GenBinaryOp("||", firstCheck, equalCheck);

         codeGen->GenIfZ(orCheck, secondLabel);

         //Executes if the subscript is bad
         badSubscriptError = codeGen->GenLoadConstant(badSubscript); 

         codeGen->GenBuiltInCall(PrintString, badSubscriptError, NULL);

         codeGen->GenBuiltInCall(Halt, NULL, NULL);

         //At this point, the subscript is valid.
         codeGen->GenLabel(secondLabel);
         fourLoc = codeGen->GenLoadConstant(4);
         fourBytes = codeGen->GenBinaryOp("*", fourLoc, subscriptLoc);     
    
         shiftLoc = codeGen->GenBinaryOp("+", firstSubLoc, fourBytes);
 
         Location * rightLoc = right->GetLocationNode();
 
         codeGen->GenStore(shiftLoc, rightLoc, 0);
            
         FnDecl::numBytes+=40;
       }
    }
 }
}
ArrayAccess::ArrayAccess(yyltype loc, Expr *b, Expr *s) : LValue(loc) {
    (base=b)->SetParent(this); 
    (subscript=s)->SetParent(this);
}

void ArrayAccess::Emit() {

  base->Emit();
  subscript->Emit();

}

     
FieldAccess::FieldAccess(Expr *b, Identifier *f) 
  : LValue(b? Join(b->GetLocation(), f->GetLocation()) : *f->GetLocation()) {
    Assert(f != NULL); // b can be be NULL (just means no explicit base)
    base = b; 
    if (base) base->SetParent(this); 
    (field=f)->SetParent(this);
}

void FieldAccess::Emit()
{
  //Gets current offset, constructs a new Location for the field (if there isn't one present) and updates the offset.
  int offset = codeGen->OffsetToCurrentLocal;
  //int offset2 = codeGen->OffsetToCurrentParam;

  Decl * d = FindDecl(this->field);
  VarDecl * v = dynamic_cast<VarDecl *>(d);
  if(v && !(v->varLoc))
  { 
         
    v->varLoc = new Location(fpRelative, offset, this->field->GetName());
    FnDecl::numBytes+=4;
    codeGen->OffsetToCurrentLocal -= 4;

    //v->varLoc = new Location(fpRelative, offset2, this->field->GetName());
    //codeGen->OffsetToCurrentParam += 4;

  }


  This * baseThis = dynamic_cast<This *>(base);

 
  if(base && baseThis)
  {
   
    Node * parent = this->GetParent();
    ClassDecl * classParent = dynamic_cast<ClassDecl *>(parent);
    Program * prog = dynamic_cast<Program *>(parent);

    while(!classParent && !prog)
    {
      parent = parent->GetParent();
      classParent = dynamic_cast<ClassDecl *>(parent);
      prog = dynamic_cast<Program *>(parent);
    }

    if(classParent)
    {
      int varCount = 0;
 
      List <Decl *> *members = classParent->GetMembers();
      for(int i = 0; i < members->NumElements(); i++)
      {
        Decl * member = members->Nth(i);
        VarDecl * varMember = dynamic_cast<VarDecl *>(member);
        if(varMember)
         varCount++; 

      }

      varCount *= 4;
      
      List<VarDecl *> *varMembers = new List<VarDecl *>();

      int offset = 0; //Offset into list of instance variables
      for(int i = 0; i < members->NumElements(); i++)
      {
        Decl * member = members->Nth(i);
        VarDecl * varMember = dynamic_cast<VarDecl *>(member);
        if(varMember) 
           varMembers->Append(varMember);
      }
      

      for(int i = 0; i < varMembers->NumElements(); i++)
      {
        VarDecl * varMember = varMembers->Nth(i);
        if(strcmp(varMember->GetId()->GetName(), field->GetName()) == 0)
            offset = (4 * i) + 4;
      }

      for(int i = 0; i < members->NumElements(); i++)
      {
        Decl * member = members->Nth(i);
        VarDecl * varMember = dynamic_cast<VarDecl *>(member);
        if(varMember && strcmp(this->GetId()->GetName(), varMember->GetId()->GetName()) == 0)
        { 
          varMember->varLoc = codeGen->GenLoad(codeGen->ThisPtr, offset); 
          FnDecl::numBytes+=4; 
        }  
      }
    }   
  }
}


Call::Call(yyltype loc, Expr *b, Identifier *f, List<Expr*> *a) : Expr(loc)  {
    Assert(f != NULL && a != NULL); // b can be be NULL (just means no explicit base)
    base = b;
    if (base) base->SetParent(this);
    (field=f)->SetParent(this);
    (actuals=a)->SetParentAll(this);
}

void Call::Emit()
{
  
  Location * varLoc;
  Location * fnLoc;
  
  //Check for array.length() here and emit the appropriate code
   
  if(base)
  {
    FieldAccess * baseField = dynamic_cast<FieldAccess *>(base);
    
    if(baseField)
    {
      Identifier * baseIdent = baseField->GetId();      
      Decl * baseDecl = FindDecl(baseIdent);
      VarDecl * v = dynamic_cast<VarDecl *>(baseDecl);
      if(v)
      {
         Type * varType = v->GetDeclaredType(); 
         ArrayType * arrayType = dynamic_cast<ArrayType *>(varType);
         if(arrayType && strcmp(field->GetName(), "length") == 0)
         {
           /*std::stringstream ss;
           ss << arrayType;
           std::string name = ss.str();
           const char * stringName = name.c_str();//Should be something like "int[]"*/

           
           Location * arrayLoc = v->GetLocationNode(); //Get location of array in the stack
           Location * lengthLoc = codeGen->GenLoad(arrayLoc, -4); //Get location of the length of the array
           FnDecl::numBytes+=4;
           v->arrayLengthLoc = lengthLoc; //Store the length of the array
           //codeGen->GenBuiltInCall(PrintInt, lengthLoc, NULL); //Generate the code to print the array length
           //printf("The string is %s\n", name.c_str());
         }
      }
    }
  }
  //Emit code for the actual parameters  
  /*for(int i = actuals->NumElements() - 1; i >= 0; i--)
    actuals->Nth(i)->Emit();*/
   for(int i = 0; i < actuals->NumElements(); i++)
    actuals->Nth(i)->Emit();
 
  
  int paramCounter = 4 * actuals->NumElements();//Used for PopParams instruction


  //For method calls, in order to load the method, we need to set up a pointer to the vtable (using the one that already exists) and
  //get a pointer to the function that we want (which is stored in the vtable).
  if(base)
  {
    FieldAccess * baseField2 = dynamic_cast<FieldAccess *>(base);
    if(baseField2)
    {    
      Decl * baseDecl = FindDecl(baseField2->GetId());
      VarDecl * baseVar = dynamic_cast<VarDecl *>(baseDecl);
      NamedType * nt = dynamic_cast<NamedType *> (baseVar->GetDeclaredType());
     
      if(nt)
      { 
        Decl * classDecl = FindDecl(nt->GetId());
        ClassDecl * classDecl2 = dynamic_cast<ClassDecl *>(classDecl);       
        List<Decl *> *members = classDecl2->GetMembers();
        List<FnDecl *> *fnMembers = new List<FnDecl *>();
        int offset = 0; //Offset in the vtable for the calling function

        for(int i = 0; i < members->NumElements(); i++)
        {
          Decl * member = members->Nth(i);
          FnDecl * fnMember = dynamic_cast<FnDecl *>(member);
          if(fnMember)
              fnMembers->Append(fnMember);
        }

        for(int i = 0; i < fnMembers->NumElements(); i++)
        { 
           FnDecl * fnMember = fnMembers->Nth(i);
           if(strcmp(fnMember->GetId()->GetName(), field->GetName()) == 0) 
             offset = 4 * i;

        }

        varLoc = codeGen->GenLoad(baseVar->GetLocationNode(), 0);
        fnLoc = codeGen->GenLoad(varLoc, offset);  //Change this to access other functions
        FnDecl::numBytes+=8;
      }
    }
  }
 
  Location * thisLoc;
  Location * fnLoc2;
  bool hasReturn;
  bool isLocalFunction = false;

   
  //Push parameters onto the stack in reverse order
  for(int i = actuals->NumElements() - 1; i >= 0; i--)
  {
    Location * tempLoc;
    FieldAccess * f = dynamic_cast<FieldAccess *>(actuals->Nth(i));
    IntConstant * ic = dynamic_cast<IntConstant *>(actuals->Nth(i));
    StringConstant * sc = dynamic_cast<StringConstant *>(actuals->Nth(i));
    BoolConstant * bc = dynamic_cast<BoolConstant *>(actuals->Nth(i));
    Call * fc = dynamic_cast<Call *>(actuals->Nth(i));
    ArrayAccess * a = dynamic_cast<ArrayAccess *>(actuals->Nth(i));
    ReadLineExpr * rle = dynamic_cast<ReadLineExpr *>(actuals->Nth(i));
    ReadIntegerExpr * rie = dynamic_cast<ReadIntegerExpr *>(actuals->Nth(i));
    LogicalExpr * le = dynamic_cast<LogicalExpr *>(actuals->Nth(i));
    RelationalExpr *re = dynamic_cast<RelationalExpr *>(actuals->Nth(i));
    ArithmeticExpr *ae = dynamic_cast<ArithmeticExpr *>(actuals->Nth(i));

    //  If the argument is a FieldAccess, use the Location Node in the corresponding variable declaration.  Otherwise,
    //proceed normally.
    if(f)
    {
      Decl * d = FindDecl(f->GetId());
      VarDecl * v = dynamic_cast<VarDecl *>(d);
      tempLoc = v->GetLocationNode();
      
    }
    else if(ic)
    {
      //tempLoc = codeGen->GenLoadConstant(ic->GetValue());   
      tempLoc = ic->GetLocationNode();   
    }
    else if(sc)
    {
       //tempLoc = codeGen->GenLoadConstant(sc->GetValue());
       tempLoc = sc->GetLocationNode();
    }
    else if(bc)
    {
       //tempLoc = codeGen->GenLoadConstant(bc->GetValue());
       tempLoc = bc->GetLocationNode();
    }
    else if(fc)
    {
       Decl * d = FindDecl(fc->GetId());
       FnDecl * fn = dynamic_cast<FnDecl *>(d);
            

       if(fc->GetLocationNode() != NULL)
         tempLoc = fc->GetLocationNode();
       else if(fn->GetLocationNode() != NULL)
         tempLoc = fn->GetLocationNode(); 
    }
    else if(a)
    {
       //printf("I am an array\n");
    }
    else if(rle)
    {
       //printf("I am reading the line\n");
       tempLoc = rle->GetLocationNode(); 

    }
    else if(rie)
    {
       //printf("I am reading the integers\n");
       tempLoc = rie->GetLocationNode();
       
    }
    else if(ae)
    {
       tempLoc = ae->GetLocationNode();
    }
    else if(re)
    {
       tempLoc = re->GetLocationNode();
    }
    else if(le)
    {
        //printf("I am a logical expression\n");
        tempLoc = le->GetLocationNode();

    }
    else
    {   
       //printf("I am a NULL parameter\n");
       tempLoc = actuals->Nth(i)->GetLocationNode(); //Change this for more cases later
       //printf("Yes you are\n");
    }

    //Check to see if you're calling a function locally from inside a class.  In other words, this part accounts
    //for calling a function from inside another function of the same class.

    Node * parent = this->GetParent();
    ClassDecl * classParent = dynamic_cast<ClassDecl *>(parent);
    Program * prog = dynamic_cast<Program *>(parent);

   
    while(!classParent && !prog)
    {
      parent = parent->GetParent();
      classParent = dynamic_cast<ClassDecl *>(parent);
      prog = dynamic_cast<Program *>(parent);
    }

    const char * fnName = this->field->GetName();
    int offset = 0;
    List<FnDecl *> *fnList = new List<FnDecl *>();

    if(classParent && i == actuals->NumElements() - 1)
    {
          List<Decl *> *members = classParent->GetMembers();
          for(int i = 0; i < members->NumElements(); i++)
          {    
            Decl * member = members->Nth(i);
            FnDecl * fn = dynamic_cast<FnDecl *>(member);
            if(fn && strcmp(fn->GetId()->GetName(), fnName) == 0)
            {
               fnList->Append(fn);
               Type * returnType = fn->GetType();
               char * returnType2 = returnType->GetName();
               if(strcmp(returnType2, "void") != 0)
                 hasReturn = true;
               else
                 hasReturn = false;
               thisLoc = codeGen->GenLoad(codeGen->ThisPtr, 0); //Pointer to "this" reference     
               FnDecl::numBytes+=4;
            }
          }
          for(int i = 0; i < fnList->NumElements(); i++)
          {    
            Decl * member = fnList->Nth(i);
            FnDecl * fn = dynamic_cast<FnDecl *>(member);
            if(fn && strcmp(fn->GetId()->GetName(), fnName) == 0)
            {
              fnLoc2 = codeGen->GenLoad(thisLoc, 4*(i+1)); //Get location of function
              FnDecl::numBytes+=4; 
              codeGen->GenPushParam(tempLoc);
            }
          }

    }
    else
    {
      codeGen->GenPushParam(tempLoc); 
    }
  }
   
  Node * parent = this->GetParent();
  ClassDecl * classParent = dynamic_cast<ClassDecl *>(parent);
  Program * prog = dynamic_cast<Program *>(parent);

  while(!classParent && !prog)
  {
      parent = parent->GetParent();
      classParent = dynamic_cast<ClassDecl *>(parent);
      prog = dynamic_cast<Program *>(parent);
  }

  if(classParent && fnLoc2) 
  {
     //printf("I should not be here\n");

     codeGen->GenPushParam(codeGen->ThisPtr);
     codeGen->GenACall(fnLoc2, hasReturn);
     isLocalFunction = true;

  }

  

  //For method calls, we need to put the "this" reference onto the stack as well 
  if(base)
  {
    FieldAccess * baseField2 = dynamic_cast<FieldAccess *>(base);
    if(baseField2)
    {    
      Decl * baseDecl = FindDecl(baseField2->GetId());
      VarDecl * baseVar = dynamic_cast<VarDecl *>(baseDecl);

      if(baseVar)
      {        
        Type * baseType = baseVar->GetDeclaredType();
        ArrayType * baseArray = dynamic_cast<ArrayType *>(baseType);

        //Prevents an ACall from being generated when the array index is something like d[d[0]].
        //if(strcmp(baseVar->GetId()->GetName(), field->GetName()) != 0)

        if(!baseArray) //Do not put the "this" reference when the base is an array (this may happen if you call the .length() method)
        {
          Location *thisLoc = new Location(fpRelative, 4, baseVar->GetLocationNode()->GetName());         
          codeGen->GenPushParam(thisLoc);
          paramCounter += 4;
          codeGen->OffsetToCurrentParam += 4; 
          codeGen->GenACall(fnLoc, false); //May need to change for method calls that do have a return value
        }
      }
    }
  }

  Decl * d = FindDecl(field);
  FnDecl * fn = dynamic_cast<FnDecl *>(d);
  //printf("Checkpoint 3\n");
  if(fn && !isLocalFunction)
  {
     //Generate LCall instruction for both return type and no return type cases
     
     Type * returnType = fn->GetType(); 
     ArrayType * arrayType = dynamic_cast<ArrayType *>(returnType);
     const char * returnTypeName;
     if(arrayType)      
        returnTypeName = arrayType->GetName();
     else
        returnTypeName = returnType->GetName();



     if(strcmp(returnTypeName, "void") != 0)
     {
       char temp[10];
       char * fnLabel;
       sprintf(temp, "_%s", field->GetName()); 
       fnLabel = temp;
       Location * fnLoc;
       fnLoc = codeGen->GenLCall(fnLabel, true);
       fn->fnDeclLoc = fnLoc;
       FnDecl::numBytes += 4;
     }
     else
     {
       char temp[10];
       char * fnLabel;
       sprintf(temp, "_%s", field->GetName());
       fnLabel = temp;
       Location * fnLoc;
       fnLoc = codeGen->GenLCall(fnLabel, false);
       FnDecl::numBytes+=4;
       fn->fnDeclLoc = fnLoc;
     }
     

  }

  //Pop parameters off the stack
  codeGen->GenPopParams(paramCounter);
  codeGen->OffsetToCurrentParam = 8;
   
}

NewExpr::NewExpr(yyltype loc, NamedType *c) : Expr(loc) { 
  Assert(c != NULL);
  (cType=c)->SetParent(this);
}

void NewExpr::Emit()
{
  //Gets the class declaration information
  Identifier * objId = cType->GetId();
  Decl * class1 = FindDecl(objId); 
  ClassDecl * class2 = dynamic_cast<ClassDecl *>(class1); 
  int classCounter = 4;//Used to count the number of instance variables in the class (plus the "this" reference)

  //For each instance variable, we add 4 to the classCounter.
  if(class2)
  {
    List<Decl *> *members = class2->GetMembers(); 
    for(int i = 0; i < members->NumElements(); i++)
    {
       Decl * classMember = members->Nth(i); 
       VarDecl * classVar = dynamic_cast<VarDecl *>(classMember);
       if(classVar)
          classCounter += 4;
    }
  }

  //Load the classCounter and allocate dynamic memory for the New() call
  Location * classLoc = codeGen->GenLoadConstant(classCounter);
  FnDecl::numBytes+=4;
  Location * allocLoc = codeGen->GenBuiltInCall(Alloc, classLoc, NULL); 
  FnDecl::numBytes+=4;

  //Load label for the class's vtable and set the vtable pointer
  Location * classLabel = codeGen->GenLoadLabel(objId->GetName());
  FnDecl::numBytes+=4;
 
  codeGen->GenStore(allocLoc, classLabel, 0);
      
  
  this->newLoc = allocLoc; 


}

NewArrayExpr::NewArrayExpr(yyltype loc, Expr *sz, Type *et) : Expr(loc) {
    Assert(sz != NULL && et != NULL);
    (size=sz)->SetParent(this); 
    (elemType=et)->SetParent(this);
}

void NewArrayExpr::Emit()
{
  size->Emit();
  
  Location * zeroLoc = codeGen->GenLoadConstant(0);
  FnDecl::numBytes+=4;
   
  Location *arraySizeLoc;

  //  The size may be either an integer constant or a integer variable. In the latter,
  //we need to get the corresponding variable declaration and use its location node.

  IntConstant * sizeInt = dynamic_cast<IntConstant*>(size);
  ArithmeticExpr * sizeArith = dynamic_cast<ArithmeticExpr *>(size);  

  if(sizeInt || sizeArith)
  {
    arraySizeLoc = size->GetLocationNode();
  }
  else
  {
     FieldAccess * sizeVar = dynamic_cast<FieldAccess *>(size);
     Decl * d = FindDecl(sizeVar->GetId());
     VarDecl * v = dynamic_cast<VarDecl *>(d);
     if(v)
     {
       arraySizeLoc = v->GetLocationNode();
     }
  }
  
  


  Location * negativeLoc = codeGen->GenBinaryOp("<", arraySizeLoc, zeroLoc);
  FnDecl::numBytes+=4;
  //codeGen->OffsetToCurrentLocal -= 4;
  

  char * firstLabel = codeGen->NewLabel();
  const char * negativeIndex = err_arr_bad_size;
  //printf("Checkpoint0\n");

  //Generate the error message for a negative array index in NewArrayExpr()
  codeGen->GenIfZ(negativeLoc, firstLabel);
  Location * errorLoc = codeGen->GenLoadConstant(negativeIndex);
  FnDecl::numBytes+=4;
  codeGen->OffsetToCurrentLocal -= 4;
  codeGen->GenBuiltInCall(PrintString, errorLoc, NULL);
  codeGen->GenBuiltInCall(Halt, NULL, NULL);

  //printf("Checkpoint1\n");
  //If the program gets to this point, it has a valid array size
  codeGen->GenLabel(firstLabel);  

  //Prepend the array length to the array
  Location * oneLoc = codeGen->GenLoadConstant(1);
  FnDecl::numBytes+=4;
  Location * sizeLoc = codeGen->GenBinaryOp("+", oneLoc, arraySizeLoc);
  FnDecl::numBytes+=4;
  //Multiply by four bytes
  Location * fourLoc = codeGen->GenLoadConstant(4); 
  FnDecl::numBytes+=4;
  Location * fourBytes = codeGen->GenBinaryOp("*", sizeLoc, fourLoc);
  FnDecl::numBytes+=4;
  //Allocates memory for the array
  
  Location * arrayAlloc = codeGen->GenBuiltInCall(Alloc, fourBytes, NULL);
  
  FnDecl::numBytes+=4;

  //Pointer at the 0th location points to the length of the array
  codeGen->GenStore(arrayAlloc, arraySizeLoc, 0);

  //Shift by 4 bytes to the start of the array contents (the data after the length)
  Location * arrayStart = codeGen->GenBinaryOp("+", arrayAlloc, fourLoc);
  FnDecl::numBytes+=4;


  this->arrayLoc = arrayStart;  
    
}

void ReadIntegerExpr::Emit()
{
  this->readIntegerLoc = codeGen->GenBuiltInCall(ReadInteger, NULL, NULL);
   FnDecl::numBytes+=4;
}

void ReadLineExpr::Emit()
{
  this->readLineLoc = codeGen->GenBuiltInCall(ReadLine, NULL, NULL);  
  FnDecl::numBytes+=4;
}       
