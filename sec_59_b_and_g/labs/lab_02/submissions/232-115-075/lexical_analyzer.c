/* Lab 02: implement the section manual's lexical contract.
   Read argv[1] with fopen/fgetc. Do not hard-code public.c or its token list.
   You may add helper functions. This starter is deliberately unfinished. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_LEXEME 256

static int is_keyword(const char *s) {
    static const char *keywords[] = {"int", "if", "else", "while", "return"};
    size_t i;
    for (i = 0; i < sizeof(keywords) / sizeof(keywords[0]); i++) {
        if (strcmp(s, keywords[i]) == 0) {
            return 1;
        }
    }
    return 0;
}

static void emit(const char *lexeme, const char *token) {
    printf("%s,%s\n", lexeme, token);
}

int main(int argc, char *argv[]) {
    FILE *fp;
    int c;
    int next;

    if (argc != 2) {
        fprintf(stderr, "Usage: %s <input_file>\n", argv[0]);
        return 1;
    }

    fp = fopen(argv[1], "r");
    if (fp == NULL) {
        fprintf(stderr, "Error: cannot open file '%s'\n", argv[1]);
        return 1;
    }

    while ((c = fgetc(fp)) != EOF) {
        /* Skip whitespace */
        if (c == ' ' || c == '\t' || c == '\r' || c == '\n') {
            continue;
        }

        /* Identifier or keyword */
        if (isalpha(c) || c == '_') {
            char buf[MAX_LEXEME];
            size_t len = 0;
            buf[len++] = (char)c;
            while ((next = fgetc(fp)) != EOF && (isalnum(next) || next == '_')) {
                if (len < MAX_LEXEME - 1) {
                    buf[len++] = (char)next;
                }
            }
            buf[len] = '\0';
            if (next != EOF) {
                ungetc(next, fp);
            }
            emit(buf, is_keyword(buf) ? "KEYWORD" : "IDENTIFIER");
            continue;
        }

        /* Integer */
        if (isdigit(c)) {
            char buf[MAX_LEXEME];
            size_t len = 0;
            buf[len++] = (char)c;
            while ((next = fgetc(fp)) != EOF && isdigit(next)) {
                if (len < MAX_LEXEME - 1) {
                    buf[len++] = (char)next;
                }
            }
            buf[len] = '\0';
            if (next != EOF) {
                ungetc(next, fp);
            }
            emit(buf, "INTEGER");
            continue;
        }

        /* Slash: line comment, block comment, or division */
        if (c == '/') {
            next = fgetc(fp);
            if (next == '/') {
                while ((c = fgetc(fp)) != EOF && c != '\n') {
                    /* skip to end of line */
                }
                continue;
            } else if (next == '*') {
                int prev = 0;
                int closed = 0;
                while ((c = fgetc(fp)) != EOF) {
                    if (prev == '*' && c == '/') {
                        closed = 1;
                        break;
                    }
                    prev = c;
                }
                if (!closed) {
                    fprintf(stderr, "Error: unclosed block comment\n");
                    fclose(fp);
                    return 1;
                }
                continue;
            } else {
                if (next != EOF) {
                    ungetc(next, fp);
                }
                emit("/", "ARITHMETIC_OPERATOR");
                continue;
            }
        }

        /* Operators and delimiters */
        switch (c) {
        case '=':
            next = fgetc(fp);
            if (next == '=') {
                emit("==", "RELATIONAL_OPERATOR");
            } else {
                if (next != EOF) ungetc(next, fp);
                emit("=", "ASSIGNMENT_OPERATOR");
            }
            break;
        case '!':
            next = fgetc(fp);
            if (next == '=') {
                emit("!=", "RELATIONAL_OPERATOR");
            } else {
                if (next != EOF) ungetc(next, fp);
                emit("!", "LOGICAL_OPERATOR");
            }
            break;
        case '<':
            next = fgetc(fp);
            if (next == '=') {
                emit("<=", "RELATIONAL_OPERATOR");
            } else {
                if (next != EOF) ungetc(next, fp);
                emit("<", "RELATIONAL_OPERATOR");
            }
            break;
        case '>':
            next = fgetc(fp);
            if (next == '=') {
                emit(">=", "RELATIONAL_OPERATOR");
            } else {
                if (next != EOF) ungetc(next, fp);
                emit(">", "RELATIONAL_OPERATOR");
            }
            break;
        case '&':
            next = fgetc(fp);
            if (next == '&') {
                emit("&&", "LOGICAL_OPERATOR");
            } else {
                if (next != EOF) ungetc(next, fp);
                emit("&", "UNKNOWN");
            }
            break;
        case '|':
            next = fgetc(fp);
            if (next == '|') {
                emit("||", "LOGICAL_OPERATOR");
            } else {
                if (next != EOF) ungetc(next, fp);
                emit("|", "UNKNOWN");
            }
            break;
        case '+':
            emit("+", "ARITHMETIC_OPERATOR");
            break;
        case '-':
            emit("-", "ARITHMETIC_OPERATOR");
            break;
        case '*':
            emit("*", "ARITHMETIC_OPERATOR");
            break;
        case '%':
            emit("%", "ARITHMETIC_OPERATOR");
            break;
        case '(':
            emit("(", "DELIMITER");
            break;
        case ')':
            emit(")", "DELIMITER");
            break;
        case '{':
            emit("{", "DELIMITER");
            break;
        case '}':
            emit("}", "DELIMITER");
            break;
        case '[':
            emit("[", "DELIMITER");
            break;
        case ']':
            emit("]", "DELIMITER");
            break;
        case ';':
            emit(";", "DELIMITER");
            break;
        case ',':
            emit(",", "DELIMITER");
            break;
        default: {
            char buf[2];
            buf[0] = (char)c;
            buf[1] = '\0';
            emit(buf, "UNKNOWN");
            break;
        }
        }
    }

    fclose(fp);
    return 0;
}
