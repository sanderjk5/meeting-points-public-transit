#include "gtree-controller.h"

#include "data-structures/g-tree.h"
#include "data-handling/importer.h"
#include "data-structures/creator.h"
#include "constants.h"

#include <iostream>
#include <fstream>

GTree* GTreeController::createOrLoadNetworkGTree(DataType dataType, int numberOfChildrenPerNode, int maxNumberOfVerticesPerLeaf, vector<int> stopIds) {
    string dataTypeString = Importer::getDataTypeString(dataType);
    string folderPath = FOLDER_PREFIX + "graphs/" + dataTypeString + "/";

    string fileName = folderPath + "g-tree-" + to_string(numberOfChildrenPerNode) + "-" + to_string(maxNumberOfVerticesPerLeaf);
    if (USE_FOOTPATHS) {
        fileName += "-with-footpaths";
    }
    fileName += ".json";

    ifstream file(fileName);
    if (file.good()) {
        file.close();
        GTree* gTree = new GTree();
        gTree->importTreeFromJson(dataType, numberOfChildrenPerNode, maxNumberOfVerticesPerLeaf);
        return gTree;
    } else {
        file.close();
        GTree* gTree = Creator::createNetworkGTree(numberOfChildrenPerNode, maxNumberOfVerticesPerLeaf, false);
        gTree->calculateBorderDistancesOfStopIds(stopIds);
        gTree->exportTreeAsJson(dataType, numberOfChildrenPerNode, maxNumberOfVerticesPerLeaf);
        return gTree;
    }
}

void GTreeController::calculateBorderDistancesOfStopIdsAndExportTree(GTree* gTree, vector<int> stopIds, DataType dataType, int numberOfChildrenPerNode, int maxNumberOfVerticesPerLeaf) {
    gTree->calculateBorderDistancesOfStopIds(stopIds);
    gTree->exportTreeAsJson(dataType, numberOfChildrenPerNode, maxNumberOfVerticesPerLeaf);
}