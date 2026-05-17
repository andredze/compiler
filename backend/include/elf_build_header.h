#ifndef ELF_BUILD_HEADER_H
#define ELF_BUILD_HEADER_H

//——————————————————————————————————————————————————————————————————————————————————————————

#include "backend.h"
#include <elf.h>

//——————————————————————————————————————————————————————————————————————————————————————————

BackendErr_t ElfBuildHeader(Elf64_Ehdr* header, size_t section_header_table_offset);

//——————————————————————————————————————————————————————————————————————————————————————————

#endif /* ELF_BUILD_HEADER_H */