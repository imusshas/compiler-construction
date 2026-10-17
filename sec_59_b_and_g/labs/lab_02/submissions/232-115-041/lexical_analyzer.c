
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

int is_keyword(const char *s) {
    const char *keywords[] = {"int", "if", "else", "while", "return"};

    for (int i = 0; i < 5; i++) {
        if (strcmp(s, keywords[i]) == 0)
            return 1;
    }
    return 0;
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s input_file\n", argv[0]);
        return 1;
    }

    FILE *fp = fopen(argv[1], "r");
    if (fp == NULL) {
        perror("Cannot open input file");
        return 1;
    }

    int c;

    while ((c = fgetc(fp)) != EOF) {
        if (isspace((unsigned char)c))
            continue;

        /* Ignore comments and recognize division */
        if (c == '/') {
            int d = fgetc(fp);

            if (d == '/') {
                while ((c = fgetc(fp)) != EOF && c != '\n') {}
                continue;
            }

            if (d == '*') {
                int previous = 0;
                int closed = 0;

                while ((c = fgetc(fp)) != EOF) {
                    if (previous == '*' && c == '/') {
                        closed = 1;
                        break;
                    }
                    previous = c;
                }

                if (!closed) {
                    fprintf(stderr, "Error: unclosed block comment\n");
                    fclose(fp);
                    return 1;
                }
                continue;
            }

            if (d != EOF)
                ungetc(d, fp);

            printf("/,ARITHMETIC_OPERATOR\n");
            continue;
        }

        /* Identifier or keyword */
        if (isalpha((unsigned char)c) || c == '_') {
            char s[1024];
            int n = 0;

            s[n++] = (char)c;

            while ((c = fgetc(fp)) != EOF &&
                   (isalnum((unsigned char)c) || c == '_')) {
                if (n < (int)sizeof(s) - 1)
                    s[n++] = (char)c;
            }

            if (c != EOF)
                ungetc(c, fp);

            s[n] = '\0';

            if (is_keyword(s))
                printf("%s,KEYWORD\n", s);
            else
                printf("%s,IDENTIFIER\n", s);

            continue;
        }

        /* Integer */
        if (isdigit((unsigned char)c)) {
            char s[1024];
            int n = 0;

            s[n++] = (char)c;

            while ((c = fgetc(fp)) != EOF &&
                   isdigit((unsigned char)c)) {
                if (n < (int)sizeof(s) - 1)
                    s[n++] = (char)c;
            }

            if (c != EOF)
                ungetc(c, fp);

            s[n] = '\0';
            printf("%s,INTEGER\n", s);
            continue;
        }

        /* Relational and assignment operators */
        if (c == '=' || c == '!' || c == '<' || c == '>') {
            int d = fgetc(fp);

            if (d == '=') {
                printf("%c=,RELATIONAL_OPERATOR\n", c);
            } else {
                if (d != EOF)
                    ungetc(d, fp);

                if (c == '=')
                    printf("=,ASSIGNMENT_OPERATOR\n");
                else if (c == '!')
                    printf("!,LOGICAL_OPERATOR\n");
                else
                    printf("%c,RELATIONAL_OPERATOR\n", c);
            }
            continue;
        }

        /* Logical operators */
        if (c == '&' || c == '|') {
            int d = fgetc(fp);

            if (d == c) {
                printf("%c%c,LOGICAL_OPERATOR\n", c, d);
            } else {
                if (d != EOF)
                    ungetc(d, fp);
                printf("%c,UNKNOWN\n", c);
            }
            continue;
        }

        /* Arithmetic operators */
        if (c == '+' || c == '-' || c == '*' || c == '%') {
            printf("%c,ARITHMETIC_OPERATOR\n", c);
            continue;
        }

        /* Delimiters */
        if (strchr("(){}[];,", c) != NULL) {
            printf("%c,DELIMITER\n", c);
            continue;
        }

        /* Any other single character */
        printf("%c,UNKNOWN\n", c);
    }

    fclose(fp);
    return 0;
}
