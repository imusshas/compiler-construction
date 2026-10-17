#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

int is_keyword(const char *s)
{
    const char *keywords[] = {
        "int", "if", "else", "while", "return"};

    for (int i = 0; i < 5; i++)
    {
        if (strcmp(s, keywords[i]) == 0)
            return 1;
    }
    return 0;
}

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        fprintf(stderr, "Usage: %s <filename>\n", argv[0]);
        return 1;
    }

    FILE *fp = fopen(argv[1], "r");
    if (fp == NULL)
    {
        perror("Error opening file");
        return 1;
    }

    int c;

    while ((c = fgetc(fp)) != EOF)
    {
        if (isspace(c))
            continue;

        if (c == '/')
        {
            int next = fgetc(fp);

            if (next == '/')
            {
                while ((c = fgetc(fp)) != EOF && c != '\n')
                    ;
                continue;
            }

            if (next == '*')
            {
                int prev = 0, closed = 0;

                while ((c = fgetc(fp)) != EOF)
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
                    fprintf(stderr, "Error: Unterminated block comment\n");
                    fclose(fp);
                    return 1;
                }
                continue;
            }

            if (next != EOF)
                ungetc(next, fp);

            printf("/,ARITHMETIC_OPERATOR\n");
            continue;
        }

        if (isalpha(c) || c == '_')
        {
            char s[256];
            int i = 0;
            s[i++] = (char)c;

            while ((c = fgetc(fp)) != EOF &&
                   (isalnum(c) || c == '_'))
            {
                if (i < 255)
                    s[i++] = (char)c;
            }

            s[i] = '\0';

            if (c != EOF)
                ungetc(c, fp);

            printf("%s,%s\n", s,
                   is_keyword(s) ? "KEYWORD" : "IDENTIFIER");
            continue;
        }

        if (isdigit(c))
        {
            char s[256];
            int i = 0;
            s[i++] = (char)c;

            while ((c = fgetc(fp)) != EOF && isdigit(c))
            {
                if (i < 255)
                    s[i++] = (char)c;
            }

            s[i] = '\0';

            if (c != EOF)
                ungetc(c, fp);

            printf("%s,INTEGER\n", s);
            continue;
        }

        if (c == '=' || c == '!' || c == '<' || c == '>')
        {
            int next = fgetc(fp);

            if (next == '=')
            {
                printf("%c=,%s\n", c,
                       c == '=' ? "RELATIONAL_OPERATOR" : "RELATIONAL_OPERATOR");
            }
            else
            {
                if (next != EOF)
                    ungetc(next, fp);

                if (c == '=')
                    printf("=,ASSIGNMENT_OPERATOR\n");
                else if (c == '!')
                    printf("!,LOGICAL_OPERATOR\n");
                else
                    printf("%c,RELATIONAL_OPERATOR\n", c);
            }
            continue;
        }

        if (c == '&' || c == '|')
        {
            int next = fgetc(fp);

            if (next == c)
            {
                printf("%c%c,LOGICAL_OPERATOR\n", c, c);
            }
            else
            {
                if (next != EOF)
                    ungetc(next, fp);
                printf("%c,UNKNOWN\n", c);
            }
            continue;
        }

        if (c == '+' || c == '-' || c == '*' || c == '%')
        {
            printf("%c,ARITHMETIC_OPERATOR\n", c);
            continue;
        }

        if (strchr("(){}[];,", c) != NULL)
        {
            printf("%c,DELIMITER\n", c);
            continue;
        }

        printf("%c,UNKNOWN\n", c);
    }

    fclose(fp);
    return 0;
}