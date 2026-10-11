
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

int is_keyword(const char *s) {
    const char *keywords[] = {"int", "if", "else", "while", "return"};
    for (int i = 0; i < 5; i++) {
        if (strcmp(s, keywords[i]) == 0) return 1;
    }
    return 0;
}

int is_identifier_start(int c) {
    return isalpha((unsigned char)c) || c == '_';
}

int is_identifier_char(int c) {
    return isalnum((unsigned char)c) || c == '_';
}

void emit(const char *lexeme, const char *token) {
    printf("%s,%s\n", lexeme, token);
}

int main(int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s input.c\n", argv[0]);
        return 2;
    }

    FILE *input = fopen(argv[1], "rb");
    if (input == NULL) {
        perror("Cannot open input file");
        return 2;
    }

    int c;
    while ((c = fgetc(input)) != EOF) {
        if (isspace((unsigned char)c)) continue;

        if (c == '/') {
            int next = fgetc(input);

            if (next == '/') {
                while ((c = fgetc(input)) != EOF && c != '\n') {}
                continue;
            }

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
                    fprintf(stderr, "Error: unterminated block comment\n");
                    fclose(input);
                    return 1;
                }
                continue;
            }

            if (next != EOF) ungetc(next, input);
            emit("/", "ARITHMETIC_OPERATOR");
            continue;
        }

        if (is_identifier_start(c)) {
            char lexeme[128];
            int n = 0;
            lexeme[n++] = (char)c;

            while ((c = fgetc(input)) != EOF && is_identifier_char(c)) {
                if (n < 127) lexeme[n++] = (char)c;
            }

            if (c != EOF) ungetc(c, input);
            lexeme[n] = '\0';

            emit(lexeme, is_keyword(lexeme) ?
                 "KEYWORD" : "IDENTIFIER");
            continue;
        }

        if (isdigit((unsigned char)c)) {
            char lexeme[128];
            int n = 0;
            lexeme[n++] = (char)c;

            while ((c = fgetc(input)) != EOF &&
                   isdigit((unsigned char)c)) {
                if (n < 127) lexeme[n++] = (char)c;
            }

            if (c != EOF) ungetc(c, input);
            lexeme[n] = '\0';
            emit(lexeme, "INTEGER");
            continue;
        }

        int next = fgetc(input);
        char pair[3] = {(char)c, '\0', '\0'};
        if (next != EOF) pair[1] = (char)next;

        if (next != EOF &&
            (strcmp(pair, "==") == 0 || strcmp(pair, "!=") == 0 ||
             strcmp(pair, "<=") == 0 || strcmp(pair, ">=") == 0)) {
            emit(pair, "RELATIONAL_OPERATOR");
            continue;
        }

        if (next != EOF &&
            (strcmp(pair, "&&") == 0 || strcmp(pair, "||") == 0)) {
            emit(pair, "LOGICAL_OPERATOR");
            continue;
        }

        if (next != EOF) ungetc(next, input);

        char one[2] = {(char)c, '\0'};

        if (c == '=') {
            emit(one, "ASSIGNMENT_OPERATOR");
        } else if (c == '<' || c == '>' ) {
            emit(one, "RELATIONAL_OPERATOR");
        } else if (c == '!' ) {
            emit(one, "LOGICAL_OPERATOR");
        } else if (c == '+' || c == '-' || c == '*' ||
                   c == '%' ) {
            emit(one, "ARITHMETIC_OPERATOR");
        } else if (strchr("(){}[];,", c) != NULL) {
            emit(one, "DELIMITER");
        } else if ((unsigned char)c < 128) {
            emit(one, "UNKNOWN");
        } else {
            fprintf(stderr, "Error: non-ASCII input byte\n");
            fclose(input);
            return 1;
        }
    }

    fclose(input);
    return 0;
}
