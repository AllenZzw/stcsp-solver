#include <cstdio>
#include <cstdlib>
#include <cstring>
#include "util.h"
#include "node.h"
#include "variable.h"
#include "solver.h"
#include "y.tab.h"

Variable *variableNew(Solver *solver, char *name, int lb, int ub) {
    int i;
    Variable *var = (Variable *)myMalloc(sizeof(Variable));
    var->solver = solver;
    var->name = strdup(name);
    if (lb > ub) {
        myLog(LOG_ERROR, "Invalid domain [%d, %d] in variable %s\n", lb, ub, name);
        exit(1);
    }
    
    var->lb = lb;
    var->ub = ub;
    
    var->prevValue = 0;
    var->currLB = (int *)myMalloc(sizeof(int) * solver->prefixK);
    var->currUB = (int *)myMalloc(sizeof(int) * solver->prefixK);
    for (i = 0; i < solver->prefixK; i++) {
        var->currLB[i] = lb;
        var->currUB[i] = ub;
    }
    
    var->propagateTimestamp = 0;
    var->propagateValue = 0;
    var->isSignature = 0;
    
    var->constraints = new vector<Constraint *>();
    var->numConstr = 0;
    
    myLog(LOG_TRACE, "var %s : [ %d, %d ];\n", name, lb, ub);
    return var;
}

void variableFree(Variable *var) {
    free(var->name);
    constraintQueueFree(var->constraints);
    myFree(var->currLB);
    myFree(var->currUB);
    myFree(var);
}

// Search only the lower half of the current domain of the variable.
void variableSplitLower(Variable *var) {
    backup(&var->currUB[0]);
    var->currUB[0] = var->currLB[0] + ((var->currUB[0] - var->currLB[0]) / 2);
    /*
    myLog(LOG_NOTICE, "  Lower: %s [%d, %d]\n", var->name, var->currLB, var->currUB);
    */
}

// Search only the upper half of the current domain of the variable.
void variableSplitUpper(Variable *var) {
    backup(&var->currLB[0]);
    var->currLB[0] = var->currLB[0] + ((var->currUB[0] - var->currLB[0]) / 2) + 1;
    /*
    myLog(LOG_NOTICE, "  Upper: %s [%d, %d]\n", var->name, var->currLB, var->currUB);
    */
}

void variableSetLB(Variable *var, int lb) {
    backup(&var->currLB[0]);
    var->currLB[0] = lb;
    /*myLog(LOG_INFO, "  %s set lb to %d\n", var->name, lb);*/
}

void variableSetUB(Variable *var, int ub) {
    backup(&var->currUB[0]);
    var->currUB[0] = ub;
    /*myLog(LOG_INFO, "  %s set ub to %d\n", var->name, ub);*/
}

void variableSetLBAt(Variable *var, int lb, int i) {
    backup(&var->currLB[i]);
    var->currLB[i] = lb;
    /*myLog(LOG_INFO, "  %s set lb to %d\n", var->name, lb);*/
}

void variableSetUBAt(Variable *var, int ub, int i) {
    backup(&var->currUB[i]);
    var->currUB[i] = ub;
    /*myLog(LOG_INFO, "  %s set ub to %d\n", var->name, ub);*/
}

// Shifts a variable by one time point.
void variableAdvanceOneTimeStep(Solver *solver, Variable *var) {
    backup(&var->prevValue);
    for (int i = 0; i < solver->prefixK; i++){
        backup(&var->currLB[i]);
        backup(&var->currUB[i]);
    }
    
    var->prevValue = variableGetValue(var);
    for (int i = 0; i < solver->prefixK - 1; i++){
        var->currLB[i] = var->currLB[i + 1];
        var->currUB[i] = var->currUB[i + 1];
    }
    var->currLB[solver->prefixK - 1] = var->lb;
    var->currUB[solver->prefixK - 1] = var->ub;
}

VariableQueue *variableQueueNew() {
    VariableQueue *queue = new vector<Variable *>();
    return queue;
}

void variableQueueFree(VariableQueue *queue) {
    /*Variable *var;
    var = queue->head;
    while (var != NULL) {
        variableFree(var);
        var = var->next;
    }
    myFree(queue);*/
    delete queue;
}

void variableQueuePush(VariableQueue *queue, Variable *var) {
    queue->push_back(var);
}

bool variableQueueFind(VariableQueue *queue, Variable *var) {
    int size = queue->size();
    bool found = false;
    for (int c = 0; !found && c < size; c++) {
        if (var == (*queue)[c]) {
            found = true;
        }
    }
    return found;
}

void variablePrint(Variable *var) {
    if (var->isSignature) {
        myLog(LOG_TRACE, "Variable %s [S]\n", var->name);
    } else {
        myLog(LOG_TRACE, "Variable %s\n", var->name);
    }
    myLog(LOG_TRACE, "Involved in constraints: ");
    int size = var->numConstr;
    for (int i = 0; i < size-1; i++) {
        myLog(LOG_TRACE, "%d ", (*(var->constraints))[i]->id);
    }
    if (size > 0) {
        myLog(LOG_TRACE, "%d ", (*(var->constraints))[size-1]->id);
    }
    myLog(LOG_TRACE, "\n\n");
}
