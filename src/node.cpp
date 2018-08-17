#include <cstdio>
#include <cstdlib>
#include <cstring>
#include "util.h"
#include "node.h"
#include "y.tab.h"

Node *nodeNew(int token, char *str, int num1, int num2, Node *left, Node *right) {
    Node *node;

    node = (Node *)myMalloc(sizeof(Node));
    node->token = token;
    node->str = str;
    node->num1 = num1;
    node->num2 = num2;
    node->left = left;
    node->right = right;

    return node;
}

void nodeFree(Node *node) {
    if (node->left != NULL) {
        nodeFree(node->left);
    }
    if (node->right != NULL) {
        nodeFree(node->right);
    }
    myFree(node);
}

void nodePrintDesc(FILE *fp, Node *node) {
    if (node->token == IDENTIFIER) {
        fprintf(fp, "%s", node->str);
    } else if (node->token == ARR_IDENTIFIER) {
        fprintf(fp, "%d", node->str);
    } else if (node->token == CONSTANT) {
        fprintf(fp, "%d", node->num1);
    } else if (node->token == RANGE) {
        fprintf(fp, "%d, %d", node->num1, node->num2);
    } else if (node->token == VAR) {
        fprintf(fp, "var: %s", node->str);
    } else if (node->token == STATEMENT) {
        fprintf(fp, "#");
    } else if (node->token == '<') {
        fprintf(fp, "<");
    } else if (node->token == '>') {
        fprintf(fp, ">");
    } else if (node->token == LE_CON) {
        fprintf(fp, "<=");
    } else if (node->token == GE_CON) {
        fprintf(fp, ">=");
    } else if (node->token == EQ_CON) {
        fprintf(fp, "==");
    } else if (node->token == NE_CON) {
        fprintf(fp, "!=");
    } else if (node->token == IMPLY_CON) {
        fprintf(fp, "->");
    } else if (node->token == UNTIL_CON) {
        fprintf(fp, "until");
    }else if (node->token == LT_OP) {
        fprintf(fp, "lt");
    } else if (node->token == GT_OP) {
        fprintf(fp, "gt");
    } else if (node->token == LE_OP) {
        fprintf(fp, "le");
    } else if (node->token == GE_OP) {
        fprintf(fp, "ge");
    } else if (node->token == EQ_OP) {
        fprintf(fp, "eq");
    } else if (node->token == NE_OP) {
        fprintf(fp, "ne");
    } else if (node->token == AND_OP) {
        fprintf(fp, "and");
    } else if (node->token == OR_OP) {
        fprintf(fp, "or");
    } else if (node->token == NOT_OP) {
        fprintf(fp, "not");
    } else if (node->token == AT ) {
        fprintf(fp, "@");
    } else if (node->token == FIRST) {
        fprintf(fp, "first");
    } else if (node->token == NEXT) {
        fprintf(fp, "next");
    } else if (node->token == FBY) {
        fprintf(fp, "fby");
    } else if (node->token == IF) {
        fprintf(fp, "if");
    } else if (node->token == THEN) {
        fprintf(fp, "then");
    } else if (node->token == ABS) {
        fprintf(fp, "abs");
    } else if (node->token < 256) {
        fprintf(fp, "%c", (char)node->token);
    } else {
        fprintf(fp, "#%d", node->token);
    }
}

void nodeDrawRe(FILE *fp, Node *node) {
    if (node != NULL) {
        fprintf(fp, "%ld [label=\"", (long)node);
        nodePrintDesc(fp, node);
        fprintf(fp, "\"];\n");
        if (node->left != NULL) {
            fprintf(fp, "%ld -> %ld;\n", (long)node, (long)node->left);
        }
        if (node->right != NULL) {
            fprintf(fp, "%ld -> %ld;\n", (long)node, (long)node->right);
        }
        nodeDrawRe(fp, node->left);
        nodeDrawRe(fp, node->right);
    }
}

void nodeDraw(Node *node, char *filename) {
    FILE *fp;
    fp = fopen(filename, "w");
    fprintf(fp, "digraph \"AST\" {\n");
    nodeDrawRe(fp, node);
    fprintf(fp, "}\n");
    fclose(fp);
}

streamListNode *streamListNodeNew(streamListNode* next_ptr, int daton){
    streamListNode *node = (streamListNode *)myMalloc(sizeof(streamListNode));
    node->next = next_ptr;
    node->daton = daton;
    return node;
}