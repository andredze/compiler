#include "parser.h"

/* ==================== Domain Specific Language for reading data ========================== */

// lnode means left_node
// rnode means right_node

/* if operation has 1 argument, it should be placed in right node */

#define ISVALUE_(node, number) (node->data.type == TYPE_NUM && \
                                CompareDoubles(node->data.value.num, (number)) == 0)

#define NUM_(number)       MathNodeCtor(math_ctx, {TYPE_NUM, { .num = (number) }}, NULL, NULL)

#define ADD_(lnode, rnode) MathNodeCtor(math_ctx, {TYPE_OP,  { .op  = OP_ADD }}, (lnode), (rnode))
#define SUB_(lnode, rnode) MathNodeCtor(math_ctx, {TYPE_OP,  { .op  = OP_SUB }}, (lnode), (rnode))
#define MUL_(lnode, rnode) MathNodeCtor(math_ctx, {TYPE_OP,  { .op  = OP_MUL }}, (lnode), (rnode))
#define DIV_(lnode, rnode) MathNodeCtor(math_ctx, {TYPE_OP,  { .op  = OP_DIV }}, (lnode), (rnode))
#define DEG_(lnode, rnode) MathNodeCtor(math_ctx, {TYPE_OP,  { .op  = OP_DEG }}, (lnode), (rnode))
#define UNARY_(oper, node) MathNodeCtor(math_ctx, {TYPE_OP,  { .op  = (oper) }},  NULL,   (node) )

/* ====================================================================================== */

#ifdef TREE_DEBUG
    #define TREE_READ_BUFFER_DUMP(expr, fmt, ...)                                                      \
            BEGIN                                                                                      \
            TreeReadBufferDump((expr->buffer), (expr->cur_p - expr->buffer), (fmt), ##__VA_ARGS__);    \
            END
#else
    #define TREE_READ_BUFFER_DUMP(expr, fmt, ...) ;

#endif

//------------------------------------------------------------------------------------------

static TreeNode_t* ParseGrammar     (Tree_t* tree, Expr_t* expr);
static TreeNode_t* ParseExpr        (Tree_t* tree, Expr_t* expr);
static TreeNode_t* ParseTerm        (Tree_t* tree, Expr_t* expr);
static TreeNode_t* ParseDegree      (Tree_t* tree, Expr_t* expr);
static TreeNode_t* ParseElement     (Tree_t* tree, Expr_t* expr);
static TreeNode_t* ParseInBrackets  (Tree_t* tree, Expr_t* expr);
static TreeNode_t* ParseNumber      (Tree_t* tree, Expr_t* expr);
static int         ParseFractalPart (Expr_t*    expr,     double* value);
static TreeNode_t* ParseIdentifier  (Tree_t* tree, Expr_t* expr);

static TreeNode_t* ParseUnaryOp     (Tree_t* tree,     Expr_t*     expr,
                                     char*      word_start_p, size_t      word_len);

static TreeNode_t* ParseVariable    (Tree_t* tree,     Expr_t*     expr,
                                     char*      word_start_p, size_t      word_len);

static MathErr_t   PutVarInTable    (Tree_t* tree,     char*       str,
                                     size_t     str_len,      MathData_t* data);

static void SkipSpaces              (Expr_t* expr);
static int  IsAcceptableStartSymbol (char ch);
static int  IsAcceptableSymbol      (char ch);

static void PrintSyntaxError        (Expr_t* expr,     const char* file,
                                     const char* func, const int line,
                                     const char* fmt, ...);

//——————————————————————————————————————————————————————————————————————————————————————————

#define SYNTAX_ERROR(math_ctx, expr, fmt, ...)                                                      \
        {   TreeDumpInfo_t dump_info = {TREE_SUCCESS, __PRETTY_FUNCTION__, __FILE__, __LINE__};     \
            if (TreeDump((math_ctx), &dump_info, (fmt), ##__VA_ARGS__))                             \
            {                                                                                       \
                return NULL;                                                                        \
            }                                                                                       \
            PrintSyntaxError(expr, __FILE__, __PRETTY_FUNCTION__, __LINE__, (fmt), ##__VA_ARGS__);  \
            return NULL; }

//------------------------------------------------------------------------------------------

MathErr_t MathParseText(Tree_t* tree, Expr_t* expr)
{
    assert(expr != NULL);

    DEBUG_TREE_CHECK(math_ctx, "START OF PARSE TEXT");

    TreeNode_t* root = ParseGrammar(math_ctx, expr);

    if (root == NULL)
        return MATH_FILE_ERROR;

    math_ctx->tree.dummy->right = root;
    root->parent = math_ctx->tree.dummy;

    TREE_CALL_DUMP(math_ctx, "PARSED TEXT");
    MathTexSection("Исходное выражение");
    MathCtxTexDump(math_ctx, NULL);

    return MATH_SUCCESS;
}

//------------------------------------------------------------------------------------------

static void SkipSpaces(Expr_t* expr)
{
    assert(expr != NULL);

    char ch = '\0';

    while ((ch = *expr->cur_p) != '\0' && isspace(ch))
    {
        expr->cur_p++;
    }
}

//------------------------------------------------------------------------------------------

static TreeNode_t* ParseGrammar(Tree_t* tree, Expr_t* expr)
{
    TreeNode_t* node = ParseExpr(math_ctx, expr);

    if (node == NULL)
        return NULL;

    SkipSpaces(expr);

    if (*expr->cur_p != END_SYMBOL)
        SYNTAX_ERROR(math_ctx, expr, "Unknown symbol at end");

    expr->cur_p++;

    return node;
}

//------------------------------------------------------------------------------------------

static TreeNode_t* ParseExpr(Tree_t* tree, Expr_t* expr)
{
    TreeNode_t* node1 = ParseTerm(math_ctx, expr);

    if (node1 == NULL)
        return NULL;

    SkipSpaces(expr);

    while (*expr->cur_p == '+' || *expr->cur_p == '-')
    {
        int operation = *expr->cur_p;
        expr->cur_p++;

        TREE_READ_BUFFER_DUMP(expr, "GET E: READ \"+ | -\"");

        TreeNode_t* node2 = ParseTerm(math_ctx, expr);

        if (node2 == NULL)
            return NULL;

        if (operation == '+')
            node1 = ADD_(node1, node2);
        else
            node1 = SUB_(node1, node2);

        SkipSpaces(expr);
    }

    return node1;
}

//------------------------------------------------------------------------------------------

static TreeNode_t* ParseTerm(Tree_t* tree, Expr_t* expr)
{
    TreeNode_t* node1 = ParseDegree(math_ctx, expr);

    if (node1 == NULL)
        return NULL;

    SkipSpaces(expr);

    while (*expr->cur_p == '*' || *expr->cur_p == '/')
    {
        int operation = *expr->cur_p;
        expr->cur_p++;
        TREE_READ_BUFFER_DUMP(expr, "GET T: READ \"* | /\"");

        TreeNode_t* node2 = ParseDegree(math_ctx, expr);

        if (node2 == NULL)
            return NULL;

        if (operation == '*')
            node1 = MUL_(node1, node2);
        else
        {
            if (ISVALUE_(node2, 0.0))
                SYNTAX_ERROR(math_ctx, expr, "Division by zero");

            node1 = DIV_(node1, node2);
        }

        SkipSpaces(expr);
    }

    return node1;
}

//------------------------------------------------------------------------------------------

static TreeNode_t* ParseDegree(Tree_t* tree, Expr_t* expr)
{
    TreeNode_t* node1 = ParseElement(math_ctx, expr);

    if (node1 == NULL)
        return NULL;

    SkipSpaces(expr);

    while (*expr->cur_p == '^')
    {
        expr->cur_p++;

        TREE_READ_BUFFER_DUMP(expr, "GET POWER: READ \"^\"");

        node1 = DEG_(node1, NULL);

        TreeNode_t* node2 = ParseElement(math_ctx, expr);

        if (node2 == NULL)
            return NULL;

        node1->right  = node2;
        node2->parent = node1;

        SkipSpaces(expr);
    }

    return node1;
}

//------------------------------------------------------------------------------------------

static TreeNode_t* ParseElement(Tree_t* tree, Expr_t* expr)
{
    TreeNode_t* node = 0;

    SkipSpaces(expr);

    if (*expr->cur_p == '(')
        node = ParseInBrackets(math_ctx, expr);
    else if (isdigit(*expr->cur_p))
        node = ParseNumber(math_ctx, expr);
    else
        node = ParseIdentifier(math_ctx, expr);

    return node;
}

//------------------------------------------------------------------------------------------

static TreeNode_t* ParseInBrackets(Tree_t* tree, Expr_t* expr)
{
    expr->cur_p++;
    TREE_READ_BUFFER_DUMP(expr, "GET P: READ \"(\"");

    TreeNode_t* node = ParseExpr(math_ctx, expr);

    if (node == NULL)
        return NULL;

    SkipSpaces(expr);

    if (*expr->cur_p != ')')
        SYNTAX_ERROR(math_ctx, expr, "no matching bracket");

    expr->cur_p++;
    TREE_READ_BUFFER_DUMP(expr, "GET P: READ \")\"");

    return node;
}

//------------------------------------------------------------------------------------------

static TreeNode_t* ParseNumber(Tree_t* tree, Expr_t* expr)
{
    DPRINTF("\tGetN\n");

    SkipSpaces(expr);

    const char* start = expr->cur_p;
    double      value = 0;

    if (!('0' <= *expr->cur_p && *expr->cur_p <= '9'))
    {
        SYNTAX_ERROR(math_ctx, expr, "Unknown symbol: not a number");
    }

    while ('0' <= *expr->cur_p && *expr->cur_p <= '9')
    {
        value = value * 10 + (*expr->cur_p - '0');
        expr->cur_p++;
    }

    if (start == expr->cur_p)
        SYNTAX_ERROR(math_ctx, expr, "Unknown symbol: not a number");

    if (ParseFractalPart(expr, &value) == -1)
        SYNTAX_ERROR(math_ctx, expr, "no numbers after point");

    TreeNode_t* node = NUM_(value);

    TREE_READ_BUFFER_DUMP(expr, "GET N: READ number \"%lg\"", value);

    return node;
}

//------------------------------------------------------------------------------------------

static int ParseFractalPart(Expr_t* expr, double* value)
{
    assert(expr != NULL);

    if ('.' == *expr->cur_p)
    {
        char*  float_start = ++expr->cur_p;
        double coeff       = 0.1;

        while ('0' <= *expr->cur_p && *expr->cur_p <= '9')
        {
            *value += coeff * (*expr->cur_p - '0');
            coeff /= 10;
            expr->cur_p++;
        }

        if (float_start == expr->cur_p)
            return -1;
    }

    return 0;
}

//------------------------------------------------------------------------------------------

static TreeNode_t* ParseElement(Tree_t* tree, Expr_t* expr)
{
    DPRINTF("\tGetString:\n");
    SkipSpaces(expr);

    if (!IsAcceptableStartSymbol(*expr->cur_p))
        SYNTAX_ERROR(math_ctx, expr, "unacceptable symbol at start of variable | unary operation");

    char* word_start_p = expr->cur_p;
    DPRINTF("\t\texpr->cur_p = %p;\n", expr->cur_p);

    while (IsAcceptableSymbol(*expr->cur_p))
    {
        DPRINTF("\t\tchar = %c; is_alpha = %d;\n", *expr->cur_p, IsAcceptableSymbol(*expr->cur_p));
        expr->cur_p++;
        DPRINTF("\t\texpr->cur_p = %p;\n", expr->cur_p);
    }

    size_t word_len = (size_t) (expr->cur_p - word_start_p);

    DPRINTF("word_start_p = %s; word_len = %zu;\n", word_start_p, word_len);

    TreeNode_t* node = ParseUnaryOp(math_ctx, expr, word_start_p, word_len);

    DPRINTF("after GetU node = %p;\n", node);

    if (node != NULL)
        return node;

    node = ParseVariable(math_ctx, expr, word_start_p, word_len);

    return node;
}

//------------------------------------------------------------------------------------------

static TreeNode_t* ParseUnaryOp(Tree_t* tree, Expr_t* expr, char* word_start_p, size_t word_len)
{
//     uint64_t hash = GetHash();
//
//     op_ptr = bsearch();
//
//     if (op_ptr == NULL)
//         return NULL;

    MathOp_t op_code = OP_UNKNOWN;

    for (size_t i = 0; i < OP_CASES_TABLE_SIZE; i++)
    {
        if (OP_CASES_TABLE[i].args_count != 1)
            continue;

        if (strncmp(word_start_p, OP_CASES_TABLE[i].str, word_len) == 0)
            op_code = OP_CASES_TABLE[i].code;
    }

    if (op_code == OP_UNKNOWN)
        return NULL;

    TREE_READ_BUFFER_DUMP(expr, "GET V: READ operation \"OP = %s (code = %d)\"",
                                OP_CASES_TABLE[op_code].str, op_code);

    if (*expr->cur_p != '(')
        SYNTAX_ERROR(math_ctx, expr, "no opening bracket after function");

    expr->cur_p++;

    TreeNode_t* node2 = ParseExpr(math_ctx, expr);

    if (*expr->cur_p != ')')
        SYNTAX_ERROR(math_ctx, expr, "no closing bracket after function");

    expr->cur_p++;

    return UNARY_(op_code, node2);
}

//------------------------------------------------------------------------------------------

static int IsAcceptableStartSymbol(char ch)
{
    return isalpha(ch) || ch == '_';
}

//------------------------------------------------------------------------------------------

static int IsAcceptableSymbol(char ch)
{
    return isalpha(ch) || isdigit(ch) || ch == '_';
}

//------------------------------------------------------------------------------------------

static TreeNode_t* ParseVariable(Tree_t* tree, Expr_t* expr, char* word_start_p, size_t word_len)
{
    TreeNode_t* node = MathNodeCtor(math_ctx, {.type = TYPE_VAR}, NULL, NULL);

    if (node == NULL)
        return NULL;

    if (PutVarInTable(math_ctx, word_start_p, word_len, &node->data))
        return NULL;

    TREE_READ_BUFFER_DUMP(expr, "GET V: READ variable \"vars[%d] = %s\"",
                                node->data.value.var,
                                math_ctx->vars.data[node->data.value.var].str);

    return node;
}

//------------------------------------------------------------------------------------------

static MathErr_t PutVarInTable(Tree_t* tree, char* str, size_t str_len, MathData_t* data)
{
    assert(math_ctx != NULL);
    assert(data     != NULL);
    assert(str      != NULL);

    MathErr_t error = MATH_SUCCESS;

    if (math_ctx->vars.size >= math_ctx->vars.capacity)
    {
        if ((error = MathVarsTableRealloc(math_ctx)))
            return error;
    }

    for (size_t i = 0; i < math_ctx->vars.size; i++)
    {
        if (strncmp(math_ctx->vars.data[i].str, str, str_len) == 0)
            return MATH_SUCCESS;
    }

    char* var_str = strndup(str, str_len);

    if (var_str == NULL)
    {
        PRINTERR("Memory allocation failed");
        return MATH_ALLOC_ERROR;
    }

    math_ctx->vars.data[math_ctx->vars.size].str = var_str;

    data->value.var = math_ctx->vars.size;

    math_ctx->vars.size++;

    MATH_VARS_DUMP(math_ctx, "DUMP AFTER PUTTING VARIABLE %s (index = %zu)",
                   var_str,
                   math_ctx->vars.size - 1);

    return MATH_SUCCESS;
}

//==========================================================================================

#undef ISVALUE_

#undef NUM_

#undef ADD_
#undef SUB_
#undef MUL_
#undef DIV_
#undef DEG_
#undef UNARY_

//==========================================================================================
