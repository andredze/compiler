#ifndef INSTRUCTION_H
#define INSTRUCTION_H

//——————————————————————————————————————————————————————————————————————————————————————————

#include <stdint.h>
#include "backend.h"

//——————————————————————————————————————————————————————————————————————————————————————————

typedef enum Register
{
    REG_RAX = 0, 
    REG_RCX = 1, 
    REG_RDX = 2, 
    REG_RBX = 3,

    REG_RSP = 4,
    REG_RBP = 5, 
    
    REG_RSI = 6, 
    REG_RDI = 7, 

    REG_R8  = 8, 
    REG_R9  = 9, 
    REG_R10 = 10, 
    REG_R11 = 11,
    REG_R12 = 12, 
    REG_R13 = 13, 
    REG_R14 = 14, 
    REG_R15 = 15,

    // REG_UNKNOWN = 16
}
Register_t;
//——————————————————————————————————————————————————————————————————————————————————————————

typedef int32_t Disp_t;

//------------------------------------------------------------------//

typedef struct MemoryOperand
{
    Register_t base;
    Disp_t     disp;
}
MemoryOperand_t;

//——————————————————————————————————————————————————————————————————————————————————————————

typedef enum OperandType
{
    OPERAND_UNKNOWN,
    
    OPERAND_NONE,

    OPERAND_REG_64,
    OPERAND_IMM_32,
    OPERAND_MEM_64,
    OPERAND_REL_32,

    OPERAND_TYPE_COUNT
}
OperandType_t;

//——————————————————————————————————————————————————————————————————————————————————————————

typedef int32_t Imm_t;
typedef int32_t RelativeAddress_t;

//------------------------------------------------------------------//

typedef union OperandValue
{
    Register_t          reg;
    Imm_t               imm;
    MemoryOperand_t     mem;
    RelativeAddress_t   rel;

    int                 none;
}
OperandValue_t;

//——————————————————————————————————————————————————————————————————————————————————————————

typedef struct Operand
{
    OperandType_t   type;
    OperandValue_t  value;
}
Operand_t;

//——————————————————————————————————————————————————————————————————————————————————————————

/* Using intel notation */
typedef enum OpcodeType
{
    OPCODE_UNKNOWN,

    OPCODE_MOV_REG_REG, 
    OPCODE_MOV_REG_MEM, 
    OPCODE_MOV_MEM_REG, 
    OPCODE_MOV_REG_IMM,
    OPCODE_ADD_REG_REG,
    OPCODE_SUB_REG_REG,
    OPCODE_IMUL_REG,
    OPCODE_IDIV_REG,
    OPCODE_PUSH_REG,
    OPCODE_POP_REG,
    OPCODE_CALL_REL,
    OPCODE_RET,
    OPCODE_JMP_REL,
    OPCODE_JE_REL, 
    OPCODE_JNE_REL,
    OPCODE_JA_REL, 
    OPCODE_JAE_REL,
    OPCODE_JB_REL, 
    OPCODE_JBE_REL,
    OPCODE_SYSCALL,
    OPCODE_CMP_REG_REG,
    OPCODE_CMP_REG_IMM,

    OPCODE_COUNT
}
OpcodeType_t;

//——————————————————————————————————————————————————————————————————————————————————————————

typedef struct Instruction
{
    OpcodeType_t opcode_type;

    Operand_t    operand_1;
    Operand_t    operand_2;
}
Instruction_t;

//——————————————————————————————————————————————————————————————————————————————————————————

const char* InstructionGetOpcodeTypeString(OpcodeType_t opcode_type);

//------------------------------------------------------------------//

/* Note: returned instr has to be freed */
Instruction_t* InstructionCreateNoneNone(OpcodeType_t opcode);
Instruction_t* InstructionCreateRelNone (OpcodeType_t opcode, RelativeAddress_t rel);
Instruction_t* InstructionCreateMemNone (OpcodeType_t opcode, Register_t base, Disp_t disp);
Instruction_t* InstructionCreateRegImm  (OpcodeType_t opcode, Register_t reg1, Imm_t imm);
Instruction_t* InstructionCreateMemReg  (OpcodeType_t opcode, Register_t base, Disp_t disp, Register_t reg2);
Instruction_t* InstructionCreateRegMem  (OpcodeType_t opcode, Register_t reg1, Register_t base, Disp_t disp);
Instruction_t* InstructionCreateRegNone (OpcodeType_t opcode, Register_t reg1);
Instruction_t* InstructionCreateRegReg  (OpcodeType_t opcode, Register_t reg1, Register_t reg2);

//------------------------------------------------------------------//

#ifdef BACKEND_DEBUG
    #define INSTRUCTION_DUMP(instr)                                   \
            BEGIN                                                     \
            InstructionDump((instr), __func__, __FILE__, __LINE__);   \
            END
#else
    #define INSTRUCTION_DUMP(instr) ;
#endif /* BACKEND_DEBUG */

//------------------------------------------------------------------//

/* Note: returned instr has to be freed */
Instruction_t* InstructionCreate(OpcodeType_t   opcode,
                                 OperandValue_t operand1_value,
                                 OperandValue_t operand2_value);

void InstructionDump(Instruction_t* instr, const char* func, const char* file, int line);

//——————————————————————————————————————————————————————————————————————————————————————————

#endif /* INSTRUCTION_H */