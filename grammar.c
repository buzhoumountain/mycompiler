#include <const.h>

void translation_unit() {
    while(token != TK_EOF) {
        external_declaration(SC_GLOBAL);
    }
}

void external_declaration(int l) {
    if (!type_specifier()) {
        expect("<类型区分符>");
    }
}