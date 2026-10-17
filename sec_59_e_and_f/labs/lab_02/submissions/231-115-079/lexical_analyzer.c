/*
 * Lab 02: Lexical Analyzer
 * Student ID: 231-115-079
 * Section: sec_59_e_and_f
 *
 * Implements the lexical contract for the C-like subset language.
 * Reads source code character-by-character from argv[1] and outputs
 * one line per token in the format: lexeme,TOKEN
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_LEXEME_LEN 256

/*
 * Helper to emit a token line to stdout in exact "lexeme,TOKEN" format.
 */
static void print_token(const char *lexeme, const char *token_type) {
    printf("%s,%s\n", lexeme, token_type);
}

/*
 * Checks if a given identifier matches one of the 5 allowed keywords:
 * int, if, else, while, return (case-sensitive).
 */
static int is_keyword(const char *str) {
    if (strcmp(str, "int") == 0 ||
        strcmp(str, "if") == 0 ||
        strcmp(str, "else") == 0 ||
        strcmp(str, "while") == 0 ||
        strcmp(str, "return") == 0) {
        return 1;
    }
    return 0;
}

/*
 * Scans an identifier or keyword starting with [A-Za-z_].
 * Subsequent characters may be [A-Za-z0-9_].
 * Look-ahead character that stops the scan is returned to the stream using ungetc.
 */
static void scan_identifier(FILE *input, int first_char) {
    char buf[MAX_LEXEME_LEN];
    size_t len = 0;
    buf[len++] = (char)first_char;

    int ch;
    /*
     * We store fgetc result in an int because EOF is a negative integer (typically -1),
     * which cannot be reliably distinguished if stored in a char.
     */
    while ((ch = fgetc(input)) != EOF) {
        if (isalnum((unsigned char)ch) || ch == '_') {
            if (len < sizeof(buf) - 1) {
                buf[len++] = (char)ch;
            }
        } else {
            /* The character doesn't belong to the identifier; put it back for the next token */
            ungetc(ch, input);
            break;
        }
    }
    buf[len] = '\0';

    if (is_keyword(buf)) {
        print_token(buf, "KEYWORD");
    } else {
        print_token(buf, "IDENTIFIER");
    }
}

/*
 * Scans an integer literal consisting of [0-9]+.
 * Preserves original spelling including leading zeros.
 * Look-ahead character that stops the scan is returned to the stream using ungetc.
 */
static void scan_integer(FILE *input, int first_char) {
    char buf[MAX_LEXEME_LEN];
    size_t len = 0;
    buf[len++] = (char)first_char;

    int ch;
    while ((ch = fgetc(input)) != EOF) {
        if (isdigit((unsigned char)ch)) {
            if (len < sizeof(buf) - 1) {
                buf[len++] = (char)ch;
            }
        } else {
            /* Character is not a digit; put it back for subsequent token analysis */
            ungetc(ch, input);
            break;
        }
    }
    buf[len] = '\0';
    print_token(buf, "INTEGER");
}

/*
 * Skips single-line comments initiated by "//" up to the newline or EOF.
 */
static void skip_line_comment(FILE *input) {
    int ch;
    while ((ch = fgetc(input)) != EOF) {
        if (ch == '\n') {
            break;
        }
    }
}

/*
 * Skips block comments initiated by forward-slash star up to closing star forward-slash.
 * Implements a state machine to correctly handle consecutive asterisks
 * (for example in empty comments or comments with multiple asterisks before closing slash).
 * The sequence slash-star-slash does not close because the closing slash immediately
 * follows the opening slash-star without a separate asterisk.
 * Returns 0 on successful closure, or -1 if EOF is hit before closing.
 */
static int skip_block_comment(FILE *input) {
    int state = 0; /* 0: inside comment body, 1: '*' encountered */
    int ch;

    while ((ch = fgetc(input)) != EOF) {
        if (state == 0) {
            if (ch == '*') {
                state = 1;
            }
        } else if (state == 1) {
            if (ch == '/') {
                return 0; /* comment successfully closed */
            } else if (ch != '*') {
                state = 0; /* reset state if followed by non-star character */
            }
            /* if ch == '*', state remains 1 */
        }
    }
    return -1; /* reached EOF without finding closing delimiter */
}

/*
 * Scans the entire file character-by-character according to the token contract.
 * Returns 0 on success, or non-zero on error (such as unclosed block comment).
 */
static int scan_file(FILE *input) {
    /*
     * Store fgetc result in an int, not a char.
     * fgetc() returns either EOF (-1) or an unsigned char cast to int (0..255).
     * Using char can lead to incorrect EOF comparison or premature termination.
     */
    int c;

    while ((c = fgetc(input)) != EOF) {
        /* 1. Skip whitespace */
        if (isspace((unsigned char)c)) {
            continue;
        }

        /* 2. Identifiers and Keywords */
        if (isalpha((unsigned char)c) || c == '_') {
            scan_identifier(input, c);
            continue;
        }

        /* 3. Integer literals */
        if (isdigit((unsigned char)c)) {
            scan_integer(input, c);
            continue;
        }

        /* 4. Division vs Comments ('/', '//', block comments) */
        if (c == '/') {
            int next = fgetc(input);
            if (next == '/') {
                skip_line_comment(input);
            } else if (next == '*') {
                if (skip_block_comment(input) != 0) {
                    fprintf(stderr, "error: unclosed block comment\n");
                    return 1;
                }
            } else {
                /* Not a comment: look-ahead character belongs to the next token */
                if (next != EOF) {
                    ungetc(next, input);
                }
                print_token("/", "ARITHMETIC_OPERATOR");
            }
            continue;
        }

        /* 5. Assignment ('=') vs Relational Equality ('==') */
        if (c == '=') {
            int next = fgetc(input);
            if (next == '=') {
                print_token("==", "RELATIONAL_OPERATOR");
            } else {
                if (next != EOF) {
                    ungetc(next, input);
                }
                print_token("=", "ASSIGNMENT_OPERATOR");
            }
            continue;
        }

        /* 6. Logical NOT ('!') vs Relational Inequality ('!=') */
        if (c == '!') {
            int next = fgetc(input);
            if (next == '=') {
                print_token("!=", "RELATIONAL_OPERATOR");
            } else {
                if (next != EOF) {
                    ungetc(next, input);
                }
                print_token("!", "LOGICAL_OPERATOR");
            }
            continue;
        }

        /* 7. Relational Less-than ('<') and Less-than-or-equal ('<=') */
        if (c == '<') {
            int next = fgetc(input);
            if (next == '=') {
                print_token("<=", "RELATIONAL_OPERATOR");
            } else {
                if (next != EOF) {
                    ungetc(next, input);
                }
                print_token("<", "RELATIONAL_OPERATOR");
            }
            continue;
        }

        /* 8. Relational Greater-than ('>') and Greater-than-or-equal ('>=') */
        if (c == '>') {
            int next = fgetc(input);
            if (next == '=') {
                print_token(">=", "RELATIONAL_OPERATOR");
            } else {
                if (next != EOF) {
                    ungetc(next, input);
                }
                print_token(">", "RELATIONAL_OPERATOR");
            }
            continue;
        }

        /* 9. Logical AND ('&&') vs Lone Ampersand ('&' is UNKNOWN) */
        if (c == '&') {
            int next = fgetc(input);
            if (next == '&') {
                print_token("&&", "LOGICAL_OPERATOR");
            } else {
                if (next != EOF) {
                    ungetc(next, input);
                }
                print_token("&", "UNKNOWN");
            }
            continue;
        }

        /* 10. Logical OR ('||') vs Lone Pipe ('|' is UNKNOWN) */
        if (c == '|') {
            int next = fgetc(input);
            if (next == '|') {
                print_token("||", "LOGICAL_OPERATOR");
            } else {
                if (next != EOF) {
                    ungetc(next, input);
                }
                print_token("|", "UNKNOWN");
            }
            continue;
        }

        /* 11. Single-character arithmetic operators (+, -, *, %) */
        if (c == '+') {
            print_token("+", "ARITHMETIC_OPERATOR");
            continue;
        }
        if (c == '-') {
            print_token("-", "ARITHMETIC_OPERATOR");
            continue;
        }
        if (c == '*') {
            print_token("*", "ARITHMETIC_OPERATOR");
            continue;
        }
        if (c == '%') {
            print_token("%", "ARITHMETIC_OPERATOR");
            continue;
        }

        /* 12. Delimiters: ( ) { } [ ] ; , */
        if (c == '(' || c == ')' || c == '{' || c == '}' ||
            c == '[' || c == ']' || c == ';' || c == ',') {
            char lexeme[2];
            lexeme[0] = (char)c;
            lexeme[1] = '\0';
            print_token(lexeme, "DELIMITER");
            continue;
        }

        /* 13. Any other non-whitespace character is UNKNOWN */
        {
            char lexeme[2];
            lexeme[0] = (char)c;
            lexeme[1] = '\0';
            print_token(lexeme, "UNKNOWN");
        }
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

    int status = scan_file(input);

    fclose(input);
    return status;
}
