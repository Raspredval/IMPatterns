#include <MappedFile.hpp>
#include <Patterns.hpp>

#include <set>
#include <string_view>

#include "Token.hpp"
#include "Print.hpp"


namespace grammar {
    using namespace imp;

    struct SrcPos {
        intptr_t
            iLine, iColumn;
    };

    struct PosTracker {
        intptr_t
            iLastLnBrk,
            iLineCount;

        SrcPos
        GetPos(intptr_t iCurPos) const noexcept {
            return {
                .iLine      = this->iLineCount + 1,
                .iColumn    = (iCurPos - this->iLastLnBrk) + 1
            };
        }

        void
        PushLnBrk(intptr_t iCurPos) {
            this->iLineCount    += 1;
            this->iLastLnBrk    = iCurPos + 1;
        }
    };

    struct TypeTracker {
        std::set<std::string_view>
            setTypes;
    };

    struct State {
        PosTracker  p;
        TypeTracker t;
    };

    IMP_DECL_RULE(static eval);
    IMP_DECL_RULE(static comment);
    IMP_DECL_RULE(static bin_op);
    IMP_DECL_RULE(static pref_un_op);
    IMP_DECL_RULE(static post_un_op);
    IMP_DECL_RULE(static call_op);
    IMP_DECL_RULE(static index_op);
    IMP_DECL_RULE(static operand);
    IMP_DECL_RULE(static number);
    IMP_DECL_RULE(static boolean);
    IMP_DECL_RULE(static string);
    IMP_DECL_RULE(static variable);
    IMP_DECL_RULE(static sub_expr);
    IMP_DECL_RULE(static full_expr);
    IMP_DECL_RULE(static brkt_expr);
    IMP_DECL_RULE(static type_expr);
    IMP_DECL_RULE(static line_break);

    IMP_MAKE_RULE(comment, (
        Capt<{TOKTYPE_COMMENT, COMMTYPE_ENDLINE}>(
            Str<"//">() >> Any(NegSet<"\n">())
        )
    ))

    IMP_MAKE_RULE(full_expr, (
        Any(Set<Space>()) >>
        CaptGr<{TOKTYPE_EXPR, EXPRTYPE_FULL}>(
            Opt(Fn<sub_expr>())
        ) >>
        Any(Set<Space>()) >> Opt(Fn<comment>()) >> (
            NoChars() | Str<";">() | Fn<line_break>()
        )
    ))

    IMP_MAKE_RULE(line_break, (
        Str<"\n">() /=
        [] (MemStream& stream, Match m, CaptureView, UserData usr_val) -> Match {
            if (m) {
                State*
                    lpState = (State*)usr_val.ptr;
                lpState->p.PushLnBrk(stream.GetPos());
            }

            return m;
        }
    ))

    IMP_MAKE_RULE(sub_expr,
        Fn<operand>() >> Any(
            Any(Set<Space>()) >> Fn<bin_op>() >>
            Any(Set<Space>()) >> Fn<operand>()
        )
    )

    IMP_MAKE_RULE(operand,
        Any(Fn<pref_un_op>() >> Any(Set<Space>())) >> (
            Fn<number>()   |
            Fn<boolean>()  |
            Fn<string>()   |
            Fn<variable>() |
            Fn<brkt_expr>()
        ) >>
        Any(Any(Set<Space>()) >> Fn<post_un_op>())
    )

    IMP_MAKE_RULE(number, (
        Capt<{TOKTYPE_VALUE, VALTYPE_INT}>(
            Some(Set<Digit>())
        ) >> Opt(
            CaptMerge<{TOKTYPE_VALUE, VALTYPE_FLOAT}>(
                Str<".">() >> Some(Set<Digit>())
            )
        )
    ))

    IMP_MAKE_RULE(boolean, (
        Capt<{TOKTYPE_VALUE, VALTYPE_BOOL}>(
            Dict<"true", "false">()
        )
    ))

    IMP_MAKE_RULE(variable, (
        NegDict<"if", "else">(
            Capt<{TOKTYPE_VALUE, VALTYPE_ID}>(
                Set<Alpha + "_">() >> Any(Set<Alnum + "_">())
            )
        )
    ))

    IMP_MAKE_RULE(string, (
        Capt<{TOKTYPE_VALUE, VALTYPE_STR}>(
            Str<"\"">() >>
            Any(NegSet<"\"\n">()) >>
            Str<"\"">()
        )
    ))

    IMP_MAKE_RULE(brkt_expr, (
        Str<"(">() >> Any(Set<Space>()) >>
        CaptGr<{TOKTYPE_EXPR, EXPRTYPE_SUB}>(Fn<sub_expr>()) >>
        Any(Set<Space>()) >> Str<")">()
    ))

    IMP_MAKE_RULE(bin_op, (
        CaptDict<
            { "^^",  { TOKTYPE_BINOP, OPTYPE_POW      }},
            { "*",   { TOKTYPE_BINOP, OPTYPE_MUL      }},
            { "/",   { TOKTYPE_BINOP, OPTYPE_DIV      }},
            { "%",   { TOKTYPE_BINOP, OPTYPE_MOD      }},
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
            { "^^=", { TOKTYPE_BINOP, OPTYPE_POWASS   }}
        >()
    ))

    IMP_MAKE_RULE(pref_un_op, (
        CaptDict<
            { "++", { TOKTYPE_UNOP, OPTYPE_PREFINC  }},
            { "--", { TOKTYPE_UNOP, OPTYPE_PREFDEC  }},
            { "+",  { TOKTYPE_UNOP, OPTYPE_PLUS     }},
            { "-",  { TOKTYPE_UNOP, OPTYPE_MINUS    }},
            { "!",  { TOKTYPE_UNOP, OPTYPE_NOT      }}
        >() | Fn<type_expr>()
    ))

    IMP_MAKE_RULE(type_expr, (
        Str<"(">() >> Any(Set<Space>()) >>
        CaptGr<{TOKTYPE_UNOP, OPTYPE_CAST}>(
            Fn<variable>(),
            [] (MemStream& stream, Match m, CaptureView, UserData usr_val) -> Match {
                if (m) {
                    std::string_view
                        strvTypename    = m.GetStringView(stream);
                    State*
                        lpState         = (State*)usr_val.ptr;
                    if (!lpState->t.setTypes.contains(strvTypename))
                        m.Toggle();
                }

                return m;
            }
        ) >> Any(Set<Space>()) >> Str<")">()
    ))

    IMP_MAKE_RULE(post_un_op, (
        CaptDict<
            { "++", { TOKTYPE_UNOP, OPTYPE_POSTINC  }},
            { "--", { TOKTYPE_UNOP, OPTYPE_POSTDEC  }}
        >() | Fn<call_op>() | Fn<index_op>()
    ))

    IMP_MAKE_RULE(call_op, (
        CaptGr<{TOKTYPE_UNOP, OPTYPE_CALL}>(
            Str<"(">() >> Any(Set<Space>()) >>
            Opt(
                CaptGr<{TOKTYPE_EXPR, EXPRTYPE_ARG}>(Fn<sub_expr>()) >> Any(Set<Space>()) >>
                Any(
                    Str<",">() >> Any(Set<Space>()) >>
                    CaptGr<{TOKTYPE_EXPR, EXPRTYPE_ARG}>(Fn<sub_expr>()) >>
                    Any(Set<Space>())
                )
            ) >> Str<")">()
        )
    ))

    IMP_MAKE_RULE(index_op, (
        CaptGr<{TOKTYPE_UNOP, OPTYPE_INDEX}>(
            Str<"[">() >> Any(Set<Space>()) >>
            CaptGr<{TOKTYPE_EXPR, EXPRTYPE_ARG}>(Fn<sub_expr>()) >> Any(Set<Space>()) >>
            Any(
                Str<",">() >> Any(Set<Space>()) >>
                CaptGr<{TOKTYPE_EXPR, EXPRTYPE_ARG}>(Fn<sub_expr>()) >>
                Any(Set<Space>())
            ) >> Str<"]">()
        )
    ))

    IMP_MAKE_RULE(eval,
        NoChars() | (Fn<full_expr>() >> Fn<eval>())
    )
}

int main() {
    std::string_view
        strvFilename    = "./assets/test.txt";

    imp::MappedFile
        mapped_file     = {strvFilename};
    if (!mapped_file) {
        std::print(stderr,
            "failed to open file: {}", strvFilename);
        return -1;
    }

    imp::Captures
        tokens  = {};
    imp::MemStream
        stream  = {mapped_file.GetView()};
    grammar::State
        state   = {
            .p = { .iLastLnBrk = 0, .iLineCount = 0 },
            .t = { .setTypes = {"int", "float", "string", "bool"} }
        };

    if (!imp::Eval(imp::Fn<grammar::eval>(), stream, tokens, &state)) {
        grammar::SrcPos
            src_pos = state.p.GetPos(stream.GetPos());
        std::print(stderr,
            "parser failed at: ln {} col {}\n",
            src_pos.iLine, src_pos.iColumn
        );
        return -1;
    }

    PrintTokens(stream, tokens);
    return 0;
}