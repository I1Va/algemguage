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
#include "lang_global_space.h"
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
const char ASM_CODE_FILE_PATH[] = "./asm_code.txt";

bool parsing_block_t_ctor(parsing_block_t *data, char *text, key_name_t *name_table, lexem_t *lexem_list,
    dot_code_t *dot_code, str_storage_t **storage, const char asm_code_file_path[])
{
    data->text_idx = 0;
    data->text = text;

    data->lexem_list = lexem_list;
    data->lexem_list_idx = 0;
    data->lexem_list_size = 0;

    data->parser_err = {};

    data->dot_code = dot_code;
    data->storage = storage;
    data->name_table = name_table;
    data->name_table_sz = get_name_table_sz(name_table);

    data->asm_code_file_ptr = fopen(asm_code_file_path, "w");
    if (!data->asm_code_file_ptr) {
        debug("failed to open '%s'", asm_code_file_path);
        return false;
    }

    return true;
}

void parsing_block_t_dtor(parsing_block_t *data) {
    if (data && data->asm_code_file_ptr) {
        fclose(data->asm_code_file_ptr);
    }
}

int main() {
    str_storage_t *storage = str_storage_t_ctor(CHUNK_SIZE);
    str_t text = read_text_from_file(CODE_FILE_PATH);
    dot_code_t dot_code = {}; dot_code_t_ctor(&dot_code, LIST_DOT_CODE_PARS);
    dot_dir_t dot_dir = {}; dot_dir_ctor(&dot_dir, DOT_DIR_PATH, DOT_FILE_NAME, DOT_IMG_NAME);

    bin_tree_t tree = {};
    bin_tree_ctor(&tree, LOG_FILE_PATH);

    lexem_t lexem_list[BUFSIZ] = {};
    key_name_t name_table[BUFSIZ] =
    {
        {"EMPTY_NAME", 10, T_EMPTY},
        {"if", 2, T_IF},
        {"while", 5, T_WHILE},
    };

    parsing_block_t data = {};
    parsing_block_t_ctor(&data, text.str_ptr, name_table, lexem_list, &dot_code, &storage, ASM_CODE_FILE_PATH);


    lex_scanner(&data);



    tree.root = get_code_block(&data);
    if (check_parser_err(stdout, &data)) {
        CLEAR_MEMORY(exit_mark);
    }

    convert_subtree_to_dot(tree.root, &dot_code, &storage);
    dot_code_render(&dot_dir, &dot_code);


    FREE(text.str_ptr);
    sub_tree_dtor(tree.root);
    str_storage_t_dtor(storage);
    dot_code_t_dtor(&dot_code);
    parsing_block_t_dtor(&data);

    return EXIT_SUCCESS;


    exit_mark:
    FREE(text.str_ptr);
    str_storage_t_dtor(storage);
    dot_code_t_dtor(&dot_code);
    sub_tree_dtor(tree.root);
    parsing_block_t_dtor(&data);
    bin_tree_dtor(&tree);


    return EXIT_FAILURE;
}
