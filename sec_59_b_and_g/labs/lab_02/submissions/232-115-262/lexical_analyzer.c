#include <stdio.h>
#include <string.h>
#include <ctype.h>

#define MAXLEN 129

static int is_keyword(const char *s) {
    static const char *kw[] = {"int", "if", "else", "while", "return"};
    for (int i = 0; i < 5; i++)
        if (strcmp(s, kw[i]) == 0) return 1;
    return 0;
}

static void emit(const char *lexeme, const char *token) {
    printf("%s,%s\n", lexeme, token);
}

/* Look ahead one char; returns 1 and consumes it if it equals want,
   otherwise puts it back and returns 0. */
static int match(FILE *in, int want) {
    int next = fgetc(in);
    if (next == want) return 1;
    if (next != EOF) ungetc(next, in);
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

    int c;
    while ((c = fgetc(input)) != EOF) {
        char buf[MAXLEN + 1];
        int n = 0;

        if (isspace(c)) continue;

        if (isalpha(c) || c == '_') {              /* identifier / keyword */
            do {
                if (n < MAXLEN) buf[n++] = (char)c;
                c = fgetc(input);
            } while (c != EOF && (isalnum(c) || c == '_'));
            buf[n] = '\0';
            if (c != EOF) ungetc(c, input);
            emit(buf, is_keyword(buf) ? "KEYWORD" : "IDENTIFIER");
        }
        else if (isdigit(c)) {                     /* integer */
            do {
                if (n < MAXLEN) buf[n++] = (char)c;
                c = fgetc(input);
            } while (c != EOF && isdigit(c));
            buf[n] = '\0';
            if (c != EOF) ungetc(c, input);
            emit(buf, "INTEGER");
        }
        else if (c == '/') {                       /* comment or division */
            int next = fgetc(input);
            if (next == '/') {
                int d;
                while ((d = fgetc(input)) != EOF && d != '\n') ;
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
                emit("/", "ARITHMETIC_OPERATOR");
            }
        }
        else if (c == '=') {
            if (match(input, '=')) emit("==", "RELATIONAL_OPERATOR");
            else emit("=", "ASSIGNMENT_OPERATOR");
        }
        else if (c == '!') {
            if (match(input, '=')) emit("!=", "RELATIONAL_OPERATOR");
            else emit("!", "LOGICAL_OPERATOR");
        }
        else if (c == '<') {
            if (match(input, '=')) emit("<=", "RELATIONAL_OPERATOR");
            else emit("<", "RELATIONAL_OPERATOR");
        }
        else if (c == '>') {
            if (match(input, '=')) emit(">=", "RELATIONAL_OPERATOR");
            else emit(">", "RELATIONAL_OPERATOR");
        }
        else if (c == '&') {
            if (match(input, '&')) emit("&&", "LOGICAL_OPERATOR");
            else emit("&", "UNKNOWN");
        }
        else if (c == '|') {
            if (match(input, '|')) emit("||", "LOGICAL_OPERATOR");
            else emit("|", "UNKNOWN");
        }
        else if (strchr("+-*%", c)) {
            char s[2] = { (char)c, '\0' };
            emit(s, "ARITHMETIC_OPERATOR");
        }
        else if (strchr("(){}[];,", c)) {
            char s[2] = { (char)c, '\0' };
            emit(s, "DELIMITER");
        }
        else {
            char s[2] = { (char)c, '\0' };
            emit(s, "UNKNOWN");
        }
    }

    fclose(input);
    return 0;
}