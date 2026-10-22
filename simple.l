%{
#include <stdlib.h>
#include "simple.h"
%}

%%
while     return WHILE;
[0-9]+    { yylval = atoi(yytext); return INTEGER; }
[ \t\r]+  ;
\n        return '\n';
.         return yytext[0];
%%
