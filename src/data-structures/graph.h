#ifndef CMAKE_GRAPH_H
#define CMAKE_GRAPH_H

#include <vector>
#include <map>

using namespace std;

struct Vertex {
    int stopId; // the id of the corresponding stop
    int vwgt; // the weight of the vertex
    int nedges; // the size of the adjacency list of the vertex
    int cewgt; // the weight of the edges that have been contracted to create the vertex (in the vertex is a multinode)
    int adjwgt; // the sum of the weights of the edges in the adjacency list of the vertex
};

struct Edge {
    int targetStopId; // the id of the target stop
    int ewgt; // the weight of the edge
};

/*
    A graph that represents the public transit network.
*/
class Graph {
    public:
        explicit Graph() {};
        ~Graph(){};

        vector<Vertex> vertices;
        vector<vector<Edge>> adjacencyList;
        vector<int> partition;

        vector<int> matchings;
        map<int, int> indexToCoarserGraphIndex;

        vector<int> getDistances(int sourceStopId, vector<int> targetStopIds);
};

#endif //CMAKE_GRAPH_H