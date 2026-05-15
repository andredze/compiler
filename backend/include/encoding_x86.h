#ifndef ENCODING_X86_H
#define ENCODING_X86_H

//——————————————————————————————————————————————————————————————————————————————————————————

#include <stdint.h>
#include "instruction.h"
#include "backend.h"
#include "common.h"

//——————————————————————————————————————————————————————————————————————————————————————————

BackendErr_t TestDSLEncoding(BackendCtx_t* backend_ctx);

//——————————————————————————————————————————————————————————————————————————————————————————

const uint8_t REX_FIRST_4_BITS = 0b0100;

//------------------------------------------------------------------//

/*
        REX prefix layout
  7                           0
+---+---+---+---+---+---+---+---+
| 0   1   0   0 | W | R | X | B |
+---+---+---+---+---+---+---+---+
*/
typedef struct REXPrefix
{
    uint8_t fixed_bit_pattern : 4; // obligatory bits 0100
    uint8_t w                 : 1; // 1 if 64-bit operand is used
    uint8_t r                 : 1; // extension to the MODRM.reg
    uint8_t x                 : 1; // extension to the SIB.index
    uint8_t b                 : 1; // extension to the MODRM.rm field or the SIB.base
}
REXPrefix_t;

//------------------------------------------------------------------//

// Used for setting W-bit
typedef enum OperandSize
{
    OPERAND_SIZE_32 = 0,
    OPERAND_SIZE_64 = 1
}
OperandSize_t;

//------------------------------------------------------------------//

// Used for setting X-bit
typedef enum SIBIndexExtension
{
    SIB_INDEX_NO_EXTEND = 0,
    SIB_INDEX_EXTEND    = 1,
}
SIBIndexExtension_t;

//——————————————————————————————————————————————————————————————————————————————————————————

typedef enum OpcodeSize
{
    OPCODE_SIZE_0_BYTE = 0,

    OPCODE_SIZE_1_BYTE = 1,
    OPCODE_SIZE_2_BYTE = 2,
    OPCODE_SIZE_3_BYTE = 3
}
OpcodeSize_t;

//------------------------------------------------------------------//

typedef union OpcodeData
{
    uint8_t  size_0_byte;

    uint8_t  size_1_byte;
    uint16_t size_2_byte;
    uint32_t size_3_byte : 24;
}
OpcodeData_t;

//------------------------------------------------------------------//

typedef struct Opcode
{
    OpcodeData_t data;
    OpcodeSize_t size;
}
Opcode_t;

//——————————————————————————————————————————————————————————————————————————————————————————

/*
        ModR/M byte fields
  7                           0
+---+---+---+---+---+---+---+---+
|  mod  |    reg    |     rm    |
+---+---+---+---+---+---+---+---+

mod (mostly, NOT FOR ALL REGISTERS!):

0b00 = [r/m] 
0b01 = [r/m + disp8] 
0b10 = [r/m + disp32] 
0b11 = r/m 

*/
typedef struct ModRM
{
    uint8_t mod : 2;
    uint8_t reg : 3;
    uint8_t rm  : 3;
}
ModRM_t;

//------------------------------------------------------------------//

typedef enum ModRMMod
{
    MODRM_MOD_RM_MIXED  = 0b00,
    MODRM_MOD_RM_DISP8  = 0b01,
    MODRM_MOD_RM_DISP32 = 0b10,
    MODRM_MOD_RM_REG    = 0b11
}
ModRMMod_t;

//------------------------------------------------------------------//

typedef enum ModRMType
{
    MODRM_TYPE_UNKNOWN,
    MODRM_TYPE_REG,
    MODRM_TYPE_RM
}
ModRMType_t;

//——————————————————————————————————————————————————————————————————————————————————————————

/*
         SIB byte fields
  7                           0
+---+---+---+---+---+---+---+---+
| scale |   index   |    base   |
+---+---+---+---+---+---+---+---+

         base scale index
           |    |    |
           v    v    v
mov rax, [rbp + 8 * rdi]

scale field:
   00 => 1
   01 => 2
   10 => 4
   11 => 8
*/

typedef struct SIB
{
    uint8_t scale : 2;
    uint8_t index : 3; // index register
    uint8_t base  : 3; // base register
}
SIB_t;

//——————————————————————————————————————————————————————————————————————————————————————————

typedef enum BinValueSize
{
    BINVALUE_SIZE_1_BYTE = 1,
    BINVALUE_SIZE_2_BYTE = 2,
    BINVALUE_SIZE_4_BYTE = 4,
    BINVALUE_SIZE_8_BYTE = 8
}
BinValueSize_t;

//------------------------------------------------------------------//

typedef union BinValueData
{
    int8_t  size_1_byte;
    int16_t size_2_byte;
    int32_t size_4_byte;
    int64_t size_8_byte;
}
BinValueData_t;

//——————————————————————————————————————————————————————————————————————————————————————————

typedef struct BinValue
{
    BinValueData_t data;
    BinValueSize_t size;
}
BinValue_t;

//——————————————————————————————————————————————————————————————————————————————————————————

typedef struct EncodeInfo
{
    int contains_rex;
    int contains_opcode;
    int contains_modrm;
    int contains_sib;
    int contains_disp;
    int contains_imm;
}
EncodeInfo_t;

//——————————————————————————————————————————————————————————————————————————————————————————

typedef struct BinInstruction
{
    EncodeInfo_t info;

    REXPrefix_t  rex;
    Opcode_t     opcode;
 
    ModRM_t      modrm;
    SIB_t        sib;
 
    BinValue_t   disp;
    BinValue_t   imm;
}
BinInstruction_t;

//——————————————————————————————————————————————————————————————————————————————————————————

#ifdef ENCODING_DEBUG
    #define BIN_INSTRUCTION_DUMP(bin_instr)                                 \
            BEGIN                                                           \
            BinInstructionDump((bin_instr), __func__, __FILE__, __LINE__);  \
            END
#else
    #define BIN_INSTRUCTION_DUMP(bin_instr) ;
#endif /* ENCODING_DEBUG */

//------------------------------------------------------------------//

void BinInstructionDump(BinInstruction_t* bin_instr, 
                        const char*       func,
                        const char*       file, 
                        int               line);

void BinInstructionDumpVerbose(BinInstruction_t* bin_instr, 
                               const char*       func,
                               const char*       file, 
                               int               line);

BackendErr_t GenerateCodeFromInstruction(BinCode_t* bin_code, Instruction_t* instr);

//——————————————————————————————————————————————————————————————————————————————————————————

#endif /* ENCODING_X86_H */