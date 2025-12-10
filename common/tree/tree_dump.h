#ifndef TREE_DUMP_H
#define TREE_DUMP_H

//——————————————————————————————————————————————————————————————————————————————————————————

#include "tree_types.h"
#include "lang_ctx.h"
#include <time.h>
#include <sys/stat.h>
#include <stdarg.h>

//——————————————————————————————————————————————————————————————————————————————————————————

const int MAX_NODE_NAME_LEN = 32;
const int MAX_LABEL_LEN     = 256;
const int MAX_DATA_LEN      = 128;

//——————————————————————————————————————————————————————————————————————————————————————————

typedef enum NodeDumpType
{
    DUMP_FULL  = 0,
    DUMP_SHORT = 1
} NodeDumpType_t;

//——————————————————————————————————————————————————————————————————————————————————————————

typedef struct NodeDumpParams
{
    NodeDumpType_t dump_type;

    char name     [MAX_NODE_NAME_LEN];
    char label    [MAX_LABEL_LEN];
    char str_data [MAX_DATA_LEN];

    const char* color;
    const char* fillcolor;
    const char* fontcolor;
    const char* shape;
} NodeDumpParams_t;

//——————————————————————————————————————————————————————————————————————————————————————————

typedef struct EdgeDumpParams
{
    char node1 [MAX_NODE_NAME_LEN];
    char node2 [MAX_NODE_NAME_LEN];

    const char* color;
    const char* constraint;
    const char* dir;
    const char* style;
    const char* arrowhead;
    const char* arrowtail;
    const char* label;
} EdgeDumpParams_t;

//——————————————————————————————————————————————————————————————————————————————————————————

TreeErr_t TreeReadBufferDump(LangCtx_t* lang_ctx, const char* fmt, ...);

LangErr_t LangIdTableDump(LangCtx_t* lang_ctx, const char* fmt, ...);

TreeErr_t TreeDump(LangCtx_t* lang_ctx,
                   const TreeDumpInfo_t* dump_info,
                   const char* fmt, ...);

TreeErr_t vTreeDump(LangCtx_t* lang_ctx,
                    const TreeDumpInfo_t* dump_info,
                    const char* fmt, va_list args);

TreeErr_t TreeOpenLogFile   (LangCtx_t* lang_ctx);
void      TreeCloseLogFile  (LangCtx_t* lang_ctx);

TreeErr_t TreeGraphDump        (LangCtx_t* lang_ctx);
void      SetGraphFilepaths    (LangCtx_t* lang_ctx);
TreeErr_t TreeConvertGraphFile (LangCtx_t* lang_ctx);

TreeErr_t TreeGraphDumpSubtree(LangCtx_t* lang_ctx, TreeNode_t* node);

void      DumpGraphTitle      (FILE* dot_file);

//——————————————————————————————————————————————————————————————————————————————————————————

void DumpTreeSingleNode(NodeDumpParams_t* params, FILE* fp, LangCtx_t* lang_ctx);

void DumpNode(NodeDumpParams_t* params, FILE* fp);
void DumpEdge(EdgeDumpParams_t* params, FILE* fp);

void PrintArg(const char* arg_name,
              const char* arg_value,
              int*        is_first_arg,
              FILE*       fp);

//——————————————————————————————————————————————————————————————————————————————————————————

const char* const DEFAULT_EDGE_COLOR = "#404040";
const char* const IMAGE_FILE_TYPE    = "svg";

//——————————————————————————————————————————————————————————————————————————————————————————

#endif /* TREE_DUMP_H */
