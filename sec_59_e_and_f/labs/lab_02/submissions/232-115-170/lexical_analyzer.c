
#include <stdio.h>

int is_letter(int c) {
    return (c >= 'A' && c <= 'Z') ||
           (c >= 'a' && c <= 'z');
}

int is_digit(int c) {
    return c >= '0' && c <= '9';
}

int is_start(int c) {
    return is_letter(c) || c == '_';
}

int is_part(int c) {
    return is_start(c) || is_digit(c);
}

int is_space(int c) {
    return c == ' ' || c == '\t' ||
           c == '\n' || c == '\r' ||
           c == '\v' || c == '\f';
}

int is_keyword(const char *s) {
    const char *words[] = {
        "int", "if", "else", "while", "return"
    };

    for (int i = 0; i < 5; i++) {
        int j = 0;
        while (s[j] == words[i][j] && s[j] != '\0') {
            j++;
        }
        if (s[j] == '\0' && words[i][j] == '\0') {
            return 1;
        }
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
        if (is_space(c)) {
            continue;
        }

        if (is_start(c)) {
            char word[129];
            int n = 0;

            do {
                if (n < 128) {
                    word[n++] = (char)c;
                }
                c = fgetc(input);
            } while (c != EOF && is_part(c));

            if (c != EOF) {
                ungetc(c, input);
            }

            word[n] = '\0';
            emit(word, is_keyword(word) ? "KEYWORD" : "IDENTIFIER");
            continue;
        }

        if (is_digit(c)) {
            char number[129];
            int n = 0;

            do {
                if (n < 128) {
                    number[n++] = (char)c;
                }
                c = fgetc(input);
            } while (c != EOF && is_digit(c));

            if (c != EOF) {
                ungetc(c, input);
            }

            number[n] = '\0';
            emit(number, "INTEGER");
            continue;
        }

        if (c == '/') {
            int d = fgetc(input);

            if (d == '/') {
                while ((c = fgetc(input)) != EOF && c != '\n') {
                }
                continue;
            }

            if (d == '*') {
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

            if (d != EOF) {
                ungetc(d, input);
            }

            emit("/", "ARITHMETIC_OPERATOR");
            continue;
        }

        if (c == '=' || c == '!' || c == '<' || c == '>' ||
            c == '&' || c == '|') {
            int d = fgetc(input);

            if (c == '=' && d == '=') {
                emit("==", "RELATIONAL_OPERATOR");
            } else if (c == '!' && d == '=') {
                emit("!=", "RELATIONAL_OPERATOR");
            } else if (c == '<' && d == '=') {
                emit("<=", "RELATIONAL_OPERATOR");
            } else if (c == '>' && d == '=') {
                emit(">=", "RELATIONAL_OPERATOR");
            } else if (c == '&' && d == '&') {
                emit("&&", "LOGICAL_OPERATOR");
            } else if (c == '|' && d == '|') {
                emit("||", "LOGICAL_OPERATOR");
            } else {
                if (d != EOF) {
                    ungetc(d, input);
                }

                if (c == '=') {
                    emit("=", "ASSIGNMENT_OPERATOR");
                } else if (c == '!') {
                    emit("!", "LOGICAL_OPERATOR");
                } else if (c == '<' || c == '>') {
                    char op[2] = {(char)c, '\0'};
                    emit(op, "RELATIONAL_OPERATOR");
                } else {
                    char op[2] = {(char)c, '\0'};
                    emit(op, "UNKNOWN");
                }
            }
            continue;
        }

        if (c == '+' || c == '-' || c == '*' || c == '%') {
            char op[2] = {(char)c, '\0'};
            emit(op, "ARITHMETIC_OPERATOR");
            continue;
        }

        if (c == '(' || c == ')' || c == '{' || c == '}' ||
            c == '[' || c == ']' || c == ';' || c == ',') {
            char delimiter[2] = {(char)c, '\0'};
            emit(delimiter, "DELIMITER");
            continue;
        }

        {
            char unknown[2] = {(char)c, '\0'};
            emit(unknown, "UNKNOWN");
        }
    }

    if (ferror(input)) {
        fprintf(stderr, "error: could not read input file\n");
        fclose(input);
        return 2;
    }

    fclose(input);
    return 0;
}
