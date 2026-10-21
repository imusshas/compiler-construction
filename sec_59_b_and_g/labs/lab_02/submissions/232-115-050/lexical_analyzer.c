#include <stdio.h>
#include <string.h>
#include <ctype.h>

#define BUF_SIZE 256

/* Returns 1 if the word is one of the 5 keywords, otherwise 0. */
static int is_keyword(const char *s) {
    const char *keywords[] = {"int", "if", "else", "while", "return"};
    for (size_t i = 0; i < sizeof(keywords) / sizeof(keywords[0]); i++) {
        if (strcmp(s, keywords[i]) == 0) {
            return 1;
        }
    }
    return 0;
}

/* Put a look-ahead character back, unless it was EOF. */
static void put_back(int c, FILE *fp) {
    if (c != EOF) {
        ungetc(c, fp);
    }
}

int main(int argc, char *argv[]) {
    /* 1. Check the command-line argument. */
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <input_file>\n", argv[0]);
        return 1;
    }

    /* 2. Open the file. */
    FILE *fp = fopen(argv[1], "rb");
    if (fp == NULL) {
        fprintf(stderr, "Error: cannot open file '%s'\n", argv[1]);
        return 1;
    }

    int c; /* int, not char, so we can tell EOF apart from a real character */

    /* 3. Read one character at a time until the end of the file. */
    while ((c = fgetc(fp)) != EOF) {

        /* Skip whitespace. */
        if (c == ' ' || c == '\t' || c == '\r' || c == '\n') {
            continue;
        }

        /* Identifier or keyword: starts with a letter or underscore. */
        if (isalpha(c) || c == '_') {
            char buf[BUF_SIZE];
            int len = 0;
            buf[len++] = (char)c;
            while ((c = fgetc(fp)) != EOF && (isalnum(c) || c == '_')) {
                if (len < BUF_SIZE - 1) {
                    buf[len++] = (char)c;
                }
            }
            buf[len] = '\0';
            put_back(c, fp); /* c belongs to the next token */
            printf("%s,%s\n", buf, is_keyword(buf) ? "KEYWORD" : "IDENTIFIER");
        }

        /* Integer: one or more digits (leading zeros are kept). */
        else if (isdigit(c)) {
            char buf[BUF_SIZE];
            int len = 0;
            buf[len++] = (char)c;
            while ((c = fgetc(fp)) != EOF && isdigit(c)) {
                if (len < BUF_SIZE - 1) {
                    buf[len++] = (char)c;
                }
            }
            buf[len] = '\0';
            put_back(c, fp);
            printf("%s,INTEGER\n", buf);
        }

        /* Slash: line comment, block comment, or division. */
        else if (c == '/') {
            int next = fgetc(fp);
            if (next == '/') {
                /* Line comment: skip until newline or EOF. */
                while ((c = fgetc(fp)) != EOF && c != '\n') {
                }
            } else if (next == '*') {
                /* Block comment: skip until the closing star-slash. */
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
            } else {
                /* Just division. */
                put_back(next, fp);
                printf("/,ARITHMETIC_OPERATOR\n");
            }
        }

        /* = == ! != < <= > >= : check for a following '=' first. */
        else if (c == '=' || c == '!' || c == '<' || c == '>') {
            int next = fgetc(fp);
            if (next == '=') {
                printf("%c=,RELATIONAL_OPERATOR\n", c);
            } else {
                put_back(next, fp);
                if (c == '=') {
                    printf("=,ASSIGNMENT_OPERATOR\n");
                } else if (c == '!') {
                    printf("!,LOGICAL_OPERATOR\n");
                } else {
                    printf("%c,RELATIONAL_OPERATOR\n", c);
                }
            }
        }

        /* && (a lone & is UNKNOWN) */
        else if (c == '&') {
            int next = fgetc(fp);
            if (next == '&') {
                printf("&&,LOGICAL_OPERATOR\n");
            } else {
                put_back(next, fp);
                printf("&,UNKNOWN\n");
            }
        }

        /* || (a lone | is UNKNOWN) */
        else if (c == '|') {
            int next = fgetc(fp);
            if (next == '|') {
                printf("||,LOGICAL_OPERATOR\n");
            } else {
                put_back(next, fp);
                printf("|,UNKNOWN\n");
            }
        }

        /* Single-character arithmetic operators. */
        else if (c == '+' || c == '-' || c == '*' || c == '%') {
            printf("%c,ARITHMETIC_OPERATOR\n", c);
        }

        /* Delimiters. */
        else if (c == '(' || c == ')' || c == '{' || c == '}' ||
                 c == '[' || c == ']' || c == ';' || c == ',') {
            printf("%c,DELIMITER\n", c);
        }

        /* Anything else. */
        else {
            printf("%c,UNKNOWN\n", c);
        }
    }

    fclose(fp);
    return 0;
}
