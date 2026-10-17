
#include <stdio.h>
#include <ctype.h>
#include <string.h>

int is_keyword(char word[]) {
    return strcmp(word, "int") == 0 ||
           strcmp(word, "if") == 0 ||
           strcmp(word, "else") == 0 ||
           strcmp(word, "while") == 0 ||
           strcmp(word, "return") == 0;
}

int is_identifier_start(int c) {
    return isalpha(c) || c == '_';
}

int is_identifier_part(int c) {
    return isalnum(c) || c == '_';
}

void print_token(char token[], char category[]) {
    printf("%s,%s\n", token, category);
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
    int next;
    int i;
    char token[129];

    while ((c = fgetc(input)) != EOF) {

        if (isspace(c)) {
            continue;
        }

        i = 0;

        /* Identifier or keyword */
        if (is_identifier_start(c)) {
            token[i++] = c;

            while ((c = fgetc(input)) != EOF &&
                   is_identifier_part(c)) {
                token[i++] = c;
            }

            token[i] = '\0';

            if (c != EOF) {
                ungetc(c, input);
            }

            if (is_keyword(token)) {
                print_token(token, "KEYWORD");
            } else {
                print_token(token, "IDENTIFIER");
            }
        }

        /* Integer */
        else if (isdigit(c)) {
            token[i++] = c;

            while ((c = fgetc(input)) != EOF &&
                   isdigit(c)) {
                token[i++] = c;
            }

            token[i] = '\0';

            if (c != EOF) {
                ungetc(c, input);
            }

            print_token(token, "INTEGER");
        }

        /* Slash or comment */
        else if (c == '/') {
            next = fgetc(input);

            if (next == '/') {
                while ((c = fgetc(input)) != EOF && c != '\n') {
                    /* Skip line comment */
                }
            }
            else if (next == '*') {
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
            }
            else {
                if (next != EOF) {
                    ungetc(next, input);
                }

                token[0] = '/';
                token[1] = '\0';
                print_token(token, "ARITHMETIC_OPERATOR");
            }
        }

        /* Relational, logical and assignment operators */
        else if (c == '=' || c == '!' ||
                 c == '<' || c == '>' ||
                 c == '&' || c == '|') {

            next = fgetc(input);
            token[0] = c;
            token[1] = '\0';
            token[2] = '\0';

            if ((c == '=' && next == '=') ||
                (c == '!' && next == '=') ||
                (c == '<' && next == '=') ||
                (c == '>' && next == '=')) {

                token[1] = next;
                token[2] = '\0';
                print_token(token, "RELATIONAL_OPERATOR");
            }
            else if ((c == '&' && next == '&') ||
                     (c == '|' && next == '|')) {

                token[1] = next;
                token[2] = '\0';
                print_token(token, "LOGICAL_OPERATOR");
            }
            else {
                if (next != EOF) {
                    ungetc(next, input);
                }

                if (c == '=') {
                    print_token(token, "ASSIGNMENT_OPERATOR");
                }
                else if (c == '!') {
                    print_token(token, "LOGICAL_OPERATOR");
                }
                else if (c == '<' || c == '>') {
                    print_token(token, "RELATIONAL_OPERATOR");
                }
                else {
                    print_token(token, "UNKNOWN");
                }
            }
        }

        /* Arithmetic operators */
        else if (c == '+' || c == '-' ||
                 c == '*' || c == '%') {

            token[0] = c;
            token[1] = '\0';
            print_token(token, "ARITHMETIC_OPERATOR");
        }

        /* Delimiters */
        else if (c == '(' || c == ')' ||
                 c == '{' || c == '}' ||
                 c == '[' || c == ']' ||
                 c == ';' || c == ',') {

            token[0] = c;
            token[1] = '\0';
            print_token(token, "DELIMITER");
        }

        /* Any unsupported character */
        else {
            token[0] = c;
            token[1] = '\0';
            print_token(token, "UNKNOWN");
        }
    }

    fclose(input);
    return 0;
}
