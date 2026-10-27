#include <stdio.h>
#include <string.h>
#include <ctype.h>

static int is_keyword(const char *s) {
    const char *kw[] = {"int", "if", "else", "while", "return"};
    for (int i = 0; i < 5; i++)
        if (strcmp(s, kw[i]) == 0) return 1;
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

    int c = fgetc(input);
    while (c != EOF) {
        /* 1. skip whitespace */
        if (isspace(c)) {
            c = fgetc(input);
            continue;
        }

        /* 2. identifier or keyword */
        if (isalpha(c) || c == '_') {
            char buf[512];
            int n = 0;
            while (c != EOF && (isalnum(c) || c == '_')) {
                if (n < 511) buf[n++] = (char)c;
                c = fgetc(input);
            }
            buf[n] = '\0';
            printf("%s,%s\n", buf, is_keyword(buf) ? "KEYWORD" : "IDENTIFIER");
            continue;
        }

        /* 3. integer */
        if (isdigit(c)) {
            char buf[512];
            int n = 0;
            while (c != EOF && isdigit(c)) {
                if (n < 511) buf[n++] = (char)c;
                c = fgetc(input);
            }
            buf[n] = '\0';
            printf("%s,INTEGER\n", buf);
            continue;
        }

        /* 4. slash: comment or division */
        if (c == '/') {
            int next = fgetc(input);
            if (next == '/') {               /* line comment */
                c = next;
                while (c != '\n' && c != EOF) c = fgetc(input);
                continue;
            } else if (next == '*') {        /* block comment */
                int prev = 0, closed = 0;
                c = fgetc(input);
                while (c != EOF) {
                    if (prev == '*' && c == '/') { closed = 1; break; }
                    prev = c;
                    c = fgetc(input);
                }
                if (!closed) {
                    fprintf(stderr, "error: unclosed block comment\n");
                    fclose(input);
                    return 1;
                }
                c = fgetc(input);
                continue;
            } else {                         /* division */
                printf("/,ARITHMETIC_OPERATOR\n");
                c = next;
                continue;
            }
        }

        /* 5. operators that may have two characters */
        if (c == '=' || c == '!' || c == '<' || c == '>' || c == '&' || c == '|') {
            int first = c;
            int next = fgetc(input);
            if (first == '=') {
                if (next == '=') { printf("==,RELATIONAL_OPERATOR\n"); c = fgetc(input); }
                else { printf("=,ASSIGNMENT_OPERATOR\n"); c = next; }
            } else if (first == '!') {
                if (next == '=') { printf("!=,RELATIONAL_OPERATOR\n"); c = fgetc(input); }
                else { printf("!,LOGICAL_OPERATOR\n"); c = next; }
            } else if (first == '<' || first == '>') {
                if (next == '=') { printf("%c=,RELATIONAL_OPERATOR\n", first); c = fgetc(input); }
                else { printf("%c,RELATIONAL_OPERATOR\n", first); c = next; }
            } else { /* & or | */
                if (next == first) { printf("%c%c,LOGICAL_OPERATOR\n", first, first); c = fgetc(input); }
                else { printf("%c,UNKNOWN\n", first); c = next; }
            }
            continue;
        }

        /* 6. single-character tokens */
        if (c == '+' || c == '-' || c == '*' || c == '%')
            printf("%c,ARITHMETIC_OPERATOR\n", c);
        else if (c == '(' || c == ')' || c == '{' || c == '}' ||
                 c == '[' || c == ']' || c == ';' || c == ',')
            printf("%c,DELIMITER\n", c);
        else
            printf("%c,UNKNOWN\n", c);
        c = fgetc(input);
    }

    fclose(input);
    return 0;
}
