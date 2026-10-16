#include <stdio.h>
#include <ctype.h>
#include <string.h>

int is_keyword(const char *word) {
    return strcmp(word, "int") == 0 ||
           strcmp(word, "if") == 0 ||
           strcmp(word, "else") == 0 ||
           strcmp(word, "while") == 0 ||
           strcmp(word, "return") == 0;
}

void print_token(const char *lexeme, const char *token) {
    printf("%s,%s\n", lexeme, token);
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
        if (isspace((unsigned char)c)) {
            continue;
        }

        /* Identifier or keyword */
        if (isalpha((unsigned char)c) || c == '_') {

            char word[129];
            int i = 0;

            word[i++] = (char)c;

            while ((c = fgetc(input)) != EOF &&
                   (isalnum((unsigned char)c) || c == '_')) {

                word[i++] = (char)c;
            }

            word[i] = '\0';

            if (c != EOF) {
                ungetc(c, input);
            }

            if (is_keyword(word)) {
                print_token(word, "KEYWORD");
            } else {
                print_token(word, "IDENTIFIER");
            }

            continue;
        }

        /* Integer */
        if (isdigit((unsigned char)c)) {

            char number[129];
            int i = 0;

            number[i++] = (char)c;

            while ((c = fgetc(input)) != EOF &&
                   isdigit((unsigned char)c)) {

                number[i++] = (char)c;
            }

            number[i] = '\0';

            if (c != EOF) {
                ungetc(c, input);
            }

            print_token(number, "INTEGER");

            continue;
        }

        /* Slash, line comment, or block comment */
        if (c == '/') {

            int next = fgetc(input);

            /* Line comment */
            if (next == '/') {

                while ((c = fgetc(input)) != EOF && c != '\n') {
                    /* skip comment */
                }

                continue;
            }

            /* Block comment */
            if (next == '*') {

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

            /* A normal slash is division */
            if (next != EOF) {
                ungetc(next, input);
            }

            print_token("/", "ARITHMETIC_OPERATOR");
            continue;
        }

        /* = or == */
        if (c == '=') {

            int next = fgetc(input);

            if (next == '=') {
                print_token("==", "RELATIONAL_OPERATOR");
            } else {
                if (next != EOF) {
                    ungetc(next, input);
                }

                print_token("=", "ASSIGNMENT_OPERATOR");
            }

            continue;
        }

        /* ! or != */
        if (c == '!') {

            int next = fgetc(input);

            if (next == '=') {
                print_token("!=", "RELATIONAL_OPERATOR");
            } else {
                if (next != EOF) {
                    ungetc(next, input);
                }

                print_token("!", "LOGICAL_OPERATOR");
            }

            continue;
        }

        /* < or <= */
        if (c == '<') {

            int next = fgetc(input);

            if (next == '=') {
                print_token("<=", "RELATIONAL_OPERATOR");
            } else {
                if (next != EOF) {
                    ungetc(next, input);
                }

                print_token("<", "RELATIONAL_OPERATOR");
            }

            continue;
        }

        /* > or >= */
        if (c == '>') {

            int next = fgetc(input);

            if (next == '=') {
                print_token(">=", "RELATIONAL_OPERATOR");
            } else {
                if (next != EOF) {
                    ungetc(next, input);
                }

                print_token(">", "RELATIONAL_OPERATOR");
            }

            continue;
        }

        /* && */
        if (c == '&') {

            int next = fgetc(input);

            if (next == '&') {
                print_token("&&", "LOGICAL_OPERATOR");
            } else {
                if (next != EOF) {
                    ungetc(next, input);
                }

                print_token("&", "UNKNOWN");
            }

            continue;
        }

        /* || */
        if (c == '|') {

            int next = fgetc(input);

            if (next == '|') {
                print_token("||", "LOGICAL_OPERATOR");
            } else {
                if (next != EOF) {
                    ungetc(next, input);
                }

                print_token("|", "UNKNOWN");
            }

            continue;
        }

        /* Arithmetic operators */
        if (c == '+' || c == '-' || c == '*' || c == '%') {

            char op[2];

            op[0] = (char)c;
            op[1] = '\0';

            print_token(op, "ARITHMETIC_OPERATOR");
            continue;
        }

        /* Delimiters */
        if (c == '(' || c == ')' ||
            c == '{' || c == '}' ||
            c == '[' || c == ']' ||
            c == ';' || c == ',') {

            char delimiter[2];

            delimiter[0] = (char)c;
            delimiter[1] = '\0';

            print_token(delimiter, "DELIMITER");
            continue;
        }

        /* Any other unsupported character */
        {
            char unknown[2];

            unknown[0] = (char)c;
            unknown[1] = '\0';

            print_token(unknown, "UNKNOWN");
        }
    }

    fclose(input);
    return 0;
}