/* 
 *
 *     Element    eBNF         Explanation
 *     Option     [X]          if (symbol in FIRST(X)) X();
 *     Closure    X+           do { X(); } while (symbol in FIRST(X));
 *                X*           while (symbol in FIRST(X)) X();
 *     List       X || D       X(); while (symbol in FIRST(D)) { D(); X(); }
 * 
 * 
 */

#include <stdio.h>
#include <stdlib.h>
#include "lexer.h"

#define INTEGER 256

//Lexer stuff
int yylval;
int symbol;

void next_symbol(void) { symbol = yylex(); }

void Z(void);
int  E(void);
int  T(void);

/*
 * Z ::= E .
 *
 * The start symbol just wraps E. Print the result here. */

void Z(void) {
    int result = E();
    if (symbol != '\n' && symbol != 0) {
        fprintf(stderr, "Eroare: se asteapta sfarsitul expresiei.\n");
        exit(EXIT_FAILURE);
    }
    printf("%d\n", result);
}

/* E ::= T ( '+' T )*               (same as E ::= T || '+')
 * 
 * List schema  X || D  with  X = T  and  D = '+':
 *     T();
 *     while (symbol in FIRST('+')) {   // i.e. symbol == '+'
 *         '+' ();                      // consume the plus
 *         T();                         // parse the next term
 *     }
 * Accumulate the sum as you go, return the result.
 */
int E(void) {
    int result = T();
    while (symbol == '+') {
        next_symbol();
        result += T();
    }
    return result;
}

/* T ::= i | '(' E ')' .
 *
 * Pure alternation. The two branches start with disjoint
 * terminals (INTEGER vs '('), so one token of lookahead picks
 * the branch unambiguously.
 */
int T(void) {
    if (symbol == INTEGER) {
        int value = yylval;
        next_symbol();
        return value;
    }
    if (symbol == '(') {
        next_symbol();
        int value = E();
        if (symbol != ')') {
            fprintf(stderr, "Eroare: lipseste ')'.\n");
            exit(EXIT_FAILURE);
        }
        next_symbol();
        return value;
    }
    // If you get here, the input is invalid. Print an error and exit.
    fprintf(stderr, "Eroare: se asteapta un numar intreg sau '('.\n");
    exit(EXIT_FAILURE);
}

int main(void) {
    next_symbol();
    while (symbol == '\n') next_symbol(); //skip blank lines
    while (symbol != 0) {
        Z(); //evaluate one line at a time
        while (symbol == '\n') next_symbol(); //skip the trailing newlines
    }
    return 0;
}
