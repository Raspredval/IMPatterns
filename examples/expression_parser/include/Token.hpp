#pragma once
#include <string_view>

enum TokenType {
    TYPE_VALUE,
    TYPE_EXPR,
    TYPE_BINOP,
    TYPE_UNOP,
    TYPE_OPENBRK,
    TYPE_CLOSEBRK,
    TYPE_COMMENT
};

enum ExprType {
    EXPRTYPE_FULL,
    EXPRTYPE_ARG
};

enum CommType {
    COMMTYPE_ENDLINE,
    COMMTYPE_INLINE
};

enum ValType {
    VALTYPE_INT,
    VALTYPE_FLOAT,
    VALTYPE_BOOL,
    VALTYPE_STR,
    VALTYPE_VAR,
    VALTYPE_FUNC
};

enum OpType {
    OPTYPE_PLUS     = 0x10,
    OPTYPE_MINUS    = 0x11,
    OPTYPE_NOT      = 0x12,

    OPTYPE_POW      = 0x20,

    OPTYPE_MUL      = 0x30,
    OPTYPE_DIV      = 0x31,
    OPTYPE_IDIV     = 0x32,
    OPTYPE_MOD      = 0x33,

    OPTYPE_ADD      = 0x40,
    OPTYPE_SUB      = 0x41,

    OPTYPE_LESS     = 0x50,
    OPTYPE_LESSEQ   = 0x51,
    OPTYPE_GREAT    = 0x52,
    OPTYPE_GREATEQ  = 0x53,

    OPTYPE_EQUAL    = 0x60,
    OPTYPE_NOTEQUAL = 0x61,

    OPTYPE_BOOLAND  = 0x70,
    OPTYPE_BOOLOR   = 0x80,

    OPTYPE_ASSIGN   = 0x90,
    OPTYPE_ADDASS   = 0x91,
    OPTYPE_SUBASS   = 0x92,
    OPTYPE_MULASS   = 0x93,
    OPTYPE_DIVASS   = 0x94,
    OPTYPE_IDIVASS  = 0x95,
    OPTYPE_MODASS   = 0x96,
    OPTYPE_POWASS   = 0x97
};

enum BrktType {
    BRKTYPE_ROUND
};

struct Token {
    static int
    getPrecedence(OpType iType) {
        return iType >> 4;
    }

    static bool
    isLeftAssociable(OpType iType) {
        switch (iType) {
        case OPTYPE_MUL:
        case OPTYPE_DIV:
        case OPTYPE_IDIV:
        case OPTYPE_MOD:
        case OPTYPE_ADD:
        case OPTYPE_SUB:
        case OPTYPE_LESS:
        case OPTYPE_LESSEQ:
        case OPTYPE_GREAT:
        case OPTYPE_GREATEQ:
        case OPTYPE_EQUAL:
        case OPTYPE_NOTEQUAL:
        case OPTYPE_BOOLAND:
        case OPTYPE_BOOLOR:
            return true;

        default:
            return false;
        }
    }

    static bool
    isRightAssociable(OpType iType) {
        return !isLeftAssociable(iType);
    }

    std::string_view
        strvValue;
    TokenType
        type;
    union {
        ExprType
            expr_type;
        ValType
            val_type;
        OpType
            op_type;
        BrktType
            brkt_type;
        CommType
            comm_type;
    };
};