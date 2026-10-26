#include <stdio.h>
#include <string.h>

#define MAX_LEX 256

static int is_ident_start(int c) {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_';
}

static int is_digit(int c) {
    return c >= '0' && c <= '9';
}

static int is_ident_part(int c) {
    return is_ident_start(c) || is_digit(c);
}

static int is_space(int c) {
    return c == ' ' || c == '\t' || c == '\r' || c == '\n' ||
           c == '\f' || c == '\v';
}

static int is_keyword(const char *s) {
    static const char *keywords[] = {"int", "if", "else", "while", "return"};
    size_t i;
    for (i = 0; i < sizeof(keywords) / sizeof(keywords[0]); i++) {
        if (strcmp(s, keywords[i]) == 0) return 1;
    }
    return 0;
}

static void emit(const char *lexeme, const char *token) {
    printf("%s,%s\n", lexeme, token);
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

    int c;
    while ((c = fgetc(input)) != EOF) {
        /* Skip whitespace. */
        if (is_space(c)) continue;

        /* Identifier or keyword. */
        if (is_ident_start(c)) {
            char buf[MAX_LEX];
            size_t n = 0;
            buf[n++] = (char)c;
            while ((c = fgetc(input)) != EOF && is_ident_part(c)) {
                if (n < MAX_LEX - 1) buf[n++] = (char)c;
            }
            buf[n] = '\0';
            if (c != EOF) ungetc(c, input);
            emit(buf, is_keyword(buf) ? "KEYWORD" : "IDENTIFIER");
            continue;
        }

        /* Integer. */
        if (is_digit(c)) {
            char buf[MAX_LEX];
            size_t n = 0;
            buf[n++] = (char)c;
            while ((c = fgetc(input)) != EOF && is_digit(c)) {
                if (n < MAX_LEX - 1) buf[n++] = (char)c;
            }
            buf[n] = '\0';
            if (c != EOF) ungetc(c, input);
            emit(buf, "INTEGER");
            continue;
        }

        /* Slash: line comment, block comment or division. */
        if (c == '/') {
            int next = fgetc(input);
            if (next == '/') {
                while ((c = fgetc(input)) != EOF && c != '\n') {
                }
            } else if (next == '*') {
                int prev = 0;
                int closed = 0;
                while ((c = fgetc(input)) != EOF) {
                    if (prev == '*' && c == '/') {
                        closed = 1;
                        break;
                    }
                    prev = c;
                }
                if (!closed) {
                    fprintf(stderr, "error: unclosed block comment\n");
                    fclose(input);
                    return 1;
                }
            } else {
                if (next != EOF) ungetc(next, input);
                emit("/", "ARITHMETIC_OPERATOR");
            }
            continue;
        }

        /* Operators and other characters. */
        switch (c) {
        case '=': {
            int next = fgetc(input);
            if (next == '=') emit("==", "RELATIONAL_OPERATOR");
            else {
                if (next != EOF) ungetc(next, input);
                emit("=", "ASSIGNMENT_OPERATOR");
            }
            break;
        }
        case '!': {
            int next = fgetc(input);
            if (next == '=') emit("!=", "RELATIONAL_OPERATOR");
            else {
                if (next != EOF) ungetc(next, input);
                emit("!", "LOGICAL_OPERATOR");
            }
            break;
        }
        case '<': {
            int next = fgetc(input);
            if (next == '=') emit("<=", "RELATIONAL_OPERATOR");
            else {
                if (next != EOF) ungetc(next, input);
                emit("<", "RELATIONAL_OPERATOR");
            }
            break;
        }
        case '>': {
            int next = fgetc(input);
            if (next == '=') emit(">=", "RELATIONAL_OPERATOR");
            else {
                if (next != EOF) ungetc(next, input);
                emit(">", "RELATIONAL_OPERATOR");
            }
            break;
        }
        case '&': {
            int next = fgetc(input);
            if (next == '&') emit("&&", "LOGICAL_OPERATOR");
            else {
                if (next != EOF) ungetc(next, input);
                emit("&", "UNKNOWN");
            }
            break;
        }
        case '|': {
            int next = fgetc(input);
            if (next == '|') emit("||", "LOGICAL_OPERATOR");
            else {
                if (next != EOF) ungetc(next, input);
                emit("|", "UNKNOWN");
            }
            break;
        }
        case '+': emit("+", "ARITHMETIC_OPERATOR"); break;
        case '-': emit("-", "ARITHMETIC_OPERATOR"); break;
        case '*': emit("*", "ARITHMETIC_OPERATOR"); break;
        case '%': emit("%", "ARITHMETIC_OPERATOR"); break;
        case '(': emit("(", "DELIMITER"); break;
        case ')': emit(")", "DELIMITER"); break;
        case '{': emit("{", "DELIMITER"); break;
        case '}': emit("}", "DELIMITER"); break;
        case '[': emit("[", "DELIMITER"); break;
        case ']': emit("]", "DELIMITER"); break;
        case ';': emit(";", "DELIMITER"); break;
        case ',': emit(",", "DELIMITER"); break;
        default: {
            char one[2];
            one[0] = (char)c;
            one[1] = '\0';
            emit(one, "UNKNOWN");
            break;
        }
        }
    }

    fclose(input);
    return 0;
}
