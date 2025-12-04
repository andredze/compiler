#ifndef TREE_GRAPH_H
#define TREE_GRAPH_H

//——————————————————————————————————————————————————————————————————————————————————————————

#include "tree_types.h"
#include "math_types.h"
#include <time.h>
#include <sys/stat.h>
#include <stdarg.h>

//——————————————————————————————————————————————————————————————————————————————————————————

int TreePrintElement(const TreeElem_t* data);

TreeErr_t TreeReadBufferDump(const char* buffer, ssize_t pos, const char* fmt, ...);

MathErr_t MathVarsTableDump(const MathCtx_t* math_ctx, const char* fmt, ...);

TreeErr_t TreeDump(MathCtx_t*      math_ctx,
                   const TreeDumpInfo_t* dump_info,
                   const char* fmt, ...);

TreeErr_t vTreeDump(MathCtx_t*      math_ctx,
                    const TreeDumpInfo_t* dump_info,
                    const char* fmt, va_list args);

TreeErr_t TreeOpenLogFile   ();
void      TreeCloseLogFile  ();

TreeErr_t TreeGraphDump        (MathCtx_t*       math_ctx);
void      SetGraphFilepaths    ();
TreeErr_t TreeConvertGraphFile ();

TreeErr_t TreeGraphDumpSubtree(MathCtx_t* math_ctx, TreeNode_t* node);

void      DumpGraphTitle      (FILE* dot_file);
void      TreeNodePrint       (const TreeNode_t* node          );
TreeErr_t TreeNodeDump        (const TreeNode_t* node, FILE* fp, MathCtx_t* math_ctx);
TreeErr_t DumpTreeNodeAndEdges(const TreeNode_t* node, FILE* fp, MathCtx_t* math_ctx);
int       DumpTreeSingleNode  (const TreeNode_t* node, FILE* fp, MathCtx_t* math_ctx);
int       DumpTreeVarNode     (const TreeNode_t* node, FILE* fp, MathCtx_t* math_ctx);
int       DumpTreeEdges       (const TreeNode_t* node, FILE* fp);
int       DumpTreeOpNode      (const TreeNode_t* node, FILE* fp);
int       DumpTreeNumNode     (const TreeNode_t* node, FILE* fp);

//——————————————————————————————————————————————————————————————————————————————————————————

int DumpTreeDefaultNode(
    const TreeNode_t* node,
    const char* str_data,
    const char* color,
    const char* fillcolor,
    const char* fontcolor,
    const char* shape,
    FILE* fp);

int DumpNode(
    const char* name,
    const char* label,
    const char* color,
    const char* fillcolor,
    const char* fontcolor,
    const char* shape,
    FILE* fp);

int DumpEdge(
    const char* node1,
    const char* node2,
    const char* color,
    const char* constraint,
    const char* dir,
    const char* style,
    const char* arrowhead,
    const char* arrowtail,
    const char* label,
    FILE*       fp);

int PrintArg(
    const char* arg_name,
    const char* arg_value,
    int*        is_first_arg,
    FILE*       fp);

//——————————————————————————————————————————————————————————————————————————————————————————

const int MAX_NODE_NAME_LEN = 32;
const int MAX_LABEL_LEN     = 256;
const int MAX_DATA_LEN      = 128;

const char* const IMAGE_FILE_TYPE = "svg";

//——————————————————————————————————————————————————————————————————————————————————————————

#endif /* TREE_GRAPH_H */
