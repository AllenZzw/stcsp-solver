#ifndef __CONSTRAINT_H
#define __CONSTRAINT_H

#include "node.h"
#include <vector>
using namespace std;

struct Solver;
struct Variable;
typedef vector<Variable *> VariableQueue;

// Used for evaluating the value of a ConstraintNode.
// Bottom element denotes an unknown value.
struct LiftedInt {
    bool tag;
    int Int;
};

struct ConstraintNode {
    int token; // Token representing the node
    int num; // If the node is a constant, the actual number
    Variable *var; // If the node is a variable/identifier node, the variable
    ConstraintNode *left;
    ConstraintNode *right;
};

#define CONSTR_NEXT      0
#define CONSTR_POINT     1

struct Constraint {
    Solver *solver; // Solver that owns the constraint
    ConstraintNode *node; // Syntactic structure of the constraint
    int type; // Either a CONSTR_NEXT or CONSTR_POINT
    int id; // ID of the constraint
    VariableQueue *variables; // List of variables the constraint involves
    int numVar; // Number of variables it has
    bool hasFirst; // Whether the constraint has a first operator in it
};

typedef vector<Constraint *> ConstraintQueue;

ConstraintNode *constraintNodeNew(int token, int num, Variable *var, ConstraintNode *left, ConstraintNode *right);
void constraintNodeFree(ConstraintNode *node);
ConstraintNode *constraintNodeNewVar(Variable *var);
ConstraintNode *constraintNodeNewConstant(int num);
ConstraintNode *constraintNodeNewFirst(ConstraintNode *node);
ConstraintNode *constraintNodeNewNext(ConstraintNode *node);
ConstraintNode *constraintNodeParse(Solver *solver, Node *node);
void constraintNodeLogPrint(ConstraintNode *node, Solver *solver);

Constraint *constraintNew(Solver *solver, ConstraintNode *node);
void constraintFree(Constraint *constr);

ConstraintQueue *constraintQueueNew();
void constraintQueueFree(ConstraintQueue *queue);
void solverConstraintQueuePush(ConstraintQueue *queue, Constraint *constr, Solver *solver, bool checkForFirst);
void constraintQueuePush(ConstraintQueue *queue, Constraint *constr);

Variable *constraintGetUnboundVar(Constraint *constr);

void constraintPrint(Constraint *constr);

Constraint *constraintTranslate(Constraint *constr, Solver *solver);

bool constraintQueueEq(ConstraintQueue *q1, ConstraintQueue *q2);

bool constraintNodeTautology(ConstraintNode *constrNode);

#endif
