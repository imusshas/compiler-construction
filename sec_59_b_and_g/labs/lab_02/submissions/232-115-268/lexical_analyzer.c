#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int is_keyword(const char *s) {
    return strcmp(s, "int") == 0 ||
           strcmp(s, "if") == 0 ||
           strcmp(s, "else") == 0 ||
           strcmp(s, "while") == 0 ||
           strcmp(s, "return") == 0;
}

static int is_ident_start(int c) {
    return (c >= 'A' && c <= 'Z') ||
           (c >= 'a' && c <= 'z') || c == '_';
}

static int is_ident_part(int c) {
    return is_ident_start(c) || (c >= '0' && c <= '9');
}

static int emit(const char *lexeme, const char *kind) {
    return printf("%s,%s\n", lexeme, kind) < 0 ? -1 : 0;
}

int main(int argc, char **argv) {
    FILE *f;
    int c;

    if (argc != 2) {
        fprintf(stderr, "usage: lexical_analyzer input.c\n");
        return 2;
    }

    f = fopen(argv[1], "rb");
    if (f == NULL) {
        perror("input");
        return 2;
    }

    while ((c = fgetc(f)) != EOF) {
        char op[3] = {0, 0, 0};
        int d;

        if (isspace((unsigned char)c))
            continue;

        /* Skip comments or tokenize division. */
        if (c == '/') {
            d = fgetc(f);

            if (d == '/') {
                while ((c = fgetc(f)) != EOF && c != '\n') {}
                continue;
            }

            if (d == '*') {
                int previous = 0;
                int closed = 0;

                while ((c = fgetc(f)) != EOF) {
                    if (previous == '*' && c == '/') {
                        closed = 1;
                        break;
                    }
                    previous = c;
                }

                if (!closed) {
                    fprintf(stderr, "error: unclosed block comment\n");
                    fclose(f);
                    return 1;
                }
                continue;
            }

            if (d != EOF)
                ungetc(d, f);

            if (emit("/", "ARITHMETIC_OPERATOR") != 0) {
                fclose(f);
                return 2;
            }
            continue;
        }

        /* Keywords and identifiers. */
        if (is_ident_start(c)) {
            size_t cap = 32, len = 0;
            char *name = malloc(cap);

            if (name == NULL) {
                fclose(f);
                return 2;
            }

            name[len++] = (char)c;

            while ((d = fgetc(f)) != EOF && is_ident_part(d)) {
                if (len + 1 >= cap) {
                    char *grown;
                    cap *= 2;
                    grown = realloc(name, cap);
                    if (grown == NULL) {
                        free(name);
                        fclose(f);
                        return 2;
                    }
                    name = grown;
                }
                name[len++] = (char)d;
            }

            if (d != EOF)
                ungetc(d, f);

            name[len] = '\0';

            if (emit(name, is_keyword(name) ? "KEYWORD" : "IDENTIFIER") != 0) {
                free(name);
                fclose(f);
                return 2;
            }

            free(name);
            continue;
        }

        /* Integer literals. */
        if (c >= '0' && c <= '9') {
            size_t cap = 32, len = 0;
            char *number = malloc(cap);

            if (number == NULL) {
                fclose(f);
                return 2;
            }

            number[len++] = (char)c;

            while ((d = fgetc(f)) != EOF && d >= '0' && d <= '9') {
                if (len + 1 >= cap) {
                    char *grown;
                    cap *= 2;
                    grown = realloc(number, cap);
                    if (grown == NULL) {
                        free(number);
                        fclose(f);
                        return 2;
                    }
                    number = grown;
                }
                number[len++] = (char)d;
            }

            if (d != EOF)
                ungetc(d, f);

            number[len] = '\0';

            if (emit(number, "INTEGER") != 0) {
                free(number);
                fclose(f);
                return 2;
            }

            free(number);
            continue;
        }

        /* Relational and assignment operators. */
        if (c == '=' || c == '!' || c == '<' || c == '>') {
            d = fgetc(f);

            if (d == '=') {
                op[0] = (char)c;
                op[1] = '=';
                if (emit(op, "RELATIONAL_OPERATOR") != 0) {
                    fclose(f);
                    return 2;
                }
            } else {
                if (d != EOF)
                    ungetc(d, f);

                op[0] = (char)c;

                if (c == '=') {
                    if (emit(op, "ASSIGNMENT_OPERATOR") != 0) {
                        fclose(f);
                        return 2;
                    }
                } else if (c == '!') {
                    if (emit(op, "LOGICAL_OPERATOR") != 0) {
                        fclose(f);
                        return 2;
                    }
                } else {
                    if (emit(op, "RELATIONAL_OPERATOR") != 0) {
                        fclose(f);
                        return 2;
                    }
                }
            }
            continue;
        }

        /* Logical operators: && and ||. */
        if (c == '&' || c == '|') {
            d = fgetc(f);

            if (d == c) {
                op[0] = (char)c;
                op[1] = (char)c;

                if (emit(op, "LOGICAL_OPERATOR") != 0) {
                    fclose(f);
                    return 2;
                }
            } else {
                if (d != EOF)
                    ungetc(d, f);

                op[0] = (char)c;

                if (emit(op, "UNKNOWN") != 0) {
                    fclose(f);
                    return 2;
                }
            }
            continue;
        }

        /* Arithmetic operators. */
        if (c == '+' || c == '-' || c == '*' || c == '%') {
            op[0] = (char)c;
            if (emit(op, "ARITHMETIC_OPERATOR") != 0) {
                fclose(f);
                return 2;
            }
            continue;
        }

        /* Delimiters. */
        if (strchr("(){}[];,", c) != NULL) {
            op[0] = (char)c;
            if (emit(op, "DELIMITER") != 0) {
                fclose(f);
                return 2;
            }
            continue;
        }

        /* Any other single ASCII character. */
        if (c >= 0 && c <= 127) {
            op[0] = (char)c;
            if (emit(op, "UNKNOWN") != 0) {
                fclose(f);
                return 2;
            }
        } else {
            fprintf(stderr, "error: non-ASCII byte in input\n");
            fclose(f);
            return 1;
        }
    }

    if (ferror(f)) {
        perror("input read");
        fclose(f);
        return 2;
    }

    if (fclose(f) != 0) {
        perror("input close");
        return 2;
    }

    return 0;
}