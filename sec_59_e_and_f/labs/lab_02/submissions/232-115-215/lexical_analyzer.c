
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_LEXEME 128
#define MAX_FILE_SIZE 65536

int is_keyword(const char *s) {
    const char *keywords[] = {
        "int", "if", "else", "while", "return"
    };

    for (int i = 0; i < 5; i++) {
        if (strcmp(s, keywords[i]) == 0)
            return 1;
    }
    return 0;
}

int main(int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr, "Usage: lexical_analyzer input.c\n");
        return 1;
    }

    FILE *input = fopen(argv[1], "rb");
    if (input == NULL) {
        perror("Cannot open input");
        return 1;
    }

    char source[MAX_FILE_SIZE + 1];
    size_t size = 0;
    int ch;

    while ((ch = fgetc(input)) != EOF) {
        if (size >= MAX_FILE_SIZE) {
            fprintf(stderr, "Error: file exceeds 64 KiB\n");
            fclose(input);
            return 1;
        }
        source[size++] = (char)ch;
    }

    if (ferror(input)) {
        fprintf(stderr, "Error: failed to read input\n");
        fclose(input);
        return 1;
    }
    fclose(input);
    source[size] = '\0';

    FILE *output = tmpfile();
    if (output == NULL) {
        fprintf(stderr, "Error: cannot create temporary output\n");
        return 1;
    }

    size_t i = 0;
    int error = 0;

    while (i < size) {
        unsigned char c = (unsigned char)source[i];

        if (isspace(c)) {
            i++;
            continue;
        }

        if (c >= 128) {
            fprintf(stderr, "Error: non-ASCII input character\n");
            error = 1;
            break;
        }

        if (c == '/' && i + 1 < size &&
            source[i + 1] == '/') {
            i += 2;
            while (i < size && source[i] != '\n')
                i++;
            continue;
        }

        if (c == '/' && i + 1 < size &&
            source[i + 1] == '*') {
            i += 2;
            int closed = 0;

            while (i + 1 < size) {
                if (source[i] == '*' && source[i + 1] == '/') {
                    i += 2;
                    closed = 1;
                    break;
                }
                i++;
            }

            if (!closed) {
                fprintf(stderr, "Error: unclosed block comment\n");
                error = 1;
                break;
            }
            continue;
        }

        char lexeme[MAX_LEXEME + 1];
        size_t length = 0;

        if (isalpha(c) || c == '_') {
            while (i < size &&
                   (isalnum((unsigned char)source[i]) ||
                    source[i] == '_')) {
                if (length >= MAX_LEXEME) {
                    fprintf(stderr, "Error: lexeme too long\n");
                    error = 1;
                    break;
                }
                lexeme[length++] = source[i++];
            }

            if (error)
                break;

            lexeme[length] = '\0';
            fprintf(output, "%s,%s\n", lexeme,
                    is_keyword(lexeme) ? "KEYWORD" : "IDENTIFIER");
            continue;
        }

        if (isdigit(c)) {
            while (i < size &&
                   isdigit((unsigned char)source[i])) {
                if (length >= MAX_LEXEME) {
                    fprintf(stderr, "Error: lexeme too long\n");
                    error = 1;
                    break;
                }
                lexeme[length++] = source[i++];
            }

            if (error)
                break;

            lexeme[length] = '\0';
            fprintf(output, "%s,INTEGER\n", lexeme);
            continue;
        }

        if (i + 1 < size) {
            char pair[3] = {source[i], source[i + 1], '\0'};

            if (strcmp(pair, "==") == 0 ||
                strcmp(pair, "!=") == 0 ||
                strcmp(pair, "<=") == 0 ||
                strcmp(pair, ">=") == 0) {
                fprintf(output, "%s,RELATIONAL\n", pair);
                i += 2;
                continue;
            }

            if (strcmp(pair, "&&") == 0 ||
                strcmp(pair, "||") == 0) {
                fprintf(output, "%s,LOGICAL\n", pair);
                i += 2;
                continue;
            }
        }

        if (strchr("=<>!", c) != NULL) {
            const char *token;

            if (c == '=')
                token = "ASSIGNMENT";
            else if (c == '!')
                token = "LOGICAL";
            else
                token = "RELATIONAL";

            fprintf(output, "%c,%s\n", c, token);
            i++;
            continue;
        }

        if (strchr("+-*/%", c) != NULL) {
            fprintf(output, "%c,ARITHMETIC\n", c);
            i++;
            continue;
        }

        if (strchr("(){}[];,", c) != NULL) {
            fprintf(output, "%c,DELIMITER\n", c);
            i++;
            continue;
        }

        fprintf(output, "%c,UNKNOWN\n", c);
        i++;
    }

    if (!error) {
        rewind(output);
        while ((ch = fgetc(output)) != EOF)
            putchar(ch);

        if (ferror(output)) {
            fprintf(stderr, "Error: failed to write output\n");
            error = 1;
        }
    }

    fclose(output);
    return error ? 1 : 0;
}
