#include <stdio.h>
#include <ctype.h>
#include <string.h>

#define MAXLEN 256

static int is_keyword(const char *s) {
    return strcmp(s, "int") == 0 || strcmp(s, "if") == 0 ||
           strcmp(s, "else") == 0 || strcmp(s, "while") == 0 ||
           strcmp(s, "return") == 0;
}

static int is_id_start(int c) { return isalpha(c) || c == '_'; }
static int is_id_part(int c)  { return isalnum(c) || c == '_'; }

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
    int status = 0;
    char buf[MAXLEN];

    while ((c = fgetc(input)) != EOF) {
        
        if (c == ' ' || c == '\t' || c == '\r' || c == '\n')
            continue;

        
        if (is_id_start(c)) {
            int n = 0;
            while (c != EOF && is_id_part(c)) {
                if (n < MAXLEN - 1) buf[n++] = (char)c;
                c = fgetc(input);
            }
            buf[n] = '\0';
            if (c != EOF) ungetc(c, input);   
            emit(buf, is_keyword(buf) ? "KEYWORD" : "IDENTIFIER");
            continue;
        }

        
        if (isdigit(c)) {
            int n = 0;
            while (c != EOF && isdigit(c)) {
                if (n < MAXLEN - 1) buf[n++] = (char)c;
                c = fgetc(input);
            }
            buf[n] = '\0';
            if (c != EOF) ungetc(c, input);
            emit(buf, "INTEGER");
            continue;
        }

        
        if (c == '/') {
            int next = fgetc(input);
            if (next == '/') {                 
                while ((c = fgetc(input)) != EOF && c != '\n')
                    ;
            } else if (next == '*') {          
                int prev = 0, closed = 0;
                while ((c = fgetc(input)) != EOF) {
                    if (prev == '*' && c == '/') { closed = 1; break; }
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

        
        if (c == '=' || c == '!' || c == '<' || c == '>' ||
            c == '&' || c == '|') {
            int next = fgetc(input);
            char two[3] = { (char)c, (char)next, '\0' };
            if (next != EOF &&
                (strcmp(two, "==") == 0 || strcmp(two, "!=") == 0 ||
                 strcmp(two, "<=") == 0 || strcmp(two, ">=") == 0)) {
                emit(two, "RELATIONAL_OPERATOR");
                continue;
            }
            if (next != EOF && (strcmp(two, "&&") == 0 || strcmp(two, "||") == 0)) {
                emit(two, "LOGICAL_OPERATOR");
                continue;
            }
            if (next != EOF) ungetc(next, input);   
            char one[2] = { (char)c, '\0' };
            if (c == '=')                    emit(one, "ASSIGNMENT_OPERATOR");
            else if (c == '<' || c == '>')   emit(one, "RELATIONAL_OPERATOR");
            else if (c == '!')               emit(one, "LOGICAL_OPERATOR");
            else                             emit(one, "UNKNOWN");  
            continue;
        }

        
        {
            char one[2] = { (char)c, '\0' };
            if (strchr("+-*%", c))            emit(one, "ARITHMETIC_OPERATOR");
            else if (strchr("(){}[];,", c))   emit(one, "DELIMITER");
            else                              emit(one, "UNKNOWN");
        }
    }

    fclose(input);
    return status;
}