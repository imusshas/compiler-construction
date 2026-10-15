
#include <stdio.h>
#include <ctype.h>
#include <string.h>

#define MAX_LEXEME 128

int is_keyword(const char *s) {
    const char *keywords[] = {
        "int", "if", "else", "while", "return"
    };

    for (int i = 0; i < 5; i++) {
        if (strcmp(s, keywords[i]) == 0)
            return 1;
    }
    return 0;
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

    int c;

    while ((c = fgetc(input)) != EOF) {
        if (isspace((unsigned char)c))
            continue;

        char lexeme[MAX_LEXEME + 1];
        int n = 0;
        int d;

        /* Identifier or keyword */
        if (isalpha((unsigned char)c) || c == '_') {
            do {
                lexeme[n++] = (char)c;
                c = fgetc(input);
            } while (c != EOF &&
                     (isalnum((unsigned char)c) || c == '_'));

            lexeme[n] = '\0';

            if (c != EOF)
                ungetc(c, input);

            print_token(lexeme,
                        is_keyword(lexeme) ? "KEYWORD" : "IDENTIFIER");
            continue;
        }

        /* Integer */
        if (isdigit((unsigned char)c)) {
            do {
                lexeme[n++] = (char)c;
                c = fgetc(input);
            } while (c != EOF && isdigit((unsigned char)c));

            lexeme[n] = '\0';

            if (c != EOF)
                ungetc(c, input);

            print_token(lexeme, "INTEGER");
            continue;
        }

        /* Slash, line comment, or block comment */
        if (c == '/') {
            d = fgetc(input);

            if (d == '/') {
                while ((c = fgetc(input)) != EOF && c != '\n')
                    ;
                continue;
            }

            if (d == '*') {
                int previous = 0;
                int closed = 0;

                while ((c = fgetc(input)) != EOF) {
                    if (previous == '*' && c == '/') {
                        closed = 1;
                        break;
                    }
                    previous = c;
                }

                if (!closed) {
                    fprintf(stderr, "Error: unclosed block comment\n");
                    fclose(input);
                    return 1;
                }
                continue;
            }

            if (d != EOF)
                ungetc(d, input);

            print_token("/", "ARITHMETIC_OPERATOR");
            continue;
        }

        /* Two-character operators: longest match first */
        if (c == '=' || c == '!' || c == '<' || c == '>') {
            d = fgetc(input);

            if (d == '=') {
                lexeme[0] = (char)c;
                lexeme[1] = '=';
                lexeme[2] = '\0';
                print_token(lexeme, "RELATIONAL_OPERATOR");
            } else {
                if (d != EOF)
                    ungetc(d, input);

                lexeme[0] = (char)c;
                lexeme[1] = '\0';

                if (c == '=') {
                    print_token(lexeme, "ASSIGNMENT_OPERATOR");
                } else if (c == '!') {
                    print_token(lexeme, "LOGICAL_OPERATOR");
                } else {
                    print_token(lexeme, "RELATIONAL_OPERATOR");
                }
            }
            continue;
        }

        if (c == '&' || c == '|') {
            d = fgetc(input);

            if (d == c) {
                lexeme[0] = (char)c;
                lexeme[1] = (char)d;
                lexeme[2] = '\0';
                print_token(lexeme, "LOGICAL_OPERATOR");
            } else {
                if (d != EOF)
                    ungetc(d, input);

                lexeme[0] = (char)c;
                lexeme[1] = '\0';
                print_token(lexeme, "UNKNOWN");
            }
            continue;
        }

        /* Single-character operators */
        lexeme[0] = (char)c;
        lexeme[1] = '\0';

        if (c == '+' || c == '-' || c == '*' || c == '%') {
            print_token(lexeme, "ARITHMETIC_OPERATOR");
        } else if (c == '(' || c == ')' ||
                   c == '{' || c == '}' ||
                   c == '[' || c == ']' ||
                   c == ';' || c == ',') {
            print_token(lexeme, "DELIMITER");
        } else {
            print_token(lexeme, "UNKNOWN");
        }
    }

    fclose(input);
    return 0;
}
