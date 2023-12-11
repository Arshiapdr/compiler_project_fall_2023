#ifndef AST_H
#define AST_H

#include "llvm/ADT/SmallVector.h"
#include "llvm/ADT/StringRef.h"

class AST;
class Expr;
class Factor;
class BinaryOp;
class WithDecl;
class Assignment;
class Group;
/*
In C++, = 0 after a virtual function declaration signifies that the function is a pure virtual function.
A pure virtual function is a virtual function that has no implementation in the base class and must be overridden
by any concrete (non-abstract) derived class. A class containing at least one pure virtual function becomes an abstract class,
meaning it cannot be instantiated directly; it serves as a base class
for other classes that provide implementations for all its pure virtual functions.
*/
class ASTVisitor {
public:
    virtual void visit(AST&) {};
    virtual void visit(Expr&) {};
    virtual void visit(Group&) = 0;
    virtual void visit(Factor&) = 0;
    virtual void visit(BinaryOp&) = 0;
    virtual void visit(WithDecl&) = 0;
};
/*
*     Abstract base class for all AST nodes.
    Contains a pure virtual function accept that accepts an ASTVisitor
    as an argument. It's meant to be overridden in derived classes.
*/

class AST {
public:
    virtual ~AST() {}// When a class is designed to be inherited
    //(as AST appears to be in the provided code), it's a good 
    //practice to declare its destructor as virtual.
    //This allows proper destruction of derived class objects
    //when they are destroyed through a pointer to the base class.
    virtual void accept(ASTVisitor& V) = 0;
};

class Expr : public AST {
public:
    Expr() {}
};
class Group : public Expr
{
    using ExprVector = llvm::SmallVector<Expr*>;

private:
    ExprVector exprs;                          // Stores the list of expressions

public:
    Group(llvm::SmallVector<Expr*> exprs) : exprs(exprs) {}

    llvm::SmallVector<Expr*> getExprs() { return exprs; }

    ExprVector::const_iterator begin() { return exprs.begin(); }

    ExprVector::const_iterator end() { return exprs.end(); }

    virtual void accept(ASTVisitor& V) override
    {
        V.visit(*this);
    }
};
class Factor : public Expr {
public:
    enum ValueKind { Ident, Number };

private:
    ValueKind Kind;
    llvm::StringRef Val;


    /*
    *  In this language numbers and variables are treated almost identically,
       so we decided to create only one AST node class to represent them.
       The main reason is we only have 1 type variables (int) and everywhere in
       out language when we have [a Z] it is defenitley a reference to a variable.
    */
public:
    Factor(ValueKind Kind, llvm::StringRef Val) : Kind(Kind), Val(Val)
    {}
    ValueKind getKind() {
        return Kind;
    }
    llvm::StringRef getVal() {
        return Val;
    }
    virtual void accept(ASTVisitor& V) override {
        V.visit(*this);
    }
};
// represents arithmetic expression
class BinaryOp : public Expr {
public:
    enum Operator { Plus, Minus, Mul, Div,equal,
    plus_equal,mul_equal,div_equal,minus_equal,
    logical_or,logical_and,is_equal,is_not_equal,
    soft_comp_greater,soft_comp_lower,
    hard_comp_greater,soft_comp_lower,
    modulo,powerr,l_paren,r_paren,
    mod_equal, hard_comp_lower
    };

private:
    Expr* Left;
    Expr* Right;
    Operator Op;

public:
    BinaryOp(Operator Op, Expr* L, Expr* R) : Op(Op), Left(L), Right(R)
    {}
    Expr* getLeft()
    {
        return Left;
    }
    Expr* getRight()
    {
        return Right;
    }
    Operator getOperator()
    {
        return Op;
    }
    virtual void accept(ASTVisitor& V) override
    {
        V.visit(*this);
    }
};
//Represents a declaration node in the AST,
//where variables are declared with an expression.
class WithDecl : public AST
{
    //type definition
    using VarVector = llvm::SmallVector<llvm::StringRef, 8>;
    VarVector Vars;
    Expr* E;

public:
    WithDecl(llvm::SmallVector<llvm::StringRef, 8> Vars, Expr* E) : Vars(Vars), E(E)
    {}
    VarVector::const_iterator begin()
    {
        return Vars.begin();
    }
    VarVector::const_iterator end()
    {
        return Vars.end();
    }
    Expr* getExpr()
    {
        return E;
    }
    virtual void accept(ASTVisitor& V) override
    {
        V.visit(*this);
    }
    class Assignment : public Expr
{
private:
  Factor *Left;                             // Left-hand side factor (identifier)
  Expr *Right;                              // Right-hand side expression

public:
  Assignment(Factor *L, Expr *R) : Left(L), Right(R) {}

  Factor *getLeft() { return Left; }

  Expr *getRight() { return Right; }

  virtual void accept(ASTVisitor &V) override
  {
    V.visit(*this);
  }
};
    class Assignment : public Expr
    {
    private:
        Factor* Left;                             // Left-hand side factor (identifier)
        Expr* Right;                              // Right-hand side expression

    public:
        Assignment(Factor* L, Expr* R) : Left(L), Right(R) {}

        Factor* getLeft() { return Left; }

        Expr* getRight() { return Right; }

        virtual void accept(ASTVisitor& V) override
        {
            V.visit(*this);
        }
    };


};

#endif
