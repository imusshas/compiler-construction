#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

// Helper to check if an identifier is actually a keyword
int is_keyword(const char *str) {
    if (strcmp(str, "int") == 0 || strcmp(str, "if") == 0 ||
        strcmp(str, "else") == 0 || strcmp(str, "while") == 0 ||
        strcmp(str, "return") == 0) {
        return 1;
    }
    return 0;
}

// Helper to check for single character delimiters
int is_delimiter(int c) {
    if (c == '(' || c == ')' || c == '{' || c == '}' || 
        c == '[' || c == ']' || c == ':' || c == ',' || c == ';') {
        return 1;
    }
    return 0;
}

int main(int argc, char *argv[]) {
    // Check for correct number of arguments
    if (argc != 2) {
        fprintf(stderr, "Error: Please provide exactly one input file.\n");
        return 1;
    }

    // Open file and handle failure
    FILE *fp = fopen(argv[1], "r");
    if (!fp) {
        fprintf(stderr, "Error: Could not open file %s\n", argv[1]);
        return 1;
    }

    int c;
    while ((c = fgetc(fp)) != EOF) {
        // Skip whitespace
        if (isspace(c)) {
            continue;
        }

        // Identifiers and Keywords
        if (isalpha(c)) {
            char buffer[256];
            int len = 0;
            buffer[len++] = c;
            
            while ((c = fgetc(fp)) != EOF && (isalnum(c) || c == '_')) {
                if (len < 255) buffer[len++] = c;
            }
            buffer[len] = '\0';
            if (c != EOF) ungetc(c, fp);

            if (is_keyword(buffer)) {
                printf("%s, KEYWORD\n", buffer);
            } else {
                printf("%s, IDENTIFIER\n", buffer);
            }
            continue;
        }

        // Integers
        if (isdigit(c)) {
            char buffer[256];
            int len = 0;
            buffer[len++] = c;
            
            while ((c = fgetc(fp)) != EOF && isdigit(c)) {
                if (len < 255) buffer[len++] = c;
            }
            buffer[len] = '\0';
            if (c != EOF) ungetc(c, fp);
            
            printf("%s, INTEGER\n", buffer);
            continue;
        }

        // Operators & Comments
        if (c == '/') {
            int next = fgetc(fp);
            if (next == '/') {
                
                while ((next = fgetc(fp)) != EOF && next != '\n');
            } else if (next == '*') {
                
                int closed = 0;
                while ((next = fgetc(fp)) != EOF) {
                    if (next == '*') {
                        int possible_slash = fgetc(fp);
                        if (possible_slash == '/') {
                            closed = 1;
                            break;
                        } else {
                            ungetc(possible_slash, fp);
                        }
                    }
                }
                // Handle unclosed block comment
                if (!closed) {
                    fprintf(stderr, "Error: Unclosed block comment.\n");
                    fclose(fp);
                    return 1; 
                }
            } else {
                if (next != EOF) ungetc(next, fp);
                printf("/, ARITHMETIC_OPERATOR\n");
            }
            continue;
        }

        // Logical AND
        if (c == '&') {
            int next = fgetc(fp);
            if (next == '&') {
                printf("&&, LOGICAL_OPERATOR\n");
            } else {
                if (next != EOF) ungetc(next, fp);
                printf("&, UNKNOWN\n"); 
            }
            continue;
        }

        // Logical OR
        if (c == '|') {
            int next = fgetc(fp);
            if (next == '|') {
                printf("||, LOGICAL_OPERATOR\n");
            } else {
                if (next != EOF) ungetc(next, fp);
                printf("|, UNKNOWN\n"); 
            }
            continue;
        }

        // Relational and Assignment operators starting with '=' or '!'
        if (c == '=' || c == '!') {
            int next = fgetc(fp);
            if (next == '=') {
                if (c == '=') printf("==, RELATIONAL_OPERATOR\n");
                else printf("!=, RELATIONAL_OPERATOR\n");
            } else {
                if (next != EOF) ungetc(next, fp);
                if (c == '=') printf("=, ASSIGNMENT_OPERATOR\n");
                else printf("!, LOGICAL_OPERATOR\n");
            }
            continue;
        }

        // Relational starting with '<' or '>'
        if (c == '<' || c == '>') {
            int next = fgetc(fp);
            if (next == '=') {
                if (c == '<') printf("<=, RELATIONAL_OPERATOR\n");
                else printf(">=, RELATIONAL_OPERATOR\n");
            } else {
                if (next != EOF) ungetc(next, fp);
                if (c == '<') printf("<, RELATIONAL_OPERATOR\n");
                else printf(">, RELATIONAL_OPERATOR\n");
            }
            continue;
        }

        
        if (c == '+' || c == '-' || c == '*' || c == '%') {
            printf("%c, ARITHMETIC_OPERATOR\n", c);
            continue;
        }

        
        if (is_delimiter(c)) {
            printf("%c, DELIMITER\n", c);
            continue;
        }

        
        printf("%c, UNKNOWN\n", c);
    }

    fclose(fp);
    return 0;
}