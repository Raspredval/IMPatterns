#include <MappedFile.hpp>
#include <Patterns.hpp>

#include "Token.hpp"

#include <string_view>
#include <format>
#include <print>

namespace grammar {
    using namespace imp;

    IMP_DECL_RULE(static eval);
    IMP_DECL_RULE(static comment);
    IMP_DECL_RULE(static bin_op);
    IMP_DECL_RULE(static un_op);
    IMP_DECL_RULE(static operand);
    IMP_DECL_RULE(static number);
    IMP_DECL_RULE(static boolean);
    IMP_DECL_RULE(static string);
    IMP_DECL_RULE(static variable);
    IMP_DECL_RULE(static sub_expr);
    IMP_DECL_RULE(static full_expr);
    IMP_DECL_RULE(static arg_expr);
    IMP_DECL_RULE(static brkt_expr);

    IMP_MAKE_RULE(comment, (
        Capt<{TOKTYPE_COMMENT, COMMTYPE_ENDLINE}>(
            Str<"#">() >> Any(NegSet<"\n">())
        )
    ))

    IMP_MAKE_RULE(full_expr, (
        CaptGr<{TOKTYPE_EXPR, EXPRTYPE_FULL}>(
            Any(Space()) >> Opt (
                Fn<sub_expr>() >> Any(Space())
            ) >> Opt(Fn<comment>()) >> (Set<";\n">() | NoChars())
        )
    ))

    IMP_MAKE_RULE(sub_expr,
        Fn<operand>() >> Any(
            Any(Space()) >> Fn<bin_op>() >>
            Any(Space()) >> Fn<operand>()
        )
    )

    IMP_MAKE_RULE(operand,
        Any(Fn<un_op>()) >> (
            Fn<number>()   |
            Fn<boolean>()  |
            Fn<string>()   |
            Fn<variable>() |
            Fn<brkt_expr>()
        )
    )

    IMP_MAKE_RULE(number, (
        Capt<{TOKTYPE_VALUE, VALTYPE_INT}>(
            Some(Digit())
        ) >> Opt(
            CaptMerge<{TOKTYPE_VALUE, VALTYPE_FLOAT}>(
                Str<".">() >> Some(Digit())
            )
        )
    ))

    IMP_MAKE_RULE(boolean, (
        Capt<{TOKTYPE_VALUE, VALTYPE_BOOL}>(
            Dict<"true", "false">()
        )
    ))

    IMP_MAKE_RULE(variable, (
        Capt<{TOKTYPE_VALUE, VALTYPE_VAR}>(
            Alpha() >> Any(imp::Alnum())
        ) >> Opt(
            Str<"(">() >> ReplaceCaptTag<{TOKTYPE_VALUE, VALTYPE_FUNC}>() >>
            Opt(
                Fn<arg_expr>() >> Any(
                    Str<",">() >> Fn<arg_expr>()
                )
            ) >> Str<")">()
        )
    ))

    IMP_MAKE_RULE(arg_expr, (
        Any(Space()) >>
        CaptGr<{TOKTYPE_EXPR, EXPRTYPE_ARG}>(Fn<sub_expr>()) >>
        Any(Space())
    ))

    IMP_MAKE_RULE(string, (
        Capt<{TOKTYPE_VALUE, VALTYPE_STR}>(
            Str<"\"">() >>
            Any(NegSet<"\"\n">()) >>
            Str<"\"">()
        )
    ))

    IMP_MAKE_RULE(brkt_expr, (
        Str<"(">() >> Any(Space()) >>
        CaptGr<{TOKTYPE_EXPR, EXPRTYPE_SUB}>(Fn<sub_expr>()) >>
        Any(Space()) >> Str<")">()
    ))

    IMP_MAKE_RULE(bin_op, (
        CaptDict<
            { "^^",  { TOKTYPE_BINOP, OPTYPE_POW      }},
            { "*",   { TOKTYPE_BINOP, OPTYPE_MUL      }},
            { "/",   { TOKTYPE_BINOP, OPTYPE_DIV      }},
            { "%",   { TOKTYPE_BINOP, OPTYPE_MOD      }},
            { "//",  { TOKTYPE_BINOP, OPTYPE_IDIV     }},
            { "+",   { TOKTYPE_BINOP, OPTYPE_ADD      }},
            { "-",   { TOKTYPE_BINOP, OPTYPE_SUB      }},
            { "<",   { TOKTYPE_BINOP, OPTYPE_LESS     }},
            { "<=",  { TOKTYPE_BINOP, OPTYPE_LESSEQ   }},
            { ">",   { TOKTYPE_BINOP, OPTYPE_GREAT    }},
            { ">=",  { TOKTYPE_BINOP, OPTYPE_GREATEQ  }},
            { "==",  { TOKTYPE_BINOP, OPTYPE_EQUAL    }},
            { "!=",  { TOKTYPE_BINOP, OPTYPE_NOTEQUAL }},
            { "&&",  { TOKTYPE_BINOP, OPTYPE_BOOLAND  }},
            { "||",  { TOKTYPE_BINOP, OPTYPE_BOOLOR   }},
            { "=",   { TOKTYPE_BINOP, OPTYPE_ASSIGN   }},
            { "+=",  { TOKTYPE_BINOP, OPTYPE_ADDASS   }},
            { "-=",  { TOKTYPE_BINOP, OPTYPE_SUBASS   }},
            { "*=",  { TOKTYPE_BINOP, OPTYPE_MULASS   }},
            { "/=",  { TOKTYPE_BINOP, OPTYPE_DIVASS   }},
            { "%=",  { TOKTYPE_BINOP, OPTYPE_MODASS   }},
            { "//=", { TOKTYPE_BINOP, OPTYPE_IDIVASS  }},
            { "^^=", { TOKTYPE_BINOP, OPTYPE_POWASS   }}
        >()
    ))

    IMP_MAKE_RULE(un_op, (
        CaptDict<
            { "+", { TOKTYPE_UNOP, OPTYPE_PLUS  }},
            { "-", { TOKTYPE_UNOP, OPTYPE_MINUS }},
            { "!", { TOKTYPE_UNOP, OPTYPE_NOT   }}
        >()
    ))

    IMP_MAKE_RULE(eval,
        Any(
            LookAhead(AnyChar()) >>
            Fn<full_expr>()
        ) >> NoChars()
    )
}

template<typename... Args>
static void
Print(int iLevel, const std::format_string<const Args&...>& strfmt, const Args&... args) {
    for (int i = 0; i < iLevel; ++i)
        putchar(' ');
    std::print(strfmt, args...);
}

static void
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
                Print(iLevel, "#expression: [ {} ]", strvToken);
                if (!tkn.children.empty()) {
                    std::print(" {{\n");
                    PrintTokens(stream, tkn.children, iLevel + 1);
                    Print(iLevel, "}}\n\n");
                }
                else
                    std::print(";\n\n");

            break; case EXPRTYPE_SUB:
                Print(iLevel, "#sub expression: [ {} ] {{\n", strvToken);
                PrintTokens(stream, tkn.children, iLevel + 1);
                Print(iLevel, "}}\n");

            break; case EXPRTYPE_ARG:
                Print(iLevel, "#argument: [ {} ] {{\n", strvToken);
                PrintTokens(stream, tkn.children, iLevel + 1);
                Print(iLevel, "}}\n");
            }
        }

        break; case TOKTYPE_COMMENT: {
            Print(iLevel, "comment:\t[ {} ]\n", strvToken);
        }

        break; case TOKTYPE_VALUE: {
            ValType
                iValID  = (ValType)iHigh;
            switch (iValID) {
            break; case VALTYPE_INT:
                Print(iLevel, "integer:\t[ {} ]\n", strvToken);

            break; case VALTYPE_FLOAT:
                Print(iLevel, "fractional:\t[ {} ]\n", strvToken);

            break; case VALTYPE_BOOL:
                Print(iLevel, "boolean:\t[ {} ]\n", strvToken);

            break; case VALTYPE_STR:
                Print(iLevel, "string:\t[ {} ]\n", strvToken);

            break; case VALTYPE_VAR:
                Print(iLevel, "variable:\t[ {} ]\n", strvToken);

            break; case VALTYPE_FUNC:
                Print(iLevel, "function:\t[ {} ]\n", strvToken);
            }
        }

        break; case TOKTYPE_BINOP: {
            OpType
                iOpID   = (OpType)iHigh;
            Print(iLevel, "binary op:\t[ {} ] (opID: 0x{:x}, {})\n",
                strvToken,
                (int)iOpID,
                isLeftAssociable(iOpID)
                    ? "left associable"
                    : "right associable"
            );
        }

        break; case TOKTYPE_UNOP: {
            OpType
                iOpID   = (OpType)iHigh;
            Print(iLevel, "unary op:\t[ {} ] (opID: 0x{:x}, {})\n",
                strvToken,
                (int)iOpID,
                isLeftAssociable(iOpID)
                    ? "left associable"
                    : "right associable"
            );
        }

        }
    }
}

int main() {
    std::string_view
        strvFilename    = "./assets/test.txt";

    imp::MappedFile
        mapped_file = {strvFilename};
    if (!mapped_file) {
        std::print(stderr,
            "failed to open file: {}", strvFilename);
        return -1;
    }

    imp::Captures
        tokens;
    imp::MemStream
        stream      = {mapped_file.GetView()};
    if (!imp::Eval(imp::Fn<grammar::eval>(), stream, tokens)) {
        std::print(stderr,
            "parser failed at: {}\n", stream.GetPos());
        return -1;
    }

    PrintTokens(stream, tokens);
    return 0;
}