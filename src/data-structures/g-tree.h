#ifndef CMAKE_G_TREE_H
#define CMAKE_G_TREE_H

#include <vector>
#include <map>

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

class GTree {
    public:
        explicit GTree() {};
        ~GTree(){};

        GNode* root;
        vector<GNode*> nodeOfStopId;
        vector<GNode*> nodeOfNodeId;

        int getMinimalDurationToNode(int sourceStopId, int targetNodeId, map<pair<int, int>, vector<pair<int, int>>> &queryPointAndNodeToBorderStopDurations);
        int getMinimalDurationToStop(int sourceStopId, int targetStopId, map<pair<int, int>, vector<pair<int, int>>> &queryPointAndNodeToBorderStopDurations);

    private:
        vector<int> getNodePath(int stopId, int nodeId);
};

#endif //CMAKE_G_TREE_H