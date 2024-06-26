#ifndef CMAKE_CREATOR_H
#define CMAKE_CREATOR_H

#include "graph.h"
#include "g-tree.h"

#include <vector>


class Creator {
    public:
        explicit Creator() {};
        ~Creator(){};

        static Graph networkGraph;
        static void loadOrCreateNetworkGraph(DataType dataType);
        
        static GTree* createNetworkGTree(DataType dataType, int numberOfChildrenPerNode, int maxNumberOfVerticesPerLeaf, bool withDistances = true);

    private:
        static void createNetworkGraph();
        static vector<Graph> partitionateGraph(Graph graph, int numberOfPartitions, int maxNumberOfVerticesInGraph);
        static vector<Graph> coarseGraph(Graph &graph, int maxNumberOfVerticesInGraph);
        static void partitionateCoarsedGraph(Graph &graph, int klIterations);
        static void refineGraphs(vector<Graph> &graphs);
        static vector<Graph> splitPartitionatedGraph(Graph &graph);
        static GTree* createGTree(Graph &originalGraph, vector<Graph> &graphs, int numberOfChildrenPerNode, int depth, bool withDistances = true);
};

#endif //CMAKE_CREATOR_H