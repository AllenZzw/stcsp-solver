#ifndef __SOLVER_H
#define __SOLVER_H

#include "token.h"
#include "node.h"
#include "variable.h"
#include "constraint.h"
#include "graph.h"
#include <vector>
#include <deque>
using namespace std;

// An arc is a (var, constraint) pair, in the generalised arc consistency sense.
struct Arc {
    Constraint *constr;
    Variable *var;
    bool inqueue;
};

typedef deque<Arc *> ArcQueue;

struct Solver {
    TokenTable *tokenTable; // Token table for printing out the internal representation of the St-CSP.
    VariableQueue *varQueue; // List of variables.
    ArrayQueue *arrayQueue; // List of prefix/array.
    ConstraintQueue *constrQueue; // (Current) List of constraints.
    int numAuxVar; // Number of auxiliary variables introduced in normalisation.
    int numNodes; // Number of states in the output automaton.
    int numFails; // Number of failures encountered. Note that a better consistency notion might give a lower numFails.
    int numSolutions; // Not used?
    double initTime; // Time taken for initialisation, i.e. parsing AST into constraint representation.
    double solveTime; // Time for solver to taken for solve the St-CSP.
    double processTime; // Time for solver to post-processing.
    int printSolution; // Boolean flag - whether to output the automaton or not.
    
    int prefixK; // Value K for prefix-K consistency enforcement.
    int numSignVar; // Number of signature variables.
    int numUntil; // Number of until signature variables.
    int numDominance; // Number of dominances detected, asymptotically equal to the number of edges in the automaton.
    int timePoint; // Denotes the current time point of the search state.
    Graph *graph; // Automaton
    int propagateTimestamp; //
    int constraintID; // Denotes the ID of the current set of constraints.
    vector<ConstraintQueue *> *seenConstraints; // List of all *sets* of constraints seen in the search.
    bool hasFirst; // Flag for whether the first operator is involved in the constraints.
    bool adversarial1; // Flag for whether to generate an adversarial plan.
	bool adversarial2; // Flag for whether to generate an simultaneous adversarial plan.
    ArcQueue *arcQueue; // Queue for enforcing prefix-k/GAC consistency.
};

Variable *solverGetVar(Solver *solver, char *name);
Variable *solverGetFirstUnboundVar(Solver *solver);

Array *solverGetArray(Solver *solver, char *name);

void solve(Node *node);
Variable *solverAuxVarNew(Solver *solver, char *var_name, int lb, int ub);
void solverAddConstrNode(Solver *solver, ConstraintNode *node);

Arc *arcNew(Constraint *constr, Variable *var);
//void arcQueueFree(ArcQueue * arcs);
bool arcQueueFind(ArcQueue *queue, Arc *arc);

#endif
