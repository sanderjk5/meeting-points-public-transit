#ifndef CMAKE_G_TREE_H
#define CMAKE_G_TREE_H

#include <vector>
#include <map>
#include <string>
#include <../data-handling/importer.h>

using namespace std;

class GNode {
    public:
        explicit GNode() {};
        ~GNode(){};

        int nodeId;
        GNode* parent;
        vector<GNode*> children;
        vector<int> stopIds;

        vector<int> borderStopIds;
        map<pair<int, int>, int> borderDurations;
};

/*
    A G-Tree that represents the public transit network.
*/
class GTree {
    public:
        explicit GTree() {
            this->root = new GNode();
            this->nodeOfStopId = vector<GNode*>(0);
            this->nodeOfNodeId = vector<GNode*>(0);
        };
        ~GTree(){};

        GNode* root;
        vector<GNode*> nodeOfStopId;
        vector<GNode*> nodeOfNodeId;

        int getMinimalDurationToNode(int sourceStopId, int targetNodeId, map<int, vector<pair<int, int>>> &nodeToBorderStopDurations);
        int getMinimalDurationToStop(int sourceStopId, int targetStopId, map<int, vector<pair<int, int>>> &nodeToBorderStopDurations);
        void exportTreeAsJson(DataType dataType, int numberOfChildrenPerNode, int maxNumberOfVerticesPerLeaf, int numberOfFiles);
        void importTreeFromJson(DataType dataType, int numberOfChildrenPerNode, int maxNumberOfVerticesPerLeaf, int numberOfFiles);
        void calculateBorderDistancesOfStopIds(vector<int> stopIds);

    private:
        vector<int> getNodePath(int stopId, int nodeId);
        bool isVertexInNode(int stopId, int nodeId);
};

#endif //CMAKE_G_TREE_H