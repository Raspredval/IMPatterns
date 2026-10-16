#pragma once
#include <string_view>

enum TokenType {
    TYPE_VALUE,
    TYPE_BINOP,
    TYPE_UNOP,
    TYPE_OPENBRK,
    TYPE_CLOSEBRK,
};

enum ValType {
    VALTYPE_NUMBER,
    VALTYPE_VARIABLE
};

enum OpType {
    OPTYPE_PLUS     = 0x10,
    OPTYPE_MINUS    = 0x11,
    OPTYPE_POW      = 0x20,
    OPTYPE_MUL      = 0x30,
    OPTYPE_DIV      = 0x31,
    OPTYPE_IDIV     = 0x32,
    OPTYPE_MOD      = 0x33,
    OPTYPE_ADD      = 0x40,
    OPTYPE_SUB      = 0x41,
    OPTYPE_ASSIGN   = 0x50,
    OPTYPE_ADDASS   = 0x51,
    OPTYPE_SUBASS   = 0x52,
    OPTYPE_MULASS   = 0x53,
    OPTYPE_DIVASS   = 0x54,
    OPTYPE_IDIVASS  = 0x55,
    OPTYPE_MODADD   = 0x56,
    OPTYPE_POWASS   = 0x57
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
        ValType
            val_type;
        OpType
            op_type;
        BrktType
            brkt_type;
    };
};