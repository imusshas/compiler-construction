/* Lab 02: implement the section manual's lexical contract.
   Read argv[1] with fopen/fgetc. Do not hard-code public.c or its token list.
   You may add helper functions. This starter is deliberately unfinished. */
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>

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
        if (isspace(c)) {
            continue;
        }

        if (c == '/') {
            int next = fgetc(input);
            if (next == '/') {
                while ((c = fgetc(input)) != EOF && c != '\n');
            } else if (next == '*') {
                int prev = 0;
                int unclosed = 1;
                while ((c = fgetc(input)) != EOF) {
                    if (prev == '*' && c == '/') {
                        unclosed = 0;
                        break;
                    }
                    prev = c;
                }
                if (unclosed) {
                    fprintf(stderr, "Error: unclosed block comment\n");
                    fclose(input);
                    return 1;
                }
            } else {
                if (next != EOF) ungetc(next, input);
                printf("/,ARITHMETIC_OPERATOR\n");
            }
            continue;
        }

        if (isalpha(c) || c == '_') {
            char buffer[128];
            int len = 0;
            buffer[len++] = c;
            while ((c = fgetc(input)) != EOF && (isalnum(c) || c == '_')) {
                if (len < 127) buffer[len++] = c;
            }
            buffer[len] = '\0';
            if (c != EOF) ungetc(c, input);

            if (strcmp(buffer, "int") == 0 || strcmp(buffer, "if") == 0 ||
                strcmp(buffer, "else") == 0 || strcmp(buffer, "while") == 0 ||
                strcmp(buffer, "return") == 0) {
                printf("%s,KEYWORD\n", buffer);
            } else {
                printf("%s,IDENTIFIER\n", buffer);
            }
            continue;
        }

        if (isdigit(c)) {
            char buffer[128];
            int len = 0;
            buffer[len++] = c;
            while ((c = fgetc(input)) != EOF && isdigit(c)) {
                if (len < 127) buffer[len++] = c;
            }
            buffer[len] = '\0';
            if (c != EOF) ungetc(c, input);
            printf("%s,INTEGER\n", buffer);
            continue;
        }

        if (c == '=' || c == '!' || c == '<' || c == '>' || c == '&' || c == '|') {
            int next = fgetc(input);
            if ((c == '=' && next == '=') || (c == '!' && next == '=') ||
                (c == '<' && next == '=') || (c == '>' && next == '=')) {
                printf("%c%c,RELATIONAL_OPERATOR\n", c, next);
                continue;
            } else if (c == '=' && next != '=') {
                if (next != EOF) ungetc(next, input);
                printf("=,ASSIGNMENT_OPERATOR\n");
                continue;
            } else if (c == '<' && next != '=') {
                if (next != EOF) ungetc(next, input);
                printf("<,RELATIONAL_OPERATOR\n");
                continue;
            } else if (c == '>' && next != '=') {
                if (next != EOF) ungetc(next, input);
                printf(">,RELATIONAL_OPERATOR\n");
                continue;
            } else if (c == '&' && next == '&') {
                printf("&&,LOGICAL_OPERATOR\n");
                continue;
            } else if (c == '|' && next == '|') {
                printf("||,LOGICAL_OPERATOR\n");
                continue;
            } else {
                if (next != EOF) ungetc(next, input);
                if (c == '!') {
                    printf("!,LOGICAL_OPERATOR\n");
                    continue;
                }
            }
        }

        if (c == '+' || c == '*') {
            printf("%c,ARITHMETIC_OPERATOR\n", c);
        } else if (c == '-') {
            printf("-,ARITHMETIC_OPERATOR\n");
        } else if (c == '(' || c == ')' || c == '{' || c == '}' || 
                   c == '[' || c == ']' || c == ':' || c == ',' || c == ';') {
            printf("%c,DELIMITER\n", c);
        } else {
            printf("%c,UNKNOWN\n", c);
        }
    }

    fclose(input);
    return 0;
}