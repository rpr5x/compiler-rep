/* File: ast_decl.cc
 * -----------------
 * Implementation of Decl node classes.
 */
#include "ast_decl.h"
#include "ast_type.h"
#include "ast_stmt.h"
#include "hashtable.h"
#include "errors.h"
#include <sstream>
#include <string>

extern Hashtable<Decl *> *table;
extern int currentLevel;
         
Decl::Decl(Identifier *n) : Node(*n->GetLocation()) {
    Assert(n != NULL);
    (id=n)->SetParent(this);
    this->branches = new List<Node *>();
    this->branches->Append(n); 
}

Identifier * Decl::getIdentifier() {
    return id;
}

Decl::~Decl() {
     

}
     

void Decl::checkNode()
{
  

} 

void Decl::checkNode2()
{

}

VarDecl::VarDecl(Identifier *n, Type *t) : Decl(n) {
    Assert(n != NULL && t != NULL);
    (type=t)->SetParent(this);
    this->branches = new List<Node *>();
    this->branches->Append(n);
    this->branches->Append(t);    
 
}

void VarDecl::checkNode()
{
   
   //Gets the parent node of this variable declaration
   Node * parentNode = this->GetParent();

   this->level = currentLevel;
  // printf("The variable is named %s and is at level %d\n", this->getIdentifier()->getName(), this->level);
   char * varName = this->getIdentifier()->getName();
   Decl * oldDecl = table->Lookup(varName);
   VarDecl * oldVarDecl = dynamic_cast<VarDecl *>(oldDecl);
   ClassDecl * oldClassDecl = dynamic_cast<ClassDecl *>(oldDecl);
   Type * t = this->getType();
   NamedType * nt = dynamic_cast<NamedType *>(t);


   //Special case if a global variable is declared with the same name as a class.  This is needed because the scoping
   //levels are slightly off because I assumed that only global variables (not classes or functions) had scope 0.
   if(oldClassDecl)
   {
     
     if(oldClassDecl->level == 1 && this->level == 0)
     {
       ReportError::DeclConflict(this, oldDecl);
     }
   }

   if(oldVarDecl)
   {
      // printf("I am variable %s, my current level is %d and my old level is %d\n", this->getIdentifier()->getName(), currentLevel, oldVarDecl->level);
       //Gets the enclosing class or function declaration of this variable object
       Node * parent = this->GetParent();
       ClassDecl * classParent = dynamic_cast<ClassDecl *>(parent);
       FnDecl * fnParent = dynamic_cast<FnDecl *>(parent);
       Program * programParent = dynamic_cast<Program *>(parent);
       while(!classParent && !fnParent && !programParent)
       {
         parent = parent->GetParent();
         classParent = dynamic_cast<ClassDecl *>(parent);
         fnParent = dynamic_cast<FnDecl *>(parent);
         programParent = dynamic_cast<Program *>(parent);
      
       }

       //Gets the enclosing class or function declaration of the variable object from the table
       Node * parent2 = oldVarDecl->GetParent();
       ClassDecl * classParent2 = dynamic_cast<ClassDecl *>(parent2);
       FnDecl * fnParent2 = dynamic_cast<FnDecl *>(parent2);
       Program * programParent2 = dynamic_cast<Program *>(parent2);
       while(!classParent2 && !fnParent2 && !programParent2)
       {
         parent2 = parent2->GetParent();
         classParent2 = dynamic_cast<ClassDecl *>(parent2);
         fnParent2 = dynamic_cast<FnDecl *>(parent2);
         programParent2 = dynamic_cast<Program *>(parent2);
      
       }  
        

      // printf("I am variable %s, the current level is %d and the old variable level is %d\n", this->getIdentifier()->getName(), currentLevel, oldVarDecl->level);


       //Shadow variable
       if(currentLevel > oldVarDecl->level)
       {
          table->Enter(this->getIdentifier()->getName(), this, false);
        
       } 
       else if(classParent && classParent2)
       {
         char * classParentName = classParent->getIdentifier()->getName();
         char * classParentName2 = classParent2->getIdentifier()->getName();
         
         if(currentLevel == oldVarDecl->level)
         {  
           if(classParent == classParent2 && strcmp(classParentName, classParentName2) == 0)
            ReportError::DeclConflict(this, oldDecl);
           else
             table->Enter(this->getIdentifier()->getName(), this, false);  

         }
       }
       else if(fnParent && fnParent2)
       {
         
         char * fnParentName = fnParent->getIdentifier()->getName();
         char * fnParentName2 = fnParent2->getIdentifier()->getName();
           
         if(currentLevel == oldVarDecl->level)
         { 
           if(fnParent == fnParent2 && strcmp(fnParentName, fnParentName2) == 0)
            ReportError::DeclConflict(this, oldDecl);
           else
             table->Enter(this->getIdentifier()->getName(), this, false);  
         }
       }
       /*else if(interfaceParent && classParent)//Return here
       {
         char * interfaceParentName = interfaceParent->getIdentifier()->getName();
         char * interfaceParentName2 = interfaceParent2->getIdentifier()->getName();
         
         if(currentLevel == oldVarDecl->level)
         { 
            
           if(fnParent == fnParent2 && strcmp(fnParentName, fnParentName2) == 0)
            ReportError::DeclConflict(this, oldDecl);
           else
             table->Enter(this->getIdentifier()->getName(), this, false);  

         }


       }*/
       else if(currentLevel == oldVarDecl->level)
       {
              
          //Error:  Cannot have two variables with the same name in the same scope        
          ReportError::DeclConflict(this, oldDecl);

       }
        
   }
   else  //Normal insert
   {
       table->Enter(this->getIdentifier()->getName(), this);
   }

   //Check that a NamedType used in a variable declaration is actually declared. The
   //NamedType should come from a class declaration.
   /*if(nt)
   {
      Decl * classDecl = table->Lookup(nt->getId()->getName());
      if(classDecl == NULL)
         ReportError::IdentifierNotDeclared(nt->getId() , LookingForType);  

   }*/
   
   




   //  Handles entering the function body scope.  Here, we want to increment the current nesting level
   //if we are at the last formal parameter.  This assumes that there is at least one formal parameter
   //in the function.  A much simpler case is specified in the checkNode() function for FnDecl when
   //a function has zero formal parameters.
   FnDecl * parentFunction = dynamic_cast<FnDecl *>(parentNode);
   if(parentFunction)
   {
     List<VarDecl *> * formalVars = parentFunction->getFormals();
     int formalListSize = formalVars->NumElements();
     if(formalListSize > 0)
     {
       Decl * lastField = formalVars->Nth(formalListSize-1);
       VarDecl * lastField2 = dynamic_cast<VarDecl *>(lastField);
       if(lastField2 && strcmp(this->getIdentifier()->getName(), lastField2->getIdentifier()->getName()) == 0 && this == lastField2)
       {  
         currentLevel++;  
       }
     }
   
   }

    Node * body = this->GetParent();
    StmtBlock * trueBody = dynamic_cast<StmtBlock *>(body);
    Node * body2 = body->GetParent();
    FnDecl * trueBody2 = dynamic_cast<FnDecl *>(body2);

   
    if(trueBody)
    {
      List<VarDecl*> *bodyDecls = trueBody->getDeclList();
      List<Stmt *> *bodyStmts = trueBody->getStmtList();
      VarDecl * lastVarDecl = bodyDecls->Nth(bodyDecls->NumElements()-1);
      int numStmts = trueBody->getStmtList()->NumElements();
      if(this == lastVarDecl && numStmts > 0)
      { 
        currentLevel++; 

      }
    }

   //Handles exiting the function body and parameter scopes when two or more variable declarations are
   //specified in the function body and no additional statements are specified.  Since all variable declarations
   //must precede statements in the function body, we can just get the location of the last one and exit the scopes
   //whenever we see that variable.
    if(trueBody && trueBody2)
    {
      List<VarDecl*> *bodyDecls = trueBody->getDeclList();
      List<Stmt *> *bodyStmts = trueBody->getStmtList();
      VarDecl * lastVarDecl = bodyDecls->Nth(bodyDecls->NumElements()-1);
         
      
      if(lastVarDecl && strcmp(lastVarDecl->getIdentifier()->getName(), this->getIdentifier()->getName()) == 0
       && bodyDecls->NumElements() >= 2 && bodyStmts->NumElements() == 0 && strcmp(lastVarDecl->getType()->getTypeName(), this->getType()->getTypeName()) == 0)
      {
       currentLevel--;//Exit function body scope
       currentLevel--;//Exit function parameter scope   
       currentLevel--;//Exit function header scope   
      }
 
   }
   //Handles exiting the class scope.  Here we get the list of fields (variables and functions) the class
   //holds and find the last one.  This only accounts for variables, NOT statements as well.  Refine this
   //when dealing with statements.
   ClassDecl * cd = dynamic_cast<ClassDecl *>(parentNode);
   if(cd)
   { 
     List<Decl *> * classFields = cd->getMembers();
     int fieldListSize = classFields->NumElements();
     Decl * lastField = classFields->Nth(fieldListSize-1);
     VarDecl * lastField2 = dynamic_cast<VarDecl *>(lastField); 

     //  Check if the variable is inside a class declaration and if it is the last entry in the class's
     //list of fields.  If so, we exit the actual class scope.
     //Note that we do not remove the class declaration from the table because we need it to make sure no other class
     //is created with the same name.
     if(lastField2 && strcmp(this->getIdentifier()->getName(), lastField2->getIdentifier()->getName()) == 0)
     {  
       currentLevel--;//Exit class local scope
       currentLevel--;//Exit class header scope
       //printf("We are exiting the class scope because of the variable:  %s\n", this->getIdentifier()->getName()); 
     }

   }

    //Handles exiting the interface scope when the last function has one or more formal parameters.
    if(parentFunction)
    {
      Node * node = parentFunction->GetParent();
      InterfaceDecl * parentInterface = dynamic_cast<InterfaceDecl *>(node);
      if(parentInterface)
      {
        List<Decl *> *members = parentInterface->getMembers();
        int listSize = members->NumElements();
        if(listSize > 0)      
        {
          Decl * lastDecl = members->Nth(listSize-1);
          FnDecl * lastFn = dynamic_cast<FnDecl *>(lastDecl);
          if(lastFn && lastFn->getFormals()->NumElements() > 0 && 
            strcmp(lastFn->getIdentifier()->getName(), parentFunction->getIdentifier()->getName()) == 0)
          { 
                        
            currentLevel--;//Exit interface local scope
            currentLevel--;//Exit interface header scope
          }      
        } 

      }
    } 
    
 }    
 
void VarDecl::checkNode2()
{
   
   //Gets the parent node of this variable declaration
   Node * parentNode = this->GetParent();

   this->level = currentLevel;
   //printf("The variable is named %s and is at level %d\n", this->getIdentifier()->getName(), this->level);
   char * varName = this->getIdentifier()->getName();
   Decl * oldDecl = table->Lookup(varName);
   VarDecl * oldVarDecl = dynamic_cast<VarDecl *>(oldDecl);
   Type * t = this->getType();
   NamedType * nt = dynamic_cast<NamedType *>(t);
   ArrayType * at = dynamic_cast<ArrayType *>(t);
  
   
   //Check that a NamedType used in a variable declaration is actually declared. The
   //NamedType should come from a class declaration or interface declaration.
   if(nt)
   {
      Decl * objDecl = table->Lookup(nt->getId()->getName());
      
      if(objDecl == NULL)
      {
         ReportError::IdentifierNotDeclared(nt->getId() , LookingForType);  
      }
   }
   else if(at)
   {
      Type * arrayType = at->getType();
      std::stringstream ss;
      ss << arrayType;
      std::string name = ss.str();
       

      Node * arrayDecl = table->Lookup(name.c_str());
      ClassDecl * arrayClass = dynamic_cast<ClassDecl *>(arrayDecl);
      
      if(!arrayClass && strcmp(name.c_str(), "int") != 0 && strcmp(name.c_str(), "double") != 0 && strcmp(name.c_str(), "bool") != 0 && strcmp(name.c_str(), "string") != 0)
      {
        Identifier * id = (Identifier *)(arrayType);
        id->setName((char *)name.c_str());
        ReportError::IdentifierNotDeclared(id, LookingForType);     
      }
   }
   //  Handles entering the function body scope.  Here, we want to increment the current nesting level
   //if we are at the last formal parameter.  This assumes that there is at least one formal parameter
   //in the function.  A much simpler case is specified in the checkNode() function for FnDecl when
   //a function has zero formal parameters.
   FnDecl * parentFunction = dynamic_cast<FnDecl *>(parentNode);
   if(parentFunction)
   {
     List<VarDecl *> * formalVars = parentFunction->getFormals();
     int formalListSize = formalVars->NumElements();
     if(formalListSize > 0)
     {
       Decl * lastField = formalVars->Nth(formalListSize-1);
       VarDecl * lastField2 = dynamic_cast<VarDecl *>(lastField);
       if(lastField2 && strcmp(this->getIdentifier()->getName(), lastField2->getIdentifier()->getName()) == 0 && this == lastField2)
       {  
         currentLevel++;  
       }
     }
   
   }

   //Handles exiting the function body and parameter scopes when two or more variable declarations are
   //specified in the function body and no additional statements are specified.  Since all variable declarations
   //must precede statements in the function body, we can just get the location of the last one and exit the scopes
   //whenever we see that variable.

    Node * body = this->GetParent();
    StmtBlock * trueBody = dynamic_cast<StmtBlock *>(body);

    if(trueBody)
    {
      List<VarDecl*> *bodyDecls = trueBody->getDeclList();
      List<Stmt *> *bodyStmts = trueBody->getStmtList();
      VarDecl * lastVarDecl = bodyDecls->Nth(bodyDecls->NumElements()-1);
         
      
      if(lastVarDecl && strcmp(lastVarDecl->getIdentifier()->getName(), this->getIdentifier()->getName()) == 0
       && bodyDecls->NumElements() >= 2 && bodyStmts->NumElements() == 0)
      {
       currentLevel--;//Exit function body scope
       currentLevel--;//Exit function parameter scope   
       currentLevel--;//Exit function header scope   
      }
 
   }
   //Handles exiting the class scope.  Here we get the list of fields (variables and functions) the class
   //holds and find the last one.  This only accounts for variables, NOT statements as well.  Refine this
   //when dealing with statements.
   ClassDecl * cd = dynamic_cast<ClassDecl *>(parentNode);
   if(cd)
   { 
     List<Decl *> * classFields = cd->getMembers();
     int fieldListSize = classFields->NumElements();
     Decl * lastField = classFields->Nth(fieldListSize-1);
     VarDecl * lastField2 = dynamic_cast<VarDecl *>(lastField); 

     //  Check if the variable is inside a class declaration and if it is the last entry in the class's
     //list of fields.  If so, we remove all of the variables in the current class scope and exit the actual class scope.
     //Note that we do not remove the class declaration from the table because we need it to make sure no other class
     //is created with the same name.
     if(lastField2 && strcmp(this->getIdentifier()->getName(), lastField2->getIdentifier()->getName()) == 0)
     {  
       currentLevel--;//Exit class local scope
       currentLevel--;//Exit class header scope
       //printf("We are exiting the class scope because of the variable:  %s\n", this->getIdentifier()->getName()); 
     }

   }

    //Handles exiting the interface scope when the last function has one or more formal parameters.
    if(parentFunction)
    {
      Node * node = parentFunction->GetParent();
      InterfaceDecl * parentInterface = dynamic_cast<InterfaceDecl *>(node);
      if(parentInterface)
      {
        List<Decl *> *members = parentInterface->getMembers();
        int listSize = members->NumElements();
        if(listSize > 0)      
        {
          Decl * lastDecl = members->Nth(listSize-1);
          FnDecl * lastFn = dynamic_cast<FnDecl *>(lastDecl);
          if(lastFn && lastFn->getFormals()->NumElements() > 0 && 
            strcmp(lastFn->getIdentifier()->getName(), parentFunction->getIdentifier()->getName()) == 0)
          { 
                        
            currentLevel--;//Exit interface local scope
            currentLevel--;//Exit interface header scope
          }      
        } 

      }
    }

}


Type* VarDecl::getType() {
 
    return type;
}

ClassDecl::ClassDecl(Identifier *n, NamedType *ex, List<NamedType*> *imp, List<Decl*> *m) : Decl(n) {
  
    
    // extends can be NULL, impl & mem may be empty lists but cannot be NULL
    Assert(n != NULL && imp != NULL && m != NULL);     
    extends = ex;
    if (extends) extends->SetParent(this);
    (implements=imp)->SetParentAll(this);
    (members=m)->SetParentAll(this);
    this->branches = new List<Node *>();
    this->branches->Append(n);
    if(extends) this->branches->Append(ex);
    for(int i = 0; i < imp->NumElements(); i++)
    {
    	this->branches->Append(imp->Nth(i));
    }
    for(int i = 0; i < m->NumElements(); i++)  
    {  
 	this->branches->Append(m->Nth(i));
    }
}


void ClassDecl::checkNode()
{
  //currentLevel++;
  //this->level = this->getIdentifier()->getLevel();
  //printf("The class is at level %d\n", this->level);
  //bool isErroneous = false;

  /*Decl * oldDecl = table->Lookup(this->getIdentifier()->getName());
  ClassDecl * oldClassDecl = dynamic_cast<ClassDecl *>(oldDecl);
  List<Decl *> *derivedMembers = this->getMembers(); 

  
  //Cannot have two class declarations with the same name in the same scope (Handle shadowing later)
  if(oldClassDecl) 
  {
    printf("This is %s\n", this->getIdentifier()->getName());
    printf("The class name is %s\n, the current level is %d and the old level is %d\n", oldClassDecl->getIdentifier()->getName(), currentLevel, oldClassDecl->level);
    if(currentLevel == oldClassDecl->level)
    {
      ReportError::DeclConflict(this, oldClassDecl);
      currentLevel--;
      return;
    }
  }

  
  
  
  table->Enter(this->getIdentifier()->getName(), this);*/
  //currentLevel++; 
  
  

}

void ClassDecl::checkNode2()
{

  Decl * oldDecl = table->Lookup(this->getIdentifier()->getName());
  ClassDecl * oldClassDecl = dynamic_cast<ClassDecl *>(oldDecl);
  List<Decl *> *derivedMembers = this->getMembers(); 

  //  Checks to see if a function in a derived class that overrides a function in a base
  //class has an incorrect type signature.
  if(this->extends)
  {
     //As a freebie, check if the class in the extends clause is declared and if not,
     //report that it has not been declared
     if(table->Lookup(this->extends->getId()->getName()) == NULL)
     {
       ReportError::IdentifierNotDeclared(this->extends->getId(), LookingForClass);  

     }

     else
     {
       //Get base class declarations (some of which may be functions)
       Decl * baseClass = table->Lookup(this->extends->getId()->getName());
       ClassDecl * baseClass2 = dynamic_cast<ClassDecl *>(baseClass);
       List<Decl *> *baseMembers = baseClass2->getMembers();   
       
 
       for(int i = 0; i < baseMembers->NumElements(); i++)
       {
         //Get variables and functions from base class
         Decl * d1 = baseMembers->Nth(i);
         VarDecl * v1 = dynamic_cast<VarDecl *>(d1);
         FnDecl * fn1 = dynamic_cast<FnDecl *> (d1);
                

         for(int j = 0; j < derivedMembers->NumElements(); j++)
         {
           //Get variables and functions from derived class
           Decl * d2 = derivedMembers->Nth(j);
           VarDecl * v2 = dynamic_cast<VarDecl *>(d2);
           FnDecl * fn2 = dynamic_cast<FnDecl *>(d2);
           

 
           if(v1 != NULL && v2 != NULL && strcmp(v1->getIdentifier()->getName(), v2->getIdentifier()->getName()) == 0)
           {
             ReportError::DeclConflict(v2, v1); 


           }
           //Check to make sure the two are actually functions and that they have the same name
           if(fn1 != NULL && fn2 != NULL && strcmp(fn1->getIdentifier()->getName(), fn2->getIdentifier()->getName()) == 0)
           {
             List<VarDecl *> *formals1 = fn1->getFormals(); 
             List<VarDecl *> *formals2 = fn2->getFormals();
 
             //Error:  The return types of the two functions do not match
             if(strcmp(fn1->getReturnType()->getTypeName(), fn2->getReturnType()->getTypeName()) != 0)    
             {       
              ReportError::OverrideMismatch(fn2);  
              //isErroneous = true;
              return;
                   
             }
             //Error:  There are a different number of formal parameters for both functions
             else if(formals1->NumElements() != formals2->NumElements())
             {
              ReportError::OverrideMismatch(fn2);
              //isErroneous = true;
              return;
            
             }
             //Error:  The formal parameters for the same functions do not match in data type
             else if(formals1->NumElements() == formals2->NumElements())
             {
               for(int k = 0; k < formals2->NumElements(); k++)
               {
                 if(strcmp(formals1->Nth(k)->getType()->getTypeName(), formals2->Nth(k)->getType()->getTypeName()) != 0)
                 {
                   ReportError::OverrideMismatch(fn2);  
                   return;
                  // isErroneous = true;
                  // break;//Break to avoid printing the same error message if multiple type mismatches occur
                 }
              
               }
             }

           }  
           //Breaks out of the inner for loop if we got an override error
           //if(isErroneous)
           //  break;
         }
         //Breaks out of the outer for loop if we got an override error
         //if(isErroneous)
         //  break;
       }

  }
}

  if(this->implements)
  {
    //Check to make sure that if a class implements an interface, that interface is properly declared
    for(int i = 0; i < this->implements->NumElements(); i++)
    {
      if(table->Lookup(this->implements->Nth(i)->getId()->getName()) == NULL)
       {
         ReportError::IdentifierNotDeclared(this->implements->Nth(i)->getId(), LookingForInterface);  
         return;
       }
    }

    //Checks to make sure that if a class implements an interface, then that class implements all of the methods
    //of that interface.
    List<NamedType *> *d = this->implements;
    bool isImplemented = false;//Used for keeping track whether a class implements all of the methods of an interface
    for(int i = 0; i < d->NumElements(); i++)
    {
     Decl * interfaceDecl = table->Lookup(d->Nth(i)->getId()->getName());
     InterfaceDecl * interfaceDecl2 = dynamic_cast<InterfaceDecl *>(interfaceDecl);//Get the interface declaration
     for(int j = 0; j < interfaceDecl2->getMembers()->NumElements(); j++)//Get all functions of the interface
     { 
        isImplemented = false;//Resets isImplemented to false to make sure we iterate through all of the interface functions correctly
        Decl * interDecl = interfaceDecl2->getMembers()->Nth(j);
        FnDecl * interDecl2 = dynamic_cast<FnDecl *>(interDecl);

        for(int k = 0; k < this->getMembers()->NumElements(); k++) //Get all functions from class
        {
          Decl * fnDecl = this->getMembers()->Nth(k);
          FnDecl * fn = dynamic_cast<FnDecl *>(fnDecl); //Get function from class

          //Compares every function in the class against an interface function declaration. If there is a class function declared
          //with the same name as an interface function, the class is considered to implement that interface function (since there
          //is no overloading in Decaf).  In this case, isImplemeted is set to true; 
          if(fn && strcmp(interDecl2->getIdentifier()->getName(), fn->getIdentifier()->getName()) == 0)
          {
            isImplemented = true;
           
             List<VarDecl *> *formals1 = interDecl2->getFormals();
             List<VarDecl *> *formals2 = fn->getFormals();
 
             //Error:  The return types of the two functions do not match
             if(strcmp(interDecl2->getReturnType()->getTypeName(), fn->getReturnType()->getTypeName()) != 0)    
             {       
              ReportError::OverrideMismatch(fn);  
              return;
                   
             }
             //Error:  There are a different number of formal parameters for both functions
             else if(formals1->NumElements() != formals2->NumElements())
             {
              ReportError::OverrideMismatch(fn);
              return;
            
             }
             //Error:  The formal parameters for the same functions do not match in data type
             else if(formals1->NumElements() == formals2->NumElements())
             {
               for(int k = 0; k < formals2->NumElements(); k++)
               {
                 if(strcmp(formals1->Nth(k)->getType()->getTypeName(), formals2->Nth(k)->getType()->getTypeName()) != 0)
                 {
                   ReportError::OverrideMismatch(fn);  
                   return;
                  // isErroneous = true;
                  // break;//Break to avoid printing the same error message if multiple type mismatches occur
                 }
              
               }
             }   




          }
        }
        //If the class does not implement one of the interface functions, report an error.
        if(!isImplemented)
           ReportError::InterfaceNotImplemented(this, d->Nth(i)); 
     }
    }
  }
}



List<Decl*> * ClassDecl::getMembers()
{

  return members;


}

InterfaceDecl::InterfaceDecl(Identifier *n, List<Decl*> *m) : Decl(n) {
    Assert(n != NULL && m != NULL);
    (members=m)->SetParentAll(this);
    this->branches = new List<Node *>();
    this->branches->Append(n);
    for(int i = 0; i < m->NumElements(); i++)
    	this->branches->Append(m->Nth(i)); 
}


void InterfaceDecl::checkNode()
{
   Decl * oldDecl = table->Lookup(this->getIdentifier()->getName());
   InterfaceDecl * oldInterfaceDecl = dynamic_cast<InterfaceDecl *>(oldDecl);
   List<Decl *> * members = this->members;
    
   //Cannot have two interface declarations with the same name in the same scope (deal with no shadowing later)
   if(oldInterfaceDecl)
   {
     if(currentLevel == oldInterfaceDecl->level)
     {
       ReportError::DeclConflict(this, oldInterfaceDecl);
     }
   } 
   else //Normal insert
   {
     table->Enter(this->getIdentifier()->getName(), this);

   }


   


   //Exit scope for the simple case of an empty interface
   if(this->members->NumElements() == 0)
   {
      currentLevel--;
   }
   

}

void InterfaceDecl::checkNode2()
{
   //Exit scope for the simple case of an empty interface
   if(this->members->NumElements() == 0)
   {
     currentLevel--;
   }

}


	
FnDecl::FnDecl(Identifier *n, Type *r, List<VarDecl*> *d) : Decl(n) {
    Assert(n != NULL && r!= NULL && d != NULL);
    (returnType=r)->SetParent(this);
    (formals=d)->SetParentAll(this);
    body = NULL;

    this->branches = new List<Node *>();
    this->branches->Append(n);
    this->branches->Append(r); 
    for(int i = 0; i < formals->NumElements(); i++)
      this->branches->Append(formals->Nth(i)); 
    
}


void FnDecl::SetFunctionBody(Stmt *b) { 
    (body=b)->SetParent(this);
    StmtBlock * trueBody = dynamic_cast<StmtBlock *>(body);
    this->branches->Append(trueBody);

}


void FnDecl::checkNode()
{
    //Function header scope
    
    Decl * oldDecl = table->Lookup(this->getIdentifier()->getName()); 
    FnDecl * oldFuncDecl = dynamic_cast<FnDecl *>(oldDecl);
     
    //Check if the function entry is already in the table
    if(oldFuncDecl != NULL)
    {
      //  If the current function declaration that we want to insert is at
      //at a deeper nesting level than the old one, make it a shadow function.
      if(currentLevel > oldFuncDecl->level)
      {
        table->Enter(this->getIdentifier()->getName(), this, false); 

      } //Error: Multiple declarations at the same scope level are not allowed.
      else if(currentLevel == oldFuncDecl->level) 
      {
         
         ReportError::DeclConflict(this, oldFuncDecl);
         return;
      }
    }
    else //Normal function insert
    {
       table->Enter(this->getIdentifier()->getName(), this);

    }

    NamedType * nt = dynamic_cast<NamedType *>(this->returnType);

    //  Check to make sure a NamedType used in a function declaration has
    //actually been declared.
    /*if(nt)
    {
      Decl * objDecl = table->Lookup(nt->getId()->getName());
      if(objDecl == NULL)
         ReportError::IdentifierNotDeclared(nt->getId() , LookingForType);  

    }*/

    //Increment for function body scope if there are no formal parameters.
    if(this->formals->NumElements() == 0)
      currentLevel++;

      
    /*//Check formal parameters   
    for(int i = 0; i < this->formals->NumElements(); i++)
    {
       Decl * d = this->formals->Nth(i);
       VarDecl * formalVar = dynamic_cast<VarDecl *>(d);
   
       Decl * decl = table->Lookup(d->getIdentifier()->getName());
       VarDecl * v = dynamic_cast<VarDecl *>(decl);
           
       if(v)
       {
          
         if(currentLevel > v->level)//Parameter variable shadows global variable
         {
           table->Enter(formalVar->getIdentifier()->getName(), formalVar, false);
         }
         else if(currentLevel == v->level)  //Illegal to have two exact variables in the same scope
         {
           ReportError::DeclConflict(formalVar, v);
         }
          
       }
       else 
       {
         table->Enter(formalVar->getIdentifier()->getName(), formalVar);
       }
    }*/

    //Handles the function body scope; refine this later for cases like for loops and while loops which require additional scopes.
    //currentLevel++;

    /*StmtBlock *temp2 = dynamic_cast<StmtBlock *>(this->body);
    List<VarDecl *> *decls = temp2->getDeclList();
    
    for(int i = 0; i < decls->NumElements(); i++)
    {
      VarDecl* tempDecl = decls->Nth(i);
      tempDecl->level = currentLevel;
      table->Enter(tempDecl->getIdentifier()->getName(), tempDecl);   

    }*/

   
     
      /*ClassDecl * cd = dynamic_cast<ClassDecl *>(parentNode);
   if(cd)
   { 
     List<Decl *> * classFields = cd->getMembers();
     int fieldListSize = classFields->NumElements();
     Decl * lastField = classFields->Nth(fieldListSize-1);
     FnDecl * lastField2 = dynamic_cast<FnDecl *>(lastField); 

     //Check if the variable is inside a class declaration and if it is the last entry in the class's
     //list of fields.  If so, we exit the class scope.
     if(strcmp(this->getIdentifier()->getName(), lastField2->getIdentifier()->getName()) == 0)
     { 
       table->Remove(cd->getIdentifier()->getName(), cd);
       currentLevel--; 
       printf("We are exiting the class scope because of the function:  %s\n", this->getIdentifier()->getName()); 
     }

    }*/

    //Exit interface scope when you have one or more internal functions and the last one has zero formal parameters.
    Node * parentNode = this->GetParent();
    InterfaceDecl * parentInterface = dynamic_cast<InterfaceDecl *>(parentNode);
    if(parentInterface)
    {    
      List<Decl *> *members = parentInterface->getMembers();
      int listSize = members->NumElements();
      if(listSize > 0)      
      {
        Decl * lastDecl = members->Nth(listSize-1);
        FnDecl * lastFn = dynamic_cast<FnDecl *>(lastDecl);
        if(lastFn && lastFn->getFormals()->NumElements() == 0 && 
           strcmp(lastFn->getIdentifier()->getName(), this->getIdentifier()->getName()) == 0)
        { 
          currentLevel--;
          

        }
      }
    }

}

void FnDecl::checkNode2()
{
    //Increment currentLevel when your function has zero formal parameters
    if(this->formals->NumElements() == 0)
      currentLevel++;
    

    //Exit interface scope when you have one or more internal functions and the last one has zero formal parameters.
    Node * parentNode = this->GetParent();
    InterfaceDecl * parentInterface = dynamic_cast<InterfaceDecl *>(parentNode);
    if(parentInterface)
    {    
      List<Decl *> *members = parentInterface->getMembers();
      int listSize = members->NumElements();
      if(listSize > 0)      
      {
        Decl * lastDecl = members->Nth(listSize-1);
        FnDecl * lastFn = dynamic_cast<FnDecl *>(lastDecl);
        if(lastFn && lastFn->getFormals()->NumElements() == 0 && 
           strcmp(lastFn->getIdentifier()->getName(), this->getIdentifier()->getName()) == 0)
        { 
          currentLevel--;
          

        }
      }
    }
}

Type * FnDecl::getReturnType()
{
  return returnType;
}

List<VarDecl*> * FnDecl::getFormals()
{
  return formals;
} 

