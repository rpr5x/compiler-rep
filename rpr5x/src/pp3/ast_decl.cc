/* File: ast_decl.cc
 * -----------------
 * Implementation of Decl node classes.
 */
#include "ast_decl.h"
#include "ast_type.h"
#include "ast_stmt.h"
#include "hashtable.h"
#include "errors.h"

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
      

VarDecl::VarDecl(Identifier *n, Type *t) : Decl(n) {
    Assert(n != NULL && t != NULL);
    (type=t)->SetParent(this);
    this->branches = new List<Node *>();
    this->branches->Append(n);
    this->branches->Append(t);    
 
}

void VarDecl::checkNode()
{
   //currentLevel++;
   this->level = currentLevel;
   char * varName = this->getIdentifier()->getName();
   Decl * oldDecl = table->Lookup(varName);
   VarDecl * oldVarDecl = dynamic_cast<VarDecl *>(oldDecl);
   Type * t = this->getType();
   NamedType * nt = dynamic_cast<NamedType *>(t);

   if(oldVarDecl)
   {
       //Shadow variable
       if(currentLevel > oldVarDecl->level)
       {
          table->Enter(this->getIdentifier()->getName(), this, false);
        
       } 
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
   if(nt)
   {
      Decl * classDecl = table->Lookup(nt->getId()->getName());
      if(classDecl == NULL)
         ReportError::IdentifierNotDeclared(nt->getId() , LookingForType);  

   }

   //Exit variable scope   
 



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
    this->branches->Append(ex);
    for(int i = 0; i < imp->NumElements(); i++)
    	this->branches->Append(imp->Nth(i));
    for(int i = 0; i < m->NumElements(); i++)    
 	this->branches->Append(m->Nth(i));

   

}


void ClassDecl::checkNode()
{
  currentLevel++;
  this->level = currentLevel;
  
  //bool isErroneous = false;

  Decl * oldDecl = table->Lookup(this->getIdentifier()->getName());
  ClassDecl * oldClassDecl = dynamic_cast<ClassDecl *>(oldDecl);
  List<Decl *> *derivedMembers = this->getMembers(); 

  //Cannot have two class declarations with the same name in the same scope (Handle shadowing later)
  if(oldClassDecl) 
  {
    if(currentLevel == oldClassDecl->level)
    {
      ReportError::DeclConflict(this, oldClassDecl);
      currentLevel--;
      return;
    }
  }

  //  Checks to see if a function in a derived class that overrides a function in a base
  //class has an incorrect type signature.
  if(this->extends)
  {
     if(table->Lookup(this->extends->getId()->getName()) == NULL)
     {
       ReportError::IdentifierNotDeclared(this->extends->getId(), LookingForClass);  

     }

     else
     {
       //Get base class declarations (some of which may be functions)
       Decl * baseClass = table->Lookup(this->extends->getId()->getName()); //SegFault here because base class is no longer in table
       ClassDecl * baseClass2 = dynamic_cast<ClassDecl *>(baseClass);
       List<Decl *> *baseMembers = baseClass2->getMembers();   
       
 
       for(int i = 0; i < baseMembers->NumElements(); i++)
       {
         //Get functions from base class
         Decl * d1 = baseMembers->Nth(i);
         FnDecl * fn1 = dynamic_cast<FnDecl *> (d1);
         List<VarDecl *> *formals1 = fn1->getFormals();       

         for(int j = 0; j < derivedMembers->NumElements(); j++)
         {
           //Get functions from derived class
           Decl * d2 = derivedMembers->Nth(j);
           FnDecl * fn2 = dynamic_cast<FnDecl *>(d2);
           List<VarDecl *> *formals2 = fn2->getFormals();


           //Check to make sure the two are actually functions and that they have the same name
           if(fn1 != NULL && fn2 != NULL && strcmp(fn1->getIdentifier()->getName(), fn2->getIdentifier()->getName()) == 0)
           {
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
  }

  table->Enter(this->getIdentifier()->getName(), this);
 
  
  /*Iterator<Decl *> iter = table->getIterator();
  Decl * decl;
  while((decl = iter.getNextValue()) != NULL)
  {


  }*/


  //Exit scope
  //table->Remove(this->getIdentifier()->getName(), this);
  currentLevel--;
    


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
   currentLevel++;
   this->level = currentLevel;
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


   //Exit scope
   //table->Remove(this->getIdentifier()->getName(), this);
   currentLevel--;
   

}




	
FnDecl::FnDecl(Identifier *n, Type *r, List<VarDecl*> *d) : Decl(n) {
    Assert(n != NULL && r!= NULL && d != NULL);
    (returnType=r)->SetParent(this);
    (formals=d)->SetParentAll(this);
    body = NULL;

    this->branches = new List<Node *>();
    this->branches->Append(r);
    this->branches->Append(n); 
    for(int i = 0; i < formals->NumElements(); i++)
      this->branches->Append(formals->Nth(i)); 
    
}


void FnDecl::SetFunctionBody(Stmt *b) { 
    (body=b)->SetParent(this);
    StmtBlock * trueBody = dynamic_cast<StmtBlock *>(body);
    List<VarDecl *> *varList = trueBody->getDeclList();
    for(int i = 0; i < varList->NumElements(); i++)
      this->branches->Append(varList->Nth(i)); 

    //this->branches->Append(body);
}


void FnDecl::checkNode()
{
    //Function header scope
    currentLevel++;
    this->level = currentLevel;
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
    if(nt)
    {
      Decl * classDecl = table->Lookup(nt->getId()->getName());
      if(classDecl == NULL)
         ReportError::IdentifierNotDeclared(nt->getId() , LookingForType);  

    }

    //Function parameter scope
    currentLevel++;
     
    //Check formal parameters   
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
    }

    //Handles the function body scope; refine this later for cases like for loops and while loops which require additional scopes.
    currentLevel++;

    StmtBlock *temp2 = dynamic_cast<StmtBlock *>(this->body);
    List<VarDecl *> *decls = temp2->getDeclList();
    
    for(int i = 0; i < decls->NumElements(); i++)
    {
      VarDecl* tempDecl = decls->Nth(i);
      tempDecl->level = currentLevel;
      table->Enter(tempDecl->getIdentifier()->getName(), tempDecl);   

    }

    
    currentLevel--;//Exit function body scope
    currentLevel--;//Exit functon parameter scope


    //table->Remove(this->getIdentifier()->getName(), this); 
    currentLevel--;//Exit function header scope
    

}

Type * FnDecl::getReturnType()
{
  return returnType;
}

List<VarDecl*> * FnDecl::getFormals()
{
  return formals;
} 

