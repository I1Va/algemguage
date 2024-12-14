#include "lang_grammar.h"
#include "lang_logger.h"
#include "assert.h"

void lexem_dump(FILE *stream, key_name_t *name_table, lexem_t lexem) {
    #define T_DESCR_(stream, lex, fmt, val) case lex: fprintf(stream, #lex"(" fmt ")", val); break;
    fprintf(stream, "[l:%lu, s:%lu]", lexem.text_pos.lines + 1, lexem.text_pos.syms + 1);
    switch (lexem.token_type) {
        T_DESCR_(stream, T_EOF, "%s", "")
        T_DESCR_(stream, T_EMPTY, "%s", "")

        T_DESCR_(stream, T_NUM, "%Ld", lexem.token_val.lval)
        T_DESCR_(stream, T_ADD, "%c", '+')
        T_DESCR_(stream, T_MUL, "%c", '*')
        T_DESCR_(stream, T_SUB, "%c", '-')
        T_DESCR_(stream, T_DIV, "%c", '/')
        T_DESCR_(stream, T_O_BRACE, "%c", '(')
        T_DESCR_(stream, T_C_BRACE, "%c", ')')
        T_DESCR_(stream, T_O_FIG_BRACE, "%c", '{')
        T_DESCR_(stream, T_C_FIG_BRACE, "%c", '}')
        T_DESCR_(stream, T_EOL, "%s", "\\n")
        T_DESCR_(stream, T_SPACE, "%c", ' ')
        T_DESCR_(stream, T_POW, "%s", "^")
        T_DESCR_(stream, T_ID, "%s", name_table[lexem.token_val.ival].name)
        T_DESCR_(stream, T_IF, "%s", name_table[lexem.token_val.ival].name)
        T_DESCR_(stream, T_WHILE, "%s", name_table[lexem.token_val.ival].name)
        default: fprintf(stream, "UNKNOWN_LEX(%d) ", lexem.token_type); break;
    }
    #undef T_DESCR_
}

void lexem_list_dump(FILE *stream, parsing_block_t *data) {
    printf("len: [%lu]\n", data->lexem_list_size);
    for (size_t i = 0; i < data->lexem_list_size; i++) {
        lexem_t lexem = data->lexem_list[i];
        lexem_dump(stream, data->name_table, lexem);
        fprintf(stream, " ");
    }
    fprintf(stream, "\n");
}

void name_table_dump(FILE *stream, key_name_t *name_table, const size_t name_table_sz) {
    fprintf(stream, "name_table[%p]\n{\n", name_table);

    for (size_t i = 0; i < name_table_sz; i++) {
        fprintf(stream, "    [%lu] = {'%s', [%d]}\n", i, name_table[i].name, name_table[i].token_type);
    }

    fprintf(stream, "}\n");
}

void grule_dump(FILE *stream, enum grammar_rule_num grule) {
    #define GR_DESCR_(stream, grule) case grule: fprintf(stream, #grule); break;

    switch (grule) {
        GR_DESCR_(stream, EMPTY_GRULE)
        GR_DESCR_(stream, GET_CODE_BLOCK)
        GR_DESCR_(stream, GET_ADDITIVE_EXPRESSION)
        GR_DESCR_(stream, GET_MULTIPLICATIVE_EXPRESSION)
        GR_DESCR_(stream, GET_DIRECT_DECLARATOR)
        GR_DESCR_(stream, GET_FUNCTION)
        GR_DESCR_(stream, GET_PRIMARY_EXPRESSION)
        GR_DESCR_(stream, GET_CONSTANT)
        GR_DESCR_(stream,GET_IDENTIFICATOR)
        default: fprintf(stream, "UNKNOWN_GRULE(%d) ", grule); break;
    }
    #undef GR_DESCR_
}

void fprintf_border(FILE* stream, const char bord_char, const size_t bord_sz, bool new_line) {
    for (size_t i = 0; i < bord_sz; i++) {
        fputc(bord_char, stream);
    }
    if (new_line) {
        fputc('\n', stream);
    }
}

void fprintf_title(FILE *stream, const char tittle[], const char bord_char, const size_t bord_sz) {
    assert(tittle != NULL);

    size_t tittle_sz = strlen(tittle);
    if (bord_sz < tittle_sz) {
        return;
    }
    size_t len = bord_sz - tittle_sz;
    fprintf_border(stream, bord_char, len / 2, false);
    fprintf_red(stream, "%s", tittle);
    fprintf_border(stream, bord_char, (len + 1) / 2, true);
}
