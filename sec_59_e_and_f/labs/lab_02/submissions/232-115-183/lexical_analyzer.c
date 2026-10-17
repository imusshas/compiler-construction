
/* Lab 02: Lexical Analyzer in C
   Reads a source file character by character and prints lexeme,TOKEN.
*/

#include <stdio.h>
#include <ctype.h>
#include <string.h>

#define MAX_LEXEME 128

/* Check whether an identifier is a keyword */
int is_keyword(const char *word) {
    return strcmp(word, "int") == 0 ||
           strcmp(word, "if") == 0 ||
           strcmp(word, "else") == 0 ||
           strcmp(word, "while") == 0 ||
           strcmp(word, "return") == 0;
}

int main(int argc, char **argv) {

    /* Validate the command-line argument */
    if (argc != 2) {
        fprintf(stderr, "usage: lexical_analyzer input.c\n");
        return 2;
    }

    /* Open the source file */
    FILE *input = fopen(argv[1], "rb");

    if (input == NULL) {
        perror("input");
        return 2;
    }

    int ch;
    char lexeme[MAX_LEXEME + 1];

    /* Read the file character by character */
    while ((ch = fgetc(input)) != EOF) {

        /* 1. Skip whitespace */
        if (isspace((unsigned char)ch)) {
            continue;
        }

        /* 2. Handle comments and division */
        if (ch == '/') {
            int next = fgetc(input);

            /* Single-line comment */
            if (next == '/') {
                while ((ch = fgetc(input)) != EOF &&
                       ch != '\n') {
                }
                continue;
            }

            /* Block comment */
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
                    fprintf(stderr,
                            "Error: unclosed block comment\n");
                    fclose(input);
                    return 1;
                }

                continue;
            }

            /* Division operator */
            if (next != EOF) {
                ungetc(next, input);
            }

            printf("/,ARITHMETIC_OPERATOR\n");
            continue;
        }

        /* 3. Identify keywords and identifiers */
        if (isalpha((unsigned char)ch) || ch == '_') {
            int i = 0;
            lexeme[i++] = (char)ch;

            while ((ch = fgetc(input)) != EOF &&
                   (isalnum((unsigned char)ch) || ch == '_')) {
                lexeme[i++] = (char)ch;
            }

            lexeme[i] = '\0';

            if (ch != EOF) {
                ungetc(ch, input);
            }

            if (is_keyword(lexeme)) {
                printf("%s,KEYWORD\n", lexeme);
            } else {
                printf("%s,IDENTIFIER\n", lexeme);
            }

            continue;
        }

        /* 4. Identify integer constants */
        if (isdigit((unsigned char)ch)) {
            int i = 0;
            lexeme[i++] = (char)ch;

            while ((ch = fgetc(input)) != EOF &&
                   isdigit((unsigned char)ch)) {
                lexeme[i++] = (char)ch;
            }

            lexeme[i] = '\0';

            if (ch != EOF) {
                ungetc(ch, input);
            }

            printf("%s,INTEGER\n", lexeme);
            continue;
        }

        /* 5. Assignment and equality operators */
        if (ch == '=') {
            int next = fgetc(input);

            if (next == '=') {
                printf("==,RELATIONAL_OPERATOR\n");
            } else {
                if (next != EOF) {
                    ungetc(next, input);
                }
                printf("=,ASSIGNMENT_OPERATOR\n");
            }
            continue;
        }

        /* 6. Relational operators */
        if (ch == '<' || ch == '>') {
            int next = fgetc(input);

            if (next == '=') {
                printf("%c=,RELATIONAL_OPERATOR\n", ch);
            } else {
                if (next != EOF) {
                    ungetc(next, input);
                }
                printf("%c,RELATIONAL_OPERATOR\n", ch);
            }
            continue;
        }

        /* 7. Logical NOT and not-equal */
        if (ch == '!') {
            int next = fgetc(input);

            if (next == '=') {
                printf("!=,RELATIONAL_OPERATOR\n");
            } else {
                if (next != EOF) {
                    ungetc(next, input);
                }
                printf("!,LOGICAL_OPERATOR\n");
            }
            continue;
        }

        /* 8. Logical AND and OR */
        if (ch == '&' || ch == '|') {
            int next = fgetc(input);

            if (next == ch) {
                printf("%c%c,LOGICAL_OPERATOR\n", ch, ch);
            } else {
                if (next != EOF) {
                    ungetc(next, input);
                }
                printf("%c,UNKNOWN\n", ch);
            }
            continue;
        }

        /* 9. Arithmetic operators */
        if (ch == '+' || ch == '-' ||
            ch == '*' || ch == '%') {
            printf("%c,ARITHMETIC_OPERATOR\n", ch);
            continue;
        }

        /* 10. Delimiters */
        if (ch == '(' || ch == ')' ||
            ch == '{' || ch == '}' ||
            ch == '[' || ch == ']' ||
            ch == ';' || ch == ',') {
            printf("%c,DELIMITER\n", ch);
            continue;
        }

        /* 11. Unsupported characters */
        printf("%c,UNKNOWN\n", ch);
    }

    /* Close the input file */
    fclose(input);

    return 0;
}
