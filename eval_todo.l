/* 
 *   - For single-character punctuation ( ) * | and newline:
 *       return the character itself (so the parser can compare
 *       against '(', ')', '*', '|', '\n').
 *
 *   - For a letter a..z or A..Z: return the special token LETTER
 *       and stash the actual character in the global `yylval`
 *       so the parser can read it later (needed for the AST leaf).
 *
 *   - For spaces and tabs: skip them (no return — flex will
 *       just move on to the next match).
 */

%{
#define LETTER 256        
extern char yylval;     
%}

%option noyywrap

%%

    /* ---------- TODO 1 ----------
     * Match any ONE of these characters:  ( ) * | newline and return that same character (hint: yytext[0]).
     * ---------------------------- */
[()*|\n] { return yytext[0]; }

    /* ---------- TODO 2 ----------
     * Match a single letter: a..z or A..Z and store the character in yylval, then return LETTER.
     * ---------------------------- */
[a-zA-Z] { yylval = yytext[0]; return LETTER; }

    /* ---------- TODO 3 ----------
     * Match one or more spaces/tabs and do nothing, they need to be skipped (just a `;` — skip whitespace silently).
     * ---------------------------- */
[ \t\r]+ { ; }

. { fprintf(stderr, "Lexical error: unexpected character '%c'\n", yytext[0]); exit(EXIT_FAILURE); }

%%
