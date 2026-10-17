#include <stdio.h>
#include <string.h>

#define MAX_LEXEME 256

static int is_space(int c) {
    return c == ' ' || c == '\t' || c == '\r' || c == '\n';
}

static int is_letter(int c) {
    return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || c == '_';
}

static int is_digit(int c) {
    return c >= '0' && c <= '9';
}

static int is_keyword(const char *s) {
    const char *keywords[] = {"int", "if", "else", "while", "return"};
    size_t i;
    for (i = 0; i < sizeof(keywords) / sizeof(keywords[0]); i++) {
        if (strcmp(s, keywords[i]) == 0) {
            return 1;
        }
    }
    return 0;
}

static int match(FILE *fp, int expected) {
    int next = fgetc(fp);
    if (next == expected) {
        return 1;
    }
    if (next != EOF) {
        ungetc(next, fp);
    }
    return 0;
}

static void print_token(const char *lexeme, const char *label) {
    printf("%s,%s\n", lexeme, label);
}

int main(int argc, char *argv[]) {
    FILE *fp;
    int c;
    int status = 0;

    if (argc != 2) {
        fprintf(stderr, "Usage: %s <input_file>\n", argv[0]);
        return 1;
    }

    fp = fopen(argv[1], "r");
    if (fp == NULL) {
        fprintf(stderr, "Error: cannot open file '%s'\n", argv[1]);
        return 1;
    }

    while ((c = fgetc(fp)) != EOF) {
        if (is_space(c)) {
            continue;
        }

        if (is_letter(c)) {
            char buf[MAX_LEXEME];
            size_t len = 0;
            buf[len++] = (char)c;
            while ((c = fgetc(fp)) != EOF && (is_letter(c) || is_digit(c))) {
                if (len < MAX_LEXEME - 1) {
                    buf[len++] = (char)c;
                }
            }
            if (c != EOF) {
                ungetc(c, fp);
            }
            buf[len] = '\0';
            print_token(buf, is_keyword(buf) ? "KEYWORD" : "IDENTIFIER");
            continue;
        }

        if (is_digit(c)) {
            char buf[MAX_LEXEME];
            size_t len = 0;
            buf[len++] = (char)c;
            while ((c = fgetc(fp)) != EOF && is_digit(c)) {
                if (len < MAX_LEXEME - 1) {
                    buf[len++] = (char)c;
                }
            }
            if (c != EOF) {
                ungetc(c, fp);
            }
            buf[len] = '\0';
            print_token(buf, "INTEGER");
            continue;
        }

        switch (c) {
        case '/':
            if (match(fp, '/')) {
                while ((c = fgetc(fp)) != EOF && c != '\n') {
                }
            } else if (match(fp, '*')) {
                int prev = 0;
                int closed = 0;
                while ((c = fgetc(fp)) != EOF) {
                    if (prev == '*' && c == '/') {
                        closed = 1;
                        break;
                    }
                    prev = c;
                }
                if (!closed) {
                    fprintf(stderr, "Error: unclosed block comment\n");
                    status = 1;
                }
            } else {
                print_token("/", "ARITHMETIC_OPERATOR");
            }
            break;

        case '=':
            if (match(fp, '=')) {
                print_token("==", "RELATIONAL_OPERATOR");
            } else {
                print_token("=", "ASSIGNMENT_OPERATOR");
            }
            break;

        case '!':
            if (match(fp, '=')) {
                print_token("!=", "RELATIONAL_OPERATOR");
            } else {
                print_token("!", "LOGICAL_OPERATOR");
            }
            break;

        case '<':
            if (match(fp, '=')) {
                print_token("<=", "RELATIONAL_OPERATOR");
            } else {
                print_token("<", "RELATIONAL_OPERATOR");
            }
            break;

        case '>':
            if (match(fp, '=')) {
                print_token(">=", "RELATIONAL_OPERATOR");
            } else {
                print_token(">", "RELATIONAL_OPERATOR");
            }
            break;

        case '&':
            if (match(fp, '&')) {
                print_token("&&", "LOGICAL_OPERATOR");
            } else {
                print_token("&", "UNKNOWN");
            }
            break;

        case '|':
            if (match(fp, '|')) {
                print_token("||", "LOGICAL_OPERATOR");
            } else {
                print_token("|", "UNKNOWN");
            }
            break;

        case '+':
            print_token("+", "ARITHMETIC_OPERATOR");
            break;
        case '-':
            print_token("-", "ARITHMETIC_OPERATOR");
            break;
        case '*':
            print_token("*", "ARITHMETIC_OPERATOR");
            break;
        case '%':
            print_token("%", "ARITHMETIC_OPERATOR");
            break;

        case '(':
            print_token("(", "DELIMITER");
            break;
        case ')':
            print_token(")", "DELIMITER");
            break;
        case '{':
            print_token("{", "DELIMITER");
            break;
        case '}':
            print_token("}", "DELIMITER");
            break;
        case '[':
            print_token("[", "DELIMITER");
            break;
        case ']':
            print_token("]", "DELIMITER");
            break;
        case ';':
            print_token(";", "DELIMITER");
            break;
        case ',':
            print_token(",", "DELIMITER");
            break;

        default: {
            char one[2];
            one[0] = (char)c;
            one[1] = '\0';
            print_token(one, "UNKNOWN");
            break;
        }
        }
    }

    fclose(fp);
    return status;
}
