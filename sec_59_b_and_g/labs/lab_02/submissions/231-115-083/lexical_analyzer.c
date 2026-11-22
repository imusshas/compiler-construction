#include <stdio.h>
#include <string.h>
#include <ctype.h>

int main(int argc, char *argv[]) {
    FILE *file;
    int c, next, i, last;
    char word[129];

    if (argc != 2) {
        fprintf(stderr, "Usage: lexical_analyzer input.c\n");
        return 1;
    }

    file = fopen(argv[1], "rb");
    if (file == NULL) {
        perror(argv[1]);
        return 1;
    }

    while ((c = fgetc(file)) != EOF) {
        if (isspace(c))
            continue;

        if (isalpha(c) || c == '_') {
            i = 0;
            word[i++] = (char)c;
            while ((next = fgetc(file)) != EOF && (isalnum(next) || next == '_'))
                word[i++] = (char)next;
            word[i] = '\0';
            if (next != EOF)
                ungetc(next, file);

            if (strcmp(word, "int") == 0 || strcmp(word, "if") == 0 ||
                strcmp(word, "else") == 0 || strcmp(word, "while") == 0 ||
                strcmp(word, "return") == 0)
                printf("%s,KEYWORD\n", word);
            else
                printf("%s,IDENTIFIER\n", word);
            continue;
        }

        if (isdigit(c)) {
            i = 0;
            word[i++] = (char)c;
            while ((next = fgetc(file)) != EOF && isdigit(next))
                word[i++] = (char)next;
            word[i] = '\0';
            if (next != EOF)
                ungetc(next, file);
            printf("%s,INTEGER\n", word);
            continue;
        }

        if (c == '/') {
            next = fgetc(file);
            if (next == '/') {
                while ((c = fgetc(file)) != EOF && c != '\n')
                    ;
                continue;
            }
            if (next == '*') {
                last = 0;
                while ((c = fgetc(file)) != EOF) {
                    if (last == '*' && c == '/')
                        break;
                    last = c;
                }
                if (c == EOF) {
                    fprintf(stderr, "Unclosed block comment\n");
                    fclose(file);
                    return 1;
                }
                continue;
            }
            if (next != EOF)
                ungetc(next, file);
            printf("/,ARITHMETIC_OPERATOR\n");
            continue;
        }

        if (c == '=' || c == '!' || c == '<' || c == '>' || c == '&' || c == '|') {
            next = fgetc(file);
            if (next == '=' && (c == '=' || c == '!' || c == '<' || c == '>')) {
                printf("%c=,RELATIONAL_OPERATOR\n", c);
                continue;
            }
            if ((c == '&' && next == '&') || (c == '|' && next == '|')) {
                printf("%c%c,LOGICAL_OPERATOR\n", c, c);
                continue;
            }
            if (next != EOF)
                ungetc(next, file);

            if (c == '=')
                printf("=,ASSIGNMENT_OPERATOR\n");
            else if (c == '!')
                printf("!,LOGICAL_OPERATOR\n");
            else if (c == '<' || c == '>')
                printf("%c,RELATIONAL_OPERATOR\n", c);
            else
                printf("%c,UNKNOWN\n", c);
            continue;
        }

        if (c == '+' || c == '-' || c == '*' || c == '%')
            printf("%c,ARITHMETIC_OPERATOR\n", c);
        else if (c == '(' || c == ')' || c == '{' || c == '}' ||
                 c == '[' || c == ']' || c == ';' || c == ',')
            printf("%c,DELIMITER\n", c);
        else
            printf("%c,UNKNOWN\n", c);
    }

    fclose(file);
    return 0;
}
