%token STATEMENT RANGE LIST
%token VAR OBJ ARR
%token LE_CON GE_CON EQ_CON NE_CON IMPLY_CON UNTIL_CON
%token LT_OP GT_OP LE_OP GE_OP EQ_OP NE_OP
%token AND_OP OR_OP NOT_OP
%token AT FIRST NEXT FBY IF THEN ELSE
%token ABS

%{
#include <cstdio>
#include <cstdlib>
#include "node.h"
#include <sys/resource.h>

#define basicNodeNew(token, left, right) nodeNew((token), NULL, 0, 0, (left), (right))

void solve(Node *node);

extern "C"
{
    int yyparse(void);
    int yylex(void);
    int yywrap();
}

void yyerror(const char *msg);

int line_num = 1;
int my_argc = 0;
char **my_argv = NULL;
%}

%union {
    char *str;
    int num;
    Node *node;
}

%token <str> IDENTIFIER
%token <str> ARR_IDENTIFIER
%token <num> CONSTANT

%type <node> statement_list statement declaration_statement objective_statement constraint_statement array_content
%type <num> constraint_operator
%type <node> expression logical_or_expression logical_and_expression logical_not_expression
%type <node> equality_expression relational_expression
%type <num> relational_operator
%type <node> additive_expression multiplicative_expression at_expression
%type <node> fby_expression
%type <node> unary_expression primary_expression

%start solve_problem

%%

solve_problem
    : statement_list { solve($1); }
    ;

statement_list
    : { $$ = NULL; }
    | statement statement_list { $$ = basicNodeNew(STATEMENT, $1, $2); }
    ;

statement
    : declaration_statement
    | objective_statement
    | constraint_statement
    ;

array_content
    : CONSTANT { $$ = nodeNew(LIST, NULL, $1, 0, NULL, NULL); }
    | array_content ',' CONSTANT { $$=nodeNew(LIST, NULL, $3, 0, $1, NULL); }
    ;

declaration_statement
    : VAR IDENTIFIER ':' '[' CONSTANT ',' CONSTANT ']' ';' { $$ = nodeNew(VAR, $2, 0, 0, NULL, nodeNew(RANGE, NULL, $5, $7, NULL, NULL)); }
    | ARR IDENTIFIER ':' '{' array_content '}' ';' { $$=nodeNew(ARR, $2, 0, 0, NULL, $5); }
    ;


objective_statement
    : OBJ IDENTIFIER ';' { $$ = nodeNew(OBJ, $2, 0, 0, NULL, NULL); }
    ;

constraint_statement
    : expression constraint_operator expression ';' { $$ = basicNodeNew($2, $1, $3); }
    ;

constraint_operator
    : '<' { $$ = (int)'<'; }
    | '>' { $$ = (int)'>'; }
    | LE_CON { $$ = LE_CON; }
    | GE_CON { $$ = GE_CON; }
    | EQ_CON { $$ = EQ_CON; }
    | NE_CON { $$ = NE_CON; }
    | UNTIL_CON { $$ = UNTIL_CON; }
    | IMPLY_CON { $$ = IMPLY_CON; }
    ;

expression
    : logical_not_expression { $$ = $1; }
    ;

logical_not_expression 
    : logical_or_expression { $$ = $1; }
    | NOT_OP logical_not_expression { $$ = basicNodeNew(NOT_OP, NULL, $2); }
    ;

logical_or_expression
    : logical_and_expression { $$ = $1; }
    | logical_or_expression OR_OP logical_and_expression { $$ = basicNodeNew(OR_OP, $1, $3); }
    ;

logical_and_expression
    : equality_expression { $$ = $1; }
    | logical_and_expression AND_OP equality_expression { $$ = basicNodeNew(AND_OP, $1, $3); }
    ;

equality_expression
    : relational_expression { $$ = $1; }
    | equality_expression EQ_OP relational_expression { $$ = basicNodeNew(EQ_OP, $1, $3); }
    | equality_expression NE_OP relational_expression { $$ = basicNodeNew(NE_OP, $1, $3); }
    ;

relational_expression
    : additive_expression { $$ = $1; }
    | relational_expression relational_operator additive_expression { $$ = basicNodeNew($2, $1, $3); }
    ;

relational_operator
    : LT_OP { $$ = LT_OP; }
    | GT_OP { $$ = GT_OP; }
    | LE_OP { $$ = LE_OP; }
    | GE_OP { $$ = GE_OP; }
    ;

additive_expression
    : multiplicative_expression { $$ = $1; }
    | additive_expression '+' multiplicative_expression { $$ = basicNodeNew('+', $1, $3); }
    | additive_expression '-' multiplicative_expression { $$ = basicNodeNew('-', $1, $3); }
    ;

multiplicative_expression
    : at_expression { $$ = $1; }
    | multiplicative_expression '*' at_expression { $$ = basicNodeNew('*', $1, $3); }
    | multiplicative_expression '/' at_expression { $$ = basicNodeNew('/', $1, $3); }
    | multiplicative_expression '%' at_expression { $$ = basicNodeNew('%', $1, $3); }
    ;

at_expression
    : fby_expression { $$ = $1;}
    | fby_expression AT CONSTANT { $$ = nodeNew(AT, NULL, $3, 0, $1, NULL); }
    ;

fby_expression
    : unary_expression { $$ = $1; }
    | unary_expression FBY fby_expression { $$ = basicNodeNew(FBY, $1, $3); }
    ;

unary_expression
    : primary_expression { $$ = $1; }
    | FIRST unary_expression { $$ = basicNodeNew(FIRST, NULL, $2); }
    | NEXT unary_expression { $$ = basicNodeNew(NEXT, NULL, $2); }
    | IF expression THEN expression ELSE unary_expression { $$ = basicNodeNew(IF, $2, basicNodeNew(THEN, $4, $6)); }
    | ABS unary_expression { $$ = basicNodeNew(ABS, NULL, $2); }
    ;

primary_expression
    : IDENTIFIER { $$ = nodeNew(IDENTIFIER, $1, 0, 0, NULL, NULL); }
    | IDENTIFIER '[' expression ']' { $$ = nodeNew(ARR_IDENTIFIER, $1, 0, 0, NULL, $3);}
    | CONSTANT { $$ = nodeNew(CONSTANT, NULL, $1, 0, NULL, NULL); }
    | '(' expression ')' { $$ = $2; }
    ;

%%

extern FILE *yyin;

int main(int argc, char *argv[]) {
    //setting the memory to be unlimit
    #undef YYMAXDEPTH
    #define YYMAXDEPTH 100000
    #if defined(__linux__)
    struct rlimit x;
    if (getrlimit(RLIMIT_STACK, &x) < 0)
        perror("getrlimit");
    x.rlim_cur = RLIM_INFINITY;
    if (setrlimit(RLIMIT_STACK, &x) < 0)
        perror("setrlimit");
    #endif
    
    int i;
    char *filename;

    my_argc = argc;
    my_argv = argv;

    filename = NULL;
    i = 1;
    while (filename == NULL && i < argc) {
        if (argv[i][0] != '-') {
            filename = argv[i];
        }
        i++;
    }

    if (filename != NULL) {
        yyin = fopen(filename, "r");
    }

    yyparse();

    if (filename != NULL) {
        fclose(yyin);
    }

    return 0;
}

void yyerror(const char *msg) {
    fprintf(stdout, "Line %d: %s\n", line_num, msg);
    exit(1);
}
