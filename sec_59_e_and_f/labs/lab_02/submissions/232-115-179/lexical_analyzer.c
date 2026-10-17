#include <stdio.h>
#include <string.h>

#define MAX_LEXEME 256

static int is_ident_start(int c)
{
    return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || c == '_';
}

static int is_digit(int c)
{
    return c >= '0' && c <= '9';
}

static int is_ident_part(int c)
{
    return is_ident_start(c) || is_digit(c);
}

static int is_space(int c)
{
    return c == ' ' || c == '\t' || c == '\r' || c == '\n' || c == '\f' || c == '\v';
}

static int is_keyword(const char *word)
{
    static const char *keywords[] = {"int", "if", "else", "while", "return"};
    size_t i;
    for (i = 0; i < sizeof(keywords) / sizeof(keywords[0]); i++)
    {
        if (strcmp(word, keywords[i]) == 0)
            return 1;
    }
    return 0;
}

static void emit(const char *lexeme, const char *token)
{
    printf("%s,%s\n", lexeme, token);
}

static void read_word(FILE *file, int first, int (*belongs)(int), char *buffer)
{
    size_t length = 0;
    int c = first;
    while (c != EOF && belongs(c))
    {
        if (length < MAX_LEXEME - 1)
            buffer[length++] = (char)c;
        c = fgetc(file);
    }
    buffer[length] = '\0';
    if (c != EOF)
        ungetc(c, file);
}

static void skip_line_comment(FILE *file)
{
    int c;
    while ((c = fgetc(file)) != EOF && c != '\n')
    {
    }
}

static int skip_block_comment(FILE *file)
{
    int previous = 0;
    int c;
    while ((c = fgetc(file)) != EOF)
    {
        if (previous == '*' && c == '/')
            return 1;
        previous = c;
    }
    return 0;
}

static void scan_symbol(FILE *file, int c)
{
    int next;
    char lexeme[3] = {(char)c, '\0', '\0'};

    switch (c)
    {
    case '=':
    case '!':
    case '<':
    case '>':
        next = fgetc(file);
        if (next == '=')
        {
            lexeme[1] = '=';
            emit(lexeme, "RELATIONAL_OPERATOR");
            return;
        }
        if (next != EOF)
            ungetc(next, file);
        if (c == '=')
            emit(lexeme, "ASSIGNMENT_OPERATOR");
        else if (c == '!')
            emit(lexeme, "LOGICAL_OPERATOR");
        else
            emit(lexeme, "RELATIONAL_OPERATOR");
        return;
    case '&':
    case '|':
        next = fgetc(file);
        if (next == c)
        {
            lexeme[1] = (char)c;
            emit(lexeme, "LOGICAL_OPERATOR");
            return;
        }
        if (next != EOF)
            ungetc(next, file);
        emit(lexeme, "UNKNOWN");
        return;
    case '+':
    case '-':
    case '*':
    case '%':
        emit(lexeme, "ARITHMETIC_OPERATOR");
        return;
    case '(':
    case ')':
    case '{':
    case '}':
    case '[':
    case ']':
    case ';':
    case ',':
        emit(lexeme, "DELIMITER");
        return;
    default:
        emit(lexeme, "UNKNOWN");
        return;
    }
}

int main(int argc, char **argv)
{
    if (argc != 2)
    {
        fprintf(stderr, "Usage: %s input.c\n", argv[0]);
        return 1;
    }
    FILE *file = fopen(argv[1], "r");
    if (file == NULL)
    {
        perror("Cannot open input");
        return 1;
    }
    int c;
    int status = 0;
    char buffer[MAX_LEXEME];
    while ((c = fgetc(file)) != EOF)
    {
        if (is_space(c))
            continue;
        if (is_ident_start(c))
        {
            read_word(file, c, is_ident_part, buffer);
            emit(buffer, is_keyword(buffer) ? "KEYWORD" : "IDENTIFIER");
        }
        else if (is_digit(c))
        {
            read_word(file, c, is_digit, buffer);
            emit(buffer, "INTEGER");
        }
        else if (c == '/')
        {
            int next = fgetc(file);
            if (next == '/')
            {
                skip_line_comment(file);
            }
            else if (next == '*')
            {
                if (!skip_block_comment(file))
                {
                    fprintf(stderr, "Unclosed block comment\n");
                    status = 1;
                    break;
                }
            }
            else
            {
                if (next != EOF)
                    ungetc(next, file);
                emit("/", "ARITHMETIC_OPERATOR");
            }
        }
        else
        {
            scan_symbol(file, c);
        }
    }
    fclose(file);
    return status;
}