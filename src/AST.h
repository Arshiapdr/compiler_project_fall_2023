#ifndef AST_H
#define AST_H

#include "llvm/ADT/SmallVector.h"
#include "llvm/ADT/StringRef.h"

// Forward declarations of classes used in the AST
class AST;
class Expr;
class GSM;
class Factor;
class BinaryOp;
class Assignment;
class Declaration;
class IfElse;
class Loop;

// ASTVisitor class defines a visitor pattern to traverse the AST
class ASTVisitor
{
public:
  // Virtual visit functions for each AST node type
  virtual void visit(AST &) {}               // Visit the base AST node
  virtual void visit(Expr &) {}              // Visit the expression node
  virtual void visit(GSM &) = 0;             // Visit the group of expressions node
  virtual void visit(Factor &) = 0;          // Visit the factor node
  virtual void visit(BinaryOp &) = 0;        // Visit the binary operation node
  virtual void visit(Assignment &) = 0;      // Visit the assignment expression node
  virtual void visit(Declaration &) = 0;     // Visit the variable declaration node
  virtual void visit(IfElse &) = 0;          // Visit the ifelse node
  virtual void visit(Loop &) = 0;            // Visit the loop node
};


// AST class serves as the base class for all AST nodes
class AST
{
public:
  virtual ~AST() {}
  virtual void accept(ASTVisitor &V) = 0; // Accept a visitor for traversal
};

// Expr class represents an expression in the AST
class Expr : public AST
{
public:
  Expr() {}
};

// GSM class represents a group of expressions in the AST
class GSM : public Expr
{
  using ExprVector = llvm::SmallVector<Expr *>;

private:
  ExprVector exprs; // Stores the list of expressions

public:
  GSM(llvm::SmallVector<Expr *> exprs) : exprs(exprs) {}

  llvm::SmallVector<Expr *> getExprs() { return exprs; }

  ExprVector::const_iterator begin() { return exprs.begin(); }

  ExprVector::const_iterator end() { return exprs.end(); }

  virtual void accept(ASTVisitor &V) override
  {
    V.visit(*this);
  }
};

// Factor class represents a factor in the AST (either an identifier or a number)
class Factor : public Expr
{
public:
  enum ValueKind
  {
    Ident,
    Number
  };

private:
  ValueKind Kind; // Stores the kind of factor (identifier or number)
  llvm::StringRef Val; // Stores the value of the factor

public:
  Factor(ValueKind Kind, llvm::StringRef Val) : Kind(Kind), Val(Val) {}

  ValueKind getKind() { return Kind; }

  llvm::StringRef getVal() { return Val; }

  virtual void accept(ASTVisitor &V) override
  {
    V.visit(*this);
  }
};

// BinaryOp class represents a binary operation in the AST (plus, minus, multiplication, division)
class BinaryOp : public Expr
{
public:
  enum Operator
  {
    Or,
    And,
    IsEq,
    IsNEq,
    GrEq,
    LoEq,
    Gr,
    Lo,
    Plus,
    Minus,
    Mul,
    Div,
    Mod,
    Pow
  };

private:
  Expr *Left; // Left-hand side expression
  Expr *Right; // Right-hand side expression
  Operator Op; // Operator of the binary operation

public:
  BinaryOp(Operator Op, Expr *L, Expr *R) : Op(Op), Left(L), Right(R) {}

  Expr *getLeft() { return Left; }

  Expr *getRight() { return Right; }

  Operator getOperator() { return Op; }

  virtual void accept(ASTVisitor &V) override
  {
    V.visit(*this);
  }
};

// Assignment class represents an assignment expression in the AST
class Assignment : public Expr
{
public:
  enum Operator
  {
    Eq,
    PlEq,
    MulEq,
    DivEq,
    MinEq,
    ModEq
  };
  
private:
  Factor *Left; // Left-hand side factor (identifier)
  Expr *Right; // Right-hand side expression
  Operator Op; // Operator of the assignment operation

public:
  Assignment(Operator Op, Factor *L, Expr *R) : Op(Op), Left(L), Right(R) {}

  Factor *getLeft() { return Left; }

  Expr *getRight() { return Right; }

  Operator getOperator() { return Op; }

  virtual void accept(ASTVisitor &V) override
  {
    V.visit(*this);
  }
};

// Declaration class represents a variable declaration with an initializer in the AST
class Declaration : public Expr
{
  using VarVector = llvm::SmallVector<llvm::StringRef, 8>;
  using ExprVector = llvm::SmallVector<Expr *>;

  VarVector Vars; // Stores the list of variables
  ExprVector Exprs; // Stores the list of expressions   
  // boolean visit = True + getter

public:
  Declaration(llvm::SmallVector<llvm::StringRef, 8> Vars, llvm::SmallVector<Expr *> Exprs) : Vars(Vars), Exprs(Exprs) {}

  VarVector::const_iterator beginVars() { return Vars.begin(); }

  VarVector::const_iterator endVars() { return Vars.end(); }

  ExprVector::const_iterator beginExprs() { return Exprs.begin(); }

  ExprVector::const_iterator endExprs() { return Exprs.end(); }

  virtual void accept(ASTVisitor &V) override
  {
    V.visit(*this);
  }
};

// IfElse class represents a condition in the AST
class IfElse : public Expr
{
  using ExprVector = llvm::SmallVector<Expr *>;
  using Assign2DVector = llvm::SmallVector<llvm::SmallVector<Assignment *>>;

  ExprVector Exprs; // Stores the list of expressions   
  Assign2DVector Assigns; // Stores the 2d array of assignments  
  bool hasElse = false; // to check if the node has else statement NEW

public:
  //IfElse(llvm::SmallVector<Expr *> Exprs, llvm::SmallVector<llvm::SmallVector<Assignment *>> Assigns,) : Exprs(Exprs), Assigns(Assigns) {}
  IfElse(ExprVector Exprs, Assign2DVector Assigns,bool hasElse) : Exprs(Exprs), Assigns(Assigns),hasElse(hasElse) {}

  ExprVector::const_iterator beginExprs() { return Exprs.begin(); }

  ExprVector::const_iterator endExprs() { return Exprs.end(); }

  // ERROR PRONE
  Assign2DVector::const_iterator beginAssigns2D() { return Assigns.begin(); }

  Assign2DVector::const_iterator endAssigns2D() { return Assigns.end(); }

  bool getHasElse() {return hasElse;};//NEW

  virtual void accept(ASTVisitor &V) override
  {
    V.visit(*this);
  }
};

// Loop class represents a loop in the AST
class Loop : public Expr
{
  using AssignVector = llvm::SmallVector<Assignment *>; 
  AssignVector Assigns; // Stores the list of assignments  
  Expr *E; // Expression

public:
  Loop(Expr *E, llvm::SmallVector<Assignment *> Assigns) : E(E), Assigns(Assigns) {}

  Expr *getCondition() { return E; }

  AssignVector::const_iterator begin() { return Assigns.begin(); }

  AssignVector::const_iterator end() { return Assigns.end(); }

  virtual void accept(ASTVisitor &V) override
  {
    V.visit(*this);
  }
};

#endif
