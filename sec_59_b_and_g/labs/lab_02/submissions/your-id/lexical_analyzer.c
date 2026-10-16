#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

int isKeyword(const char *word) {
    const char *keywords[] = {
        "int", "if", "else", "while", "return"
    };
    int i;
    for (i = 0; i < 5; i++) {
        if (strcmp(word, keywords[i]) == 0) {
            return 1;
        }
    }
    return 0;
}
void printToken(const char *lexeme, const char *type) {
    printf("%s,%s\n", lexeme, type);
}
int main(int argc, char *argv[]) {
    FILE *fp;
    int c, next;
    char lexeme[129];
    int i;
    if (argc != 2) {
        fprintf(stderr, "Usage: %s filename\n", argv[0]);
        return 1;
    }
    fp = fopen(argv[1], "r");
    if (fp == NULL) {
        fprintf(stderr, "Error: Cannot open file\n");
        return 1;
    }
    while ((c = fgetc(fp)) != EOF) {
        if (isspace((unsigned char)c)) {
            continue;
        }
        if (isalpha((unsigned char)c) || c == '_') {
            i = 0;
            lexeme[i++] = (char)c;
            while ((c = fgetc(fp)) != EOF &&
                   (isalnum((unsigned char)c) || c == '_')) {
                lexeme[i++] = (char)c;
            }
            lexeme[i] = '\0';
            if (c != EOF) {
                ungetc(c, fp);
            }
            if (isKeyword(lexeme)) {
                printToken(lexeme, "KEYWORD");
            } else {
                printToken(lexeme, "IDENTIFIER");
            }
        }
        else if (isdigit((unsigned char)c)) {
            i = 0;
            lexeme[i++] = (char)c;
            while ((c = fgetc(fp)) != EOF &&
                   isdigit((unsigned char)c)) {
                lexeme[i++] = (char)c;
            }
            lexeme[i] = '\0';
            if (c != EOF) {
                ungetc(c, fp);
            }
            printToken(lexeme, "INTEGER");
        }
        else if (c == '/') {
            next = fgetc(fp);
            if (next == '/') {
                while ((c = fgetc(fp)) != EOF && c != '\n') {
                }
            }
            else if (next == '*') {
                int closed = 0;
                int previous = 0;
                while ((c = fgetc(fp)) != EOF) {
                    if (previous == '*' && c == '/') {
                        closed = 1;
                        break;
                    }
                    previous = c;
                }
                if (!closed) {
                    fprintf(stderr, "Error: Unclosed block comment\n");
                    fclose(fp);
                    return 1;
                }
            }
            else {
                if (next != EOF) {
                    ungetc(next, fp);
                }
                printToken("/", "ARITHMETIC_OPERATOR");
            }
        }
        else if (c == '=' || c == '!' ||
                 c == '<' || c == '>') {
            lexeme[0] = (char)c;
            lexeme[1] = '\0';
            next = fgetc(fp);
            if (next == '=' &&
                (c == '=' || c == '!' ||
                 c == '<' || c == '>')) {
                lexeme[1] = '=';
                lexeme[2] = '\0';
                printToken(lexeme, "RELATIONAL_OPERATOR");
            }
            else {
                if (next != EOF) {
                    ungetc(next, fp);
                }
                if (c == '=') {
                    printToken(lexeme, "ASSIGNMENT_OPERATOR");
                } else if (c == '!') {
                    printToken(lexeme, "LOGICAL_OPERATOR");
                } else {
                    printToken(lexeme, "RELATIONAL_OPERATOR");
                }
            }
        }
        else if (c == '&' || c == '|') {
            next = fgetc(fp);

            if (next == c) {
                lexeme[0] = (char)c;
                lexeme[1] = (char)c;
                lexeme[2] = '\0';
                printToken(lexeme, "LOGICAL_OPERATOR");
            } else {
                if (next != EOF) {
                    ungetc(next, fp);
                }

                lexeme[0] = (char)c;
                lexeme[1] = '\0';
                printToken(lexeme, "UNKNOWN");
            }
        }
        else if (c == '+' || c == '-' ||
                 c == '*' || c == '%') {
            lexeme[0] = (char)c;
            lexeme[1] = '\0';
            printToken(lexeme, "ARITHMETIC_OPERATOR");
        }
        else if (c == '(' || c == ')' ||
                 c == '{' || c == '}' ||
                 c == '[' || c == ']' ||
                 c == ';' || c == ',') {
            lexeme[0] = (char)c;
            lexeme[1] = '\0';
            printToken(lexeme, "DELIMITER");
        }
        else {
            lexeme[0] = (char)c;
            lexeme[1] = '\0';
            printToken(lexeme, "UNKNOWN");
        }
    }
    fclose(fp);
    return 0;
}
