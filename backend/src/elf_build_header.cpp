#include "elf_build_header.h"
#include "elf_build.h"

//==========================================================================================

BackendErr_t ElfBuildHeader(Elf64_Ehdr* header, size_t section_header_table_offset)
{
    assert(header);

    header->e_ident[EI_MAG0]       = ELFMAG0; // 0x7f
    header->e_ident[EI_MAG1]       = ELFMAG1; // 'E'
    header->e_ident[EI_MAG2]       = ELFMAG2; // 'L'
    header->e_ident[EI_MAG3]       = ELFMAG3; // 'F'
    header->e_ident[EI_CLASS]      = ELFCLASS64;
    header->e_ident[EI_DATA]       = ELFDATA2LSB;
    header->e_ident[EI_VERSION]    = EV_CURRENT;
    header->e_ident[EI_OSABI]      = ELFOSABI_SYSV;
    header->e_ident[EI_ABIVERSION] = 0;

    for (int ei = EI_PAD; ei < EI_NIDENT; ei++)
    {
        header->e_ident[ei] = 0;
    }

    header->e_type      = ET_REL;
    header->e_machine   = EM_X86_64;
    header->e_version   = 1; // EV_CURRENT;
    header->e_entry     = 0; // ignore (relocatable file)
    header->e_phoff     = 0; // ignore (relocatable file)
    header->e_shoff     = section_header_table_offset;
    header->e_flags     = 0;
    header->e_ehsize    = sizeof(Elf64_Ehdr);
    header->e_phentsize = 0; // ignore (relocatable file) 
    header->e_phnum     = 0; // ignore (relocatable file) 
    header->e_shentsize = sizeof(SectionHeader_t);
    header->e_shnum     = SECTION_HEADERS_COUNT;
    header->e_shstrndx  = SH_SH_STRTAB_INDEX;

    return BACKEND_SUCCESS;
}

//==========================================================================================
