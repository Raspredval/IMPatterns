#pragma once
#include <string_view>

enum TokenType : unsigned {
    TYPE_VALUE,
    TYPE_BINOP,
    TYPE_UNOP,
    TYPE_OPENBRK,
    TYPE_CLOSEBRK,
};

enum ValType : unsigned {
    VALTYPE_NUMBER,
    VALTYPE_VARIABLE
};

enum OpType : unsigned {
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

enum BrktType : unsigned {
    BRKTYPE_ROUND
};

struct Token {
    static unsigned
    getPrecedence(OpType uType) {
        return uType >> 4;
    }

    static bool
    isLeftAssociable(OpType uType) {
        switch (uType) {
        case OPTYPE_POW:
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
    isRightAssociable(OpType uType) {
        return !isLeftAssociable(uType);
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