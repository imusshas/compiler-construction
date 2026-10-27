#include <stdio.h>
#include <ctype.h>
#include <string.h>

#define MAX_LEXEME 128

static void emit(const char *lexeme, const char *token) {
    printf("%s,%s\n", lexeme, token);
}

static int is_start(int c) {
    return (c >= 'A' && c <= 'Z') ||
           (c >= 'a' && c <= 'z') || c == '_';
}

static int is_part(int c) {
    return is_start(c) || (c >= '0' && c <= '9');
}

static const char *keyword(const char *s) {
    const char *words[] = {
        "int", "if", "else", "while", "return"
    };

    for (int i = 0; i < 5; i++) {
        if (strcmp(s, words[i]) == 0)
            return "KEYWORD";
    }

    return "IDENTIFIER";
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

        char lexeme[MAX_LEXEME + 1];
        int i = 0;

        /* Identifiers and keywords */
        if (is_start(c)) {
            do {
                lexeme[i++] = (char)c;
                c = fgetc(input);
            } while (c != EOF && is_part(c));

            lexeme[i] = '\0';

            if (c != EOF)
                ungetc(c, input);

            emit(lexeme, keyword(lexeme));
            continue;
        }

        /* Integer literals */
        if (c >= '0' && c <= '9') {
            do {
                lexeme[i++] = (char)c;
                c = fgetc(input);
            } while (c != EOF && c >= '0' && c <= '9');

            lexeme[i] = '\0';

            if (c != EOF)
                ungetc(c, input);

            emit(lexeme, "INTEGER");
            continue;
        }

        /* Comments and division operator */
        if (c == '/') {
            int next = fgetc(input);

            if (next == '/') {
                while ((c = fgetc(input)) != EOF && c != '\n')
                    ;
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
                    fprintf(stderr, "error: unclosed block comment\n");
                    fclose(input);
                    return 2;
                }

                continue;
            }

            if (next != EOF)
                ungetc(next, input);

            emit("/", "ARITHMETIC_OPERATOR");
            continue;
        }

        /* Two-character operators: longest match first */
        if (c == '=' || c == '!' || c == '<' || c == '>') {
            int next = fgetc(input);

            if (next == '=') {
                lexeme[0] = (char)c;
                lexeme[1] = '=';
                lexeme[2] = '\0';

                emit(lexeme, c == '=' || c == '!' ||
                     c == '<' || c == '>'
                     ? (c == '=' ? "RELATIONAL_OPERATOR" :
                        "RELATIONAL_OPERATOR")
                     : "UNKNOWN");
            } else {
                if (next != EOF)
                    ungetc(next, input);

                lexeme[0] = (char)c;
                lexeme[1] = '\0';

                if (c == '=')
                    emit(lexeme, "ASSIGNMENT_OPERATOR");
                else if (c == '!')
                    emit(lexeme, "LOGICAL_OPERATOR");
                else
                    emit(lexeme, "RELATIONAL_OPERATOR");
            }
            continue;
        }

        if (c == '&' || c == '|') {
            int next = fgetc(input);

            if (next == c) {
                lexeme[0] = (char)c;
                lexeme[1] = (char)c;
                lexeme[2] = '\0';
                emit(lexeme, "LOGICAL_OPERATOR");
            } else {
                if (next != EOF)
                    ungetc(next, input);

                lexeme[0] = (char)c;
                lexeme[1] = '\0';
                emit(lexeme, "UNKNOWN");
            }
            continue;
        }

        /* Single-character operators and delimiters */
        lexeme[0] = (char)c;
        lexeme[1] = '\0';

        if (c == '+' || c == '-' || c == '*' || c == '%') {
            emit(lexeme, "ARITHMETIC_OPERATOR");
        } else if (c == '(' || c == ')' ||
                   c == '{' || c == '}' ||
                   c == '[' || c == ']' ||
                   c == ';' || c == ',') {
            emit(lexeme, "DELIMITER");
        } else {
            emit(lexeme, "UNKNOWN");
        }
    }

    if (ferror(input)) {
        perror("input");
        fclose(input);
        return 2;
    }

    fclose(input);
    return 0;
}