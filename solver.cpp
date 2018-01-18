#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <csignal>
#include <cmath>
#include <unistd.h>
#include <list>
#include <vector>
#include "util.h"
#include "token.h"
#include "node.h"
#include "variable.h"
#include "constraint.h"
#include "solver.h"
#include "graph.h"
#include "solveralgorithm.h"
#include "y.tab.h"
#include <deque>
extern int my_argc;
extern char **my_argv;

// Name |-> Variable.
Variable *solverGetVar(Solver *solver, char *name) {
    Variable *var = NULL;
    int size = solver->varQueue->size();
    bool found = false;
    for (int i = 0; !found && i < size; i++) {
        if (strcmp(name, (*(solver->varQueue))[i]->name) == 0) {
            var = (*(solver->varQueue))[i];
            found = true;
        }
    }
    if (var == NULL) {
        myLog(LOG_ERROR, "Variable '%s' has not been defined.\n", name);
        exit(1);
    }
    return var;
}

// First variable that is not bound yet. Used in branching in search.
Variable *solverGetFirstUnboundVar(Solver *solver) {
    Variable *var = NULL;

    int size = solver->varQueue->size();
    bool found = false;
    for (int i = 0; !found && i < size; i++) {
        if ((*(solver->varQueue))[i]->currLB[0] < (*(solver->varQueue))[i]->currUB[0]) {
            var = (*(solver->varQueue))[i];
            found = true;
        }
    }
    return var;
}

Array *solverGetArray(Solver *solver, char *name){
    Array * array = NULL;
    int size = solver->arrayQueue->size();
    bool found = false;
    for (int i = 0; !found && i < size; i++) {
        if (strcmp(name, (*(solver->arrayQueue))[i]->name) == 0) {
            array = (*(solver->arrayQueue))[i];
            found = true;
        }
    }
    if (array == NULL) {
        myLog(LOG_ERROR, "Variable '%s' has not been defined.\n", name);
        exit(1);
    }
    return array;
}


Solver *solverNew(int k, int l, int prefixK, char *varOrder, int printSolution, bool adversarial1, bool adversarial2) {
    Solver *solver = (Solver *)myMalloc(sizeof(Solver));
    solver->tokenTable = tokenTableNew();
    solver->varQueue = variableQueueNew();
    solver->arrayQueue = arrayQueueNew();
    solver->constrQueue = constraintQueueNew();
    solver->numAuxVar = 0;
    solver->numSignVar = 0;
    solver->numUntil = 0;
    solver->numNodes = 0;
    solver->numFails = 0;
    solver->numSolutions = 0;
    solver->numDominance = 0;
    solver->propagateTimestamp = 0;
    solver->initTime = 0;
    solver->solveTime = 0;
    solver->processTime = 0;
    solver->timePoint = 0;
    solver->graph = graphNew();
    solver->printSolution = printSolution;

    solver->prefixK = prefixK;
    /*if (strcmp(varOrder, "lex_asc") == 0) {
        solver->getNextVar = solverOmegaGetFirstUnboundVar;
    } else {
        myLog(LOG_ERROR, "No such variable ordering: %s\n", varOrder);
        exit(1);
    }*/
    solver->constraintID = 0;
    solver->seenConstraints = new vector<ConstraintQueue *>();
    solver->hasFirst = false;
    solver->adversarial1 = adversarial1;
	solver->adversarial2 = adversarial2;
    solver->arcQueue = new deque<Arc *>();
    return solver;
}

Variable *solverAddVar(Solver *solver, char *var_name, int lb, int ub) {
    Variable *var = NULL;

    var = variableNew(solver, var_name, lb, ub);
    variableQueuePush(solver->varQueue, var);

    return var;
}

Variable *solverAuxVarNew(Solver *solver, char *var_name, int lb, int ub) {
    char name[16];
    sprintf(var_name == NULL ? name : var_name, "_V%d", solver->numAuxVar);
    solver->numAuxVar++;
    return solverAddVar(solver, var_name == NULL ? name : var_name, lb, ub);
}

Array *solverAddArr(Solver *solver, char *arr_name, vector<int> array){
    Array * arr = NULL;
    arr = arrayNew(solver, arr_name, array);
    arrayQueuePush(solver->arrayQueue, arr);
    return arr;
}

void solverAddConstrNode(Solver *solver, ConstraintNode *node) {
    Constraint *constr = constraintNew(solver, node);
    solverConstraintQueuePush(solver->constrQueue, constr, solver, true);
}

void solverParse(Solver *solver, Node *node) {
    if (node != NULL) {
        if (node->token == STATEMENT) {
            solverParse(solver, node->left);
            solverParse(solver, node->right);
        } else if (node->token == VAR) {
            solverAddVar(solver, node->str, node->right->num1, node->right->num2);
        } else if (node->token == ARR) {
            list<int> temp;
            Node * array_node = node->right;
            while(array_node != NULL){
                temp.push_front(array_node->num1);
                array_node = array_node->left;
            }
            vector<int> elements(temp.begin(), temp.end());
            solverAddArr(solver, node->str, elements);
        } else { /* assume to be constraint */
            solverAddConstr(solver, node);
            //solverAddConstrNode(solver, constraintNodeParse(solver, node));
        }
    }
}

void solverFree(Solver *solver) {
    int size = solver->varQueue->size();
    for (int i = 0; i < size; i++) {
        variableFree((*(solver->varQueue))[i]);
    }
    //size = solver->constrQueue->size();
    //for (int i = 0; i < size; i++) {
    //    constraintFree((*(solver->constrQueue))[i]);
    //}
    tokenTableFree(solver->tokenTable);
    variableQueueFree(solver->varQueue);
    //constraintQueueFree(solver->constrQueue);
    if (solver->graph != NULL) {
        graphFree(solver->graph);
    }
    // Freeing seenConstraints also frees the current constrQueue
    size = solver->seenConstraints->size();
    for (int i = 0; i < size; i++) {
        int queueSize = (*(solver->seenConstraints))[i]->size();
        for (int j = 0; j < queueSize; j++) {
            constraintFree((*((*(solver->seenConstraints))[i]))[j]);
        }
        constraintQueueFree((*(solver->seenConstraints))[i]);
    }
    delete solver->seenConstraints;
    delete solver->arcQueue;
    myFree(solver);
}

void solverTimeout(int sig) {
    signal(SIGALRM, SIG_IGN);
    exit(0);
}

void solve(Node *node) {
    Solver *solver;
    char c;
    int begin_k = 1;
    int begin_l = 0;
    int end_k = -1;
    int end_l = -1;
    int printSolution = 0;
    char varOrder[64] = "lex_asc";
    int prefixK = 2;
    double initTimeStart = 0;
    int timeLimit = 0;
    bool testing = false;
    bool adversarial1 = false;
	bool adversarial2 = false;
	
    c = getopt(my_argc, my_argv, "b:e:cv:l:stk:m:az");
    while (c != -1) {
        if (c == 'b') {
            if (sscanf(optarg, "%d,%d", &begin_k, &begin_l) != 2) {
                myLog(LOG_ERROR, "Invalid argument: %s\n", optarg);
                exit(1);
            }
        } else if (c == 'e') {
            if (sscanf(optarg, "%d,%d", &end_k, &end_l) != 2) {
                myLog(LOG_ERROR, "Invalid argument: %s\n", optarg);
                exit(1);
            }
        } else if (c == 'l') {
            if (sscanf(optarg, "%d", &logLevel) != 1) {
                myLog(LOG_ERROR, "Invalid argument: %s\n", optarg);
                exit(1);
            }
        } else if (c == 'k') {
            if (sscanf(optarg, "%d", &prefixK) != 1) {
                myLog(LOG_ERROR, "Invalid argument: %s\n", optarg);
                exit(1);
            }
        } else if (c == 'm') {
            if (sscanf(optarg, "%d", &timeLimit) != 1) {
                myLog(LOG_ERROR, "Invalid argument: %s\n", optarg);
                exit(1);
            }
        } else if (c == 's') {
            printSolution = 1;
        } else if (c == 'v') {
            strcpy(varOrder, optarg);
        } else if (c == 't') {
            testing = true;
        } else if (c == 'a') {
            adversarial1 = true;
        }
		  else if (c == 'z'){
			adversarial2 = true;
		}
		else {
            myLog(LOG_ERROR, "Unknown argument: %c\n", c);
            exit(1);
        }
        c = getopt(my_argc, my_argv, "b:e:cv:l:st:k:m:az");
    }

#ifdef DEBUG
    nodeDraw(node, "ast.dot");
#endif

    myLog(LOG_CONFIG, "Solver: %s, From [ %d, %d ] to [ %d, %d ] ; ", "Omega", begin_k, begin_l, end_k, end_l);
    myLog(LOG_CONFIG, "Print solutions? %s ; ", printSolution ? "Yes" : "No");
    myLog(LOG_CONFIG, "Log Level: %d ; ", logLevel);
    myLog(LOG_CONFIG, "Time Limit: %d\n", timeLimit);

    if (timeLimit > 0) {
        signal(SIGALRM, solverTimeout);
        alarm(timeLimit);
    }

    solver = solverNew(0, 0, prefixK, varOrder, printSolution, adversarial1,adversarial2);
    initTimeStart = cpuTime();
    if (node != NULL) {
        // parse the statement list
        solverParse(solver, node);
        solver->initTime = cpuTime() - initTimeStart;
        //for debug: print out the variable
        int size = solver->varQueue->size();
        for (int i = 0; i < size; i++) {
            variablePrint((*(solver->varQueue))[i]);
        }
        //for debug: print out varaibles of constraint
        size = solver->constrQueue->size();
        myLog(LOG_TRACE, "ConstraintQueue size: %d\n\n", size);
        for (int i = 0; i < size; i++) {
            constraintPrint((*(solver->constrQueue))[i]);
        }

        solverSolve(solver, testing); // Actual solving done.
    } else {
        printf("No constraints!\n");
    }
    solverFree(solver);

    if (testing) {
        vector<double> times;
        int numTimes = 0;
        bool converge = false;

        while (!converge) {
            printf("%d ", numTimes);
            fflush(stdout);
            if (timeLimit > 0) {
                signal(SIGALRM, solverTimeout);
                alarm(timeLimit);
            }
            solver = solverNew(0, 0, prefixK, varOrder, printSolution, adversarial1, adversarial2);
            initTimeStart = cpuTime();
            if (node != NULL) {
                solverParse(solver, node); //parsing the statement
                solver->initTime = cpuTime() - initTimeStart;
                //print out variable
                int size = solver->varQueue->size();
                for (int i = 0; i < size; i++) {
                    variablePrint((*(solver->varQueue))[i]);
                }
                //print constraint variable names
                size = solver->constrQueue->size();
                myLog(LOG_TRACE, "ConstraintQueue size: %d\n\n", size);
                for (int i = 0; i < size; i++) {
                    constraintPrint((*(solver->constrQueue))[i]);
                }
                times.push_back(solverSolve(solver, false));//testing)); // Actual solving done.
            } else {
                printf("No constraints!\n");
            }
            solverFree(solver);

            numTimes++;

            if (numTimes >= 10) {
                double mean = 0;
                for (int c = 0; c < numTimes; c++) {
                    mean += times[c];
                }
                mean = mean/((double) numTimes);

                double sampleVar = 0;
                for (int c = 0; c < numTimes; c++) {
                    sampleVar += (times[c] - mean) * (times[c] - mean);
                }
                sampleVar = sampleVar/((double) numTimes - 1);
                if (2 * 1.96 * sqrt(sampleVar) / sqrt(numTimes) < 0.05 * mean) {
                    converge = true;
                    printf("\nMean execution time is %f pm %f\n", mean, 1.96 * sqrt(sampleVar) / sqrt(numTimes));
                }
            }
        }
    }

    if (node != NULL) {
        nodeFree(node);
    }

    freeMemory();
#if MEMORY
    myLog(LOG_DEBUG, "Malloc = %d, Free = %d, Diff = %d\n", mallocCount, freeCount, mallocCount - freeCount);
#endif
}

Arc *arcNew(Constraint *constr, Variable *var) {
    Arc *arc = (Arc *)myMalloc(sizeof(Arc));
    arc->constr = constr;
    arc->var = var;
    arc->inqueue = false;
    return arc;
}

void arcQueueFree(ArcQueue * arcs){
    while(!arcs->empty()){
        Arc * arc = arcs->front();
        arcs->pop_front();
        myFree(arc);
    }
    delete arcs;
}

bool arcQueueFind(ArcQueue *queue, Arc *arc) {
    bool found = false;
    int size = queue->size();
    for (int c = 0; !found && c < size; c++) {
        Arc *temp = (*queue)[c];
        if (temp->constr == arc->constr && temp->var == arc->var) {
            found = true;
        }
    }
    return found;
}
