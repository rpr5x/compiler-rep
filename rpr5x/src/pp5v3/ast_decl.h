/* File: ast_decl.h
 * ----------------
 * In our parse tree, Decl nodes are used to represent and
 * manage declarations. There are 4 subclasses of the base class,
 * specialized for declarations of variables, functions, classes,
 * and interfaces.
 *
 * pp5: You will need to extend the Decl classes to implement 
 * code generation for declarations.
 */

#ifndef _H_ast_decl
#define _H_ast_decl

#include "ast.h"
#include "list.h"
#include "tac.h"

class Type;
class NamedType;
class Identifier;
class Stmt;
class FnDecl;
class InterfaceDecl;

class Decl : public Node 
{
  protected:
    Identifier *id;
    Location * declLoc;  

  public:
    Decl(Identifier *name);
    friend std::ostream& operator<<(std::ostream& out, Decl *d) { return out << d->id; }
    Identifier *GetId() { return id; }
    const char *GetName() { return id->GetName(); }
    
    virtual bool ConflictsWithPrevious(Decl *prev);

    virtual bool IsVarDecl() { return false; } // jdz: could use typeid/dynamic_cast for these
    virtual bool IsClassDecl() { return false; }
    virtual bool IsInterfaceDecl() { return false; }
    virtual bool IsFnDecl() { return false; } 
    virtual bool IsMethodDecl() { return false; }
    virtual Location * GetLocationNode() {return declLoc; }
};

class VarDecl : public Decl 
{
  protected:
    Type *type;
    //Location * varLoc;

  public:
    Location * varLoc;
    Location * arrayLengthLoc;
    VarDecl(Identifier *name, Type *type);
    void Check();
    void Emit();
    Type *GetDeclaredType() { return type; }
    Location *GetLocationNode() {return varLoc; }
    Location *GetArrayLength() {return arrayLengthLoc; }
    Location * counterLoc;
    Location *GetCounterLoc() {return counterLoc; } 
};

class ClassDecl : public Decl 
{
  protected:
    List<Decl*> *members;
    NamedType *extends;
    List<NamedType*> *implements;
    Type *cType;
    List<InterfaceDecl*> *convImp;

  public:
    ClassDecl(Identifier *name, NamedType *extends, 
              List<NamedType*> *implements, List<Decl*> *members);
    void Check();
    void Emit();
    bool IsClassDecl() { return true; }
    Scope *PrepareScope();
    List<Decl*> *GetMembers() {return members; }
    List<const char *> *fnLabels;
    List<const char *> *GetFnLabels() {return fnLabels; }
    Location * classLoc;
    Location * GetLocationNode() {return classLoc; }
    Location * thisLoc;
    Location * GetThisNode() {return thisLoc; }
    NamedType * GetExtends() {return extends; }
};

class InterfaceDecl : public Decl 
{
  protected:
    List<Decl*> *members;
    
  public:
    InterfaceDecl(Identifier *name, List<Decl*> *members);
    void Check();
    void Emit();
    bool IsInterfaceDecl() { return true; }
    Scope *PrepareScope();
};

class FnDecl : public Decl 
{

  protected:
    List<VarDecl*> *formals;
    Type *returnType;
    Stmt *body;
    //Location * fnDeclLoc;

  public:
    FnDecl(Identifier *name, Type *returnType, List<VarDecl*> *formals);
    void SetFunctionBody(Stmt *b);
    void Check();
    void Emit();
    bool IsFnDecl() { return true; }
    bool IsMethodDecl();
    bool ConflictsWithPrevious(Decl *prev);
    bool MatchesPrototype(FnDecl *other);
    static int numBytes;
    Type * GetType() {return returnType;}
    Location * fnDeclLoc;
    Location * GetLocationNode() {return fnDeclLoc;}
    const char * result2;
    const char * GetFnLabel() {return result2; } 
    List<VarDecl*> *GetFormals() {return formals; }
};

#endif
