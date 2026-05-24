## Грамматика в расширенной форме Бэкуса — Наура
```
Program                ::= Body '\0'

Body                   ::= { Statement 'дальше' }*

Statement              ::= IfStatement
                         | WhileStatement
                         | FunctionDeclaration
                         | ReturnStatement
                         | VariableDeclaration
                         | Assignment
                         | Expression

IfStatement            ::= 'в случае' Expression 'свершится'
                         BlockStatement
                         ElseStatement?

ElseStatement          ::= 'иначе'
                         BlockStatement

WhileStatement         ::=  'снова и снова'
                            'снова и снова'
                            'снова и снова'
                            'снова и снова'
                            'снова и снова'
                            Expression
                            BlockStatement

BlockStatement         ::= 'ВХОД' 'ВХОД' 'ВХОД' 'ВХОД' 'ВХОД'
                            { Statement 'дальше' }*
                           'ВЫХОД' 'ВЫХОД' 'ВЫХОД' 'ВЫХОД'

FunctionDeclaration    ::= 'босс' Identifier 'для призыва нужны:' FunctionParameters?
                            FunctionBlockStatement

FunctionParameters     ::= Identifier { 'и' Identifier }*

FunctionBlockStatement ::= 'ПРИЗЫВ' 'ПРИЗЫВ' 'ПРИЗЫВ'
                            { Statement 'дальше' }*
                           'КОНЕЦ?'

ReturnStatement        ::= 'оставь' Expression

VariableDeclaration    ::= 'моб' Identifier

Assignment             ::= 'пересади в' Identifier Expression

Expression             ::= Comparison

Comparison             ::= Expression
                           { ('такой же как' | 'противоположен' | 'больше' |
                              'меньше' | 'больше-равен' | 'меньше-равен')
                             Expression }*

Expression             ::= Term { ('нарастить на' | 'избавить от')  Term }*

Term                   ::= Power { ('усилить в' | 'расщепить на')    Power }*

Power                  ::= Factor { 'расплодить в'                   Factor }*

Factor                 ::= '(' Expression ')'
                         | UnaryOperatorCall
                         | FunctionCall
                         | BuiltinCall
                         | Number
                         | Identifier

UnaryOperatorCall      ::= UnaryOperator Expression

UnaryOperator          ::= 'заставь их услышать' | 'скажи мне кто ты,'

FunctionCall           ::= 'вызови' Identifier 'с жертвой' FunctionArguments?

FunctionArguments      ::= Expression { 'и' Expression }*

BuiltinCall            ::= BuiltinFunctionName [ Expression { 'и' Expression }* ]

BuiltinFunctionName    ::= 'исток'
                         | 'рисуй'
                         | 'поставь точку'
                         | 'инициализируй экран'
                         | 'удали экран'
                         | 'нарисуй экран'
                         | 'поставь на экран'
                         /* и другие библиотечные функции */

Identifier             ::= ['A'-'Z''a'-'z''А'-'Я''а'-'я''_']
                           ['A'-'Z''a'-'z''А'-'Я''а'-'я''_''0'-'9']+

Number                 ::= [-]*['0'-'9']+

```