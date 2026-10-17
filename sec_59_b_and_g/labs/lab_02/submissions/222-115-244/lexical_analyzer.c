#include <stdio.h>

#define MAX_LEXEME 256

static int is_alpha_(int c) {
    return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || c == '_';
}

static int is_digit(int c) {
    return c >= '0' && c <= '9';
}

static int is_space(int c) {
    return c == ' ' || c == '\t' || c == '\r' || c == '\n' || c == '\f' || c == '\v';
}

static int is_keyword(const char *s) {
    const char *kw[] = {"int", "if", "else", "while", "return"};
    for (size_t i = 0; i < sizeof(kw) / sizeof(kw[0]); i++) {
        const char *a = s, *b = kw[i];
        while (*a && *a == *b) { a++; b++; }
        if (*a == '\0' && *b == '\0') return 1;
    }
    return 0;
}

static void emit(const char *lexeme, const char *token) {
    printf("%s,%s\n", lexeme, token);
}

/* Read the next char; if it equals 'expected' consume it and return 1,
   otherwise put it back (unless EOF) and return 0. */
static int match_next(FILE *fp, int expected) {
    int n = fgetc(fp);
    if (n == expected) return 1;
    if (n != EOF) ungetc(n, fp);
    return 0;
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <input_file>\n",
                argc > 0 ? argv[0] : "lexical_analyzer");
        return 1;
    }

    FILE *fp = fopen(argv[1], "rb");
    if (fp == NULL) {
        fprintf(stderr, "Error: cannot open file '%s'\n", argv[1]);
        return 1;
    }

    int c;
    while ((c = fgetc(fp)) != EOF) {
        if (is_space(c)) continue;

        /* Identifier or keyword */
        if (is_alpha_(c)) {
            char buf[MAX_LEXEME];
            size_t len = 0;
            buf[len++] = (char)c;
            while ((c = fgetc(fp)) != EOF && (is_alpha_(c) || is_digit(c))) {
                if (len < MAX_LEXEME - 1) buf[len++] = (char)c;
            }
            if (c != EOF) ungetc(c, fp);
            buf[len] = '\0';
            emit(buf, is_keyword(buf) ? "KEYWORD" : "IDENTIFIER");
            continue;
        }

        /* Integer */
        if (is_digit(c)) {
            char buf[MAX_LEXEME];
            size_t len = 0;
            buf[len++] = (char)c;
            while ((c = fgetc(fp)) != EOF && is_digit(c)) {
                if (len < MAX_LEXEME - 1) buf[len++] = (char)c;
            }
            if (c != EOF) ungetc(c, fp);
            buf[len] = '\0';
            emit(buf, "INTEGER");
            continue;
        }

        switch (c) {
        case '/': {
            int n = fgetc(fp);
            if (n == '/') {                       /* line comment */
                while ((c = fgetc(fp)) != EOF && c != '\n') { }
            } else if (n == '*') {                /* block comment */
                int prev = 0, closed = 0;
                while ((c = fgetc(fp)) != EOF) {
                    if (prev == '*' && c == '/') { closed = 1; break; }
                    prev = c;
                }
                if (!closed) {
                    fprintf(stderr, "Error: unclosed block comment\n");
                    fclose(fp);
                    return 1;
                }
            } else {                              /* division */
                if (n != EOF) ungetc(n, fp);
                emit("/", "ARITHMETIC_OPERATOR");
            }
            break;
        }
        case '+': emit("+", "ARITHMETIC_OPERATOR"); break;
        case '-': emit("-", "ARITHMETIC_OPERATOR"); break;
        case '*': emit("*", "ARITHMETIC_OPERATOR"); break;
        case '%': emit("%", "ARITHMETIC_OPERATOR"); break;

        case '=':
            if (match_next(fp, '=')) emit("==", "RELATIONAL_OPERATOR");
            else emit("=", "ASSIGNMENT_OPERATOR");
            break;
        case '!':
            if (match_next(fp, '=')) emit("!=", "RELATIONAL_OPERATOR");
            else emit("!", "LOGICAL_OPERATOR");
            break;
        case '<':
            if (match_next(fp, '=')) emit("<=", "RELATIONAL_OPERATOR");
            else emit("<", "RELATIONAL_OPERATOR");
            break;
        case '>':
            if (match_next(fp, '=')) emit(">=", "RELATIONAL_OPERATOR");
            else emit(">", "RELATIONAL_OPERATOR");
            break;
        case '&':
            if (match_next(fp, '&')) emit("&&", "LOGICAL_OPERATOR");
            else emit("&", "UNKNOWN");
            break;
        case '|':
            if (match_next(fp, '|')) emit("||", "LOGICAL_OPERATOR");
            else emit("|", "UNKNOWN");
            break;

        case '(': emit("(", "DELIMITER"); break;
        case ')': emit(")", "DELIMITER"); break;
        case '{': emit("{", "DELIMITER"); break;
        case '}': emit("}", "DELIMITER"); break;
        case '[': emit("[", "DELIMITER"); break;
        case ']': emit("]", "DELIMITER"); break;
        case ';': emit(";", "DELIMITER"); break;
        case ',': emit(",", "DELIMITER"); break;

        default: {
            char buf[2] = { (char)c, '\0' };
            emit(buf, "UNKNOWN");
            break;
        }
        }
    }

    fclose(fp);
    return 0;
}