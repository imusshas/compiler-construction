
/* Lab 02: Lexical Analyzer
   Reads the input filename from argv[1].
   Uses fopen() and fgetc().
   Does not hard-code the input filename or token list.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/* Check ASCII letters */
int is_ascii_letter(int c)
{
    return (c >= 'A' && c <= 'Z') ||
           (c >= 'a' && c <= 'z');
}

/* Check ASCII digits */
int is_ascii_digit(int c)
{
    return c >= '0' && c <= '9';
}

/* Check identifier characters */
int is_identifier_char(int c)
{
    return is_ascii_letter(c) ||
           is_ascii_digit(c) ||
           c == '_';
}

/* Append a character to a dynamically growing buffer */
int append_char(char **buf, size_t *len, size_t *cap, int c)
{
    if (*len + 1 >= *cap) {
        size_t new_cap = (*cap > 0) ? *cap * 2 : 32;

        if (new_cap <= *cap) {
            return 0;
        }

        char *temp = realloc(*buf, new_cap);

        if (temp == NULL) {
            return 0;
        }

        *buf = temp;
        *cap = new_cap;
    }

    (*buf)[(*len)++] = (char)c;
    (*buf)[*len] = '\0';

    return 1;
}

/* Check keyword or identifier */
void check_keyword_or_id(const char *str)
{
    if (strcmp(str, "int") == 0 ||
        strcmp(str, "if") == 0 ||
        strcmp(str, "else") == 0 ||
        strcmp(str, "while") == 0 ||
        strcmp(str, "return") == 0) {

        printf("%s,KEYWORD\n", str);

    } else {
        printf("%s,IDENTIFIER\n", str);
    }
}

/* Read an identifier or integer without truncating it */
char *read_token(FILE *input, int *c, int is_identifier)
{
    char *buf = NULL;
    size_t len = 0;
    size_t cap = 0;

    while (*c != EOF &&
           (is_identifier
                ? is_identifier_char(*c)
                : is_ascii_digit(*c))) {

        if (!append_char(&buf, &len, &cap, *c)) {
            free(buf);
            return NULL;
        }

        *c = fgetc(input);
    }

    return buf;
}

int main(int argc, char **argv)
{
    if (argc != 2) {
        fprintf(stderr,
                "Usage: lexical_analyzer input.c\n");
        return 2;
    }

    FILE *input = fopen(argv[1], "rb");

    if (input == NULL) {
        perror("Error opening input file");
        return 2;
    }

    int c = fgetc(input);

    while (c != EOF) {

        /* Ignore whitespace */
        if (isspace((unsigned char)c)) {
            c = fgetc(input);
            continue;
        }

        /* Identifier or keyword */
        if (is_ascii_letter(c) || c == '_') {
            char *token = read_token(input, &c, 1);

            if (token == NULL) {
                fprintf(stderr, "Error: Memory allocation failed\n");
                fclose(input);
                return 1;
            }

            check_keyword_or_id(token);
            free(token);
            continue;
        }

        /* Integer */
        if (is_ascii_digit(c)) {
            char *token = read_token(input, &c, 0);

            if (token == NULL) {
                fprintf(stderr, "Error: Memory allocation failed\n");
                fclose(input);
                return 1;
            }

            printf("%s,INTEGER\n", token);
            free(token);
            continue;
        }

        /* Comments and division operator */
        if (c == '/') {
            int next = fgetc(input);

            /* Single-line comment */
            if (next == '/') {
                while ((next = fgetc(input)) != EOF &&
                       next != '\n') {
                    /* Skip comment characters */
                }

                c = (next == EOF) ? EOF : fgetc(input);
                continue;
            }

            /* Multi-line comment */
            if (next == '*') {
                int prev = 0;
                int closed = 0;

                while ((next = fgetc(input)) != EOF) {
                    if (prev == '*' && next == '/') {
                        closed = 1;
                        break;
                    }

                    prev = next;
                }

                if (!closed) {
                    fprintf(stderr,
                            "Error: Unclosed block comment\n");
                    fclose(input);
                    return 1;
                }

                c = fgetc(input);
                continue;
            }

            /* Division operator */
            if (next != EOF) {
                if (ungetc(next, input) == EOF) {
                    fprintf(stderr, "Error: Input read failure\n");
                    fclose(input);
                    return 1;
                }
            }

            printf("/,ARITHMETIC_OPERATOR\n");
            c = fgetc(input);
            continue;
        }

        /* Check two-character operators */
        int next = fgetc(input);

        if (c == '=' && next == '=') {
            printf("==,RELATIONAL_OPERATOR\n");
            c = fgetc(input);
            continue;
        }

        if (c == '!' && next == '=') {
            printf("!=,RELATIONAL_OPERATOR\n");
            c = fgetc(input);
            continue;
        }

        if (c == '<' && next == '=') {
            printf("<=,RELATIONAL_OPERATOR\n");
            c = fgetc(input);
            continue;
        }

        if (c == '>' && next == '=') {
            printf(">=,RELATIONAL_OPERATOR\n");
            c = fgetc(input);
            continue;
        }

        if (c == '&' && next == '&') {
            printf("&&,LOGICAL_OPERATOR\n");
            c = fgetc(input);
            continue;
        }

        if (c == '|' && next == '|') {
            printf("||,LOGICAL_OPERATOR\n");
            c = fgetc(input);
            continue;
        }

        /* No two-character operator matched */
        if (next != EOF) {
            if (ungetc(next, input) == EOF) {
                fprintf(stderr, "Error: Input read failure\n");
                fclose(input);
                return 1;
            }
        }

        /* Single-character operators and delimiters */
        if (c == '=') {
            printf("=,ASSIGNMENT_OPERATOR\n");
        }
        else if (c == '+' || c == '-' ||
                 c == '*' || c == '%') {
            printf("%c,ARITHMETIC_OPERATOR\n", c);
        }
        else if (c == '<' || c == '>') {
            printf("%c,RELATIONAL_OPERATOR\n", c);
        }
        else if (c == '!') {
            printf("!,LOGICAL_OPERATOR\n");
        }
        else if (c == '(' || c == ')' ||
                 c == '{' || c == '}' ||
                 c == '[' || c == ']' ||
                 c == ';' || c == ',') {
            printf("%c,DELIMITER\n", c);
        }
        else {
            printf("%c,UNKNOWN\n", c);
        }

        c = fgetc(input);
    }

    if (ferror(input)) {
        fprintf(stderr, "Error: Failed to read input file\n");
        fclose(input);
        return 1;
    }

    fclose(input);
    return 0;
}
