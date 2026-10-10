#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

int is_keyword(const char *s) {
    return strcmp(s, "int") == 0 ||
           strcmp(s, "if") == 0 ||
           strcmp(s, "else") == 0 ||
           strcmp(s, "while") == 0 ||
           strcmp(s, "return") == 0;
}

int main(int argc, char **argv) {
    if (argc!= 2) {
        fprintf(stderr, "usage: lexical_analyzer input.c\n");
        return 2;
    }
    FILE *input = fopen(argv[1], "rb");
    if (input == NULL) {
        perror("input");
        return 2;
    }

    int c;
    while ((c = fgetc(input))!= EOF) {
        if (c == ' ' || c == '\t' || c == '\r' || c == '\n') continue;

        // comments
        if (c == '/') {
            int nxt = fgetc(input);
            if (nxt == '/') {
                while ((c = fgetc(input))!= EOF && c!= '\n');
                continue;
            } else if (nxt == '*') {
                int prev = 0, curr, closed = 0;
                while ((curr = fgetc(input))!= EOF) {
                    if (prev == '*' && curr == '/') { closed = 1; break; }
                    prev = curr;
                }
                if (!closed) {
                    fprintf(stderr, "Error: unclosed block comment\n");
                    fclose(input);
                    return 1;
                }
                continue;
            } else {
                if (nxt!= EOF) ungetc(nxt, input);
                printf("/,ARITHMETIC_OPERATOR\n");
                continue;
            }
        }

        // identifier / keyword
        if (isalpha(c) || c == '_') {
            char lex[129]; int len = 0;
            lex[len++] = (char)c;
            while ((c = fgetc(input))!= EOF && (isalnum(c) || c == '_')) {
                if (len < 128) lex[len++] = (char)c;
            }
            lex[len] = '\0';
            if (c!= EOF) ungetc(c, input);
            if (is_keyword(lex)) printf("%s,KEYWORD\n", lex);
            else printf("%s,IDENTIFIER\n", lex);
            continue;
        }

        // integer
        if (isdigit(c)) {
            char lex[129]; int len = 0;
            lex[len++] = (char)c;
            while ((c = fgetc(input))!= EOF && isdigit(c)) {
                if (len < 128) lex[len++] = (char)c;
            }
            lex[len] = '\0';
            if (c!= EOF) ungetc(c, input);
            printf("%s,INTEGER\n", lex);
            continue;
        }

        // two-char operators
        if (c == '=' || c == '!' || c == '<' || c == '>' || c == '&' || c == '|') {
            int nxt = fgetc(input);
            if (c == '=' && nxt == '=') { printf("==,RELATIONAL_OPERATOR\n"); continue; }
            if (c == '!' && nxt == '=') { printf("!=,RELATIONAL_OPERATOR\n"); continue; }
            if (c == '<' && nxt == '=') { printf("<=,RELATIONAL_OPERATOR\n"); continue; }
            if (c == '>' && nxt == '=') { printf(">=,RELATIONAL_OPERATOR\n"); continue; }
            if (c == '&' && nxt == '&') { printf("&&,LOGICAL_OPERATOR\n"); continue; }
            if (c == '|' && nxt == '|') { printf("||,LOGICAL_OPERATOR\n"); continue; }
            if (nxt!= EOF) ungetc(nxt, input);
            if (c == '=') printf("=,ASSIGNMENT_OPERATOR\n");
            else if (c == '<' || c == '>') printf("%c,RELATIONAL_OPERATOR\n", c);
            else if (c == '!') printf("!,LOGICAL_OPERATOR\n");
            else printf("%c,UNKNOWN\n", c);
            continue;
        }

        if (c == '+' || c == '-' || c == '*' || c == '%') {
            printf("%c,ARITHMETIC_OPERATOR\n", c);
            continue;
        }

        if (c == '(' || c == ')' || c == '{' || c == '}' ||
            c == '[' || c == ']' || c == ';' || c == ',') {
            printf("%c,DELIMITER\n", c);
            continue;
        }

        printf("%c,UNKNOWN\n", c);
    }

    fclose(input);
    return 0;
}