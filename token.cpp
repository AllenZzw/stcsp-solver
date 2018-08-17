#include <cstdio>
#include <cstdlib>
#include <cstring>
#include "util.h"
#include "node.h"
#include "token.h"
#include "y.tab.h"

Token *tokenNew(int id, char *str, int level) {
    Token *token = (Token *)myMalloc(sizeof(Token));
    token->id = id;
    token->str = strdup(str);
    token->level = level;
    token->next = NULL;
    return token;
} 

void tokenFree(Token *token) {
    free(token->str);
    myFree(token);
}

void tokenTableAddToken(TokenTable *table, int id, char *str, int level) {
    Token *token = tokenNew(id, str, level);
    if (table->size == 0) {
        table->head = token;
    } else {
        table->tail->next = token;
    }
    table->tail = token;
    table->size++;
}

void tokenTableInit(TokenTable *table) {
    /* constraint_operator */
    tokenTableAddToken(table, '<', "<", 1);
    tokenTableAddToken(table, '>', ">", 1);
    tokenTableAddToken(table, LE_CON, "<=", 1);
    tokenTableAddToken(table, GE_CON, ">=", 1);
    tokenTableAddToken(table, EQ_CON, "==", 1);
    tokenTableAddToken(table, NE_CON, "!=", 1);
    tokenTableAddToken(table, UNTIL_CON, "until", 1);
    tokenTableAddToken(table, IMPLY_CON, "->", 1);

    /* logical_not_expression */
    tokenTableAddToken(table, NOT_OP, "not", 2);

    /* logical_or_expression */
    tokenTableAddToken(table, OR_OP, "or", 3);

    /* logical_and_expression */
    tokenTableAddToken(table, AND_OP, "and", 4);



    /* equality_expression */
    tokenTableAddToken(table, EQ_OP, "eq", 5);
    tokenTableAddToken(table, NE_OP, "ne", 5);

    /* relational_operator */
    tokenTableAddToken(table, LT_OP, "lt", 6);
    tokenTableAddToken(table, GT_OP, "gt", 6);
    tokenTableAddToken(table, LE_OP, "le", 6);
    tokenTableAddToken(table, GE_OP, "ge", 6);

    /* additive_expression */
    tokenTableAddToken(table, '+', "+", 7);
    tokenTableAddToken(table, '-', "-", 7);

    /* multiplicative_expression */
    tokenTableAddToken(table, '*', "*", 8);
    tokenTableAddToken(table, '/', "/", 8);
    tokenTableAddToken(table, '%', "%", 8);

    /* at_expression */
    tokenTableAddToken(table, AT, "@", 9); 

    /* fby_expression */
    tokenTableAddToken(table, FBY, "fby", 10);

    /* unary_expression */
    tokenTableAddToken(table, FIRST, "first", 11);
    tokenTableAddToken(table, NEXT, "next", 11);
    tokenTableAddToken(table, IF, "if", 11);
    tokenTableAddToken(table, THEN, "then", 11);
    tokenTableAddToken(table, ABS, "abs", 11);
}

TokenTable *tokenTableNew() {
    TokenTable *table = (TokenTable *)myMalloc(sizeof(TokenTable));
    table->head = NULL;
    table->tail = NULL;
    table->size = 0;
    tokenTableInit(table);
    return table;
}

void tokenTableFree(TokenTable *table) {
    Token *token, *temp;
    token = table->head;
    while (token != NULL) {
        temp = token;
        token = token->next;
        tokenFree(temp);
    }
    myFree(table);
}

Token *tokenTableGetToken(TokenTable *table, int id) {
    Token *token = NULL;
    Token *tempToken = table->head;
    while (token == NULL && tempToken != NULL) {
        if (tempToken->id == id) {
            token = tempToken;
        } else {
            tempToken = tempToken->next;
        }
    }
    return token;
}

char *tokenString(TokenTable *table, int id) {
    Token *token = tokenTableGetToken(table, id);
    return token->str;
}

int tokenLevel(TokenTable *table, int id) {
    Token *token = tokenTableGetToken(table, id);
    return token->level;
}
