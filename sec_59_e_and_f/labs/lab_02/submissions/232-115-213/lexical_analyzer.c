/* Lab 02: implement the section manual's lexical contract.
   Read argv[1] with fopen/fgetc. Do not hard-code public.c or its token list.
   You may add helper functions. This starter is deliberately unfinished. */
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

int is_keyword(const char *str) {
    if (strcmp(str, "int") == 0 ||
        strcmp(str, "if") == 0 ||
        strcmp(str, "else") == 0 ||
        strcmp(str, "while") == 0 ||
        strcmp(str, "return") == 0) {
        return 1;
    }
    return 0;
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Error: Missing input file argument.\n");
        return 1;
    }

    FILE *file = fopen(argv[1], "r");
    if (!file) {
        fprintf(stderr, "Error: Could not open file %s\n", argv[1]);
        return 1;
    }

    int c;
    while ((c = fgetc(file)) != EOF) {
        if (isspace(c)) {
            continue;
        }

        if (isalpha(c) || c == '_') {
            char buf[129];
            int len = 0;
            buf[len++] = (char)c;
            while ((c = fgetc(file)) != EOF && (isalnum(c) || c == '_')) {
                if (len < 128) {
                    buf[len++] = (char)c;
                }
            }
            buf[len] = '\0';
            if (c != EOF) {
                ungetc(c, file);
            }

            if (is_keyword(buf)) {
                printf("%s, KEYWORD\n", buf);
            } else {
                printf("%s, IDENTIFIER\n", buf);
            }
            continue;
        }

        if (isdigit(c)) {
            char buf[129];
            int len = 0;
            buf[len++] = (char)c;
            while ((c = fgetc(file)) != EOF && isdigit(c)) {
                if (len < 128) {
                    buf[len++] = (char)c;
                }
            }
            buf[len] = '\0';
            if (c != EOF) {
                ungetc(c, file);
            }

            printf("%s, INTEGER\n", buf);
            continue;
        }

        if (c == '/') {
            int next = fgetc(file);
            if (next == '/') {
                while ((c = fgetc(file)) != EOF && c != '\n') {
                }
                continue;
            } else if (next == '*') {
                int prev = 0;
                int closed = 0;
                while ((c = fgetc(file)) != EOF) {
                    if (prev == '*' && c == '/') {
                        closed = 1;
                        break;
                    }
                    prev = c;
                }
                if (!closed) {
                    fprintf(stderr, "Error: Unclosed block comment.\n");
                    fclose(file);
                    return 1;
                }
                continue;
            } else {
                if (next != EOF) {
                    ungetc(next, file);
                }
                printf("/, ARITHMETIC_OPERATOR\n");
                continue;
            }
        }

        if (c == '=' || c == '!' || c == '<' || c == '>') {
            int next = fgetc(file);
            if (next == '=') {
                if (c == '=') printf("==, RELATIONAL_OPERATOR\n");
                else if (c == '!') printf("!=, RELATIONAL_OPERATOR\n");
                else if (c == '<') printf("<=, RELATIONAL_OPERATOR\n");
                else if (c == '>') printf(">=, RELATIONAL_OPERATOR\n");
            } else {
                if (next != EOF) {
                    ungetc(next, file);
                }
                if (c == '=') printf("=, ASSIGNMENT_OPERATOR\n");
                else if (c == '!') printf("!, LOGICAL_OPERATOR\n");
                else if (c == '<') printf("<, RELATIONAL_OPERATOR\n");
                else if (c == '>') printf(">, RELATIONAL_OPERATOR\n");
            }
            continue;
        }

        if (c == '&') {
            int next = fgetc(file);
            if (next == '&') {
                printf("&&, LOGICAL_OPERATOR\n");
            } else {
                if (next != EOF) ungetc(next, file);
                printf("&, UNKNOWN\n");
            }
            continue;
        }

        if (c == '|') {
            int next = fgetc(file);
            if (next == '|') {
                printf("||, LOGICAL_OPERATOR\n");
            } else {
                if (next != EOF) ungetc(next, file);
                printf("|, UNKNOWN\n");
            }
            continue;
        }

        if (c == '+' || c == '-' || c == '*') {
            printf("%c, ARITHMETIC_OPERATOR\n", c);
            continue;
        }

        if (strchr("(),;{}[].:", c) != NULL) {
            printf("%c, DELIMITER\n", c);
            continue;
        }

        printf("%c, UNKNOWN\n", c);
    }

    fclose(file);
    return 0;
}
