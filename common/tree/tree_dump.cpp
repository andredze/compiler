#include "tree_debug.h"

//——————————————————————————————————————————————————————————————————————————————————————————

static TreeDebugData_t debug = {};

//------------------------------------------------------------------------------------------

static TreeErr_t TreeDumpSetDebugFilePaths  ();
static TreeErr_t TreeDumpSetDirs            ();
static TreeErr_t TreeDumpSetLogFilePath     ();
static TreeErr_t TreeDumpMakeDirs           ();
static void      TreeDumpSetTime            ();

//——————————————————————————————————————————————————————————————————————————————————————————

static TreeErr_t TreeDumpSetDebugFilePaths()
{
    TreeDumpSetTime();

    if (TreeDumpSetDirs())
        return TREE_FILE_ERROR;

    // DPRINTF("> creating TREE LOGS directories:\n"
    //         "\tlog_dir: %s\n"
    //         "\timg_dir: %s\n"
    //         "\tdot_dir: %s\n",
    //         debug.log_dir,
    //         debug.img_dir,
    //         debug.dot_dir);

    if (TreeDumpMakeDirs())
        return TREE_FILE_ERROR;

    // DPRINTF("> TREE LOGS directories successfully created!\n");

    if (TreeDumpSetLogFilePath())
        return TREE_FILE_ERROR;

    DPRINTF("> TREE LOG file path: \"%s\"", debug.log_file_path);

    return TREE_SUCCESS;
}

//------------------------------------------------------------------------------------------

static void TreeDumpSetTime()
{
    time_t rawtime = time(NULL);

    struct tm* info = localtime(&rawtime);

    strftime(debug.str_time, sizeof(debug.str_time), "[%Y-%m-%d_%H%M%S]", info);
}

//------------------------------------------------------------------------------------------

static TreeErr_t TreeDumpMakeDirs()
{
    mkdir(debug.log_dir, 0777);
    mkdir(debug.img_dir, 0777);
    mkdir(debug.dot_dir, 0777);

    return TREE_SUCCESS;
}

//------------------------------------------------------------------------------------------

static TreeErr_t TreeDumpSetDirs()
{
    if (snprintf(debug.log_dir, sizeof(debug.log_dir), "log/%s",
                 debug.str_time) < 0)
    {
        PRINTERR("Error with setting \"log_dir\"");
        return TREE_FILE_ERROR;
    }
    if (snprintf(debug.img_dir, sizeof(debug.img_dir), "%s/%s",
                 debug.log_dir, IMAGE_FILE_TYPE) < 0)
    {
        PRINTERR("Error with setting \"img_dir\"");
        return TREE_FILE_ERROR;
    }
    if (snprintf(debug.dot_dir, sizeof(debug.dot_dir), "%s/dot",
                 debug.log_dir) < 0)
    {
        PRINTERR("Error with setting \"dot_dir\"");
        return TREE_FILE_ERROR;
    }

    return TREE_SUCCESS;
}

//------------------------------------------------------------------------------------------

static TreeErr_t TreeDumpSetLogFilePath()
{
    if (snprintf(debug.log_file_path, sizeof(debug.log_file_path),
                 "%s/tree.html", debug.log_dir) < 0)
    {
        PRINTERR("Error with setting log_file_path");
        return TREE_FILE_ERROR;
    }

    return TREE_SUCCESS;
}

//------------------------------------------------------------------------------------------

TreeErr_t TreeDump(LangCtx_t*            lang_ctx,
                   const TreeDumpInfo_t* dump_info,
                   const char* fmt, ...)
{
    va_list args = {};
    va_start(args, fmt);

    TreeErr_t result = vTreeDump(lang_ctx, dump_info, fmt, args);

    va_end(args);

    return result;
}

//------------------------------------------------------------------------------------------

MathErr_t MathVarsTableDump(const Tree_t* tree, const char* fmt, ...)
{
    assert(fmt != NULL);

    va_list args = {};
    va_start(args, fmt);

    FILE* fp = debug.fp;

    fprintf(fp, "<pre><h4><font color=blue>");

    vfprintf(fp, fmt, args);

    fprintf(fp, "</h4></font>");

    fprintf(fp, "vars_table [%p]:\n\n"
                "size     = %zu\n"
                "capacity = %zu\n\n",
                math_ctx->vars.data,
                math_ctx->vars.size,
                math_ctx->vars.capacity);

    fprintf(fp, "index: ");

    for (size_t i = 0; i < math_ctx->vars.capacity; i++)
    {
        fprintf(fp, "%12zu |", i);
    }

    fprintf(fp, "\nnames: ");

    for (size_t i = 0; i < math_ctx->vars.capacity; i++)
    {
        fprintf(fp, "%12s |", math_ctx->vars.data[i].str);
    }

    va_end(args);

    fflush(fp);

    return MATH_SUCCESS;
}

//------------------------------------------------------------------------------------------

TreeErr_t TreeReadBufferDump(const char* buffer, ssize_t pos, const char* fmt, ...)
{
    assert(fmt != NULL);

    va_list args = {};
    va_start(args, fmt);

    FILE* fp = debug.fp;

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

TreeErr_t vTreeDump(const Tree_t*         tree,
                    const TreeDumpInfo_t* dump_info,
                    const char* fmt, va_list args)
{
    assert(dump_info != NULL);
    assert(tree      != NULL);

    Tree_t* tree = &math_ctx->tree;
    FILE*   fp   = debug.fp;

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

    if ((graph_error = TreeGraphDump(math_ctx)))
    {
        fflush(fp);
        return graph_error;
    }

    int image_width = tree->size <= 5 ? 25 : 50;

    fprintf(fp, "\n<img src = svg/%s.svg width = %d%%>\n\n"
                "============================================================="
                "=============================================================\n\n",
                 debug.graph_file_name, image_width);

    fflush(fp);

    return TREE_SUCCESS;
}

//------------------------------------------------------------------------------------------

TreeErr_t TreeOpenLogFile()
{
    if (TreeDumpSetDebugFilePaths())
        return TREE_FILE_ERROR;

    debug.fp = fopen(debug.log_file_path, "w");

    if (debug.fp == NULL)
    {
        PRINTERR("Opening logfile %s failed", debug.log_file_path);
        return TREE_FILE_ERROR;
    }

    return TREE_SUCCESS;
}

//------------------------------------------------------------------------------------------

void TreeCloseLogFile()
{
    fclose(debug.fp);
    debug.fp = NULL;
}

//------------------------------------------------------------------------------------------

TreeErr_t TreeGraphDumpSubtree(Tree_t* tree, TreeNode_t* node)
{
    assert(math_ctx != NULL);

    Tree_t*   tree  = &math_ctx->tree;
    TreeErr_t error = TREE_SUCCESS;

    if (tree == NULL)
    {
        PRINTERR("TREE_NULL");
        return    TREE_NULL;
    }

    SetGraphFilepaths();

    FILE* dot_file = fopen(debug.dot_file_path, "w");

    debug.graphs_count++;

    if (dot_file == NULL)
    {
        PRINTERR("Failed opening logfile");
        return TREE_DUMP_ERROR;
    }

    DumpGraphTitle(dot_file);

    if (DumpTreeEdges(node, dot_file))
        return TREE_DUMP_ERROR;

    if ((error = TreeNodeDump(node, dot_file, math_ctx)))
        return error;

    fprintf(dot_file, "}\n");

    fclose(dot_file);

    if ((error = TreeConvertGraphFile()))
        return error;

    int image_width = tree->size <= 5 ? 25 : 50;

    fprintf(debug.fp, "\n<img src = svg/%s.svg width = %d%%>\n\n"
                      "============================================================="
                      "=============================================================\n\n",
                       debug.graph_file_name, image_width);

    return TREE_SUCCESS;
}

//------------------------------------------------------------------------------------------

TreeErr_t TreeGraphDump(Tree_t* tree)
{
    assert(math_ctx != NULL);

    Tree_t*   tree  = &math_ctx->tree;
    TreeErr_t error = TREE_SUCCESS;

    if (tree == NULL)
    {
        PRINTERR("TREE_NULL");
        return    TREE_NULL;
    }

    SetGraphFilepaths();

    FILE* dot_file = fopen(debug.dot_file_path, "w");

    debug.graphs_count++;

    if (dot_file == NULL)
    {
        PRINTERR("Failed opening logfile");
        return TREE_DUMP_ERROR;
    }

    DumpGraphTitle(dot_file);

    if (DumpTreeDefaultNode(tree->dummy, "type = PZN | value = PZN", "#3E3A22", "#ecede8", "#3E3A22", "record", dot_file))
        return TREE_DUMP_ERROR;

    if (DumpTreeEdges(tree->dummy, dot_file))
        return TREE_DUMP_ERROR;

    if (tree->dummy->right != NULL)
    {
        if ((error = TreeNodeDump(tree->dummy->right, dot_file, math_ctx)))
            return error;
    }

    fprintf(dot_file, "}\n");

    fclose(dot_file);

    if ((error = TreeConvertGraphFile()))
        return error;

    return TREE_SUCCESS;
}

//------------------------------------------------------------------------------------------

void SetGraphFilepaths()
{
    snprintf(debug.graph_file_name,
             sizeof(debug.graph_file_name),
             "graph_%04d",
             debug.graphs_count);
    // DPRINTF("debug.graph_file_name = %s;\n", debug.graph_file_name);

    snprintf(debug.dot_file_path,
             sizeof(debug.dot_file_path),
             "%s/%s.dot",
             debug.dot_dir,
             debug.graph_file_name);
    // DPRINTF("debug.dot_file_path = %s;\n", debug.dot_file_path);

    snprintf(debug.img_file_path,
             sizeof(debug.img_file_path),
             "%s/%s.%s",
             debug.img_dir,
             debug.graph_file_name,
             IMAGE_FILE_TYPE);
    // DPRINTF("debug.img_file_path = %s;\n", debug.img_file_path);
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

TreeErr_t TreeConvertGraphFile()
{
    char command[MAX_COMMAND_LEN] = {};

    snprintf(command, sizeof(command), "dot %s -T %s -o %s",
                                       debug.dot_file_path,
                                       IMAGE_FILE_TYPE,
                                       debug.img_file_path);

    int result = system(command);

    if (result == 0)
    {
        // DPRINTF("Generated graph dump: %s\n", debug.img_file_path);
    }
    else
    {
        PRINTERR("TREE_SYSTEM_FUNC_ERROR");
        return    TREE_SYSTEM_FUNC_ERROR;
    }

    return TREE_SUCCESS;
}

//------------------------------------------------------------------------------------------

TreeErr_t TreeNodeDump(const TreeNode_t* node, FILE* fp, Tree_t* tree)
{
    assert(node != NULL);
    assert(fp   != NULL);

    TreeErr_t error = TREE_SUCCESS;

    if (node->left != NULL)
    {
        if ((error = TreeNodeDump(node->left, fp, math_ctx)))
            return error;
    }

    if ((error = DumpTreeNodeAndEdges(node, fp, math_ctx)))
        return error;

    if (node->right != NULL)
    {
        if ((error = TreeNodeDump(node->right, fp, math_ctx)))
            return error;
    }

    return TREE_SUCCESS;
}

//------------------------------------------------------------------------------------------

void TreeNodePrint(const TreeNode_t* node)
{
    assert(node != NULL);

    printf("(");

    if (node->left != NULL)
    {
        TreeNodePrint(node->left);
    }

    printf(" ");
    TreePrintElement(&node->data);
    printf(" ");

    if (node->right != NULL)
    {
        TreeNodePrint(node->right);
    }

    printf(")");
}

//------------------------------------------------------------------------------------------

TreeErr_t DumpTreeNodeAndEdges(const TreeNode_t* node, FILE* fp, Tree_t* tree)
{
    if (DumpTreeSingleNode(node, fp, math_ctx))
    {
        return TREE_DUMP_ERROR;
    }
    if (DumpTreeEdges(node, fp))
    {
        return TREE_DUMP_ERROR;
    }

    return TREE_SUCCESS;
}

//------------------------------------------------------------------------------------------

int DumpTreeEdges(const TreeNode_t* node, FILE* fp)
{
    assert(fp   != NULL);
    assert(node != NULL);

    if (node->left  == NULL &&
        node->right == NULL)
    {
        return 0;
    }

    char name_node [MAX_NODE_NAME_LEN] = "";
    sprintf (name_node , "node%p", node       );

    if (node->left != NULL)
    {
        char name_left [MAX_NODE_NAME_LEN] = "";
        sprintf (name_left , "node%p", node->left );
        DumpEdge(name_node, name_left , "#404040", NULL, NULL, NULL, NULL, NULL, NULL, fp);
    }
    if (node->right != NULL)
    {
        char name_right[MAX_NODE_NAME_LEN] = "";
        sprintf (name_right, "node%p", node->right);
        DumpEdge(name_node, name_right, "#404040", NULL, NULL, NULL, NULL, NULL, NULL, fp);
    }

    return 0;
}

//------------------------------------------------------------------------------------------

int DumpTreeSingleNode(const TreeNode_t* node, FILE* fp, Tree_t* tree)
{
    assert(node != NULL);
    assert(fp   != NULL);

    switch (node->data.type)
    {
        case TYPE_OP:
            DumpTreeOpNode(node, fp);
            break;

        case TYPE_NUM:
            DumpTreeNumNode(node, fp);
            break;

        case TYPE_VAR:
            DumpTreeVarNode(node, fp, math_ctx);
            break;

        default:
            return 1;
    }

    return 0;
}

//------------------------------------------------------------------------------------------

int DumpTreeOpNode(const TreeNode_t* node, FILE* fp)
{
    assert(node != NULL);
    assert(fp   != NULL);

    char str_data[MAX_DATA_LEN] = {};

    snprintf(str_data, sizeof(str_data), "type = %s | value = %s",
                                         TYPE_CASES_TABLE[node->data.type].name,
                                         OP_CASES_TABLE[node->data.value.op].str);

    return DumpTreeDefaultNode(node, str_data,
                               OP_CASES_TABLE[node->data.value.op].color,
                               OP_CASES_TABLE[node->data.value.op].fillcolor,
                               OP_CASES_TABLE[node->data.value.op].fontcolor,
                               TYPE_CASES_TABLE[node->data.type].shape,
                               fp);
}

//------------------------------------------------------------------------------------------

int DumpTreeVarNode(const TreeNode_t* node, FILE* fp, Tree_t* tree)
{
    assert(math_ctx != NULL);
    assert(node     != NULL);
    assert(fp       != NULL);

    char str_data[MAX_DATA_LEN] = {};

    snprintf(str_data, sizeof(str_data), "type = %s | value = %s (%zu)",
                                         TYPE_CASES_TABLE[node->data.type].name,
                                         math_ctx->vars.data[node->data.value.var].str,
                                         node->data.value.var);

    return DumpTreeDefaultNode(node, str_data,
                               TYPE_CASES_TABLE[node->data.type].color,
                               TYPE_CASES_TABLE[node->data.type].fillcolor,
                               TYPE_CASES_TABLE[node->data.type].fontcolor,
                               TYPE_CASES_TABLE[node->data.type].shape,
                               fp);
}

//------------------------------------------------------------------------------------------

int DumpTreeNumNode(const TreeNode_t* node, FILE* fp)
{
    assert(node != NULL);
    assert(fp   != NULL);

    char str_data[MAX_DATA_LEN] = {};

    snprintf(str_data, sizeof(str_data), "type = %s | value = %lg",
                                         TYPE_CASES_TABLE[node->data.type].name,
                                         node->data.value.num);

    return DumpTreeDefaultNode(node, str_data,
                               TYPE_CASES_TABLE[node->data.type].color,
                               TYPE_CASES_TABLE[node->data.type].fillcolor,
                               TYPE_CASES_TABLE[node->data.type].fontcolor,
                               TYPE_CASES_TABLE[node->data.type].shape,
                               fp);
}

//------------------------------------------------------------------------------------------

int DumpTreeDefaultNode(const TreeNode_t* node,
                        const char* str_data,
                        const char* color,
                        const char* fillcolor,
                        const char* fontcolor,
                        const char* shape,
                        FILE* fp)
{
    assert(fp   != NULL);
    assert(node != NULL);

    char name[MAX_NODE_NAME_LEN] = "";

    sprintf(name, "node%p", node);

    char label[MAX_LABEL_LEN] = "";

    snprintf(label, MAX_LABEL_LEN, "{ %p | %s | parent = %p | { left = %p | right = %p }}",
                                   node, str_data, node->parent, node->left, node->right);

    DumpNode(name, label, color, fillcolor, fontcolor, shape, fp);

    return 0;
}

//------------------------------------------------------------------------------------------

int DumpNode(const char* name,
             const char* label,
             const char* color,
             const char* fillcolor,
             const char* fontcolor,
             const char* shape,
             FILE* fp)
{
    assert(name != NULL);
    assert(fp   != NULL);

    fprintf(fp, "\t%s", name);

    int is_first_arg = 1;

    PrintArg("label",     label,     &is_first_arg, fp);
    PrintArg("color",     color,     &is_first_arg, fp);
    PrintArg("fillcolor", fillcolor, &is_first_arg, fp);
    PrintArg("fontcolor", fontcolor, &is_first_arg, fp);
    PrintArg("shape",     shape,     &is_first_arg, fp);

    if (is_first_arg == 0)
    {
        fprintf(fp, "]");
    }

    fprintf(fp, ";\n");

    return 0;
}

//------------------------------------------------------------------------------------------

int DumpEdge(const char* node1,
             const char* node2,
             const char* color,
             const char* constraint,
             const char* dir,
             const char* style,
             const char* arrowhead,
             const char* arrowtail,
             const char* label,
             FILE*       fp)
{
    assert(node1 != NULL);
    assert(node2 != NULL);

    fprintf(fp, "\t%s->%s", node1, node2);

    int is_first_arg = 1;

    PrintArg("color",      color,      &is_first_arg, fp);
    PrintArg("constraint", constraint, &is_first_arg, fp);
    PrintArg("dir",        dir,        &is_first_arg, fp);
    PrintArg("style",      style,      &is_first_arg, fp);
    PrintArg("arrowhead",  arrowhead,  &is_first_arg, fp);
    PrintArg("arrowtail",  arrowtail,  &is_first_arg, fp);
    PrintArg("label",      label,      &is_first_arg, fp);

    if (is_first_arg == 0)
    {
        fprintf(fp, "]");
    }

    fprintf(fp, ";\n");

    return 0;
}

//------------------------------------------------------------------------------------------

int PrintArg(const char* arg_name,
             const char* arg_value,
             int*        is_first_arg,
             FILE*       fp)
{
    assert(arg_name     != NULL);
    assert(is_first_arg != NULL);

    if (arg_value != NULL)
    {
        if (*is_first_arg)
        {
            fprintf(fp, " [");
            *is_first_arg = 0;
        }
        else
        {
            fprintf(fp, ", ");
        }

        fprintf(fp, "%s = \"%s\"", arg_name, arg_value);
    }

    return 0;
}

//------------------------------------------------------------------------------------------
