#include "tree_commands.h"
#include "lang_funcs.h"
#include "lexer.h"
#include "data_read.h"
#include <wchar.h>
#include <locale.h>
// #include "parser.h"

//------------------------------------------------------------------------------------------

int main()
{
    setlocale(LC_ALL, "en_US.utf8");

    fwprintf(stderr, L"code = \n");

    LangCtx_t lang_ctx = {};

    if (LangCtxCtor(&lang_ctx))
        return EXIT_FAILURE;
//
//     wchar_t code_buffer[2000] =
// LR"(
// за Факториал отомсти н
//
//     за что? за что? за что?
//
//     пересади в результат 1    ЗАМОЛЧИ
//     пересади в итератор  1    ЗАМОЛЧИ
//
//     снова и снова
//     снова и снова
//     снова и снова
//     снова и снова
//     снова и снова
//
//     н избавить от итератор
//
//     ЗАТКНИСЬ ЗАТКНИСЬ ЗАТКНИСЬ ЗАТКНИСЬ ЗАТКНИСЬ
//
//         пересади в результат результат усилить в итератор ЗАМОЛЧИ
//         пересади в итератор  итератор  нарастить на 1     ЗАМОЛЧИ
//
//     ОНИ СМОТРЯТ ОНИ СМОТРЯТ ОНИ СМОТРЯТ ОНИ СМОТРЯТ
//
//     оставь результат ЗАМОЛЧИ
//
//     нет нет нет нет все кончено?
//
//
// скажи мне кто ты, приятель                                            ЗАМОЛЧИ
// пересади в ответ ритуал: во имя Факториал принеси в жертву приятель   ЗАМОЛЧИ
// заставь их услышать ответ                                             ЗАМОЛЧИ

// )";

    // FILE* fp = fopen("test.txt", "w");
    // if (fp == NULL)
    //     printf("open file fail\n");
    // fwprintf(fp, L"%ls", code_buffer);
    // fclose(fp);

    TreeReadInputData(&lang_ctx);

    // lang_ctx.code   = wcsdup(code_buffer);
    // lang_ctx.buffer = lang_ctx.code;
    wprintf(L"code = %ls\n", lang_ctx.code);

    LexicallyAnalyze(&lang_ctx);

    LangCtxDtor(&lang_ctx);

    return EXIT_SUCCESS;
}

//------------------------------------------------------------------------------------------
