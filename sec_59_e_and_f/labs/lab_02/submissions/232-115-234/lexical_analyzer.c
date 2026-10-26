/* Lab 02: implement the section manual's lexical contract.
   Read argv[1] with fopen/fgetc. Do not hard-code public.c or its token list.
   You may add helper functions. This starter is deliberately unfinished. */

/* Lab 02: Lexical Analyzer */
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#define MAX_TOKEN 129

int is_keyword(const char *word) {
    const char *keywords[] = {
        "int", "if", "else", "while", "return"
    };

    int i;
    for (i = 0; i < 5; i++) {
        if (strcmp(word, keywords[i]) == 0) {
            return 1;
        }
    }
    return 0;
}

int is_letter(int ch) {
    return (ch >= 'A' && ch <= 'Z') ||
           (ch >= 'a' && ch <= 'z') || ch == '_';
}

int is_digit(int ch) {
    return ch >= '0' && ch <= '9';
}

void print_token(const char *lexeme, const char *type) {
    printf("%s,%s\n", lexeme, type);
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

    int ch;
    int next;
    int status = 0;
    char token[MAX_TOKEN];

    while ((ch = fgetc(input)) != EOF) {
        if (isspace((unsigned char)ch)) {
            continue;
        }

        /* Identifiers and keywords */
        if (is_letter(ch)) {
            int len = 0;
            token[len++] = (char)ch;

            while ((next = fgetc(input)) != EOF &&
                   (is_letter(next) || is_digit(next))) {
                if (len < MAX_TOKEN - 1) {
                    token[len++] = (char)next;
                }
            }

            if (next != EOF) {
                ungetc(next, input);
            }

            token[len] = '\0';

            if (is_keyword(token)) {
                print_token(token, "KEYWORD");
            } else {
                print_token(token, "IDENTIFIER");
            }
            continue;
        }

        /* Integers */
        if (is_digit(ch)) {
            int len = 0;
            token[len++] = (char)ch;

            while ((next = fgetc(input)) != EOF && is_digit(next)) {
                if (len < MAX_TOKEN - 1) {
                    token[len++] = (char)next;
                }
            }

            if (next != EOF) {
                ungetc(next, input);
            }

            token[len] = '\0';
            print_token(token, "INTEGER");
            continue;
        }

        /* Comments and division */
        if (ch == '/') {
            next = fgetc(input);

            if (next == '/') {
                while ((ch = fgetc(input)) != EOF && ch != '\n') {
                    /* Skip line comment */
                }
                continue;
            }

            if (next == '*') {
                int previous = 0;
                int closed = 0;

                while ((ch = fgetc(input)) != EOF) {
                    if (previous == '*' && ch == '/') {
                        closed = 1;
                        break;
                    }
                    previous = ch;
                }

                if (!closed) {
                    fprintf(stderr, "error: unclosed block comment\n");
                    status = 1;
                    break;
                }
                continue;
            }

            if (next != EOF) {
                ungetc(next, input);
            }

            print_token("/", "ARITHMETIC_OPERATOR");
            continue;
        }

        /* Two-character operators */
        if (ch == '=' || ch == '!' || ch == '<' || ch == '>') {
            next = fgetc(input);

            if (next == '=') {
                token[0] = (char)ch;
                token[1] = '=';
                token[2] = '\0';
                print_token(token, "RELATIONAL_OPERATOR");
            } else {
                if (next != EOF) {
                    ungetc(next, input);
                }

                token[0] = (char)ch;
                token[1] = '\0';

                if (ch == '=') {
                    print_token(token, "ASSIGNMENT_OPERATOR");
                } else if (ch == '!') {
                    print_token(token, "LOGICAL_OPERATOR");
                } else {
                    print_token(token, "RELATIONAL_OPERATOR");
                }
            }
            continue;
        }

        if (ch == '&' || ch == '|') {
            next = fgetc(input);

            if (next == ch) {
                token[0] = (char)ch;
                token[1] = (char)ch;
                token[2] = '\0';
                print_token(token, "LOGICAL_OPERATOR");
            } else {
                if (next != EOF) {
                    ungetc(next, input);
                }

                token[0] = (char)ch;
                token[1] = '\0';
                print_token(token, "UNKNOWN");
            }
            continue;
        }

        /* Single-character arithmetic operators */
        if (ch == '+' || ch == '-' || ch == '*' || ch == '%') {
            token[0] = (char)ch;
            token[1] = '\0';
            print_token(token, "ARITHMETIC_OPERATOR");
            continue;
        }

        /* Delimiters */
        if (ch == '(' || ch == ')' ||
            ch == '{' || ch == '}' ||
            ch == '[' || ch == ']' ||
            ch == ';' || ch == ',') {
            token[0] = (char)ch;
            token[1] = '\0';
            print_token(token, "DELIMITER");
            continue;
        }

        /* Unsupported character */
        token[0] = (char)ch;
        token[1] = '\0';
        print_token(token, "UNKNOWN");
    }

    if (ferror(input)) {
        perror("error reading input");
        status = 1;
    }

    fclose(input);
    return status;
}
