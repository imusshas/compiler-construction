/* Lab 02: Lexical Analyzer
   Reads the input filename from argv[1].
   Prints one lexeme,TOKEN line per token.
*/

#include <stdio.h>
#include <ctype.h>
#include <string.h>

#define MAX_TOKEN 4096

int is_letter(int c) {
    return (c >= 'A' && c <= 'Z') ||
           (c >= 'a' && c <= 'z') || c == '_';
}

int is_digit(int c) {
    return c >= '0' && c <= '9';
}

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

        /* Skip whitespace */
        if (isspace((unsigned char)c))
            continue;

        char token[MAX_TOKEN];
        int len = 0;
        token[len++] = (char)c;
        token[len] = '\0';

        /* Identifier or keyword */
        if (is_letter(c)) {
            while ((c = fgetc(input)) != EOF &&
                   (is_letter(c) || is_digit(c))) {
                if (len < MAX_TOKEN - 1)
                    token[len++] = (char)c;
            }

            if (c != EOF)
                ungetc(c, input);

            token[len] = '\0';

            if (is_keyword(token))
                print_token(token, "KEYWORD");
            else
                print_token(token, "IDENTIFIER");

            continue;
        }

        /* Integer */
        if (is_digit(c)) {
            while ((c = fgetc(input)) != EOF && is_digit(c)) {
                if (len < MAX_TOKEN - 1)
                    token[len++] = (char)c;
            }

            if (c != EOF)
                ungetc(c, input);

            token[len] = '\0';
            print_token(token, "INTEGER");
            continue;
        }

        /* Comments and division operator */
        if (c == '/') {
            int next = fgetc(input);

            if (next == '/') {
                /* Skip single-line comment */
                while ((c = fgetc(input)) != EOF && c != '\n')
                    ;
                continue;
            }

            if (next == '*') {
                /* Skip block comment */
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

            if (next != EOF)
                ungetc(next, input);

            print_token("/", "ARITHMETIC_OPERATOR");
            continue;
        }

        /* Assignment and relational operators */
        if (c == '=' || c == '!' || c == '<' || c == '>') {
            int next = fgetc(input);

            if (next == '=') {
                token[len++] = '=';
                token[len] = '\0';

                if (c == '=')
                    print_token(token, "RELATIONAL_OPERATOR");
                else if (c == '!')
                    print_token(token, "RELATIONAL_OPERATOR");
                else
                    print_token(token, "RELATIONAL_OPERATOR");
            } else {
                if (next != EOF)
                    ungetc(next, input);

                if (c == '=')
                    print_token(token, "ASSIGNMENT_OPERATOR");
                else if (c == '!')
                    print_token(token, "LOGICAL_OPERATOR");
                else
                    print_token(token, "RELATIONAL_OPERATOR");
            }
            continue;
        }

        /* Logical operators */
        if (c == '&' || c == '|') {
            int next = fgetc(input);

            if (next == c) {
                token[len++] = (char)next;
                token[len] = '\0';

                if (c == '&')
                    print_token(token, "LOGICAL_OPERATOR");
                else
                    print_token(token, "LOGICAL_OPERATOR");
            } else {
                if (next != EOF)
                    ungetc(next, input);

                print_token(token, "UNKNOWN");
            }
            continue;
        }

        /* Arithmetic operators */
        if (c == '+' || c == '-' || c == '*' || c == '%') {
            print_token(token, "ARITHMETIC_OPERATOR");
            continue;
        }

        /* Delimiters */
        if (strchr("(){}[];,", c) != NULL) {
            print_token(token, "DELIMITER");
            continue;
        }

        /* Any other character */
        print_token(token, "UNKNOWN");
    }

    fclose(input);
    return 0;
}