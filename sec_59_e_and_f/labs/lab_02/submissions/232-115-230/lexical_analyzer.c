
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_LEXEME 128

int is_identifier_start(int c) {
    return (c >= 'A' && c <= 'Z') ||
           (c >= 'a' && c <= 'z') ||
           c == '_';
}

int is_identifier_char(int c) {
    return is_identifier_start(c) ||
           (c >= '0' && c <= '9');
}

int is_digit(int c) {
    return c >= '0' && c <= '9';
}

int is_whitespace(int c) {
    return c == ' ' || c == '\t' ||
           c == '\r' || c == '\n';
}

int is_keyword(const char *word) {
    return strcmp(word, "int") == 0 ||
           strcmp(word, "if") == 0 ||
           strcmp(word, "else") == 0 ||
           strcmp(word, "while") == 0 ||
           strcmp(word, "return") == 0;
}

void emit(const char *lexeme, const char *category) {
    printf("%s,%s\n", lexeme, category);
}

void scan_identifier(FILE *file, int first) {
    char lexeme[MAX_LEXEME + 1];
    int length = 0;
    int c = first;

    while (is_identifier_char(c)) {
        if (length < MAX_LEXEME) {
            lexeme[length++] = (char)c;
        }
        c = fgetc(file);
    }

    if (c != EOF) {
        ungetc(c, file);
    }

    lexeme[length] = '\0';

    if (is_keyword(lexeme)) {
        emit(lexeme, "KEYWORD");
    } else {
        emit(lexeme, "IDENTIFIER");
    }
}

void scan_integer(FILE *file, int first) {
    char lexeme[MAX_LEXEME + 1];
    int length = 0;
    int c = first;

    while (is_digit(c)) {
        if (length < MAX_LEXEME) {
            lexeme[length++] = (char)c;
        }
        c = fgetc(file);
    }

    if (c != EOF) {
        ungetc(c, file);
    }

    lexeme[length] = '\0';
    emit(lexeme, "INTEGER");
}

int is_delimiter(int c) {
    return c == '(' || c == ')' ||
           c == '{' || c == '}' ||
           c == '[' || c == ']' ||
           c == ';' || c == ',';
}

int scan_file(FILE *file) {
    int c;

    while ((c = fgetc(file)) != EOF) {
        char lexeme[3] = {'\0', '\0', '\0'};
        int next;

        if (is_whitespace(c)) {
            continue;
        }

        if (is_identifier_start(c)) {
            scan_identifier(file, c);
            continue;
        }

        if (is_digit(c)) {
            scan_integer(file, c);
            continue;
        }

        if (c == '/') {
            next = fgetc(file);

            if (next == '/') {
                while ((c = fgetc(file)) != EOF && c != '\n') {
                    /* Skip line comment. */
                }
                continue;
            }

            if (next == '*') {
                int previous = 0;
                int closed = 0;

                while ((c = fgetc(file)) != EOF) {
                    if (previous == '*' && c == '/') {
                        closed = 1;
                        break;
                    }
                    previous = c;
                }

                if (!closed) {
                    fprintf(stderr,
                            "Error: unclosed block comment.\n");
                    return 1;
                }

                continue;
            }

            if (next != EOF) {
                ungetc(next, file);
            }

            emit("/", "ARITHMETIC_OPERATOR");
            continue;
        }

        lexeme[0] = (char)c;

        if (c == '=' || c == '!' || c == '<' || c == '>' ||
            c == '&' || c == '|') {

            next = fgetc(file);

            if ((c == '=' && next == '=') ||
                (c == '!' && next == '=') ||
                (c == '<' && next == '=') ||
                (c == '>' && next == '=') ||
                (c == '&' && next == '&') ||
                (c == '|' && next == '|')) {

                lexeme[1] = (char)next;

                if (c == '=' || c == '!' ||
                    c == '<' || c == '>') {
                    emit(lexeme, "RELATIONAL_OPERATOR");
                } else {
                    emit(lexeme, "LOGICAL_OPERATOR");
                }

                continue;
            }

            if (next != EOF) {
                ungetc(next, file);
            }

            if (c == '=') {
                emit(lexeme, "ASSIGNMENT_OPERATOR");
            } else if (c == '!') {
                emit(lexeme, "LOGICAL_OPERATOR");
            } else if (c == '<' || c == '>') {
                emit(lexeme, "RELATIONAL_OPERATOR");
            } else {
                emit(lexeme, "UNKNOWN");
            }

            continue;
        }

        if (c == '+' || c == '-' || c == '*' || c == '%') {
            emit(lexeme, "ARITHMETIC_OPERATOR");
            continue;
        }

        if (is_delimiter(c)) {
            emit(lexeme, "DELIMITER");
            continue;
        }

        emit(lexeme, "UNKNOWN");
    }

    if (ferror(file)) {
        fprintf(stderr, "Error: failed to read input file.\n");
        return 1;
    }

    return 0;
}

int main(int argc, char *argv[]) {
    FILE *file;
    int status;

    if (argc != 2) {
        fprintf(stderr, "Usage: %s <input-file>\n", argv[0]);
        return 1;
    }

    file = fopen(argv[1], "r");

    if (file == NULL) {
        fprintf(stderr, "Error: cannot open file '%s'.\n", argv[1]);
        return 1;
    }

    status = scan_file(file);

    if (fclose(file) != 0) {
        fprintf(stderr, "Error: failed to close input file.\n");
        status = 1;
    }

    return status;
}
