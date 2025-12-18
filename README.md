# Язык программирования

## Синтаксис

### Таблица операторов

| Номер | Оператор | Имя                         |
|-------|----------|-----------------------------|
|   1   | OP_ELSE  | ""                          |
|   2   | OP_ADD   | "нарастить на"              |
|   3   | OP_SUB   | "избавить от"               |
|   4   | OP_MUL   | "усилить в"                 |
|   5   | OP_DIV   | "расщепить на"              |
|   6   | OP_POW   | "расплодить" <power> "раз"  |

### Пример программы, считающей факториал:

```
голоса: ААААААААААААААА

за Факториал отомсти н

    за что? за что? за что?

    пересади в результат 1    ЗАМОЛЧИ
    пересади в итератор  1    ЗАМОЛЧИ

    снова и снова
    снова и снова
    снова и снова
    снова и снова
    снова и снова

    н избавить от итератор

    ЗАТКНИСЬ ЗАТКНИСЬ ЗАТКНИСЬ ЗАТКНИСЬ ЗАТКНИСЬ

        пересади в результат результат усилить в итератор ЗАМОЛЧИ
        пересади в итератор  итератор  нарастить на 1     ЗАМОЛЧИ

    ОНИ СМОТРЯТ ОНИ СМОТРЯТ ОНИ СМОТРЯТ ОНИ СМОТРЯТ

    оставь результат ЗАМОЛЧИ

    нет нет нет нет все кончено?


скажи мне кто ты, приятель                                            ЗАМОЛЧИ
пересади в ответ ритуал: во имя Факториал принеси в жертву приятель   ЗАМОЛЧИ
заставь их услышать ответ                                             ЗАМОЛЧИ

```

### Эквивалент программы на C - подобном языке

```cpp

int ComputeFactorial(int n)
{
    int result = 1;

    int i = 1;

    while (i <= n)
    {
        result *= i;
        i = i + 1;
    }

    return result;
}

scanf(n);
res = ComputeFactorial(n);
printf(res);


```

## Грамматика в расширенной форме Бэкуса — Наура
```
Program                ::= Body '\0'

FunctionDeclaration    ::= 'за' Identifier 'отомсти' FunctionParameters?
                            FunctionBlockStatement

FunctionParameters     ::= Identifier { 'и' Identifier }*

FunctionBlockStatement ::= 'за что? за что? за что?'
                           {FunctionStatement OperatorSeparator}+
                           'нет нет нет нет все кончено?'

FunctionStatement      ::= Statement | ReturnStatement

ReturnStatement        ::= 'оставь' Expression

Body                   ::= {Statement OperatorSeparator}*

OperatorSeparator      ::= 'ЗАМОЛЧИ'

Statement              ::= IfStatement
                         | WhileStatement
                         | FunctionDeclaration
                         | Assignment
                         | Expression

IfStatement            ::= 'ты думаешь' Expression 'сможет что-то изменить'
                         BlockStatement
                         ElseStatement?

ElseStatement          ::= 'не верь им'
                         BlockStatement

WhileStatement         ::=  'снова и снова'
                            'снова и снова'
                            'снова и снова'
                            'снова и снова'
                            'снова и снова'
                            Expression
                            BlockStatement

BlockStatement         ::= 'ЗАТКНИСЬ' 'ЗАТКНИСЬ' 'ЗАТКНИСЬ' 'ЗАТКНИСЬ' 'ЗАТКНИСЬ'
                            {Statement OperatorSeparator}*
                           'ОНИ СМОТРЯТ' 'ОНИ СМОТРЯТ' 'ОНИ СМОТРЯТ' 'ОНИ СМОТРЯТ'

Assignment             ::= 'пересади в' Identifier Expression

Expression             ::= Term   { ['нарастить на''избавить от']  Term   }*
Term                   ::= Power  { ['усилить в''расщепить на']    Power  }*
Power                  ::= Factor { 'расплодить в'                 Factor }*

Factor                 ::= '(' Expression ')'
                         | UnaryOperatorCall
                         | FunctionCall
                         | Number
                         | Identifier

UnaryOperatorCall      ::= UnaryOperator Expression

UnaryOperator          ::= 'заставь их услышать' | 'скажи мне кто ты,'

FunctionCall           ::= 'ритуал: во имя' Identifier 'принеси в жертву' FunctionArguments?

FunctionArguments      ::= Expression { 'и' Expression }*

Identifier             ::= ['A'-'Z''a'-'z''А'-'Я''а'-'я''_']['A'-'Z''a'-'z''А'-'Я''а'-'я''_''0'-'9']+
Number                 ::= ['0'-'9']+ | ['0'-'9']+ '.' ['0'-'9']+

```

// in function call may be 'никого' if zero arguments

## Абстрактное Синтаксическое Дерево (AST)


```
{ "func":
    {"params": }
    {"=": }
}
```
