#ifndef OPCODE_CASES_H
#define OPCODE_CASES_H

//——————————————————————————————————————————————————————————————————————————————————————————

#include "instruction.h"
#include "encoding_x86.h"

//——————————————————————————————————————————————————————————————————————————————————————————

typedef BackendErr_t (*AssembleFunction_t) (BackendCtx_t*, Instruction_t*);

//------------------------------------------------------------------//

typedef BackendErr_t (*EncodeFunction_t) (BinInstruction_t*, Instruction_t*);

//------------------------------------------------------------------//

BackendErr_t EncodeRegReg      (BinInstruction_t* bin_instr, Instruction_t* instr);
BackendErr_t EncodeRegMem      (BinInstruction_t* bin_instr, Instruction_t* instr);
BackendErr_t EncodeMemReg      (BinInstruction_t* bin_instr, Instruction_t* instr);
BackendErr_t EncodeRegImm      (BinInstruction_t* bin_instr, Instruction_t* instr);
BackendErr_t EncodeRegNone     (BinInstruction_t* bin_instr, Instruction_t* instr);
BackendErr_t EncodeRegNoneShort(BinInstruction_t* bin_instr, Instruction_t* instr);
BackendErr_t EncodeNoneNone    (BinInstruction_t* bin_instr, Instruction_t* instr);
BackendErr_t EncodeRelNone     (BinInstruction_t* bin_instr, Instruction_t* instr);

//------------------------------------------------------------------//

typedef struct OpcodeCase
{
    const char*        opcode_name;
    OperandType_t      op1_type;
    OperandType_t      op2_type;
    Opcode_t           opcode;
    int                modrm_reg_extension;
    EncodeFunction_t   encode_function;
    AssembleFunction_t assemble_function;
}
OpcodeCase_t;

// TODO: in table AssembleFunction

//——————————————————————————————————————————————————————————————————————————————————————————

#define SET_OPCODE_CASE(opcode, op1_type, op2_type, op_size, bin, modrm_reg, encode_func, assemble_func) \
    [opcode] = {#opcode, op1_type, op2_type, {.data = {.size_ ## op_size ## _byte = bin},                \
                                              .size = OPCODE_SIZE_ ## op_size ## _BYTE},                 \
                modrm_reg, encode_func, assemble_func}

//------------------------------------------------------------------//

const OpcodeCase_t OPCODE_CASES_TABLE[OPCODE_COUNT] = {
//                      opcode        |    op1_type    |   op2_type  |size|bin_code|ModRm.reg| encode_func  |  assemble_func
    SET_OPCODE_CASE(OPCODE_UNKNOWN    , OPERAND_UNKNOWN, OPERAND_UNKNOWN, 0,   0x00,   -1, NULL               , NULL), // < Opcode from intel manual >
    SET_OPCODE_CASE(OPCODE_MOV_REG_REG, OPERAND_REG_64 , OPERAND_REG_64 , 1,   0x8B,   -1, EncodeRegReg       , NULL), // REX.W + 8B /r
    SET_OPCODE_CASE(OPCODE_MOV_REG_MEM, OPERAND_REG_64 , OPERAND_MEM_64 , 1,   0x8B,   -1, EncodeRegMem       , NULL), // REX.W + 8B /r
    SET_OPCODE_CASE(OPCODE_MOV_MEM_REG, OPERAND_MEM_64 , OPERAND_REG_64 , 1,   0x89,   -1, EncodeMemReg       , NULL), // REX.W + 89 /r
    SET_OPCODE_CASE(OPCODE_MOV_REG_IMM, OPERAND_REG_64 , OPERAND_IMM_32 , 1,   0xC7,    0, EncodeRegImm       , NULL), // REX.W + C7 /0 id
    SET_OPCODE_CASE(OPCODE_ADD_REG_REG, OPERAND_REG_64 , OPERAND_REG_64 , 1,   0x03,   -1, EncodeRegReg       , NULL), // REX.W + 03 /r
    SET_OPCODE_CASE(OPCODE_SUB_REG_REG, OPERAND_REG_64 , OPERAND_REG_64 , 1,   0x2B,   -1, EncodeRegReg       , NULL), // REX.W + 2B /r
    SET_OPCODE_CASE(OPCODE_IMUL_REG   , OPERAND_REG_64 , OPERAND_NONE   , 1,   0xF7,    5, EncodeRegNone      , NULL), // REX.W + F7 /5
    SET_OPCODE_CASE(OPCODE_IDIV_REG   , OPERAND_REG_64 , OPERAND_NONE   , 1,   0xF7,    7, EncodeRegNone      , NULL), // REX.W + F7 /7
    SET_OPCODE_CASE(OPCODE_PUSH_REG   , OPERAND_REG_64 , OPERAND_NONE   , 1,   0x50,   -1, EncodeRegNoneShort , NULL), // 50+rd
    SET_OPCODE_CASE(OPCODE_POP_REG    , OPERAND_REG_64 , OPERAND_NONE   , 1,   0x58,   -1, EncodeRegNoneShort , NULL), // 58+rd
    SET_OPCODE_CASE(OPCODE_CALL_REL   , OPERAND_REL_32 , OPERAND_NONE   , 1,   0xE8,   -1, EncodeRelNone      , NULL), // E8 cd
    SET_OPCODE_CASE(OPCODE_RET        , OPERAND_NONE   , OPERAND_NONE   , 1,   0xC3,   -1, EncodeNoneNone     , NULL), // C3
    SET_OPCODE_CASE(OPCODE_JMP_REL    , OPERAND_REL_32 , OPERAND_NONE   , 1,   0xE9,   -1, EncodeRelNone      , NULL), // E9 cd 
    SET_OPCODE_CASE(OPCODE_JE_REL     , OPERAND_REL_32 , OPERAND_NONE   , 2, 0x0F84,   -1, EncodeRelNone      , NULL), // 0F 84 cd 
    SET_OPCODE_CASE(OPCODE_JNE_REL    , OPERAND_REL_32 , OPERAND_NONE   , 2, 0x0F85,   -1, EncodeRelNone      , NULL), // 0F 85 cd 
    SET_OPCODE_CASE(OPCODE_JA_REL     , OPERAND_REL_32 , OPERAND_NONE   , 2, 0x0F87,   -1, EncodeRelNone      , NULL), // 0F 87 cd 
    SET_OPCODE_CASE(OPCODE_JAE_REL    , OPERAND_REL_32 , OPERAND_NONE   , 2, 0x0F83,   -1, EncodeRelNone      , NULL), // 0F 83 cd 
    SET_OPCODE_CASE(OPCODE_JB_REL     , OPERAND_REL_32 , OPERAND_NONE   , 2, 0x0F82,   -1, EncodeRelNone      , NULL), // 0F 82 cd 
    SET_OPCODE_CASE(OPCODE_JBE_REL    , OPERAND_REL_32 , OPERAND_NONE   , 2, 0x0F86,   -1, EncodeRelNone      , NULL), // 0F 86 cd 
    SET_OPCODE_CASE(OPCODE_SYSCALL    , OPERAND_NONE   , OPERAND_NONE   , 2, 0x0F05,   -1, EncodeNoneNone     , NULL), // 0F 05
    SET_OPCODE_CASE(OPCODE_CMP_REG_REG, OPERAND_REG_64 , OPERAND_REG_64 , 1,   0x3B,   -1, EncodeRegReg       , NULL), // REX.W + 3B /r
    SET_OPCODE_CASE(OPCODE_CMP_REG_IMM, OPERAND_REG_64 , OPERAND_IMM_32 , 1,   0x81,    7, EncodeRegImm       , NULL), // REX.W + 81 /7 id
};

//------------------------------------------------------------------//

#undef SET_OPCODE_CASE_

//——————————————————————————————————————————————————————————————————————————————————————————

#endif /* OPCODE_CASES_H */