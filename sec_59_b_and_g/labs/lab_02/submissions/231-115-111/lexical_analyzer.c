#include <ctype.h>
#include <stdio.h>
#include <string.h>

#define MAX_LEXEME 128

static int is_keyword(const char *word)
{
    static const char *keywords[] = {"int", "if", "else", "while", "return"};
    size_t i;

    for (i = 0; i < sizeof(keywords) / sizeof(keywords[0]); i++) {
        if (strcmp(word, keywords[i]) == 0) {
            return 1;
        }
    }
    return 0;
}

static void emit(const char *lexeme, const char *category)
{
    printf("%s,%s\n", lexeme, category);
}

static void emit_character(int ch, const char *category)
{
    char lexeme[2] = {(char)ch, '\0'};
    emit(lexeme, category);
}

static int scan(FILE *input)
{
    int ch;

    while ((ch = fgetc(input)) != EOF) {
        int next;
        char lexeme[MAX_LEXEME + 1];
        size_t length = 0;

        if (isspace((unsigned char)ch)) {
            continue;
        }

        if (isalpha((unsigned char)ch) || ch == '_') {
            lexeme[length++] = (char)ch;
            while ((next = fgetc(input)) != EOF &&
                   (isalnum((unsigned char)next) || next == '_')) {
                if (length < MAX_LEXEME) {
                    lexeme[length++] = (char)next;
                }
            }
            if (next != EOF) {
                ungetc(next, input);
            }
            lexeme[length] = '\0';
            emit(lexeme, is_keyword(lexeme) ? "KEYWORD" : "IDENTIFIER");
            continue;
        }

        if (isdigit((unsigned char)ch)) {
            lexeme[length++] = (char)ch;
            while ((next = fgetc(input)) != EOF && isdigit((unsigned char)next)) {
                if (length < MAX_LEXEME) {
                    lexeme[length++] = (char)next;
                }
            }
            if (next != EOF) {
                ungetc(next, input);
            }
            lexeme[length] = '\0';
            emit(lexeme, "INTEGER");
            continue;
        }

        if (ch == '/') {
            next = fgetc(input);
            if (next == '/') {
                while ((ch = fgetc(input)) != EOF && ch != '\n') {
                    /* Ignore characters in this line comment. */
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
                    fprintf(stderr, "Error: unclosed block comment\n");
                    return 1;
                }
                continue;
            }
            if (next != EOF) {
                ungetc(next, input);
            }
            emit_character('/', "ARITHMETIC_OPERATOR");
            continue;
        }

        if (ch == '=' || ch == '!' || ch == '<' || ch == '>' ||
            ch == '&' || ch == '|') {
            next = fgetc(input);
            if ((ch == '=' && next == '=') ||
                (ch == '!' && next == '=') ||
                (ch == '<' && next == '=') ||
                (ch == '>' && next == '=') ||
                (ch == '&' && next == '&') ||
                (ch == '|' && next == '|')) {
                lexeme[0] = (char)ch;
                lexeme[1] = (char)next;
                lexeme[2] = '\0';
                emit(lexeme, (ch == '&' || ch == '|') ?
                     "LOGICAL_OPERATOR" : "RELATIONAL_OPERATOR");
            } else {
                if (next != EOF) {
                    ungetc(next, input);
                }
                if (ch == '=') {
                    emit_character(ch, "ASSIGNMENT_OPERATOR");
                } else if (ch == '!') {
                    emit_character(ch, "LOGICAL_OPERATOR");
                } else if (ch == '<' || ch == '>') {
                    emit_character(ch, "RELATIONAL_OPERATOR");
                } else {
                    emit_character(ch, "UNKNOWN");
                }
            }
            continue;
        }

        if (ch == '+' || ch == '-' || ch == '*' || ch == '%') {
            emit_character(ch, "ARITHMETIC_OPERATOR");
        } else if (ch == '(' || ch == ')' || ch == '{' || ch == '}' ||
                   ch == '[' || ch == ']' || ch == ';' || ch == ',') {
            emit_character(ch, "DELIMITER");
        } else {
            emit_character(ch, "UNKNOWN");
        }
    }

    if (ferror(input)) {
        perror("Error reading input");
        return 1;
    }
    return 0;
}

int main(int argc, char **argv)
{
    FILE *input;
    int status;

    if (argc != 2) {
        fprintf(stderr, "Usage: %s input.c\n", argv[0]);
        return 1;
    }

    input = fopen(argv[1], "rb");
    if (input == NULL) {
        perror("Cannot open input file");
        return 1;
    }

    status = scan(input);
    if (fclose(input) != 0) {
        perror("Cannot close input file");
        return 1;
    }
    return status;
}
