#pragma once
#include <Patterns.hpp>
#include <print>

#include "Token.hpp"

template<typename... Args>
inline void
Print(int iLevel, const std::format_string<const Args&...>& strfmt, const Args&... args) {
    for (int i = 0; i < iLevel; ++i) {
        std::print(" ");
    } std::print(strfmt, args...);
}

inline void
PrintTokens(imp::MemStream& stream, imp::CaptureView tokens, int iLevel = 0) {
    for (const auto& tkn : tokens) {
        std::string_view
            strvToken   = tkn.match.GetStringView(stream);
        if (strvToken.ends_with('\n'))
            strvToken.remove_suffix(1);

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
                Print(iLevel, "expression: <[ {} ]>", strvToken);
                if (!tkn.children.empty()) {
                    std::print(" {{\n");
                    PrintTokens(stream, tkn.children, iLevel + 1);
                    Print(iLevel, "}}");
                } std::print("\n\n");

            break; case EXPRTYPE_SUB:
                Print(iLevel, "sub expression: <[ {} ]> {{\n", strvToken);
                PrintTokens(stream, tkn.children, iLevel + 1);
                Print(iLevel, "}}\n");

            break; case EXPRTYPE_ARG:
                Print(iLevel, "argument: <[ {} ]> {{\n", strvToken);
                PrintTokens(stream, tkn.children, iLevel + 1);
                Print(iLevel, "}}\n");
            }
        }

        break; case TOKTYPE_COMMENT: {
            Print(iLevel, "comment:\t<[ {} ]>\n", strvToken);
        }

        break; case TOKTYPE_VALUE: {
            ValType
                iValID  = (ValType)iHigh;
            switch (iValID) {
            break; case VALTYPE_INT:
                Print(iLevel, "integer:\t<[ {} ]>\n", strvToken);

            break; case VALTYPE_FLOAT:
                Print(iLevel, "fractional:\t<[ {} ]>\n", strvToken);

            break; case VALTYPE_BOOL:
                Print(iLevel, "boolean:\t<[ {} ]>\n", strvToken);

            break; case VALTYPE_STR:
                Print(iLevel, "string:\t<[ {} ]>\n", strvToken);

            break; case VALTYPE_ID:
                Print(iLevel, "identifier:\t<[ {} ]>\n", strvToken);
            }
        }

        break; case TOKTYPE_BINOP: {
            OpType
                iOpID   = (OpType)iHigh;
            Print(iLevel, "binary op:\t<[ {} ]> (opID: 0x{:02x}, precedence: {}, {})\n",
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
            Print(iLevel, "unary op:\t<[ {} ]> (opID: 0x{:02x}, precedence: {}, {})",
                strvToken,
                (int)iOpID,
                getOpPrecedence(iOpID),
                isLeftAssociable(iOpID)
                    ? "left associable"
                    : "right associable"
            );

            if (iOpID == OPTYPE_CALL || iOpID == OPTYPE_INDEX) {
                if (!tkn.children.empty()) {
                    std::print(" {{\n");
                    PrintTokens(stream, tkn.children, iLevel + 1);
                    Print(iLevel, "}}");
                }
            } std::print("\n");
        }

        }
    }
}