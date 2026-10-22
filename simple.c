#include <stdio.h>
#include <stdlib.h>
#include "types.h"
#include "simple.h"

int symbol;
int yylval;

/* Gramatica (η este tokenul INTEGER):
 * E -> while(E) E | TC
 * C -> epsilon | +TC
 * T -> η | (E)
 */
nodeType *E(void);
nodeType *T(void);
nodeType *C(nodeType *acc);
int ex(nodeType *p);

void error(void)
{
    printf("Eroare de sintaxa\n");
    exit(1);
}

void next_symbol(void)
{
    symbol = yylex();
}

nodeType *con(int value)
{
    nodeType *p = malloc(sizeof(nodeType));
    p->type = typeCon;
    p->con.value = value;
    return p;
}

nodeType *opr(int oper, int nops, nodeType *left, nodeType *right)
{
    nodeType *p = malloc(sizeof(nodeType) + sizeof(nodeType *));
    p->type = typeOpr;
    p->opr.oper = oper;
    p->opr.nops = nops;
    p->opr.op[0] = left;
    p->opr.op[1] = right;
    return p;
}

nodeType *T(void)
{
    nodeType *t;

    if (symbol == INTEGER) {
        t = con(yylval);
        next_symbol();
        return t;
    } else if (symbol == '(') {
        next_symbol();
        t = E();
        if (symbol == ')')
            next_symbol();
        else
            error();
        return t;
    } else {
        error();
        return NULL;
    }
}


nodeType *E(void)
{
    if (symbol == WHILE) {
        nodeType *condition;
        nodeType *body;

        next_symbol();
        if (symbol != '(')
            error();
        next_symbol();
        condition = E();
        if (symbol != ')')
            error();
        next_symbol();
        body = E();
        return opr(WHILE, 2, condition, body);
    } else if (symbol == INTEGER || symbol == '(') {
        nodeType *t = T();
        return C(t);
    } else {
        error();
        return NULL;
    }
}

nodeType *C(nodeType *acc)
{
    if (symbol == ')' || symbol == '\n' || symbol == 0) {
        return acc;
    } else if (symbol == '+') {
        nodeType *t;
        next_symbol();
        t = T();
        return C(opr('+', 2, acc, t));
    } else {
        error();
        return NULL;
    }
}

void parser(void)
{
    nodeType *t;
    t = E();
    if (symbol != '\n' && symbol != 0)
        error();
    ex(t);
}

int main(void)
{
    next_symbol();
    while (symbol != 0) {
        parser();
        next_symbol();
    }
    return 0;
}
