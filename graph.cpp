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
    v->valid = false;
    v->final = false;
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
void vertexOut(Vertex *vertex, Graph *g, FILE *fp, int numVar, int numSignVar, int numUntil) {
    if (!(vertex->visited)) {
        vertex->visited = true;
        g->visitedVertices.push_back(vertex);
        
        // determine whether it is a final state
        if (vertex->final)
            fprintf(fp, "%d [shape=doublecircle, ", vertex->id);
        else
            fprintf(fp, "%d [shape=circle, ", vertex->id);

        fprintf(fp, "label=\"", vertex->id);
        fprintf(fp, "%d: ", vertex->signature->constraintID);
        if (vertex != g->root) {
            for (int i = 0; i < numSignVar + numUntil; i++) {
                fprintf(fp, "%d", vertex->signature->sigValues[i]);
                if (i != numSignVar + numUntil - 1) {
                    fprintf(fp, ", ");
                }
            }
        } else {
            fprintf(fp, "S");
        }
        fprintf(fp, "\"];\n");
        
        for (hash_map<int, slist<Edge *> *>::iterator it = vertex->edges->begin(); it != vertex->edges->end(); it++) {
            slist<Edge *>::iterator destIt = it->second->begin();
            if (destIt != it->second->end()) {
                for (slist<Edge *>::iterator edgeIt = it->second->begin(); edgeIt != it->second->end(); edgeIt++) {
                    edgeOut(*edgeIt, fp, numVar, numSignVar);
                }
                vertexOut((*destIt)->dst, g, fp, numVar, numSignVar, numUntil);
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
void graphOut(Graph *g, FILE *fp, int numVar, int numSignVar, int numUntil) {
    vertexOut(g->root, g, fp, numVar, numSignVar, numUntil);

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

// bool graphTraverseRe(Vertex * vertex, vector<Vertex *> & visitedVertices) {
//     if(vertex->visited == false){
//         vertex->visited = true;
//         visitedVertices.push_back(vertex);
//         vector<int> deleteedge;

//         for (hash_map<int, slist<Edge *> *>::iterator it = vertex->edges->begin(); it != vertex->edges->end(); it++) {
//             slist<Edge *>::iterator destIt = it->second->begin();
//             if (destIt != it->second->end()) {
//                 bool destValid = graphTraverseRe((*destIt)->dst, visitedVertices);
//                 if(!destValid){
//                     deleteedge.push_back(it->first);
//                 }
//                 vertex->valid |= destValid;
//             }
//         }

//         // if edge is not a 
//         for(vector<int>::iterator it = deleteedge.begin(); it != deleteedge.end(); it++){
//             if(*it != vertex->id)
//                 vertex->edges->erase(*it);
//         }
            
//     }
//     return vertex->valid;
// }

void graphTraverse(Graph * graph, int numSignVar, int numUntil) {
    hash_map<int, slist<Vertex *> *> Parent_map;
    slist<Vertex *> nodequeue;
    for(hash_map<Signature, Vertex *, signatureHash, signatureEq>::iterator it = (*graph->vertexTable).begin(); it != (*graph->vertexTable).end(); it ++){
        Signature signature = it->first;
        Vertex * vertex = it->second;
        bool final = true;
        if (vertex != graph->root){
            for(int c = numSignVar; final && c < numSignVar + numUntil; c++)
                final = (vertex->signature->sigValues[c] == 1);
            vertex->final = final;
            vertex->valid = final;
            if(final)
                nodequeue.push_front(vertex);
        }
        else{
            vertex->valid = false;
        }

        for(hash_map<int, slist<Edge *> *>::iterator hash_it = vertex->edges->begin(); hash_it != vertex->edges->end(); hash_it++) {
            int child_no = hash_it->first;
            if(Parent_map.find(child_no) == Parent_map.end()) {
                Parent_map[child_no] = new slist<Vertex *>();
            }
            if(child_no != vertex->id){
                Parent_map[child_no]->push_front(vertex);
            }
        }
    }

    while(!nodequeue.empty()){
        Vertex * current_vertex = nodequeue.front();
        nodequeue.pop_front();
        if(Parent_map.find(current_vertex->id) != Parent_map.end()){
            for(slist<Vertex *>::iterator sit = Parent_map[current_vertex->id]->begin(); sit != Parent_map[current_vertex->id]->end(); sit++ ){
                if(!(*sit)->valid){
                    (*sit)->valid = true;
                    nodequeue.push_front(*sit);
                }
            }
        }
    }

    // while(change) {
    //     change = false;
    //     for(hash_map<Signature, Vertex *, signatureHash, signatureE>::iterator it = (*graph->vertexTable).begin(); it != (*graph->vertexTable).end(); it ++){
    //         Vertex * vertex = it->second;
    //         if(vertex->valid)
    //             continue;
    //         else {
    //             for(hash_map<int, slist<Edge *> *>::iterator hash_it = vertex->edges->begin(); hash_it != vertex->edges->end(); hash_it++) {
    //                 slist<Edge *> * edge_list = hash_it->second;
    //                 if( edge_list->front()->dst->valid ){
    //                     vertex->valid = true;
    //                     change = true;
    //                     break;
    //                 }
    //             }
    //         }
    //     }
    // }

    // delete all edge point to an invalid node
    for(hash_map<Signature, Vertex *, signatureHash, signatureEq>::iterator it = (*graph->vertexTable).begin(); it != (*graph->vertexTable).end(); it ++){
        Vertex * vertex = it->second;
        if(vertex->valid || vertex == graph->root){
            vector<int> deleteEdge;
            for (hash_map<int, slist<Edge *> *>::iterator hash_it = vertex->edges->begin(); hash_it != vertex->edges->end(); hash_it++) {
                slist<Edge *> * edge_list = hash_it->second;
                if( !edge_list->front()->dst->valid ){
                    deleteEdge.push_back(hash_it->first);
                }
            }

            for(vector<int>::iterator vec_it = deleteEdge.begin(); vec_it != deleteEdge.end(); vec_it++ )
                vertex->edges->erase(*vec_it);
        } 
    }

    // release Parentmap 
    // renumber the vertex in the vertex table 
    nodequeue.push_front(graph->root);
    int vertex_no = 0;
    while(!nodequeue.empty()){
        Vertex * vertex = nodequeue.front();
        nodequeue.pop_front();
        if (!(vertex->visited)) {
            vertex->visited = true;
            vertex->id = vertex_no;
            vertex_no++;
            for (hash_map<int, slist<Edge *> *>::iterator hash_it = vertex->edges->begin(); hash_it != vertex->edges->end(); hash_it++) {
                slist<Edge *> * edge_list = hash_it->second;
                nodequeue.push_front(edge_list->front()->dst);
            }
        }
    }

    for(hash_map<Signature, Vertex *, signatureHash, signatureEq>::iterator it = (*graph->vertexTable).begin(); it != (*graph->vertexTable).end(); it ++){
        Vertex * vertex = it->second;
        vertex->visited = false;
    }
}


void signatureOut(Signature &signature) {
    int size = signature.sigValues.size();
    myLog(LOG_TRACE, "%d: ", signature.constraintID);
    for (int c = 0; c < size-1; c++) {
        myLog(LOG_TRACE, "%d, ", signature.sigValues[c]);
    }
    myLog(LOG_TRACE, "%d\n", signature.sigValues[size-1]);
}
