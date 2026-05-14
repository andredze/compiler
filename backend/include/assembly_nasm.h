#ifndef ASSEMBLY_NASM_H
#define ASSEMBLY_NASM_H

//——————————————————————————————————————————————————————————————————————————————————————————

#include "instruction.h"
#include "backend.h"

//——————————————————————————————————————————————————————————————————————————————————————————

const int SYSCALL_CODE_EXIT = 0x3c;

//——————————————————————————————————————————————————————————————————————————————————————————

const char* const REG_NAMES_TABLE[] = {
    [REG_RAX] = "rax",
    [REG_RCX] = "rcx",
    [REG_RDX] = "rdx",
    [REG_RBX] = "rbx",
    [REG_RSP] = "rsp",
    [REG_RBP] = "rbp",
    [REG_RSI] = "rsi",
    [REG_RDI] = "rdi",
    [REG_R8 ] = "r8",
    [REG_R9 ] = "r9",
    [REG_R10] = "r10",
    [REG_R11] = "r11",
    [REG_R12] = "r12",
    [REG_R13] = "r13",
    [REG_R14] = "r14",
    [REG_R15] = "r15"
};

//——————————————————————————————————————————————————————————————————————————————————————————

void PrintAsm(FILE* asm_file, const wchar_t* format_string, ...);
// __attribute__((format(wprintf, 2, 3)))

//------------------------------------------------------------------//

BackendErr_t AssembleInstruction(BackendCtx_t* backend_ctx, Instruction_t* instr);

//——————————————————————————————————————————————————————————————————————————————————————————

#endif /* ASSEMBLY_NASM_H */