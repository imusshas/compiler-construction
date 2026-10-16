
#include <stdio.h>
#include <ctype.h>
#include <string.h>

int is_keyword(const char *s) {
    const char *keywords[] = {"int", "if", "else", "while", "return"};
    for (int i = 0; i < 5; i++) {
        if (strcmp(s, keywords[i]) == 0)
            return 1;
    }
    return 0;
}

void print_token(const char *s, const char *type) {
    printf("%s,%s\n", s, type);
}

int main(int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr, "Usage: lexical_analyzer input.c\n");
        return 1;
    }

    FILE *input = fopen(argv[1], "rb");
    if (!input) {
        perror("Input file");
        return 1;
    }

    int c;
    char token[129];

    while ((c = fgetc(input)) != EOF) {
        if (isspace((unsigned char)c))
            continue;

        if (isalpha((unsigned char)c) || c == '_') {
            int i = 0;
            do {
                if (i < 128)
                    token[i++] = (char)c;
                c = fgetc(input);
            } while (c != EOF &&
                     (isalnum((unsigned char)c) || c == '_'));

            token[i] = '\0';
            if (c != EOF)
                ungetc(c, input);

            print_token(token, is_keyword(token) ? "KEYWORD" : "IDENTIFIER");
        } else if (isdigit((unsigned char)c)) {
            int i = 0;
            do {
                if (i < 128)
                    token[i++] = (char)c;
                c = fgetc(input);
            } while (c != EOF && isdigit((unsigned char)c));

            token[i] = '\0';
            if (c != EOF)
                ungetc(c, input);

            print_token(token, "INTEGER");
        } else if (c == '/') {
            int next = fgetc(input);

            if (next == '/') {
                while ((c = fgetc(input)) != EOF && c != '\n')
                    ;
            } else if (next == '*') {
                int previous = 0, closed = 0;
                while ((c = fgetc(input)) != EOF) {
                    if (previous == '*' && c == '/') {
                        closed = 1;
                        break;
                    }
                    previous = c;
                }
                if (!closed) {
                    fprintf(stderr, "Error: unclosed block comment\n");
                    fclose(input);
                    return 1;
                }
            } else {
                if (next != EOF)
                    ungetc(next, input);
                print_token("/", "ARITHMETIC_OPERATOR");
            }
        } else {
            int next = fgetc(input);
            char op[3] = {(char)c, '\0', '\0'};
            const char *type = "UNKNOWN";
            int paired = 0;

            if (next != EOF) {
                if ((c == '=' && next == '=') ||
                    (c == '!' && next == '=') ||
                    (c == '<' && next == '=') ||
                    (c == '>' && next == '=') ||
                    (c == '&' && next == '&') ||
                    (c == '|' && next == '|')) {
                    op[1] = (char)next;
                    paired = 1;
                }
            }

            if (paired) {
                if (op[0] == '&' || op[0] == '|')
                    type = "LOGICAL_OPERATOR";
                else
                    type = "RELATIONAL_OPERATOR";
            } else if (next != EOF) {
                ungetc(next, input);
            }

            if (!paired) {
                if (c == '=') type = "ASSIGNMENT_OPERATOR";
                else if (c == '+' || c == '-' || c == '*' ||
                         c == '/' || c == '%')
                    type = "ARITHMETIC_OPERATOR";
                else if (c == '<' || c == '>')
                    type = "RELATIONAL_OPERATOR";
                else if (c == '!') type = "LOGICAL_OPERATOR";
                else if (c == '(' || c == ')' || c == '{' ||
                         c == '}' || c == '[' || c == ']' ||
                         c == ';' || c == ',')
                    type = "DELIMITER";
            }

            print_token(op, type);
        }
    }

    fclose(input);
    return 0;
}
