
#include <stdio.h>
#include <ctype.h>
#include <string.h>

#define MAX_LEXEME 128

int is_keyword(const char *word) {
    return strcmp(word, "int") == 0 ||
           strcmp(word, "if") == 0 ||
           strcmp(word, "else") == 0 ||
           strcmp(word, "while") == 0 ||
           strcmp(word, "return") == 0;
}

void print_token(const char *lexeme, const char *token_type) {
    printf("%s,%s\n", lexeme, token_type);
}

int is_identifier_start(int ch) {
    return isalpha((unsigned char)ch) || ch == '_';
}

int is_identifier_part(int ch) {
    return isalnum((unsigned char)ch) || ch == '_';
}

int is_delimiter(int ch) {
    return strchr("(){}[];,", ch) != NULL;
}

int scan_file(FILE *input) {
    int ch;

    while ((ch = fgetc(input)) != EOF) {
        char lexeme[MAX_LEXEME + 1];
        int next;

        if (isspace((unsigned char)ch)) {
            continue;
        }

        /* Handle comments and division operator. */
        if (ch == '/') {
            next = fgetc(input);

            if (next == '/') {
                while ((ch = fgetc(input)) != EOF && ch != '\n') {
                    /* Skip single-line comment. */
                }
                continue;
            }

            if (next == '*') {
                int previous = 0;
                int closed = 0;

                while ((ch = fgetc(input)) != EOF) {
                    if (previous == '*' && ch == '/') {
                        closed = 1;
                        break;
                    }
                    previous = ch;
                }

                if (!closed) {
                    fprintf(stderr, "error: unterminated block comment\n");
                    return 1;
                }

                continue;
            }

            if (next != EOF) {
                ungetc(next, input);
            }

            print_token("/", "ARITHMETIC_OPERATOR");
            continue;
        }

        /* Recognize identifiers and keywords. */
        if (is_identifier_start(ch)) {
            int length = 0;

            lexeme[length++] = (char)ch;

            while ((next = fgetc(input)) != EOF &&
                   is_identifier_part(next)) {
                if (length < MAX_LEXEME) {
                    lexeme[length++] = (char)next;
                } else {
                    fprintf(stderr, "error: lexeme exceeds maximum length\n");
                    return 1;
                }
            }

            if (next != EOF) {
                ungetc(next, input);
            }

            lexeme[length] = '\0';

            if (is_keyword(lexeme)) {
                print_token(lexeme, "KEYWORD");
            } else {
                print_token(lexeme, "IDENTIFIER");
            }

            continue;
        }

        /* Recognize integer literals. */
        if (isdigit((unsigned char)ch)) {
            int length = 0;

            lexeme[length++] = (char)ch;

            while ((next = fgetc(input)) != EOF &&
                   isdigit((unsigned char)next)) {
                if (length < MAX_LEXEME) {
                    lexeme[length++] = (char)next;
                } else {
                    fprintf(stderr, "error: lexeme exceeds maximum length\n");
                    return 1;
                }
            }

            if (next != EOF) {
                ungetc(next, input);
            }

            lexeme[length] = '\0';
            print_token(lexeme, "INTEGER");
            continue;
        }

        /* Recognize relational, logical, and assignment operators. */
        if (ch == '=' || ch == '!' || ch == '<' || ch == '>' ||
            ch == '&' || ch == '|') {
            next = fgetc(input);

            if ((ch == '=' && next == '=') ||
                (ch == '!' && next == '=') ||
                (ch == '<' && next == '=') ||
                (ch == '>' && next == '=')) {
                lexeme[0] = (char)ch;
                lexeme[1] = (char)next;
                lexeme[2] = '\0';

                print_token(lexeme, "RELATIONAL_OPERATOR");
            } else if ((ch == '&' && next == '&') ||
                       (ch == '|' && next == '|')) {
                lexeme[0] = (char)ch;
                lexeme[1] = (char)next;
                lexeme[2] = '\0';

                print_token(lexeme, "LOGICAL_OPERATOR");
            } else {
                if (next != EOF) {
                    ungetc(next, input);
                }

                lexeme[0] = (char)ch;
                lexeme[1] = '\0';

                if (ch == '=') {
                    print_token(lexeme, "ASSIGNMENT_OPERATOR");
                } else if (ch == '!') {
                    print_token(lexeme, "LOGICAL_OPERATOR");
                } else if (ch == '<' || ch == '>') {
                    print_token(lexeme, "RELATIONAL_OPERATOR");
                } else {
                    print_token(lexeme, "UNKNOWN");
                }
            }

            continue;
        }

        /* Recognize arithmetic operators. */
        if (ch == '+' || ch == '-' || ch == '*' || ch == '%') {
            lexeme[0] = (char)ch;
            lexeme[1] = '\0';
            print_token(lexeme, "ARITHMETIC_OPERATOR");
            continue;
        }

        /* Recognize delimiters. */
        if (is_delimiter(ch)) {
            lexeme[0] = (char)ch;
            lexeme[1] = '\0';
            print_token(lexeme, "DELIMITER");
            continue;
        }

        /* Report any other character as UNKNOWN. */
        lexeme[0] = (char)ch;
        lexeme[1] = '\0';
        print_token(lexeme, "UNKNOWN");
    }

    if (ferror(input)) {
        fprintf(stderr, "error: failed while reading input file\n");
        return 1;
    }

    return 0;
}

int main(int argc, char **argv) {
    FILE *input;
    int result;

    if (argc != 2) {
        fprintf(stderr, "usage: lexical_analyzer input.c\n");
        return 2;
    }

    input = fopen(argv[1], "rb");

    if (input == NULL) {
        perror("input");
        return 2;
    }

    result = scan_file(input);

    if (fclose(input) != 0) {
        perror("input");
        return 2;
    }

    return result;
}
