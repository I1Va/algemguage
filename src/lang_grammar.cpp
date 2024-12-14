#include <assert.h>
#include <cstddef>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <sys/types.h>

#include "lang_grammar.h"
#include "diff_funcs.h"
#include "diff_tree.h"
#include "general.h"
#include "diff_DSL.h"
#include "string_funcs.h"

// void SyntaxError() {
//     abort();
// }

#define SyntaxError(p) {debug("SyntaxError: p=%d", p); fprintf(stderr, WHT); abort();}
#define ScannerError(p, c) {debug("ScannerError: text[%d] : '%c'", p, c); fprintf(stderr, WHT); abort();}

bool char_in_str_lex(int c) {
    return ('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z') || (c == '_');
}

size_t add_to_name_table(char *new_name, key_name_t *name_table, size_t *name_table_sz) {
    assert(new_name);
    assert(name_table);
    assert(name_table_sz);

    for (size_t name_idx = 0; name_idx < *name_table_sz; name_idx++) {
        if (strcmp(new_name, name_table[name_idx].name) == 0) {
            return name_idx;
        }
    }
    name_table[(*name_table_sz)++] = {new_name, strlen(new_name), T_ID};

    return (*name_table_sz) - 1;
}

lexem_t next_lexem(parsing_block_t *data) {
    char *s = data->text;
    size_t *p = &data->text_idx;

    int c = s[(*p)++];

    long long lval = 0;
    char bufer[MEDIUM_BUFER_SZ] = {};
    size_t bufer_idx = 0;
    char *str = NULL;

    lexem_t lexem = {};

    if (isdigit(c)) {
        lexem.token_type = T_NUM;
        size_t start_p = *p;

        while (isdigit(c)) {
            lval = 10 * lval + c - '0';
            c = s[(*p)++];
        }
        (*p)--;

        lexem.token_val.lval = lval;
        lexem.len = (*p) + 1 - start_p;

        return lexem;
    }

    if (char_in_str_lex(c)) {
        while (char_in_str_lex(c)) {
            bufer[bufer_idx++] = (char) c;
            c = s[(*p)++];
        }
        (*p)--;

        str = get_new_str_ptr(data->storage, bufer_idx);
        strncpy(str, bufer, bufer_idx); // FIXME: можно ускорить

        size_t name_idx = add_to_name_table(str, data->name_table, &data->name_table_sz);

        lexem.token_type = data->name_table[name_idx].token_type;
        lexem.token_val.ival = (int) name_idx;
        lexem.len = data->name_table[name_idx].len;

        return lexem;
    }

    switch (c) {
        case '+': return {T_ADD, {}, {}, 1}; case '-': return {T_SUB, {}, {}, 1};
        case '*': return {T_MUL, {}, {}, 1};
        case '(': return {T_O_BRACE, {}, {}, 1}; case ')': return {T_C_BRACE, {}, {}, 1};
        case '{': return {T_O_FIG_BRACE, {}, {}, 1}; case '}': return {T_C_FIG_BRACE, {}, {}, 1};
        case '\n': return {T_EOL, {}, {}, 1};
        case ' ': return {T_SPACE, {}, {}, 1};
        case '/': return {T_DIV, {}, {}, 1};
        case '\t': return {T_SPACE, {}, {}, 4};
        case '^': return {T_POW, {}, {}, 1};
        case EOF: return {T_EOF, {}, {}, 1};
        case '\0': return {T_EOF, {}, {}, 1};
        default: ScannerError(*p, s[*p])
    }
    return {T_EOF};
}

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

void text_pos_update(text_pos_t *text_pos, const lexem_t lexem) {
    if (lexem.token_type == T_EOL) {
        text_pos->lines++;
        text_pos->syms = 0;
        return;
    }

    text_pos->syms += lexem.len;
}

void lex_scanner(parsing_block_t *data) {
    assert(data != NULL);

    size_t token_idx = 0;
    text_pos_t cur_text_pos = {};

    printf("text: '%s'\n", data->text);

    while (1) {
        lexem_t lexem = next_lexem(data);

        lexem.text_pos = cur_text_pos;
        text_pos_update(&cur_text_pos, lexem);

        if (lexem.token_type != T_SPACE && lexem.token_type != T_EOL) {
            data->lexem_list[token_idx++] = lexem;
        }
        if (lexem.token_type == T_EOF) {
            break;
        }
    }
    data->lexem_list_size = token_idx;

    lexem_list_dump(stdout, data);
}




















bin_tree_elem_t *get_code_block(parsing_block_t *data) {
    assert(data != NULL);

    lexem_t *tl = data->lexem_list;
    size_t *tp = &(data->lexem_list_idx);

    bin_tree_elem_t *val = get_additive_expression(data);

    if (tl[*tp].token_type != T_EOF) {
        SyntaxError(*tp);
    }
    (*tp)++;
    return val;
}

bin_tree_elem_t *get_additive_expression(parsing_block_t *data) {
    assert(data != NULL);

    lexem_t *tl = data->lexem_list;
    size_t *tp = &(data->lexem_list_idx);

    bin_tree_elem_t *val = get_multiplicative_expression(data);
    while (tl[*tp].token_type == T_ADD || tl[*tp].token_type == T_SUB) {
        token_t op = tl[*tp].token_type;
        (*tp)++;
        bin_tree_elem_t * val2 = get_multiplicative_expression(data);

        if (op == T_ADD) {
            val = _ADD(val, val2);
        } else {
            val = _SUB(val, val2);
        }
    }

    return val;
}

bin_tree_elem_t *get_multiplicative_expression(parsing_block_t *data) {
    assert(data != NULL);

    lexem_t *tl = data->lexem_list;
    size_t *tp = &(data->lexem_list_idx);
    bin_tree_elem_t *val = get_direct_declarator(data);

    while (tl[*tp].token_type == T_MUL || tl[*tp].token_type == T_DIV) {
        token_t op = tl[*tp].token_type;
        (*tp)++;
        bin_tree_elem_t *val2 = get_direct_declarator(data);
        if (op == T_MUL) {
            val = _MUL(val, val2);
        } else {
            val = _DIV(val, val2);
        }
        printf("type: (%d)\n", tl[*tp].token_type);
    }

    return val;
}

// there is should be pow grammar rule

bin_tree_elem_t *get_direct_declarator(parsing_block_t *data) {
    assert(data != NULL);

    lexem_t *tl = data->lexem_list;
    size_t *tp = &(data->lexem_list_idx);
    if (tl[*tp].token_type == T_O_BRACE) {
        (*tp)++;
        bin_tree_elem_t *val = get_additive_expression(data);
        if (tl[*tp].token_type != T_C_BRACE) {
            SyntaxError(*tp);
            return NULL;
        }
        (*tp)++;
        return val;
    } else if (tl[*tp].token_type == T_ID && tl[(*tp) + 1].token_type == T_O_BRACE) {
        return get_function(data);
    } else if (tl[*tp].token_type == T_ID || tl[*tp].token_type == T_NUM) {
        return get_primary_expression(data);
    } else {
        SyntaxError(*tp);
        return NULL;
    }
}

bin_tree_elem_t *get_function(parsing_block_t *data) {
    assert(data != NULL);

    lexem_t *tl = data->lexem_list;
    size_t *tp = &(data->lexem_list_idx);

    if (tl[*tp].token_type == T_ID && tl[(*tp) + 1].token_type == T_O_BRACE) {
        lexem_t func_lexem = tl[*tp];
        (*tp) += 2;
        bin_tree_elem_t *val = get_additive_expression(data);
        if (tl[*tp].token_type != T_C_BRACE) {
            SyntaxError(*tp);
            return NULL;
        }
        (*tp)++;
        return _FUNC(val, data->name_table[func_lexem.token_val.ival].name);
    } else {
        SyntaxError(*tp);
        return NULL;
    }
}

bin_tree_elem_t *get_primary_expression(parsing_block_t *data) {
    assert(data != NULL);

    lexem_t *tl = data->lexem_list;
    size_t *tp = &(data->lexem_list_idx);

    if (tl[*tp].token_type == T_ID) {
        return get_identificator(data);
    } else if (tl[*tp].token_type == T_NUM) {
        return get_constant(data);
    } else {
        SyntaxError(*tp);
        return NULL;
    }

    // FIXME: добавить возможность поддержки выражений вида ((((1007))))
}

bin_tree_elem_t *get_constant(parsing_block_t *data) {
    assert(data != NULL);

    lexem_t *tl = data->lexem_list;
    size_t *tp = &(data->lexem_list_idx);

    long long val = 0;
    if (tl[*tp].token_type != T_NUM) {
        SyntaxError(*tp);
        return NULL;
    }

    val = tl[*tp].token_val.lval;
    (*tp)++;

    return _NUM(val);
}

bin_tree_elem_t *get_identificator(parsing_block_t *data) {
    assert(data != NULL);

    lexem_t *tl = data->lexem_list;
    size_t *tp = &(data->lexem_list_idx);

    if (tl[*tp].token_type != T_ID) {
        SyntaxError(*tp);
        return NULL;
    }
    char *var_name = data->name_table[tl[*tp].token_val.ival].name;
    (*tp)++;

    return _VAR(var_name);
}
