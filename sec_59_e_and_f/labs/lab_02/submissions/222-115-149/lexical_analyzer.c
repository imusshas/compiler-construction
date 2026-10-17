/*
 * Lab 02 - A lexical analyzer in C
 * Compiler Construction, Section 59 E & F
 *
 * Reads a C source file (the small language subset defined in the lab) as a
 * stream of characters and prints one "lexeme,TOKEN" line for every lexeme it
 * finds, in source order. Whitespace and comments produce no tokens.
 *
 * Usage:   ./lexical_analyzer input.c
 * Success returns 0. A usage error, an unreadable file, or an unterminated
 * block comment writes a diagnostic to stderr and returns a nonzero status.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* The lab guarantees lexemes of at most 128 characters; keep headroom. */
#define MAX_LEXEME 256

/* [A-Za-z_] : a character that may begin an identifier. */
static int is_ident_start(int c)
{
    return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || c == '_';
}

/* [A-Za-z0-9_] : a character that may continue an identifier. */
static int is_ident_part(int c)
{
    return is_ident_start(c) || (c >= '0' && c <= '9');
}

/* [0-9] : a decimal digit. */
static int is_digit(int c)
{
    return c >= '0' && c <= '9';
}

/* The only five keywords in this subset. Matching is case-sensitive. */
static int is_keyword(const char *s)
{
    return strcmp(s, "int") == 0 ||
           strcmp(s, "if") == 0 ||
           strcmp(s, "else") == 0 ||
           strcmp(s, "while") == 0 ||
           strcmp(s, "return") == 0;
}

int main(int argc, char *argv[])
{
    FILE *in;
    int c;

    /* Exactly one argument: the source filename. */
    if (argc != 2) {
        fprintf(stderr, "usage: %s <source-file>\n", argv[0]);
        return 1;
    }

    in = fopen(argv[1], "r");
    if (in == NULL) {
        fprintf(stderr, "error: cannot open file '%s'\n", argv[1]);
        return 1;
    }

    /*
     * `c` always holds the next unprocessed character (or EOF). Each branch
     * leaves `c` pointing at the first character of the following token, so a
     * look-ahead character is simply kept in `c` instead of being pushed back.
     * An int (not char) is used so EOF stays distinct from any byte value.
     */
    c = fgetc(in);
    while (c != EOF) {

        /* Skip spaces, tabs, carriage returns and newlines. */
        if (c == ' ' || c == '\t' || c == '\r' || c == '\n') {
            c = fgetc(in);
            continue;
        }

        /* A slash starts a comment or is the division operator. */
        if (c == '/') {
            int next = fgetc(in);

            if (next == '/') {
                /* Line comment: discard through newline or end of file. */
                while (next != EOF && next != '\n') {
                    next = fgetc(in);
                }
                c = next;               /* either '\n' or EOF; loop re-checks */
                continue;
            }

            if (next == '*') {
                /* Block comment: discard through the closing marker. */
                int prev = 0;
                int closed = 0;
                int d = fgetc(in);
                while (d != EOF) {
                    if (prev == '*' && d == '/') {
                        closed = 1;
                        break;
                    }
                    prev = d;
                    d = fgetc(in);
                }
                if (!closed) {
                    fprintf(stderr, "error: unterminated block comment\n");
                    fclose(in);
                    return 1;
                }
                c = fgetc(in);
                continue;
            }

            /* Not a comment: a lone slash is an arithmetic operator. The
             * look-ahead character becomes the next token to classify. */
            printf("/,ARITHMETIC_OPERATOR\n");
            c = next;
            continue;
        }

        /* Identifier or keyword: [A-Za-z_][A-Za-z0-9_]* */
        if (is_ident_start(c)) {
            char buf[MAX_LEXEME + 1];
            int len = 0;
            while (c != EOF && is_ident_part(c)) {
                if (len < MAX_LEXEME) {
                    buf[len++] = (char)c;
                }
                c = fgetc(in);
            }
            buf[len] = '\0';
            printf("%s,%s\n", buf, is_keyword(buf) ? "KEYWORD" : "IDENTIFIER");
            continue;
        }

        /* Integer: [0-9]+  (original spelling preserved, leading zeros kept) */
        if (is_digit(c)) {
            char buf[MAX_LEXEME + 1];
            int len = 0;
            while (c != EOF && is_digit(c)) {
                if (len < MAX_LEXEME) {
                    buf[len++] = (char)c;
                }
                c = fgetc(in);
            }
            buf[len] = '\0';
            printf("%s,INTEGER\n", buf);
            continue;
        }

        /*
         * Operators and delimiters. The longest listed operator wins, so each
         * two-character operator is checked before falling back to one char.
         * ++, --, += and friends are not combined in this subset, so a lone
         * '+' etc. is emitted on its own; a lone '&' or '|' is UNKNOWN.
         */
        switch (c) {
        case '=': {
            int next = fgetc(in);
            if (next == '=') {
                printf("==,RELATIONAL_OPERATOR\n");
                c = fgetc(in);
            } else {
                printf("=,ASSIGNMENT_OPERATOR\n");
                c = next;
            }
            continue;
        }
        case '!': {
            int next = fgetc(in);
            if (next == '=') {
                printf("!=,RELATIONAL_OPERATOR\n");
                c = fgetc(in);
            } else {
                printf("!,LOGICAL_OPERATOR\n");
                c = next;
            }
            continue;
        }
        case '<': {
            int next = fgetc(in);
            if (next == '=') {
                printf("<=,RELATIONAL_OPERATOR\n");
                c = fgetc(in);
            } else {
                printf("<,RELATIONAL_OPERATOR\n");
                c = next;
            }
            continue;
        }
        case '>': {
            int next = fgetc(in);
            if (next == '=') {
                printf(">=,RELATIONAL_OPERATOR\n");
                c = fgetc(in);
            } else {
                printf(">,RELATIONAL_OPERATOR\n");
                c = next;
            }
            continue;
        }
        case '&': {
            int next = fgetc(in);
            if (next == '&') {
                printf("&&,LOGICAL_OPERATOR\n");
                c = fgetc(in);
            } else {
                printf("&,UNKNOWN\n");
                c = next;
            }
            continue;
        }
        case '|': {
            int next = fgetc(in);
            if (next == '|') {
                printf("||,LOGICAL_OPERATOR\n");
                c = fgetc(in);
            } else {
                printf("|,UNKNOWN\n");
                c = next;
            }
            continue;
        }
        case '+': case '-': case '*': case '%':
            printf("%c,ARITHMETIC_OPERATOR\n", c);
            c = fgetc(in);
            continue;
        case '(': case ')': case '{': case '}':
        case '[': case ']': case ';': case ',':
            printf("%c,DELIMITER\n", c);
            c = fgetc(in);
            continue;
        default:
            /* Any other single non-whitespace ASCII character. */
            printf("%c,UNKNOWN\n", c);
            c = fgetc(in);
            continue;
        }
    }

    fclose(in);
    return 0;
}
