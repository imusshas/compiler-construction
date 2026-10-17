#include <stdio.h>
#include <ctype.h>
#include <string.h>

int isKeyword(const char *word)
{
    const char *keywords[] = {"int", "if", "else", "while", "return"};
    for (int i = 0; i < 5; i++)
    {
        if (strcmp(word, keywords[i]) == 0)
            return 1;
    }
    return 0;
}

void printToken(const char *lexeme, const char *token)
{
    printf("%s,%s\n", lexeme, token);
}

int main(int argc, char **argv)
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

    int c, next;
    char lexeme[129];
    int length;

    while ((c = fgetc(input)) != EOF)
    {
        if (isspace((unsigned char)c))
            continue;

        if (c == '/')
        {
            next = fgetc(input);

            if (next == '/')
            {
                while ((c = fgetc(input)) != EOF && c != '\n')
                {
                }
                continue;
            }

            if (next == '*')
            {
                int previous = 0;
                int closed = 0;

                while ((c = fgetc(input)) != EOF)
                {
                    if (previous == '*' && c == '/')
                    {
                        closed = 1;
                        break;
                    }
                    previous = c;
                }

                if (!closed)
                {
                    fprintf(stderr, "Unclosed block comment\n");
                    fclose(input);
                    return 1;
                }
                continue;
            }

            if (next != EOF)
                ungetc(next, input);

            printToken("/", "ARITHMETIC_OPERATOR");
            continue;
        }

        if (isalpha((unsigned char)c) || c == '_')
        {
            length = 0;
            lexeme[length++] = (char)c;

            while ((c = fgetc(input)) != EOF &&
                   (isalnum((unsigned char)c) || c == '_'))
            {
                lexeme[length++] = (char)c;
            }

            if (c != EOF)
                ungetc(c, input);

            lexeme[length] = '\0';

            if (isKeyword(lexeme))
                printToken(lexeme, "KEYWORD");
            else
                printToken(lexeme, "IDENTIFIER");

            continue;
        }

        if (isdigit((unsigned char)c))
        {
            length = 0;
            lexeme[length++] = (char)c;

            while ((c = fgetc(input)) != EOF &&
                   isdigit((unsigned char)c))
            {
                lexeme[length++] = (char)c;
            }

            if (c != EOF)
                ungetc(c, input);

            lexeme[length] = '\0';
            printToken(lexeme, "INTEGER");
            continue;
        }

        next = fgetc(input);

        if (c == '=' && next == '=')
        {
            printToken("==", "RELATIONAL_OPERATOR");
        }
        else if (c == '!' && next == '=')
        {
            printToken("!=", "RELATIONAL_OPERATOR");
        }
        else if (c == '<' && next == '=')
        {
            printToken("<=", "RELATIONAL_OPERATOR");
        }
        else if (c == '>' && next == '=')
        {
            printToken(">=", "RELATIONAL_OPERATOR");
        }
        else if (c == '&' && next == '&')
        {
            printToken("&&", "LOGICAL_OPERATOR");
        }
        else if (c == '|' && next == '|')
        {
            printToken("||", "LOGICAL_OPERATOR");
        }
        else
        {
            if (next != EOF)
                ungetc(next, input);

            char single[2] = {(char)c, '\0'};

            if (c == '=')
            {
                printToken(single, "ASSIGNMENT_OPERATOR");
            }
            else if (c == '<' || c == '>' || c == '!')
            {
                printToken(single, c == '!' ? "LOGICAL_OPERATOR" : "RELATIONAL_OPERATOR");
            }
            else if (c == '+' || c == '-' || c == '*' || c == '%')
            {
                printToken(single, "ARITHMETIC_OPERATOR");
            }
            else if (c == '(' || c == ')' || c == '{' ||
                     c == '}' || c == '[' || c == ']' ||
                     c == ';' || c == ',')
            {
                printToken(single, "DELIMITER");
            }
            else
            {
                printToken(single, "UNKNOWN");
            }
        }
    }

    fclose(input);
    return 0;
}