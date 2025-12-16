#ifndef TREE_DUMP_H
#define TREE_DUMP_H

//——————————————————————————————————————————————————————————————————————————————————————————

#include "tree_types.h"
#include "op_cases.h"
#include "lang_ctx.h"
#include <time.h>
#include <sys/stat.h>
#include <stdarg.h>

//——————————————————————————————————————————————————————————————————————————————————————————

const int MAX_NODE_NAME_LEN = 32;
const int MAX_LABEL_LEN     = 256;
const int MAX_DATA_LEN      = 128;

//——————————————————————————————————————————————————————————————————————————————————————————

#define GRAPH_DUMP_(lang_ctx, node, dump_type, fmt, ...) \
        BEGIN \
        TreeDumpInfo_t _dump_info_ = {TREE_SUCCESS, __func__, __FILE__, __LINE__}; \
        GraphDump((lang_ctx), (node), &_dump_info_, (dump_type), fmt); \
        END

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

    TreeNode_t* node;

    wchar_t name     [MAX_NODE_NAME_LEN];
    wchar_t label    [MAX_LABEL_LEN];
    wchar_t str_data [MAX_DATA_LEN];

    const wchar_t* color;
    const wchar_t* fillcolor;
    const wchar_t* fontcolor;
    const wchar_t* shape;
} NodeDumpParams_t;

//——————————————————————————————————————————————————————————————————————————————————————————

typedef struct EdgeDumpParams
{
    wchar_t node1 [MAX_NODE_NAME_LEN];
    wchar_t node2 [MAX_NODE_NAME_LEN];

    const wchar_t* color;
    const wchar_t* constraint;
    const wchar_t* dir;
    const wchar_t* style;
    const wchar_t* arrowhead;
    const wchar_t* arrowtail;
    const wchar_t* label;
} EdgeDumpParams_t;

//——————————————————————————————————————————————————————————————————————————————————————————

TreeErr_t TreeReadBufferDump(LangCtx_t* lang_ctx, const char* fmt, ...);

LangErr_t LangIdTableDump(LangCtx_t* lang_ctx, const char* fmt, ...);

TreeErr_t TreeDump(LangCtx_t* lang_ctx,
                   const TreeDumpInfo_t* dump_info,
                   NodeDumpType_t dump_type,
                   const char* fmt, ...);

TreeErr_t vTreeDump(LangCtx_t* lang_ctx,
                    const TreeDumpInfo_t* dump_info,
                    NodeDumpType_t dump_type,
                    const char* fmt, va_list args);

TreeErr_t TreeOpenLogFile   (LangCtx_t* lang_ctx);
void      TreeCloseLogFile  (LangCtx_t* lang_ctx);

TreeErr_t TreeGraphDump        (LangCtx_t* lang_ctx, NodeDumpType_t dump_type);
void      SetGraphFilepaths    (LangCtx_t* lang_ctx);
TreeErr_t TreeConvertGraphFile (LangCtx_t* lang_ctx);

TreeErr_t GraphDump(LangCtx_t*     lang_ctx,  TreeNode_t*    node, const TreeDumpInfo_t* dump_info,
                    NodeDumpType_t dump_type, const wchar_t* fmt, ...);

void      DumpGraphTitle      (FILE* dot_file);

//——————————————————————————————————————————————————————————————————————————————————————————

const NodeDumpParams_t DUMMY_NODE_PARAMS = {DUMP_FULL, {}, {}, {}, {}, L"#3E3A22", L"#ecede8", L"#3E3A22", L"record"};

const wchar_t* const DEFAULT_EDGE_COLOR = L"#404040";
const wchar_t* const IMAGE_FILE_TYPE    = L"svg";

//——————————————————————————————————————————————————————————————————————————————————————————

#endif /* TREE_DUMP_H */
