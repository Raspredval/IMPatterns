#include <MappedFile.hpp>
#include <Patterns.hpp>

#include "Token.hpp"

#include <string_view>
#include <stdexcept>
#include <format>
#include <print>

namespace grammar {
    namespace handler {
        static imp::Match
        full_expr(imp::MemStream& stream, const imp::Match& m, imp::CapturesView captures, const std::any&){
            if (m) {
                for (const auto& c : captures) {
                    auto [uLow, uHigh] = imp::to_halfptr(c.u);

                    switch ((TokenType)uLow) {
                    break; case TYPE_VALUE: {
                        switch ((ValType)uHigh) {
                        break; case VALTYPE_NUMBER:
                            std::print("number: ");

                        break; case VALTYPE_VARIABLE:
                            std::print("variable: ");
                        }
                    }

                    break; case TYPE_BINOP: {
                        OpType
                            uOpID   = (OpType)uHigh;
                        std::print("opID: 0x{:x}; precedence: {}; left associative: {}\n",
                            (unsigned)uOpID,
                            Token::getPrecedence(uOpID),
                            Token::isLeftAssociable(uOpID));
                        std::print(" binary operator: ");
                    }

                    break; case TYPE_UNOP: {
                        OpType
                            uOpID   = (OpType)uHigh;
                        std::print("opID: 0x{:x}; precedence: {}; left associative: {}\n",
                            (unsigned)uOpID,
                            Token::getPrecedence((OpType)uHigh),
                            Token::isLeftAssociable((OpType)uHigh));
                        std::print(" unary operator: ");
                    }

                    break; case TYPE_OPENBRK:
                        std::print("open bracket: ");

                    break; case TYPE_CLOSEBRK:
                        std::print("close bracket: ");
                    } std::print("'{}'\n", c.m.GetStringView(stream));
                } std::print("### END OF EXPRESSION ###\n\n");
            }

            return m;
        }
    }

    IMP_DECL_RULE(static eval);
    IMP_DECL_RULE(static bin_op);
    IMP_DECL_RULE(static un_op);
    IMP_DECL_RULE(static operand);
    IMP_DECL_RULE(static number);
    IMP_DECL_RULE(static variable);
    IMP_DECL_RULE(static sub_expr);
    IMP_DECL_RULE(static full_expr);
    IMP_DECL_RULE(static brkt_expr);

    IMP_MAKE_RULE(full_expr,
        imp::CaptGr(
            imp::Fn<sub_expr>() >> (imp::Set<";\n">() | imp::NoChars()),
            imp::Fn<handler::full_expr>()
        )
    )

    IMP_MAKE_RULE(sub_expr,
        imp::Any(imp::Space()) >> imp::Opt(
            imp::Fn<operand>() >> imp::Any(
                imp::Any(imp::Space()) >> imp::Fn<bin_op>() >>
                imp::Any(imp::Space()) >> imp::Fn<operand>()
            ) >> imp::Any(imp::Space())
        )
    )

    IMP_MAKE_RULE(operand,
        imp::Any(imp::Fn<un_op>()) >> (
            imp::Fn<number>() |
            imp::Fn<variable>() |
            imp::Fn<brkt_expr>()
        )
    )

    IMP_MAKE_RULE(number, (
        imp::Capt<TYPE_VALUE, VALTYPE_NUMBER>(
            imp::Opt(imp::Set<"+-">()) >>
            imp::Some(imp::Digit()) >> imp::Opt(
                imp::Str<".">() >>
                imp::Some(imp::Digit()) >> imp::Opt(
                    imp::Set<"eE">() >>
                    imp::Opt(imp::Set<"+-">()) >>
                    imp::Some(imp::Digit())
                )
            )
        )
    ))

    IMP_MAKE_RULE(variable, (
        imp::Capt<TYPE_VALUE, VALTYPE_VARIABLE>(
            imp::Alpha() >> imp::Any(imp::Alnum())
        )
    ))

    IMP_MAKE_RULE(brkt_expr, (
        imp::Capt<TYPE_OPENBRK, BRKTYPE_ROUND>(
            imp::Str<"(">()
        ) >> imp::Fn<sub_expr>() >>
        imp::Capt<TYPE_CLOSEBRK, BRKTYPE_ROUND>(
            imp::Str<")">()
        )
    ))

    IMP_MAKE_RULE(bin_op, (
        imp::CaptDict<
            {"+",   TYPE_BINOP, OPTYPE_ADD      },
            {"-",   TYPE_BINOP, OPTYPE_SUB      },
            {"*",   TYPE_BINOP, OPTYPE_MUL      },
            {"/",   TYPE_BINOP, OPTYPE_DIV      },
            {"%",   TYPE_BINOP, OPTYPE_MOD      },
            {"//",  TYPE_BINOP, OPTYPE_IDIV     },
            {"^^",  TYPE_BINOP, OPTYPE_POW      },
            {"=",   TYPE_BINOP, OPTYPE_ASSIGN   },
            {"+=",  TYPE_BINOP, OPTYPE_ADDASS   },
            {"-=",  TYPE_BINOP, OPTYPE_SUBASS   },
            {"*=",  TYPE_BINOP, OPTYPE_MULASS   },
            {"/=",  TYPE_BINOP, OPTYPE_DIVASS   },
            {"%=",  TYPE_BINOP, OPTYPE_MODADD   },
            {"//=", TYPE_BINOP, OPTYPE_IDIVASS  },
            {"^^=", TYPE_BINOP, OPTYPE_POWASS   }
        >()
    ))

    IMP_MAKE_RULE(un_op, (
        imp::CaptDict<
            { "+", TYPE_UNOP, OPTYPE_PLUS   },
            { "-", TYPE_UNOP, OPTYPE_MINUS  }
        >()
    ))

    IMP_MAKE_RULE(eval,
        imp::Any(
            imp::LookAhead(imp::AnyChar()) >>
            imp::Fn<full_expr>()
        ) >> imp::NoChars()
    )
}

static void
PrecompileChunk(std::span<const char> spnSource) {
    imp::MemStream
        stream  = {spnSource};
    if (!imp::Eval(imp::Fn<grammar::eval>(), stream)) {
        throw std::runtime_error(std::format(
            "parser failed at: {}", stream.GetPos()
        ));
    }
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