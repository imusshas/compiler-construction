
#include <stdio.h>
#include <ctype.h>
#include <string.h>

static int is_keyword(const char *word) {
    return strcmp(word, "int") == 0 ||
           strcmp(word, "if") == 0 ||
           strcmp(word, "else") == 0 ||
           strcmp(word, "while") == 0 ||
           strcmp(word, "return") == 0;
}

static void emit(const char *lexeme, const char *token) {
    printf("%s,%s\n", lexeme, token);
}

static void emit_char(int c, const char *token) {
    char lexeme[2] = { (char)c, '\0' };
    emit(lexeme, token);
}

int main(int argc, char *argv[]) {
    FILE *input;
    int c, next;
    char word[129];
    int length;

    if (argc != 2) {
        fprintf(stderr, "Usage: %s <input-file>\n", argv[0]);
        return 1;
    }

    input = fopen(argv[1], "r");
    if (input == NULL) {
        fprintf(stderr, "Could not open file: %s\n", argv[1]);
        return 1;
    }

    while ((c = fgetc(input)) != EOF) {
        if (isspace((unsigned char)c)) {
            continue;
        }

        if (isalpha((unsigned char)c) || c == '_') {
            length = 0;
            word[length++] = (char)c;

            while ((next = fgetc(input)) != EOF &&
                   (isalnum((unsigned char)next) || next == '_')) {
                word[length++] = (char)next;
            }

            word[length] = '\0';

            if (next != EOF) {
                ungetc(next, input);
            }

            if (is_keyword(word)) {
                emit(word, "KEYWORD");
            } else {
                emit(word, "IDENTIFIER");
            }

            continue;
        }

        if (isdigit((unsigned char)c)) {
            length = 0;
            word[length++] = (char)c;

            while ((next = fgetc(input)) != EOF &&
                   isdigit((unsigned char)next)) {
                word[length++] = (char)next;
            }

            word[length] = '\0';

            if (next != EOF) {
                ungetc(next, input);
            }

            emit(word, "INTEGER");
            continue;
        }

        if (c == '/') {
            next = fgetc(input);

            if (next == '/') {
                while ((next = fgetc(input)) != EOF && next != '\n') {
                }
                continue;
            }

            if (next == '*') {
                int previous = 0;
                int closed = 0;

                while ((next = fgetc(input)) != EOF) {
                    if (previous == '*' && next == '/') {
                        closed = 1;
                        break;
                    }
                    previous = next;
                }

                if (!closed) {
                    fprintf(stderr, "Unclosed block comment\n");
                    fclose(input);
                    return 1;
                }

                continue;
            }

            if (next != EOF) {
                ungetc(next, input);
            }

            emit("/", "ARITHMETIC_OPERATOR");
            continue;
        }

        if (c == '=') {
            next = fgetc(input);
            if (next == '=') {
                emit("==", "RELATIONAL_OPERATOR");
            } else {
                if (next != EOF) ungetc(next, input);
                emit("=", "ASSIGNMENT_OPERATOR");
            }
            continue;
        }

        if (c == '!') {
            next = fgetc(input);
            if (next == '=') {
                emit("!=", "RELATIONAL_OPERATOR");
            } else {
                if (next != EOF) ungetc(next, input);
                emit("!", "LOGICAL_OPERATOR");
            }
            continue;
        }

        if (c == '<' || c == '>') {
            char operator_text[3] = { (char)c, '\0', '\0' };
            next = fgetc(input);

            if (next == '=') {
                operator_text[1] = '=';
                emit(operator_text, "RELATIONAL_OPERATOR");
            } else {
                if (next != EOF) ungetc(next, input);
                emit(operator_text, "RELATIONAL_OPERATOR");
            }
            continue;
        }

        if (c == '&' || c == '|') {
            next = fgetc(input);

            if (next == c) {
                char operator_text[3] = { (char)c, (char)c, '\0' };
                emit(operator_text, "LOGICAL_OPERATOR");
            } else {
                if (next != EOF) ungetc(next, input);
                emit_char(c, "UNKNOWN");
            }
            continue;
        }

        if (c == '+' || c == '-' || c == '*' || c == '%') {
            emit_char(c, "ARITHMETIC_OPERATOR");
            continue;
        }

        if (c == '(' || c == ')' || c == '{' || c == '}' ||
            c == '[' || c == ']' || c == ';' || c == ',') {
            emit_char(c, "DELIMITER");
            continue;
        }

        emit_char(c, "UNKNOWN");
    }

    fclose(input);
    return 0;
}