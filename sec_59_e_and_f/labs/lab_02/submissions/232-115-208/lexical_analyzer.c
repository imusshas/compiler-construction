
#include <stdio.h>
#include <string.h>

int is_start(int c) {
    return (c >= 'a' && c <= 'z') ||
           (c >= 'A' && c <= 'Z') || c == '_';
}

int is_part(int c) {
    return is_start(c) || (c >= '0' && c <= '9');
}

int keyword(char *s) {
    return !strcmp(s, "int") || !strcmp(s, "if") ||
           !strcmp(s, "else") || !strcmp(s, "while") ||
           !strcmp(s, "return");
}

void emit(char *s, char *type) {
    printf("%s,%s\n", s, type);
}

int scan(FILE *f) {
    int c, d;
    char s[129];

    while ((c = fgetc(f)) != EOF) {
        int i = 0;

        if (c == ' ' || c == '\t' || c == '\r' || c == '\n')
            continue;

        if (is_start(c)) {
            s[i++] = c;
            while ((d = fgetc(f)) != EOF && is_part(d))
                s[i++] = d;
            if (d != EOF) ungetc(d, f);
            s[i] = '\0';
            emit(s, keyword(s) ? "KEYWORD" : "IDENTIFIER");
            continue;
        }

        if (c >= '0' && c <= '9') {
            s[i++] = c;
            while ((d = fgetc(f)) != EOF && d >= '0' && d <= '9')
                s[i++] = d;
            if (d != EOF) ungetc(d, f);
            s[i] = '\0';
            emit(s, "INTEGER");
            continue;
        }

        if (c == '/') {
            d = fgetc(f);

            if (d == '/') {
                while ((c = fgetc(f)) != EOF && c != '\n');
                continue;
            }

            if (d == '*') {
                int prev = 0, closed = 0;
                while ((c = fgetc(f)) != EOF) {
                    if (prev == '*' && c == '/') {
                        closed = 1;
                        break;
                    }
                    prev = c;
                }
                if (!closed) {
                    fprintf(stderr, "Error: unclosed block comment\n");
                    return 1;
                }
                continue;
            }

            if (d != EOF) ungetc(d, f);
            emit("/", "ARITHMETIC_OPERATOR");
            continue;
        }

        if (c == '=' || c == '!' || c == '<' || c == '>') {
            d = fgetc(f);
            s[0] = c;
            s[1] = '\0';

            if (d == '=') {
                s[1] = '=';
                s[2] = '\0';
                emit(s, "RELATIONAL_OPERATOR");
            } else {
                if (d != EOF) ungetc(d, f);
                if (c == '=')
                    emit(s, "ASSIGNMENT_OPERATOR");
                else if (c == '!')
                    emit(s, "LOGICAL_OPERATOR");
                else
                    emit(s, "RELATIONAL_OPERATOR");
            }
            continue;
        }

        if (c == '&' || c == '|') {
            d = fgetc(f);
            s[0] = c;
            s[1] = '\0';

            if (d == c) {
                s[1] = c;
                s[2] = '\0';
                emit(s, "LOGICAL_OPERATOR");
            } else {
                if (d != EOF) ungetc(d, f);
                emit(s, "UNKNOWN");
            }
            continue;
        }

        s[0] = c;
        s[1] = '\0';

        if (c == '+' || c == '-' || c == '*' || c == '%')
            emit(s, "ARITHMETIC_OPERATOR");
        else if (strchr("(){}[];,", c))
            emit(s, "DELIMITER");
        else
            emit(s, "UNKNOWN");
    }

    return 0;
}

int main(int argc, char *argv[]) {
    FILE *f;
    int result;

    if (argc != 2) {
        fprintf(stderr, "Usage: lexical_analyzer <filename>\n");
        return 1;
    }

    f = fopen(argv[1], "r");
    if (f == NULL) {
        fprintf(stderr, "Error: cannot open file\n");
        return 1;
    }

    result = scan(f);
    fclose(f);
    return result;
}
