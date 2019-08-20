#include <malloc.h>
#include <stdarg.h>
#include <stdio.h>
#include "TokenWord.h"
#include "dynarray.h"
#include "dynstring.h"

#define CH_EOF -1
#define MAXKEY 1024

TkWord *tk_hashtable[MAXKEY];
DynArray tktable;
DynString tkstr;
DynString sourcestr;

int tkvalue;
char ch;
int token;
int line_num;

char* filename;
FILE* fin;

int elf_hash(char *key) {
    int h = 0, g;
    while (*key)
    {
        h = (h << 4) + * key ++;
        g = h & 0xf0000000;
        if (g) {
            h ^= g >> 24;
        }
        h &= ~g;
    }
    return h % MAXKEY;
}

TkWord * tkword_direct_insert(TkWord * tp) {
    int keyno;
    dynarray_add(&tktable, tp);
    keyno = elf_hash(tp->spelling);
    tp->next = tk_hashtable[keyno];
    tk_hashtable[keyno] = tp;
    return tp;
}

TkWord * tkword_find(char *p, int keyno) {
    TkWord * tp = NULL, *tp1;
    for (tp1 = tk_hashtable[keyno]; tp1; tp1 = tp1->next) {
        if (!strcmp(p, tp1->spelling)) {
            token = tp1->tkcode;
            tp = tp1;
        }
    }
    return tp;
}

TkWord * tkword_insert(char *p) {
    TkWord * tp;
    int keyno;
    char *s;
    char *end;
    int length;

    keyno = elf_hash(p);
    tp = tkword_find(p, keyno);
    if (tp == NULL) {
        length = strlen(p);
        tp = (TkWord*) mallocz(sizeof(TkWord) + length + 1);
        tp->next = tk_hashtable[keyno];
        tk_hashtable[keyno] = tp;
        dynarray_add(&tktable, tp);
        tp->tkcode = tktable.count - 1;
        s = (char*)tp + sizeof(TkWord);
        tp->spelling = (char*)s;
        for (end = p + length; p < end;) {
            *s++ = *p++;
        }
        *s = (char)'\0';
    }
    return tp;
}

void * mallocz(int size) {
    void * ptr;
    ptr = malloc(size);
    if (!ptr && size) {
        printf("内存分配失败");
    }
    memset(ptr, 0, size);
    return ptr;
}

void init_lex() {
    TkWord * tp;
    static TkWord keywords[] = {
        {TK_PLUS, NULL, "+", NULL, NULL},
        {TK_MINUS, NULL, "-", NULL, NULL},
        {TK_STAR, NULL, "*", NULL, NULL},
        {TK_DIVIDE, NULL, "/", NULL, NULL},
        {TK_MOD, NULL, "%", NULL, NULL},
        {TK_EQ, NULL, "==", NULL, NULL},
        {TK_NEQ, NULL, "!=", NULL, NULL},
        {TK_LT, NULL, "<", NULL, NULL},
        {TK_LEQ, NULL, "<=", NULL, NULL},
        {TK_GT, NULL, ">", NULL, NULL},
        {TK_GEQ, NULL, ">=", NULL, NULL},
        {TK_ASSIGN, NULL, "=", NULL, NULL},
        {TK_POINTSTO, NULL, "->", NULL, NULL},
        {TK_DOT, NULL, ".", NULL, NULL},
        {TK_AND, NULL, "&", NULL, NULL},
        {TK_OPENPA, NULL, "(", NULL, NULL},
        {TK_CLOSEPA, NULL, ")", NULL, NULL},
        {TK_OPENBR, NULL, "[", NULL, NULL},
        {TK_CLOSEBR, NULL, "]", NULL, NULL},
        {TK_BEGIN, NULL, "{", NULL, NULL},
        {TK_END, NULL, "}", NULL, NULL},
        {TK_SEMICOLON, NULL, ";", NULL, NULL},
        {TK_COMMA, NULL, ",", NULL, NULL},
        {TK_ELLIPSIS, NULL, "...", NULL, NULL},
        {TK_EOF, NULL, "End_Of_File", NULL, NULL},

        {TK_CINT, NULL, "整形常量", NULL, NULL},
        {TK_CCHAR, NULL, "字符常量", NULL, NULL},
        {TK_CSTR, NULL, "字符串常量", NULL, NULL},

        {KW_CHAR, NULL, "char", NULL, NULL},
        {KW_SHORT, NULL, "short", NULL, NULL},
        {KW_INT, NULL, "int", NULL, NULL},
        {KW_VOID, NULL, "void", NULL, NULL},
        {KW_STRUCT, NULL, "struct", NULL, NULL},

        {KW_IF, NULL, "if", NULL, NULL},
        {KW_ELSE, NULL, "else", NULL, NULL},
        {KW_FOR, NULL, "for", NULL, NULL},
        {KW_CONTINUE, NULL, "continue", NULL, NULL},
        {KW_BREAK, NULL, "break", NULL, NULL},
        {KW_RETURN, NULL, "return", NULL, NULL},
        {KW_SIZEOF, NULL, "sizeof", NULL, NULL},
        {KW_ALIGN, NULL, "__align", NULL, NULL},
        {KW_CDECL, NULL, "__cdecl", NULL, NULL},
        {KW_STDCALL, NULL, "__stdcall", NULL, NULL},
        {0, NULL, NULL, NULL, NULL}
    };

    dynarray_init(&tktable, 8);
    for (tp = &keywords[0]; tp->spelling != NULL; tp++) {
        tkword_direct_insert(tp);
    }
}

enum e_ErrorLevel {
    LEVEL_WARNING,
    LEVEL_ERROR,
};

enum e_WorkStage {
    STAGE_COMPILE,
    STAGE_LINK,
};

void handle_exception(int stage, int level, char *fmt, va_list ap) {
    char buf[1024];
    vsprintf(buf, fmt, ap);
    if (stage == STAGE_COMPILE) {
        if (level == LEVEL_WARNING) {
            printf("%s(第%d行)：编译警告：%s!\n", filename, line_num, buf);
        } else {
            printf("%s(第%d行)：编译错误：%s!\n", filename, line_num, buf);
            exit(-1);
        }
    } else {
        printf("链接错误：%s!\n", buf);
        exit(-1);
    }
}

void warning(char *fmt, ...) {
    va_list ap;

    va_start(ap, fmt);
    handle_exception(STAGE_COMPILE, LEVEL_WARNING, fmt, ap);
    va_end(ap);
}

void error(char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    handle_exception(STAGE_COMPILE, LEVEL_ERROR, fmt, ap);
    va_end(ap);
}

void expect(char * msg) {
    error("缺少%s", msg);
}

void skip(int c) {
    if (token != c) {
        error("缺少'%s'", get_tkstr(c));
    }
    get_token();
}

char * get_tkstr(int v) {
    if (v > tktable.count) {
        return NULL;
    } else if (v >= TK_CINT && v <= TK_CSTR) {
        return sourcestr.data;
    } else {
        return ((TkWord*)tktable.data[v])->spelling;
    }
}

void link_error(char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    handle_exception(STAGE_LINK, LEVEL_ERROR, fmt, ap);
    va_end(ap);
}

char getch() {
    ch = getc(fin);
    return ch;
}

void get_token() {
    preprocess();
    switch (ch)
    {
    case 'a':case 'b':case 'c':case 'd':case 'e':case 'f':case 'g':
    case 'h':case 'i':case 'j':case 'k':case 'l':case 'm':case 'n':
    case 'o':case 'p':case 'q':case 'r':case 's':case 't':
    case 'u':case 'v':case 'w':case 'x':case 'y':case 'z':
    case 'A':case 'B':case 'C':case 'D':case 'E':case 'F':case 'G':
    case 'H':case 'I':case 'J':case 'K':case 'L':case 'M':case 'N':
    case 'O':case 'P':case 'Q':case 'R':case 'S':case 'T':
    case 'U':case 'V':case 'W':case 'X':case 'Y':case 'Z':
    case '_':
        {
            TkWord * tp;
            parse_identifier();
            tp = tkword_insert(tkstr.data);
            token = tp->tkcode;
            break; 
        }
    case '0':case '1':case '2':case '3':case '4':
    case '5':case '6':case '7':case '8':case '9':
        parse_num();
        token = TK_CINT;
        break;
    case '+':
        getch();
        token = TK_PLUS;
        break;
    case '-':
        getch();
        if (ch == '>') {
            token = TK_POINTSTO;
            getch();
        } else {
            token = TK_MINUS;
        }
        break;
    case '*':
        getch();
        token = TK_STAR;
        break;
    case '/':
        // todo: support //
        getch();
        token = TK_DIVIDE;
        break;
    case '%':
        getch();
        token = TK_MOD;
        break;
    case '=':
        getch();
        if (ch == '=') {
            token = TK_EQ;
            getch();
        } else {
            token = TK_ASSIGN;
        }
        break;
    case '!':
        getch();
        if (ch == '=') {
            token = TK_NEQ;
            getch();
        } else {
            error("暂不支持‘！’（非操作符）");
        }
        break;
    case '<':
        getch();
        if (ch == '=') {
            token = TK_LEQ;
            getch();
        } else {
            token = TK_LT;
        }
        break;
    case '>':
        getch();
        if (ch == '=') {
            token = TK_GEQ;
            getch();
        } else {
            token = TK_GT;
        }
        break;
    case '.':
        getch();
        if (ch == '.') {
            getch();
            if (ch == '.') {
                token = TK_ELLIPSIS;
                getch();
            } else {
                error("省略号拼写错误");
            }
        } else {
            token = TK_DOT;
        }
        break;
    case '&':
        getch();
        token = TK_AND;
        break;
    case '(':
        token = TK_OPENPA;
        getch();
        break;
    case ')':
        token = TK_CLOSEPA;
        getch();
        break;
    case '[':
        token = TK_OPENBR;
        getch();
        break;
    case ']':
        token = TK_CLOSEBR;
        getch();
        break;
    case '{':
        token = TK_BEGIN;
        getch();
        break;
    case '}':
        token = TK_END;
        getch();
        break;
    case ';':
        token = TK_SEMICOLON;
        getch();
        break;
    case ',':
        token = TK_COMMA;
        getch();
        break;
    case '\'':
        parse_string(ch);
        token = TK_CCHAR;
        tkvalue = *(char*) tkstr.data;
        break;
    case '\"':
        parse_string(ch);
        token = TK_CSTR;
        break;
    case EOF:
        token = TK_EOF;
        break;
    default:
        error("不认识的字符：\\x%02x", ch);
        getch();
        break;
    }
}

void preprocess() {
    while(1) {
        if (ch == ' ' || ch == '\t' || ch == '\r' || ch == '\n') {
            skip_white_space();
        } else if (ch == '/') {
            getch();
            if (ch == '*') {
                parse_comment();
            } else if (ch == '/') {
                parse_comment_line();
            }else {
                ungetc(ch, fin);
                ch = '/';
                break;
            }
        } else {
            break;
        }
    }
}

void parse_comment() {
    getch();
    do {
        do {
            if (ch == '\n' || ch == '*' || ch == CH_EOF) {
                break;
            } else {
                getch();
            }
        } while(1);
        if (ch == '\n') {
            line_num ++;
            getch();
        } else if (ch == '*') {
            getch();
            if (ch == '/') {
                getch();
                return;
            }
        } else {
            error("一直到文件末尾看到配对的注释结束符");
            return;
        }
    } while (1);
}

void parse_comment_line() {
    getch();
    do {
        if (ch == CH_EOF) {
            break;
        } else {
            if (ch == '\n') {
                line_num ++;
                getch();
                break;
            }
            getch();
        }
    } while (1);
}

void skip_white_space() {
    // windows: \r\n
    // unix: \n
    // mac: \r
    while(ch == ' ' || ch == '\t' || ch == '\r' || ch == '\n') {
        if (ch == '\n') {
            line_num++;
        }
        printf("%c", ch);
        getch();
    }
}

// 解析标识符
int is_nodigit(char c) {
    return (c >= 'a' && c <= 'z') ||
        (c >= 'A' && c <= 'Z') ||
        c == '_';
}

int is_digit(char c) {
    return c >= '0' && c <= '9';
}

void parse_identifier() {
    dynstring_reset(&tkstr);
    dynstring_chcat(&tkstr, ch);
    getch();
    while (is_nodigit(ch) || is_digit(ch))
    {
        dynstring_chcat(&tkstr, ch);
        getch();
    }
    dynstring_chcat(&tkstr, '\0');
}

void parse_num() {
    dynstring_reset(&tkstr);
    dynstring_reset(&sourcestr);
    do {
        dynstring_chcat(&tkstr, ch);
        dynstring_chcat(&sourcestr, ch);
        getch();
    } while (is_digit(ch));
    if (ch == '.') {
        do {
            dynstring_chcat(&tkstr, ch);
            dynstring_chcat(&sourcestr, ch);
            getch();
        } while (is_digit(ch));
    }
    dynstring_chcat(&tkstr, '\0');
    dynstring_chcat(&sourcestr, '\0');
    tkvalue = atoi(tkstr.data);
}

void parse_string(char sep) {
    char c;
    dynstring_reset(&tkstr);
    dynstring_reset(&sourcestr);
    dynstring_chcat(&sourcestr, sep);
    getch();
    for (;;)
    {
        if (ch == sep) {
            break;
        } else if (ch == '\\') {
            dynstring_chcat(&sourcestr, ch);
            getch();
            switch (ch)
            {
            case '0':
                c = '\0';
                break;
            case 'a':
                c = '\a';
                break;
            case 'b':
                c = '\b';
                break;
            case 't':
                c = '\t';
                break;
            case 'n':
                c = '\n';
                break;
            case 'v':
                c = '\v';
                break;
            case 'f':
                c = '\f';
                break;
            case 'r':
                c = '\r';
                break;
            case '\"':
                c = '\"';
                break;
            case '\'':
                c = '\'';
                break;
            case '\\':
                c = '\\';
                break;
            default:
                c = ch;
                if (c >= '!' && c <= '~') {
                    warning("非法转义字符：\'\\%c\'", c);
                } else {
                    warning("非法转义字符：\'\\0x%x\'", c);
                }
                break;
            }
            // todo: ???
            dynstring_chcat(&tkstr, c);
            dynstring_chcat(&sourcestr, ch);
            getch();
        } else {
            // todo: ???
            dynstring_chcat(&tkstr, ch);
            dynstring_chcat(&sourcestr, ch);
            getch();
        }
    }
    dynstring_chcat(&tkstr, '\0');
    dynstring_chcat(&sourcestr, sep);
    dynstring_chcat(&sourcestr, '\0');
    getch();
}

enum e_LexState {
    LEX_NORMAL,
    LEX_SEP,
};



void color_token(int lex_state) {
#ifdef _WIN32
    #include <windows.h>
    HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);
    char *p;
    switch (lex_state)
    {
    case LEX_NORMAL:
        {
            if (token >= TK_IDENT) {
                // 标识符为灰色
                SetConsoleTextAttribute(h, FOREGROUND_INTENSITY)；
            } else if (token >= KW_CHAR) {
                // 关键字为绿色
                SetConsoleTextAttribute(h, FOREGROUND_GREEN | FOREGROUND_INTENSITY);
            } else if (token >= TK_CINT) {
                // 常量为黄色
                SetConsoleTextAttribute(h, FOREGROUND_RED | FOREGROUND_GREEN);
            } else {
                // 运算符及分隔符为红色
                SetConsoleTextAttribute(h, FOREGROUND_RED | FOREGROUND_INTENSITY);
            }
            p = get_tkstr(token);
            printf("%s", p);
            break;
        }
        break;
    case LEX_SEP:
        printf("%c", ch);
        break;
    }
#endif

#ifdef __unix__
#include <stdio.h>
    // Linux 终端控制台字体颜色,用到一个 转义序列
    // \e[F;B;Om
    // \e   转义字符开始
    // [    开始定义颜色
    // 'F'  字体颜色    编号30~37
    // 'B'  背景色  编号40~47
    // 'O'  为特殊意义代码
    // m 是标记
    // m 后面不用跟空格，是所定义的彩色字和背景

    // PS1='[\[\e[32;40m\]\u@\h \w \t]$ '

    // 颜色表
    // 前景 背景颜色
    // -------------------------
    // 30 40 黑色
    // 31 41 红色
    // 32 42 绿色
    // 33 43 黄色
    // 34 44 蓝色
    // 35 45 洋红 紫色
    // 36 46 青色 深绿
    // 37 47 白色

    // 特别代码意义
    // -------------------------
    // 0 OFF
    // 1 高亮显示
    // 4 underline
    // 5 闪烁
    // 7 反白显示
    // 8 不可见

    // \033[22;30m - black
    // \033[22;31m - red
    // \033[22;32m - green
    // \033[22;33m - brown
    // \033[22;34m - blue
    // \033[22;35m - magenta
    // \033[22;36m - cyan
    // \033[22;37m - gray
    // \033[01;30m - dark gray
    // \033[01;31m - light red
    // \033[01;32m - light green
    // \033[01;33m - yellow
    // \033[01;34m - light blue
    // \033[01;35m - light magenta
    // \033[01;36m - light cyan
    // \033[01;37m - white
    
    // ANSI控制码: 
    // QUOTE: 
    // /033[0m 关闭所有属性   
    // /033[1m 设置高亮度   
    // /03[4m 下划线   
    // /033[5m 闪烁   
    // /033[7m 反显   
    // /033[8m 消隐   
    // /033[30m -- /033[37m 设置前景色   
    // /033[40m -- /033[47m 设置背景色   
    // /033[nA 光标上移n行   
    // /03[nB 光标下移n行   
    // /033[nC 光标右移n行   
    // /033[nD 光标左移n行   
    // /033[y;xH设置光标位置   
    // /033[2J 清屏   
    // /033[K 清除从光标到行尾的内容   
    // /033[s 保存光标位置   
    // /033[u 恢复光标位置   
    // /033[?25l 隐藏光标   
    // /33[?25h 显示光标
    
    // echo -e  "\033[35;1m Shocking \033[0m"
    // printf("\033[34mThis is blue.\033[0m\n");

#endif
}

int main(int argc, char **argv) {
    fin = fopen(argv[1], "rb");
    if (!fin) {
        printf("不能打开sc源文件!\n");
        return 0;
    }
    init();

    getch();
    do {
        get_token();
        color_token(LEX_NORMAL);
    } while (token != TK_EOF);
    printf("\n代码行数：%d行\n", line_num);

    cleanup();
    fclose(fin);
    printf("%s词法分析成功!", argv[1]);
    return 1;
}

void init() {
    line_num = 1;
    init_lex();
}

void cleanup() {
    int i;
    printf("\n tktable.count=%d\n", tktable.count);
    for (i = TK_IDENT; i < tktable.count; i++) {
        free(tktable.data[i]);
    }
    free(tktable.data);
}
