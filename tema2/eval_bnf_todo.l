%{
#include <stdlib.h>
#include <stdio.h>
#define INTEGER 256
extern int yylval;        /* defined in main_bnf.c */
//match an integer character, transform to int, return integer, return + and parantheses, ignore spaces
%}

%option noyywrap

%%

[0-9]+       { yylval = atoi(yytext); return INTEGER; }
[+()\n]      { return yytext[0]; }
[ \t]+       ;    /* skip whitespace */
.            { fprintf(stderr, "Eroare: caracter invalid '%s'.\n", yytext); exit(EXIT_FAILURE); }

%%
