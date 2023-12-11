#ifndef PARSER_H
#define PARSER_H

#include "AST.h"
#include "Lexer.h"
#include "llvm/Support/raw_ostream.h"

class Parser
{
    Lexer& Lex;    // retrieve the next token from the input
    Token Tok;     // stores the next token
    bool HasError; // indicates if an error was detected

    void error()
    {
        llvm::errs() << "Unexpected: " << Tok.getText() << "\n";
        HasError = true;
    }

    // retrieves the next token from the lexer.expect()
    // tests whether the look-ahead is of the expected kind
    void advance() 
    { 
        Lex.next(Tok);
    }

    bool expect(Token::TokenKind Kind)
    {
        if (Tok.getKind() != Kind)
        {
            error();
            return true;
        }
        return false;
    }

    // retrieves the next token if the look-ahead is of the expected kind
    bool consume(Token::TokenKind Kind)
    {
        if (expect(Kind))
            return true;
        advance();
        return false;
    }

    AST* parseMainGoal();
    Expr* parseIfElse();
    Expr* parseIf();
    Expr* parseElIf();
    Expr* parseElse();
    Expr* parseLoop();
    Expr* parseBody();
    Expr* parseGoal();
    Expr* parseEqualization();
    Expr* parseEquHard();
    Expr* parseEquSoft();
    Expr* parseHardDefinition();
    Expr* parseSoftDefinition();
    Expr* parseAssign();
    Expr* parseDisjunction();
    Expr* parseConjunction();
    Expr* parseIsEqual();
    Expr* parseSoftComparison();
    Expr* parseHardComparison();
    Expr* parseExpression();
    Expr* parseTerm();
    Expr* parseFactor();

public:
    // initializes all members and retrieves the first token
    Parser(Lexer& Lex) : Lex(Lex), HasError(false)
    {
        advance();
    }

    // get the value of error flag
    bool hasError() { return HasError; }

    // a memebr function from the Parser class that returns a pointer to an AST object
    //just the difinition not the implementation
    AST* parse();
};

#endif