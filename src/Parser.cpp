#include "Parser.h"

AST* Parser::parse() {
    AST* Res = parseMainGoal();
    expect(Token::eoi);
    return Res;
}

AST* Parser::parseMainGoal() {
    //Expr* E;
    llvm::SmallVector<llvm::StringRef> vars;
    while (!Tok.is(Token::eoi)) 
    {
        switch (Tok.getKind())
        {
        case Token::KW_int:
            Expr* goal = parseGoal();
            if (goal)
                vars.push_back(goal);
            else
                goto _error;
            break;

        case Token::ident:
            Expr* goal = parseGoal();
            if (goal)
                vars.push_back(goal);
            else
                goto _error;
            break;


        case Token::KW_if:
            Expr* IfElse = parseIfElse();
            if (IfElse)
                vars.push_back(IfElse);
            else
                goto _error;
            break;
        case Token::KW_loopc:
            Expr* Loop = parseLoop();
            if (Loop)
                vars.push_back(Loop);
            else
                goto _error;
            break;
        default:
            while (Tok.isOneOf(Token::KW_end,
                Token::eoi))
            {
                advance();
            }
            error();  
        }
        advance();
    }
    //
    return new Group(vars);// ***

_error:
    while (Tok.getKind() != Token::eoi)
        advance();
    return nullptr;
}

Expr* Parser::parseIfElse() {
    Assignment* A;
    Expr* E;
    llvm::SmallVector<Expr*> conditions;
    llvm::SmallVector<llvm::SmallVector<Assignment*>> assignments;

    if (expect(Token::ifc))
        error((const char*)"ifc");
    advance();

    E = parseExpr();
    llvm::errs() << "E: " << E << "\n";
    conditions.push_back(E);

    if (expect(Token::colon))
        error((const char*)"colon");
    advance();

    if (expect(Token::begin))
        error((const char*)"begin");
    advance();

    llvm::SmallVector<Assignment*> currentAssignments;
    while (!Tok.is(Token::end)) {
        if (Tok.is(Token::ident)) {
            A = parseAssign();

            if (!Tok.is(Token::semicolon))
            {
                error((const char*)";");
            }
            advance();
            if (A)
                currentAssignments.push_back(A);
            else
                error((const char*)"kossher");
        }
        else error((const char*)"ident");
    }
    assignments.push_back(currentAssignments);
    if (expect(Token::end))
        error((const char*)"end");
    advance();

    while (Tok.is(Token::elif)) {
        if (expect(Token::elif))
            error();
        advance();

        E = parseExpr();
        conditions.push_back(E);

        if (expect(Token::colon))
            error();
        advance();

        if (expect(Token::begin))
            error();
        advance();

        currentAssignments.clear();
        while (!Tok.is(Token::end)) {
            if (Tok.is(Token::ident)) {
                A = parseAssign();

                if (!Tok.is(Token::semicolon))
                {
                    error();
                }
                advance();
                if (A)
                    currentAssignments.push_back(A);
                else
                    error();
            }
            else error();
        }
        assignments.push_back(currentAssignments);
        advance();
    }

    if (Tok.is(Token::elsec)) {
        advance();

        if (expect(Token::colon))
            error();
        advance();

        if (expect(Token::begin))
            error();
        advance();

        currentAssignments.clear();
        while (!Tok.is(Token::end)) {
            if (Tok.is(Token::ident)) {
                A = parseAssign();

                if (!Tok.is(Token::semicolon))
                {
                    error();
                    goto _error;
                }
                advance();
                if (A)
                    currentAssignments.push_back(A);
                else
                    error();
            }
            else error();
        }
        assignments.push_back(currentAssignments);
        advance();
    }

    return new IfElse(conditions, assignments);
_error: // TODO: Check this later in case of error :)
    while (Tok.getKind() != Token::eoi)
        advance();
    return nullptr;
}

Expr* Parser::parseLoop() {
    Assignment* A;
    Expr* Condition;
    llvm::SmallVector<Assignment*> assignments;

    if (expect(Token::loopc)) {
        error();
        goto _error;
    }

    advance();

    Condition = parseExpr();

    if (expect(Token::colon)) {
        error();
        goto _error;
    }

    advance();

    if (expect(Token::begin)) {
        error();
        goto _error;
    }
    advance();

    while (!Tok.is(Token::end)) {
        if (Tok.is(Token::ident)) {
            A = parseAssign();

            if (!Tok.is(Token::semicolon)) {
                error();
                goto _error;
            }
            advance();
            if (A)
                assignments.push_back(A);
            else {
                error();
                goto _error;
            }
        }
        else {
            error();
            goto _error;
        }

    }

    return new Loop(Condition, assignments);
_error: // TODO: Check this later in case of error :)
    while (Tok.getKind() != Token::eoi)
        advance();
    return nullptr;

}

Expr* Parser::parseBody() {
    
    Expr* EquSoft = parseEquSoft();
    Expr* Equsoft2 ;
    if (consume(Token::semicolon))
        error();

    while (Tok.getKind() != Token::semicolon) {
        advance();
        llvm::SmallVector<llvm::StringRef> vars;
        Equsoft2 = parseEquSoft();
        vars.push_back(Equsoft2);
    }
    return new WithDecl(vars,EquSoft);

}
Expr* Parser::parseGoal() {
    Expr* Equalization = parseEqualization();
    Expr* Equalization2;
    if (consume(Token::semicolon))
        error();
    while (Tok.getKind() != Token::semicolon) {
        advance();
        llvm::SmallVector<llvm::StringRef> vars;
        Equalization2 = parseEqualization();
        vars.push_back(Equalization2);
    }
    return new WithDecl(vars, Equalization);

}

Expr* Parser::parseEqualization() {
    llvm::SmallVector<llvm::StringRef> vars;
    while (Tok.is(Token::semicolon))//follow for Equalization
    {
        switch (Tok.getKind())
        {
        case Token::ident:
            Expr* EquSoft = parseEquSoft();
            if (EquSoft)
                vars.push_back(EquSoft);
            else
                goto _error;
            break;
        case Token::number:
            Expr* EquHard = parseEquHard();
            if (EquHard)
                vars.push_back(EquHard);
            else
                goto _error;
            break;
            default:
                error();
                while (Tok.isOneOf(Token::semicolon))
                {
                    advance();
                }
        }
    }

_error:
    while (Tok.getKind() != Token::semicolon)
        advance();
    return nullptr;

}
Expr* Parser::parseEquHard() {
    Expr* Left = parseDisjunction();
    while (Tok.is(Token::KW_logical_or)) {
        BinaryOp::Operator Op = BinaryOp::logical_or;
        advance();
        Expr* Right = parseDisjunction();
        Left = new BinaryOp(Op, Left, Right);
    }
    return Left;


}
Expr* Parser::parseEquSoft() {
    Expr* Left = parseDisjunction();
    while (Tok.is(Token::KW_logical_or)) {
        BinaryOp::Operator Op = BinaryOp::logical_or;
        advance();
        Expr* Right = parseDisjunction();
        Left = new BinaryOp(Op, Left, Right);
    }
    return Left;

}
Expr* Parser::parseHardDefinition() {
    Expr* Left = parseDisjunction();
    while (Tok.is(Token::KW_logical_or)) {
        BinaryOp::Operator Op = BinaryOp::logical_or;
        advance();
        Expr* Right = parseDisjunction();
        Left = new BinaryOp(Op, Left, Right);
    }
    return Left;


}
Expr* Parser::parseSoftDefinition() {
    Expr* Left = parseDisjunction();
    while (Tok.is(Token::KW_logical_or)) {
        BinaryOp::Operator Op = BinaryOp::logical_or;
        advance();
        Expr* Right = parseDisjunction();
        Left = new BinaryOp(Op, Left, Right);
    }
    return Left;

}
Expr* Parser::parseAssign() {
    Expr* Left = parseDisjunction();
    while (Tok.is(Token::KW_logical_or)) {
        BinaryOp::Operator Op = BinaryOp::logical_or;
        advance();
        Expr* Right = parseDisjunction();
        Left = new BinaryOp(Op, Left, Right);
    }
    return Left;

}
Expr* Parser::parseDisjunction() {
    Expr* Left = parseConjunction();
    while (Tok.is(Token::KW_logical_and)) {
        BinaryOp::Operator Op = BinaryOp::logical_and;
        advance();
        Expr* Right = parseConjunction();
        Left = new BinaryOp(Op, Left, Right);
    }
    return Left;
}
Expr* Parser::parseConjunction() {
    Expr* Left = parseIsEqual();
    while (Tok.isOneOf(Token::is_equal, Token::is_not_equal)) {
        BinaryOp::Operator Op =
            Tok.is(Token::is_equal) ?
            BinaryOp::is_equal : BinaryOp::is_not_equal;
        advance();
        Expr* Right = parseIsEqual();
        Left = new BinaryOp(Op, Left, Right);
    }
    return Left;
}
Expr* Parser::parseIsEqual() {
    Expr* Left = parseSoftComparison();
    while (Tok.isOneOf(Token::soft_comp_greater, Token::soft_comp_lower)) {
        BinaryOp::Operator Op =
            Tok.is(Token::soft_comp_greater) ?
            BinaryOp::soft_comp_greater
            : BinaryOp::soft_comp_lower;
        advance();
        Expr* Right = parseSoftComparison();
        Left = new BinaryOp(Op, Left, Right);
    }
    return Left;
}
Expr* Parser::parseSoftComparison() {
    Expr* Left = parseHardComparison();
    while (Tok.isOneOf(Token::hard_comp_greater, Token::hard_comp_lower)) {
        BinaryOp::Operator Op =
            Tok.is(Token::hard_comp_greater) ?
            BinaryOp::hard_comp_greater
            : BinaryOp::hard_comp_lower;
        advance();
        Expr* Right = parseHardComparison();
        Left = new BinaryOp(Op, Left, Right);
    }
    return Left;

}
Expr* Parser::parseHardComparison() {
    Expr* Left = parseExpression();
    while (Tok.isOneOf(Token::plus, Token::minus)) {
        BinaryOp::Operator Op =
            Tok.is(Token::plus) ?
            BinaryOp::Plus
            : BinaryOp::Minus;
        advance();
        Expr* Right = parseExpression();
        Left = new BinaryOp(Op, Left, Right);
    }
    return Left;


}
Expr* Parser::parseExpression() {
    Expr* Left = parseTerm();
    BinaryOp::Operator Op;
    while (Tok.isOneOf(Token::star, Token::slash,
        Token::mod)) {
        switch (Tok.getKind())
        {
        case Token::star:
            Op = BinaryOp::Mul;
        break;
        case Token::slash:
             Op = BinaryOp::Div;
        break;
        case Token::mod:
             Op = BinaryOp::modulo;
            break;
        }
        advance();
        Expr* Right = parseTerm();
        Left = new BinaryOp(Op, Left, Right);
    }
    return Left;

}
Expr* Parser::parseTerm() {
    Expr* Left = parseFactor();
    while (Tok.is(Token::power)) {
        BinaryOp::Operator Op = BinaryOp::powerr;
        advance();
        Expr* Right = parseFactor();
        Left = new BinaryOp(Op, Left, Right);
    }
    return Left;

}

Expr* Parser::parseFactor() {
    Expr* Res = nullptr;
    switch (Tok.getKind()) {
    case Token::number:
        Res = new Factor(Factor::Number, Tok.getText());
        advance();
        break;

    case Token::ident:
        Res = new Factor(Factor::Ident, Tok.getText());
        advance();
        break;

    case Token::l_paren:
        advance();
        Res = parseAssign();
        if (!consume(Token::r_paren))
            break;
    default:
        if (!Res)//if it was null the parser It implies that the
            //parser was unable to successfully construct an expression
            // node based on the encountered tokens within the switch-case logic.
            error();
        /*
        * The purpose of this while loop is to skip tokens until
        it reaches a point where it finds a token that allows the
        parser to continue parsing without errors. It essentially
        skips tokens until it encounters one of the specified tokens.
        */
        while (!Tok.isOneOf(Token::r_paren, Token::star,
            Token::plus, Token::minus,
            Token::slash, Token::eoi))
            advance();
    }
    return Res;
}
