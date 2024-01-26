#ifndef CMAKE_G_TREE_H
#define CMAKE_GTREE_H

class GNode {
    public:
        explicit GNode() {};
        ~GNode(){};

        vector<GNode> children;
        vector<int> stopIds;

        map<int, int> stopIdToIndex;
        vector<int> borderDurations;
};

class GTree {
    public:
        explicit GTree() {};
        ~GTree(){};

        GNode root;
};

#endif //CMAKE_G_TREE_H