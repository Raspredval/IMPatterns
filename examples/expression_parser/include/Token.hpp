#pragma once

enum TokenType {
    TOKTYPE_VALUE,
    TOKTYPE_EXPR,
    TOKTYPE_TERNOP,
    TOKTYPE_BINOP,
    TOKTYPE_UNOP,
    TOKTYPE_COMMENT
};

enum ExprType {
    EXPRTYPE_FULL,
    EXPRTYPE_SUB,
    EXPRTYPE_ARG,
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
    VALTYPE_ID
};

enum OpType {
    OPTYPE_POSTINC  = 0x01,
    OPTYPE_POSTDEC  = 0x02,
    OPTYPE_CALL     = 0x03,
    OPTYPE_INDEX    = 0x04,
    OPTYPE_CAST     = 0x05,

    OPTYPE_PLUS     = 0x10,
    OPTYPE_MINUS    = 0x11,
    OPTYPE_NOT      = 0x12,
    OPTYPE_PREFINC  = 0x13,
    OPTYPE_PREFDEC  = 0x14,
    OPTYPE_DEREF    = 0x15,
    OPTYPE_ADDROF   = 0x16,

    OPTYPE_POW      = 0x20,

    OPTYPE_MUL      = 0x30,
    OPTYPE_DIV      = 0x31,
    OPTYPE_MOD      = 0x32,

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

    OPTYPE_PIPEBK   = 0x90,
    OPTYPE_PIPEFR   = 0x91,

    OPTYPE_ASSIGN   = 0xA0,
    OPTYPE_ADDASS   = 0xA1,
    OPTYPE_SUBASS   = 0xA2,
    OPTYPE_MULASS   = 0xA3,
    OPTYPE_DIVASS   = 0xA4,
    OPTYPE_MODASS   = 0xA5,
    OPTYPE_POWASS   = 0xA6,
    OPTYPE_CONDOP   = 0xA7
};

inline int
getOpPrecedence(OpType iType) {
    return iType >> 4;
}

inline bool
isLeftAssociable(OpType iType) {
    switch (iType) {
    case OPTYPE_POSTINC:
    case OPTYPE_POSTDEC:
    case OPTYPE_CALL:
    case OPTYPE_INDEX:
    case OPTYPE_CAST:

    case OPTYPE_MUL:
    case OPTYPE_DIV:
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
    case OPTYPE_PIPEBK:
    case OPTYPE_PIPEFR:
        return true;

    default:
        return false;
    }
}

inline bool
isRightAssociable(OpType iType) {
    return !isLeftAssociable(iType);
}