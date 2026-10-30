#include <stdio.h>
#include <string.h>
#include <ctype.h>

#define MAX_LEXEME_LEN 256

static int is_whitespace(int ch) {
    return ch == ' ' || ch == '\t' || ch == '\r' || ch == '\n';
}

static int is_identifier_start(int ch) {
    return isalpha((unsigned char)ch) || ch == '_';
}

static int is_identifier_part(int ch) {
    return isalnum((unsigned char)ch) || ch == '_';
}

static int is_keyword(const char *lexeme) {
    return strcmp(lexeme, "int") == 0 ||
           strcmp(lexeme, "if") == 0 ||
           strcmp(lexeme, "else") == 0 ||
           strcmp(lexeme, "while") == 0 ||
           strcmp(lexeme, "return") == 0;
}

static int is_delimiter(int ch) {
    return ch == '(' || ch == ')' ||
           ch == '{' || ch == '}' ||
           ch == '[' || ch == ']' ||
           ch == ';' || ch == ',';
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <source_file>\n", argc > 0 ? argv[0] : "lexical_analyzer");
        return 1;
    }

    FILE *fp = fopen(argv[1], "r");
    if (fp == NULL) {
        fprintf(stderr, "Error: Cannot open file '%s'.\n", argv[1]);
        return 1;
    }

    int ch;
    while ((ch = fgetc(fp)) != EOF) {
        if (is_whitespace(ch)) {
            continue;
        }

        if (is_identifier_start(ch)) {
            char lexeme[MAX_LEXEME_LEN];
            int len = 0;
            lexeme[len++] = (char)ch;

            int next = fgetc(fp);
            while (next != EOF && is_identifier_part(next)) {
                if (len < MAX_LEXEME_LEN - 1) {
                    lexeme[len++] = (char)next;
                }
                next = fgetc(fp);
            }
            if (next != EOF) {
                ungetc(next, fp);
            }
            lexeme[len] = '\0';

            if (is_keyword(lexeme)) {
                printf("%s,KEYWORD\n", lexeme);
            } else {
                printf("%s,IDENTIFIER\n", lexeme);
            }
            continue;
        }

        if (isdigit((unsigned char)ch)) {
            char lexeme[MAX_LEXEME_LEN];
            int len = 0;
            lexeme[len++] = (char)ch;

            int next = fgetc(fp);
            while (next != EOF && isdigit((unsigned char)next)) {
                if (len < MAX_LEXEME_LEN - 1) {
                    lexeme[len++] = (char)next;
                }
                next = fgetc(fp);
            }
            if (next != EOF) {
                ungetc(next, fp);
            }
            lexeme[len] = '\0';

            printf("%s,INTEGER\n", lexeme);
            continue;
        }

        if (ch == '/') {
            int next = fgetc(fp);
            if (next == '/') {
                int c;
                while ((c = fgetc(fp)) != EOF && c != '\n') {
                }
            } else if (next == '*') {
                int prev = 0;
                int c;
                int closed = 0;
                while ((c = fgetc(fp)) != EOF) {
                    if (prev == '*' && c == '/') {
                        closed = 1;
                        break;
                    }
                    prev = c;
                }
                if (!closed) {
                    fprintf(stderr, "Error: Unclosed block comment.\n");
                    fclose(fp);
                    return 1;
                }
            } else {
                if (next != EOF) {
                    ungetc(next, fp);
                }
                printf("/,ARITHMETIC_OPERATOR\n");
            }
            continue;
        }

        if (ch == '=') {
            int next = fgetc(fp);
            if (next == '=') {
                printf("==,RELATIONAL_OPERATOR\n");
            } else {
                if (next != EOF) {
                    ungetc(next, fp);
                }
                printf("=,ASSIGNMENT_OPERATOR\n");
            }
            continue;
        }

        if (ch == '!') {
            int next = fgetc(fp);
            if (next == '=') {
                printf("!=,RELATIONAL_OPERATOR\n");
            } else {
                if (next != EOF) {
                    ungetc(next, fp);
                }
                printf("!,LOGICAL_OPERATOR\n");
            }
            continue;
        }

        if (ch == '<') {
            int next = fgetc(fp);
            if (next == '=') {
                printf("<=,RELATIONAL_OPERATOR\n");
            } else {
                if (next != EOF) {
                    ungetc(next, fp);
                }
                printf("<,RELATIONAL_OPERATOR\n");
            }
            continue;
        }

        if (ch == '>') {
            int next = fgetc(fp);
            if (next == '=') {
                printf(">=,RELATIONAL_OPERATOR\n");
            } else {
                if (next != EOF) {
                    ungetc(next, fp);
                }
                printf(">,RELATIONAL_OPERATOR\n");
            }
            continue;
        }

        if (ch == '&') {
            int next = fgetc(fp);
            if (next == '&') {
                printf("&&,LOGICAL_OPERATOR\n");
            } else {
                if (next != EOF) {
                    ungetc(next, fp);
                }
                printf("&,UNKNOWN\n");
            }
            continue;
        }

        if (ch == '|') {
            int next = fgetc(fp);
            if (next == '|') {
                printf("||,LOGICAL_OPERATOR\n");
            } else {
                if (next != EOF) {
                    ungetc(next, fp);
                }
                printf("|,UNKNOWN\n");
            }
            continue;
        }

        if (ch == '+' || ch == '-' || ch == '*' || ch == '%') {
            printf("%c,ARITHMETIC_OPERATOR\n", ch);
            continue;
        }

        if (is_delimiter(ch)) {
            printf("%c,DELIMITER\n", ch);
            continue;
        }

        printf("%c,UNKNOWN\n", ch);
    }

    fclose(fp);
    return 0;
}
