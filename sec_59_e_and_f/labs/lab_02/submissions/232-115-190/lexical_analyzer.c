#include <stdio.h>
#include <string.h>

#define MAXLEN 256

static int is_id_start(int c) {
    return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || c == '_';
}

static int is_digit(int c) { return c >= '0' && c <= '9'; }

static int is_space(int c) {
    return c == ' ' || c == '\t' || c == '\r' || c == '\n' ||
           c == '\f' || c == '\v';
}

static int is_keyword(const char *s) {
    return strcmp(s, "int") == 0 || strcmp(s, "if") == 0 ||
           strcmp(s, "else") == 0 || strcmp(s, "while") == 0 ||
           strcmp(s, "return") == 0;
}

static void emit(const char *lexeme, const char *token) {
    printf("%s,%s\n", lexeme, token);
}

static void emit_char(int c, const char *token) {
    printf("%c,%s\n", c, token);
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

        if (is_id_start(c)) {
            char buf[MAXLEN];
            size_t n = 0;
            while (c != EOF && (is_id_start(c) || is_digit(c))) {
                if (n < MAXLEN - 1) buf[n++] = (char)c;
                c = fgetc(input);
            }
            buf[n] = '\0';
            if (c != EOF) ungetc(c, input);
            emit(buf, is_keyword(buf) ? "KEYWORD" : "IDENTIFIER");
            continue;
        }

        if (is_digit(c)) {
            char buf[MAXLEN];
            size_t n = 0;
            while (c != EOF && is_digit(c)) {
                if (n < MAXLEN - 1) buf[n++] = (char)c;
                c = fgetc(input);
            }
            buf[n] = '\0';
            if (c != EOF) ungetc(c, input);
            emit(buf, "INTEGER");
            continue;
        }

        switch (c) {
        case '/': {
            int next = fgetc(input);
            if (next == '/') {
                int d;
                while ((d = fgetc(input)) != EOF && d != '\n') {
                }
            } else if (next == '*') {
                int prev = 0, d, closed = 0;
                while ((d = fgetc(input)) != EOF) {
                    if (prev == '*' && d == '/') { closed = 1; break; }
                    prev = d;
                }
                if (!closed) {
                    fprintf(stderr, "error: unclosed block comment\n");
                    fclose(input);
                    return 1;
                }
            } else {
                if (next != EOF) ungetc(next, input);
                emit_char('/', "ARITHMETIC_OPERATOR");
            }
            break;
        }
        case '+': case '-': case '*': case '%':
            emit_char(c, "ARITHMETIC_OPERATOR");
            break;
        case '=': case '<': case '>': case '!': {
            int next = fgetc(input);
            if (next == '=') {
                char two[3] = { (char)c, '=', '\0' };
                emit(two, "RELATIONAL_OPERATOR");
            } else {
                if (next != EOF) ungetc(next, input);
                if (c == '=') emit_char(c, "ASSIGNMENT_OPERATOR");
                else if (c == '!') emit_char(c, "LOGICAL_OPERATOR");
                else emit_char(c, "RELATIONAL_OPERATOR");
            }
            break;
        }
        case '&': case '|': {
            int next = fgetc(input);
            if (next == c) {
                char two[3] = { (char)c, (char)c, '\0' };
                emit(two, "LOGICAL_OPERATOR");
            } else {
                if (next != EOF) ungetc(next, input);
                emit_char(c, "UNKNOWN");
            }
            break;
        }
        case '(': case ')': case '{': case '}':
        case '[': case ']': case ';': case ',':
            emit_char(c, "DELIMITER");
            break;
        default:
            emit_char(c, "UNKNOWN");
            break;
        }
    }

    fclose(input);
    return 0;
}