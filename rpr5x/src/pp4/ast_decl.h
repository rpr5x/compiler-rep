/* File: ast_decl.h
 * ----------------
 * In our parse tree, Decl nodes are used to represent and
 * manage declarations. There are 4 subclasses of the base class,
 * specialized for declarations of variables, functions, classes,
 * and interfaces.
 *
 * pp3: You will need to extend the Decl classes to implement 
 * semantic processing including detection of declaration conflicts 
 * and managing scoping issues.
 */

#ifndef _H_ast_decl
#define _H_ast_decl

#include "ast.h"
#include "list.h"

class Type;
class NamedType;
class Identifier;
class Stmt;


class Decl : public Node 
{
  protected:
    Identifier *id;
    bool checkDecls(Decl * d, Identifier * n);
    List<Node *>  *branches; 
  public:
    Decl(Identifier *name);
    friend std::ostream& operator<<(std::ostream& out, Decl *d) { return out << d->id; }
    virtual ~Decl();
    Identifier * getIdentifier();
    void checkNode();
    void checkNode2();
   // int level;//Change this to private/protected later
    virtual List<Node *> * GetBranchList() {return branches;}
};

class VarDecl : public Decl 
{
  protected:
    Type *type;
    List<Node *> *branches;
 
  public:
    int level;//Change this to private/protected later
    VarDecl(Identifier *name, Type *type);
    Type* getType();
    void checkNode();
    void checkNode2();
    List<Node *> * GetBranchList() {return branches;}
};

class ClassDecl : public Decl 
{
  protected:
    List<Decl*> *members;
    NamedType *extends;
    List<NamedType*> *implements;
    List <Node *> *branches;
  public:
    ClassDecl(Identifier *name, NamedType *extends, 
              List<NamedType*> *implements, List<Decl*> *members);
    int level;//Change this to private/protected later
    List<Decl*> * getMembers();
    void checkNode();
    void checkNode2();
    List<Node *> * GetBranchList() {return branches;}
    NamedType * getExtends() {return this->extends;}
    List<NamedType *> *getImplements() {return this->implements;}
    
};

class InterfaceDecl : public Decl 
{
  protected:
    List<Decl*> *members;
    List<Node*> *branches;
  public:
    InterfaceDecl(Identifier *name, List<Decl*> *members);
    int level;//Change this to protected/private later
    void checkNode();
    void checkNode2();
    List<Node *> * GetBranchList() {return branches;}
    List<Decl *> * getMembers() {return members;}
};

class FnDecl : public Decl 
{
  protected:
    List<VarDecl*> *formals;
    Type *returnType;
    Stmt *body;
    List<Node*> *branches;    
  public:
    FnDecl(Identifier *name, Type *returnType, List<VarDecl*> *formals);
    void SetFunctionBody(Stmt *b);
    int level;
    Type * getReturnType();
    List<VarDecl*> * getFormals();
    void checkNode();
    void checkNode2();
    List<Node *> * GetBranchList() {return branches;}
    Stmt * getBody() {return body;}
};

#endif
