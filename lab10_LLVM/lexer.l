%option noyywrap
%{
#include <cerrno>
#include <cstdlib>
#include <limits>

#include "tokens.h"

int yylval;
%}

%%

"if"           return IF;
"else"         return ELSE;
[0-9]+         {
                   errno = 0;
                   long value = std::strtol(yytext, nullptr, 10);
                   if (errno == ERANGE || value > std::numeric_limits<int>::max())
                       return INVALID;
                   yylval = static_cast<int>(value);
                   return NUMBER;
               }
[-+*/%(){}]    return *yytext;
[[:space:]]+   ;
.              return INVALID;

%%
