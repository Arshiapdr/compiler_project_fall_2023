#include "Sema.h"
#include "llvm/ADT/StringSet.h"
#include "llvm/Support/raw_ostream.h"

namespace {
class InputCheck : public ASTVisitor {
// StringSet to store declared variables
  llvm::StringSet<> Scope; 
// Flag to indicate if an error occurred
  bool HasError; 
// Enum to represent error types: Twice - variable declared twice, Not - variable not declared
  enum ErrorType { Twice, Not }; 
  void error(ErrorType ET, llvm::StringRef V) {
    // Function to report errors
    llvm::errs() << "Variable " << V << " is "
                 << (ET == Twice ? "already" : "not")
                 << " declared\n";
    // Set error flag to true
    HasError = true; 
  }

public:
// Constructor
  InputCheck() : HasError(false) {} 

// Function to check if an error occurred
  bool hasError() { return HasError; } 

  // Visit function for GSM nodes
  virtual void visit(GSM &Node) override { 
    for (auto I = Node.begin(), E = Node.end(); I != E; ++I)
    {
      // Visit each child node
      (*I)->accept(*this); 
    }
  };

  // Visit function for Factor nodes
  virtual void visit(Factor &Node) override {
    if (Node.getKind() == Factor::Ident) {
      // Check if identifier is in the scope
      if (Scope.find(Node.getVal()) == Scope.end())
        error(Not, Node.getVal());
    }
  };

  // Visit function for BinaryOp nodes
  virtual void visit(BinaryOp &Node) override {
    if (Node.getLeft())
      Node.getLeft()->accept(*this);
    else
      HasError = true;

    auto right = Node.getRight();
    if (right)
      right->accept(*this);
    else
      HasError = true;

    if (Node.getOperator() == BinaryOp::Operator::Div && right) {
      Factor * f = (Factor *)right;

      if (right && f->getKind() == Factor::ValueKind::Number) {
        int intval;
        f->getVal().getAsInteger(10, intval);

        if (intval == 0) {
          llvm::errs() << "Division by zero is not allowed." << "\n";
          HasError = true;
        }
      }
    }
  };

  // Visit function for Assignment nodes
  virtual void visit(Assignment &Node) override {
    Factor *dest = Node.getLeft();

    dest->accept(*this);

    if (dest->getKind() == Factor::Number) {
        llvm::errs() << "Assignment destination must be an identifier.";
        HasError = true;
    }

    if (dest->getKind() == Factor::Ident) {
      // Check if the identifier is in the scope
      if (Scope.find(dest->getVal()) == Scope.end())
        error(Not, dest->getVal());
    }

    if (Node.getRight())
      Node.getRight()->accept(*this);
  };

  virtual void visit(Declaration &Node) override {
    for (auto I = Node.begin(), E = Node.end(); I != E;
         ++I) {
      if (!Scope.insert(*I).second)
        // If the insertion fails (element already exists in Scope), report a "Twice" error
        error(Twice, *I); 
    }
    if (Node.getExpr())
      // If the Declaration node has an expression, recursively visit the expression node
      Node.getExpr()->accept(*this); 
  };
};
}

bool Sema::semantic(AST *Tree) {
  if (!Tree)
    // If the input AST is not valid, return false indicating no errors
    return false; 

   // Create an instance of the InputCheck class for semantic analysis
  InputCheck Check;
   // Initiate the semantic analysis by traversing the AST using the accept function
  Tree->accept(Check);

   // Return the result of Check.hasError() indicating if any errors were detected during the analysis
  return Check.hasError();
}
