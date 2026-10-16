
#include <stdio.h>
#include <string.h>

int is_letter(int c) {
    return (c >= 'A' && c <= 'Z') ||
           (c >= 'a' && c <= 'z') || c == '_';
}

int is_digit(int c) {
    return c >= '0' && c <= '9';
}

int is_identifier_part(int c) {
    return is_letter(c) || is_digit(c);
}

int is_space(int c) {
    return c == ' ' || c == '\t' ||
           c == '\r' || c == '\n' ||
           c == '\v' || c == '\f';
}

void emit(const char *lexeme, const char *token) {
    printf("%s,%s\n", lexeme, token);
}

const char *keyword_type(const char *word) {
    if (strcmp(word, "int") == 0 ||
        strcmp(word, "if") == 0 ||
        strcmp(word, "else") == 0 ||
        strcmp(word, "while") == 0 ||
        strcmp(word, "return") == 0) {
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
        if (is_space(c)) {
            continue;
        }

        /* Skip comments or recognize division. */
        if (c == '/') {
            int next = fgetc(input);

            if (next == '/') {
                while ((c = fgetc(input)) != EOF && c != '\n') {
                    /* Skip line comment. */
                }
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
                            "error: unterminated block comment\n");
                    fclose(input);
                    return 2;
                }

                continue;
            }

            if (next != EOF) {
                ungetc(next, input);
            }

            emit("/", "ARITHMETIC_OPERATOR");
            continue;
        }

        /* Recognize identifiers and keywords. */
        if (is_letter(c)) {
            char word[129];
            int length = 0;

            word[length++] = (char)c;

            while ((c = fgetc(input)) != EOF &&
                   is_identifier_part(c)) {
                if (length >= 128) {
                    fprintf(stderr,
                            "error: identifier exceeds 128 characters\n");
                    fclose(input);
                    return 2;
                }

                word[length++] = (char)c;
            }

            if (c != EOF) {
                ungetc(c, input);
            }

            word[length] = '\0';
            emit(word, keyword_type(word));
            continue;
        }

        /* Recognize integers. */
        if (is_digit(c)) {
            char number[129];
            int length = 0;

            number[length++] = (char)c;

            while ((c = fgetc(input)) != EOF && is_digit(c)) {
                if (length >= 128) {
                    fprintf(stderr,
                            "error: integer exceeds 128 characters\n");
                    fclose(input);
                    return 2;
                }

                number[length++] = (char)c;
            }

            if (c != EOF) {
                ungetc(c, input);
            }

            number[length] = '\0';
            emit(number, "INTEGER");
            continue;
        }

        /* Recognize assignment and relational operators. */
        if (c == '=' || c == '!' || c == '<' || c == '>') {
            int next = fgetc(input);
            char lexeme[3] = {(char)c, '\0', '\0'};

            if (next == '=') {
                lexeme[1] = '=';
                lexeme[2] = '\0';
                emit(lexeme, "RELATIONAL_OPERATOR");
            } else {
                if (next != EOF) {
                    ungetc(next, input);
                }

                if (c == '=') {
                    emit(lexeme, "ASSIGNMENT_OPERATOR");
                } else if (c == '!') {
                    emit(lexeme, "LOGICAL_OPERATOR");
                } else {
                    emit(lexeme, "RELATIONAL_OPERATOR");
                }
            }

            continue;
        }

        /* Recognize logical operators && and ||. */
        if (c == '&' || c == '|') {
            int next = fgetc(input);

            if (next == c) {
                char lexeme[3] = {(char)c, (char)c, '\0'};
                emit(lexeme, "LOGICAL_OPERATOR");
            } else {
                if (next != EOF) {
                    ungetc(next, input);
                }

                char lexeme[2] = {(char)c, '\0'};
                emit(lexeme, "UNKNOWN");
            }

            continue;
        }

        /* Recognize arithmetic operators. */
        if (c == '+' || c == '-' || c == '*' || c == '%') {
            char lexeme[2] = {(char)c, '\0'};
            emit(lexeme, "ARITHMETIC_OPERATOR");
            continue;
        }

        /* Recognize delimiters. */
        if (c == '(' || c == ')' || c == '{' || c == '}' ||
            c == '[' || c == ']' || c == ';' || c == ',') {
            char lexeme[2] = {(char)c, '\0'};
            emit(lexeme, "DELIMITER");
            continue;
        }

        /* All other non-whitespace ASCII characters are unknown. */
        if (c >= 0 && c <= 127) {
            char lexeme[2] = {(char)c, '\0'};
            emit(lexeme, "UNKNOWN");
        }
    }

    if (ferror(input)) {
        fprintf(stderr, "error: failed to read input file\n");
        fclose(input);
        return 2;
    }

    fclose(input);
    return 0;
}
