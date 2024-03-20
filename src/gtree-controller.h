#ifndef CMAKE_GTREE_CONTROLLER_H
#define CMAKE_GTREE_CONTROLLER_H


#include "data-handling/importer.h"
#include "data-structures/g-tree.h"

class GTreeController {
    public:
        explicit GTreeController(){};
        ~GTreeController(){};

        static GTree* createOrLoadNetworkGTree(DataType dataType, int numberOfChildrenPerNode, int maxNumberOfVerticesPerLeaf); 
        static void calculateBorderDistancesOfStopIdsAndExportTree(GTree* gTree, vector<int> stopIds, DataType dataType, int numberOfChildrenPerNode, int maxNumberOfVerticesPerLeaf);
};

#endif // CMAKE_GTREE_CONTROLLER_H