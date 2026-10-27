
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_LEXEME 128

int is_keyword(const char *s)
{
    const char *words[] = {"int", "if", "else", "while", "return"};

    for (int i = 0; i < 5; i++)
        if (strcmp(s, words[i]) == 0)
            return 1;

    return 0;
}

void print_token(const char *s, const char *type)
{
    printf("%s,%s\n", s, type);
}

int append_char(char *s, int *n, int c)
{
    if (*n >= MAX_LEXEME)
        return 0;

    s[(*n)++] = (char)c;
    s[*n] = '\0';
    return 1;
}

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        fprintf(stderr, "usage: lexical_analyzer input.c\n");
        return 2;
    }

    FILE *input = fopen(argv[1], "rb");
    if (input == NULL)
    {
        perror("input");
        return 2;
    }

    int c;
    while ((c = fgetc(input)) != EOF)
    {
        if (isspace((unsigned char)c))
            continue;

        char s[MAX_LEXEME + 1] = {0};
        int n = 0;

        if (isalpha((unsigned char)c) || c == '_')
        {
            append_char(s, &n, c);

            while ((c = fgetc(input)) != EOF &&
                   (isalnum((unsigned char)c) || c == '_'))
            {
                if (!append_char(s, &n, c))
                {
                    fprintf(stderr, "error: lexeme exceeds 128 characters\n");
                    fclose(input);
                    return 1;
                }
            }

            if (c != EOF)
                ungetc(c, input);

            print_token(s, is_keyword(s) ? "KEYWORD" : "IDENTIFIER");
        }
        else if (isdigit((unsigned char)c))
        {
            append_char(s, &n, c);

            while ((c = fgetc(input)) != EOF &&
                   isdigit((unsigned char)c))
            {
                if (!append_char(s, &n, c))
                {
                    fprintf(stderr, "error: lexeme exceeds 128 characters\n");
                    fclose(input);
                    return 1;
                }
            }

            if (c != EOF)
                ungetc(c, input);

            print_token(s, "INTEGER");
        }
        else if (c == '/')
        {
            int d = fgetc(input);

            if (d == '/')
            {
                while ((c = fgetc(input)) != EOF && c != '\n')
                    ;
            }
            else if (d == '*')
            {
                int prev = 0;
                int closed = 0;

                while ((c = fgetc(input)) != EOF)
                {
                    if (prev == '*' && c == '/')
                    {
                        closed = 1;
                        break;
                    }
                    prev = c;
                }

                if (!closed)
                {
                    fprintf(stderr, "error: unclosed block comment\n");
                    fclose(input);
                    return 1;
                }
            }
            else
            {
                if (d != EOF)
                    ungetc(d, input);

                print_token("/", "ARITHMETIC_OPERATOR");
            }
        }
        else if (strchr("=<>!&|", c) != NULL)
        {
            int d = fgetc(input);
            int two = (d == '=' && strchr("=<>!", c) != NULL) ||
                      (d == c && (c == '&' || c == '|'));

            if (two)
            {
                s[0] = (char)c;
                s[1] = (char)d;
                s[2] = '\0';
            }
            else
            {
                if (d != EOF)
                    ungetc(d, input);

                s[0] = (char)c;
                s[1] = '\0';
            }

            if (strcmp(s, "=") == 0)
                print_token(s, "ASSIGNMENT_OPERATOR");
            else if (strcmp(s, "==") == 0 || strcmp(s, "!=") == 0 ||
                     strcmp(s, "<") == 0 || strcmp(s, "<=") == 0 ||
                     strcmp(s, ">") == 0 || strcmp(s, ">=") == 0)
                print_token(s, "RELATIONAL_OPERATOR");
            else if (strcmp(s, "!") == 0 || strcmp(s, "&&") == 0 ||
                     strcmp(s, "||") == 0)
                print_token(s, "LOGICAL_OPERATOR");
            else
                print_token(s, "UNKNOWN");
        }
        else if (strchr("+-*%", c) != NULL)
        {
            s[0] = (char)c;
            s[1] = '\0';
            print_token(s, "ARITHMETIC_OPERATOR");
        }
        else if (strchr("(){}[];,", c) != NULL)
        {
            s[0] = (char)c;
            s[1] = '\0';
            print_token(s, "DELIMITER");
        }
        else
        {
            s[0] = (char)c;
            s[1] = '\0';
            print_token(s, "UNKNOWN");
        }
    }

    fclose(input);
    return 0;
}
