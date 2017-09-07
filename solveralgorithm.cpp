#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <unistd.h>
#include "util.h"
#include "token.h"
#include "node.h"
#include "variable.h"
#include "constraint.h"
#include "solver.h"
#include "solveralgorithm.h"
#include "y.tab.h"
#include "graph.h"

/* OmegaSolver */
void solverAddConstr(Solver *solver, Node *node) {
    int lb, ub;
    Constraint *constr = constraintNew(solver, constraintNormalise(solver, constraintNodeParse(solver, node), lb, ub));
    if (!constraintNodeTautology(constr->node)) {
        solverConstraintQueuePush(solver->constrQueue, constr, solver, true);
    } else {
        constraintFree(constr);
    }
}

/* OmegaSolver */
void solverAddConstrFirstEqFirst(Solver *solver, Variable *x, Variable *y) {
    solverAddConstrNode(solver, constraintNodeNew(EQ_CON, 0, NULL,
                                                  constraintNodeNewFirst(constraintNodeNewVar(x)),
                                                  constraintNodeNewFirst(constraintNodeNewVar(y))));
}

/* OmegaSolver */
void solverAddConstrVarEqNext(Solver *solver, Variable *x, Variable *y) {
    solverAddConstrNode(solver, constraintNodeNew(EQ_CON, 0, NULL,
                                constraintNodeNewVar(x),
                                constraintNodeNewNext(constraintNodeNewVar(y))));
}

/* OmegaSolver */
void solverAddConstrVarEqNode(Solver *solver, Variable *x, ConstraintNode *node) {
    solverAddConstrNode(solver, constraintNodeNew(EQ_CON, 0, NULL,
                                constraintNodeNewVar(x),
                                node));
}

/* OmegaSolver */
ConstraintNode *constraintNormalise(Solver *solver, ConstraintNode *node, int &lb, int &ub) {
    ConstraintNode *constrNode = NULL;
    ConstraintNode *constrNodeLeft = NULL;
    ConstraintNode *constrNodeRight = NULL;
    Variable *x, *y, *z;
    int myLB, myUB, myLB2, myUB2;
    
    if (node != NULL) {
        if (node->token != IDENTIFIER && node->token != CONSTANT) {
            myLog(LOG_DETAILED_TRACE, "Token: %s\n", tokenString(solver->tokenTable, node->token));
        } else if (node->token != CONSTANT) {
            myLog(LOG_DETAILED_TRACE, "Identifier: %s\n", node->var->name);
        } else {
            myLog(LOG_DETAILED_TRACE, "Constant: %d\n", node->num);
        }
        if (node->token == FIRST) {
            if (node->right->token == CONSTANT) {
                lb = node->right->num;
                ub = node->right->num;
                constrNode = node->right;
                myFree(node);
            } else if (node->right->token == IDENTIFIER) {
                lb = node->right->var->lb;
                ub = node->right->var->ub;
                constrNode = node;
            } else if (node->right->token == FBY) {
                constrNodeRight = node->right->left;
                constraintNodeFree(node->right->right);
                myFree(node->right);
                node->right = constrNodeRight;
                constrNode = constraintNormalise(solver, node, lb, ub);
            } else {
                constrNodeRight = constraintNormalise(solver, node->right, myLB, myUB);
                if (constrNodeRight->token == FIRST) {
                    ConstraintNode *temp = constrNodeRight->right;
                    myFree(constrNodeRight);
                    constrNodeRight = temp;
                }
                node->right = constrNodeRight;
                constrNode = node;
                lb = myLB;
                ub = myUB;
            }
        } else if (node->token == NEXT) {
            if (node->right->token == CONSTANT) {
                lb = node->right->num;
                ub = node->right->num;
                constrNode = node->right;
                myFree(node);
            } else if (node->right->token == IDENTIFIER) {
                lb = node->right->var->lb;
                ub = node->right->var->ub;
                x = solverAuxVarNew(solver, NULL, lb, ub);
                solverAddConstrVarEqNext(solver, x, node->right->var);
                myFree(node->right);
                myFree(node);
                constrNode = constraintNodeNew(IDENTIFIER, 0, x, NULL, NULL);
            } else if (node->right->token == FBY) {
                constrNode = node->right->right;
                constraintNodeFree(node->right->left);
                myFree(node->right);
                myFree(node);
                constrNode = constraintNormalise(solver, constrNode, lb, ub);
            } else {
                constrNodeRight = constraintNormalise(solver, node->right, myLB, myUB);
                if (constrNodeRight->token == FIRST) {
                    myFree(node);
                    constrNode = constrNodeRight;
                } else if (constrNodeRight->token == CONSTANT || constrNodeRight->token == IDENTIFIER) {
                    node->right = constrNodeRight;
                    constrNode = constraintNormalise(solver, node, myLB, myUB);
                } else {
                    x = solverAuxVarNew(solver, NULL, myLB, myUB);
                    solverAddConstrVarEqNode(solver, x, constrNodeRight);
                    y = solverAuxVarNew(solver, NULL, myLB, myUB);
                    solverAddConstrVarEqNext(solver, y, x);
                    constrNode = constraintNodeNew(IDENTIFIER, 0, y, NULL, NULL);
                    myFree(node);
                }
                lb = myLB;
                ub = myUB;
            }
        } else if (node->token == FBY) {
            if (node->left->token == IDENTIFIER) {
                myLB = node->left->var->lb;
                myUB = node->left->var->ub;
                y = node->left->var;
                myFree(node->left);
            } else {
                constrNodeLeft = constraintNormalise(solver, node->left, myLB, myUB);
                y = solverAuxVarNew(solver, NULL, myLB, myUB);
                solverAddConstrVarEqNode(solver, y, constrNodeLeft);
            }
            if (node->right->token == IDENTIFIER) {
                myLB2 = node->right->var->lb;
                myUB2 = node->right->var->ub;
                z = node->right->var;
                myFree(node->right);
            } else {
                constrNodeRight = constraintNormalise(solver, node->right, myLB2, myUB2);
                z = solverAuxVarNew(solver, NULL, myLB2, myUB2);
                solverAddConstrVarEqNode(solver, z, constrNodeRight);
            }
            lb = MIN(myLB, myLB2);
            ub = MAX(myUB, myUB2);
            x = solverAuxVarNew(solver, NULL, lb, ub);
            solverAddConstrFirstEqFirst(solver, x, y);
            solverAddConstrVarEqNext(solver, z, x);
            constrNode = constraintNodeNew(IDENTIFIER, 0, x, NULL, NULL);
            myFree(node);
        } else if (node->token == IDENTIFIER || node->token == CONSTANT) {
            if (node->token == IDENTIFIER) {
                lb = node->var->lb;
                ub = node->var->ub;
            } else {
                lb = node->num;
                ub = node->num;
            }
            constrNode = node;
        } else {
            //myLog(LOG_TRACE, "before left\n");
            //if (node->left->token == IDENTIFIER) {
            //    myLog(LOG_TRACE, "id: %s\n", node->left->var->name);
            //}
            constrNodeLeft = constraintNormalise(solver, node->left, myLB, myUB);
            //myLog(LOG_TRACE, "before right\n");
            constrNodeRight = constraintNormalise(solver, node->right, myLB2, myUB2);
            //myLog(LOG_TRACE, "after right\n");

            if (node->token == ABS) {
                if (myLB2 < 0 && myUB2 < 0) {
                    lb = -myUB2;
                    ub = -myLB2;
                } else if (myLB2 < 0 && myUB2 > 0) {
                    lb = 0;
                    ub = MAX(-myLB2, myUB2);
                } else {
                    lb = myLB2;
                    ub = myUB2;
                }
            } else if (node->token == IF) {
                lb = myLB2;
                ub = myUB2;
            } else if (node->token == THEN) {
                lb = MIN(myLB, myLB2);
                ub = MAX(myUB, myUB2);
            } else if (node->token == '<' || node->token == '>' || node->token == LE_CON || node->token == GE_CON ||
                       node->token == EQ_CON || node->token == NE_CON ||
                       node->token == LT_OP || node->token == GT_OP || node->token == LE_OP || node->token == GE_OP ||
                       node->token == EQ_OP || node->token == NE_OP ||
                       node->token == AND_OP || node->token == OR_OP) {
                lb = 0;
                ub = 1;
            } else if (node->token == '+') {
                lb = myLB + myLB2;
                ub = myUB + myUB2;
            } else if (node->token == '-') {
                lb = myLB - myUB2;
                ub = myUB - myLB2;
            } else if (node->token == '*') {
                if (myLB >= 0 && myLB2 >= 0) {
                    lb = myLB * myLB2;
                    ub = myUB * myUB2;
                } else if (myLB >= 0 && myUB2 >= 0 && myLB2 < 0) {
                    lb = myUB * myLB2;
                    ub = myUB * myUB2;
                } else if (myUB >= 0 && myLB < 0 && myLB2 >= 0) {
                    lb = myLB * myLB2;
                    ub = myUB * myUB2;
                } else {
                    lb = myUB * myUB2;
                    ub = myLB * myLB2;
                }
            } else if (node->token == '/') {
                lb = MY_INT_MIN;
                ub = MY_INT_MAX;
            } else if (node->token == '%') {
                lb = MY_INT_MIN;
                ub = MY_INT_MAX;
            }

            node->left = constrNodeLeft;
            node->right = constrNodeRight;
            constrNode = node; //constraintNodeNew(node->token, 0, NULL, constrNodeLeft, constrNodeRight);
        }
    } else {
        myLog(LOG_ERROR, "ConstraintNode to be normalised is NULL!\n");
    }
    return constrNode;
}

/* OmegaSolver */
int solverValidateRe(ConstraintNode *node) {
    int left = 0;
    int right = 0;
    int temp = 0;
    int res = 0;
    if (node != NULL) {
        if (node->token == IDENTIFIER) {
            res = node->var->propagateValue;
        } else if (node->token == CONSTANT) {
            res = node->num;
        } else if (node->token == ABS) {
            res = abs(solverValidateRe(node->right));
        } else if (node->token == IF) {
            temp = solverValidateRe(node->left);
            if (temp) {
                res = solverValidateRe(node->right->left);
            } else {
                res = solverValidateRe(node->right->right);
            }
        } else if (node->token == FIRST) {
            res = solverValidateRe(node->right);
        } else if (node->token == AND_OP) {
            temp = solverValidateRe(node->left);
            if (temp) {
                res = solverValidateRe(node->right);
            } else {
                res = 0;
            }
        } else if (node->token == OR_OP) {
            temp = solverValidateRe(node->left);
            if (temp) {
                res = 1;
            } else {
                res = solverValidateRe(node->right);
            }
        } else {
            left = solverValidateRe(node->left);
            right = solverValidateRe(node->right);
            if (node->token == '<' || node->token == LT_OP) {
                res = (left < right);
            } else if (node->token == '>' || node->token == GT_OP) {
                res = (left > right);
            } else if (node->token == LE_CON || node->token == LE_OP) {
                res = (left <= right);
            } else if (node->token == GE_CON || node->token == GE_OP) {
                res = (left >= right);
            } else if (node->token == EQ_CON || node->token == EQ_OP) {
                res = (left == right);
            } else if (node->token == NE_CON || node->token == NE_OP) {
                res = (left != right);
            } else if (node->token == '+') {
                res = left + right;
            } else if (node->token == '-') {
                res = left - right;
            } else if (node->token == '*') {
                res = left * right;
            } else if (node->token == '/') {
                res = left / right;
            } else if (node->token == '%') {
                res = left % right;
            }
        }
    }
    return res;
}

// Validates a constraint given propagation values. Used in consistency enforcement
// and hence implicitly for constraint checking.
bool validate(Constraint *constr) {
    return solverValidateRe(constr->node);
}

// Recursive search for findSupport. Recursing in index, i.e. the index of the current
// searched variable in constr's variable list.
bool findSupportRe(Constraint *constr, Variable *var, int point, int index, int numVar) {
    Variable *thisVar = (*constr->variables)[index];
    if (index == numVar-1) {
        if (thisVar == var) {
            return validate(constr);
        } else {
            bool supported = false;
            int lb = thisVar->currLB[point];
            int ub = thisVar->currUB[point];
            for (int c = lb; !supported && c <= ub; c++) {
                thisVar->propagateValue = c;
                supported = validate(constr);
            }
            return supported;
        }
    } else {
        if (thisVar == var) {
            return findSupportRe(constr, var, point, index+1, numVar);
        } else {
            bool supported = false;
            int lb = thisVar->currLB[point];
            int ub = thisVar->currUB[point];
            for (int c = lb; !supported && c <= ub; c++) {
                thisVar->propagateValue = c;
                supported = findSupportRe(constr, var, point, index+1, numVar);
            }
            return supported;
        }
    }
}

// Returns true if there is a support for var in constr at time point.
bool findSupport(Constraint *constr, Variable *var, int point) {
    int numVar = constr->variables->size();
    return findSupportRe(constr, var, point, 0, numVar);
}

// Assumes constr is a pointwise constraint.
// Returns true if there is no inconsistency for the arc (var, constr) at time point.
bool enforcePointConsistencyAt(Constraint *constr, Variable *var, bool &change, int point) {
    bool supported = false;
    int lb = var->currLB[point];
    int ub = var->currUB[point];
    
    for (int c = lb; !supported && c <= ub; c++) {
        var->propagateValue = c;
        supported = findSupport(constr, var, point);
    }
    
    // If no support for lower bound then inconsistent
    if (supported) {
        if (lb != var->propagateValue) {
            change = true;
            backup(&var->currLB[point]);
            var->currLB[point] = var->propagateValue;
        }
        supported = false;
        lb = var->currLB[point];
        for (int c = ub; !supported && c > lb; c--) {
            var->propagateValue = c;
            supported = findSupport(constr, var, point);
        }
        
        if (!supported) {
            if (ub != var->currLB[point]) {
                change = true;
                backup(&var->currUB[point]);
                var->currUB[point] = var->currLB[point];
            }
            supported = true;
        } else {
            if (ub != var->propagateValue) {
                change = true;
                backup(&var->currUB[point]);
                var->currUB[point] = var->propagateValue;
            }
        }
    }
    return supported;
}

// Assumes constr is a pointwise constraint.
// Returns true if the arc (var, constr) can be made consistent at all k time points.
bool enforcePointConsistency(Solver *solver, Constraint *constr, Variable *var, bool &change) {
    if (!constr->hasFirst) {
        bool consistent = true;
        int k = solver->prefixK;
        for (int c = 0; consistent && c < k; c++) {
            consistent = enforcePointConsistencyAt(constr, var, change, c);
        }
        return consistent;
    } else {
        return enforcePointConsistencyAt(constr, var, change, 0);
    }
}

// Assumes constr is a constraint of form X == next Y.
// Returns true if the arc (var, constr) can be made consistent.
bool enforceNextConsistency(Solver *solver, Constraint *constr, Variable *var, bool &change) {
    bool consistent = true;
    if (var == constr->node->right->right->var) {
        int k = solver->prefixK;
        for (int c = 1; consistent && c < k; c++) {
            if (var->currLB[c] < constr->node->left->var->currLB[c-1]) {
                change = true;
                backup(&var->currLB[c]);
                var->currLB[c] = constr->node->left->var->currLB[c-1];
            }
            if (var->currUB[c] > constr->node->left->var->currUB[c-1]) {
                change = true;
                backup(&var->currUB[c]);
                var->currUB[c] = constr->node->left->var->currUB[c-1];
            }
            if (var->currLB[c] > var->currUB[c]) {
                consistent = false;
            }
        }
    } else {
        Variable *otherVar = constr->node->right->right->var;
        int k = solver->prefixK;
        for (int c = 0; consistent && c < k-1; c++) {
            if (var->currLB[c] < otherVar->currLB[c+1]) {
                change = true;
                backup(&var->currLB[c]);
                var->currLB[c] = otherVar->currLB[c+1];
            }
            if (var->currUB[c] > otherVar->currUB[c+1]) {
                change = true;
                backup(&var->currUB[c]);
                var->currUB[c] = otherVar->currUB[c+1];
            }
            if (var->currLB[c] > var->currUB[c]) {
                consistent = false;
            }
        }
    }
    return consistent;
}

// GAC algorithm
bool generalisedArcConsistent(Solver *solver) {
    bool consistent = true;
    bool change = false;
    // Push in all arcs
    int numConstr = solver->constrQueue->size();
    // print contraint queue
    for (int c = 0; c < numConstr; c++) {
        myLog(LOG_DEBUG, "constraint at this level\n");
        constraintNodeLogPrint((*(solver->constrQueue))[c]->node, solver);
    }

    for (int c = 0; c < numConstr; c++) {
        Constraint *constr = (*(solver->constrQueue))[c];
        int numVar = constr->numVar;
        VariableQueue *variables = constr->variables;
        for (int d = 0; d < numVar; d++) {
            Arc *temp = arcNew(constr, (*variables)[d]);
            solver->arcQueue->push_back(temp);
        }
    }
    // Enforce consistency
    while (consistent && !solver->arcQueue->empty()) {
        Arc *arc = solver->arcQueue->front();
        solver->arcQueue->pop_front();
        Constraint *constr = arc->constr;
        Variable *var = arc->var;
        
        // Enforce arc
        if (constr->type == CONSTR_NEXT) {
            consistent = enforceNextConsistency(solver, constr, var, change);
        } else if (constr->type == CONSTR_POINT) {
            consistent = enforcePointConsistency(solver, constr, var, change);
        }
        if (consistent && change) {
            // Push in relevant arcs
            ConstraintQueue *constraints = var->constraints;
            int size = constraints->size();
            for (int c = 0; c < size; c++) {
                if ((*constraints)[c] != constr) {
                    VariableQueue *variables = (*constraints)[c]->variables;
                    int numVar = variables->size();
                    for (int d = 0; d < numVar; d++) {
                        if ((*variables)[d] != var) {
                            Arc *temp = arcNew((*constraints)[c], (*variables)[d]);
                            if (!arcQueueFind(solver->arcQueue, temp)) {
                                solver->arcQueue->push_back(temp);
                            } else {
                                myFree(temp);
                            }
                        }
                    }
                }
            }
        }
        myFree(arc);
        change = false;
    }
    return consistent;
}

// Outputs solution in the DOT format.
void solverOut(Solver *solver) {
    FILE *fp = fopen("solutions.dot", "w");
    fprintf(fp, "# Number of nodes = %d\n", solver->numNodes);
    fprintf(fp, "#");
    int numVar = solver->varQueue->size();
    for (int c = 0; c < numVar; c++) {
        fprintf(fp, " %s", ((*(solver->varQueue))[c])->name);
    }
    fprintf(fp, "\n");
    fprintf(fp, "#");
    for (int c = 0; c < numVar; c++) {
        if (((*(solver->varQueue))[c])->isSignature) {
            fprintf(fp, " %s", ((*(solver->varQueue))[c])->name);
        }
    }
    fprintf(fp, "\n");
    fprintf(fp, "digraph \"StCSP\" {\n");
    fflush(fp);
    graphOut(solver->graph, fp, numVar, solver->numSignVar);
    fprintf(fp, "}\n");
    fclose(fp);
}

/* OmegaSolver */
int solverSolveRe(Solver *solver, Vertex *vertex) {
    Vertex *temp;
    Edge *edge;
    bool ok = false;
    
    Variable *var = solverGetFirstUnboundVar(solver);
    if (var == NULL) {
        int size = solver->varQueue->size();
        myLog(LOG_TRACE, "Before level up\n");
        levelUp();
        myLog(LOG_TRACE, "After level up\n");
        
        // timePoint
        backup(&solver->timePoint);
        solver->timePoint++;
        
        vector<ConstraintQueue *> varConstraints;
        ConstraintQueue *solverConstraintsBackup = solver->constrQueue;
        
        if (solver->timePoint == 1 && solver->hasFirst) {
            // Constraint translation and identification
            // Only applicable in the first time point, because only "first" constraints warrant translation
            // Use variable values for constraint translation
            
            for (int c = 0; c < size; c++) {
                Variable *var = (*(solver->varQueue))[c];
                backup(&var->numConstr);
                
                // Backup var's constraints
                varConstraints.push_back(var->constraints);
                var->constraints = constraintQueueNew();
                // Done
            }
            
            // Now translate constraints
            backup(&solver->constraintID);
            solver->constrQueue = constraintQueueNew();
            int oldNumConstr = solverConstraintsBackup->size();
            for (int c = 0; c < oldNumConstr; c++) {
                Constraint *output = constraintTranslate((*solverConstraintsBackup)[c], solver);
                if (output != NULL) {
                    solverConstraintQueuePush(solver->constrQueue, output, solver, false);
                }
            }
            // Done
            
            int tempnum = solver->constrQueue->size();
            for (int i = 0; i < tempnum; i++) {
                constraintPrint((*(solver->constrQueue))[i]);
            }
            
            // Check constraintID
            // forall (ConstraintQueue* constrs : seenConstraints) {
            //     if (constrs == solver->constrQueue) {
            //         free all constraints in solver->constrQueue, then the queue itself
            //         solver->constrQueue = constrs;
            //         update solver->constraintID
            //     }
            // }
            
            int numSeenConstraints = solver->seenConstraints->size();
            bool constraintQueueFound = false;
            for (int c = 0; !constraintQueueFound && c < numSeenConstraints; c++) {
                if (constraintQueueEq(solver->constrQueue, (*(solver->seenConstraints))[c])) {
                    // Need to find a way to free this memory without compromising
                    // correctness or efficiency
                    /*int queueSize = solver->constrQueue->size();
                    for (int j = 0; j < queueSize; j++) {
                        //constraintFree((*(solver->constrQueue))[j]);
                        constraintQueuePush(solver->leftOverConstraints, (*(solver->constrQueue))[j]);
                    }*/
                    constraintQueueFree(solver->constrQueue);
                    solver->constrQueue = (*(solver->seenConstraints))[c];
                    solver->constraintID = c;
                    constraintQueueFound = true;
                }
            }
            if (!constraintQueueFound) {
                solver->constraintID = numSeenConstraints;
                solver->seenConstraints->push_back(solver->constrQueue);
            }
            // Done
        }
        
        for (int c = 0; c < size; c++) {
            myLog(LOG_TRACE, "%s: %d ", (*(solver->varQueue))[c]->name, variableGetValue((*(solver->varQueue))[c]));
        }
        myLog(LOG_TRACE, "constraintID: %d", solver->constraintID);
        myLog(LOG_TRACE, "\n\n");
        
        // Prepare variable signature
        myLog(LOG_TRACE, "Creating signature\n");
        vector<int> varSig;
        for (int c = 0; c < size; c++) {
            if ((*(solver->varQueue))[c]->isSignature) {
                varSig.push_back(variableGetValue((*(solver->varQueue))[c]));
            }
        }
        
        Signature *signature = new Signature(varSig, solver->constraintID);
        //delete varSig;
        myLog(LOG_TRACE, "Finish creating signature\n");
        temp = vertexTableGetVertex(solver->graph->vertexTable, *signature);
        myLog(LOG_TRACE, "Finish fetching\n");
        if (temp == NULL) {
            myLog(LOG_TRACE, "Nothing fetched\n");
            // New vertex representing the search node after have a complete edge from the "vertex" argument
            temp = vertexNew(solver->graph, signature, solver->timePoint);
            myLog(LOG_TRACE, "New state created\n");
            vertexTableAddVertex(solver->graph->vertexTable, temp);
            myLog(LOG_TRACE, "Added to table\n");
            
            // variable->prevValue
            for (int c = 0; c < size; c++) {
                Variable *var = (*(solver->varQueue))[c];
                variableAdvanceOneTimeStep(solver, var);
            }
            myLog(LOG_TRACE, "Previous value\n");
            
            if (generalisedArcConsistent(solver)) {
                myLog(LOG_TRACE, "After consistency1\n");
                ok = solverSolveRe(solver, temp);
            } else {
                myLog(LOG_TRACE, "After consistency2\n");
                solver->numFails++;
                ok = false;
            }
        } else {
            myLog(LOG_TRACE, "*** Dominance detected\n\n");
            solver->numDominance++;
            //myFree(signature);
            delete signature;
            ok = true;
        }
        myLog(LOG_TRACE, "After branching\n");
        if (solver->timePoint == 1 && solver->hasFirst) {
            // Undo constraint translations
            // No need to free current constrQueue because seenConstraints needs to keep it
            solver->constrQueue = solverConstraintsBackup;
            // Done
            
            for (int c = 0; c < size; c++) {
                Variable *var = (*(solver->varQueue))[c];
                
                // Restore var's constraints
                constraintQueueFree(var->constraints);
                var->constraints = varConstraints[c];
                // Done
            }
        }
        myLog(LOG_TRACE, "After restoring constraints\n");
        levelDown();
        myLog(LOG_TRACE, "After levelling down\n");
        if (ok) {
            edge = edgeNew(vertex, temp, solver->varQueue, solver->varQueue->size());
            vertexAddEdge(vertex, edge);
        } else {
            vertexTableRemoveVertex(solver->graph->vertexTable, temp);
            myLog(LOG_TRACE, "After removing vertex\n");
            vertexFree(temp);
            myLog(LOG_TRACE, "After freeing vertex\n");
        }
    } else {
        levelUp();
        variableSplitLower(var);
        
        // myLog(LOG_TRACE, "  selected: %s [%d,%d]\n", var->name, var->currLB[0], var->currUB[0]);

        if (generalisedArcConsistent(solver)) {
            ok |= solverSolveRe(solver, vertex);
        } else {
            solver->numFails++;
        }
        levelDown();

        levelUp();
        variableSplitUpper(var);

        // myLog(LOG_TRACE, "  selected: %s [%d,%d]\n", var->name, var->currLB[0], var->currUB[0]);
        
        if (generalisedArcConsistent(solver)) {
            ok |= solverSolveRe(solver, vertex);
        } else {
            solver->numFails++;
        }
        levelDown();
    }

    return ok;
}

// Entry point from solve() in solver.cpp.
double solverSolve(Solver *solver, bool testing) {
    //solverPrint(solver);
    solver->solveTime = cpuTime();
    solver->seenConstraints->push_back(solver->constrQueue);
    levelUp();
    if (generalisedArcConsistent(solver)) {
        solverSolveRe(solver, solver->graph->root);
    } else {
        solver->numFails++;
    }
    levelDown();
    solver->solveTime = cpuTime() - solver->solveTime;

    solver->numNodes = solver->graph->vertexTable->size();
    myLog(LOG_INFO, "Dominance: %d\n", solver->numDominance);
    myLog(LOG_INFO, "Nodes: %d\n", solver->numNodes);
    myLog(LOG_INFO, "Fails: %d\n", solver->numFails);

    if (solver->printSolution) {
        solverOut(solver);
    }

    if (!testing) {
        // init_time, var, con, dom, node, fail, solve_time
        printf("%.2f\t%d\t%d\t%d\t%d\t%d\t%.2f\n", solver->initTime, (int) solver->varQueue->size(), (int) solver->constrQueue->size(), solver->numDominance, solver->numNodes, solver->numFails, solver->solveTime);
        fflush(stdout);
    }
    return solver->solveTime;
}
