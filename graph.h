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
void vertexOut(Vertex *vertex, FILE *fp, int numVar, int numSignVar);
Edge *edgeNew(Vertex *src, Vertex *dst, VariableQueue *varQueue, int numVar);
void edgeOut(Edge *edge, FILE *fp, int numVar, int numSignVar);
void vertexTableAddVertex(VertexTable *table, Vertex *vertex);
void vertexTableRemoveVertex(VertexTable *table, Vertex *vertex);
Vertex *vertexTableGetVertex(VertexTable *table, Signature signature);
Graph *graphNew();
void graphOut(Graph *g, FILE *fp, int numVar, int numSignVar);
void graphFree(Graph *g);
void vertexFree(Vertex *vertex);

void signatureOut(Signature &signature);

#endif
