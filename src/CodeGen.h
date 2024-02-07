#ifndef CODEGEN_H
#define CODEGEN_H

#include "AST.h"

class CodeGen
{
public:
 void compile(AST *Tree);
 void collectIdentifiers(AST *Tree);
 void computeDepends(AST *Tree);
 void computeDead();
};
#endif
