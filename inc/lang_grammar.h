#ifndef LANG_GRAMMAR_H
#define LANG_GRAMMAR_H

#include "diff_tree.h"
#include "graphviz_funcs.h"
#include "string_funcs.h"

const size_t TOKEN_LIST_MAX_SZ = 1028;
union token_value_t {
    int ival;
    long long lval;
    long double dval;
};

enum token_t {
    T_EOF = -1,
    T_EMPTY = 0,

    T_NUM = 1,
    T_ADD = 2,
    T_MUL = 3,
    T_OBRACE = 5,
    T_CBRACE = 6,
    T_EOL = 7,
    T_SUB = 8,
    T_SPACE = 9,
    T_DIV = 11,
    T_POW = 12,
    T_VAR = 13,
    T_IF = 14,
    T_WHILE = 15,
    T_FUNC = 16,

};

struct lexem_t {
    enum token_t token_type;
    union token_value_t token_val;
};

struct key_name_t {
    char *name;
    size_t len;
    token_t token_type;
};

struct parsing_block_t {
    lexem_t *lexem_list;
    size_t lexem_list_sz;

    char *text;
    size_t text_idx;

    key_name_t *name_table;
    size_t name_table_sz;

    bin_tree_t *tree;
    dot_code_t *dot_code;
    str_storage_t **storage;
};

void draw_parsing_text(parsing_block_t *data);

void SyntaxError();



lexem_t next_lexem(parsing_block_t *data);

void token_list_dump(FILE *stream, parsing_block_t *data);
void name_table_dump(FILE *stream, key_name_t *name_table, const size_t name_table_sz);

void lex_scanner(parsing_block_t *data);


bin_tree_elem_t *get_G(parsing_block_t *data);
bin_tree_elem_t *get_E(parsing_block_t *data);
bin_tree_elem_t *get_T(parsing_block_t *data);
bin_tree_elem_t *get_P(parsing_block_t *data);
bin_tree_elem_t *get_N(parsing_block_t *data);
bin_tree_elem_t *get_V(parsing_block_t *data);
bin_tree_elem_t *get_F(parsing_block_t *data);

#endif // LANG_GRAMMAR_H