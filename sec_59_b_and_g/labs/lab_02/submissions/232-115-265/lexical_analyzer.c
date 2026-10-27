/* Lab 02: lexical analyzer for the section manual's C subset.
 *
 * Usage:  ./lexical_analyzer input.c
 * Output: one line per token, in source order:  lexeme,TOKEN
 *
 * The file is read one character at a time with fgetc. Every character is
 * stored in an int so that EOF (-1) can never be mistaken for a real char.
 * When a scanner reads one character too far (look-ahead), the extra
 * character is put back with ungetc so the next token can start with it.
 */
#include <stdio.h>
#include <string.h>

#define MAX_LEXEME 256 /* tests promise lexemes of at most 128 characters */

/* ---------- character classes (ASCII only, locale independent) ---------- */

static int is_letter(int c) {
    return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || c == '_';
}

static int is_digit(int c) {
    return c >= '0' && c <= '9';
}

static int is_ident_char(int c) {
    return is_letter(c) || is_digit(c);
}

static int is_space(int c) {
    return c == ' ' || c == '\t' || c == '\r' || c == '\n' || c == '\f' || c == '\v';
}

/* ---------- small helpers ---------- */

static void emit(const char *lexeme, const char *token) {
    printf("%s,%s\n", lexeme, token);
}

static int is_keyword(const char *word) {
    static const char *keywords[] = {"int", "if", "else", "while", "return"};
    size_t i;
    for (i = 0; i < sizeof keywords / sizeof keywords[0]; i++) {
        if (strcmp(word, keywords[i]) == 0) {
            return 1;
        }
    }
    return 0;
}

/* Look at the next character. If it equals `expected`, consume it and return 1.
   Otherwise put it back (unless it is EOF) and return 0. */
static int match_next(FILE *in, int expected) {
    int next = fgetc(in);
    if (next == expected) {
        return 1;
    }
    if (next != EOF) {
        ungetc(next, in);
    }
    return 0;
}

/* Read a run of characters accepted by `accept`, starting with `first`
   (which the caller has already read). The first rejected character is
   put back. Result is stored in buf as a C string. */
static void read_run(FILE *in, int first, int (*accept)(int), char *buf) {
    size_t len = 0;
    int c = first;
    do {
        if (len < MAX_LEXEME - 1) {
            buf[len++] = (char)c;
        }
        c = fgetc(in);
    } while (c != EOF && accept(c));
    buf[len] = '\0';
    if (c != EOF) {
        ungetc(c, in);
    }
}

/* ---------- comments and the slash ---------- */

// Called after the two slashes of a line comment have been consumed:
// skip through the newline or EOF.
static void skip_line_comment(FILE *in) {
    int c;
    while ((c = fgetc(in)) != EOF && c != '\n') {
        /* discard */
    }
}

// Called after the opening slash-star has been consumed.
// Returns 1 if the closing star-slash was found, 0 if EOF came first.
// `prev` starts at 0, so the characters slash-star-slash do not form a
// complete comment (the star must come after the opening marker).
static int skip_block_comment(FILE *in) {
    int prev = 0;
    int c;
    while ((c = fgetc(in)) != EOF) {
        if (prev == '*' && c == '/') {
            return 1;
        }
        prev = c;
    }
    return 0;
}

/* Called after a '/' has been read. Decides between a line comment,
   a block comment and the division operator.
   Returns 0 normally, -1 for an unclosed block comment. */
static int scan_slash(FILE *in) {
    if (match_next(in, '/')) {
        skip_line_comment(in);
    } else if (match_next(in, '*')) {
        if (!skip_block_comment(in)) {
            fprintf(stderr, "error: unclosed block comment\n");
            return -1;
        }
    } else {
        emit("/", "ARITHMETIC_OPERATOR");
    }
    return 0;
}

/* ---------- operators, delimiters and unknown characters ---------- */

/* `c` is an already-read character that is not whitespace, a letter, a digit
   or '/'. Two-character operators are tried first (longest match). */
static void scan_symbol(FILE *in, int c) {
    char one[2];
    one[0] = (char)c;
    one[1] = '\0';

    switch (c) {
    case '=':
        if (match_next(in, '=')) emit("==", "RELATIONAL_OPERATOR");
        else emit("=", "ASSIGNMENT_OPERATOR");
        break;
    case '!':
        if (match_next(in, '=')) emit("!=", "RELATIONAL_OPERATOR");
        else emit("!", "LOGICAL_OPERATOR");
        break;
    case '<':
        if (match_next(in, '=')) emit("<=", "RELATIONAL_OPERATOR");
        else emit("<", "RELATIONAL_OPERATOR");
        break;
    case '>':
        if (match_next(in, '=')) emit(">=", "RELATIONAL_OPERATOR");
        else emit(">", "RELATIONAL_OPERATOR");
        break;
    case '&':
        if (match_next(in, '&')) emit("&&", "LOGICAL_OPERATOR");
        else emit("&", "UNKNOWN"); /* a lone & is not in this subset */
        break;
    case '|':
        if (match_next(in, '|')) emit("||", "LOGICAL_OPERATOR");
        else emit("|", "UNKNOWN"); /* a lone | is not in this subset */
        break;
    case '+':
    case '-':
    case '*':
    case '%':
        emit(one, "ARITHMETIC_OPERATOR");
        break;
    case '(':
    case ')':
    case '{':
    case '}':
    case '[':
    case ']':
    case ';':
    case ',':
        emit(one, "DELIMITER");
        break;
    default:
        emit(one, "UNKNOWN");
        break;
    }
}

/* ---------- main ---------- */

int main(int argc, char **argv) {
    FILE *input;
    char lexeme[MAX_LEXEME];
    int status = 0;
    int c;

    if (argc != 2) {
        fprintf(stderr, "usage: lexical_analyzer input.c\n");
        return 2;
    }
    input = fopen(argv[1], "rb");
    if (input == NULL) {
        perror("input");
        return 2;
    }

    while ((c = fgetc(input)) != EOF) {
        if (is_space(c)) {
            continue;
        }
        if (is_letter(c)) {
            read_run(input, c, is_ident_char, lexeme);
            emit(lexeme, is_keyword(lexeme) ? "KEYWORD" : "IDENTIFIER");
        } else if (is_digit(c)) {
            read_run(input, c, is_digit, lexeme);
            emit(lexeme, "INTEGER");
        } else if (c == '/') {
            if (scan_slash(input) != 0) {
                status = 1;
                break; /* nothing sensible can follow an unclosed comment */
            }
        } else {
            scan_symbol(input, c);
        }
    }

    fclose(input);
    return status;
}
