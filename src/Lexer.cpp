#include "Lexer.h"

// classifying characters
namespace charinfo
{
    // ignore whitespaces
    LLVM_READNONE inline bool isWhitespace(char c)
    {
        return c == ' ' || c == '\t' || c == '\f' || c == '\v' ||
               c == '\r' || c == '\n';
    }

    LLVM_READNONE inline bool isDigit(char c)
    {
        return c >= '0' && c <= '9';
    }

    LLVM_READNONE inline bool isLetter(char c)
    {
        return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
    }

    LLVM_READNONE inline bool isSpecialCharacter(char c)
    {
        return c == '=' || c == '+' || c == '-' || c == '*' || c == '/' 
            || c == '!' || c == '>' || c == '<' || c == '(' || c == ')' 
            || c == ',' || c == ';' || c == '%' || c == '^' || c == ':';
    }
}

void Lexer::next(Token &token)
{
    while (*BufferPtr && charinfo::isWhitespace(*BufferPtr))
    {
        ++BufferPtr;
    }
    // make sure we didn't reach the end of input
    if (!*BufferPtr)
    {
        token.Kind = Token::eoi;
        return;
    }
    // collect characters and check for keywords or ident
    if (charinfo::isLetter(*BufferPtr))
    {
        const char *end = BufferPtr + 1;
        while (charinfo::isLetter(*end))
            ++end;
        llvm::StringRef Name(BufferPtr, end - BufferPtr);
        Token::TokenKind kind;
        if (Name == "int")
            kind = Token::KW_int;
        else if (Name == "begin")
            kind = Token::KW_begin;
        else if (Name == "end")
            kind = Token::KW_end;
        else if (Name == "if")
            kind = Token::KW_if;
        else if (Name == "elif")
            kind = Token::KW_elif;
        else if (Name == "else")
            kind = Token::KW_else;
        else if (Name == "loopc")
            kind = Token::KW_loopc;
        else if (Name == "or")
            kind = Token::KW_logical_or;
        else if (Name == "and")
            kind = Token::KW_logical_and;
        else
            kind = Token::ident;
        // generate the token
        formToken(token, end, kind);
        return;
    }
    // check for numbers
    else if (charinfo::isDigit(*BufferPtr))
    {
        const char *end = BufferPtr + 1;
        while (charinfo::isDigit(*end))
            ++end;
        formToken(token, end, Token::number);
        return;
    }
    // check for special characters
    else if (charinfo::isSpecialCharacter(*BufferPtr))
    {
        const char *end = BufferPtr + 1;
        while (charinfo::isSpecialCharacter(*end))
            ++end;
        llvm::StringRef Name(BufferPtr, end - BufferPtr);
        llvm::StringRef parenName(BufferPtr, BufferPtr + 1 - BufferPtr);
        Token::TokenKind kind;
        bool is_valid = true;
        if (parenName == "(") 
        {
            kind = Token::l_paren;
            formToken(token, BufferPtr + 1, kind);
        }
        else if (parenName == ")") 
        {
            kind = Token::r_paren;
            formToken(token, BufferPtr + 1, kind);
        }
        else {
            if (Name == "=")
            {
                kind = Token::equal;
            }
            else if (Name == "+=")
            {
                kind = Token::plus_equal;
            }
            else if (Name == "-=")
            {
                kind = Token::minus_equal;
            }
            else if (Name == "*=")
            {
                kind = Token::mult_equal;
            }
            else if (Name == "/=")
            {
                kind = Token::div_equal;
            }
            else if (Name == "%=")
            {
                kind = Token::mod_equal;
            }
            else if (Name == "==")
            {
                kind = Token::is_equal;
            }
            else if (Name == "!=")
            {
                kind = Token::is_not_equal;
            }
            else if (Name == ">=")
            {
                kind = Token::soft_comp_greater;
            }
            else if (Name == "<=")
            {
                kind = Token::soft_comp_lower;
            }
            else if (Name == ">")
            {
                kind = Token::hard_comp_greater;
            }
            else if (Name == "<")
            {
                kind = Token::hard_comp_lower;
            }
            else if (Name == ",")
            {
                kind = Token::comma;
            }
            else if (Name == ";")
            {
                kind = Token::semicolon;
            }
            else if (Name == "+")
            {
                kind = Token::plus;
            }
            else if (Name == "-")
            {
                kind = Token::minus;
            }
            else if (Name == "*")
            {
                kind = Token::star;
            }
            else if (Name == "/")
            {
                kind = Token::slash;
            }
            else if (Name == "%")
            {
                kind = Token::mod;
            }
            else if (Name == "^")
            {
                kind = Token::power;
            }
            else if (Name == ":")
            {
                kind = Token::colon;
            }
            // else if (Name == "(")
            // {
            //     kind = Token::l_paren;
            // }
            // else if (Name == ")")
            // {
            //     kind = Token::r_paren;
            // }
            else {
                is_valid = false;
            }

            if (is_valid)
            {
                formToken(token, end, kind);
            }
            else
            {
                formToken(token, BufferPtr + 1, Token::unknown);
            }
        }
        return;
    }    
    // check for unknown characters
    else
    {
    formToken(token, BufferPtr + 1, Token::unknown);
    return;
    }

    // // check for special characters
    // else if (charinfo::isSpecialCharacter(*BufferPtr)) {
    //     const char *end = BufferPtr + 1;
    //     while (charinfo::isSpecialCharacter(*end))
    //         ++end;
    //     llvm::StringRef Name(BufferPtr, end - BufferPtr);
    //     Token::TokenKind kind;
    //     bool is_valid = true;
        
    //     if (Name == "+=")
    //     {
    //         kind = Token::plus_equal;
    //     }
    //     else if (Name == "-=")
    //     {
    //         kind = Token::minus_equal;
    //     }
    //     else if (Name == "*=")
    //     {
    //         kind = Token::mult_equal;
    //     }
    //     else if (Name == "/=")
    //     {
    //         kind = Token::div_equal;
    //     }
    //     else if (Name == "%=")
    //     {
    //         kind = Token::mod_equal;
    //     }
    //     else if (Name == "==")
    //     {
    //         kind = Token::is_equal;
    //     }
    //     else if (Name == "!=")
    //     {
    //         kind = Token::is_not_equal;
    //     }
    //     else if (Name == ">=")
    //     {
    //         kind = Token::soft_comp_greater;
    //     }
    //     else if (Name == "<=")
    //     {
    //         kind = Token::soft_comp_lower;
    //     }
    //     else {
    //         is_valid = false;
    //     }
    //     if (is_valid)
    //     {
    //         formToken(token, end, kind);
    //     }
    //     else
    //     {
    //         formToken(token, BufferPtr + 1, Token::unknown);
    //     }

    //     switch (*BufferPtr)
    //     {

    //     return;
    // }
    // }

//     else
//     {
//         switch (*BufferPtr)
//         {
// #define CASE(ch, tok)                         \
//     case ch:                                  \
//         formToken(token, BufferPtr + 1, tok); \
//         break
//             CASE('=', Token::equal);
//             // CASE('+=', Token::plus_equal);
//             // CASE('-=', Token::minus_equal);
//             // CASE('*=', Token::mult_equal);
//             // CASE('/=', Token::div_equal);
//             // CASE('%=', Token::mod_equal);
//             // CASE('==', Token::is_equal);
//             // CASE('!=', Token::is_not_equal);
//             // CASE('>=', Token::soft_comp_greater);
//             // CASE('<=', Token::soft_comp_lower);
//             CASE('>', Token::hard_comp_greater);
//             CASE('<', Token::hard_comp_lower);
//             CASE(',', Token::Token::comma);
//             CASE(';', Token::semicolon);
//             CASE('+', Token::plus);
//             CASE('-', Token::minus);
//             CASE('*', Token::star);
//             CASE('/', Token::slash);
//             CASE('%', Token::mod);
//             CASE('^', Token::power);
//             CASE(':', Token::colon);
//             CASE('(', Token::l_paren);
//             CASE(')', Token::r_paren);
// #undef CASE
//         default:
//             formToken(token, BufferPtr + 1, Token::unknown);
//         }
//         return;
//     }
//  return;
}

void Lexer::formToken(Token &Tok, const char *TokEnd,
                      Token::TokenKind Kind)
{
    Tok.Kind = Kind;
    Tok.Text = llvm::StringRef(BufferPtr, TokEnd - BufferPtr);
    BufferPtr = TokEnd;
}
