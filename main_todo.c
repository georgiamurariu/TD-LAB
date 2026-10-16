/* 
 * Recursive-descent parser for the AST problem.
 *
 * The grammar:
 *
 *     E -> T '|' E | T          (alternation, right-recursive)
 *     T -> F T   | F            (concatenation, no operator)
 *     F -> P*    | P            
 *     P -> (E)   | letter
 *
 * You produce an AST where each operator becomes a node:
 *
 *     '|'  (alternation)
 *     '&'  (concatenation -- we invent this symbol since
 *                   the regex has no visible operator for it)
 *     '*'  (star, uses left child only)
 *     a..z leaves  (op holds the letter, children are NULL)
 *
 * The invariant that makes the whole thing work:
 *     Every parsing function returns with `symbol` pointing at
 *     the FIRST TOKEN AFTER whatever it consumed.
 */

#include <stdio.h>
#include <stdlib.h>
#include "lexer.h"

#define LETTER 256

char yylval;              
int  symbol;                               

void next_symbol(void) { symbol = yylex(); }

typedef struct Node {
    char op;
    struct Node *left;
    struct Node *right;
} Node;

Node* makeNode(char op, Node* left, Node* right) {
    Node* n = malloc(sizeof(Node));
    if (!n) { fprintf(stderr, "out of memory\n"); exit(1); }
    n->op = op;
    n->left = left;
    n->right = right;
    return n;
}

Node* E(void);
Node* T(void);
Node* F(void);
Node* P(void);

/* 
 * TODO 1 -- P -> (E) | letter
 * 
 * - If symbol is '(':
 *       consume it, call E() to parse the inner expression,
 *       check that the next symbol is ')' and consume it,
 *       return the inner node (parens leave NO node in the AST).
 * - If symbol is LETTER:
 *       make a leaf node whose op is the current `yylval`,
 *       consume the letter, return the leaf.
 * - Otherwise: print an error and exit.
 */
Node* P(void) {
    if (symbol == '(') {
        next_symbol();
        Node* inner = E();
        if (symbol != ')') {
            fprintf(stderr, "Syntax error: expected ')'\n");
            exit(EXIT_FAILURE);
        }
        next_symbol();
        return inner;
    }
    if (symbol == LETTER) {
        Node* leaf = makeNode(yylval, NULL, NULL);
        next_symbol();
        return leaf;
    }
    fprintf(stderr, "Syntax error: expected a letter or '('\n");
    exit(EXIT_FAILURE);
}

/* 
 * TODO 2 -- F -> P* | P
 * 
 * - Parse a P.
 * - If the next symbol is '*':
 *      consume it and wrap the P in a '*' node, 
 *      star node will be the root, left = the P, right = NULL.
 * - Return the (possibly wrapped) node.
 */
Node* F(void) {
    Node* node = P();
    if (symbol == '*') {
        next_symbol();
        node = makeNode('*', node, NULL);
    }
    return node;
}

/* 
 * TODO 3 -- T -> F T | F     (concatenation, no visible operator)
 * 
 * Trick: there is no concatenation symbol. You detect "another F
 * is coming" by LOOKAHEAD -- only '(' and LETTER can start an F.
 *
 * - If symbol is '(' or LETTER:
 *     return a node with op='&', left=F(), right=T()  (right-recursive!)
 *   Otherwise:
 *       return left.
 *
 * Right-recursion matters: for "abc" the tree must be
 *     & (a, & (b, c))     NOT     & (& (a, b), c).
 */
Node* T(void) {
    Node* left = F();
    if (symbol == '(' || symbol == LETTER) {
        Node* right = T();
        return makeNode('&', left, right);
    }
    return left;
}

/* 
 * TODO 4 -- E -> T '|' E | T
 * 
 * - Parse a T into `left`.
 * - If symbol is '|':
 *       consume it, build a '|' node with left=T and right=E().
 *   Otherwise:
 *       return left.
 */
Node* E(void) {
    Node* left = T();
    if (symbol == '|') {
        next_symbol();
        Node* right = E();
        return makeNode('|', left, right);
    }
    return left;
}


int treeWidth(Node* node) {
    if (!node) return 0;
    if (!node->left && !node->right) return 1;
    if (!node->right) return treeWidth(node->left);
    return treeWidth(node->left) + 4 + treeWidth(node->right);
}

int treeHeight(Node* node) {
    if (!node) return 0;
    int left = treeHeight(node->left);
    int right = treeHeight(node->right);
    return 1 + (left > right ? left : right);
}

int drawTree(Node* node, char* canvas, int width, int row, int column) {
    int root = column;
    if (node->left && node->right) {
        int left = drawTree(node->left, canvas, width, row + 2, column);
        int right = drawTree(node->right, canvas, width, row + 2,
                             column + treeWidth(node->left) + 4);
        root = (left + right) / 2;
        canvas[(row + 1) * width + (left + root) / 2] = '/';
        canvas[(row + 1) * width + (root + right + 1) / 2] = '\\';
    } else if (node->left) {
        root = drawTree(node->left, canvas, width, row + 2, column);
        canvas[(row + 1) * width + root] = '|';
    }
    canvas[row * width + root] = node->op;
    return root;
}

void printTree(Node* node) {
    if (!node) return;
    int width = treeWidth(node);
    int rows = 2 * treeHeight(node) - 1;
    char* canvas = malloc((size_t)width * rows);
    if (!canvas) { fprintf(stderr, "out of memory\n"); exit(EXIT_FAILURE); }
    for (size_t i = 0; i < (size_t)width * rows; i++) canvas[i] = ' ';
    drawTree(node, canvas, width, 0, 0);
    for (int row = 0; row < rows; row++) {
        int end = width;
        while (end > 0 && canvas[row * width + end - 1] == ' ') end--;
        fwrite(canvas + row * width, 1, (size_t)end, stdout);
        putchar('\n');
    }
    free(canvas);
}

void freeTree(Node* node) {
    if (!node) return;
    freeTree(node->left);
    freeTree(node->right);
    free(node);
}


int main(void) {
    next_symbol();
    while (symbol == '\n') next_symbol(); 
    while (symbol != 0) {
        Node* root = E();
        if (symbol != '\n' && symbol != 0) {
            fprintf(stderr, "Syntax error: unexpected '%c' after expression\n", symbol);
            freeTree(root);
            return EXIT_FAILURE;
        }
        printf("Parse tree:\n");
        printTree(root);
        freeTree(root);
        while (symbol == '\n') next_symbol(); 
    }
    return 0;
}
