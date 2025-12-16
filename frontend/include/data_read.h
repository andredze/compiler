#ifndef DATA_READ_H
#define DATA_READ_H

//——————————————————————————————————————————————————————————————————————————————————————————

#include "tree_commands.h"
#include "tree_dump.h"
#include <sys/stat.h>
#include <stdlib.h>
#include <wchar.h>
#include "io_file.h"

//——————————————————————————————————————————————————————————————————————————————————————————

LangErr_t TreeReadInputData (LangCtx* lang_ctx);
LangErr_t TreeReadData      (LangCtx* lang_ctx, const char* data_file_path);

//——————————————————————————————————————————————————————————————————————————————————————————

#endif /* DATA_READ_H */
