#include <ctype.h>
#include <stdio.h>
#include <string.h>

#define MAX_LEXEME 128

static int is_keyword(const char *s) {
    static const char *const keywords[] = {"int", "if", "else", "while", "return"};
    for (size_t i = 0; i < sizeof keywords / sizeof keywords[0]; i++) {
        if (strcmp(s, keywords[i]) == 0) {
            return 1;
        }
    }
    return 0;
}

/* Look-ahead: consume the next character only if it is the expected one.
   Otherwise put it back so it can start the next token. */
static int match(FILE *fp, int expected) {
    int c = fgetc(fp);
    if (c == expected) {
        return 1;
    }
    ungetc(c, fp); /* ungetc(EOF, fp) changes nothing, so EOF is safe here */
    return 0;
}

static int is_ident_start(int c) { return isalpha(c) || c == '_'; }
static int is_ident_part(int c) { return isalnum(c) || c == '_'; }

/* c is the identifier's first character, already read by the caller. */
static void scan_identifier(FILE *fp, int c) {
    char buf[MAX_LEXEME + 1];
    int len = 0;
    while (is_ident_part(c)) {
        if (len < MAX_LEXEME) {
            buf[len++] = (char)c;
        }
        c = fgetc(fp);
    }
    ungetc(c, fp); /* this character belongs to the next token */
    buf[len] = '\0';
    printf("%s,%s\n", buf, is_keyword(buf) ? "KEYWORD" : "IDENTIFIER");
}

/* c is the integer's first digit, already read by the caller. */
static void scan_integer(FILE *fp, int c) {
    char buf[MAX_LEXEME + 1];
    int len = 0;
    while (isdigit(c)) {
        if (len < MAX_LEXEME) {
            buf[len++] = (char)c;
        }
        c = fgetc(fp);
    }
    ungetc(c, fp);
    buf[len] = '\0';
    printf("%s,INTEGER\n", buf);
}

static void skip_line_comment(FILE *fp) {
    int c;
    while ((c = fgetc(fp)) != EOF && c != '\n') {
        /* discard the rest of the line */
    }
}

/* The opening marker has been read. Returns 1 if closed, 0 if EOF came first.
   prev starts at 0, so the opening star cannot also close the comment. */
static int skip_block_comment(FILE *fp) {
    int prev = 0;
    int c;
    while ((c = fgetc(fp)) != EOF) {
        if (prev == '*' && c == '/') {
            return 1;
        }
        prev = c;
    }
    return 0;
}

static void emit_char(int c, const char *label) {
    printf("%c,%s\n", c, label);
}

/* Returns the program's exit status. */
static int scan(FILE *fp) {
    int c; /* int, not char, so that EOF stays distinct from every character */

    while ((c = fgetc(fp)) != EOF) {
        if (isspace(c)) {
            continue;
        }
        if (is_ident_start(c)) {
            scan_identifier(fp, c);
            continue;
        }
        if (isdigit(c)) {
            scan_integer(fp, c);
            continue;
        }

        switch (c) {
        case '/':
            if (match(fp, '/')) {
                skip_line_comment(fp);
            } else if (match(fp, '*')) {
                if (!skip_block_comment(fp)) {
                    fprintf(stderr, "Error: unclosed block comment\n");
                    return 1;
                }
            } else {
                emit_char(c, "ARITHMETIC_OPERATOR");
            }
            break;

        case '=':
            printf(match(fp, '=') ? "==,RELATIONAL_OPERATOR\n" : "=,ASSIGNMENT_OPERATOR\n");
            break;
        case '!':
            printf(match(fp, '=') ? "!=,RELATIONAL_OPERATOR\n" : "!,LOGICAL_OPERATOR\n");
            break;
        case '<':
            printf(match(fp, '=') ? "<=,RELATIONAL_OPERATOR\n" : "<,RELATIONAL_OPERATOR\n");
            break;
        case '>':
            printf(match(fp, '=') ? ">=,RELATIONAL_OPERATOR\n" : ">,RELATIONAL_OPERATOR\n");
            break;
        case '&':
            printf(match(fp, '&') ? "&&,LOGICAL_OPERATOR\n" : "&,UNKNOWN\n");
            break;
        case '|':
            printf(match(fp, '|') ? "||,LOGICAL_OPERATOR\n" : "|,UNKNOWN\n");
            break;

        case '+':
        case '-':
        case '*':
        case '%':
            emit_char(c, "ARITHMETIC_OPERATOR");
            break;

        case '(':
        case ')':
        case '{':
        case '}':
        case '[':
        case ']':
        case ';':
        case ',':
            emit_char(c, "DELIMITER");
            break;

        default:
            emit_char(c, "UNKNOWN");
            break;
        }
    }

    if (ferror(fp)) {
        fprintf(stderr, "Error: failed while reading the input file\n");
        return 1;
    }
    return 0;
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

    int status = scan(input);
    fclose(input);
    return status;
}
