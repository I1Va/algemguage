#include <assert.h>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <stdio.h>
#include <stdlib.h>

#include "general.h"
#include "diff_tree.h"
#include "graphviz_funcs.h"
#include "stack_funcs.h"
#include "diff_funcs.h"
#include "string_funcs.h"
#include "lang_grammar.h"
#include "diff_DSL.h"

const char DOT_DIR_PATH[] = "./logs";
const char LOG_FILE_PATH[] = "./logs/log.html";
const char DOT_FILE_NAME[] = "graph.dot";
const char DOT_IMG_NAME[] = "gr_img.png";
const char CODE_FILE_PATH[] = "./code.txt";

// struct parsing_block_t {
//     token_t *token_list;
//     size_t token_list_idx;

//     char *text;
//     size_t text_idx;

//     key_name_t *name_table;
//     size_t name_table_sz;

//     bin_tree_t *tree;
//     dot_code_t *dot_code;
//     str_storage_t **storage;
// };

int main() {
    str_storage_t *storage = str_storage_t_ctor(CHUNK_SIZE);
    str_t text = read_text_from_file(CODE_FILE_PATH);
    dot_code_t dot_code = {}; dot_code_t_ctor(&dot_code, LIST_DOT_CODE_PARS);
    dot_dir_t dot_dir = {}; dot_dir_ctor(&dot_dir, DOT_DIR_PATH, DOT_FILE_NAME, DOT_IMG_NAME);

    bin_tree_t tree = {};
    bin_tree_ctor(&tree, LOG_FILE_PATH);

    lexem_t lexem_list[BUFSIZ] = {};
    key_name_t name_table[BUFSIZ] = {};

    parsing_block_t data = {};

    data.text_idx = 0;
    data.text = text.str_ptr;
    data.lexem_list = lexem_list;
    data.lexem_list_idx = 0;
    data.tree = &tree;
    data.dot_code = &dot_code;
    data.storage = &storage;
    data.name_table = name_table;
    data.name_table_sz = sizeof(name_table) / sizeof(key_name_t);

    lex_scanner(&data);























    // tree.root = get_G(&data);
    // convert_subtree_to_dot(tree.root, &dot_code, &storage);
    // dot_code_render(&dot_dir, &dot_code);

    FREE(text.str_ptr);
    sub_tree_dtor(tree.root);
    str_storage_t_dtor(storage);
    dot_code_t_dtor(&dot_code);

    return EXIT_SUCCESS;
}
