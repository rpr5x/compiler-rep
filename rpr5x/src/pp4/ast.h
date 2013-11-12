/* File: ast.h
 * ----------- 
 * This file defines the abstract base class Node and the concrete 
 * Identifier and Error node subclasses that are used through the tree as 
 * leaf nodes. A parse tree is a hierarchical collection of ast nodes (or, 
 * more correctly, of instances of concrete subclassses such as VarDecl,
 * ForStmt, and AssignExpr).
 * 
 * Location: Each node maintains its lexical location (line and columns in 
 * file), that location can be NULL for those nodes that don't care/use 
 * locations. The location is typcially set by the node constructor.  The 
 * location is used to provide the context when reporting semantic errors.
 *
 * Parent: Each node has a pointer to its parent. For a Program node, the 
 * parent is NULL, for all other nodes it is the pointer to the node one level
 * up in the parse tree.  The parent is not set in the constructor (during a 
 * bottom-up parse we don't know the parent at the time of construction) but 
 * instead we wait until assigning the children into the parent node and then 
 * set up links in both directions. The parent link is typically not used 
 * during parsing, but is more important in later phases.
 *
 * Semantic analysis: For pp3 you are adding "Check" behavior to the ast
 * node classes. Your semantic analyzer should do an inorder walk on the
 * parse tree, and when visiting each node, verify the particular
 * semantic rules that apply to that construct.

 */

#ifndef _H_ast
#define _H_ast

#include <stdlib.h>   // for NULL
#include "location.h"
#include <iostream>
#include "list.h"

class Node 
{
  protected:
    yyltype *location;
    Node *parent;
    List<Node *> *branches;

  public:
    Node(yyltype loc);
    Node();
    
    yyltype *GetLocation()               { return location;   }
    void SetParent(Node *p)              { parent = p;        }
    Node *GetParent()                    { return parent;     }
    virtual List<Node *>  *GetBranchList() { return branches;    }
    void SetBranch(int i, Node *n)       { branches->InsertAt(n, i);}
    virtual void checkNode() =0;//Pure virtual function
    virtual void checkNode2() =0; //Second pass 

};
   

class Identifier : public Node 
{
  protected:
    char *name;
    List<Node *> *branches;
    int level; 
  public:
    Identifier(yyltype loc, const char *name);
    Identifier(const char *name);  //Dummy identifier needed for some error messages
    friend std::ostream& operator<<(std::ostream& out, Identifier *id) { return out << id->name; }
    char * getName() const;
    void checkNode();
    void checkNode2();
    List<Node *> * GetBranchList() {return branches;}
    int getLevel() {return level;}
    void setName(char * name) {this->name = name;}
};


// This node class is designed to represent a portion of the tree that 
// encountered syntax errors during parsing. The partial completed tree
// is discarded along with the states being popped, and an instance of
// the Error class can stand in as the placeholder in the parse tree
// when your parser can continue after an error.
class Error : public Node
{
  public:
    Error() : Node() {}
    void checkNode() {}
    void checkNode2() {}
};



#endif