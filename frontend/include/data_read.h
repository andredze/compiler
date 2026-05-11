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

LangErr_t FrontendReadInputInTree(FrontendCtx* frontend_ctx, char* input_file_path);
LangErr_t TreeReadData           (FrontendCtx* frontend_ctx, const char* data_file_path);

//——————————————————————————————————————————————————————————————————————————————————————————

#endif /* DATA_READ_H */
