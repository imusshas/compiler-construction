/* Lab 02: implement the section manual's lexical contract.
   Read argv[1] with fopen/fgetc. Do not hard-code public.c or its token list.
   You may add helper functions. This starter is deliberately unfinished. */
#include <stdio.h>
#include <string.h>

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

        /* Skip whitespace */
        if (c == ' ' || c == '\t' || c == '\n' || c == '\r') {
            continue;
        }

        /* Skip single-line comments */
        if (c == '/') {
            int next = fgetc(input);

            if (next == '/') {
                while ((c = fgetc(input)) != EOF && c != '\n') {
                    /* Skip comment */
                }
                continue;
            }

            if (next == '*') {
                int previous = 0;

                while ((c = fgetc(input)) != EOF) {
                    if (previous == '*' && c == '/') {
                        break;
                    }
                    previous = c;
                }
                continue;
            }

            if (next != EOF) {
                ungetc(next, input);
            }

            printf("/,OPERATOR\n");
            continue;
        }

        /* Read identifiers and keywords */
        if ((c >= 'a' && c <= 'z') ||
            (c >= 'A' && c <= 'Z') || c == '_') {

            char lexeme[256];
            int i = 0;

            lexeme[i++] = (char)c;

            while ((c = fgetc(input)) != EOF &&
                   ((c >= 'a' && c <= 'z') ||
                    (c >= 'A' && c <= 'Z') ||
                    (c >= '0' && c <= '9') || c == '_')) {

                if (i < 255) {
                    lexeme[i++] = (char)c;
                }
            }

            lexeme[i] = '\0';

            if (c != EOF) {
                ungetc(c, input);
            }

            const char *keywords[] = {
                "int", "char", "float", "double", "void",
                "if", "else", "while", "for", "do",
                "return", "break", "continue", "switch",
                "case", "default", "struct", "const",
                "long", "short", "unsigned", "signed",
                "sizeof", "static", "extern"
            };

            int is_keyword = 0;

            for (int k = 0;
                 k < (int)(sizeof(keywords) / sizeof(keywords[0]));
                 k++) {
                if (strcmp(lexeme, keywords[k]) == 0) {
                    is_keyword = 1;
                    break;
                }
            }

            printf("%s,%s\n", lexeme,
                   is_keyword ? "KEYWORD" : "IDENTIFIER");
            continue;
        }

        /* Read integer numbers */
        if (c >= '0' && c <= '9') {
            char lexeme[256];
            int i = 0;

            lexeme[i++] = (char)c;

            while ((c = fgetc(input)) != EOF &&
                   c >= '0' && c <= '9') {
                if (i < 255) {
                    lexeme[i++] = (char)c;
                }
            }

            lexeme[i] = '\0';

            if (c != EOF) {
                ungetc(c, input);
            }

            printf("%s,NUMBER\n", lexeme);
            continue;
        }

        /* Operators */
        if (c == '+' || c == '-' || c == '*' ||
            c == '=' || c == '<' || c == '>' ||
            c == '!' || c == '%' || c == '&' ||
            c == '|' || c == '^') {

            char lexeme[3];
            lexeme[0] = (char)c;
            lexeme[1] = '\0';
            lexeme[2] = '\0';

            int next = fgetc(input);

            if (next != EOF &&
                (next == '=' ||
                 (next == c && (c == '+' || c == '-' ||
                                c == '&' || c == '|')))) {
                lexeme[1] = (char)next;
                lexeme[2] = '\0';
            } else if (next != EOF) {
                ungetc(next, input);
            }

            printf("%s,OPERATOR\n", lexeme);
            continue;
        }

        /* Punctuation / delimiters */
        if (c == ';' || c == ',' || c == '(' ||
            c == ')' || c == '{' || c == '}' ||
            c == '[' || c == ']' || c == '.') {

            printf("%c,PUNCTUATION\n", c);
            continue;
        }

        /* String literals */
        if (c == '"') {
            char lexeme[512];
            int i = 0;

            lexeme[i++] = '"';

            while ((c = fgetc(input)) != EOF) {
                if (i < 510) {
                    lexeme[i++] = (char)c;
                }

                if (c == '\\') {
                    int escaped = fgetc(input);
                    if (escaped == EOF) {
                        break;
                    }
                    if (i < 510) {
                        lexeme[i++] = (char)escaped;
                    }
                    continue;
                }

                if (c == '"') {
                    break;
                }
            }

            lexeme[i] = '\0';
            printf("%s,STRING\n", lexeme);
            continue;
        }

        /* Character literals */
        if (c == '\'') {
            char lexeme[32];
            int i = 0;

            lexeme[i++] = '\'';

            while ((c = fgetc(input)) != EOF) {
                if (i < 30) {
                    lexeme[i++] = (char)c;
                }

                if (c == '\\') {
                    int escaped = fgetc(input);
                    if (escaped == EOF) {
                        break;
                    }
                    if (i < 30) {
                        lexeme[i++] = (char)escaped;
                    }
                    continue;
                }

                if (c == '\'') {
                    break;
                }
            }

            lexeme[i] = '\0';
            printf("%s,CHARACTER\n", lexeme);
        }
    }
    fclose(input);
    return 0;
}
