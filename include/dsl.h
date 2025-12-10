#ifdef _DSL_DEFINE_

//------------------------------------------------------------------------------------------

#define OPERATOR_(op_code)   TreeNodeCtor(lang_ctx->tree, { TYPE_OP , {.op  = (op_code) } }, NULL, NULL)
#define IDENTIFIER_(var_ind) TreeNodeCtor(lang_ctx->tree, { TYPE_ID , {.var = (var_ind) } }, NULL, NULL)
#define NUMBER_(number)      TreeNodeCtor(lang_ctx->tree, { TYPE_NUM, {.num = (number)  } }, NULL, NULL)

//------------------------------------------------------------------------------------------

#undef    _DSL_DEFINE_
#endif /* _DSL_DEFINE_ */

//——————————————————————————————————————————————————————————————————————————————————————————

#ifdef _DSL_UNDEF_

//------------------------------------------------------------------------------------------

#undef OPERATOR_
#undef IDENTIFIER_
#undef NUMBER_

//------------------------------------------------------------------------------------------

#undef    _DSL_UNDEF_
#endif /* _DSL_UNDEF_*/
