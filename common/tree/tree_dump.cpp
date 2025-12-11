#include "tree_dump.h"

//——————————————————————————————————————————————————————————————————————————————————————————

static TreeErr_t TreeDumpSetDebugFilePaths  (LangCtx_t* lang_ctx);
static TreeErr_t TreeDumpSetDirs            (LangCtx_t* lang_ctx);
static TreeErr_t TreeDumpSetLogFilePath     (LangCtx_t* lang_ctx);
static TreeErr_t TreeDumpMakeDirs           (LangCtx_t* lang_ctx);
static void      TreeDumpSetTime            (LangCtx_t* lang_ctx);

//——————————————————————————————————————————————————————————————————————————————————————————

static TreeErr_t TreeDumpSetDebugFilePaths(LangCtx_t* lang_ctx)
{
    assert(lang_ctx);

    TreeDumpSetTime(lang_ctx);

    if (TreeDumpSetDirs(lang_ctx))
        return TREE_FILE_ERROR;

    DPRINTF("> creating TREE LOGS directories:\n"
            "\tlog_dir: %s\n"
            "\timg_dir: %s\n"
            "\tdot_dir: %s\n",
            lang_ctx->debug.log_dir,
            lang_ctx->debug.img_dir,
            lang_ctx->debug.dot_dir);

    if (TreeDumpMakeDirs(lang_ctx))
        return TREE_FILE_ERROR;

    DPRINTF("> TREE LOGS directories successfully created!\n");

    if (TreeDumpSetLogFilePath(lang_ctx))
        return TREE_FILE_ERROR;

    DPRINTF("> TREE LOG file path: \"%s\"\n", lang_ctx->debug.log_file_path);

    return TREE_SUCCESS;
}

//------------------------------------------------------------------------------------------

static void TreeDumpSetTime(LangCtx_t* lang_ctx)
{
    time_t rawtime = time(NULL);

    struct tm* info = localtime(&rawtime);

    strftime(lang_ctx->debug.str_time, sizeof(lang_ctx->debug.str_time), "[%Y-%m-%d_%H%M%S]", info);
}

//------------------------------------------------------------------------------------------

static TreeErr_t TreeDumpMakeDirs(LangCtx_t* lang_ctx)
{
    mkdir(lang_ctx->debug.log_dir, 0777);
    mkdir(lang_ctx->debug.img_dir, 0777);
    mkdir(lang_ctx->debug.dot_dir, 0777);

    return TREE_SUCCESS;
}

//------------------------------------------------------------------------------------------

static TreeErr_t TreeDumpSetDirs(LangCtx_t* lang_ctx)
{
    if (snprintf(lang_ctx->debug.log_dir, sizeof(lang_ctx->debug.log_dir), "log/%s",
                 lang_ctx->debug.str_time) < 0)
    {
        PRINTERR("Error with setting \"log_dir\"");
        return TREE_FILE_ERROR;
    }
    if (snprintf(lang_ctx->debug.img_dir, sizeof(lang_ctx->debug.img_dir), "%s/%s",
                 lang_ctx->debug.log_dir, IMAGE_FILE_TYPE) < 0)
    {
        PRINTERR("Error with setting \"img_dir\"");
        return TREE_FILE_ERROR;
    }
    if (snprintf(lang_ctx->debug.dot_dir, sizeof(lang_ctx->debug.dot_dir), "%s/dot",
                 lang_ctx->debug.log_dir) < 0)
    {
        PRINTERR("Error with setting \"dot_dir\"");
        return TREE_FILE_ERROR;
    }

    return TREE_SUCCESS;
}

//------------------------------------------------------------------------------------------

static TreeErr_t TreeDumpSetLogFilePath(LangCtx_t* lang_ctx)
{
    if (snprintf(lang_ctx->debug.log_file_path, sizeof(lang_ctx->debug.log_file_path),
                 "%s/tree.html", lang_ctx->debug.log_dir) < 0)
    {
        PRINTERR("Error with setting log_file_path");
        return TREE_FILE_ERROR;
    }

    return TREE_SUCCESS;
}

//------------------------------------------------------------------------------------------

TreeErr_t TreeDump(LangCtx_t*            lang_ctx,
                   const TreeDumpInfo_t* dump_info,
                   NodeDumpType_t dump_type,
                   const char* fmt, ...)
{
    va_list args = {};
    va_start(args, fmt);

    TreeErr_t result = vTreeDump(lang_ctx, dump_info, dump_type, fmt, args);

    va_end(args);

    return result;
}

//------------------------------------------------------------------------------------------

LangErr_t LangIdTableDump(LangCtx_t* lang_ctx, const char* fmt, ...)
{
    assert(lang_ctx != NULL);

    va_list args = {};
    va_start(args, fmt);

    FILE* fp = lang_ctx->debug.fp;

    fprintf(fp, "<pre><h4><font color=blue>");

    vfprintf(fp, fmt, args);

    fprintf(fp, "</h4></font>");

    fprintf(fp, "vars_table [%p]:\n\n"
                "size     = %zu\n"
                "capacity = %zu\n\n",
                lang_ctx->id_table.data,
                lang_ctx->id_table.size,
                lang_ctx->id_table.capacity);

    fprintf(fp, "index: ");

    for (size_t i = 0; i < lang_ctx->id_table.capacity; i++)
    {
        fprintf(fp, "%12zu |", i);
    }

    fprintf(fp, "\nnames: ");

    for (size_t i = 0; i < lang_ctx->id_table.capacity; i++)
    {
        fprintf(fp, "%12s |", lang_ctx->id_table.data[i]);
    }

    va_end(args);

    fflush(fp);

    return LANG_SUCCESS;
}

//------------------------------------------------------------------------------------------

TreeErr_t TreeReadBufferDump(LangCtx_t* lang_ctx, const char* fmt, ...)
{
    assert(fmt != NULL);

    int   pos    = (int) (lang_ctx->code - lang_ctx->buffer);
    char* buffer = lang_ctx->buffer;

    va_list args = {};
    va_start(args, fmt);

    FILE* fp = lang_ctx->debug.fp;

    fprintf(fp, "<pre><h4><font color=green>");

    vfprintf(fp, fmt, args);

    fprintf(fp, "</h4></font>\n"
                "<font color=gray>");

    fprintf(fp, "\"");

    for (int i = 0; i < pos; i++)
    {
        fprintf(fp, "%c", buffer[i]);
    }

    fprintf(fp, "</font><font color=red>%c</font>", buffer[pos]);

    if (*(buffer + pos) != '\0')
    {
        fprintf(fp, "<font color=blue>%s</font>\n\n", buffer + pos + 1);
    }

    fprintf(fp, "\"");

    va_end(args);

    fflush(fp);

    return TREE_SUCCESS;
}

//------------------------------------------------------------------------------------------

TreeErr_t vTreeDump(LangCtx_t* lang_ctx,
                    const TreeDumpInfo_t* dump_info,
                    NodeDumpType_t dump_type,
                    const char* fmt, va_list args)
{
    assert(dump_info != NULL);
    assert(lang_ctx  != NULL);

    Tree_t* tree = &lang_ctx->tree;
    FILE*   fp   = lang_ctx->debug.fp;

    fprintf(fp, "<pre>\n<h3><font color=blue>");

    vfprintf(fp, fmt, args);

    fprintf(fp, "</font></h3>");

    fprintf(fp, dump_info->error == TREE_SUCCESS ?
                "<font color=green><b>" :
                "<font color=red><b>ERROR: ");

    fprintf(fp, "%s (code %d)</b></font>\n"
                "TREE DUMP called from %s at %s:%d\n\n",
                TREE_STR_ERRORS[dump_info->error],
                dump_info->error,
                dump_info->func,
                dump_info->file,
                dump_info->line);

    fprintf(fp, "tree [%p]:\n\n"
                "size  = %zu;\n"
                "dummy = %p;\n",
                tree, tree->size, tree->dummy);

    TreeErr_t graph_error = TREE_SUCCESS;

    if ((graph_error = TreeGraphDump(lang_ctx, dump_type)))
    {
        fflush(fp);
        return graph_error;
    }

    int image_width = tree->size <= 5 ? 25 : 50;

    fprintf(fp, "\n<img src = svg/%s.svg width = %d%%>\n\n"
                "============================================================="
                "=============================================================\n\n",
                 lang_ctx->debug.graph_file_name, image_width);

    fflush(fp);

    return TREE_SUCCESS;
}

//------------------------------------------------------------------------------------------

TreeErr_t TreeOpenLogFile(LangCtx_t* lang_ctx)
{
    if (TreeDumpSetDebugFilePaths(lang_ctx))
        return TREE_FILE_ERROR;

    lang_ctx->debug.fp = fopen(lang_ctx->debug.log_file_path, "w");

    if (lang_ctx->debug.fp == NULL)
    {
        PRINTERR("Opening logfile %s failed", lang_ctx->debug.log_file_path);
        return TREE_FILE_ERROR;
    }

    return TREE_SUCCESS;
}

//------------------------------------------------------------------------------------------

void TreeCloseLogFile(LangCtx_t* lang_ctx)
{
    fclose(lang_ctx->debug.fp);
    lang_ctx->debug.fp = NULL;
}

//------------------------------------------------------------------------------------------

static void ASTNodeDump(TreeNode_t* node,     FILE*          fp,
                        LangCtx_t*  lang_ctx, NodeDumpType_t dump_type);

//——————————————————————————————————————————————————————————————————————————————————————————

TreeErr_t TreeGraphDumpSubtree(LangCtx_t* lang_ctx, TreeNode_t* node, NodeDumpType_t dump_type)
{
    assert(lang_ctx != NULL);

    Tree_t*   tree  = &lang_ctx->tree;
    TreeErr_t error = TREE_SUCCESS;

    if (tree == NULL)
    {
        PRINTERR("TREE_NULL");
        return    TREE_NULL;
    }

    SetGraphFilepaths(lang_ctx);

    FILE* dot_file = fopen(lang_ctx->debug.dot_file_path, "w");

    lang_ctx->debug.graphs_count++;

    if (dot_file == NULL)
    {
        PRINTERR("Failed opening logfile");
        return TREE_DUMP_ERROR;
    }

    DumpGraphTitle(dot_file);

    ASTNodeDump(node, dot_file, lang_ctx, dump_type);

    fprintf(dot_file, "}\n");

    fclose(dot_file);

    if ((error = TreeConvertGraphFile(lang_ctx)))
        return error;

    int image_width = tree->size <= 5 ? 25 : 50;

    fprintf(lang_ctx->debug.fp, "\n<img src = svg/%s.svg width = %d%%>\n\n"
                        "============================================================="
                        "=============================================================\n\n",
                         lang_ctx->debug.graph_file_name, image_width);

    return TREE_SUCCESS;
}

//——————————————————————————————————————————————————————————————————————————————————————————

static void DumpDefaultTreeNode(NodeDumpParams_t* params, FILE* fp);

//——————————————————————————————————————————————————————————————————————————————————————————

TreeErr_t TreeGraphDump(LangCtx_t* lang_ctx, NodeDumpType_t dump_type)
{
    assert(lang_ctx != NULL);

    Tree_t*   tree  = &lang_ctx->tree;
    TreeErr_t error = TREE_SUCCESS;

    if (tree == NULL)
    {
        PRINTERR("TREE_NULL");
        return    TREE_NULL;
    }

    SetGraphFilepaths(lang_ctx);

    FILE* dot_file = fopen(lang_ctx->debug.dot_file_path, "w");

    lang_ctx->debug.graphs_count++;

    if (dot_file == NULL)
    {
        PRINTERR("Failed opening logfile");
        return TREE_DUMP_ERROR;
    }

    DumpGraphTitle(dot_file);

    NodeDumpParams_t dummy_params = DUMMY_NODE_PARAMS;

    snprintf(dummy_params.name, sizeof(dummy_params.name), "dummy: node%p", tree->dummy);
    dummy_params.dump_type = dump_type;
    snprintf(dummy_params.str_data, sizeof(dummy_params.str_data), "type = PZN | value = PZN");

    DumpDefaultTreeNode(&dummy_params, dot_file);

    if (tree->dummy->right != NULL)
    {
        ASTNodeDump(tree->dummy->right, dot_file, lang_ctx, dump_type);
    }

    fprintf(dot_file, "}\n");

    fclose(dot_file);

    if ((error = TreeConvertGraphFile(lang_ctx)))
        return error;

    return TREE_SUCCESS;
}

//------------------------------------------------------------------------------------------

void SetGraphFilepaths(LangCtx_t* lang_ctx)
{
    snprintf(lang_ctx->debug.graph_file_name,
             sizeof(lang_ctx->debug.graph_file_name),
             "graph_%04d",
             lang_ctx->debug.graphs_count);

    snprintf(lang_ctx->debug.dot_file_path,
             sizeof(lang_ctx->debug.dot_file_path),
             "%s/%s.dot",
             lang_ctx->debug.dot_dir,
             lang_ctx->debug.graph_file_name);

    snprintf(lang_ctx->debug.img_file_path,
             sizeof(lang_ctx->debug.img_file_path),
             "%s/%s.%s",
             lang_ctx->debug.img_dir,
             lang_ctx->debug.graph_file_name,
             IMAGE_FILE_TYPE);
}

//------------------------------------------------------------------------------------------

void DumpGraphTitle(FILE* dot_file)
{
    assert(dot_file != NULL);

    fprintf(dot_file,
    "digraph Tree\n{\n\t"
    R"(ranksep=0.75;
    nodesep=0.5;
    node [
        fontname  = "Arial",
        shape     = "Mrecord",
        style     = "filled",
        color     = "#3E3A22",
        fillcolor = "#E3DFC9",
        fontcolor = "#3E3A22"
    ];)""\n");
}

//------------------------------------------------------------------------------------------

TreeErr_t TreeConvertGraphFile(LangCtx_t* lang_ctx)
{
    char command[MAX_COMMAND_LEN] = {};

    snprintf(command, sizeof(command), "dot %s -T %s -o %s",
                                       lang_ctx->debug.dot_file_path,
                                       IMAGE_FILE_TYPE,
                                       lang_ctx->debug.img_file_path);

    int result = system(command);

    if (result != 0)
    {
        PRINTERR("TREE_SYSTEM_FUNC_ERROR");
        return    TREE_SYSTEM_FUNC_ERROR;
    }

    return TREE_SUCCESS;
}

//——————————————————————————————————————————————————————————————————————————————————————————

static void ASTDumpNodeWithEdges(TreeNode_t* node,     FILE*          fp,
                                 LangCtx_t*  lang_ctx, NodeDumpType_t dump_type);

static void DumpNode         (NodeDumpParams_t* params, FILE* fp);
static void DumpEdge         (EdgeDumpParams_t* params, FILE* fp);
static void ASTDumpSingleNode(NodeDumpParams_t* params, FILE* fp, LangCtx_t* lang_ctx);

//——————————————————————————————————————————————————————————————————————————————————————————

static void ASTNodeDump(TreeNode_t* node,     FILE*          fp,
                        LangCtx_t*  lang_ctx, NodeDumpType_t dump_type)
{
    assert(lang_ctx != NULL);
    assert(node     != NULL);
    assert(fp       != NULL);

    if (node->left)
        ASTNodeDump(node->left, fp, lang_ctx, dump_type);

    ASTDumpNodeWithEdges(node, fp, lang_ctx, dump_type);

    if (node->right)
        ASTNodeDump(node->right, fp, lang_ctx, dump_type);
}

//------------------------------------------------------------------------------------------

static void ASTDumpNodeWithEdges(TreeNode_t* node,     FILE*          fp,
                                 LangCtx_t*  lang_ctx, NodeDumpType_t dump_type)
{
    NodeDumpParams_t node_params = {};

    node_params.node      = node;
    node_params.dump_type = dump_type;

    snprintf(node_params.name, sizeof(node_params.name), "node%p", node);

    ASTDumpSingleNode(&node_params, fp, lang_ctx);

    //----------------------------------------------------------------------//

    EdgeDumpParams_t edge_params = { .color = DEFAULT_EDGE_COLOR };

    snprintf(edge_params.node1, sizeof(edge_params.node1), "node%p", node);

    if (node->left != NULL)
    {
        snprintf(edge_params.node2, sizeof(edge_params.node2), "node%p", node->left);
        DumpEdge(&edge_params, fp);
    }
    if (node->right != NULL)
    {
        snprintf(edge_params.node2, sizeof(edge_params.node2), "node%p", node->right);
        DumpEdge(&edge_params, fp);
    }
}

//——————————————————————————————————————————————————————————————————————————————————————————

static void DumpNodeDataOperator    (NodeDumpParams_t* params, LangCtx_t* lang_ctx);
static void DumpNodeDataIdentifier  (NodeDumpParams_t* params, LangCtx_t* lang_ctx);
static void DumpNodeDataNumber      (NodeDumpParams_t* params, LangCtx_t* lang_ctx);

//------------------------------------------------------------------------------------------

void (* const DUMP_NODE_DATA_TABLE[]) (NodeDumpParams_t* params, LangCtx_t* lang_ctx) =
{
    [TYPE_OP ] = DumpNodeDataOperator,
    [TYPE_ID ] = DumpNodeDataIdentifier,
    [TYPE_NUM] = DumpNodeDataNumber
};

//——————————————————————————————————————————————————————————————————————————————————————————

static void ASTDumpSingleNode(NodeDumpParams_t* params, FILE* fp, LangCtx_t* lang_ctx)
{
    assert(lang_ctx != NULL);
    assert(params   != NULL);
    assert(fp       != NULL);

    DUMP_NODE_DATA_TABLE[params->node->data.type](params, lang_ctx);

    params->color     = TYPE_CASES_TABLE[params->node->data.type].color;
    params->fillcolor = TYPE_CASES_TABLE[params->node->data.type].fillcolor;
    params->fontcolor = TYPE_CASES_TABLE[params->node->data.type].fontcolor;
    params->shape     = TYPE_CASES_TABLE[params->node->data.type].shape;

    DumpDefaultTreeNode(params, fp);
}

//------------------------------------------------------------------------------------------

static void DumpNodeDataOperator(NodeDumpParams_t* params, LangCtx_t* lang_ctx)
{
    assert(lang_ctx != NULL);
    assert(params   != NULL);

    snprintf(params->str_data, sizeof(params->str_data),
             "type = %s | code = %s | value = %s",
             TYPE_CASES_TABLE[params->node->data.type].name,
             OP_CASES_TABLE[params->node->data.value.opcode].code_str,
             OP_CASES_TABLE[params->node->data.value.opcode].name);
}

//------------------------------------------------------------------------------------------

static void DumpNodeDataIdentifier(NodeDumpParams_t* params, LangCtx_t* lang_ctx)
{
    assert(lang_ctx != NULL);
    assert(params   != NULL);

    snprintf(params->str_data, sizeof(params->str_data),
             "type = %s | value = %s (%zu)",
             TYPE_CASES_TABLE[params->node->data.type].name,
             lang_ctx->id_table.data[params->node->data.value.id_index],
             params->node->data.value.id_index);
}

//------------------------------------------------------------------------------------------

static void DumpNodeDataNumber(NodeDumpParams_t* params, LangCtx_t* lang_ctx)
{
    assert(lang_ctx != NULL);
    assert(params   != NULL);

    snprintf(params->str_data, sizeof(params->str_data),
             "type = %s | value = %lg",
             TYPE_CASES_TABLE[params->node->data.type].name,
             params->node->data.value.number);
}

//——————————————————————————————————————————————————————————————————————————————————————————

static inline int DumpAllowsRecordLabel(NodeDumpParams_t* params);

//——————————————————————————————————————————————————————————————————————————————————————————

static void DumpDefaultTreeNode(NodeDumpParams_t* params, FILE* fp)
{
    assert(params != NULL);
    assert(fp     != NULL);

    snprintf(params->name, sizeof(params->name), "node%p", params->node);

    TreeNode_t* node = params->node;

    if (params->dump_type == DUMP_SHORT)
    {
        snprintf(params->label, sizeof(params->label), "%s", params->str_data);
    }
    else if (DumpAllowsRecordLabel(params))
    {
        snprintf(params->label, sizeof(params->label),
                 "{ %p | %s | parent = %p | { left = %p | right = %p }}",
                 node, params->str_data, node->parent, node->left, node->right);
    }
    else
    {
        snprintf(params->label, sizeof(params->label),
                 "%p \\n %s \\n parent = %p \\n left = %p \\n right = %p",
                 node, params->str_data, node->parent, node->left, node->right);
    }

    DumpNode(params, fp);
}

//------------------------------------------------------------------------------------------

static inline int DumpAllowsRecordLabel(NodeDumpParams_t* params)
{
    return ((params->shape != NULL) && (strcmp(params->shape, "record" ) == 0
                                    ||  strcmp(params->shape, "Mrecord") == 0));
}

//——————————————————————————————————————————————————————————————————————————————————————————

static void PrintArg(const char* arg_name,
                     const char* arg_value,
                     bool*       is_first_arg,
                     FILE*       fp);

//——————————————————————————————————————————————————————————————————————————————————————————

static void DumpNode(NodeDumpParams_t* params, FILE* fp)
{
    assert(params != NULL);
    assert(fp     != NULL);

    fprintf(fp, "\t%s", params->name);

    bool is_first_arg = true;

    PrintArg("label",     params->label,     &is_first_arg, fp);
    PrintArg("color",     params->color,     &is_first_arg, fp);
    PrintArg("fillcolor", params->fillcolor, &is_first_arg, fp);
    PrintArg("fontcolor", params->fontcolor, &is_first_arg, fp);
    PrintArg("shape",     params->shape,     &is_first_arg, fp);

    if (!is_first_arg)
    {
        fprintf(fp, "]");
    }

    fprintf(fp, ";\n");
}

//------------------------------------------------------------------------------------------

static void DumpEdge(EdgeDumpParams_t* params, FILE* fp)
{
    assert(params != NULL);
    assert(fp     != NULL);

    fprintf(fp, "\t%s->%s", params->node1, params->node2);

    bool is_first_arg = true;

    PrintArg("color",      params->color,      &is_first_arg, fp);
    PrintArg("constraint", params->constraint, &is_first_arg, fp);
    PrintArg("dir",        params->dir,        &is_first_arg, fp);
    PrintArg("style",      params->style,      &is_first_arg, fp);
    PrintArg("arrowhead",  params->arrowhead,  &is_first_arg, fp);
    PrintArg("arrowtail",  params->arrowtail,  &is_first_arg, fp);
    PrintArg("label",      params->label,      &is_first_arg, fp);

    if (!is_first_arg)
    {
        fprintf(fp, "]");
    }

    fprintf(fp, ";\n");
}

//------------------------------------------------------------------------------------------

static void PrintArg(const char* arg_name,
                     const char* arg_value,
                     bool*       is_first_arg,
                     FILE*       fp)
{
    assert(is_first_arg != NULL);
    assert(arg_name     != NULL);
    assert(fp           != NULL);

    if (arg_value == NULL)
        return;

    if (*is_first_arg)
    {
        fprintf(fp, " [");
        *is_first_arg = false;
    }
    else
    {
        fprintf(fp, ", ");
    }

    fprintf(fp, "%s = \"%s\"", arg_name, arg_value);
}

//------------------------------------------------------------------------------------------
