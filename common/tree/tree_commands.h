#ifndef TREE_H
#define TREE_H

//——————————————————————————————————————————————————————————————————————————————————————————

#include "lang_ctx.h"
#include "tree_types.h"
#include <stdarg.h>
#include "tree_dump.h"

//——————————————————————————————————————————————————————————————————————————————————————————

#ifdef TREE_DEBUG

#define DEBUG_TREE_CHECK(tree_ptr, fmt, ...)                            \
        BEGIN                                                           \
        TreeErr_t tree_check_status = TreeCheck((tree_ptr),             \
                                                __PRETTY_FUNCTION__,    \
                                                __FILE__,               \
                                                __LINE__,               \
                                                (fmt), ##__VA_ARGS__);  \
        if ((tree_check_status != TREE_SUCCESS))                        \
        {                                                               \
            return MATH_TREE_ERROR;                                     \
        }                                                               \
        END

#define TREE_CALL_DUMP(tree_ptr, fmt, ...)                                                  \
        BEGIN                                                                               \
        TreeDumpInfo_t dump_info = {TREE_SUCCESS, __PRETTY_FUNCTION__, __FILE__, __LINE__}; \
        if (TreeDump((tree_ptr), &dump_info, (fmt), ##__VA_ARGS__))                         \
        {                                                                                   \
            return MATH_TREE_ERROR;                                                         \
        }                                                                                   \
        END

#else

#define DEBUG_TREE_CHECK(tree_ptr, fmt, ...)  ;
#define TREE_CALL_DUMP(tree_ptr, fmt, ...)    ;

#endif /* TREE_DEBUG */

//——————————————————————————————————————————————————————————————————————————————————————————

TreeNode_t* LangOperatorNodeCtor  (LangCtx_t*  lang_ctx, Operator_t  opcode,
                                   TreeNode_t* left,     TreeNode_t* right);

TreeNode_t* LangIdentifierNodeCtor(LangCtx_t*  lang_ctx, size_t     id_index);
TreeNode_t* LangNumberNodeCtor    (LangCtx_t*  lang_ctx, double     number  );

TreeErr_t TreeVerify           (const Tree_t* tree);
TreeErr_t TreeCtor             (Tree_t*     tree);

TreeNode_t* TreeNodeCtor       (Tree_t* tree, TokenData_t data, TreeNode_t* left, TreeNode_t* right, TreeNode_t* parent);
TreeErr_t   TreeGetData        (TreeNode_t* node, TokenType_t type, TokenData_t data);
TreeNode_t* TreeCopySubtree    (Tree_t* dest_tree, TreeNode_t* node, TreeNode_t* parent);
TreeErr_t   TreeDtor           (Tree_t*     tree);

TreeErr_t TreeNodeVerify       (const Tree_t* tree, TreeNode_t* node, size_t* calls_count, TreeNode_t* parent);
TreeErr_t TreeSingleNodeDtor   (TreeNode_t*  node    , Tree_t* tree);
TreeErr_t TreeNodeDtor         (TreeNode_t*  node    , Tree_t* tree);
TreeErr_t TreeSubtreesDtor     (TreeNode_t*  node    , Tree_t* tree);
TreeErr_t TreeLeftSubtreeDtor  (TreeNode_t*  node    , Tree_t* tree);
TreeErr_t TreeRightSubtreeDtor (TreeNode_t*  node    , Tree_t* tree);
TreeErr_t TreeSubtreeDtor      (TreeNode_t** node_ptr, Tree_t* tree);

#ifdef TREE_DEBUG
TreeErr_t TreeCheck(LangCtx_t*  lang_ctx,
                    const char* func,
                    const char* file,
                    int         line,
                    const char* fmt, ...);
#endif /* TREE_DEBUG */

int  CompareDoubles  (double val1, double val2);

//——————————————————————————————————————————————————————————————————————————————————————————

const double DOUBLE_EPS = 1e-7;

//——————————————————————————————————————————————————————————————————————————————————————————

#endif /* TREE_H */
