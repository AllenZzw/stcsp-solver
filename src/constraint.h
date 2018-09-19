#ifndef __CONSTRAINT_H
#define __CONSTRAINT_H

#include "node.h"
#include <vector>
#include <deque>
using namespace std;

struct Solver;
struct Variable;
struct Array;
struct Arc;
typedef vector<Variable *> VariableQueue;
typedef vector<Array *> ArrayQueue;
typedef deque<Arc *> ArcQueue;

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
    Array *array; // If the node is a array/identifier node, the array
    ConstraintNode *left;
    ConstraintNode *right;
};

#define CONSTR_NEXT      0
#define CONSTR_POINT     1
#define CONSTR_UNTIL     2
#define CONSTR_AT        3

struct Constraint {
    Solver *solver; // Solver that owns the constraint
    ConstraintNode *node; // Syntactic structure of the constraint
    int type; // Either a CONSTR_NEXT or CONSTR_POINT or CONSTR_UNTIL
    int id; // ID of the constraint
    int expire; // for until constraint, check whether it is expired or not
    VariableQueue *variables; // List of variables the constraint involves
    ArrayQueue *arraies; // List of arraies the constraint involves
    int numVar; // Number of variables it has
    bool hasFirst; // Whether the constraint has a first operator in it
    ArcQueue *arcs; // List of arcs related to this constraint 
};

typedef vector<Constraint *> ConstraintQueue;

ConstraintNode *constraintNodeNew(int token, int num, Variable *var, Array* array, ConstraintNode *left, ConstraintNode *right);
void constraintNodeFree(ConstraintNode *node);
ConstraintNode *constraintNodeNewVar(Variable *var);
ConstraintNode *constraintNodeNewConstant(int num);
ConstraintNode *constraintNodeNewFirst(ConstraintNode *node);
ConstraintNode *constraintNodeNewNext(ConstraintNode *node);
ConstraintNode *constraintNodeNewAt(Variable *var, int timepoint);
ConstraintNode *constraintNodeParse(Solver *solver, Node *node);
void constraintNodeLogPrint(ConstraintNode *node, Solver *solver);

Constraint *constraintNew(Solver *solver, ConstraintNode *node, int expire = 0);
Constraint *constraintNew(Solver *solver, ConstraintNode *node, int expire);
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
