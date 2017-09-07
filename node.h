#ifndef __NODE_H
#define __NODE_H

// Node data structure for the AST of the constraint program.
typedef struct _Node {
    int token;
    char *str;
    int num1;
    int num2;
    struct _Node *left;
    struct _Node *right;
} Node;

Node *nodeNew(int token, char *str, int num1, int num2, Node *left, Node *right);
void nodeFree(Node *node);
void nodeDraw(Node *node, char *filename);

#endif
