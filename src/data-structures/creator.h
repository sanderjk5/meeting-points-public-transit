#ifndef CMAKE_CREATOR_H
#define CMAKE_CREATOR_H

#include <graph.h>

#include <vector>

class Creator {
    public:
        explicit Creator() {};
        ~Creator(){};

        static Graph networkGraph;
        static GTree networkGTree;
        static void createNetworkGraph();
        static void createNetworkGTree(int numberOfChildrenPerNode, int numberOfVerticesPerLeaf);

    private:
        static vector<Graph> partitionateGraph(Graph &graph, int numberOfPartitions);
        static void coarseGraph(Graph &graph);
        static vector<Graph> partitionateCoarsedGraph(Graph &graph);
        static void refineGraphs(vector<Graph> &graphs);
        static GTree createGTree(vector<Graph> graphs, int depth);
        static void setBorderDurations(GTree &gTree);
};

#endif //CMAKE_CREATOR_H