#include <cstdio>
#include <cstdlib>
#include <cstring>
#include "util.h"
#include "token.h"
#include "node.h"
#include "constraint.h"
#include "solver.h"
#include "y.tab.h"
#include "variable.h"

ConstraintNode *constraintNodeNew(int token, int num, Variable *var, ConstraintNode *left, ConstraintNode *right) {
    ConstraintNode *node = (ConstraintNode *)myMalloc(sizeof(ConstraintNode));
    node->token = token;
    node->num = num;
    node->var = var;
    node->left = left;
    node->right = right;
    return node;
}

void constraintNodeFree(ConstraintNode *node) {
    if (node->left != NULL) {
        constraintNodeFree(node->left);
    }
    if (node->right != NULL) {
        constraintNodeFree(node->right);
    }
    myFree(node);
}

ConstraintNode *constraintNodeNewVar(Variable *var) {
    return constraintNodeNew(IDENTIFIER, 0, var, NULL, NULL);
}

ConstraintNode *constraintNodeNewConstant(int num) {
    return constraintNodeNew(CONSTANT, num, NULL, NULL, NULL);
}

ConstraintNode *constraintNodeNewFirst(ConstraintNode *node) {
    return constraintNodeNew(FIRST, 0, NULL, NULL, node);
}

ConstraintNode *constraintNodeNewNext(ConstraintNode *node) {
    return constraintNodeNew(NEXT, 0, NULL, NULL, node);
}

// Parses an AST node into a ConstraintNode.
ConstraintNode *constraintNodeParse(Solver *solver, Node *node) {
    ConstraintNode *constrNode = NULL;
    if (node->token == '<' || node->token == '>' ||
        node->token == LE_CON || node->token == GE_CON ||
        node->token == EQ_CON || node->token == NE_CON ||
        node->token == '+' || node->token == '-' || node->token == '*' || node->token == '/' || node->token == '%' ||
        node->token == LT_OP || node->token == GT_OP || node->token == LE_OP || node->token == GE_OP ||
        node->token == EQ_OP || node->token == NE_OP || node->token == AND_OP || node->token == OR_OP ||
        node->token == FBY || node->token == IF || node->token == THEN) {
        constrNode = constraintNodeNew(node->token, 0, NULL,
                            constraintNodeParse(solver, node->left),
                            constraintNodeParse(solver, node->right));
    } else if (node->token == ABS || node->token == FIRST || node->token == NEXT) {
        constrNode = constraintNodeNew(node->token, 0, NULL, NULL, constraintNodeParse(solver, node->right));
    } else if (node->token == CONSTANT) {
        constrNode = constraintNodeNew(node->token, node->num1, NULL, NULL, NULL);
    } else if (node->token == IDENTIFIER) {
        constrNode = constraintNodeNew(node->token, 0,
                                       solverGetVar(solver, node->str),
                                        NULL, NULL);
    } else {
        myLog(LOG_ERROR, "Unknown token: %d\n", node->token);
        exit(1);
    }
    return constrNode;
}

void constraintNodeLogPrint(ConstraintNode *node, Solver *solver) {
    if (node->token == '<' || node->token == '>' ||
        node->token == LE_CON || node->token == GE_CON ||
        node->token == EQ_CON || node->token == NE_CON ||
        node->token == '+' || node->token == '-' || node->token == '*' || node->token == '/' || node->token == '%' ||
        node->token == LT_OP || node->token == GT_OP || node->token == LE_OP || node->token == GE_OP ||
        node->token == EQ_OP || node->token == NE_OP || node->token == AND_OP || node->token == OR_OP) {
        if (node->left->token != CONSTANT && node->left->token != IDENTIFIER &&
            tokenLevel(solver->tokenTable, node->token) > tokenLevel(solver->tokenTable, node->left->token)) {
            myLog(LOG_TRACE, "(");
            constraintNodeLogPrint(node->left, solver);
            myLog(LOG_TRACE, ")");
        } else {
            constraintNodeLogPrint(node->left, solver);
        }
        myLog(LOG_TRACE, " %s ", tokenString(solver->tokenTable, node->token));
        if (node->right->token != CONSTANT && node->right->token != IDENTIFIER &&
            tokenLevel(solver->tokenTable, node->token) > tokenLevel(solver->tokenTable, node->right->token)) {
            myLog(LOG_TRACE, "(");
            constraintNodeLogPrint(node->right, solver);
            myLog(LOG_TRACE, ")");
        } else {
            constraintNodeLogPrint(node->right, solver);
        }
    } else if (node->token == FBY) {
        myLog(LOG_TRACE, "(");
        constraintNodeLogPrint(node->left, solver);
        myLog(LOG_TRACE, ")");
        myLog(LOG_TRACE, " %s ", tokenString(solver->tokenTable, node->token));
        myLog(LOG_TRACE, "(");
        constraintNodeLogPrint(node->right, solver);
        myLog(LOG_TRACE, ")");
    } else if (node->token == IF) {
        myLog(LOG_TRACE, "%s ", tokenString(solver->tokenTable, node->token));
        myLog(LOG_TRACE, "(");
        constraintNodeLogPrint(node->left, solver);
        myLog(LOG_TRACE, ")");
        constraintNodeLogPrint(node->right, solver);
    } else if (node->token == THEN) {
        myLog(LOG_TRACE, " %s ", tokenString(solver->tokenTable, node->token));
        myLog(LOG_TRACE, "(");
        constraintNodeLogPrint(node->left, solver);
        myLog(LOG_TRACE, ")");
        myLog(LOG_TRACE, " else ");
        myLog(LOG_TRACE, "(");
        constraintNodeLogPrint(node->right, solver);
        myLog(LOG_TRACE, ")");
    } else if (node->token == ABS || node->token == FIRST || node->token == NEXT) {
        myLog(LOG_TRACE, "%s(", tokenString(solver->tokenTable, node->token));
        constraintNodeLogPrint(node->right, solver);
        myLog(LOG_TRACE, ")");
    } else if (node->token == CONSTANT) {
        myLog(LOG_TRACE, "%d", node->num);
    } else if (node->token == IDENTIFIER) {
        myLog(LOG_TRACE, "%s", node->var->name);
    }
}

Constraint *constraintNew(Solver *solver, ConstraintNode *node) {
    Constraint *constr = (Constraint *)myMalloc(sizeof(Constraint));
    constr->solver = solver;
    constr->node = node;
    constr->variables = new vector<Variable *>();
    constr->numVar = 0;
    constr->hasFirst = false;
    
    return constr;
}

void constraintFree(Constraint *constr) {
    constraintNodeFree(constr->node);
    variableQueueFree(constr->variables);
    myFree(constr);
}

// Creates new ConstraintQueue
ConstraintQueue *constraintQueueNew() {
    ConstraintQueue *queue = new vector<Constraint *>();
    return queue;
}

// Frees a ConstraintQueue
void constraintQueueFree(ConstraintQueue *queue) {
    /*Constraint *constr;
    constr = queue->head;
    while (constr != NULL) {
        constraintFree(constr);
        constr = constr->next;
    }
    myFree(queue);*/
    delete queue;
}

// Queue push operation for a ConstraintQueue
void constraintQueuePush(ConstraintQueue *queue, Constraint *constr) {
    queue->push_back(constr);
}

// Recursive traversal of the ConstraintNode structure for
// constraintVariableLink.
void constraintVarLinkRe(Constraint *constr, ConstraintNode *node) {
    if (node != NULL) {
        if (node->token == IDENTIFIER) {
            Variable *var = node->var;
            if (!variableQueueFind(constr->variables, var)) {
                variableQueuePush(constr->variables, var);
                constraintQueuePush(var->constraints, constr);
                (var->numConstr)++;
                (constr->numVar)++;
            }
        } else {
            constraintVarLinkRe(constr, node->left);
            constraintVarLinkRe(constr, node->right);
        }
    }
}

// Given a constraint, initialise the constraint's list of variables
// and add this constraint to the variables' list of constraints.
void constraintVariableLink(Constraint *constr) {
    constraintVarLinkRe(constr, constr->node);
}

// Checks whether a ConstraintNode contains a first operator.
// Used to determine if it is necessary to do constraint translation
// during solving.
bool constraintNodeHasFirst(ConstraintNode *constrNode) {
    if (constrNode == NULL) {
        return false;
    } else if (constrNode->token == FIRST) {
        return true;
    } else if (constrNode->token == IDENTIFIER || constrNode->token == CONSTANT) {
        return false;
    } else {
        return (constraintNodeHasFirst(constrNode->left) || constraintNodeHasFirst(constrNode->right));
    }
}

// Pushes a Constraint into the solver's ConstraintQueue. More processing
// than a normal constraintQueuePush because solver contains more information.
void solverConstraintQueuePush(ConstraintQueue *queue, Constraint *constr, Solver *solver, bool checkForFirst) {
    constr->id = queue->size();
    /*if (constr->node->right->token == FIRST) {
        constr->type = CONSTR_FIRST;
        myLog(LOG_TRACE, "FIRST: ");
    } else */
    if (constr->node->right->token == NEXT) {
        if (!constr->node->left->var->isSignature) {
            constr->node->left->var->isSignature = 1;
            solver->numSignVar++;
        }
        constr->type = CONSTR_NEXT;
        myLog(LOG_TRACE, "NEXT : ");
    } else {
        constr->type = CONSTR_POINT;
        myLog(LOG_TRACE, "POINT: ");
    }
    
    if (checkForFirst /*&& (!solver->hasFirst)*/ && constraintNodeHasFirst(constr->node)) {
        solver->hasFirst = true;
        constr->hasFirst = true;
    }
    
    constraintNodeLogPrint(constr->node, solver);
    myLog(LOG_TRACE, ";\n");
    //myLog(LOG_TRACE, "Scope:");
    //constraintAddScopeOmega(constr, constr->node);
    //myLog(LOG_TRACE, "\n");
    
    /* if (queue->size == 0) {
        queue->head = constr;
    } else {
        queue->tail->next = constr;
    }
    queue->tail = constr;
    queue->size++; */
    constraintVariableLink(constr);
    
    // Variable queue reverse for optimising prefix-k support finding
    // for nested if-then-elses
    int size = constr->variables->size();
    for (int c = 0; c < size/2; c++) {
        Variable *temp = (*constr->variables)[c];
        (*constr->variables)[c] = (*constr->variables)[size-c-1];
        (*constr->variables)[size-c-1] = temp;
    }
    
    myLog(LOG_TRACE, "Constraint ID: %d\n", constr->id);
    queue->push_back(constr);
}

void constraintPrint(Constraint *constr) {
    myLog(LOG_TRACE, "Constraint %d\n", constr->id);
    myLog(LOG_TRACE, "Concerns variables: ");
    int size = constr->numVar;
    for (int i = 0; i < size-1; i++) {
        myLog(LOG_TRACE, "%s ", (*(constr->variables))[i]->name);
    }
    if (size > 0) {
        myLog(LOG_TRACE, "%s ", (*(constr->variables))[size-1]->name);
    }
    myLog(LOG_TRACE, "\n\n");
}

// Attempts to compute the value of a ConstraintNode,
// for tautology checks after constraint translations.
LiftedInt constraintNodeValue(ConstraintNode *constrNode) {
    LiftedInt result;
    if (constrNode->token == IDENTIFIER) {
        result.tag = true;
    } else if (constrNode->token == CONSTANT) {
        result.tag = false;
        result.Int = constrNode->num;
    } else if (constrNode->token == FIRST) {
        LiftedInt rightResult = constraintNodeValue(constrNode->right);
        if (rightResult.tag) {
            // Shouldn't happen?
            result.tag = true;
        } else {
            result = rightResult;
        }
    } else if (constrNode->token == NEXT) {
        result.tag = true;
    } else if (constrNode->token == ABS) {
        LiftedInt rightResult = constraintNodeValue(constrNode->right);
        if (rightResult.tag) {
            result.tag = true;
        } else {
            result = rightResult;
            result.Int = abs(result.Int);
        }
    } else if (constrNode->token == IF) {
        LiftedInt leftResult = constraintNodeValue(constrNode->left);
        if (leftResult.tag) {
            result.tag = true;
        } else if (leftResult.Int) {
            result = constraintNodeValue(constrNode->right->left);
        } else {
            result = constraintNodeValue(constrNode->right->right);
        }
    } else if (constrNode->token == AND_OP) {
        LiftedInt leftResult = constraintNodeValue(constrNode->left);
        if (leftResult.tag) {
            result.tag = true;
        } else if (leftResult.Int == 0) {
            result.tag = false;
            result.Int = 0;
        } else {
            LiftedInt rightResult = constraintNodeValue(constrNode->right);
            if (rightResult.tag) {
                result.tag = true;
            } else {
                result = rightResult;
            }
        }
    } else if (constrNode->token == OR_OP) {
        LiftedInt leftResult = constraintNodeValue(constrNode->left);
        if (leftResult.tag) {
            result.tag = true;
        } else if (leftResult.Int != 1) {
            result.tag = false;
            result.Int = 1;
        } else {
            LiftedInt rightResult = constraintNodeValue(constrNode->right);
            if (rightResult.tag) {
                result.tag = true;
            } else {
                result = rightResult;
            }
        }
    } else {
        LiftedInt leftResult = constraintNodeValue(constrNode->left);
        LiftedInt rightResult = constraintNodeValue(constrNode->right);
        
        if (leftResult.tag || rightResult.tag) {
            result.tag = true;
        } else {
            result.tag = false;
            switch (constrNode->token) {
                case LT_OP : result.Int = (leftResult.Int < rightResult.Int); break;
                case GT_OP : result.Int = (leftResult.Int < rightResult.Int); break;
                case LE_OP : result.Int = (leftResult.Int <= rightResult.Int); break;
                case GE_OP : result.Int = (leftResult.Int >= rightResult.Int); break;
                case EQ_OP : result.Int = (leftResult.Int == rightResult.Int); break;
                case NE_OP : result.Int = (leftResult.Int != rightResult.Int); break;
                case '+' : result.Int = leftResult.Int + rightResult.Int; break;
                case '-' : result.Int = leftResult.Int - rightResult.Int; break;
                case '*' : result.Int = leftResult.Int * rightResult.Int; break;
                case '/' : result.Int = leftResult.Int / rightResult.Int; break;
                case '%' : result.Int = leftResult.Int % rightResult.Int; break;
            }
        }
    }
    return result;
}

// Checks for constraints that are tautologies. Only does numerical
// evaluation, i.e. zeroth order arithmetic truth.
bool constraintNodeTautology(ConstraintNode *constrNode) {
    LiftedInt leftResult = constraintNodeValue(constrNode->left);
    LiftedInt rightResult = constraintNodeValue(constrNode->right);
    
    if (leftResult.tag || rightResult.tag) {
        return false;
    } else {
        switch (constrNode->token) {
            case '<' : return leftResult.Int < rightResult.Int; break;
            case '>' : return leftResult.Int > rightResult.Int; break;
            case LE_CON : return leftResult.Int <= rightResult.Int; break;
            case GE_CON : return leftResult.Int >= rightResult.Int; break;
            case EQ_CON : return leftResult.Int == rightResult.Int; break;
            case NE_CON : return leftResult.Int != rightResult.Int; break;
            default : myLog(LOG_TRACE, "Weird constraint in constraintNodeTautology.\n"); return false; break;
        }
    }
}

// Recursive translation of ConstraintNode. Only invoked inside a subtree of a
// first operator. Substitutes variables by the values just taken.
ConstraintNode *constraintNodeTranslateFirst(ConstraintNode *constrNode, Solver *solver) {
    if (constrNode == NULL) {
        return NULL;
    } else {
        ConstraintNode *result;
        if (constrNode->token == IDENTIFIER) {
            int num = variableGetValue(constrNode->var);
            result = constraintNodeNew(CONSTANT, num, NULL,
                                       NULL, NULL);
        } else {
            result = constraintNodeNew(constrNode->token, constrNode->num, NULL,
                                       constraintNodeTranslateFirst(constrNode->left, solver), constraintNodeTranslateFirst(constrNode->right, solver));
        }
        return result;
    }
}

// Recursive translation of ConstraintNode. Switches to constraintNodeTranslateFirst
// when a first operator is encountered.
ConstraintNode *constraintNodeTranslate(ConstraintNode *constrNode, Solver *solver) {
    if (constrNode == NULL) {
        return NULL;
    } else {
        ConstraintNode *result;
        if (constrNode->token == FIRST) {
            result = constraintNodeTranslateFirst(constrNode->right, solver);
            LiftedInt num = constraintNodeValue(result);
            if (num.tag) {
                myLog(LOG_TRACE, "constraintNodeTranslate error: content in \"first\" cannot be completely evaluated.\n");
            } else {
                constraintNodeFree(result);
                result = constraintNodeNew(CONSTANT, num.Int, NULL, NULL, NULL);
            }
        } else {
            result = constraintNodeNew(constrNode->token, constrNode->num, constrNode->var,
                                       constraintNodeTranslate(constrNode->left, solver), constraintNodeTranslate(constrNode->right, solver));
        }
        return result;
    }
}

// Wrapper function for translating constraints.
Constraint *constraintTranslate(Constraint *constr, Solver *solver) {
    ConstraintNode *output = constraintNodeTranslate(constr->node, solver);
    if (constraintNodeTautology(output)) {
        constraintNodeFree(output);
        return NULL;
    } else {
        return constraintNew(solver, output);
    }
}

// Test for structural equality of two ConstraintNodes.
bool constraintNodeEq(ConstraintNode *node1, ConstraintNode *node2) {
    if (node1 == NULL && node2 == NULL) {
        return true;
    } else if (node1 == NULL || node2 == NULL) {
        return false;
    } else if (node1->token == node2->token && node1->num == node2->num && node1->var == node2->var) {
        return constraintNodeEq(node1->left, node2->left) && constraintNodeEq(node1->right, node2->right);
    } else {
        return false;
    }
}

// Test for structural equality of two ConstraintQueues.
bool constraintQueueEq(ConstraintQueue *q1, ConstraintQueue *q2) {
    int size = q1->size();
    int size2 = q2->size();
    if (size != size2) {
        return false;
    } else {
        bool result = true;
        for (int c = 0; result && c < size; c++) {
            result = result && (constraintNodeEq((*q1)[c]->node, (*q2)[c]->node));
        }
        return result;
    }
}
