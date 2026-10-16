/* Lab 02 lexical analyzer */
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#define MAX_LEXEME 4096

static const char *keywords[] = {
    "int", "if", "else", "while", "return"
};

static int is_keyword(const char *s) {
    size_t n = sizeof(keywords) / sizeof(keywords[0]);
    for (size_t i = 0; i < n; i++) {
        if (strcmp(s, keywords[i]) == 0) return 1;
    }
    return 0;
}

static void emit(const char *s, const char *type) {
    printf("%s,%s\n", s, type);
}

int main(int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr, "usage: lexical_analyzer input.c\n");
        return 2;
    }

    FILE *input = fopen(argv[1], "rb");
    if (input == NULL) {
        perror("input");
        return 2;
    }

    int c;
    while ((c = fgetc(input)) != EOF) {
        if (isspace((unsigned char)c)) continue;

        if (c == '/') {
            int next = fgetc(input);
            if (next == '/') {
                while ((c = fgetc(input)) != EOF && c != '\n') {}
                continue;
            }
            if (next == '*') {
                int prev = 0, closed = 0;
                while ((c = fgetc(input)) != EOF) {
                    if (prev == '*' && c == '/') {
                        closed = 1;
                        break;
                    }
                    prev = c;
                }
                if (!closed) {
                    fprintf(stderr, "error: unclosed block comment\n");
                    fclose(input);
                    return 2;
                }
                continue;
            }
            if (next != EOF) ungetc(next, input);
            emit("/", "ARITHMETIC_OPERATOR");
            continue;
        }

        if (isalpha((unsigned char)c) || c == '_') {
            char s[MAX_LEXEME];
            size_t n = 0;
            s[n++] = (char)c;
            while ((c = fgetc(input)) != EOF &&
                   (isalnum((unsigned char)c) || c == '_')) {
                if (n + 1 < sizeof(s)) s[n++] = (char)c;
            }
            s[n] = '\0';
            if (c != EOF) ungetc(c, input);
            emit(s, is_keyword(s) ? "KEYWORD" : "IDENTIFIER");
            continue;
        }

        if (isdigit((unsigned char)c)) {
            char s[MAX_LEXEME];
            size_t n = 0;
            s[n++] = (char)c;
            while ((c = fgetc(input)) != EOF &&
                   isdigit((unsigned char)c)) {
                if (n + 1 < sizeof(s)) s[n++] = (char)c;
            }
            s[n] = '\0';
            if (c != EOF) ungetc(c, input);
            emit(s, "INTEGER");
            continue;
        }

        if (strchr("(){}[];,", c) != NULL) {
            char s[] = {(char)c, '\0'};
            emit(s, "DELIMITER");
            continue;
        }

        if (strchr("+-*%=", c) != NULL ||
            strchr("<>!&|", c) != NULL) {
            char s[3] = {(char)c, '\0', '\0'};
            int next = fgetc(input);

            if ((c == '=' && next == '=') ||
                (c == '!' && next == '=') ||
               (c == '<' && (next == '=' || next == '<')) ||
(c == '>' && (next == '=' || next == '>')) ||
                (c == '&' && next == '&') ||
                (c == '|' && next == '|')) {
                s[1] = (char)next;
            } else if (next != EOF) {
                ungetc(next, input);
            }

            if (strcmp(s, "=") == 0) emit(s, "ASSIGNMENT_OPERATOR");
            else if (strcmp(s, "==") == 0 || strcmp(s, "!=") == 0 ||
                     strcmp(s, "<") == 0 || strcmp(s, "<=") == 0 ||
                     strcmp(s, ">") == 0 || strcmp(s, ">=") == 0) {
                emit(s, "RELATIONAL_OPERATOR");
            } else if (strcmp(s, "&&") == 0 || strcmp(s, "||") == 0 ||
                       strcmp(s, "!") == 0) {
                emit(s, "LOGICAL_OPERATOR");
            } else if (strchr("+-*%", c) != NULL) {
                emit(s, "ARITHMETIC_OPERATOR");
            } else {
                emit(s, "UNKNOWN");
            }
            continue;
        }

        {
            char s[] = {(char)c, '\0'};
            emit(s, "UNKNOWN");
        }
    }

    if (ferror(input)) {
        perror("input");
        fclose(input);
        return 2;
    }

    fclose(input);
    return 0;
}