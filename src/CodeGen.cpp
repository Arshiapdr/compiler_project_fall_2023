#include "CodeGen.h"
#include "llvm/ADT/StringMap.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/Support/raw_ostream.h"

using namespace llvm;

// Define a visitor class for generating LLVM IR from the AST.
namespace
{
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
    };

    virtual void visit(Factor &Node) override
    {
      if (Node.getKind() == Factor::Ident)
      {
        // If the factor is an identifier, load its value from memory.
        V = Builder.CreateLoad(Int32Ty, nameMap[Node.getVal()]);
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
    };

    virtual void visit(Declaration &Node) override
    {
      auto Exprs_iterator = Node.beginExprs();
      auto Vars_iterator = Node.beginVars();
      //by the end of this loop we have assigned each declared variable with corresponding expression value
      for(Exprs_iterator;Exprs_iterator != Node.endExprs();++Exprs_iterator,++Vars_iterator)
      {
            (*Exprs_iterator)->accept(*this);
            Value *val = V; //V will get assined with the final value of expression which could be assignment-BinaryOpration etc..
            StringRef Var = *Vars_iterator;
            nameMap[Var] = Builder.CreateAlloca(Int32Ty);
            Builder.CreateStore(val,nameMap[Var]);
      }
      // instanciate remaining declared variables with 0
      for(Vars_iterator;Vars_iterator != Node.endVars();Vars_iterator++)
      {
            Value *zero = ConstantInt::get(Int32Ty,0,true);
            StringRef Var = *Vars_iterator;
            nameMap[Var] = Builder.CreateAlloca(Int32Ty);
            Builder.CreateStore(zero,nameMap[Var]); // I think insted of zero we could use 'Int32Zero'
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
