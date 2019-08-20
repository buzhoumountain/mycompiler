×××××××××××××××××××××××××
上下文无关文法 + LL(1)


×××××××××××××××××××××××××
sc 文法知识
sc 词法定义
sc 语法定义

词法定义：

2.2 词法分析方法
2.3 ll(k)分析器

3.4 sc词法定义
关键词，定义，语义
标识符，定义，语义，解释
整数常量：定义，语义，解释
字符常量：
字符串常量：
运算符及分隔符：定义，语义，约束：[] () {}成对


3.5 语法定义：
3.5.1 外部定义
1.定义
<翻译单元>::={外部声明}<文件结束符>
<外部声明>::=(函数定义)|<声明>

2.语义
每个sc源文件是一个翻译单元，它由一系列外部声明组成。将这些描述为“外部的”是因为他们出现在任何函数之外，因而具有全局作用域

3.5.1.1 函数定义
1. 定义
<函数定义>::=<类型区分符><声明符><函数体>
<函数体>::=<复合语句>

2。语义
函数定义中的声明符指定要定义的函数名及形式参数列表

3.约束
在函数定义中声明的标识符（即函数名）应为函数类型，该类型由函数定义的声明符部分所说明。

3.5.1.2 声明： 
定义，语义，约束
<声明>::=<类型区分符>[<初值声明符表>]<分号>
<初值声明符表>::=<初值声明符>{<逗号><初值声明符>}
<初值声明符>::=<声明符>|<声明符><赋值运算符><初值符>

<类型区分符>::=<void关键字>|<char>|<short>|<int>|<结构区分符>
<结构区分符>::=<struct><标识符><左大括号><结构声明表><右大括号>|<struct><标识符>
<结构声明表>::=<结构声明>{<结构声明>}
<结构声明>::=<类型区分符>{<结构声明符表>}<分号>
<结构声明符表>::=<声明符>{<逗号><声明符>}


×××××××××××××××××××××××××
sc 语言词法分析
目标：根据语言的词法规则分析并识别具有独立意义的最小语法单位：单词，并以某种编码形式输出
单词编码：单词编码表
enum e_TokenCode {
    TK_PLUS, //+
    -,
    *,
    /,
    %,
    ==
    !=,
    <
    <=
    >
    >=
    =
    ->
    .
    &
    (
    )
    [
    ]
    {
    }
    ;
    ,
    ...
    TK_EOF,

    /*常量*/
    TK_CINT
    TK_CCHAR
    TK_CSTR

    /*关键字*/
    KW_CHAR,
    KW_SHORT
    int,
    void,
    struct,
    if
    else
    for
    continue,
    return
    sizeoff
    KW_ALIGH, // __align
    KE_CDECL, // __cdecl
    KW_STDCALL, // __stdcall

    /*标识符*/
    TK_IDENT
}

词法分析用到的数据结构
1.动态字符串
2.动态数组
3.hash 表


×××××××××××××××××××××××××
sc 语言语法分析
5.1 外部定义
5.1.1 翻译单元
<translation_unit>::={<external_declaration>}<TK_EOF>

5.1.2 外部声明
<external_declaration>::=<function_definition>|<declaration>
<function_definition>::=<type_specifier><declarator><funcbody>
<declaration>::=<type_specifier><TK_SEMICOLON>|<type_specifier><init_declarator_list><TK_SEMICOLON>
<init_declarator_list>::=<init_declarator>{<TK_COMMA><init_declarator>}
<init_declarator>::=<declarator>{<TK_ASSIGN><initializer>}
