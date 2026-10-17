#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

int isKeyword(const char *str) {
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
        fprintf(stderr, "Error: Missing or too many arguments.\n");
        return 1;
    }

    FILE *file = fopen(argv[1], "r");
    if (!file) {
        fprintf(stderr, "Error: Cannot open file %s.\n", argv[1]);
        return 1;
    }

    int current;
    while ((current = fgetc(file)) != EOF) {
        // Skip whitespace
        if (isspace(current)) {
            continue;
        }

        // Handle Comments and Division operator '/'
        if (current == '/') {
            int next = fgetc(file);
            if (next == '/') {
                // Line comment, skip until newline or EOF
                while ((current = fgetc(file)) != EOF && current != '\n');
            } else if (next == '*') {
                // Block comment, skip until */
                int prev = 0;
                int unclosed = 1;
                while ((current = fgetc(file)) != EOF) {
                    if (prev == '*' && current == '/') {
                        unclosed = 0;
                        break;
                    }
                    prev = current;
                }
                if (unclosed) {
                    fprintf(stderr, "Error: Unclosed block comment.\n");
                    fclose(file);
                    return 1;
                }
            } else {
                // Division operator
                if (next != EOF) ungetc(next, file);
                printf("/,ARITHMETIC_OPERATOR\n");
            }
            continue;
        }

        // Handle Identifiers and Keywords
        if (isalpha(current) || current == '_') {
            char buffer[256];
            int len = 0;
            buffer[len++] = current;
            
            while ((current = fgetc(file)) != EOF && (isalnum(current) || current == '_')) {
                if (len < 255) buffer[len++] = current;
            }
            buffer[len] = '\0';
            if (current != EOF) ungetc(current, file);

            if (isKeyword(buffer)) {
                printf("%s,KEYWORD\n", buffer);
            } else {
                printf("%s,IDENTIFIER\n", buffer);
            }
            continue;
        }

        // Handle Integers
        if (isdigit(current)) {
            char buffer[256];
            int len = 0;
            buffer[len++] = current;

            while ((current = fgetc(file)) != EOF && isdigit(current)) {
                if (len < 255) buffer[len++] = current;
            }
            buffer[len] = '\0';
            if (current != EOF) ungetc(current, file);

            printf("%s,INTEGER\n", buffer);
            continue;
        }

        // Handle Relational and Logical Operators
        if (current == '=') {
            int next = fgetc(file);
            if (next == '=') {
                printf("==,RELATIONAL_OPERATOR\n");
            } else {
                if (next != EOF) ungetc(next, file);
                printf("=,ASSIGNMENT_OPERATOR\n");
            }
            continue;
        }

        if (current == '!') {
            int next = fgetc(file);
            if (next == '=') {
                printf("!=,RELATIONAL_OPERATOR\n");
            } else {
                if (next != EOF) ungetc(next, file);
                printf("!,LOGICAL_OPERATOR\n");
            }
            continue;
        }

        if (current == '<') {
            int next = fgetc(file);
            if (next == '=') {
                printf("<=,RELATIONAL_OPERATOR\n");
            } else {
                if (next != EOF) ungetc(next, file);
                printf("<,RELATIONAL_OPERATOR\n");
            }
            continue;
        }

        if (current == '>') {
            int next = fgetc(file);
            if (next == '=') {
                printf(">=,RELATIONAL_OPERATOR\n");
            } else {
                if (next != EOF) ungetc(next, file);
                printf(">,RELATIONAL_OPERATOR\n");
            }
            continue;
        }

        if (current == '&') {
            int next = fgetc(file);
            if (next == '&') {
                printf("&&,LOGICAL_OPERATOR\n");
            } else {
                if (next != EOF) ungetc(next, file);
                printf("&,UNKNOWN\n");
            }
            continue;
        }

        if (current == '|') {
            int next = fgetc(file);
            if (next == '|') {
                printf("||,LOGICAL_OPERATOR\n");
            } else {
                if (next != EOF) ungetc(next, file);
                printf("|,UNKNOWN\n");
            }
            continue;
        }

        // Arithmetic Operators
        if (current == '+') {
            printf("+,ARITHMETIC_OPERATOR\n");
            continue;
        }
        if (current == '-') {
            printf("-,ARITHMETIC_OPERATOR\n");
            continue;
        }
        if (current == '*') {
            printf("*,ARITHMETIC_OPERATOR\n");
            continue;
        }

        // Delimiters
        if (current == '(' || current == ')' || current == '{' || current == '}' ||
            current == '[' || current == ']' || current == ':' || current == ',' ||
            current == ';' || current == '.') {
            printf("%c,DELIMITER\n", current);
            continue;
        }

        // Any other unsupported single character
        printf("%c,UNKNOWN\n", current);
    }

    fclose(file);
    return 0;
}