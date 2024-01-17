#include "CodeGen.h"
#include "llvm/ADT/StringMap.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/ADT/STLExtras.h" 
#include "llvm/Support/raw_ostream.h"

  /*

  need list of all variables  depends[][]   a->vector  b->vector  c->vector  [String][Stringlist]

  override assignment

  override declaration

  AST performElimination(){
    traverse 1 of AST -> compute depends     compute depends

    traverse 2 of AST -> tag dead assignments and declerations visit -> false
  }
  
  */

using namespace llvm;

llvm::SmallVector<llvm::StringRef> allVars;
StringMap<llvm::SmallVector<StringRef>> dependsMap;
llvm::SmallVector<llvm::StringRef> deadVars;
llvm::SmallVector<llvm::StringRef> alive;


// Define a visitor class for generating LLVM IR from the AST.
namespace
{
  class IdentifiersCollector : public ASTVisitor
  {
    public:
  
    virtual void visit(GSM &Node) override
    {
      // Iterate over the children of the GSM node and visit each child.
      for (auto I = Node.begin(), E = Node.end(); I != E; ++I)
      {
        (*I)->accept(*this);
      }
    };

    virtual void visit(Declaration &Node) override
    {
      auto Vars_iterator = Node.beginVars();
      for(Vars_iterator;Vars_iterator != Node.endVars();Vars_iterator++)
      {
        allVars.push_back(*Vars_iterator);
      }
    };


    virtual void visit(Assignment &) override {}; 
    virtual void visit(BinaryOp &) override {};
    virtual void visit(Factor &) override {}; 
    virtual void visit(Loop &) override {};
    virtual void visit(IfElse &) override {}; 



    void collect(AST *Tree)
    {
      Tree->accept(*this);
    }

  };

  
  
  class ComputeDepends : public ASTVisitor
  {   
    public:
      llvm::SmallVector<llvm::StringRef> depends; // auxilary variable to store dependencies of variables throughout taversing process of AST

      virtual void visit(GSM &Node) override
    {
      // Iterate over the children of the GSM node and visit each child.
      for (auto I = Node.begin(), E = Node.end(); I != E; ++I)
      {
        (*I)->accept(*this);
      }
    };

      virtual void visit(Declaration &Node) override
      {
        auto expression = Node.beginExprs();
        auto var = Node.beginVars();
        StringRef varName = *var;

        if (expression != Node.endExprs())
        {
          (*expression)->accept(*this);

          llvm::SmallVector<llvm::StringRef> tempDepends(depends.begin(), depends.end());
          // map[var] = depends
          dependsMap[varName] = tempDepends;
          depends.clear();
        }
        else
        {
          //do nothing since no dependency is found
          // depends.clear();
        }

      };
      
      virtual void visit(Factor &Node) override
      {
        if (Node.getKind() == Factor::Ident)
        {
          // if is Ident add the var to depends

          llvm::StringRef var = Node.getVal();

          if (llvm::find(depends, var) == depends.end())
          {
            // If it's not in depends, add it
            depends.push_back(var);
          }
        }
      };

      virtual void visit(BinaryOp &Node) override
      {
        Node.getLeft()->accept(*this);
        Node.getRight()->accept(*this);
      };

      virtual void visit(Assignment &Node) override 
      {
        auto var = Node.getLeft()->getVal();
        auto operation = Node.getOperator();
        if(operation == Assignment::Eq)
        {
          dependsMap[var].clear();
          Node.getRight()->accept(*this);

          //map[var] = depends
          dependsMap[var] = depends;
          //depends.clear
          depends.clear();
        }
        else// += -= etc
        {
          Node.getRight()->accept(*this);
          dependsMap[var].insert(dependsMap[var].end(), depends.begin(), depends.end());
          depends.clear();

        }
      
      };

      virtual void visit(Loop &) override {};
      virtual void visit(IfElse &) override {};


    void compute(AST *Tree)
      {
        // Initialize dependsMap with keys from allVars
        for (const auto &var : allVars)
          {
          dependsMap[var] = llvm::SmallVector<StringRef>();//HERE
          }
      Tree->accept(*this);
      }


  };

  class ToIRVisitor : public ASTVisitor
  {
    Module *M;// easy IR generation
    IRBuilder<> Builder;
    Type *VoidTy;
    Type *Int32Ty;
    Type *Int8PtrTy;
    Type *Int8PtrPtrTy;
    Constant *Int32Zero;
    Function *MainFn;
    Value *V; // current calculated value updated through tree traversal
    StringMap<AllocaInst *> nameMap;// maps a variable name to the value that's returned by calc_read()
    FunctionType *CalcWriteFnTy;
    Function *CalcWriteFn;


    //llvm::SmallVector<llvm::StringRef> allVars;


  public:
    // Constructor for the visitor class.
    ToIRVisitor(Module *M) : M(M), Builder(M->getContext())
    {
      // Initialize LLVM types and constants.
      VoidTy = Type::getVoidTy(M->getContext());
      Int32Ty = Type::getInt32Ty(M->getContext());
      Int8PtrTy = Type::getInt8PtrTy(M->getContext());
      Int8PtrPtrTy = Int8PtrTy->getPointerTo();
      Int32Zero = ConstantInt::get(Int32Ty, 0, true);
    }
    
    // Entry point for generating LLVM IR from the AST.
    void run(AST *Tree)
    {
      // Create the main function with the appropriate function type.
      FunctionType *MainFty = FunctionType::get(Int32Ty, {Int32Ty, Int8PtrPtrTy}, false);
      MainFn = Function::Create(MainFty, GlobalValue::ExternalLinkage, "main", M);

      // Create a basic block for the entry point of the main function.
      BasicBlock *BB = BasicBlock::Create(M->getContext(), "entry", MainFn);
      Builder.SetInsertPoint(BB);

      // Visit the root node of the AST to generate IR.
      // begin the AST traversal 
      Tree->accept(*this);

      // Create a return instruction at the end of the main function.
      Builder.CreateRet(Int32Zero);
    }

    // Visit function for the GSM node in the AST.
    virtual void visit(GSM &Node) override
    {
      // Iterate over the children of the GSM node and visit each child.
      for (auto I = Node.begin(), E = Node.end(); I != E; ++I)
      {
        (*I)->accept(*this);
      }
    };

    virtual void visit(Assignment &Node) override
    {
      // Visit the right-hand side of the assignment and get its value.
      Node.getRight()->accept(*this);
      Value *val = V;

      // Get the name of the variable being assigned.
      auto varName = Node.getLeft()->getVal();
      bool isDead = false;
      if(!(llvm::find(deadVars, varName) == deadVars.end()))
      {
        isDead = true;
      }

      if(!isDead)
      {

        if(val != nullptr)// if right side included a dead variable ignore the assignment
        {
          Value *var_value = Builder.CreateLoad(Int32Ty,nameMap[Node.getLeft()->getVal()]); // ex)a += 2;  -> first we should the current value of a
          Value *temp;

          switch (Node.getOperator())
          {
            case Assignment::Eq:
              break;
            case Assignment::PlEq:
              temp = Builder.CreateNSWAdd(var_value,val);
              val = temp;
              break;
            case Assignment::MulEq:
              temp = Builder.CreateNSWMul(var_value,val);
              val = temp;
              break;
            case Assignment::DivEq:
              temp = Builder.CreateSDiv(var_value,val);
              val = temp;
              break;        
            case Assignment::ModEq:
              temp = Builder.CreateURem(var_value,val);
              val = temp;
              break;
            case Assignment::MinEq:
              temp = Builder.CreateNSWSub(var_value,val);
              val = temp;
              break;
          }

          // Create a store instruction to assign the value to the variable.
          Builder.CreateStore(val, nameMap[varName]);

          // Create a function type for the "gsm_write" function.
          CalcWriteFnTy = FunctionType::get(VoidTy, {Int32Ty}, false);
          // Create a function declaration for the "gsm_write" function.
          CalcWriteFn = Function::Create(CalcWriteFnTy, GlobalValue::ExternalLinkage, "gsm_write", M);

          // Create a call instruction to invoke the "gsm_write" function with the value.
          CallInst *Call = Builder.CreateCall(CalcWriteFnTy, CalcWriteFn, {val});
        }
      }
    };

    virtual void visit(Factor &Node) override
    {

      if (Node.getKind() == Factor::Ident)
      {

      if (llvm::find(deadVars, Node.getVal()) == deadVars.end()) 
        {
        // If the factor is an identifier, load its value from memory.
        V = Builder.CreateLoad(Int32Ty, nameMap[Node.getVal()]);
        }
        else
        {
          V = nullptr;
        }
      }
      else
      {
        // If the factor is a literal, convert it to an integer and create a constant.
        int intval;
        Node.getVal().getAsInteger(10, intval);
        V = ConstantInt::get(Int32Ty, intval, true);
      }
    };

    virtual void visit(BinaryOp &Node) override
    {
      // Visit the left-hand side of the binary operation and get its value.
      Node.getLeft()->accept(*this);
      Value *Left = V;     
      
      
      // Visit the right-hand side of the binary operation and get its value.
      Node.getRight()->accept(*this);
      Value *Right = V;

      if(Left != nullptr && Right != nullptr)
      {
        // Perform the binary operation based on the operator type and create the corresponding instruction.
        switch (Node.getOperator())
      {
      case BinaryOp::Or:
        V = Builder.CreateOr(Left, Right);
        break;
      case BinaryOp::And:
        V = Builder.CreateAnd(Left, Right);
        break;
      case BinaryOp::IsEq:
        V = Builder.CreateICmpEQ(Left, Right);
        break;
      case BinaryOp::IsNEq:
        V = Builder.CreateICmpNE(Left, Right);
        break;
      case BinaryOp::GrEq:
        V = Builder.CreateICmpSGE(Left, Right);
        break;
      case BinaryOp::LoEq:
        V = Builder.CreateICmpSLE(Left, Right);
        break;
      case BinaryOp::Gr:
        V = Builder.CreateICmpSGT(Left, Right);
        break;
      case BinaryOp::Lo:
        V = Builder.CreateICmpSLT(Left, Right);
        break;
      case BinaryOp::Plus:
        V = Builder.CreateNSWAdd(Left, Right);
        break;
      case BinaryOp::Minus:
        V = Builder.CreateNSWSub(Left, Right);
        break;
      case BinaryOp::Mul:
        V = Builder.CreateNSWMul(Left, Right);
        break;
      case BinaryOp::Div:
        V = Builder.CreateSDiv(Left, Right);
        break;
      case BinaryOp::Mod:
        V = Builder.CreateSRem(Left, Right);
        break;
      case BinaryOp::Pow: //ERROR
        auto *intConstant = dyn_cast<ConstantInt>(Right);
        int iterations = intConstant->getSExtValue();
        Value *NewLeft = Left;

        for (int i = 0; i < iterations - 1; i++)
        {
          Left = Builder.CreateNSWMul(Left, NewLeft);
        }

        V = Left;
        break;
      }
      }
      else
      {
        V = nullptr;
      }
    };

    virtual void visit(Declaration &Node) override
    {

      auto Exprs_iterator = Node.beginExprs();
      auto Vars_iterator = Node.beginVars();
      /* TODO check if we should ignore this node
      if a dead variable exists in Exprs-iterator or Vars_iterator -> ignore this node
      */

     //by the end of this loop we have assigned each declared variable with corresponding expression value

      StringRef leftSide = *Vars_iterator;
      bool isDead = false;
      if (!(llvm::find(deadVars, leftSide) == deadVars.end())) 
      {
        isDead = true;
      }
      
      if(!isDead)
      {
        for(Exprs_iterator;Exprs_iterator != Node.endExprs();++Exprs_iterator,++Vars_iterator)
        {
              (*Exprs_iterator)->accept(*this);
              Value *val = V; //V will get assigned with the final value of expression which could be assignment-BinaryOpration etc..
              StringRef Var = *Vars_iterator;
              if(val != nullptr)
              {
                nameMap[Var] = Builder.CreateAlloca(Int32Ty);
                Builder.CreateStore(val,nameMap[Var]);
              }
              else//just declare0
              {
                Value *zero = ConstantInt::get(Int32Ty,0,true);
                nameMap[Var] = Builder.CreateAlloca(Int32Ty);
                Builder.CreateStore(zero,nameMap[Var]);
              }
        }
        // instanciate remaining declared variables with 0
        for(Vars_iterator;Vars_iterator != Node.endVars();Vars_iterator++)
        {
              Value *zero = ConstantInt::get(Int32Ty,0,true);
              StringRef Var = *Vars_iterator;
              nameMap[Var] = Builder.CreateAlloca(Int32Ty);
              Builder.CreateStore(zero,nameMap[Var]); // I think insted of zero we could use 'Int32Zero'
        } 
      }
    };
    
    virtual void visit(IfElse &Node) override
    {
      // Create basic blocks for if, elif, else, and merge
      
      BasicBlock *MergeBB = BasicBlock::Create(M->getContext(), "merge", MainFn);
      BasicBlock *ElseBB = nullptr;

      if(Node.getHasElse())//if we have an else statment we create its BB
      {
        ElseBB = BasicBlock::Create(M->getContext(), "else", MainFn); 
      }
      // Iterate through each expression and corresponding assignments
      auto exprIterator = Node.beginExprs();
      auto assignIterator = Node.beginAssigns2D();
      
      BasicBlock *IfBB = BasicBlock::Create(M->getContext(), "if", MainFn);// for checking conditions
      BasicBlock *IfNotMetBB = BasicBlock::Create(M->getContext(), "if.not.met", MainFn);
      BasicBlock *AssignBB = BasicBlock::Create(M->getContext(), "assign", MainFn);

      Builder.CreateBr(IfBB);//check if condition

      Builder.SetInsertPoint(IfBB);
      // setCurr(IfBB);
      // Eevaluate the condition
      (*exprIterator)->accept(*this);
      Value *Condition = V;

      Builder.CreateCondBr(Condition,AssignBB,IfNotMetBB);

      Builder.SetInsertPoint(AssignBB);
      // setCurr(AssignBB); 
      // do the required assignments
      auto IfAssignments = *assignIterator; //first row of 2D vector
      for(auto a = IfAssignments.begin(); a != IfAssignments.end();++a)// a is represents each assignment in the first row
      {
        (*a)->accept(*this);// do each assignment in the if statement   
      }
      // goto merge BB because the whole ifElse node is performed
      Builder.CreateBr(MergeBB);  
      // end of assignment BB
      
      Builder.SetInsertPoint(IfNotMetBB);
      // setCurr(IfNotMetBB);

      // if we have iterated all expressions if and all elif statements have been checked
      // so we should either perfrom else statement or merge
      if(exprIterator == Node.endExprs())
      {
        if(Node.getHasElse())
        {
          ++assignIterator;
          Builder.CreateBr(AssignBB);
        }
        else
        {
          Builder.CreateBr(MergeBB);
        }
      }

      ++exprIterator;
      ++assignIterator;

      Builder.CreateBr(IfBB);
      // end of IfNotMet BB


      Builder.SetInsertPoint(MergeBB);
      // setCurr(MergeBB);
      };

    virtual void visit(Loop &Node) override
    {  
      BasicBlock *LoopCondBB = BasicBlock::Create(M->getContext(), "loop.cond", MainFn);
      BasicBlock *LoopBodyBB = BasicBlock::Create(M->getContext(), "loop.body", MainFn);
      BasicBlock *AfterLoopBB = BasicBlock::Create(M->getContext(), "after.loop", MainFn);
      
      // Emit LLVM IR instructions
      Builder.CreateBr(LoopCondBB);
      Builder.SetInsertPoint(LoopCondBB);
      // setCurr(LoopCondBB);

      // Assuming there's a function to emit the loop condition expression
      Node.getCondition()->accept(*this);
      Value *Condition = V;
      Builder.CreateCondBr(Condition, LoopBodyBB, AfterLoopBB);
      
      Builder.SetInsertPoint(LoopBodyBB);
      // setCurr(LoopBodyBB);

      // accept statements within the loop body
      
      auto assignment_iterator = Node.begin(); //  Node.begin() retrieves loop assignments/statements
      for(assignment_iterator;assignment_iterator != Node.end();++assignment_iterator)
      {
        (*assignment_iterator)->accept(*this);
      }
      Builder.CreateBr(LoopCondBB);//current = LoopbodyBB -> we want to branch to LoopCondBB

      Builder.SetInsertPoint(AfterLoopBB);
      // setCurr(AfterLoopBB);
    };
  };
}; // namespace



// new funciton added
void CodeGen::collectIdentifiers(AST *Tree)
{
  IdentifiersCollector IdentifierCollector;
  IdentifierCollector.collect(Tree);
}


void CodeGen::computeDepends(AST *Tree){
  ComputeDepends computeDepends;
  computeDepends.compute(Tree);
  
}

// initialize deadVars
void CodeGen::computeDead()
{
  void addDependenciesRecursive(const llvm::StringRef &variable, llvm::SmallVector<llvm::StringRef> &alive);

  llvm::SmallVector<llvm::StringRef> resultDepends = dependsMap["result"];//error prone
  for(const auto &variable : resultDepends)
  {
    addDependenciesRecursive(variable, alive);
  }

  for(const auto &variable : allVars)
  {
    // Check if var is not in alive
    if (llvm::find(alive,variable ) == alive.end()) 
    {
        // Add var to deadVars
        if(variable != "result")
        {
          deadVars.push_back(variable);
        }
    }
  }
  for (const auto &var : deadVars)
  {
    llvm::outs() << "dead " << var << "\n";
  }

}

//auxiliary function to perfrom the recursice algorithm
void addDependenciesRecursive(const llvm::StringRef &variable, llvm::SmallVector<llvm::StringRef> &alive) {
    // Check if the variable is already in the 'alive' vector to avoid duplicates
    if (llvm::find(alive, variable) == alive.end()) {
        // Add the variable to 'alive'
        alive.push_back(variable);

        // Recursively add dependencies
        const auto &dependencies = dependsMap[variable];
        for (const auto &dependency : dependencies) {
            addDependenciesRecursive(dependency, alive);
        }
    }
}

void CodeGen::compile(AST *Tree)
{
  // Create an LLVM context and a module.
  LLVMContext Ctx;
  Module *M = new Module("calc.expr", Ctx);

  // Create an instance of the ToIRVisitor and run it on the AST to generate LLVM IR.
  ToIRVisitor ToIR(M);
  ToIR.run(Tree);

  // Print the generated module to the standard output.
  M->print(outs(), nullptr);
}
