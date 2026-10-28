
#include <stdio.h>
#include <ctype.h>
#include <string.h>

#define MAX_LEXEME 129

int is_keyword(const char *word) {
    const char *keywords[] = {
        "int", "if", "else", "while", "return"
    };

    for (int i = 0; i < 5; i++) {
        if (strcmp(word, keywords[i]) == 0)
            return 1;
    }
    return 0;
}

void emit(const char *lexeme, const char *token) {
    printf("%s,%s\n", lexeme, token);
}

int is_identifier_start(int c) {
    return isalpha((unsigned char)c) || c == '_';
}

int is_identifier_part(int c) {
    return isalnum((unsigned char)c) || c == '_';
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

        char lexeme[MAX_LEXEME];
        int i = 0;
        lexeme[i++] = (char)c;

        /* Identifiers and keywords */
        if (is_identifier_start(c)) {
            while ((c = fgetc(input)) != EOF &&
                   is_identifier_part(c)) {
                if (i < MAX_LEXEME - 1)
                    lexeme[i++] = (char)c;
            }

            if (c != EOF)
                ungetc(c, input);

            lexeme[i] = '\0';

            emit(lexeme, is_keyword(lexeme)
                         ? "KEYWORD" : "IDENTIFIER");
            continue;
        }

        /* Integers */
        if (isdigit((unsigned char)c)) {
            while ((c = fgetc(input)) != EOF &&
                   isdigit((unsigned char)c)) {
                if (i < MAX_LEXEME - 1)
                    lexeme[i++] = (char)c;
            }

            if (c != EOF)
                ungetc(c, input);

            lexeme[i] = '\0';
            emit(lexeme, "INTEGER");
            continue;
        }

        /* Slash, division, and comments */
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
                    fprintf(stderr,
                            "Error: unclosed block comment\n");
                    fclose(input);
                    return 1;
                }
                continue;
            }

            if (next != EOF)
                ungetc(next, input);

            lexeme[i] = '\0';
            emit(lexeme, "ARITHMETIC_OPERATOR");
            continue;
        }

        /* Two-character operators */
        if (c == '=' || c == '!' || c == '<' || c == '>' ||
            c == '&' || c == '|') {
            int next = fgetc(input);
            int paired = 0;

            if ((c == '=' && next == '=') ||
                (c == '!' && next == '=') ||
                (c == '<' && next == '=') ||
                (c == '>' && next == '=') ||
                (c == '&' && next == '&') ||
                (c == '|' && next == '|')) {
                lexeme[i++] = (char)next;
                paired = 1;
            } else if (next != EOF) {
                ungetc(next, input);
            }

            lexeme[i] = '\0';

            if (c == '&' || c == '|') {
                if (paired)
                    emit(lexeme, "LOGICAL_OPERATOR");
                else
                    emit(lexeme, "UNKNOWN");
            } else if (c == '!') {
                emit(lexeme, paired
                             ? "RELATIONAL_OPERATOR"
                             : "LOGICAL_OPERATOR");
            } else if (c == '<' || c == '>') {
                emit(lexeme, "RELATIONAL_OPERATOR");
            } else {
                emit(lexeme, paired
                             ? "RELATIONAL_OPERATOR"
                             : "ASSIGNMENT_OPERATOR");
            }
            continue;
        }

        /* Single-character operators */
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
        fprintf(stderr, "Error: failed to read input file\n");
        fclose(input);
        return 2;
    }

    fclose(input);
    return 0;
}
