/* File: ast_decl.cc
 * -----------------
 * Implementation of Decl node classes.
 */
#include "ast_decl.h"
#include "ast_type.h"
#include "ast_stmt.h"
#include "scope.h"
#include "errors.h"
#include "codegen.h"

extern CodeGenerator * codeGen;        
         
Decl::Decl(Identifier *n) : Node(*n->GetLocation()) {
    Assert(n != NULL);
    (id=n)->SetParent(this); 
}

bool Decl::ConflictsWithPrevious(Decl *prev) {
    ReportError::DeclConflict(this, prev);
    return true;
}

VarDecl::VarDecl(Identifier *n, Type *t) : Decl(n) {
    Assert(n != NULL && t != NULL);
    (type=t)->SetParent(this);
}
  
void VarDecl::Check() { type->Check(); }

void VarDecl::Emit()
{
  Node * parent = this->GetParent();
  StmtBlock * stmt = dynamic_cast<StmtBlock *>(parent);// Local variables
  FnDecl * fn = dynamic_cast<FnDecl *>(parent); //Formal parameters
  Program * prog = dynamic_cast<Program *>(parent); //Global variables
  Node * parent2 = parent->GetParent();
  ClassDecl * classParent = dynamic_cast<ClassDecl *>(parent2);

  if(classParent)
  {
    this->varLoc = new Location(codeGen->ThisPtr->GetSegment(), codeGen->OffsetToCurrentParam, this->GetId()->GetName());
    codeGen->OffsetToCurrentParam += 4;
 
  }
  else if(stmt)
  {
    this->varLoc = new Location(fpRelative, codeGen->OffsetToCurrentLocal, this->GetId()->GetName());
    FnDecl::numBytes += 4;
    codeGen->OffsetToCurrentLocal -= 4;  
  }
  else if(fn) //Accounts for formal parameters specified in the function
  {    
     this->varLoc = new Location(fpRelative, codeGen->OffsetToCurrentParam, this->GetId()->GetName());
     codeGen->OffsetToCurrentParam += 4;
  }
  else if(prog) //Accounts for global variables
  {
     this->varLoc = new Location(gpRelative, codeGen->OffsetToCurrentGlobal, this->GetId()->GetName());
     codeGen->OffsetToCurrentGlobal += 4;
  }

}


ClassDecl::ClassDecl(Identifier *n, NamedType *ex, List<NamedType*> *imp, List<Decl*> *m) : Decl(n) {
    // extends can be NULL, impl & mem may be empty lists but cannot be NULL
    Assert(n != NULL && imp != NULL && m != NULL);     
    extends = ex;
    if (extends) extends->SetParent(this);
    (implements=imp)->SetParentAll(this);
    (members=m)->SetParentAll(this);
    cType = new NamedType(n);
    cType->SetParent(this);
    convImp = NULL;
}

void ClassDecl::Check() {

    if (extends && !extends->IsClass()) {
        ReportError::IdentifierNotDeclared(extends->GetId(), LookingForClass);
        extends = NULL;
    }
    for (int i = 0; i < implements->NumElements(); i++) {
        NamedType *in = implements->Nth(i);
        if (!in->IsInterface()) {
            ReportError::IdentifierNotDeclared(in->GetId(), LookingForInterface);
            implements->RemoveAt(i--);
        }
    }
    PrepareScope();
    members->CheckAll();
}

void ClassDecl::Emit()
{
  //this->classLoc = new Location(fpRelative, codeGen->OffsetToCurrentLocal, this->GetId()->GetName());    
  this->fnLabels = new List<const char *>();
   

  codeGen->OffsetToCurrentLocal -= 4;

  for(int i = 0; i < members->NumElements(); i++)
  {
     members->Nth(i)->Emit();
  }

   
  codeGen->GenVTable(this->GetId()->GetName(), this->fnLabels);  



}

// This is not done very cleanly. I should sit down and sort this out. Right now
// I was using the copy-in strategy from the old compiler, but I think the link to
// parent may be the better way now.
Scope *ClassDecl::PrepareScope()
{
    if (nodeScope) return nodeScope;
    nodeScope = new Scope();  
    if (extends) {
        ClassDecl *ext = dynamic_cast<ClassDecl*>(parent->FindDecl(extends->GetId())); 
        if (ext) nodeScope->CopyFromScope(ext->PrepareScope(), this);
    }
    convImp = new List<InterfaceDecl*>;
    for (int i = 0; i < implements->NumElements(); i++) {
        NamedType *in = implements->Nth(i);
        InterfaceDecl *id = dynamic_cast<InterfaceDecl*>(in->FindDecl(in->GetId()));
        if (id) {
		nodeScope->CopyFromScope(id->PrepareScope(), NULL);
            convImp->Append(id);
	  }
    }
    members->DeclareAll(nodeScope);
    return nodeScope;
}

InterfaceDecl::InterfaceDecl(Identifier *n, List<Decl*> *m) : Decl(n) {
    Assert(n != NULL && m != NULL);
    (members=m)->SetParentAll(this);
}

void InterfaceDecl::Check() {
    PrepareScope();
    members->CheckAll();
}

void InterfaceDecl::Emit()
{



}
  
Scope *InterfaceDecl::PrepareScope() {
    if (nodeScope) return nodeScope;
    nodeScope = new Scope();  
    members->DeclareAll(nodeScope);
    return nodeScope;
}

int FnDecl::numBytes = 0;
	
FnDecl::FnDecl(Identifier *n, Type *r, List<VarDecl*> *d) : Decl(n) {
    Assert(n != NULL && r!= NULL && d != NULL);
    (returnType=r)->SetParent(this);
    (formals=d)->SetParentAll(this);
    body = NULL;
}

void FnDecl::SetFunctionBody(Stmt *b) { 
    (body=b)->SetParent(this);
}

void FnDecl::Check() {
    returnType->Check();
    if (body) {
        nodeScope = new Scope();
        formals->DeclareAll(nodeScope);
        formals->CheckAll();
	body->Check();
    }
}

void FnDecl::Emit()
{
  numBytes = 0;
  codeGen->OffsetToCurrentLocal = -8;
  //codeGen->OffsetToCurrentParam = 4;

  Node * parent = this->GetParent();
  ClassDecl * classParent = dynamic_cast<ClassDecl *>(parent);
  Program * prog = dynamic_cast<Program *>(parent);

  Decl * declNode = FindDecl(this->GetId());
  FnDecl * fn = dynamic_cast<FnDecl *>(declNode);

  //Special case for main function
  if(fn && strcmp(this->GetId()->GetName(), "main") == 0)
  {
    codeGen->GenLabel(this->GetId()->GetName());

  }
  else if (fn) //Handles all other functions
  {
    char result[255];
    //const char * result2;
    sprintf(result, "_%s", this->GetId()->GetName());
    this->result2 = strdup(result);
       
    while(!classParent && !prog)
    {
      parent = parent->GetParent();
      classParent = dynamic_cast<ClassDecl *>(parent);
      prog = dynamic_cast<Program *>(parent);

    }

    if(!classParent)
    {
       codeGen->OffsetToCurrentParam = 4;
    }
    if(classParent)
    {
       const char * className = classParent->GetId()->GetName();
       char classResult [255];     
       sprintf(classResult, "_%s.%s", className, result2+1);
       this->result2 = strdup(classResult);
       //const char * result3 = strcat(classParent->GetId()->GetName(), result2);
       classParent->fnLabels->Append(this->result2);
    }
    codeGen->GenLabel(result2); 

    for(int i = 0; i < (sizeof(result)/sizeof(*result)); i++)
      result[i] = '\0';
  }

  //Handles the BeginFunc label
  BeginFunc *b = codeGen->GenBeginFunc();

  formals->EmitAll();

  body->Emit();
    
  //Backpatches the frame size into BeginFunc 
  b->SetFrameSize(numBytes);

  //if(strcmp(returnType->GetName(), "void") == 0)
  {
    //Ends the function if there is no return type
    codeGen->GenEndFunc();
  }

}

bool FnDecl::ConflictsWithPrevious(Decl *prev) {
 // special case error for method override
    if (IsMethodDecl() && prev->IsMethodDecl() && parent != prev->GetParent()) { 
        if (!MatchesPrototype(dynamic_cast<FnDecl*>(prev))) {
            ReportError::OverrideMismatch(this);
            return true;
        }
        return false;
    }
    ReportError::DeclConflict(this, prev);
    return true;
}

bool FnDecl::IsMethodDecl() 
  { return dynamic_cast<ClassDecl*>(parent) != NULL || dynamic_cast<InterfaceDecl*>(parent) != NULL; }

bool FnDecl::MatchesPrototype(FnDecl *other) {
    if (!returnType->IsEquivalentTo(other->returnType)) return false;
    if (formals->NumElements() != other->formals->NumElements())
        return false;
    for (int i = 0; i < formals->NumElements(); i++)
        if (!formals->Nth(i)->GetDeclaredType()->IsEquivalentTo(other->formals->Nth(i)->GetDeclaredType()))
            return false;
    return true;
}



