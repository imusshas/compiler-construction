/* Lab 02: implement the section manual's lexical contract.
   Read argv[1] with fopen/fgetc. Do not hard-code public.c or its token list.
   You may add helper functions. This starter is deliberately unfinished. */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

int is_keyword(const char *word) {
    const char *keywords[] = {"int", "if", "else", "while", "return"};
    int i;

    for (i = 0; i < 5; i++) {
        if (strcmp(word, keywords[i]) == 0)
            return 1;
    }
    return 0;
}

void print_token(const char *lexeme, const char *type) {
    printf("%s,%s\n", lexeme, type);
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

    int c, next;
    char token[129];
    int i;

    while ((c = fgetc(input)) != EOF) {
        if (isspace((unsigned char)c))
            continue;

        if (isalpha((unsigned char)c) || c == '_') {
            i = 0;
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
        }
        else if (isdigit((unsigned char)c)) {
            i = 0;
            do {
                if (i < 128)
                    token[i++] = (char)c;
                c = fgetc(input);
            } while (c != EOF && isdigit((unsigned char)c));

            token[i] = '\0';
            if (c != EOF)
                ungetc(c, input);

            print_token(token, "INTEGER");
        }
        else if (c == '/') {
            next = fgetc(input);

            if (next == '/') {
                while ((c = fgetc(input)) != EOF && c != '\n')
                    ;
            }
            else if (next == '*') {
                int previous = 0;
                int closed = 0;

                while ((c = fgetc(input)) != EOF) {
                    if (previous == '*' && c == '/') {
                        closed = 1;
                        break;
                    }
                    previous = c;
                }

                if (!closed) {
                    fprintf(stderr, "error: unclosed block comment\n");
                    fclose(input);
                    return 1;
                }
            }
            else {
                if (next != EOF)
                    ungetc(next, input);
                print_token("/", "ARITHMETIC_OPERATOR");
            }
        }
        else {
            next = fgetc(input);
            token[0] = (char)c;
            token[1] = '\0';

            if (c == '=' && next == '=') {
                print_token("==", "RELATIONAL_OPERATOR");
            }
            else if (c == '!' && next == '=') {
                print_token("!=", "RELATIONAL_OPERATOR");
            }
            else if (c == '<' && next == '=') {
                print_token("<=", "RELATIONAL_OPERATOR");
            }
            else if (c == '>' && next == '=') {
                print_token(">=", "RELATIONAL_OPERATOR");
            }
            else if (c == '&' && next == '&') {
                print_token("&&", "LOGICAL_OPERATOR");
            }
            else if (c == '|' && next == '|') {
                print_token("||", "LOGICAL_OPERATOR");
            }
            else {
                if (next != EOF)
                    ungetc(next, input);

                switch (c) {
                    case '=':
                        print_token(token, "ASSIGNMENT_OPERATOR");
                        break;
                    case '<':
                    case '>':
                        print_token(token, "RELATIONAL_OPERATOR");
                        break;
                    case '!':
                        print_token(token, "LOGICAL_OPERATOR");
                        break;
                    case '+':
                    case '-':
                    case '*':
                    case '%':
                        print_token(token, "ARITHMETIC_OPERATOR");
                        break;
                    case '(':
                    case ')':
                    case '{':
                    case '}':
                    case '[':
                    case ']':
                    case ';':
                    case ',':
                        print_token(token, "DELIMITER");
                        break;
                    default:
                        print_token(token, "UNKNOWN");
                }
            }
        }
    }

    fclose(input);
    return 0;
}
