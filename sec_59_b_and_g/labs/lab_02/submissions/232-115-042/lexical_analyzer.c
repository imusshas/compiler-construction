
#include <stdio.h>
#include <ctype.h>
#include <string.h>

int keyword(char s[]) {
    char *words[] = {"int", "if", "else", "while", "return"};

    for (int i = 0; i < 5; i++) {
        if (strcmp(s, words[i]) == 0)
            return 1;
    }
    return 0;
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: ./lexer filename\n");
        return 1;
    }

    FILE *fp = fopen(argv[1], "r");

    if (fp == NULL) {
        fprintf(stderr, "Cannot open file\n");
        return 1;
    }

    int c;

    while ((c = fgetc(fp)) != EOF) {
        if (isspace(c))
            continue;

        if (isalpha(c) || c == '_') {
            char s[100];
            int i = 0;

            do {
                if (i < 99)
                    s[i++] = c;
                c = fgetc(fp);
            } while (c != EOF && (isalnum(c) || c == '_'));

            s[i] = '\0';

            if (c != EOF)
                ungetc(c, fp);

            if (keyword(s))
                printf("%s,KEYWORD\n", s);
            else
                printf("%s,IDENTIFIER\n", s);
        }

        else if (isdigit(c)) {
            char s[100];
            int i = 0;

            do {
                if (i < 99)
                    s[i++] = c;
                c = fgetc(fp);
            } while (c != EOF && isdigit(c));

            s[i] = '\0';

            if (c != EOF)
                ungetc(c, fp);

            printf("%s,INTEGER\n", s);
        }

        else if (c == '/') {
            int next = fgetc(fp);

            if (next == '/') {
                while ((c = fgetc(fp)) != EOF && c != '\n');
            }
            else if (next == '*') {
                int prev = 0, closed = 0;

                while ((c = fgetc(fp)) != EOF) {
                    if (prev == '*' && c == '/') {
                        closed = 1;
                        break;
                    }
                    prev = c;
                }

                if (!closed) {
                    fprintf(stderr, "Unclosed comment\n");
                    fclose(fp);
                    return 1;
                }
            }
            else {
                if (next != EOF)
                    ungetc(next, fp);
                printf("/,ARITHMETIC_OPERATOR\n");
            }
        }

        else if (c == '=' || c == '!' || c == '<' || c == '>') {
            int next = fgetc(fp);

            if (next == '=') {
                printf("%c=,RELATIONAL_OPERATOR\n", c);
            }
            else {
                if (next != EOF)
                    ungetc(next, fp);

                if (c == '=')
                    printf("=,ASSIGNMENT_OPERATOR\n");
                else if (c == '!')
                    printf("!,LOGICAL_OPERATOR\n");
                else
                    printf("%c,RELATIONAL_OPERATOR\n", c);
            }
        }

        else if (c == '&' || c == '|') {
            int next = fgetc(fp);

            if (next == c)
                printf("%c%c,LOGICAL_OPERATOR\n", c, c);
            else {
                if (next != EOF)
                    ungetc(next, fp);
                printf("%c,UNKNOWN\n", c);
            }
        }

        else if (c == '+' || c == '-' || c == '*' || c == '%') {
            printf("%c,ARITHMETIC_OPERATOR\n", c);
        }

        else if (c == '(' || c == ')' || c == '{' || c == '}' ||
                 c == '[' || c == ']' || c == ';' || c == ',') {
            printf("%c,DELIMITER\n", c);
        }

        else {
            printf("%c,UNKNOWN\n", c);
        }
    }

    fclose(fp);
    return 0;
}

