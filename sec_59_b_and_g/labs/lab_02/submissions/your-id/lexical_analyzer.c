#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_LEXEME 129

static int is_keyword(const char *word) {
    const char *keywords[] = {"int", "if", "else", "while", "return"};
    size_t i;
    for (i = 0; i < sizeof(keywords) / sizeof(keywords[0]); i++) {
        if (strcmp(word, keywords[i]) == 0) return 1;
    }
    return 0;
}

static void print_token(const char *lexeme, const char *category) {
    printf("%s,%s\n", lexeme, category);
}

static int is_identifier_start(int c) {
    return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || c == '_';
}

static int is_identifier_part(int c) {
    return is_identifier_start(c) || (c >= '0' && c <= '9');
}

int main(int argc, char *argv[]) {
    FILE *file;
    int c;

    if (argc != 2) {
        fprintf(stderr, "Usage: %s <input-file>\n", argv[0]);
        return 1;
    }

    file = fopen(argv[1], "r");
    if (file == NULL) {
        fprintf(stderr, "Error: cannot open file '%s'\n", argv[1]);
        return 1;
    }

    while ((c = fgetc(file)) != EOF) {
        char lexeme[MAX_LEXEME];
        int next;

        if (c == ' ' || c == '\t' || c == '\r' || c == '\n') continue;

        if (is_identifier_start(c)) {
            size_t length = 0;
            do {
                if (length < MAX_LEXEME - 1) lexeme[length++] = (char)c;
                c = fgetc(file);
            } while (c != EOF && is_identifier_part(c));
            lexeme[length] = '\0';
            if (c != EOF) ungetc(c, file);
            print_token(lexeme, is_keyword(lexeme) ? "KEYWORD" : "IDENTIFIER");
            continue;
        }

        if (c >= '0' && c <= '9') {
            size_t length = 0;
            do {
                if (length < MAX_LEXEME - 1) lexeme[length++] = (char)c;
                c = fgetc(file);
            } while (c != EOF && c >= '0' && c <= '9');
            lexeme[length] = '\0';
            if (c != EOF) ungetc(c, file);
            print_token(lexeme, "INTEGER");
            continue;
        }

        if (c == '/') {
            next = fgetc(file);
            if (next == '/') {
                while ((c = fgetc(file)) != EOF && c != '\n') {}
                continue;
            }
            if (next == '*') {
                int previous = 0;
                int closed = 0;
                while ((c = fgetc(file)) != EOF) {
                    if (previous == '*' && c == '/') {
                        closed = 1;
                        break;
                    }
                    previous = c;
                }
                if (!closed) {
                    fprintf(stderr, "Error: unclosed block comment\n");
                    fclose(file);
                    return 1;
                }
                continue;
            }
            if (next != EOF) ungetc(next, file);
            print_token("/", "ARITHMETIC_OPERATOR");
            continue;
        }

        if (c == '=' || c == '!' || c == '<' || c == '>') {
            next = fgetc(file);
            if (next == '=' && c == '=') {
                print_token("==", "RELATIONAL_OPERATOR");
            } else if (next == '=' && c == '!') {
                print_token("!=", "RELATIONAL_OPERATOR");
            } else if (next == '=' && c == '<') {
                print_token("<=", "RELATIONAL_OPERATOR");
            } else if (next == '=' && c == '>') {
                print_token(">=", "RELATIONAL_OPERATOR");
            } else {
                if (next != EOF) ungetc(next, file);
                if (c == '=') print_token("=", "ASSIGNMENT_OPERATOR");
                else if (c == '!') print_token("!", "LOGICAL_OPERATOR");
                else if (c == '<' || c == '>') {
                    lexeme[0] = (char)c;
                    lexeme[1] = '\0';
                    print_token(lexeme, "RELATIONAL_OPERATOR");
                }
            }
            continue;
        }

        if (c == '&' || c == '|') {
            next = fgetc(file);
            if (next == c) {
                lexeme[0] = (char)c;
                lexeme[1] = (char)next;
                lexeme[2] = '\0';
                print_token(lexeme, "LOGICAL_OPERATOR");
            } else {
                if (next != EOF) ungetc(next, file);
                lexeme[0] = (char)c;
                lexeme[1] = '\0';
                print_token(lexeme, "UNKNOWN");
            }
            continue;
        }

        if (c == '+' || c == '-' || c == '*' || c == '%') {
            lexeme[0] = (char)c;
            lexeme[1] = '\0';
            print_token(lexeme, "ARITHMETIC_OPERATOR");
            continue;
        }

        if (c == '(' || c == ')' || c == '{' || c == '}' ||
            c == '[' || c == ']' || c == ';' || c == ',') {
            lexeme[0] = (char)c;
            lexeme[1] = '\0';
            print_token(lexeme, "DELIMITER");
            continue;
        }

        if ((unsigned char)c <= 127 && !isspace((unsigned char)c)) {
            lexeme[0] = (char)c;
            lexeme[1] = '\0';
            print_token(lexeme, "UNKNOWN");
        }
    }

    fclose(file);
    return 0;
}
