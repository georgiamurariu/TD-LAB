%option noyywrap nounput noinput
%{
#include <cerrno>
#include <climits>
#include <cstdlib>
#define NUMBER 256
int yylval;
%}

%%

[0-9]+ {
    errno = 0;
    long value = strtol(yytext, nullptr, 10);
    if (errno == ERANGE || value > INT_MAX) {
        fprintf(stderr, "Eroare: numarul depaseste limita 2147483647.\n");
        return 257;
    }
    yylval = static_cast<int>(value);
    return NUMBER;
}
[ \t\r]+ ;
[+*/%()=\n-] return *yytext;
. return *yytext;

%%
