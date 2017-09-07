#include <cstdio>
#include <cstdlib>
#include <cstring>
#include "util.h"
#include "variable.h"
#include "graph.h"
#include <ext/hash_map>
#include <ext/slist>
#include <vector>
using namespace __gnu_cxx;
using namespace std;

Vertex *vertexNew(Graph *g, Signature *signature, int timePoint) {
    Vertex *v = (Vertex *)myMalloc(sizeof(Vertex));
    if (v == NULL) {
        myLog(LOG_TRACE, "Failed to allocate Vertex");
    }
    v->id = g->nextVertexId;
    v->timePoint = timePoint;
    v->signature = signature;
    v->visited = false;
    v->edges = new EdgeMap();
    g->nextVertexId++;
    return v;
}

// Edges are indexed by destination, hence the relatively complex
// implementation.
void vertexAddEdge(Vertex *vertex, Edge *edge) {
    if (vertex->edges->find(edge->dst->id) == vertex->edges->end()) {
        (*(vertex->edges))[edge->dst->id] = new slist<Edge *>();
    }
    (*(vertex->edges))[edge->dst->id]->push_front(edge);
}

// DFS recursion for outputting the graph in DOT format.
void vertexOut(Vertex *vertex, Graph *g, FILE *fp, int numVar, int numSignVar) {
    if (!(vertex->visited)) {
        vertex->visited = true;
        g->visitedVertices.push_back(vertex);
        
        fprintf(fp, "%d [shape=circle, label=\"", vertex->id);
        fprintf(fp, "%d: ", vertex->signature->constraintID);
        /*
	if (vertex != g->root) {
            for (int i = 0; i < numSignVar; i++) {
                fprintf(fp, "%d", vertex->signature->sigValues[i]);
                if (i != numSignVar - 1) {
                    fprintf(fp, ", ");
                }
            }
        } else {
            fprintf(fp, "S");
        }
	*/
        fprintf(fp, "\"];\n");
        
        for (hash_map<int, slist<Edge *> *>::iterator it = vertex->edges->begin(); it != vertex->edges->end(); it++) {
            slist<Edge *>::iterator destIt = it->second->begin();
            if (destIt != it->second->end()) {
                for (slist<Edge *>::iterator edgeIt = it->second->begin(); edgeIt != it->second->end(); edgeIt++) {
                    edgeOut(*edgeIt, fp, numVar, numSignVar);
                }
                vertexOut((*destIt)->dst, g, fp, numVar, numSignVar);
            }
        }
    }
}

Edge *edgeNew(Vertex *src, Vertex *dst, VariableQueue *varQueue, int numVar) {
    Edge *e = (Edge *)myMalloc(sizeof(Edge));
    e->src = src;
    e->dst = dst;
    e->values = (int *)myMalloc(sizeof(int) * numVar);
    
    int size = varQueue->size();
    for (int i = 0; i < size; i++) {
        e->values[i] = variableGetValue((*varQueue)[i]);
    }
    return e;
}

// Prints the edge in DOT format.
void edgeOut(Edge *edge, FILE *fp, int numVar, int numSignVar) {
    fprintf(fp, "%d -> %d [label=\"", edge->src->id, edge->dst->id);
    for (int i = 0; i < numVar; i++) {
        fprintf(fp, "%d", edge->values[i]);
        if (i != numVar - 1) {
            fprintf(fp, ", ");
        }
    }
    fprintf(fp, "\"];\n");
}

void edgeFree(Edge *edge) {
    myFree(edge->values);
    myFree(edge);
}

void vertexTableAddVertex(VertexTable *table, Vertex *vertex) {
    (*table)[*(vertex->signature)] = vertex;
}

void vertexTableRemoveVertex(VertexTable *table, Vertex *vertex) {
    Signature sig(vertex->signature->sigValues, vertex->signature->constraintID);
    table->erase(sig);
}

Vertex *vertexTableGetVertex(VertexTable *table, Signature signature) {
    Vertex *vertex = NULL;
    if (table->find(signature) != table->end()) {
        vertex = (*table)[signature];
    }
    return vertex;
}

void vertexFree(Vertex *vertex) {
    for (hash_map<int, slist<Edge *> *>::iterator it = vertex->edges->begin(); it != vertex->edges->end(); it++) {
        for (slist<Edge *>::iterator edgeIt = it->second->begin(); edgeIt != it->second->end(); edgeIt++) {
            edgeFree(*edgeIt);
        }
        delete (it->second);
    }
    delete vertex->edges;
    delete vertex->signature;
    myFree(vertex);
}

Graph *graphNew() {
    Graph *g = (Graph *)myMalloc(sizeof(Graph));
    g->nextVertexId = 0;
    g->vertexTable = new VertexTable();
    vector<int> temp;
    Signature *signature = new Signature(temp, 0);
    g->root = vertexNew(g, signature, 0);
    vertexTableAddVertex(g->vertexTable, g->root);
    return g;
}

// Wrapper for outputting an automaton in DOT format.
void graphOut(Graph *g, FILE *fp, int numVar, int numSignVar) {
    vertexOut(g->root, g, fp, numVar, numSignVar);

    int size = g->visitedVertices.size();
    for (int c = 0; c < size; c++) {
        ((g->visitedVertices)[c])->visited = false;
    }
    (g->visitedVertices).clear();
}

void graphFree(Graph *g) {
    for (hash_map<Signature, Vertex *, signatureHash, signatureEq>::iterator it = g->vertexTable->begin(); it != g->vertexTable->end(); it++) {
        vertexFree(it->second);
    }
    delete g->vertexTable;
    myFree(g);
}

void signatureOut(Signature &signature) {
    int size = signature.sigValues.size();
    myLog(LOG_TRACE, "%d: ", signature.constraintID);
    for (int c = 0; c < size-1; c++) {
        myLog(LOG_TRACE, "%d, ", signature.sigValues[c]);
    }
    myLog(LOG_TRACE, "%d\n", signature.sigValues[size-1]);
}
