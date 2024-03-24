#include "gtree-controller.h"

#include "data-structures/g-tree.h"
#include "data-handling/importer.h"
#include "data-structures/creator.h"
#include "constants.h"

#include <iostream>
#include <fstream>

GTree* GTreeController::createOrLoadNetworkGTree(DataType dataType, int numberOfChildrenPerNode, int maxNumberOfVerticesPerLeaf) {
    vector<int> stopIds = vector<int>(0);
    for (int i = 0; i < Creator::networkGraph.vertices.size(); i++) {
        stopIds.push_back(Creator::networkGraph.vertices[i].stopId);
    }

    int numberOfFiles = 2;
    if (dataType == gesamt_de) {
        numberOfFiles = 200;
    } else if (dataType == schienenregionalverkehr_de || dataType == schienenfern_und_regionalverkehr_de) {
        numberOfFiles = 5;
    }
    
    string dataTypeString = Importer::getDataTypeString(dataType);
    string folderPath = FOLDER_PREFIX + "graphs/" + dataTypeString + "/";

    string fileName = folderPath + "g-tree-" + to_string(numberOfChildrenPerNode) + "-" + to_string(maxNumberOfVerticesPerLeaf);
    if (USE_FOOTPATHS) {
        fileName += "-with-footpaths";
    }
    fileName += "-0.json";

    ifstream file(fileName);
    if (file.good()) {
        file.close();
        GTree* gTree = new GTree();
        gTree->importTreeFromJson(dataType, numberOfChildrenPerNode, maxNumberOfVerticesPerLeaf, numberOfFiles);
        return gTree;
    } else {
        file.close();
        GTree* gTree = Creator::createNetworkGTree(numberOfChildrenPerNode, maxNumberOfVerticesPerLeaf, false);
        gTree->calculateBorderDistancesOfStopIds(stopIds);
        gTree->exportTreeAsJson(dataType, numberOfChildrenPerNode, maxNumberOfVerticesPerLeaf, numberOfFiles);
        return gTree;
    }
}

void GTreeController::calculateBorderDistancesOfStopIdsAndExportTree(GTree* gTree, vector<int> stopIds, DataType dataType, int numberOfChildrenPerNode, int maxNumberOfVerticesPerLeaf, int numberOfFiles) {
    gTree->calculateBorderDistancesOfStopIds(stopIds);
    gTree->exportTreeAsJson(dataType, numberOfChildrenPerNode, maxNumberOfVerticesPerLeaf, numberOfFiles);
}