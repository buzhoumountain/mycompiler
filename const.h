#define NULL 0
enum e_TokenCode {
    // 运算符，分隔符 红色
    TK_PLUS = 1,
    TK_MINUS,
    TK_STAR,
    TK_DIVIDE,
    TK_MOD,
    TK_EQ,
    TK_NEQ,
    TK_LT,
    TK_LEQ,
    TK_GT,
    TK_GEQ,
    TK_ASSIGN,
    TK_POINTSTO,
    TK_DOT,
    TK_AND,
    TK_OPENPA,
    TK_CLOSEPA,
    TK_OPENBR,
    TK_CLOSEBR,
    TK_BEGIN,
    TK_END,
    TK_SEMICOLON,
    TK_COMMA,
    TK_ELLIPSIS,
    TK_EOF,

    // 常量 黄色
    TK_CINT,
    TK_CCHAR,
    TK_CSTR,
    
    // 关键字 绿色
    KW_CHAR,
    KW_SHORT,
    KW_INT,
    KW_VOID,
    KW_STRUCT,

    KW_IF,
    KW_ELSE,
    KW_FOR,
    KW_CONTINUE,
    KW_BREAK,
    KW_RETURN,
    KW_SIZEOF,
    KW_ALIGN,
    KW_CDECL,
    KW_STDCALL,

    // 标识符 灰色
    TK_IDENT
};