
#include <stdio.h>
#include <ctype.h>
#include <string.h>

#define MAX_LEXEME 128

// Check whether a word is one of the supported keywords.
int is_keyword(const char *word) {
    return strcmp(word, "int") == 0 ||
           strcmp(word, "if") == 0 ||
           strcmp(word, "else") == 0 ||
           strcmp(word, "while") == 0 ||
           strcmp(word, "return") == 0;
}

// Print a token in the required format.
void print_token(const char *lexeme, const char *token) {
    printf("%s,%s\n", lexeme, token);
}

int main(int argc, char **argv) {
    // Step 1: Check the command-line arguments.
    if (argc != 2) {
        fprintf(stderr, "usage: lexical_analyzer input.c\n");
        return 2;
    }

    // Step 2: Open the input file.
    FILE *input = fopen(argv[1], "rb");

    if (input == NULL) {
        perror("input");
        return 2;
    }

    int c;

    // Step 3: Read the file one character at a time.
    while ((c = fgetc(input)) != EOF) {

        // Ignore whitespace.
        if (isspace((unsigned char)c)) {
            continue;
        }

        char lexeme[MAX_LEXEME + 1];
        int i = 0;

        // Step 4: Recognize identifiers and keywords.
        if (isalpha((unsigned char)c) || c == '_') {
            do {
                lexeme[i++] = (char)c;
                c = fgetc(input);
            } while (c != EOF &&
                     (isalnum((unsigned char)c) || c == '_'));

            lexeme[i] = '\0';

            if (c != EOF) {
                ungetc(c, input);
            }

            if (is_keyword(lexeme)) {
                print_token(lexeme, "KEYWORD");
            } else {
                print_token(lexeme, "IDENTIFIER");
            }

            continue;
        }

        // Step 5: Recognize integers.
        if (isdigit((unsigned char)c)) {
            do {
                lexeme[i++] = (char)c;
                c = fgetc(input);
            } while (c != EOF && isdigit((unsigned char)c));

            lexeme[i] = '\0';

            if (c != EOF) {
                ungetc(c, input);
            }

            print_token(lexeme, "INTEGER");
            continue;
        }

        // Step 6: Handle comments and division.
        if (c == '/') {
            int next = fgetc(input);

            if (next == '/') {
                // Skip a line comment.
                while ((c = fgetc(input)) != EOF && c != '\n') {
                }
                continue;
            }

            if (next == '*') {
                // Skip a block comment.
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

                continue;
            }

            // A slash not starting a comment is division.
            if (next != EOF) {
                ungetc(next, input);
            }

            print_token("/", "ARITHMETIC_OPERATOR");
            continue;
        }

        // Step 7: Recognize two-character operators.
        if (c == '=' || c == '!' || c == '<' ||
            c == '>' || c == '&' || c == '|') {

            int next = fgetc(input);

            if ((c == '=' && next == '=') ||
                (c == '!' && next == '=') ||
                (c == '<' && next == '=') ||
                (c == '>' && next == '=') ||
                (c == '&' && next == '&') ||
                (c == '|' && next == '|')) {

                lexeme[0] = (char)c;
                lexeme[1] = (char)next;
                lexeme[2] = '\0';

                if (c == '&' || c == '|') {
                    print_token(lexeme, "LOGICAL_OPERATOR");
                } else {
                    print_token(lexeme, "RELATIONAL_OPERATOR");
                }

                continue;
            }

            if (next != EOF) {
                ungetc(next, input);
            }

            lexeme[0] = (char)c;
            lexeme[1] = '\0';

            if (c == '=') {
                print_token(lexeme, "ASSIGNMENT_OPERATOR");
            } else if (c == '!') {
                print_token(lexeme, "LOGICAL_OPERATOR");
            } else if (c == '<' || c == '>') {
                print_token(lexeme, "RELATIONAL_OPERATOR");
            } else {
                print_token(lexeme, "UNKNOWN");
            }

            continue;
        }

        // Step 8: Recognize arithmetic operators.
        if (c == '+' || c == '-' || c == '*' || c == '%') {
            lexeme[0] = (char)c;
            lexeme[1] = '\0';
            print_token(lexeme, "ARITHMETIC_OPERATOR");
            continue;
        }

        // Step 9: Recognize delimiters.
        if (c == '(' || c == ')' ||
            c == '{' || c == '}' ||
            c == '[' || c == ']' ||
            c == ';' || c == ',') {

            lexeme[0] = (char)c;
            lexeme[1] = '\0';
            print_token(lexeme, "DELIMITER");
            continue;
        }

        // Step 10: Handle unsupported characters.
        lexeme[0] = (char)c;
        lexeme[1] = '\0';
        print_token(lexeme, "UNKNOWN");
    }

    // Step 11: Close the file and finish successfully.
    fclose(input);
    return 0;
}
