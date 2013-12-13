/* File: ast_stmt.cc
 * -----------------
 * Implementation of statement node classes.
 */
#include "ast_stmt.h"
#include "ast_type.h"
#include "ast_decl.h"
#include "ast_expr.h"
#include "scope.h"
#include "errors.h"
#include "codegen.h"
#include "stdio.h"
#include "stdlib.h"


extern CodeGenerator *codeGen;


Program::Program(List<Decl*> *d) {
    Assert(d != NULL);
    (decls=d)->SetParentAll(this);
}

void Program::Check() {
    nodeScope = new Scope();
    decls->DeclareAll(nodeScope);
    decls->CheckAll();
}

void Program::Emit() {
    if(decls->NumElements() == 0)
    {
      ReportError::NoMainFound();
      return;
    }
    else
    {
      bool isMain = false;
      for(int i = 0; i < decls->NumElements(); i++)
      {
        Decl * d = decls->Nth(i);
        FnDecl * fn = dynamic_cast<FnDecl *>(d);
        if(fn)
        {        
          if(strcmp(FindDecl(fn->GetId())->GetName(), "main") == 0)
             isMain = true;
        }
      }
      if(!isMain)
      {
        ReportError::NoMainFound();
        return;
      }
    }

    decls->EmitAll();
    codeGen->DoFinalCodeGen();
}

StmtBlock::StmtBlock(List<VarDecl*> *d, List<Stmt*> *s) {
    Assert(d != NULL && s != NULL);
    (decls=d)->SetParentAll(this);
    (stmts=s)->SetParentAll(this);
}
void StmtBlock::Check() {
    nodeScope = new Scope();
    decls->DeclareAll(nodeScope);
    decls->CheckAll();
    stmts->CheckAll();
}
void StmtBlock::Emit() {
 
    decls->EmitAll();
    stmts->EmitAll();

}
ConditionalStmt::ConditionalStmt(Expr *t, Stmt *b) { 
    Assert(t != NULL && b != NULL);
    (test=t)->SetParent(this); 
    (body=b)->SetParent(this);
}

void ConditionalStmt::Check() {
    body->Check();
}

void ConditionalStmt::Emit() {


}

void LoopStmt::Emit() {


}

ForStmt::ForStmt(Expr *i, Expr *t, Expr *s, Stmt *b): LoopStmt(t, b) { 
    Assert(i != NULL && t != NULL && s != NULL && b != NULL);
    (init=i)->SetParent(this);
    (step=s)->SetParent(this);
}

void ForStmt::Emit() {

   char * label1 = codeGen->NewLabel();
   char * label2 = codeGen->NewLabel();

   this->breakLabel = label2;  
 
   init->Emit();
   
   codeGen->GenLabel(label1);
   test->Emit();
   
   //Get location node of test expression
   Location * testLoc = test->GetLocationNode();
        
   //Generate IfZ instruction; jump to second label (i.e. out of the while loop) if the expression is false
   codeGen->GenIfZ(testLoc, label2);
   
   body->Emit();

   step->Emit();
   
   codeGen->GenGoto(label1);

   codeGen->GenLabel(label2);
   

}

void WhileStmt::Emit() {
  
   char * label1 = codeGen->NewLabel();
   char * label2 = codeGen->NewLabel();

   this->breakLabel = label2;
 
   //First Label for While Loop
   codeGen->GenLabel(label1);

   test->Emit();

   //Get location node of test expression
   Location * testLoc = test->GetLocationNode();
        
   //Generate IfZ instruction; jump to second label (i.e. out of the while loop) if the expression is false
   codeGen->GenIfZ(testLoc, label2);
   
   //Emit code for the body of the while loop
   body->Emit();    

   //Return to first label to continue the while loop
   codeGen->GenGoto(label1);

   //Second Label for code after the while loop finishes
   codeGen->GenLabel(label2);
  

}

IfStmt::IfStmt(Expr *t, Stmt *tb, Stmt *eb): ConditionalStmt(t, tb) { 
    Assert(t != NULL && tb != NULL); // else can be NULL
    elseBody = eb;
    if (elseBody) elseBody->SetParent(this);
}
void IfStmt::Check() {
    ConditionalStmt::Check();
    if (elseBody) elseBody->Check();
}
void IfStmt::Emit() {
    test->Emit();//Emit() call for the test expression
    char * label1;
    char * label2;

    Call * testCall = dynamic_cast<Call *>(test);
    FieldAccess * boolField = dynamic_cast<FieldAccess *>(test);

    //Used for if-else statements
    if(elseBody)
    {
       label1 = codeGen->NewLabel();
       label2 = codeGen->NewLabel();


       //Get location node of test expression
       Location * testLoc = test->GetLocationNode();
       
       if(testCall)
       {
          Decl * d = FindDecl(testCall->GetId());
          FnDecl * fn = dynamic_cast<FnDecl *>(d);
          if(fn)
          {
            //testLoc = fn->GetLocationNode();
              testLoc = test->GetLocationNode();
          }
       }
       else if(boolField)
       {
         Decl * d = FindDecl(boolField->GetId());
         VarDecl * v = dynamic_cast<VarDecl *>(d);
         if(v)
         {
           testLoc = v->GetLocationNode(); 
         }
       }
       else
       {
         testLoc = test->GetLocationNode();
       }
 
       //Generate IfZ instruction; jump to label if expression is false
       codeGen->GenIfZ(testLoc, label1);
       body->Emit();//Emit code for the body (this is executed if expression is true)
       codeGen->GenGoto(label2);//Go to second label to skip "else" statement
       codeGen->GenLabel(label1);//Generate label for "false" case
       elseBody->Emit();//Emit code for "else" case
       codeGen->GenLabel(label2);//Generate code for second label
    }
    else //Used for if statements without an "else" body
    {
       label1 = codeGen->NewLabel();

       //Get location node of test expression
       Location * testLoc; 

       if(testCall)
       {
          Decl * d = FindDecl(testCall->GetId());
          FnDecl * fn = dynamic_cast<FnDecl *>(d);
          if(fn)
          {
            testLoc = fn->GetLocationNode();
          }
       }
       else if(boolField)
       {
         Decl * d = FindDecl(boolField->GetId());
         VarDecl * v = dynamic_cast<VarDecl *>(d);
         if(v)
         {
           testLoc = v->GetLocationNode(); 
         }
       }
       else
       {
         testLoc = test->GetLocationNode();
       }
        
       //Generate IfZ instruction; jump to label if expression is false
       codeGen->GenIfZ(testLoc, label1); //SegFault here because testLoc is NULL
       
       body->Emit();//Emit code for the body (this is executed if expression is true)
      
       codeGen->GenLabel(label1);//Generate label for "false" case
              
    }
}


void BreakStmt::Emit()
{
  Node * parent = this->GetParent();
  ForStmt * forParent = dynamic_cast<ForStmt *>(parent);
  WhileStmt * whileParent = dynamic_cast<WhileStmt *>(parent);
  Program * prog = dynamic_cast<Program *>(parent);

  while(!forParent && !whileParent && !prog)
  {  
    parent = parent->GetParent();
    forParent = dynamic_cast<ForStmt *>(parent);
    whileParent = dynamic_cast<WhileStmt *>(parent);
    prog = dynamic_cast<Program *>(parent);
  }

 

 
  if(forParent)
    codeGen->GenGoto(forParent->breakLabel);
  else if(whileParent)
    codeGen->GenGoto(whileParent->breakLabel);
    
  


}


ReturnStmt::ReturnStmt(yyltype loc, Expr *e) : Stmt(loc) { 
    Assert(e != NULL);
    (expr=e)->SetParent(this);
}

void ReturnStmt::Emit()
{
    
   expr->Emit();

   EmptyExpr * empty = dynamic_cast<EmptyExpr *>(expr);
   if(!empty)
   {
     Location * returnLoc;
     FieldAccess * field = dynamic_cast<FieldAccess *>(expr);
     ArrayAccess * array = dynamic_cast<ArrayAccess *>(expr);
     if(field)
     {
       Decl * d = FindDecl(field->GetId());
       VarDecl * v = dynamic_cast<VarDecl *>(d);
         
       
       //Check for instance variable here
       if(v)
       {
         Node * parent = v->GetParent();
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
            bool isInstanceVar = false;
            List<Decl *> *members = classParent->GetMembers();
            for(int i = 0; i < members->NumElements(); i++)
            {    
              Decl * member = members->Nth(i);
              VarDecl * var = dynamic_cast<VarDecl *>(member);
              
              if(var && strcmp(var->GetId()->GetName(), v->GetId()->GetName()) == 0)
              {
                 isInstanceVar = true;
                 int offset = 4 *(i+1);     
                 Location *instanceLoc1 = codeGen->GenLoad(codeGen->ThisPtr, offset);
                 FnDecl::numBytes+=4;
                 returnLoc = instanceLoc1;
              }
            }
            if(!isInstanceVar)
            {
               isInstanceVar = false;
               returnLoc = v->GetLocationNode();
               
            }
         }

         else
         { 
            returnLoc = v->GetLocationNode();
            Node * parent = v->GetParent();
            FnDecl * fnParent = dynamic_cast<FnDecl *>(parent);
            Program * prog = dynamic_cast<Program *>(parent);
         
            while(!fnParent && !prog)
            {
             parent = parent->GetParent();
             classParent = dynamic_cast<ClassDecl *>(parent);
             prog = dynamic_cast<Program *>(parent);
            }

            if(fnParent)
            {
              Type * fieldType = v->GetDeclaredType();
              ArrayType * arrayType = dynamic_cast<ArrayType *>(fieldType);
              if(arrayType)
               fnParent->arrayLengthLoc = returnLoc; 
            }
             
         }
         
       }
     }
     else if(array)
     {
       Expr * base = array->GetBase();  
       Expr * sub = array->GetSubscript();
       
       Location * varLoc;
       FieldAccess * baseField = dynamic_cast<FieldAccess *>(base);

       if(baseField)
       {
         Decl * d = FindDecl(baseField->GetId());
         VarDecl * v = dynamic_cast<VarDecl *>(d);
         varLoc = v->GetLocationNode();
       }

       IntConstant * sub1 = dynamic_cast<IntConstant *>(sub); 
       FieldAccess * sub2 = dynamic_cast<FieldAccess *>(sub);
       ArrayAccess * sub3 = dynamic_cast<ArrayAccess *>(sub);

       

       if(sub3)
       {
         Expr * base2 = sub3->GetBase(); 
         Expr * intSub = sub3->GetSubscript();

         FieldAccess * baseField2 = dynamic_cast<FieldAccess *>(base2);
         Decl * d2 = FindDecl(baseField2->GetId());
         VarDecl * v = dynamic_cast<VarDecl *>(d2);
         Location * baseLoc = v->GetLocationNode();         
         char * firstLabel = codeGen->NewLabel();
         char * secondLabel = codeGen->NewLabel();
         const char * badSubscript = err_arr_bad_size;

 
         IntConstant * ic = dynamic_cast<IntConstant *>(intSub);
         if(ic)
         {
            Location * indexLoc = codeGen->GenLoadConstant(ic->GetValue());
          
            //Error checking on subscript goes here
            Location * zeroLoc = codeGen->GenLoadConstant(0);
            Location * firstCheck = codeGen->GenBinaryOp("<", indexLoc, zeroLoc);
         
            Location * arrayLength = codeGen->GenLoad(baseLoc, -4);
            Location * secondCheck = codeGen->GenBinaryOp("<", indexLoc, arrayLength);

            Location * equalCheck = codeGen->GenBinaryOp("==", secondCheck, zeroLoc);
 
            Location * orCheck = codeGen->GenBinaryOp("||", firstCheck, equalCheck);

            codeGen->GenIfZ(orCheck, firstLabel);

           //Executes if the subscript is bad
           Location * badSubscriptError = codeGen->GenLoadConstant(badSubscript); 

           codeGen->GenBuiltInCall(PrintString, badSubscriptError, NULL);

           codeGen->GenBuiltInCall(Halt, NULL, NULL);

           //At this point, the subscript is valid
           codeGen->GenLabel(firstLabel); 
           Location * fourLoc = codeGen->GenLoadConstant(4);
           Location * fourBytes = codeGen->GenBinaryOp("*", fourLoc, indexLoc);
         
           Location * addressLoc = codeGen->GenBinaryOp("+", baseLoc, fourBytes);

           Location * loadLoc = codeGen->GenLoad(addressLoc, 0);

           //  At this point, we have indexed into the nested array.  Now, we just have to go through the enclosing array
           //to get the value we want.
           
          
           //Error checking on subscript goes here
           zeroLoc = codeGen->GenLoadConstant(0);
           firstCheck = codeGen->GenBinaryOp("<", loadLoc, zeroLoc);
         
           arrayLength = codeGen->GenLoad(baseLoc, -4);
           secondCheck = codeGen->GenBinaryOp("<", indexLoc, arrayLength);

           equalCheck = codeGen->GenBinaryOp("==", secondCheck, zeroLoc);
 
           orCheck = codeGen->GenBinaryOp("||", firstCheck, equalCheck);

           codeGen->GenIfZ(orCheck, secondLabel);

           //Executes if the subscript is bad
           badSubscriptError = codeGen->GenLoadConstant(badSubscript); 

           codeGen->GenBuiltInCall(PrintString, badSubscriptError, NULL);

           codeGen->GenBuiltInCall(Halt, NULL, NULL);

           //At this point, the subscript is valid
           codeGen->GenLabel(secondLabel); 
           fourLoc = codeGen->GenLoadConstant(4);
           fourBytes = codeGen->GenBinaryOp("*", fourLoc, indexLoc);
         
           addressLoc = codeGen->GenBinaryOp("+", baseLoc, fourBytes);

           loadLoc = codeGen->GenLoad(addressLoc, 0);

           returnLoc = loadLoc;


           FnDecl::numBytes+=96;

         }

       }
     }
     else
     {
      
      returnLoc = expr->GetLocationNode();
      
     }
      
      codeGen->GenReturn(returnLoc);//Handle both the case of having a return value and not having one
   }
   else
   {
    
    codeGen->GenReturn();
   } 
}
 
PrintStmt::PrintStmt(List<Expr*> *a) {    
    Assert(a != NULL);
    (args=a)->SetParentAll(this);
}

void PrintStmt::Emit()
{

  //Call Emit() for all child nodes as part of postorder tree traversal
  for(int i = 0; i < args->NumElements(); i++)
    args->Nth(i)->Emit();

  

  
  for(int i = 0; i < args->NumElements(); i++)
  {
    Expr * arg = args->Nth(i); 
    FieldAccess * field = dynamic_cast<FieldAccess *>(arg);
    IntConstant * intPrint = dynamic_cast<IntConstant *>(arg);
    StringConstant * stringPrint = dynamic_cast<StringConstant *>(arg);
    BoolConstant * boolPrint = dynamic_cast<BoolConstant *>(arg);
    EqualityExpr * eqExpr = dynamic_cast<EqualityExpr *>(arg); 
    ArithmeticExpr * arithExpr = dynamic_cast<ArithmeticExpr *>(arg);
    RelationalExpr * relExpr = dynamic_cast<RelationalExpr *>(arg);
    LogicalExpr * logExpr = dynamic_cast<LogicalExpr *>(arg);
    ArrayAccess * arrayPrint = dynamic_cast<ArrayAccess *>(arg);
    Call * fnCall = dynamic_cast<Call *>(arg);

    //Add in an additional test case for array accesses later
     
    if(intPrint) //Print integer constant
    { 
        Location * intLoc = intPrint->GetLocationNode();
        codeGen->GenBuiltInCall(PrintInt, intLoc , NULL);
    }
    else if(stringPrint) //Print string constant
    {
       Location * stringLoc = stringPrint->GetLocationNode();
       codeGen->GenBuiltInCall(PrintString, stringLoc, NULL); 
    }
    else if(boolPrint) //Print bool constant
    {
        Location * boolLoc = boolPrint->GetLocationNode();
        codeGen->GenBuiltInCall(PrintBool, boolLoc, NULL);
    }
    else if(field) //Print variable content (if it is either of type int, string or bool)
    {
        Decl * d = FindDecl(field->GetId());
        VarDecl * v = dynamic_cast<VarDecl *>(d); 
        
        //Handle field when it is a variable of type int, string or bool
        if(v)
        {
          Type * t = v->GetDeclaredType();
          char * typeName = t->GetName();
           
          Location * varLoc = v->GetLocationNode();

          //Execute built in call to the appropriate Print function
          if(strcmp(typeName, "int") == 0) 
          {
            codeGen->GenBuiltInCall(PrintInt, varLoc, NULL);  
          }
          else if(strcmp(typeName, "string") == 0)
          {
            codeGen->GenBuiltInCall(PrintString, varLoc, NULL); 
          }
          else if(strcmp(typeName, "bool") == 0)
          {
                            
            codeGen->GenBuiltInCall(PrintBool, varLoc, NULL);
          }
             
        }    
    }
    else if(fnCall)
    {
       
       const char * fnName = fnCall->GetId()->GetName();
       if(fnCall->GetBase())
       {
         Expr * e = fnCall->GetBase();
         FieldAccess * f = dynamic_cast<FieldAccess *>(e);
         Call * fn = dynamic_cast<Call *>(e);
         NewArrayExpr * nae = dynamic_cast<NewArrayExpr *>(e);
         if(f)
         {
           Decl * d = FindDecl(f->GetId());
           VarDecl * v = dynamic_cast<VarDecl *>(d);
           if(v)
           {
             Location * arrayLengthLoc = v->GetArrayLength();
             if(arrayLengthLoc && strcmp(fnName, "length") == 0)
             {
                codeGen->GenBuiltInCall(PrintInt, arrayLengthLoc, NULL); 
             }
             else
             { 
                Type * type = v->GetDeclaredType();
                NamedType *nt = dynamic_cast<NamedType *>(type);
                
                Decl * ntDecl = FindDecl(nt->GetId());
                ClassDecl * classDecl = dynamic_cast<ClassDecl*>(ntDecl);
                if(classDecl)
                {     
                 List<Decl *> *members = classDecl->GetMembers();
                  for(int i = 0; i < members->NumElements(); i++)
                  {
                    Decl * member = members->Nth(i);
                    FnDecl * fnMember = dynamic_cast<FnDecl *>(member);
                    
                    if(fnMember && strcmp(fnMember->GetId()->GetName(), fnName) == 0)
                    {
                     
                      char * typeName = fnMember->GetType()->GetName();
                      Location *fnLoc = fnMember->GetLocationNode();
                      

                      if(fnLoc && strcmp(typeName, "int") == 0)
                        codeGen->GenBuiltInCall(PrintInt, fnLoc, NULL);
                      else if(fnLoc && strcmp(typeName, "string") == 0)
                        codeGen->GenBuiltInCall(PrintString, fnLoc, NULL);
                      else if(fnLoc && strcmp(typeName, "bool") == 0)
                       codeGen->GenBuiltInCall(PrintBool, fnLoc, NULL);
                   }
                 }
              }
           }
          }
        }
        else if(fn)
        {
          
          Decl * fnName = FindDecl(fn->GetId());
          FnDecl * fnDecl = dynamic_cast<FnDecl *>(fnName); 
          if(fnDecl)
          {
            const char * name = fnDecl->GetId()->GetName();
            //printf("The name is %s\n", name);
            Location * fnLoc = fnDecl->GetArrayLength();
            Type * fnType = fnDecl->GetType();
            char * typeName = fnType->GetName();
            
            if(fnLoc && strcmp(fnCall->GetId()->GetName(), "length") == 0)
            {
                codeGen->GenBuiltInCall(PrintInt, fnLoc, NULL); 
            }

          }
        }
        else if(nae)
        {
          Location * arrayLengthLoc = nae->GetArrayLengthLoc();
          codeGen->GenBuiltInCall(PrintInt, arrayLengthLoc, NULL);

        }
       }
       else
       {
        Identifier * callId = fnCall->GetId();
        Decl * d = FindDecl(callId);
     
        FnDecl * fn = dynamic_cast<FnDecl *>(d);
    
        Type * type = fn->GetType();
        char * typeName = type->GetName();
        Location *fnLoc = fn->GetLocationNode();
        if(fnLoc && strcmp(typeName, "int") == 0)
          codeGen->GenBuiltInCall(PrintInt, fnLoc, NULL);
        else if(fnLoc && strcmp(typeName, "string") == 0)
          codeGen->GenBuiltInCall(PrintString, fnLoc, NULL);
        else if(fnLoc && strcmp(typeName, "bool") == 0)
          codeGen->GenBuiltInCall(PrintBool, fnLoc, NULL);
                 

      
       }
       /*Identifier * callId = fnCall->GetId();
       Decl * d = FindDecl(callId);
     
 
       FnDecl * fn = dynamic_cast<FnDecl *>(d); //SegFault for object functions 
       Location * fnCallLoc = fn->GetLocationNode();
       List<Expr *> *args = fnCall->GetActuals();
       
       for(int i = 0; i < args->NumElements(); i++)
       {
         Expr * arg= args->Nth(i);
         FieldAccess * varField = dynamic_cast<FieldAccess *>(arg);
         if(varField)
         {
           Decl * varArg = FindDecl(varField->GetId());
           VarDecl * varArg2 = dynamic_cast<VarDecl *>(varArg);
           codeGen->GenPushParam(varArg2->GetLocationNode());
         } 
         else
         {
           codeGen->GenPushParam(args->Nth(i)->GetLocationNode()); 
         }  
         codeGen->OffsetToCurrentParam += 4; 
       }

       Type * returnType = fn->GetType();       
       bool isReturn = false;
       if(!returnType || strcmp(returnType->GetName(), "void") == 0)
         isReturn = false;
       else
         isReturn = true;
       
       Location * returnLoc = codeGen->GenLCall(fn->GetFnLabel(), isReturn);

       codeGen->GenPopParams(4*args->NumElements());
       for(int i = 0; i < args->NumElements(); i++) 
         codeGen->OffsetToCurrentParam -= 4;





       if(returnLoc)
       {
         if(strcmp(returnType->GetName(), "int") == 0)
           codeGen->GenBuiltInCall(PrintInt, returnLoc, NULL);
         else if(strcmp(returnType->GetName(), "string") == 0)
           codeGen->GenBuiltInCall(PrintString, returnLoc, NULL);
         else if(strcmp(returnType->GetName(), "bool") == 0)
           codeGen->GenBuiltInCall(PrintBool, returnLoc, NULL);
      }*/



    }
    else if(eqExpr)
    {
       Location * eqLoc = eqExpr->GetLocationNode();
       codeGen->GenBuiltInCall(PrintBool, eqLoc, NULL); 

    }
    else if(arithExpr)
    {
    
       Location * arithLoc = arithExpr->GetLocationNode();
       codeGen->GenBuiltInCall(PrintInt, arithLoc, NULL); 

    }
    else if(relExpr)
    {
       Location * relLoc = relExpr->GetLocationNode();
       codeGen->GenBuiltInCall(PrintBool, relLoc, NULL); 
       
    }
    else if(logExpr)
    { 
       Location * logLoc = logExpr->GetLocationNode();
       codeGen->GenBuiltInCall(PrintBool, logLoc, NULL);
    }
    else if(arrayPrint)
    {
       
       Expr * base = arrayPrint->GetBase();
       Expr * subscript = arrayPrint->GetSubscript();
       FieldAccess * baseField = dynamic_cast<FieldAccess *>(base);
       Location * baseLoc;

       if(baseField)
       {
         Decl * d = FindDecl(baseField->GetId());
         VarDecl * v = dynamic_cast<VarDecl *>(d); 
         baseLoc = v->GetLocationNode();
       }

       ArithmeticExpr * subscript1 = dynamic_cast<ArithmeticExpr *>(subscript); 
       IntConstant * subscript2 = dynamic_cast<IntConstant *>(subscript);
       FieldAccess * subscript3 = dynamic_cast<FieldAccess *>(subscript);
       ArrayAccess * subscript4 = dynamic_cast<ArrayAccess *>(subscript);

       //Location * baseLoc = v->GetLocationNode();
       const char * badSubscript = err_arr_out_of_bounds;
       char * firstLabel = codeGen->NewLabel();
       char * secondLabel = codeGen->NewLabel();

       //Used for multi-dimensional arrays only
       ArrayAccess * firstArrayBase = dynamic_cast<ArrayAccess *>(base);
       if(firstArrayBase) 
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

         Expr * secondSubscript = arrayPrint->GetSubscript();
         intSub = dynamic_cast<IntConstant *>(secondSubscript);
         f1 = dynamic_cast<FieldAccess *>(secondSubscript);         
      
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
 
         Location * loadLoc = codeGen->GenLoad(shiftLoc, 0);

         FnDecl::numBytes+=44; 
         Type * outerArrayType = a2->GetDeclaredType();
         ArrayType * outerArrayType2 = dynamic_cast<ArrayType *>(outerArrayType);
         
         const char * typeName = outerArrayType2->GetName();
          
         Location * returnLoc;

         if(strcmp(typeName, "int[][]") == 0)
           returnLoc = codeGen->GenBuiltInCall(PrintInt, loadLoc, NULL);
         else if(strcmp(typeName, "string[][]") == 0)
           returnLoc = codeGen->GenBuiltInCall(PrintString, loadLoc, NULL);
         else if(strcmp(typeName, "bool[][]") == 0)
           returnLoc = codeGen->GenBuiltInCall(PrintBool, loadLoc, NULL);
           
         FnDecl::numBytes+=4;
         
        
       
       }
      }
      else //Used for single-dimensional arrays only
      {
       if(subscript1)
       {
         Decl * d = FindDecl(baseField->GetId());
         VarDecl * v = dynamic_cast<VarDecl *>(d);


         Location * indexLoc = subscript1->GetLocationNode(); 
         Location * zeroLoc = codeGen->GenLoadConstant(0);
         Location * firstCheck = codeGen->GenBinaryOp("<", indexLoc, zeroLoc);
         
         Location * arrayLength = codeGen->GenLoad(baseLoc, -4);
         Location * secondCheck = codeGen->GenBinaryOp("<", indexLoc, arrayLength);

         Location * equalCheck = codeGen->GenBinaryOp("==", secondCheck, zeroLoc);
 
         Location * orCheck = codeGen->GenBinaryOp("||", firstCheck, equalCheck);

         codeGen->GenIfZ(orCheck, firstLabel);

         
         //Executes if the subscript is bad
         Location * badSubscriptError = codeGen->GenLoadConstant(badSubscript); 

         codeGen->GenBuiltInCall(PrintString, badSubscriptError, NULL);

         codeGen->GenBuiltInCall(Halt, NULL, NULL);

         //At this point, the subscript is valid
         codeGen->GenLabel(firstLabel); 
         Location * fourLoc = codeGen->GenLoadConstant(4);
         Location * fourBytes = codeGen->GenBinaryOp("*", fourLoc, indexLoc);
         
         Location * addressLoc = codeGen->GenBinaryOp("+", baseLoc, fourBytes);

         Location * loadLoc = codeGen->GenLoad(addressLoc, 0);

         Type * arrayType = v->GetDeclaredType();
         ArrayType * arrayType2 = dynamic_cast<ArrayType *>(arrayType);

         const char * typeName = arrayType2->GetName();
          
         Location * returnLoc;

         if(strcmp(typeName, "int[]") == 0)
           returnLoc = codeGen->GenBuiltInCall(PrintInt, loadLoc, NULL);
         else if(strcmp(typeName, "string[]") == 0)
           returnLoc = codeGen->GenBuiltInCall(PrintString, loadLoc, NULL);
         else if(strcmp(typeName, "bool[]") == 0)
           returnLoc = codeGen->GenBuiltInCall(PrintBool, loadLoc, NULL);
           

         FnDecl::numBytes+=48;
  
       }
       else if(subscript2)
       {
         Decl * d = FindDecl(baseField->GetId());
         VarDecl * v = dynamic_cast<VarDecl *>(d);

         Location * indexLoc = codeGen->GenLoadConstant(subscript2->GetValue());
          
         //Error checking on subscript goes here
         Location * zeroLoc = codeGen->GenLoadConstant(0);
         Location * firstCheck = codeGen->GenBinaryOp("<", indexLoc, zeroLoc);
         
         Location * arrayLength = codeGen->GenLoad(baseLoc, -4);
         Location * secondCheck = codeGen->GenBinaryOp("<", indexLoc, arrayLength);

         Location * equalCheck = codeGen->GenBinaryOp("==", secondCheck, zeroLoc);
 
         Location * orCheck = codeGen->GenBinaryOp("||", firstCheck, equalCheck);

         codeGen->GenIfZ(orCheck, firstLabel);

         
         //Executes if the subscript is bad
         Location * badSubscriptError = codeGen->GenLoadConstant(badSubscript); 

         codeGen->GenBuiltInCall(PrintString, badSubscriptError, NULL);

         codeGen->GenBuiltInCall(Halt, NULL, NULL);

         //At this point, the subscript is valid
         codeGen->GenLabel(firstLabel); 
         Location * fourLoc = codeGen->GenLoadConstant(4);
         Location * fourBytes = codeGen->GenBinaryOp("*", fourLoc, indexLoc);
         
         Location * addressLoc = codeGen->GenBinaryOp("+", baseLoc, fourBytes);

         Location * loadLoc = codeGen->GenLoad(addressLoc, 0);

         Type * arrayType = v->GetDeclaredType();
         ArrayType * arrayType2 = dynamic_cast<ArrayType *>(arrayType);

         const char * typeName = arrayType2->GetName();
          
         Location * returnLoc;

         if(strcmp(typeName, "int[]") == 0)
           returnLoc = codeGen->GenBuiltInCall(PrintInt, loadLoc, NULL);
         else if(strcmp(typeName, "string[]") == 0)
           returnLoc = codeGen->GenBuiltInCall(PrintString, loadLoc, NULL);
         else if(strcmp(typeName, "bool[]") == 0)
           returnLoc = codeGen->GenBuiltInCall(PrintBool, loadLoc, NULL);
           

         FnDecl::numBytes+=48;
  

       } 
       else if(subscript3)
       {
         Decl * d = FindDecl(subscript3->GetId());
         VarDecl * v = dynamic_cast<VarDecl *>(d);

         Location * indexLoc = v->GetLocationNode();
         
         //Error checking on subscript goes here
         Location * zeroLoc = codeGen->GenLoadConstant(0);
         Location * firstCheck = codeGen->GenBinaryOp("<", indexLoc, zeroLoc);
         
         Location * arrayLength = codeGen->GenLoad(baseLoc, -4);
         Location * secondCheck = codeGen->GenBinaryOp("<", indexLoc, arrayLength);

         Location * equalCheck = codeGen->GenBinaryOp("==", secondCheck, zeroLoc);
 
         Location * orCheck = codeGen->GenBinaryOp("||", firstCheck, equalCheck);

         codeGen->GenIfZ(orCheck, firstLabel);

         //Executes if the subscript is bad
         Location * badSubscriptError = codeGen->GenLoadConstant(badSubscript); 

         codeGen->GenBuiltInCall(PrintString, badSubscriptError, NULL);

         codeGen->GenBuiltInCall(Halt, NULL, NULL);

         //At this point the subscript is valid
         codeGen->GenLabel(firstLabel);
         Location * fourLoc = codeGen->GenLoadConstant(4);
         Location * fourBytes = codeGen->GenBinaryOp("*", fourLoc, indexLoc);
         
         Location * addressLoc = codeGen->GenBinaryOp("+", baseLoc, fourBytes);

         Location * loadLoc = codeGen->GenLoad(addressLoc, 0);

         Type * arrayType = v->GetDeclaredType();
         ArrayType * arrayType2 = dynamic_cast<ArrayType *>(arrayType);

         const char * typeName = arrayType2->GetName();
          
         Location * returnLoc;

         if(strcmp(typeName, "int[]") == 0)
           returnLoc = codeGen->GenBuiltInCall(PrintInt, loadLoc, NULL);
         else if(strcmp(typeName, "string[]") == 0)
           returnLoc = codeGen->GenBuiltInCall(PrintString, loadLoc, NULL);
         else if(strcmp(typeName, "bool[]") == 0)
           returnLoc = codeGen->GenBuiltInCall(PrintBool, loadLoc, NULL);
           

         FnDecl::numBytes+=48;
  




       } 
       else if(subscript4)
       {
         Expr * nestedArrayBase = subscript4->GetBase();
         Expr * nestedArraySubscript = subscript4->GetSubscript();

         
         FieldAccess * nestedField = dynamic_cast<FieldAccess *>(nestedArrayBase);
         Decl * nestedArray = FindDecl(nestedField->GetId());    
         VarDecl * nestedArray2 = dynamic_cast<VarDecl *>(nestedArray);
        
         if(nestedArray2)
         {
           IntConstant * firstSubscript = dynamic_cast<IntConstant *>(nestedArraySubscript); 
           FieldAccess * secondSubscript = dynamic_cast<FieldAccess *>(nestedArraySubscript);
           ArithmeticExpr * thirdSubscript = dynamic_cast<ArithmeticExpr *>(nestedArraySubscript);//Handle this later
                     
           Location * arrayLoc = nestedArray2->GetLocationNode();

           Location * subscriptLoc;
           if(firstSubscript)
           { 
             subscriptLoc = codeGen->GenLoadConstant(firstSubscript->GetValue());
             FnDecl::numBytes+=4;
           }
           else if(secondSubscript)
           {
             Decl * d = FindDecl(nestedField->GetId());
             VarDecl * v = dynamic_cast<VarDecl *>(d); 
             if(v)
             {
               subscriptLoc = v->GetLocationNode();
             }
 
           }

           Location * zeroLoc = codeGen->GenLoadConstant(0);
           Location * firstCheck = codeGen->GenBinaryOp("<", subscriptLoc, zeroLoc);
         
           Location * arrayLength = codeGen->GenLoad(baseLoc, -4);
           Location * secondCheck = codeGen->GenBinaryOp("<", subscriptLoc, arrayLength);

           Location * equalCheck = codeGen->GenBinaryOp("==", secondCheck, zeroLoc);
 
           Location * orCheck = codeGen->GenBinaryOp("||", firstCheck, equalCheck);

           codeGen->GenIfZ(orCheck, firstLabel);

           //Executes if the subscript is bad
           Location * badSubscriptError = codeGen->GenLoadConstant(badSubscript); 

           codeGen->GenBuiltInCall(PrintString, badSubscriptError, NULL);

           codeGen->GenBuiltInCall(Halt, NULL, NULL);

           //At this point, the subscript is valid
           codeGen->GenLabel(firstLabel); 
           Location * fourLoc = codeGen->GenLoadConstant(4);
           Location * fourBytes = codeGen->GenBinaryOp("*", fourLoc, subscriptLoc);
         
           Location * addressLoc = codeGen->GenBinaryOp("+", baseLoc, fourBytes);

           Location * loadLoc = codeGen->GenLoad(addressLoc, 0);

            FnDecl::numBytes+=44;

           //Logic for the "outer" array index after the value from the "inner" one is obtained 
           zeroLoc = codeGen->GenLoadConstant(0);
           firstCheck = codeGen->GenBinaryOp("<", loadLoc, zeroLoc);
         
           arrayLength = codeGen->GenLoad(baseLoc, -4);
           secondCheck = codeGen->GenBinaryOp("<", loadLoc, arrayLength);

           equalCheck = codeGen->GenBinaryOp("==", secondCheck, zeroLoc);
 
           orCheck = codeGen->GenBinaryOp("||", firstCheck, equalCheck);
           codeGen->GenIfZ(orCheck, secondLabel);

           //Executes if the subscript is bad
           badSubscriptError = codeGen->GenLoadConstant(badSubscript); 

           codeGen->GenBuiltInCall(PrintString, badSubscriptError, NULL);

           codeGen->GenBuiltInCall(Halt, NULL, NULL);

           //At this point, the subscript is valid
           codeGen->GenLabel(secondLabel); 
           fourLoc = codeGen->GenLoadConstant(4);
           fourBytes = codeGen->GenBinaryOp("*", fourLoc, loadLoc);
         
           addressLoc = codeGen->GenBinaryOp("+", baseLoc, fourBytes);

           loadLoc = codeGen->GenLoad(addressLoc, 0);

           Type * outerArrayType = nestedArray2->GetDeclaredType();
           ArrayType * outerArrayType2 = dynamic_cast<ArrayType *>(outerArrayType);

           const char * typeName = outerArrayType2->GetName();
          

           Location * returnLoc;

           if(strcmp(typeName, "int[]") == 0)
             returnLoc = codeGen->GenBuiltInCall(PrintInt, loadLoc, NULL);
           else if(strcmp(typeName, "string[]") == 0)
             returnLoc = codeGen->GenBuiltInCall(PrintString, loadLoc, NULL);
           else if(strcmp(typeName, "bool[]") == 0)
             returnLoc = codeGen->GenBuiltInCall(PrintBool, loadLoc, NULL);
           

           FnDecl::numBytes+=48;
  
         }
         
          





       }
       }         
    }
  }
}




