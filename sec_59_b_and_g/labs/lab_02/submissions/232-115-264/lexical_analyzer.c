/* Lab 02: lexical analyzer for a small C subset.
   Reads argv[1] with fopen/fgetc and prints one "lexeme,TOKEN" line per token. */
#include <stdio.h>
#include <string.h>

#define MAX_LEXEME 1024

static int is_space(int c) {
    return c == ' ' || c == '\t' || c == '\r' || c == '\n' || c == '\f' || c == '\v';
}
static int is_letter(int c) {
    return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || c == '_';
}
static int is_digit(int c) {
    return c >= '0' && c <= '9';
}
static int is_keyword(const char *s) {
    static const char *keywords[] = {"int", "if", "else", "while", "return"};
    for (size_t i = 0; i < sizeof keywords / sizeof keywords[0]; i++)
        if (strcmp(s, keywords[i]) == 0)
            return 1;
    return 0;
}

static void emit(const char *lexeme, const char *token) {
    printf("%s,%s\n", lexeme, token);
}

/* Emit a one- or two-character lexeme. */
static void emit_chars(int c1, int c2, const char *token) {
    char buf[3];
    buf[0] = (char)c1;
    if (c2 != 0) {
        buf[1] = (char)c2;
        buf[2] = '\0';
    } else {
        buf[1] = '\0';
    }
    emit(buf, token);
}

/* Called after the opening comment marker was consumed. Returns 1 if closed, 0 if EOF reached first. */
static int skip_block_comment(FILE *in) {
    int prev = 0, c;
    while ((c = fgetc(in)) != EOF) {
        if (prev == '*' && c == '/')
            return 1;
        prev = c;
    }
    return 0;
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

    int status = 0;
    int c;
    while ((c = fgetc(input)) != EOF) {
        if (is_space(c))
            continue;

        if (is_letter(c)) {                       /* identifier or keyword */
            char buf[MAX_LEXEME];
            size_t n = 0;
            while (c != EOF && (is_letter(c) || is_digit(c))) {
                if (n < MAX_LEXEME - 1)
                    buf[n++] = (char)c;
                c = fgetc(input);
            }
            buf[n] = '\0';
            if (c != EOF)
                ungetc(c, input);                 /* put back look-ahead */
            emit(buf, is_keyword(buf) ? "KEYWORD" : "IDENTIFIER");
        } else if (is_digit(c)) {                 /* integer */
            char buf[MAX_LEXEME];
            size_t n = 0;
            while (c != EOF && is_digit(c)) {
                if (n < MAX_LEXEME - 1)
                    buf[n++] = (char)c;
                c = fgetc(input);
            }
            buf[n] = '\0';
            if (c != EOF)
                ungetc(c, input);
            emit(buf, "INTEGER");
        } else if (c == '/') {                    /* comment or division */
            int next = fgetc(input);
            if (next == '/') {
                int d;
                while ((d = fgetc(input)) != EOF && d != '\n')
                    ;
            } else if (next == '*') {
                if (!skip_block_comment(input)) {
                    fprintf(stderr, "error: unclosed block comment\n");
                    status = 1;
                }
            } else {
                if (next != EOF)
                    ungetc(next, input);
                emit("/", "ARITHMETIC_OPERATOR");
            }
        } else if (c == '=' || c == '<' || c == '>' || c == '!') {
            int next = fgetc(input);
            if (next == '=') {
                emit_chars(c, '=', "RELATIONAL_OPERATOR");
            } else {
                if (next != EOF)
                    ungetc(next, input);
                if (c == '=')
                    emit("=", "ASSIGNMENT_OPERATOR");
                else if (c == '!')
                    emit("!", "LOGICAL_OPERATOR");
                else
                    emit_chars(c, 0, "RELATIONAL_OPERATOR");
            }
        } else if (c == '&' || c == '|') {
            int next = fgetc(input);
            if (next == c) {
                emit_chars(c, c, "LOGICAL_OPERATOR");
            } else {
                if (next != EOF)
                    ungetc(next, input);
                emit_chars(c, 0, "UNKNOWN");
            }
        } else if (c == '+' || c == '-' || c == '*' || c == '%') {
            emit_chars(c, 0, "ARITHMETIC_OPERATOR");
        } else if (c == '(' || c == ')' || c == '{' || c == '}' ||
                   c == '[' || c == ']' || c == ';' || c == ',') {
            emit_chars(c, 0, "DELIMITER");
        } else {
            emit_chars(c, 0, "UNKNOWN");
        }
    }

    fclose(input);
    return status;
}
