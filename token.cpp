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

    /* logical_or_expression */
    tokenTableAddToken(table, OR_OP, "or", 2);

    /* logical_and_expression */
    tokenTableAddToken(table, AND_OP, "and", 3);

    /* equality_expression */
    tokenTableAddToken(table, EQ_OP, "eq", 4);
    tokenTableAddToken(table, NE_OP, "ne", 4);

    /* relational_operator */
    tokenTableAddToken(table, LT_OP, "lt", 5);
    tokenTableAddToken(table, GT_OP, "gt", 5);
    tokenTableAddToken(table, LE_OP, "le", 5);
    tokenTableAddToken(table, GE_OP, "ge", 5);

    /* additive_expression */
    tokenTableAddToken(table, '+', "+", 6);
    tokenTableAddToken(table, '-', "-", 6);

    /* multiplicative_expression */
    tokenTableAddToken(table, '*', "*", 7);
    tokenTableAddToken(table, '/', "/", 7);
    tokenTableAddToken(table, '%', "%", 7);

    /* at_expression */
    tokenTableAddToken(table, AT, "@", 8); 

    /* fby_expression */
    tokenTableAddToken(table, FBY, "fby", 9);

    /* unary_expression */
    tokenTableAddToken(table, FIRST, "first", 10);
    tokenTableAddToken(table, NEXT, "next", 10);
    tokenTableAddToken(table, IF, "if", 10);
    tokenTableAddToken(table, THEN, "then", 10);
    tokenTableAddToken(table, ABS, "abs", 10);
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
