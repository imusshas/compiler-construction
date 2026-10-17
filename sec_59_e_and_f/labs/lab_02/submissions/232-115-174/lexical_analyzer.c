
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_FILE_SIZE 65536
#define MAX_LEXEME_SIZE 129

typedef struct {
    char lexeme[MAX_LEXEME_SIZE];
    const char *type;
} Token;

static int is_identifier_start(int c) {
    return (c >= 'A' && c <= 'Z') ||
           (c >= 'a' && c <= 'z') || c == '_';
}

static int is_identifier_char(int c) {
    return is_identifier_start(c) || (c >= '0' && c <= '9');
}

static const char *classify_word(const char *s) {
    const char *keywords[] = {"int", "if", "else", "while", "return", NULL};
    for (int i = 0; keywords[i] != NULL; i++)
        if (strcmp(s, keywords[i]) == 0)
            return "KEYWORD";
    return "IDENTIFIER";
}

static int emit(const char *s, size_t n, const char *type) {
    if (n >= MAX_LEXEME_SIZE) {
        fprintf(stderr, "Error: lexeme exceeds 128 characters\n");
        return 0;
    }
    char lexeme[MAX_LEXEME_SIZE];
    memcpy(lexeme, s, n);
    lexeme[n] = '\0';
    printf("%s,%s\n", lexeme, type);
    return 1;
}

int main(int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr, "Usage: lexical_analyzer input.c\n");
        return 2;
    }

    FILE *input = fopen(argv[1], "rb");
    if (input == NULL) {
        perror("Unable to open input file");
        return 2;
    }

    char source[MAX_FILE_SIZE + 1];
    size_t length = fread(source, 1, MAX_FILE_SIZE + 1, input);
    if (ferror(input)) {
        perror("Error reading input file");
        fclose(input);
        return 2;
    }
    fclose(input);

    if (length > MAX_FILE_SIZE) {
        fprintf(stderr, "Error: input file exceeds 64 KiB\n");
        return 2;
    }
    source[length] = '\0';

    size_t i = 0;
    while (i < length) {
        unsigned char c = (unsigned char)source[i];

        if (c == ' ' || c == '\t' || c == '\r' || c == '\n') {
            i++;
            continue;
        }

        if (c == '/' && i + 1 < length && source[i + 1] == '/') {
            i += 2;
            while (i < length && source[i] != '\n')
                i++;
            continue;
        }

        if (c == '/' && i + 1 < length && source[i + 1] == '*') {
            i += 2;
            int closed = 0;
            while (i + 1 < length) {
                if (source[i] == '*' && source[i + 1] == '/') {
                    i += 2;
                    closed = 1;
                    break;
                }
                i++;
            }
            if (!closed) {
                fprintf(stderr, "Error: unclosed block comment\n");
                return 2;
            }
            continue;
        }

        if (is_identifier_start(c)) {
            size_t start = i++;
            while (i < length &&
                   is_identifier_char((unsigned char)source[i]))
                i++;
            const char *type = classify_word(source + start);
            if (!emit(source + start, i - start, type))
                return 2;
            continue;
        }

        if (c >= '0' && c <= '9') {
            size_t start = i++;
            while (i < length && source[i] >= '0' && source[i] <= '9')
                i++;
            if (!emit(source + start, i - start, "INTEGER"))
                return 2;
            continue;
        }

        if (i + 1 < length) {
            const char *two = source + i;
            if ((two[0] == '=' && two[1] == '=') ||
                (two[0] == '!' && two[1] == '=') ||
                (two[0] == '<' && two[1] == '=') ||
                (two[0] == '>' && two[1] == '=') ||
                (two[0] == '&' && two[1] == '&') ||
                (two[0] == '|' && two[1] == '|')) {
                const char *type =
                    (two[0] == '&' || two[0] == '|')
                    ? "LOGICAL_OPERATOR" : "RELATIONAL_OPERATOR";
                if (!emit(two, 2, type))
                    return 2;
                i += 2;
                continue;
            }
        }

        const char *type = NULL;
        if (c == '=') type = "ASSIGNMENT_OPERATOR";
        else if (c == '+' || c == '-' || c == '*' ||
                 c == '/' || c == '%')
            type = "ARITHMETIC_OPERATOR";
        else if (c == '<' || c == '>' || c == '!')
            type = (c == '!') ? "LOGICAL_OPERATOR" : "RELATIONAL_OPERATOR";
        else if (c == '(' || c == ')' || c == '{' || c == '}' ||
                 c == '[' || c == ']' || c == ';' || c == ',')
            type = "DELIMITER";
        else
            type = "UNKNOWN";

        if (!emit(source + i, 1, type))
            return 2;
        i++;
    }

    return 0;
}
