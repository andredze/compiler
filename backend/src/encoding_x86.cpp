#include "encoding_x86.h"
#include "bin_code_buffer.h"

//——————————————————————————————————————————————————————————————————————————————————————————

#define _DSL_DEFINE_
#include "dsl.h"

//==========================================================================================

static void BinInstrSetOpcode(BinInstruction_t* bin_instr, 
                              Opcode_t          opcode)
{
    assert(bin_instr);

    bin_instr->info.contains_opcode = 1;
    bin_instr->opcode               = opcode;
}

//==========================================================================================

static void BinInstrSetREXPrefix(BinInstruction_t*     bin_instr,
                                 OperandSize_t         w_bit,
                                 uint8_t               r_bit,
                                 SIBIndexExtension_t   x_bit,
                                 uint8_t               b_bit)
{
    assert(bin_instr);

    bin_instr->info.contains_rex     = 1;
    bin_instr->rex.fixed_bit_pattern = REX_FIRST_4_BITS;
    bin_instr->rex.w                 = w_bit & 1;
    bin_instr->rex.r                 = r_bit & 1;
    bin_instr->rex.x                 = x_bit & 1;
    bin_instr->rex.b                 = b_bit & 1;
}

//==========================================================================================

static void BinInstrSetModRM(BinInstruction_t* bin_instr,
                             ModRMMod_t        mod,
                             uint8_t           reg,
                             uint8_t           rm)
{
    assert(bin_instr);

    bin_instr->info.contains_modrm = 1;
    bin_instr->modrm.mod           = mod & 0b11;
    bin_instr->modrm.reg           = reg & 0b111;
    bin_instr->modrm.rm            = rm  & 0b111;
}

//==========================================================================================

static void BinInstrSetSIB(BinInstruction_t* bin_instr,
                           uint8_t           scale,
                           uint8_t           index,
                           uint8_t           base)
{
    assert(bin_instr);

    bin_instr->info.contains_sib = 1;
    bin_instr->sib.scale         = scale & 0b11;
    bin_instr->sib.index         = index & 0b111;
    bin_instr->sib.base          = base  & 0b111;
}

//==========================================================================================

static void BinInstrSetDisp32(BinInstruction_t* bin_instr,
                              Disp_t            disp)
{
    assert(bin_instr);

    bin_instr->info.contains_disp    = 1;
    bin_instr->disp.size             = BINVALUE_SIZE_4_BYTE;
    bin_instr->disp.data.size_4_byte = disp;
}

//==========================================================================================

static void BinInstrSetImm32(BinInstruction_t* bin_instr,
                             Imm_t             imm)
{
    assert(bin_instr);

    bin_instr->info.contains_imm    = 1;
    bin_instr->imm.size             = BINVALUE_SIZE_4_BYTE;
    bin_instr->imm.data.size_4_byte = imm;
}

//==========================================================================================

static void BinInstrSetOpcodeLower3Bits(BinInstruction_t* bin_instr, uint8_t bits)
{
    assert(bin_instr);

    uint32_t opcode = bin_instr->opcode.data.size_3_byte;

    // wfcprintf(stderr, RED, L"opcode = %x\n", opcode);

    opcode = (opcode >> 3) << 3;
    opcode |= (bits & 0b111);

    bin_instr->opcode.data.size_3_byte = opcode & 0xFFFFFF;
}

//==========================================================================================

static int GetModRMRegExtension(OpcodeType_t opcode)
{
    int extension = OPCODE_CASES_TABLE[opcode].modrm_reg_extension;

    if (extension == -1)
    {
        return 0;
    }

    return extension;
}

//==========================================================================================

static uint8_t GetRegCode(Register_t reg)
{
    return (uint8_t) reg & 0b111;
}

//==========================================================================================

static uint8_t GetModRMExtensionBit(Register_t reg)
{
    return (reg >> 3) & 1;   
}

//==========================================================================================

static uint8_t GetModRMExtensionBitForOperand(Operand_t operand)
{
    switch (operand.type)
    {
        case OPERAND_REG_64:
            return GetModRMExtensionBit(operand.value.reg);
        
        case OPERAND_MEM_64:
            return GetModRMExtensionBit(operand.value.mem.base);

        case OPERAND_IMM_32:
        case OPERAND_REL_32:
        case OPERAND_NONE:
        case OPERAND_TYPE_UNKNOWN:
        case OPERAND_TYPE_COUNT:
        default:
            return 0;
    }

    return 0;
}

//==========================================================================================

static void BinInstrSetREXPrefixAuto(BinInstruction_t*   bin_instr, 
                                     Instruction_t*      instr,
                                     ModRMType_t         op1_modrm_type, 
                                     ModRMType_t         op2_modrm_type,
                                     OperandSize_t       w_bit,
                                     SIBIndexExtension_t x_bit)
{
    assert(bin_instr);
    assert(instr);

    uint8_t r_bit = 0;
    uint8_t b_bit = 0;

    switch (op1_modrm_type)
    {
        case MODRM_TYPE_RM:
            b_bit = GetModRMExtensionBitForOperand(instr->operand_1);
            break;

        case MODRM_TYPE_REG:
            r_bit = GetModRMExtensionBitForOperand(instr->operand_1);
            break;

        case MODRM_TYPE_UNKNOWN:
        default:
            break;
    }

    switch (op2_modrm_type)
    {
        case MODRM_TYPE_RM:
            b_bit = GetModRMExtensionBitForOperand(instr->operand_2);
            break;

        case MODRM_TYPE_REG:
            r_bit = GetModRMExtensionBitForOperand(instr->operand_2);
            break;

        case MODRM_TYPE_UNKNOWN:
        default:
            break;
    }

    BinInstrSetREXPrefix(bin_instr, w_bit, r_bit, x_bit, b_bit);
}

//==========================================================================================

static void BinInstrSetREXPrefixDefault(BinInstruction_t* bin_instr, 
                                        Instruction_t*    instr, 
                                        ModRMType_t       op1_modrm_type,
                                        ModRMType_t       op2_modrm_type)
{
    return BinInstrSetREXPrefixAuto(bin_instr, instr, 
                                    op1_modrm_type, 
                                    op2_modrm_type, 
                                    OPERAND_SIZE_64, 
                                    SIB_INDEX_NO_EXTEND);
}

//==========================================================================================

BackendErr_t EncodeRegReg(BinInstruction_t* bin_instr, Instruction_t* instr)
{
    assert(bin_instr);
    assert(instr);

    ENCODE_VERIFY_(instr->opcode_type == OPCODE_MOV_REG_REG ||
                   instr->opcode_type == OPCODE_ADD_REG_REG ||
                   instr->opcode_type == OPCODE_SUB_REG_REG ||
                   instr->opcode_type == OPCODE_CMP_REG_REG);

    ENCODE_VERIFY_(instr->operand_1.type == OPERAND_REG_64);
    ENCODE_VERIFY_(instr->operand_2.type == OPERAND_REG_64);

    // REX.W + OPCODE + /r
    BinInstrSetREXPrefixDefault(bin_instr, instr, MODRM_TYPE_REG, MODRM_TYPE_RM);

    BinInstrSetModRM(bin_instr, MODRM_MOD_RM_ONLY,
                                GetRegCode(instr->operand_1.value.reg),
                                GetRegCode(instr->operand_2.value.reg));

    return BACKEND_SUCCESS;
}

//==========================================================================================

BackendErr_t EncodeRegMem(BinInstruction_t* bin_instr, Instruction_t* instr)
{
    assert(bin_instr);
    assert(instr);

    ENCODE_VERIFY_(instr->opcode_type    == OPCODE_MOV_REG_MEM);
    ENCODE_VERIFY_(instr->operand_1.type == OPERAND_REG_64);
    ENCODE_VERIFY_(instr->operand_2.type == OPERAND_MEM_64);

    // REX.W + OPCODE + /r
    BinInstrSetREXPrefixDefault(bin_instr, instr, MODRM_TYPE_REG, MODRM_TYPE_RM);

    ModRMMod_t modrm_mod = (instr->operand_2.value.mem.disp == 0) ?
                            MODRM_MOD_RM_ONLY :
                            MODRM_MOD_RM_DISP32; // using only 32 bit in this version
    
    BinInstrSetModRM(bin_instr, modrm_mod,
                                GetRegCode(instr->operand_1.value.reg),
                                GetRegCode(instr->operand_2.value.reg));

    if (modrm_mod == MODRM_MOD_RM_ONLY)
    {
        return BACKEND_SUCCESS;
    }

    BinInstrSetDisp32(bin_instr, instr->operand_2.value.mem.disp);

    return BACKEND_SUCCESS;
}

//==========================================================================================

BackendErr_t EncodeMemReg(BinInstruction_t* bin_instr, Instruction_t* instr)
{
    assert(bin_instr);
    assert(instr);

    ENCODE_VERIFY_(instr->opcode_type    == OPCODE_MOV_MEM_REG);
    ENCODE_VERIFY_(instr->operand_1.type == OPERAND_MEM_64);
    ENCODE_VERIFY_(instr->operand_2.type == OPERAND_REG_64);

    // REX.W + OPCODE + /r
    BinInstrSetREXPrefixDefault(bin_instr, instr, MODRM_TYPE_RM, MODRM_TYPE_REG);

    ModRMMod_t modrm_mod = (instr->operand_1.value.mem.disp == 0) ?
                            MODRM_MOD_RM_ONLY :
                            MODRM_MOD_RM_DISP32; // using only 32 bit in this version
    
    BinInstrSetModRM(bin_instr, modrm_mod,
                                GetRegCode(instr->operand_2.value.reg),
                                GetRegCode(instr->operand_1.value.reg));

    if (modrm_mod == MODRM_MOD_RM_ONLY)
    {
        return BACKEND_SUCCESS;
    }

    BinInstrSetDisp32(bin_instr, instr->operand_1.value.mem.disp);

    return BACKEND_SUCCESS;
}

//==========================================================================================

BackendErr_t EncodeRegImm(BinInstruction_t* bin_instr, Instruction_t* instr)
{
    assert(bin_instr);
    assert(instr);

    ENCODE_VERIFY_(instr->opcode_type == OPCODE_MOV_REG_IMM ||
                   instr->opcode_type == OPCODE_CMP_REG_IMM);

    ENCODE_VERIFY_(instr->operand_1.type == OPERAND_REG_64);
    ENCODE_VERIFY_(instr->operand_2.type == OPERAND_IMM_32);

    // REX.W + OPCODE /[reg_extension] id
    BinInstrSetREXPrefixDefault(bin_instr, instr, MODRM_TYPE_RM, MODRM_TYPE_UNKNOWN);

    BinInstrSetModRM(bin_instr, MODRM_MOD_RM_ONLY,
                                GetModRMRegExtension(instr->opcode_type) & 0b111,
                                GetRegCode(instr->operand_1.value.reg));

    BinInstrSetImm32(bin_instr, instr->operand_2.value.imm);

    return BACKEND_SUCCESS;
}

//==========================================================================================

BackendErr_t EncodeRegNone(BinInstruction_t* bin_instr, Instruction_t* instr)
{
    assert(bin_instr);
    assert(instr);

    ENCODE_VERIFY_(instr->opcode_type    == OPCODE_IMUL_REG ||
                   instr->opcode_type    == OPCODE_IDIV_REG);
    ENCODE_VERIFY_(instr->operand_1.type == OPERAND_REG_64);
    ENCODE_VERIFY_(instr->operand_2.type == OPERAND_NONE);

    // REX.W + OPCODE /[reg_extension]
    BinInstrSetREXPrefixDefault(bin_instr, instr, MODRM_TYPE_RM, MODRM_TYPE_UNKNOWN);

    BinInstrSetModRM(bin_instr, MODRM_MOD_RM_ONLY,
                                GetModRMRegExtension(instr->opcode_type) & 0b111,
                                GetRegCode(instr->operand_1.value.reg));

    return BACKEND_SUCCESS;
}

//==========================================================================================

BackendErr_t EncodeRegNoneShort(BinInstruction_t* bin_instr, Instruction_t* instr)
{
    assert(bin_instr);
    assert(instr);

    ENCODE_VERIFY_(instr->opcode_type    == OPCODE_PUSH_REG ||
                   instr->opcode_type    == OPCODE_POP_REG);
    ENCODE_VERIFY_(instr->operand_1.type == OPERAND_REG_64);
    ENCODE_VERIFY_(instr->operand_2.type == OPERAND_NONE);

    // OPCODE + rd
    // non-64-bit: register is encoded in lower 3 bits of opcode
    // 64-bit:     register is encoded in REX.b + lower 3 bits of opcode
    BinInstrSetOpcodeLower3Bits(bin_instr, GetRegCode(instr->operand_1.value.reg));

    // if 32-bit mode
    if (GetModRMExtensionBitForOperand(instr->operand_1) == 0)
    {
        return BACKEND_SUCCESS;
    }

    // else - set rex prefix with REX.b bit
    BinInstrSetREXPrefixDefault(bin_instr, instr, MODRM_TYPE_RM, MODRM_TYPE_UNKNOWN);

    return BACKEND_SUCCESS;
}

//==========================================================================================

BackendErr_t EncodeNoneNone(BinInstruction_t* bin_instr, Instruction_t* instr)
{
    assert(bin_instr);
    assert(instr);

    ENCODE_VERIFY_(instr->opcode_type    == OPCODE_RET ||
                   instr->opcode_type    == OPCODE_SYSCALL);
    ENCODE_VERIFY_(instr->operand_1.type == OPERAND_NONE);
    ENCODE_VERIFY_(instr->operand_2.type == OPERAND_NONE);

    // OPCODE
    return BACKEND_SUCCESS;
}

//==========================================================================================

BackendErr_t EncodeRelNone(BinInstruction_t* bin_instr, Instruction_t* instr)
{
    assert(bin_instr);
    assert(instr);

    ENCODE_VERIFY_(instr->opcode_type == OPCODE_CALL_REL ||
                   instr->opcode_type == OPCODE_JMP_REL  ||
                   instr->opcode_type == OPCODE_JE_REL   ||
                   instr->opcode_type == OPCODE_JNE_REL  ||
                   instr->opcode_type == OPCODE_JA_REL   ||
                   instr->opcode_type == OPCODE_JAE_REL  ||
                   instr->opcode_type == OPCODE_JB_REL   ||
                   instr->opcode_type == OPCODE_JBE_REL);

    ENCODE_VERIFY_(instr->operand_1.type == OPERAND_REL_32);
    ENCODE_VERIFY_(instr->operand_2.type == OPERAND_NONE);

    // OPCODE + cd
    BinInstrSetDisp32(bin_instr, instr->operand_1.value.rel);

    return BACKEND_SUCCESS;
}

//==========================================================================================

static BackendErr_t EncodeInstruction(BinInstruction_t* bin_instr, Instruction_t* instr)
{
    assert(bin_instr);
    assert(instr);

    OpcodeCase_t opcode_case = OPCODE_CASES_TABLE[instr->opcode_type];

    BinInstrSetOpcode(bin_instr, opcode_case.opcode);
    
    EncodeFunction_t encode_function = opcode_case.encode_function;
    
    if (encode_function == NULL)
    {
        WPRINTERR(L"Can't encode instruction %s", 
                  InstructionGetOpcodeTypeString(instr->opcode_type));
        
        return BACKEND_INSTRUCTION_CAN_NOT_BE_ENCODED;
    }

    return encode_function(bin_instr, instr);
}

//==========================================================================================

static uint8_t ReverseREXPrefix(REXPrefix_t rex)
{
    int byte = (rex.fixed_bit_pattern << 4) |
               (rex.w << 3) |
               (rex.r << 2) |
               (rex.x << 1) |
                rex.b;
    
    return (uint8_t) byte;
}

//==========================================================================================

static uint8_t ReverseModRM(ModRM_t modrm)
{
    int byte = (modrm.mod << 6) |
               (modrm.reg << 3) |
                modrm.rm;

    return (uint8_t) byte;
}

//==========================================================================================

static uint8_t ReverseSIB(SIB_t sib)
{
    int byte = (sib.scale << 5) | 
               (sib.index << 2) |
                sib.base;
    
    return (uint8_t) byte;
}

//==========================================================================================

static void ReverseOpcodeByteOrder(Opcode_t* opcode)
{
    assert(opcode);

    uint32_t right_order    = opcode->data.size_3_byte;
    uint32_t reversed_order = 0;

    for (int i = 0; i < opcode->size; i++)
    {
        reversed_order = (reversed_order << 8) | (right_order & 0xFF);
        right_order >>= 8;
    }

    opcode->data.size_3_byte = reversed_order & 0xFFFFFF;
}

//==========================================================================================

static BackendErr_t GenerateCodeFromBinInstruction(BinCode_t*        bin_code, 
                                                   BinInstruction_t* bin_instr)
{
    assert(bin_instr);
    assert(bin_code);
    assert(bin_code->buffer);
    
    EncodeInfo_t* encode_info = &bin_instr->info;

    if (encode_info->contains_rex)
    {
        uint8_t reversed = ReverseREXPrefix(bin_instr->rex);

        if (BinCodeWrite(bin_code, &reversed, sizeof(reversed)))
            return BACKEND_BINCODE_BUFFER_ERROR;
    }
    if (encode_info->contains_opcode)
    {
        ReverseOpcodeByteOrder(&bin_instr->opcode);
        // argument src = &bin_instr->opcode.data.size_1_byte,
        // however the actual size written in bin_code is specified
        // by bin_instr->opcode.size field and union field doesn't change anything
        if (BinCodeWrite(bin_code,
                         &bin_instr->opcode.data.size_1_byte, 
                         (size_t) bin_instr->opcode.size))
            return BACKEND_BINCODE_BUFFER_ERROR;
    }
    if (encode_info->contains_modrm)
    {
        uint8_t reversed = ReverseModRM(bin_instr->modrm);

        if (BinCodeWrite(bin_code, &reversed, sizeof(reversed)))
            return BACKEND_BINCODE_BUFFER_ERROR;
    }
    if (encode_info->contains_sib)
    {
        uint8_t reversed = ReverseSIB(bin_instr->sib);

        if (BinCodeWrite(bin_code, &reversed, sizeof(reversed)))
            return BACKEND_BINCODE_BUFFER_ERROR;
    }
    if (encode_info->contains_disp)
    {
        // same logic as with opcode union
        if (BinCodeWrite(bin_code, 
                         &bin_instr->disp.data.size_1_byte, 
                         bin_instr->disp.size))
            return BACKEND_BINCODE_BUFFER_ERROR;
    }
    if (encode_info->contains_imm)
    {
        // same logic as with opcode union
        if (BinCodeWrite(bin_code, 
                         &bin_instr->imm.data.size_1_byte, 
                         bin_instr->imm.size))
            return BACKEND_BINCODE_BUFFER_ERROR;
    }

    return BACKEND_SUCCESS;
}

//==========================================================================================

BackendErr_t GenerateCodeFromInstruction(BinCode_t* bin_code, Instruction_t* instr)
{
    assert(instr);

    BinInstruction_t bin_instr = {};
    BackendErr_t     error     = BACKEND_SUCCESS;

    if ((error = EncodeInstruction(&bin_instr, instr)))
    {
        free(instr);
        return error;
    }

    BIN_INSTRUCTION_DUMP(&bin_instr);

    if ((error = GenerateCodeFromBinInstruction(bin_code, &bin_instr)))
    {
        free(instr);
        return error;
    }

    free(instr);

    return BACKEND_SUCCESS;
}

//==========================================================================================

static uint32_t GetReversedOpcode(Opcode_t opcode)
{
    uint32_t bytes    = opcode.data.size_3_byte;
    uint32_t reversed = 0;

    for (int i = 0; i < opcode.size * 8; i++)
    {
        reversed = (reversed << 1) | (bytes & 1);
        bytes >>= 1;
    }

    return reversed;
}

//==========================================================================================

static void BinInstructionRexDump(REXPrefix_t rex)
{
    wcprintf(CYAN, 
LR"(REX prefix
    | fixed w  r  x  b |
    | %04b  %01b  %01b  %01b  %01b |
)", 
    rex.fixed_bit_pattern,
    rex.w,
    rex.r,
    rex.x,
    rex.b);
}

//------------------------------------------------------------------//

#define SET_GET_STRING_CASE(enum_value)         \
        BEGIN                                   \
        case enum_value: return #enum_value;    \
        END

//------------------------------------------------------------------//

static const char* BinInstructionGetModRMModString(ModRMMod_t mod)
{
    switch (mod)
    {
        SET_GET_STRING_CASE(MODRM_MOD_RM_ONLY);
        SET_GET_STRING_CASE(MODRM_MOD_RM_DISP8);
        SET_GET_STRING_CASE(MODRM_MOD_RM_DISP32);
        SET_GET_STRING_CASE(MODRM_MOD_RM_MIXED);
        
        default:
            break;
    }

    return "MODRM_MOD_LIMIT_EXCEEDED";
}

//------------------------------------------------------------------//

#undef SET_OPCODE_STRING_CASE

//------------------------------------------------------------------//

static void BinInstructionOpcodeDump(Opcode_t opcode)
{
    wcprintf(CYAN, 
LR"(opcode:
    size = %d;
    data = %x;
)", 
    opcode.size,
    opcode.data.size_3_byte);
}

//------------------------------------------------------------------//

static void BinInstructionModRMDump(ModRM_t modrm)
{
    wcprintf(CYAN, 
LR"(ModRM byte
    | mod reg rm  |
    | %02b  %03b %03b |
    mod = %s;
)", 
    modrm.mod,
    modrm.reg,
    modrm.rm,
    BinInstructionGetModRMModString((ModRMMod_t) modrm.mod));
}

//------------------------------------------------------------------//

static void BinInstructionSIBDump(SIB_t sib)
{
    wcprintf(CYAN, 
LR"(SIB byte
    | scale  index  base |
    |   %02b    %03b    %03b |
)", 
    sib.scale,
    sib.index,
    sib.base);
}

//------------------------------------------------------------------//

static void BinInstructionBinValueDump(BinValue_t value)
{
    wcprintf(CYAN, L"    size = %d;\n"
                   L"    data = %x;\n",
                   value.size,
                   value.data.size_8_byte);
}

//==========================================================================================

static void BinInstructionDumpPrefixPrint(int contains_field)
{
    if (contains_field)
    {
        wcprintf(CYAN, L"\tCONTAINS ");
    }    
    else
    {    
        wcprintf(CYAN, L"\tNO ");
    }
}

//------------------------------------------------------------------//

void BinInstructionDump(BinInstruction_t* bin_instr,
                        const char*       func,
                        const char*       file,
                        int               line)
{
    assert(bin_instr);

    wcprintf(CYAN, LR"(
===========================================
Dump: BinInstruction_t from %s at %s:%d
-------------------------------------------
bin_instr = %p
{
)",
    func,
    file,
    line,
    bin_instr);

    BinInstructionDumpPrefixPrint(bin_instr->info.contains_rex);
    wcprintf(CYAN, L"REX\n");
    BinInstructionDumpPrefixPrint(bin_instr->info.contains_opcode);
    wcprintf(CYAN, L"OPCODE\n");
    BinInstructionDumpPrefixPrint(bin_instr->info.contains_modrm);
    wcprintf(CYAN, L"MODRM\n");
    BinInstructionDumpPrefixPrint(bin_instr->info.contains_sib);
    wcprintf(CYAN, L"SIB\n");
    BinInstructionDumpPrefixPrint(bin_instr->info.contains_imm);
    wcprintf(CYAN, L"IMM\n");
    BinInstructionDumpPrefixPrint(bin_instr->info.contains_disp);
    wcprintf(CYAN, L"DISP");

    wcprintf(CYAN, 
LR"(
             REX              OPCODE           ModRM             SIB             DISP       IMM
    | fixed w  r  x  b |      opcode      | mod reg rm  | scale  index  base |   disp   |   imm    |
    | %04b  %01b  %01b  %01b  %01b | %-16.08b | %02b  %03b %03b |   %02b    %03b    %03b | %08x | %08x |
    |        %02x        | %-16.02x |      %02x     |         %02x         | %08x | %08x |

    mod = %s;
)", 
    bin_instr->rex.fixed_bit_pattern,
    bin_instr->rex.w,
    bin_instr->rex.r,
    bin_instr->rex.x,
    bin_instr->rex.b,
    GetReversedOpcode(bin_instr->opcode),
    bin_instr->modrm.mod,
    bin_instr->modrm.reg,
    bin_instr->modrm.rm,
    bin_instr->sib.scale,
    bin_instr->sib.index,
    bin_instr->sib.base,
    bin_instr->disp.data.size_4_byte,
    bin_instr->imm.data.size_4_byte,
    ReverseREXPrefix(bin_instr->rex),
    bin_instr->opcode.data.size_3_byte,
    ReverseModRM(bin_instr->modrm),
    ReverseSIB(bin_instr->sib),
    bin_instr->disp.data.size_4_byte,
    bin_instr->imm.data.size_4_byte,
    BinInstructionGetModRMModString((ModRMMod_t) bin_instr->modrm.mod)
    );

    wcprintf(CYAN, LR"(}
===========================================
)");
}

//==========================================================================================

void BinInstructionDumpVerbose(BinInstruction_t* bin_instr, 
                               const char*       func,
                               const char*       file, 
                               int               line)
{
    assert(bin_instr);

    wcprintf(CYAN, LR"(
===========================================
Dump: BinInstruction_t from %s at %s:%d
-------------------------------------------
bin_instr = %p
{
)",
    func,
    file,
    line,
    bin_instr);

    BinInstructionDumpPrefixPrint(bin_instr->info.contains_rex);
    BinInstructionRexDump        (bin_instr->rex);

    BinInstructionDumpPrefixPrint(bin_instr->info.contains_opcode);
    BinInstructionOpcodeDump     (bin_instr->opcode);

    BinInstructionDumpPrefixPrint(bin_instr->info.contains_modrm);
    BinInstructionModRMDump      (bin_instr->modrm);

    BinInstructionDumpPrefixPrint(bin_instr->info.contains_sib);
    BinInstructionSIBDump        (bin_instr->sib);

    BinInstructionDumpPrefixPrint(bin_instr->info.contains_imm);
    wcprintf(CYAN, L"imm:\n");
    BinInstructionBinValueDump(bin_instr->imm);

    BinInstructionDumpPrefixPrint(bin_instr->info.contains_disp);
    wcprintf(CYAN, L"disp:\n");
    BinInstructionBinValueDump(bin_instr->disp);

    wcprintf(CYAN, LR"(}
===========================================
)");
}

//==========================================================================================

#define _DSL_UNDEF_
#include "dsl.h"

//——————————————————————————————————————————————————————————————————————————————————————————
