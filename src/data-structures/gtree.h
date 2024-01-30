#ifndef CMAKE_GTREE_H
#define CMAKE_GTREE_H

class GNode {
    public:
        explicit GNode() {};
        ~GNode(){};

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
};

#endif //CMAKE_GTREE_H