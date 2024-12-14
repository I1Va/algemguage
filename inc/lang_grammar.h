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
    T_SUB = 4,
    T_DIV = 5,
    T_POW = 6,

    T_O_BRACE = 7,
    T_C_BRACE = 8,
    T_O_FIG_BRACE = 9,
    T_C_FIG_BRACE = 10,

    T_EOL = 11, // '\n'

    T_SPACE = 12,


    T_ID = 13,
    T_IF = 14, // key_words
    T_WHILE = 15,


};

struct text_pos_t {
    size_t lines;
    size_t syms;
};

struct lexem_t {
    enum token_t token_type;
    union token_value_t token_val;

    text_pos_t text_pos;
    size_t len;
};

struct key_name_t {
    char *name;
    size_t len;
    token_t token_type;
};


struct parser_err_t {
    text_pos_t err_lex_pos;
    lexem_t lex;
};

struct parsing_block_t {
    char *text;
    size_t text_idx;

    key_name_t *name_table;
    size_t name_table_sz;

    lexem_t *lexem_list;
    size_t lexem_list_idx;
    size_t lexem_list_size;

    parser_err_t parser_err;

    bin_tree_t *tree;
    dot_code_t *dot_code;
    str_storage_t **storage;
};

void draw_parsing_text(parsing_block_t *data);

void SyntaxError();



lexem_t next_lexem(parsing_block_t *data);

void token_list_dump(FILE *stream, parsing_block_t *data);
void name_table_dump(FILE *stream, key_name_t *name_table, const size_t name_table_sz);
void lexem_dump(FILE *stream, key_name_t *name_table, lexem_t lexem);

void lex_scanner(parsing_block_t *data);


bin_tree_elem_t *get_code_block(parsing_block_t *data);
bin_tree_elem_t *get_additive_expression(parsing_block_t *data);
bin_tree_elem_t *get_multiplicative_expression(parsing_block_t *data);
bin_tree_elem_t *get_direct_declarator(parsing_block_t *data);

bin_tree_elem_t *get_function(parsing_block_t *data);
bin_tree_elem_t *get_primary_expression(parsing_block_t *data);
bin_tree_elem_t *get_constant(parsing_block_t *data);
bin_tree_elem_t *get_identificator(parsing_block_t *data);
#endif // LANG_GRAMMAR_H