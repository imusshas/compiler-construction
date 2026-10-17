
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_LEXEME 129

int is_keyword(const char *s) {
    const char *words[] = {"int", "if", "else", "while", "return"};
    for (int i = 0; i < 5; i++)
        if (strcmp(s, words[i]) == 0) return 1;
    return 0;
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <source-file>\n", argv[0]);
        return 1;
    }

    FILE *f = fopen(argv[1], "r");
    if (!f) {
        fprintf(stderr, "Error: cannot open input file\n");
        return 1;
    }

    int c, d;
    char s[MAX_LEXEME];

    while ((c = fgetc(f)) != EOF) {
        if (isspace((unsigned char)c)) continue;

        if (c == '/') {
            d = fgetc(f);
            if (d == '/') {
                while ((c = fgetc(f)) != EOF && c != '\n');
                continue;
            }
            if (d == '*') {
                int prev = 0, closed = 0;
                while ((c = fgetc(f)) != EOF) {
                    if (prev == '*' && c == '/') {
                        closed = 1;
                        break;
                    }
                    prev = c;
                }
                if (!closed) {
                    fprintf(stderr, "Error: unclosed block comment\n");
                    fclose(f);
                    return 1;
                }
                continue;
            }
            if (d != EOF) ungetc(d, f);
            puts("/,ARITHMETIC_OPERATOR");
            continue;
        }

        if (isalpha((unsigned char)c) || c == '_') {
            int i = 0;
            s[i++] = (char)c;
            while ((c = fgetc(f)) != EOF &&
                   (isalnum((unsigned char)c) || c == '_')) {
                if (i >= 128) {
                    fprintf(stderr, "Error: identifier too long\n");
                    fclose(f);
                    return 1;
                }
                s[i++] = (char)c;
            }
            s[i] = '\0';
            if (c != EOF) ungetc(c, f);
            printf("%s,%s\n", s, is_keyword(s) ? "KEYWORD" : "IDENTIFIER");
            continue;
        }

        if (isdigit((unsigned char)c)) {
            int i = 0;
            s[i++] = (char)c;
            while ((c = fgetc(f)) != EOF && isdigit((unsigned char)c)) {
                if (i >= 128) {
                    fprintf(stderr, "Error: integer too long\n");
                    fclose(f);
                    return 1;
                }
                s[i++] = (char)c;
            }
            s[i] = '\0';
            if (c != EOF) ungetc(c, f);
            printf("%s,INTEGER\n", s);
            continue;
        }

        if (c == '=' || c == '!' || c == '<' || c == '>') {
            d = fgetc(f);
            if (d == '=') {
                printf("%c=,%s\n", c,
                    c == '=' || c == '!' || c == '<' || c == '>'
                    ? "RELATIONAL_OPERATOR" : "UNKNOWN");
            } else {
                if (d != EOF) ungetc(d, f);
                if (c == '=') puts("=,ASSIGNMENT_OPERATOR");
                else if (c == '!') puts("!,LOGICAL_OPERATOR");
                else printf("%c,RELATIONAL_OPERATOR\n", c);
            }
            continue;
        }

        if (c == '&' || c == '|') {
            d = fgetc(f);
            if (d == c) printf("%c%c,LOGICAL_OPERATOR\n", c, d);
            else {
                if (d != EOF) ungetc(d, f);
                printf("%c,UNKNOWN\n", c);
            }
            continue;
        }

        if (strchr("+-*%", c)) {
            printf("%c,ARITHMETIC_OPERATOR\n", c);
            continue;
        }

        if (strchr("(){}[];,", c)) {
            printf("%c,DELIMITER\n", c);
            continue;
        }

        printf("%c,UNKNOWN\n", c);
    }

    fclose(f);
    return 0;
}
