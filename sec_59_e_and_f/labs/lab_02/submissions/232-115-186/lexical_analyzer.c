/* Lab 02: lexical analyzer for a small C subset.
   Usage: ./lexical_analyzer input.c
   Output: one "lexeme,TOKEN" line per token on stdout.
   Diagnostics go to stderr. */
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#define MAX_LEXEME 129 /* lexemes are at most 128 chars, plus '\0' */

static int is_keyword(const char *s) {
    static const char *keywords[] = {"int", "if", "else", "while", "return"};
    for (size_t i = 0; i < sizeof keywords / sizeof keywords[0]; i++) {
        if (strcmp(s, keywords[i]) == 0) return 1;
    }
    return 0;
}

static int is_ident_start(int c) {
    return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || c == '_';
}

static int is_digit(int c) { return c >= '0' && c <= '9'; }

static int is_ident_part(int c) { return is_ident_start(c) || is_digit(c); }

static int is_space(int c) {
    return c == ' ' || c == '\t' || c == '\r' || c == '\n' ||
           c == '\v' || c == '\f';
}

static void emit(const char *lexeme, const char *token) {
    printf("%s,%s\n", lexeme, token);
}

int main(int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr, "usage: lexical_analyzer input.c\n");
        return 2;
    }
    FILE *input = fopen(argv[1], "rb");
    if (input == NULL) {
        perror(argv[1]);
        return 2;
    }

    int status = 0;
    int c = fgetc(input);

    while (c != EOF) {
        /* 1. whitespace */
        if (is_space(c)) {
            c = fgetc(input);
            continue;
        }

        /* 2. identifier or keyword */
        if (is_ident_start(c)) {
            char buf[MAX_LEXEME];
            size_t n = 0;
            while (c != EOF && is_ident_part(c)) {
                if (n < MAX_LEXEME - 1) buf[n++] = (char)c;
                c = fgetc(input);
            }
            buf[n] = '\0';
            emit(buf, is_keyword(buf) ? "KEYWORD" : "IDENTIFIER");
            continue; /* c already holds the next unread character */
        }

        /* 3. integer */
        if (is_digit(c)) {
            char buf[MAX_LEXEME];
            size_t n = 0;
            while (c != EOF && is_digit(c)) {
                if (n < MAX_LEXEME - 1) buf[n++] = (char)c;
                c = fgetc(input);
            }
            buf[n] = '\0';
            emit(buf, "INTEGER");
            continue;
        }

        /* 4. slash: line comment, block comment, or division */
        if (c == '/') {
            int next = fgetc(input);
            if (next == '/') {
                /* skip through newline or EOF */
                c = fgetc(input);
                while (c != EOF && c != '\n') c = fgetc(input);
                continue; /* newline (or EOF) is handled by the loop */
            } else if (next == '*') {
                int prev = 0;
                int cur = fgetc(input);
                int closed = 0;
                while (cur != EOF) {
                    if (prev == '*' && cur == '/') {
                        closed = 1;
                        break;
                    }
                    prev = cur;
                    cur = fgetc(input);
                }
                if (!closed) {
                    fprintf(stderr, "error: unterminated block comment\n");
                    status = 1;
                    break;
                }
                c = fgetc(input);
                continue;
            } else {
                emit("/", "ARITHMETIC_OPERATOR");
                c = next; /* look-ahead belongs to the next token */
                continue;
            }
        }

        /* 5. operators and delimiters (two-character forms first) */
        {
            int cur = c;
            int next = fgetc(input);

            if (cur == '=' && next == '=') {
                emit("==", "RELATIONAL_OPERATOR");
                c = fgetc(input);
            } else if (cur == '!' && next == '=') {
                emit("!=", "RELATIONAL_OPERATOR");
                c = fgetc(input);
            } else if (cur == '<' && next == '=') {
                emit("<=", "RELATIONAL_OPERATOR");
                c = fgetc(input);
            } else if (cur == '>' && next == '=') {
                emit(">=", "RELATIONAL_OPERATOR");
                c = fgetc(input);
            } else if (cur == '&' && next == '&') {
                emit("&&", "LOGICAL_OPERATOR");
                c = fgetc(input);
            } else if (cur == '|' && next == '|') {
                emit("||", "LOGICAL_OPERATOR");
                c = fgetc(input);
            } else {
                /* single-character token; next is look-ahead for later */
                char lex[2] = {(char)cur, '\0'};
                const char *tok;
                switch (cur) {
                    case '=': tok = "ASSIGNMENT_OPERATOR"; break;
                    case '+': case '-': case '*': case '%':
                        tok = "ARITHMETIC_OPERATOR"; break;
                    case '<': case '>':
                        tok = "RELATIONAL_OPERATOR"; break;
                    case '!':
                        tok = "LOGICAL_OPERATOR"; break;
                    case '(': case ')': case '{': case '}':
                    case '[': case ']': case ';': case ',':
                        tok = "DELIMITER"; break;
                    default:
                        tok = "UNKNOWN"; break;
                }
                emit(lex, tok);
                c = next;
            }
        }
    }

    fclose(input);
    return status;
}
