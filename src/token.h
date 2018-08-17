#ifndef __TOKEN_H
#define __TOKEN_H

struct Token {
    int id;
    char *str;
    int level;
    struct Token *next;
};

struct TokenTable {
    Token *head;
    Token *tail;
    int size;
};

Token *tokenNew(int id, char *str, int level);
void tokenFree(Token *token);

TokenTable *tokenTableNew();
void tokenTableFree(TokenTable *table);
char *tokenString(TokenTable *table, int id);
int tokenLevel(TokenTable *table, int id);

#endif
