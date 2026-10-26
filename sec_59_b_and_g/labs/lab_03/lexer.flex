%option noyywrap yylineno


%{
#include <stdio.h>
static void emit(const char *kind);
%}


DIGIT      [0-9]
NUMBER     {DIGIT}+
ID         [a-zA-Z_][a-zA-Z0-9_]*
KEYWORD    "int"|"float"|"if"|"else"|"return"
OPERATOR   "=="|"!="|"<="|">="|[+*/=<>-]
SEPARATOR  [(){};,]
COMMENT    "//"[^\n]*
WHITESPACE [ \t\r\n]+
ERROR      .


%%

{KEYWORD}    { emit("KEYWORD"); }
{ID}         { emit("IDENTIFIER"); }
{NUMBER}     { emit("NUMBER"); }
{OPERATOR}   { emit("OPERATOR"); }
{SEPARATOR}  { emit("SEPARATOR"); }
{COMMENT}    { /* Ignore single-line comments */ }
{WHITESPACE} { /* Ignore whitespace */ }
{ERROR}      { printf("Error occurred at line %d\n", yylineno); }

%%


static void emit(const char *kind)
{
    printf("%-12s %s\n", kind, yytext);
}

int main(void)
{
    printf("%-12s %s\n", "TOKEN", "LEXEME");
    yylex();
    return 0;
}
