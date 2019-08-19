typedef struct TkWord
{
    int tkcode;
    struct TkWord *next;
    char *spelling;
    struct Symbol * sym_struct;
    struct Symbol * sym_identifier;
} TkWord;

TkWord * tkword_direct_insert(TkWord * tp);
TkWord * tkword_find(char *p, int keyno);
TkWord * tkword_insert(char *p);
