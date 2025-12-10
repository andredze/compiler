#ifdef _DSL_DEFINE_

//——————————————————————————————————————————————————————————————————————————————————————————

#include "tree_types.h"
#include "tree_commands.h"

//------------------------------------------------------------------------------------------

#define OPERATOR_(op_code)    LangOperatorNodeCtor      (lang_ctx, (op_code ))
#define IDENTIFIER_(id_index) LangIdentifierNodeCtor    (lang_ctx, (id_index))
#define NUMBER_(number)       LangNumberNodeCtor        (lang_ctx, (number  ))

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
