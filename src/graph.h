#ifndef __GRAPH_H
#define __GRAPH_H

#include <cstdio>
#include <ext/hash_map>
#include <ext/slist>
#include <vector>
using namespace __gnu_cxx;
using namespace std;


// A signature is the finite representation of an St-CSP.
// Equivalence of signature implies equivalence of St-CSP.
struct Signature{
    vector<int> sigValues; // Signature values, i.e. values taken for "next" streams.
    int constraintID; // ID for the set of constraints the St-CSP has.
    Signature(vector<int> &a, int b) {
        sigValues = a;
        constraintID = b;
    }
};

struct signatureHash {
    size_t operator()(Signature const& a) const{
        int temp = a.constraintID;
        int size = a.sigValues.size();
        for (int i = 0; i < size; i++) {
            temp += a.sigValues[i];
            temp *= 2;
        }
        hash<int> H;
        return H(temp);
    }
};

struct signatureEq {
    bool operator()(Signature const& a, Signature const& b) {
        bool temp = a.constraintID == b.constraintID;
        return temp && (a.sigValues == b.sigValues);
    }
};

struct Edge;

typedef hash_map<int, slist<Edge *> *> EdgeMap;

struct Vertex {
    int id; // ID for a vertex.
    Signature *signature; // Signature denoting the implicit St-CSP the vertex is. Vertices can be looked up by signature in a VertexTable.s
    bool visited; // Boolean flag for automaton traversal (for outputting automaton).
    bool fail; // Boolean flag for indicating whether the node is a failure.
    bool valid; // Boolean flag for indicating whether the node can reach a final node.
    bool final; // Boolean flag for indicating whether the node is final or not 
    EdgeMap *edges;
    int timePoint;
};

struct Edge {
    Vertex *src;
    Vertex *dst;
    int *values;
};

typedef hash_map<Signature, Vertex *, signatureHash, signatureEq> VertexTable;

struct Graph {
    int nextVertexId;
    VertexTable* vertexTable;
    Vertex *root;
    
    vector<Vertex *> visitedVertices;
};

struct Variable;

Vertex *vertexNew(Graph *g, Signature *signature, int timePoint);
void vertexAddEdge(Vertex *vertex, Edge *edge);
void vertexOut(Vertex *vertex, FILE *fp, int numVar, int numSignVar, int numUntil);
Edge *edgeNew(Vertex *src, Vertex *dst, VariableQueue *varQueue, int numVar);
void edgeOut(Edge *edge, FILE *fp, int numVar, int numSignVar);
void vertexTableAddVertex(VertexTable *table, Vertex *vertex);
void vertexTableRemoveVertex(VertexTable *table, Vertex *vertex);
Vertex *vertexTableGetVertex(VertexTable *table, Signature signature);
Graph *graphNew();
void graphOut(Graph *g, FILE *fp, int numVar, int numSignVar, int numUntil);
void graphFree(Graph *g);
void vertexFree(Vertex *vertex);
void graphTraverse(Graph * graph, int numSignVar, int numUntil);
void adversarialTraverse(Graph * graph, VariableQueue * varQueue);
void adversarialTraverse2(Graph * graph, VariableQueue * varQueue);
void renumberVertex(Graph * graph);

//void graphTraverse(Graph * graph, int numSignVar, int numUntil);
// bool graphTraverseRe(Vertex * vertex, vector<Vertex *> & visitedVertices);

void signatureOut(Signature &signature);

#endif
