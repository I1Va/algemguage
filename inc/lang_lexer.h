#ifndef LANG_LEXER_H
#define LANG_LEXER_H

#include <string.h>
#include "lang_global_space.h"

#define ScannerError(p, c) {debug("ScannerError: text[%d] : '%c'", p, c); fprintf(stderr, WHT); abort();}

bool char_in_str_lex(int c);
size_t add_to_name_table(char *new_name, key_name_t *name_table, size_t *name_table_sz);
lexem_t next_lexem(parsing_block_t *data);
void text_pos_update(text_pos_t *text_pos, const lexem_t lexem);
void lex_scanner(parsing_block_t *data);
size_t get_constantame_table_sz(key_name_t *name_table);

#endif // LANG_LEXER_H