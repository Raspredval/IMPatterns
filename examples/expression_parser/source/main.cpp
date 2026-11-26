#include <MappedFile.hpp>
#include <Patterns.hpp>


#include <cassert>
#include <string_view>

#include "Token.hpp"
#include "Print.hpp"
#include "LineTracker.hpp"
#include "TypeTracker.hpp"

struct State {
    LineTracker l;
    TypeTracker t;
};

#define IMP_CAPTURETAG  Token
#define IMP_USERDATA    State&
using pt = IMP_PATTERNS();

namespace grammar {
    IMP_DECL_RULE(static eval);
    IMP_DECL_RULE(static endl_comment);
    IMP_DECL_RULE(static inln_comment);
    IMP_DECL_RULE(static cond_op);
    IMP_DECL_RULE(static bin_op);
    IMP_DECL_RULE(static pref_un_op);
    IMP_DECL_RULE(static post_un_op);
    IMP_DECL_RULE(static call_op);
    IMP_DECL_RULE(static index_op);
    IMP_DECL_RULE(static operand);
    IMP_DECL_RULE(static number);
    IMP_DECL_RULE(static boolean);
    IMP_DECL_RULE(static string);
    IMP_DECL_RULE(static identifier);
    IMP_DECL_RULE(static sub_expr);
    IMP_DECL_RULE(static full_expr);
    IMP_DECL_RULE(static brkt_expr);
    IMP_DECL_RULE(static cast_op);
    IMP_DECL_RULE(static line_break);
    IMP_DECL_RULE(static spacing);
    IMP_DECL_RULE(static pipe_op);

    IMP_MAKE_RULE(endl_comment, (
        pt::Capt<{TOKTYPE_COMMENT, COMMTYPE_ENDLINE}>(
            pt::Str<"//">() >> pt::Any(pt::NegSet<imp::LineBreak>())
        ) >> (pt::Fn<line_break>() | pt::NoChars())
    ))

    IMP_MAKE_RULE(spacing, (
        pt::Any(pt::Set<imp::Space>()) >> pt::Any(
            (pt::Fn<line_break>() | pt::Fn<inln_comment>() | pt::Fn<endl_comment>()) >> pt::Any(pt::Set<imp::Space>())
        )
    ))

    IMP_MAKE_RULE(inln_comment, (
        pt::Capt<{TOKTYPE_COMMENT, COMMTYPE_INLINE}> (
            pt::Str<"/*">() >> pt::Any(pt::NegSet<"*\n">()) >> pt::Any(
                (pt::Fn<line_break>() | pt::LookAhead(pt::Not(pt::Str<"*/">())) >> pt::Str<"*">()) >>
                pt::Any(pt::NegSet<"*\n">())
            ) >> pt::Str<"*/">()
        )
    ))

    IMP_MAKE_RULE(full_expr, (
        pt::Fn<spacing>() >> pt::Opt(
            pt::CaptGr<{TOKTYPE_EXPR, EXPRTYPE_FULL}>(
                pt::Fn<sub_expr>()
            ) >> pt::Fn<spacing>()
        ) >> pt::Str<";">() >> pt::Fn<spacing>()
    ))

    IMP_MAKE_HANDLER(static line_break_handler, {
        if (m) {
            usr_val.l.InsertLine(stream.GetPos());
        }

        return m;
    })

    IMP_MAKE_RULE(line_break, (
        pt::Set<imp::LineBreak>() / pt::Fn<line_break_handler>()
    ))

    IMP_MAKE_RULE(sub_expr,
        pt::Fn<operand>() >> pt::Any(
            pt::Fn<spacing>() >> (
                pt::Fn<pipe_op>() | pt::Fn<cond_op>() |
                pt::Fn<bin_op>() >> pt::Fn<spacing>() >> pt::Fn<operand>()
            )
        )
    )

    IMP_MAKE_RULE(pipe_op, (
        pt::CaptDict<
            { "|>", { TOKTYPE_BINOP, OPTYPE_PIPEBK }},
            { "|<", { TOKTYPE_BINOP, OPTYPE_PIPEFR }}
        >() >> pt::Fn<spacing>() >>
        (pt::Fn<brkt_expr>() | pt::Fn<identifier>()) >>
        pt::Fn<spacing>() >> pt::Fn<call_op>()
    ))

    IMP_MAKE_RULE(cond_op, (
        pt::CaptGr<{TOKTYPE_TERNOP, OPTYPE_CONDOP}>(
            pt::Set<"?">() >> pt::Fn<spacing>() >>
            pt::CaptGr<{TOKTYPE_EXPR, EXPRTYPE_ARG}>(pt::Fn<sub_expr>()) >>
            pt::Fn<spacing>() >> pt::Set<":">() >> pt::Fn<spacing>() >>
            pt::CaptGr<{TOKTYPE_EXPR, EXPRTYPE_ARG}>(pt::Fn<sub_expr>())
        )
    ))

    IMP_MAKE_RULE(operand,
        pt::Any(
            pt::Fn<pref_un_op>() >> pt::Fn<spacing>()) >> (
                pt::Fn<number>()     |
                pt::Fn<boolean>()    |
                pt::Fn<string>()     |
                pt::Fn<identifier>() |
                pt::Fn<brkt_expr>()
            ) >> pt::Any(pt::Fn<spacing>() >> pt::Fn<post_un_op>())
    )

    IMP_MAKE_RULE(number, (
        pt::Capt<{TOKTYPE_VALUE, VALTYPE_INT}>(
            pt::Some(pt::Set<imp::Digit>())
        ) >> pt::Opt(
            pt::CaptMerge<{TOKTYPE_VALUE, VALTYPE_FLOAT}>(
                pt::Str<".">() >> pt::Some(pt::Set<imp::Digit>())
            )
        )
    ))

    IMP_MAKE_RULE(boolean, (
        pt::Capt<{TOKTYPE_VALUE, VALTYPE_BOOL}>(
            pt::Dict<"true", "false">()
        )
    ))

    IMP_MAKE_RULE(identifier, (
        pt::NegDict<"if", "else">(
            pt::Capt<{TOKTYPE_VALUE, VALTYPE_ID}>(
                pt::Set<imp::Alpha + "_">() >> pt::Any(pt::Set<imp::Alnum + "_">())
            )
        )
    ))

    IMP_MAKE_RULE(string, (
        pt::Capt<{TOKTYPE_VALUE, VALTYPE_STR}>(
            pt::Str<"\"">() >>
            pt::Any(pt::NegSet<"\"\n">()) >>
            pt::Str<"\"">()
        )
    ))

    IMP_MAKE_RULE(brkt_expr, (
        pt::Str<"(">() >> pt::Fn<spacing>() >>
        pt::CaptGr<{TOKTYPE_EXPR, EXPRTYPE_SUB}>(pt::Fn<sub_expr>()) >>
        pt::Fn<spacing>() >> pt::Str<")">()
    ))

    IMP_MAKE_RULE(bin_op, (
        pt::CaptDict<
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
        pt::CaptDict<
            { "++", { TOKTYPE_UNOP, OPTYPE_PREFINC  }},
            { "--", { TOKTYPE_UNOP, OPTYPE_PREFDEC  }},
            { "+",  { TOKTYPE_UNOP, OPTYPE_PLUS     }},
            { "-",  { TOKTYPE_UNOP, OPTYPE_MINUS    }},
            { "!",  { TOKTYPE_UNOP, OPTYPE_NOT      }},
            { "*",  { TOKTYPE_UNOP, OPTYPE_DEREF    }},
            { "&",  { TOKTYPE_UNOP, OPTYPE_ADDROF   }}
        >() | pt::Fn<cast_op>()
    ))

    IMP_MAKE_HANDLER(cast_op_handler, {
        if (m) {
            std::string_view
                strvTypename    = m.GetStringView(stream);
            if (!usr_val.t.HasType(strvTypename))
                m.Toggle();
        }

        return m;
    })

    IMP_MAKE_RULE(cast_op, (
        pt::Str<"(">() >> pt::Fn<spacing>() >>
        pt::CaptGr<{TOKTYPE_UNOP, OPTYPE_CAST}>(
            pt::Fn<identifier>(), pt::Fn<cast_op_handler>()
        ) >> pt::Fn<spacing>() >> pt::Str<")">()
    ))

    IMP_MAKE_RULE(post_un_op, (
        pt::CaptDict<
            { "++", { TOKTYPE_UNOP, OPTYPE_POSTINC  }},
            { "--", { TOKTYPE_UNOP, OPTYPE_POSTDEC  }}
        >() | pt::Fn<call_op>() | pt::Fn<index_op>()
    ))

    IMP_MAKE_RULE(call_op, (
        pt::CaptGr<{TOKTYPE_UNOP, OPTYPE_CALL}>(
            pt::Str<"(">() >> pt::Fn<spacing>() >>
            pt::Opt(
                pt::CaptGr<{TOKTYPE_EXPR, EXPRTYPE_ARG}>(pt::Fn<sub_expr>()) >>
                pt::Fn<spacing>() >> pt::Any(
                    pt::Str<",">() >> pt::Fn<spacing>() >>
                    pt::CaptGr<{TOKTYPE_EXPR, EXPRTYPE_ARG}>(pt::Fn<sub_expr>()) >>
                    pt::Fn<spacing>()
                )
            ) >> pt::Str<")">()
        )
    ))

    IMP_MAKE_RULE(index_op, (
        pt::CaptGr<{TOKTYPE_UNOP, OPTYPE_INDEX}>(
            pt::Str<"[">() >> pt::Fn<spacing>() >>
            pt::CaptGr<{TOKTYPE_EXPR, EXPRTYPE_ARG}>(pt::Fn<sub_expr>()) >>
            pt::Fn<spacing>() >> pt::Any(
                pt::Str<",">() >> pt::Fn<spacing>() >>
                pt::CaptGr<{TOKTYPE_EXPR, EXPRTYPE_ARG}>(pt::Fn<sub_expr>()) >>
                pt::Fn<spacing>()
            ) >> pt::Str<"]">()
        )
    ))

    IMP_MAKE_RULE(eval,
        pt::NoChars() | pt::Fn<full_expr>() >> pt::Fn<eval>()
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

    imp::Captures<Token>
        tokens  = {};
    imp::MemStream
        stream  = {mapped_file.GetView()};
    State
        state   = {};

    if (!pt::Eval(pt::Fn<grammar::eval>(), stream, tokens, state)) {
        SrcPos
            src_pos = state.l.GetSrcPos(stream.GetPos());
        std::print(stderr,
            "parser failed at: ln {} col {}\n",
            src_pos.iLn, src_pos.iCol
        );
        return -1;
    }

    PrintTokens(stream, tokens, state.l);
    return 0;
}