#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

int is_keyword(const char *str) {
    const char *keywords[] = {"int", "if", "else", "while", "return"};
    for (int i = 0; i < 5; i++) {
        if (strcmp(str, keywords[i]) == 0) {
            return 1;
        }
    }
    return 0;
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <source_file>\n", argv[0]);
        return 1;
    }

    FILE *file = fopen(argv[1], "r");
    if (!file) {
        fprintf(stderr, "Error: Cannot open file %s\n", argv[1]);
        return 1;
    }

    int current = fgetc(file);
    while (current != EOF) {
        // Skip whitespace
        if (isspace(current)) {
            current = fgetc(file);
            continue;
        }

        // Handle comments and division '/'
        if (current == '/') {
            int next = fgetc(file);
            if (next == '/') {
                // Line comment
                while (current != '\n' && current != EOF) {
                    current = fgetc(file);
                }
            } else if (next == '*') {
                // Block comment
                int prev = 0;
                while (1) {
                    current = fgetc(file);
                    if (current == EOF) {
                        fprintf(stderr, "Error: Unclosed block comment\n");
                        fclose(file);
                        return 1;
                    }
                    if (prev == '*' && current == '/') {
                        break;
                    }
                    prev = current;
                }
                current = fgetc(file);
            } else {
                // Division operator
                printf("/,ARITHMETIC_OPERATOR\n");
                current = next;
                continue;
            }
            current = fgetc(file);
            continue;
        }

        // Handle Identifiers and Keywords: [A-Za-z_][A-Za-z0-9_]*
        if (isalpha(current) || current == '_') {
            char buffer[256];
            int len = 0;
            while (isalnum(current) || current == '_') {
                if (len < 255) buffer[len++] = current;
                current = fgetc(file);
            }
            buffer[len] = '\0';

            if (is_keyword(buffer)) {
                printf("%s,KEYWORD\n", buffer);
            } else {
                printf("%s,IDENTIFIER\n", buffer);
            }
            continue;
        }

        // Handle Integers: [0-9]+
        if (isdigit(current)) {
            char buffer[256];
            int len = 0;
            while (isdigit(current)) {
                if (len < 255) buffer[len++] = current;
                current = fgetc(file);
            }
            buffer[len] = '\0';
            printf("%s,INTEGER\n", buffer);
            continue;
        }

        // Handle Two-character Operators: ==, !=, <=, >=, &&
        if (current == '=' || current == '!' || current == '<' || current == '>' || current == '&' || current == '|') {
            int next = fgetc(file);
            if ((current == '=' && next == '=') ||
                (current == '!' && next == '=') ||
                (current == '<' && next == '=') ||
                (current == '>' && next == '=') ||
                (current == '&' && next == '&')) {
                if (current == '=' && next == '=') printf("==,RELATIONAL_OPERATOR\n");
                else if (current == '!' && next == '=') printf("!=,RELATIONAL_OPERATOR\n");
                else if (current == '<' && next == '=') printf("<=,RELATIONAL_OPERATOR\n");
                else if (current == '>' && next == '=') printf(">=,RELATIONAL_OPERATOR\n");
                else if (current == '&' && next == '&') printf("&&,LOGICAL_OPERATOR\n");
                current = fgetc(file);
                continue;
            } else {
                // Single character fallback for those symbols if not matched as double
                if (current == '=') {
                    printf("=,ASSIGNMENT_OPERATOR\n");
                } else if (current == '<') {
                    printf("<,RELATIONAL_OPERATOR\n");
                } else if (current == '>') {
                    printf(">,RELATIONAL_OPERATOR\n");
                } else {
                    printf("%c,UNKNOWN\n", current);
                }
                current = next;
                continue;
            }
        }

        // Single Character Arithmetic Operators: +, -, *, %
        if (current == '+' || current == '-' || current == '*' || current == '%') {
            if (current == '+') printf("+,ARITHMETIC_OPERATOR\n");
            else if (current == '-') printf("-,ARITHMETIC_OPERATOR\n");
            else if (current == '*') printf("*,ARITHMETIC_OPERATOR\n");
            else if (current == '%') printf("%%,ARITHMETIC_OPERATOR\n");
            current = fgetc(file);
            continue;
        }

        // Delimiters: (, ), {, }, [, ], ;, ,
        if (current == '(' || current == ')' || current == '{' || current == '}' ||
            current == '[' || current == ']' || current == ';' || current == ',') {
            printf("%c,DELIMITER\n", current);
            current = fgetc(file);
            continue;
        }

        // Unknown characters
        printf("%c,UNKNOWN\n", current);
        current = fgetc(file);
    }

    fclose(file);
    return 0;
}
