#ifndef OPERATOR_H
#define OPERATOR_H

//——————————————————————————————————————————————————————————————————————————————————————————

typedef enum TokenType
{
    TYPE_OP   = 0,
    TYPE_ID   = 1,
    TYPE_NUM  = 2
} TokenType_t;

//——————————————————————————————————————————————————————————————————————————————————————————

typedef enum Operator
{
    OP_NONE           = 0,
    OP_BRACKET_OPEN   = 1,
    OP_BRACKET_CLOSE  = 2,
    OP_IF             = 3,
    OP_ELSE           = 4,
    OP_WHILE          = 5,
    OP_BLOCK_BEGIN    = 6,
    OP_BLOCK_END      = 7,
    OP_PARAM          = 8,
    OP_FUNCTION       = 9,
    OP_ADD            = 10,
    OP_SUB            = 11,
    OP_MUL            = 12,
    OP_DIV            = 13,
    OP_NEWLINE        = 14,
    OP_SEPARATOR      = 15,
    OP_ABORT          = 16,
    OP_OUTPUT         = 17,
    OP_INPUT          = 18,
    OP_ASSIGNMENT     = 19
} Operator_t;

//——————————————————————————————————————————————————————————————————————————————————————————

typedef struct OperatorCase
{
    Operator_t  code;

    const char* code_str;

    const char* name;
    size_t      name_len;

    int         child_count;

} OperatorCase_t;

//——————————————————————————————————————————————————————————————————————————————————————————

#define SET_OP_CASE_(code,          name,                                    child_count) \
        [(code)] = {(code), #code, (name), sizeof(name) / sizeof(char) - 1, (child_count)}

//------------------------------------------------------------------------------------------

const OperatorCase_t OP_CASES_TABLE[] =
{
    SET_OP_CASE_(OP_NONE,          NULL,       0),
    SET_OP_CASE_(OP_BRACKET_OPEN,  "(",        0),
    SET_OP_CASE_(OP_BRACKET_CLOSE, ")",        0),
    SET_OP_CASE_(OP_IF,            "если",     2),
    SET_OP_CASE_(OP_ELSE,          "иначе",    1),
    SET_OP_CASE_(OP_WHILE,         "пока",     2),
    SET_OP_CASE_(OP_BLOCK_BEGIN,   "{",        2),
    SET_OP_CASE_(OP_BLOCK_END,     "}",        0),
    SET_OP_CASE_(OP_PARAM,         "параметр", 1),
    SET_OP_CASE_(OP_FUNCTION,      "функция",  2),
    SET_OP_CASE_(OP_ADD,           "+",        2),
    SET_OP_CASE_(OP_SUB,           "-",        2),
    SET_OP_CASE_(OP_MUL,           "*",        2),
    SET_OP_CASE_(OP_DIV,           "/",        2),
    SET_OP_CASE_(OP_NEWLINE,       ";",        2),
    SET_OP_CASE_(OP_SEPARATOR,     ",",        2),
    SET_OP_CASE_(OP_ABORT,         "аборт",    2), // hlt
    SET_OP_CASE_(OP_OUTPUT,        "печать",   2),
    SET_OP_CASE_(OP_INPUT,         "читать",   2),
    SET_OP_CASE_(OP_ASSIGNMENT,    "=",        2)
};

//------------------------------------------------------------------------------------------

#undef SET_OP_CASE_

//——————————————————————————————————————————————————————————————————————————————————————————

const size_t OPERATORS_COUNT       = sizeof(OP_CASES_TABLE) / sizeof(OP_CASES_TABLE[0]);
const size_t MAX_OPERATOR_NAME_LEN = 128;

//——————————————————————————————————————————————————————————————————————————————————————————

#endif /* OPERATOR_H */
