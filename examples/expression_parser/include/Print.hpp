#pragma once
#include <Patterns.hpp>
#include <print>

#include "Token.hpp"
#include "LineTracker.hpp"

template<typename... Args>
inline void
Print(int iLevel, const std::format_string<const Args&...>& strfmt, const Args&... args) {
    for (int i = 0; i < iLevel; ++i) {
        std::print("  ");
    } std::print(strfmt, args...);
}

inline void
PrintTokens(imp::MemStream& stream, imp::CaptureView tokens, const LineTracker& l, int iLevel = 0) {
    for (const auto& tkn : tokens) {
        std::string_view
            strvToken   = tkn.match.GetStringView(stream);
        SrcPos
            posToken    = l.GetSrcPos(tkn.match.Begin());

        Print(iLevel, "ln {} col {}: ",
            posToken.iLn, posToken.iCol);

        auto [ iLow, iHigh] =
            tkn.tag.ipr;

        TokenType
            iTknID  = (TokenType)iLow;
        switch (iTknID) {
        break; case TOKTYPE_EXPR: {
            ExprType
                iExprID = (ExprType)iHigh;
            switch (iExprID) {
            break; case EXPRTYPE_FULL:
                std::print("expression: <[ {} ]>", strvToken);
                if (!tkn.children.empty()) {
                    std::print(" {{\n");
                    PrintTokens(stream, tkn.children, l, iLevel + 1);
                    Print(iLevel, "}}");
                } std::print("\n\n");

            break; case EXPRTYPE_SUB:
                std::print("sub expression: <[ {} ]> {{\n", strvToken);
                PrintTokens(stream, tkn.children, l, iLevel + 1);
                Print(iLevel, "}}\n");

            break; case EXPRTYPE_ARG:
                std::print("argument: <[ {} ]> {{\n", strvToken);
                PrintTokens(stream, tkn.children, l, iLevel + 1);
                Print(iLevel, "}}\n");
            }
        }

        break; case TOKTYPE_COMMENT: {
            std::print("comment: <[ {} ]>\n\n", strvToken);
        }

        break; case TOKTYPE_VALUE: {
            ValType
                iValID  = (ValType)iHigh;
            switch (iValID) {
            break; case VALTYPE_INT:
                std::print("integer: <[ {} ]>\n", strvToken);

            break; case VALTYPE_FLOAT:
                std::print("fractional: <[ {} ]>\n", strvToken);

            break; case VALTYPE_BOOL:
                std::print("boolean: <[ {} ]>\n", strvToken);

            break; case VALTYPE_STR:
                std::print("string: <[ {} ]>\n", strvToken);

            break; case VALTYPE_ID:
                std::print("identifier: <[ {} ]>\n", strvToken);
            }
        }

        break; case TOKTYPE_BINOP: {
            OpType
                iOpID   = (OpType)iHigh;
            std::print("binary op: <[ {} ]> (opID: 0x{:02x}, precedence: {}, {})\n",
                strvToken,
                (int)iOpID,
                getOpPrecedence(iOpID),
                isLeftAssociable(iOpID)
                    ? "left associable"
                    : "right associable"
            );
        }

        break; case TOKTYPE_UNOP: {
            OpType
                iOpID   = (OpType)iHigh;
            std::print("unary op: <[ {} ]> (opID: 0x{:02x}, precedence: {}, {})",
                strvToken,
                (int)iOpID,
                getOpPrecedence(iOpID),
                isLeftAssociable(iOpID)
                    ? "left associable"
                    : "right associable"
            );

            switch (iOpID) {
            case OPTYPE_CALL:
            case OPTYPE_INDEX:
            case OPTYPE_CAST:
                if (!tkn.children.empty()) {
                    std::print(" {{\n");
                    PrintTokens(stream, tkn.children, l, iLevel + 1);
                    Print(iLevel, "}}");
                }

            [[fallthrough]];
            default:
                std::print("\n");
            }
        }

        }
    }
}