
#include <stdio.h>
#include <ctype.h>
#include <string.h>

#define MAX_LEN 128

int is_start(int c) {
    return (c >= 'A' && c <= 'Z') ||
           (c >= 'a' && c <= 'z') || c == '_';
}

int is_part(int c) {
    return is_start(c) || (c >= '0' && c <= '9');
}

int is_keyword(const char *s) {
    const char *words[] = {
        "int", "if", "else", "while", "return"
    };

    for (int i = 0; i < 5; i++) {
        if (strcmp(s, words[i]) == 0)
            return 1;
    }
    return 0;
}

void emit(const char *lexeme, const char *token) {
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
        if (isspace((unsigned char)c))
            continue;

        char s[MAX_LEN + 1];
        int n = 0;
        s[n++] = (char)c;
        s[n] = '\0';

        if (is_start(c)) {
            while ((c = fgetc(input)) != EOF && is_part(c))
                s[n++] = (char)c;

            if (c != EOF)
                ungetc(c, input);

            s[n] = '\0';
            emit(s, is_keyword(s) ? "KEYWORD" : "IDENTIFIER");
        }
        else if (c >= '0' && c <= '9') {
            while ((c = fgetc(input)) != EOF &&
                   c >= '0' && c <= '9')
                s[n++] = (char)c;

            if (c != EOF)
                ungetc(c, input);

            s[n] = '\0';
            emit(s, "INTEGER");
        }
        else if (c == '/') {
            int next = fgetc(input);

            if (next == '/') {
                while ((c = fgetc(input)) != EOF && c != '\n')
                    ;
            }
            else if (next == '*') {
                int prev = 0;
                int closed = 0;

                while ((c = fgetc(input)) != EOF) {
                    if (prev == '*' && c == '/') {
                        closed = 1;
                        break;
                    }
                    prev = c;
                }

                if (!closed) {
                    fprintf(stderr, "Error: unclosed block comment\n");
                    fclose(input);
                    return 1;
                }
            }
            else {
                if (next != EOF)
                    ungetc(next, input);
                emit("/", "ARITHMETIC_OPERATOR");
            }
        }
        else if (c == '=' || c == '!' ||
                 c == '<' || c == '>') {
            int next = fgetc(input);

            if (next == '=') {
                s[1] = '=';
                s[2] = '\0';

                if (c == '=' || c == '!')
                    emit(s, "RELATIONAL_OPERATOR");
                else
                    emit(s, "RELATIONAL_OPERATOR");
            }
            else {
                if (next != EOF)
                    ungetc(next, input);

                if (c == '=')
                    emit(s, "ASSIGNMENT_OPERATOR");
                else if (c == '!')
                    emit(s, "LOGICAL_OPERATOR");
                else
                    emit(s, "RELATIONAL_OPERATOR");
            }
        }
        else if (c == '&' || c == '|') {
            int next = fgetc(input);

            if (next == c) {
                s[1] = (char)c;
                s[2] = '\0';
                emit(s, "LOGICAL_OPERATOR");
            }
            else {
                if (next != EOF)
                    ungetc(next, input);
                emit(s, "UNKNOWN");
            }
        }
        else if (c == '+' || c == '-' || c == '*' || c == '%') {
            emit(s, "ARITHMETIC_OPERATOR");
        }
        else if (strchr("(){}[];,", c) != NULL) {
            emit(s, "DELIMITER");
        }
        else if (c >= 32 && c <= 126) {
            emit(s, "UNKNOWN");
        }
        else {
            emit(s, "UNKNOWN");
        }
    }

    fclose(input);
    return 0;
}
