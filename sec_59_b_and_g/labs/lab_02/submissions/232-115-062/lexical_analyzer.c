
#include <stdio.h>
#include <string.h>

#define MAX_LEXEME 129

static int is_letter_or_underscore(int c)
{
    return (c >= 'A' && c <= 'Z') ||
           (c >= 'a' && c <= 'z') || c == '_';
}

static int is_identifier_part(int c)
{
    return is_letter_or_underscore(c) ||
           (c >= '0' && c <= '9');
}

static int is_keyword(const char *s)
{
    return strcmp(s, "int") == 0 ||
           strcmp(s, "if") == 0 ||
           strcmp(s, "else") == 0 ||
           strcmp(s, "while") == 0 ||
           strcmp(s, "return") == 0;
}

static void emit(const char *lexeme, const char *type)
{
    printf("%s,%s\n", lexeme, type);
}

static int put_back(int c, FILE *fp)
{
    if (c == EOF)
        return 1;

    if (ungetc(c, fp) == EOF) {
        fprintf(stderr, "Error: failed to restore input character\n");
        return 0;
    }

    return 1;
}

int main(int argc, char **argv)
{
    if (argc != 2) {
        fprintf(stderr, "Usage: %s filename\n", argv[0]);
        return 1;
    }

    FILE *input = fopen(argv[1], "rb");
    if (input == NULL) {
        perror(argv[1]);
        return 1;
    }

    int c;

    while ((c = fgetc(input)) != EOF) {
        if (c == ' ' || c == '\t' || c == '\r' ||
            c == '\n' || c == '\v' || c == '\f') {
            continue;
        }

        /* Identifier or keyword */
        if (is_letter_or_underscore(c)) {
            char word[MAX_LEXEME];
            size_t n = 0;

            do {
                word[n++] = (char)c;
                c = fgetc(input);
            } while (c != EOF && is_identifier_part(c));

            word[n] = '\0';

            if (!put_back(c, input)) {
                fclose(input);
                return 1;
            }

            emit(word, is_keyword(word) ? "KEYWORD" : "IDENTIFIER");
            continue;
        }

        /* Integer: preserve leading zeros */
        if (c >= '0' && c <= '9') {
            char number[MAX_LEXEME];
            size_t n = 0;

            do {
                number[n++] = (char)c;
                c = fgetc(input);
            } while (c >= '0' && c <= '9');

            number[n] = '\0';

            if (!put_back(c, input)) {
                fclose(input);
                return 1;
            }

            emit(number, "INTEGER");
            continue;
        }

        /* Slash, line comment, or block comment */
        if (c == '/') {
            int next = fgetc(input);

            if (next == '/') {
                while ((c = fgetc(input)) != EOF && c != '\n') {
                    /* Skip line comment */
                }
                continue;
            }

            if (next == '*') {
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
                    fprintf(stderr, "Error: unclosed block comment\n");
                    fclose(input);
                    return 1;
                }
                continue;
            }

            if (!put_back(next, input)) {
                fclose(input);
                return 1;
            }

            emit("/", "ARITHMETIC_OPERATOR");
            continue;
        }

        /* Operators that may contain two characters */
        if (c == '=' || c == '!' || c == '<' || c == '>' ||
            c == '&' || c == '|') {
            int next = fgetc(input);
            char op[3] = { (char)c, '\0', '\0' };

            if ((c == '=' && next == '=') ||
                (c == '!' && next == '=') ||
                (c == '<' && next == '=') ||
                (c == '>' && next == '=') ||
                (c == '&' && next == '&') ||
                (c == '|' && next == '|')) {
                op[1] = (char)next;

                if (c == '&' || c == '|')
                    emit(op, "LOGICAL_OPERATOR");
                else
                    emit(op, "RELATIONAL_OPERATOR");
            } else {
                if (!put_back(next, input)) {
                    fclose(input);
                    return 1;
                }

                if (c == '=')
                    emit("=", "ASSIGNMENT_OPERATOR");
                else if (c == '!')
                    emit("!", "LOGICAL_OPERATOR");
                else if (c == '<' || c == '>')
                    emit(op, "RELATIONAL_OPERATOR");
                else
                    emit(op, "UNKNOWN");
            }
            continue;
        }

        /* Single-character arithmetic operators */
        if (c == '+' || c == '-' || c == '*' || c == '%') {
            char op[2] = { (char)c, '\0' };
            emit(op, "ARITHMETIC_OPERATOR");
            continue;
        }

        /* Delimiters */
        if (c == '(' || c == ')' || c == '{' || c == '}' ||
            c == '[' || c == ']' || c == ';' || c == ',') {
            char delimiter[2] = { (char)c, '\0' };
            emit(delimiter, "DELIMITER");
            continue;
        }

        /* Unsupported single character */
        {
            char unknown[2] = { (char)c, '\0' };
            emit(unknown, "UNKNOWN");
        }
    }

    if (ferror(input)) {
        fprintf(stderr, "Error: failed to read input file\n");
        fclose(input);
        return 1;
    }

    fclose(input);
    return 0;
}
