/* Lab 02: implement the section manual's lexical contract.
   Read argv[1] with fopen/fgetc. Do not hard-code public.c or its token list.
   You may add helper functions. This starter is deliberately unfinished. */
#include <stdio.h>
#include <ctype.h>
#include <string.h>

int isKeyword(const char *word) {
    return strcmp(word, "int") == 0 ||
           strcmp(word, "if") == 0 ||
           strcmp(word, "else") == 0 ||
           strcmp(word, "while") == 0 ||
           strcmp(word, "return") == 0;
}

int isIdentifierStart(int c) {
    return isalpha((unsigned char)c) || c == '_';
}

int isIdentifierPart(int c) {
    return isalnum((unsigned char)c) || c == '_';
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
    if (isspace((unsigned char)c)) {
        continue;
    }

    if (isIdentifierStart(c)) {
        char word[129];
        int i = 0;

        do {
            word[i++] = (char)c;
            c = fgetc(input);
        } while (c != EOF && isIdentifierPart(c) && i < 128);

        word[i] = '\0';

        if (c != EOF) {
            ungetc(c, input);
        }

        printf("%s,%s\n", word,
               isKeyword(word) ? "KEYWORD" : "IDENTIFIER");

        continue;
    }
    
    if (isdigit((unsigned char)c)) {
        char number[129];
        int i = 0;

        do {
            number[i++] = (char)c;
            c = fgetc(input);
        } while (c != EOF && isdigit((unsigned char)c) && i < 128);

        number[i] = '\0';

        if (c != EOF) {
            ungetc(c, input);
        }

        printf("%s,INTEGER\n", number);
        continue;
    }

    if (c == '+' || c == '-' || c == '*' || c == '%') {
        printf("%c,ARITHMETIC_OPERATOR\n", c);
        continue;
    }

if (c == '/') {
    int next = fgetc(input);

    if (next == '/') {
        while ((c = fgetc(input)) != EOF && c != '\n') {
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
            fprintf(stderr, "Error: Unclosed block comment\n");
            fclose(input);
            return 1;
        }
        continue;
    }

    if (next != EOF) {
        ungetc(next, input);
    }

    printf("/,ARITHMETIC_OPERATOR\n");
    continue;
}

if (c == '=' || c == '!' || c == '<' || c == '>') {
    int next = fgetc(input);

    if (next == '=' && (c == '=' || c == '!' ||
                        c == '<' || c == '>')) {
        printf("%c=,RELATIONAL_OPERATOR\n", c);
    } else {
        if (next != EOF) {
            ungetc(next, input);
        }

        if (c == '=') {
            printf("=,ASSIGNMENT_OPERATOR\n");
        } else if (c == '!') {
            printf("!,LOGICAL_OPERATOR\n");
        } else if (c == '<' || c == '>') {
            printf("%c,RELATIONAL_OPERATOR\n", c);
        }
    }
    continue;
}

if (c == '&' || c == '|') {
    int next = fgetc(input);

    if (next == c) {
        printf("%c%c,LOGICAL_OPERATOR\n", c, c);
    } else {
        if (next != EOF) {
            ungetc(next, input);
        }
        printf("%c,UNKNOWN\n", c);
    }

    continue;
}

if (c == '(' || c == ')' || c == '{' || c == '}' ||
    c == '[' || c == ']' || c == ';' || c == ',') {

    printf("%c,DELIMITER\n", c);
    continue;
}

printf("%c,UNKNOWN\n", c);

}

    fclose(input);
    return 0;
}
