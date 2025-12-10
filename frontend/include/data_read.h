#ifndef DATA_READ_H
#define DATA_READ_H

//——————————————————————————————————————————————————————————————————————————————————————————

#include "tree_commands.h"
#include "tree_dump.h"
#include <sys/stat.h>

//——————————————————————————————————————————————————————————————————————————————————————————

LangErr_t TreeReadInputData(Tree_t* tree);

LangErr_t TreeReadData      (LangCtx_t*     lang_ctx,
                             const char*    data_file_path);

int       ReadFile          (FILE*          fp,
                             char**         buffer_ptr,
                             const char*    file_path);

int       CountSize         (const char* file_path, size_t* size);

//——————————————————————————————————————————————————————————————————————————————————————————

#endif /* DATA_READ_H */
