#ifndef SIMPLE_H
#define SIMPLE_H

/* Shared token values for the scanner, parser and code generator. */
enum {
    INTEGER = 258, VARIABLE, WHILE, IF, PRINT,
    IFX, ELSE, GE, LE, EQ, NE, UMINUS
};

extern int yylval;
int yylex(void);

#endif
