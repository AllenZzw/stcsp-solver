D       [0-9]
L       [a-zA-Z]

%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "util.h"
#include "node.h"
#include "y.tab.h"

extern int line_num;
#ifdef __cplusplus
extern "C"{
#endif
%}

%%

"//"[^\n]*\n    { }
"/*"[^"*/"]*"*/"  { }

"var"           { return VAR; }
"obj"           { return OBJ; }
"arr"			{ return ARR; }

"<"             { return '<'; }
">"             { return '>'; }
"<="            { return LE_CON; }
">="            { return GE_CON; }
"=="            { return EQ_CON; }
"!="            { return NE_CON; }
"->"			{ return IMPLY_CON; }
"until"			{ return UNTIL_CON; }

"lt"            { return LT_OP; }
"gt"            { return GT_OP; }
"le"            { return LE_OP; }
"ge"            { return GE_OP; }
"eq"            { return EQ_OP; }
"ne"            { return NE_OP; }

"and"           { return AND_OP; }
"or"            { return OR_OP; }

"+"             { return '+'; }
"-"             { return '-'; }
"*"             { return '*'; }
"/"             { return '/'; }
"%"             { return '%'; }

"@" 			{ return AT; }

"abs"           { return ABS; }

"first"         { return FIRST; }
"next"          { return NEXT; }
"fby"           { return FBY; }
"if"            { return IF; }
"then"          { return THEN; }
"else"          { return ELSE; }

"("             { return '('; }
")"             { return ')'; }
"["             { return '['; }
"]"             { return ']'; }
"{"             { return '{'; }
"}"             { return '}'; }


","             { return ','; }
";"             { return ';'; }
":"             { return ':'; }

{L}({L}|{D})*   { yylval.str = strdup(yytext); return IDENTIFIER; }
[-]?{D}+            { yylval.num = atoi(yytext); return CONSTANT; }

"'"[^\n]*       { }
[ \t\v\f]       { }
\n              { line_num++; }
.               { return yytext[0]; }

%%

int yywrap() {
    return 1;
}

#ifdef __cplusplus
}
#endif
