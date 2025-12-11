# Язык программирования

## Синтаксис



## Грамматика в расширенной форме Бэкуса — Наура
```
Program             ::= (Statement)* '\0'

FunctionStatement   ::= Statement | ReturnStatement

ReturnStatement     ::= 'верни' Expression

Statement           ::= IfStatement
                      | WhileStatement
                      | FunctionDeclaration
                      | Assignment ';'
                      | Expression ';'

FunctionDeclaration ::= 'функция' Identifier '(' FunctionParameters? ')' '{' FunctionStatement* '}'

FunctionParameters  ::= Parameter { ',' Parameter }*
Parameter           ::= 'параметр' Identifier

IfStatement         ::= 'если'  '(' Expression ')' BlockStatement ElseStatement?
WhileStatement      ::= 'пока'  '(' Expression ')' BlockStatement
ElseStatement       ::= 'иначе' BlockStatement

BlockStatement      ::= '{' Statement* '}'

Expression          ::= Term   { ['+''-'] Term  }*
Term                ::= Power  { ['*''/'] Power }*
Power               ::= Factor { '^' Factor }*

Factor              ::= '(' Expression ')'
                      | UnaryOperatorCall
                      | FunctionCall
                      | Number
                      | Identifier
                      | String

UnaryOperatorCall   ::= UnaryOperator '(' Expression ')'
UnaryOperator       ::= 'sin' | 'cos' | 'ln' | 'tg' | 'sqrt' | 'печать' | 'читать'

FunctionCall        ::= Identifier '(' FunctionArguments? ')'

FunctionArguments   ::= Expression { ',' Expression }*

String              ::= '"' [^'"''\n'] '"'
Identifier          ::= ['А'-'Я''а'-'я''_']['А'-'Я''а'-'я''_''0'-'9']+
Number              ::= ['0'-'9']+ | ['0'-'9']+ '.' ['0'-'9']+
```


## Абстрактное Синтаксическое Дерево (AST)


```
{ "func":
    {"params": }
    {"=": }
}
```
