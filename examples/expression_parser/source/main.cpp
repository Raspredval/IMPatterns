#include <MappedFile.hpp>
#include <Patterns.hpp>

#include "Token.hpp"

#include <string_view>
#include <stdexcept>
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
        Capt<{TYPE_COMMENT, COMMTYPE_ENDLINE}>(
            Str<"#">() >> Any(NegSet<"\n">())
        )
    ))

    IMP_MAKE_RULE(full_expr, (
        CaptGr<{TYPE_EXPR, EXPRTYPE_FULL}>(
            Fn<sub_expr>() >>
            Opt(Fn<comment>()) >>
            (Set<";\n">() | NoChars())
        )
    ))

    IMP_MAKE_RULE(sub_expr,
        Any(Space()) >> Opt(
            Fn<operand>() >> Any(
                Any(Space()) >> Fn<bin_op>() >>
                Any(Space()) >> Fn<operand>()
            ) >> Any(Space())
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
        Capt<{TYPE_VALUE, VALTYPE_INT}>(
            Some(Digit())
        ) >> Opt(
            CaptMerge<{TYPE_VALUE, VALTYPE_FLOAT}>(
                Str<".">() >> Some(Digit())
            )
        )
    ))

    IMP_MAKE_RULE(boolean, (
        Capt<{TYPE_VALUE, VALTYPE_BOOL}>(
            Dict<"true", "false">()
        )
    ))

    IMP_MAKE_RULE(variable, (
        Capt<{TYPE_VALUE, VALTYPE_VAR}>(
            Alpha() >> Any(imp::Alnum())
        ) >> Opt(
            Str<"(">() >> ReplaceCaptTag<{TYPE_VALUE, VALTYPE_FUNC}>() >>
            Opt(
                Fn<arg_expr>() >> Any(
                    Str<",">() >> Fn<arg_expr>()
                )
            ) >> Str<")">()
        )
    ))

    IMP_MAKE_RULE(arg_expr, (
        CaptGr<{TYPE_EXPR, EXPRTYPE_ARG}>(Fn<sub_expr>(),
        [] (MemStream&, Match m, CaptureView captures, UserData) -> Match {
            if (m) {
                if (captures.empty())
                    m.Toggle();
            }

            return m;
        })
    ))

    IMP_MAKE_RULE(string, (
        Capt<{TYPE_VALUE, VALTYPE_STR}>(
            Str<"\"">() >>
            Any(NegSet<"\"\n">()) >>
            Str<"\"">()
        )
    ))

    IMP_MAKE_RULE(brkt_expr, (
        Capt<{TYPE_OPENBRK, BRKTYPE_ROUND}>(
            Str<"(">()
        ) >> Fn<sub_expr>() >>
        Capt<{TYPE_CLOSEBRK, BRKTYPE_ROUND}>(
            Str<")">()
        )
    ))

    IMP_MAKE_RULE(bin_op, (
        CaptDict<
            {"^^",  { TYPE_BINOP, OPTYPE_POW        }},
            {"*",   { TYPE_BINOP, OPTYPE_MUL        }},
            {"/",   { TYPE_BINOP, OPTYPE_DIV        }},
            {"%",   { TYPE_BINOP, OPTYPE_MOD        }},
            {"//",  { TYPE_BINOP, OPTYPE_IDIV       }},
            {"+",   { TYPE_BINOP, OPTYPE_ADD        }},
            {"-",   { TYPE_BINOP, OPTYPE_SUB        }},
            {"<",   { TYPE_BINOP, OPTYPE_LESS       }},
            {"<=",  { TYPE_BINOP, OPTYPE_LESSEQ     }},
            {">",   { TYPE_BINOP, OPTYPE_GREAT      }},
            {">=",  { TYPE_BINOP, OPTYPE_GREATEQ    }},
            {"==",  { TYPE_BINOP, OPTYPE_EQUAL      }},
            {"!=",  { TYPE_BINOP, OPTYPE_NOTEQUAL   }},
            {"&&",  { TYPE_BINOP, OPTYPE_BOOLAND    }},
            {"||",  { TYPE_BINOP, OPTYPE_BOOLOR     }},
            {"=",   { TYPE_BINOP, OPTYPE_ASSIGN     }},
            {"+=",  { TYPE_BINOP, OPTYPE_ADDASS     }},
            {"-=",  { TYPE_BINOP, OPTYPE_SUBASS     }},
            {"*=",  { TYPE_BINOP, OPTYPE_MULASS     }},
            {"/=",  { TYPE_BINOP, OPTYPE_DIVASS     }},
            {"%=",  { TYPE_BINOP, OPTYPE_MODASS     }},
            {"//=", { TYPE_BINOP, OPTYPE_IDIVASS    }},
            {"^^=", { TYPE_BINOP, OPTYPE_POWASS     }}
        >()
    ))

    IMP_MAKE_RULE(un_op, (
        CaptDict<
            { "+", { TYPE_UNOP, OPTYPE_PLUS     }},
            { "-", { TYPE_UNOP, OPTYPE_MINUS    }},
            { "!", { TYPE_UNOP, OPTYPE_NOT      }}
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
PrintTokens(imp::MemStream& stream, imp::CaptureView captures, int iLevel) {
    for (const auto& c : captures) {
        std::string_view
            strvCapture     = c.match.GetStringView(stream);
        if (strvCapture.ends_with('\n'))
            strvCapture.remove_suffix(1);

        imp::UserData
            usr_val = c.tag;
        auto
            iLow    = usr_val.ipr.l,
            iHigh   = usr_val.ipr.h;

        TokenType
            iTknID  = (TokenType)iLow;
        switch (iTknID) {
        break; case TYPE_EXPR: {
            ExprType
                iExprID = (ExprType)iHigh;
            switch (iExprID) {
            break; case EXPRTYPE_FULL:
                Print(iLevel, "#expr: [ {} ] {{\n", strvCapture);
                PrintTokens(stream, c.children, iLevel + 1);
                Print(iLevel, "}}\n\n");

            break; case EXPRTYPE_ARG:
                Print(iLevel, "#arg expr: [ {} ] {{\n", strvCapture);
                PrintTokens(stream, c.children, iLevel + 1);
                Print(iLevel, "}}\n");
            }
        }

        break; case TYPE_COMMENT: {
            Print(iLevel, "comment: [ {} ]\n", strvCapture);
        }

        break; case TYPE_VALUE: {
            ValType
                iValID  = (ValType)iHigh;
            switch (iValID) {
            break; case VALTYPE_INT:
                Print(iLevel, "integer: [ {} ]\n", strvCapture);

            break; case VALTYPE_FLOAT:
                Print(iLevel, "float: [ {} ]\n", strvCapture);

            break; case VALTYPE_BOOL:
                Print(iLevel, "boolean: [ {} ]\n", strvCapture);

            break; case VALTYPE_STR:
                Print(iLevel, "string: [ {} ]\n", strvCapture);

            break; case VALTYPE_VAR:
                Print(iLevel, "variable: [ {} ]\n", strvCapture);

            break; case VALTYPE_FUNC:
                Print(iLevel, "function: [ {} ]\n", strvCapture);
            }
        }

        break; case TYPE_BINOP: {
            OpType
                iOpID   = (OpType)iHigh;
            Print(iLevel, "binary op: (0x{:x} {}) [ {} ]\n",
                (int)iOpID,
                Token::isLeftAssociable(iOpID)
                    ? "left ass." : "right ass.",
                strvCapture
            );
        }

        break; case TYPE_UNOP: {
            OpType
                iOpID   = (OpType)iHigh;
            Print(iLevel, "unary op: (0x{:x} {}) [ {} ]\n",
                (int)iOpID,
                Token::isLeftAssociable(iOpID)
                    ? "left ass." : "right ass.",
                strvCapture
            );
        }

        break; case TYPE_OPENBRK:
            Print(iLevel, "open bracket: [ {} ]\n", strvCapture);

        break; case TYPE_CLOSEBRK:
            Print(iLevel, "close bracket: [ {} ]\n", strvCapture);
        }
    }
}

static void
PrecompileChunk(std::span<const char> spnSource) {
    imp::MemStream
        stream      = {spnSource};
    imp::Captures
        captures    = {};
    if (!imp::Eval(imp::Fn<grammar::eval>(), stream, captures)) {
        throw std::runtime_error(std::format(
            "parser failed at: {}", stream.GetPos()
        ));
    }

    PrintTokens(stream, captures, 0);
}

static void
PrecompileFile(std::string_view strvFilename) {
    imp::MappedFile
        mapped_file = {strvFilename};
    if (!mapped_file) {
        throw std::runtime_error(std::format(
            "failed to open file: {}", strvFilename
        ));
    }

    return PrecompileChunk(mapped_file.GetView());
}

int main() {
    try {
        std::string_view
            strvFilename    = "./assets/test.txt";

        PrecompileFile(strvFilename);

        std::print("success\n");
    }
    catch (const std::exception& err) {
        std::print(stderr, "error: {}\n", err.what());
        return -1;
    }

    return 0;
}