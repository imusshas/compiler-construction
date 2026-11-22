%{
#include <stdio.h>
#include <stdlib.h>

int line_num = 1;

void print_tok(const char *token, const char *lexeme) {
    printf("%-12s %s\n", token, lexeme);
}
%}

DIGIT       [0-9]
LETTER      [A-Za-z_]
ID          {LETTER}({LETTER}|{DIGIT})*
NUM         {DIGIT}+

%%

[ \t\r]+            { /* ignore horizontal whitespace */ }
\n                  { line_num++; }
"//".*              { /* ignore single-line comments */ }

"int"|"float"|"if"|"else"|"return" {
    print_tok("KEYWORD", yytext);
}

{ID} {
    print_tok("IDENTIFIER", yytext);
}

{NUM} {
    print_tok("NUMBER", yytext);
}

"=="|"!="|"<="|">="|"+"|"-"|"*"|"/"|"="|"<"|">" {
    print_tok("OPERATOR", yytext);
}

"("|")"|"{"|"}"|";"|"," {
    print_tok("SEPARATOR", yytext);
}

. {
    printf("Error occurred at line %d\n", line_num);
}

%%

int yywrap(void) {
    return 1;
}

int main(int argc, char **argv) {
    if (argc > 1) {
        FILE *fp = fopen(argv[1], "r");
        if (!fp) {
            perror("File open error");
            return 1;
        }
        yyin = fp;
    }

    printf("%-12s %s\n", "TOKEN", "LEXEME");
    yylex();
    return 0;
}