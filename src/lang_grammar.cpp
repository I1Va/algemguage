#include <assert.h>
#include <cstddef>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

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

        while (isdigit(c)) {
            lval = 10 * lval + c - '0';
            c = s[(*p)++];
        }
        (*p)--;

        lexem.token_val.lval = lval;
        return lexem;
    }

    if (char_in_str_lex(c)) {
        lexem.token_type = T_STR;
        while (char_in_str_lex(c)) {
            bufer[bufer_idx++] = (char) c;
            c = s[(*p)++];
        }
        (*p)--;

        printf("recognised word: '%s'\n", bufer);

        str = get_new_str_ptr(data->storage, bufer_idx);
        strncpy(str, bufer, bufer_idx);

        lexem.token_val.sval = str;
        return lexem;
    }

    switch (c) {
        case '+': return {T_ADD}; case '-': return {T_SUB};
        case '*': return {T_MUL};
        case '(': return {T_OBRACE}; case ')': return {T_CBRACE};
        case '\n': return {T_EOL};
        case ' ': return {T_SPACE};
        case '/': return {T_DIV};
        case '\t': return {T_SPACE};
        case '^': return {T_POW};
        case EOF: return {T_EOF};
        case '\0': return {T_EOF};
        default: ScannerError(*p, s[*p])
    }
    return {T_EOF};
}

void lexem_list_dump(FILE *stream, lexem_t *lexem_list, const size_t len) {
    #define T_DESCR_(stream, lex, fmt, val) case lex: fprintf(stream, #lex"(" fmt ") ", val); break;

    for (size_t i = 0; i < len; i++) {
        switch (lexem_list[i].token_type) {
            T_DESCR_(stream, T_ADD, "%c", '+')
            T_DESCR_(stream, T_EOF, "%s", "")
            T_DESCR_(stream, T_NUM, "%Ld", lexem_list[i].token_val.lval)
            T_DESCR_(stream, T_MUL, "%c", '*')
            T_DESCR_(stream, T_SUB, "%c", '-')
            T_DESCR_(stream, T_DIV, "%c", '/')
            T_DESCR_(stream, T_OBRACE, "%c", '(')
            T_DESCR_(stream, T_CBRACE, "%c", ')')
            T_DESCR_(stream, T_EOL, "%s", "\\n")
            T_DESCR_(stream, T_POW, "%s", "^")
            T_DESCR_(stream, T_STR, "%s", lexem_list[i].token_val.sval)
            T_DESCR_(stream, T_SPACE, "%c", ' ')
            default: fprintf(stream, "UNKNOWN_LEX(%d) ", lexem_list[i].token_type); break;
        }
    }
    fprintf(stream, "\n");

    #undef T_DESCR_
}

void lex_scanner(parsing_block_t *data) {
    assert(data != NULL);

    size_t token_idx = 0;

    while (1) {
        lexem_t lexem = next_lexem(data);
        if (lexem.token_type != T_SPACE && lexem.token_type != T_EOL) {
            data->lexem_list[token_idx++] = lexem;
        }
        if (lexem.token_type == T_EOF) {
            break;
        }
    }

    lexem_list_dump(stdout, data->lexem_list, token_idx);
}




















bin_tree_elem_t *get_G(parsing_block_t *data) {
    assert(data != NULL);

    lexem_t *tl = data->lexem_list;
    size_t *tp = &(data->lexem_list_idx);
    bin_tree_elem_t *val = get_E(data);

    if (tl[*tp].token_type != T_EOF) {
        SyntaxError(*tp);
    }
    (*tp)++;
    return val;
}

bin_tree_elem_t *get_E(parsing_block_t *data) {
    assert(data != NULL);

    lexem_t *tl = data->lexem_list;
    size_t *tp = &(data->lexem_list_idx);

    bin_tree_elem_t *val = get_T(data);
    while (tl[*tp].token_type == T_ADD || tl[*tp].token_type == T_SUB) {
        token_t op = tl[*tp].token_type;
        (*tp)++;
        bin_tree_elem_t * val2 = get_T(data);

        if (op == T_ADD) {
            val = _ADD(val, val2);
        } else {
            val = _SUB(val, val2);
        }
    }

    return val;
}

bin_tree_elem_t *get_T(parsing_block_t *data) {
    assert(data != NULL);

    lexem_t *tl = data->lexem_list;
    size_t *tp = &(data->lexem_list_idx);
    bin_tree_elem_t *val = get_P(data);

    while (tl[*tp].token_type == T_MUL || tl[*tp].token_type == T_DIV) {
        token_t op = tl[*tp].token_type;
        (*tp)++;
        bin_tree_elem_t *val2 = get_P(data);
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


bin_tree_elem_t *get_P(parsing_block_t *data) {
    assert(data != NULL);

    lexem_t *tl = data->lexem_list;
    size_t *tp = &(data->lexem_list_idx);

    if (tl[*tp].token_type == T_OBRACE) {
        (*tp)++;
        bin_tree_elem_t *val = get_E(data);
        if (tl[*tp].token_type != T_CBRACE) {
            SyntaxError(*tp);
        }
        (*tp)++;
        return val;
    } else if (tl[*tp].token_type == T_STR && tl[(*tp) + 1].token_type == T_OBRACE) {
        return get_F(data);
    } else if (tl[*tp].token_type == T_STR && strcmp(tl[*tp].token_val.sval, "x") == 0) {
        return get_V(data);
    } else if (tl[*tp].token_type == T_NUM) {
        return get_N(data);
    } else {
        SyntaxError(*tp)
        return NULL;
    }
}

bin_tree_elem_t *get_N(parsing_block_t *data) {
    assert(data != NULL);

    lexem_t *tl = data->lexem_list;
    size_t *tp = &(data->lexem_list_idx);

    long long val = 0;
    if (tl[*tp].token_type != T_NUM) {
        SyntaxError(*tp);
    }

    val = tl[*tp].token_val.lval;
    (*tp)++;

    return _NUM(val);
}

bin_tree_elem_t *get_F(parsing_block_t *data) {
    assert(data != NULL);

    lexem_t *tl = data->lexem_list;
    size_t *tp = &(data->lexem_list_idx);

    if (tl[*tp].token_type == T_STR && tl[(*tp) + 1].token_type == T_OBRACE) {
        char *func_name = tl[*tp].token_val.sval;

        (*tp) += 2;
        bin_tree_elem_t *val = get_E(data);
        printf("E : '%d'\n", val->data.type);
        if (tl[*tp].token_type != T_CBRACE) {
            SyntaxError(*tp);
        }
        (*tp)++;

        return _FUNC(val, func_name);
    } else {
        SyntaxError(*tp);
        return NULL;
    }
}

bin_tree_elem_t *get_V(parsing_block_t *data) {
    assert(data != NULL);

    lexem_t *tl = data->lexem_list;
    size_t *tp = &(data->lexem_list_idx);

    if (tl[*tp].token_type != T_STR && strcmp(tl[*tp].token_val.sval, "x") != 0) {
        SyntaxError(*tp);
    }
    (*tp)++;

    return _VAR();
}
