#include "g-tree.h"

#include <../constants.h>
#include <../data-handling/importer.h>
#include <../data-structures/creator.h>
#include <../dist/json/json.h>
#include <iostream>
#include <vector>
#include <map>
#include <algorithm>
#include <limits.h>
#include <fstream>
#include <chrono>
#include <omp.h>

using namespace std;

/*
    Calculate the minimal duration to a target node from a source stop. Fill the map with the border stop durations of the nodes on the path if they are not already filled.
*/
int GTree::getMinimalDurationToNode(int sourceStopId, int targetNodeId, map<int, vector<pair<int, int>>> &nodeToBorderStopDurations) {
    int minDuration = INT_MAX;
    
    // get the path from the source stop to the target node in the G-tree
    vector<int> path = getNodePath(sourceStopId, targetNodeId);

    // default case: the source stop is in the target node
    if(isVertexInNode(sourceStopId, targetNodeId)) {
        return 0;
    }

    // fill the map with the border stop durations of the first node if it is not already filled
    int sourceNodeId = path[0];
    if (nodeToBorderStopDurations.find(sourceNodeId) == nodeToBorderStopDurations.end()) {
        vector<pair<int, int>> distancesToBorderStops = vector<pair<int, int>>(0);
        for (int i = 0; i < nodeOfNodeId[path[0]]->borderStopIds.size(); i++) {
            int borderStopId = nodeOfNodeId[path[0]]->borderStopIds[i];
            int distanceToBorderStop = nodeOfStopId[sourceStopId]->borderDurations[make_pair(sourceStopId, borderStopId)];
            distancesToBorderStops.push_back(make_pair(distanceToBorderStop, borderStopId));
        }
        nodeToBorderStopDurations[sourceNodeId] = distancesToBorderStops;
    }

    // fill the map with the border stop durations of the other nodes on the path if they are not already filled
    int previousNodeId = path[0];
    for(int i = 1; i < path.size(); i++) {
        int currentNodeId = path[i];
        if (nodeToBorderStopDurations.find(currentNodeId) == nodeToBorderStopDurations.end()) {
            vector<pair<int, int>> distancesToBorderStopsOfPreviousNode = nodeToBorderStopDurations[previousNodeId];
            vector<pair<int, int>> distancesToBorderStopsOfCurrentNode = vector<pair<int, int>>(0);
            int parentNodeId;
            // case 1: the node is a parent of the previous node
            if (nodeOfNodeId[previousNodeId]->parent->nodeId == currentNodeId) {
                parentNodeId = currentNodeId;
            }
            // case 2: the node is a sibling of the previous node
            else if (nodeOfNodeId[previousNodeId]->parent->nodeId == nodeOfNodeId[currentNodeId]->parent->nodeId) {
                parentNodeId = nodeOfNodeId[currentNodeId]->parent->nodeId;
            }
            // case 3: the node is a child of the previous node
            else {
                parentNodeId = previousNodeId;
            }

            // calculate the duration by adding the border stop durations of the previous node to the border stop durations of the current node
            for (int j = 0; j < nodeOfNodeId[currentNodeId]->borderStopIds.size(); j++) {
                int borderStopId = nodeOfNodeId[currentNodeId]->borderStopIds[j];
                int minDistanceToCurrentBorderStop = INT_MAX;
                for (int k = 0; k < distancesToBorderStopsOfPreviousNode.size(); k++) {
                    int distanceToPreviousBorderStop = distancesToBorderStopsOfPreviousNode[k].first;
                    int previousBorderStopId = distancesToBorderStopsOfPreviousNode[k].second;
                    int distanceBetweenBorderStops = nodeOfNodeId[parentNodeId]->borderDurations[make_pair(previousBorderStopId, borderStopId)];
                    if (distanceToPreviousBorderStop == INT_MAX || distanceBetweenBorderStops == INT_MAX) {
                        continue;
                    }
                    int distanceToBorderStopOfCurrentNode = distanceToPreviousBorderStop + distanceBetweenBorderStops;
                    if (distanceToBorderStopOfCurrentNode < minDistanceToCurrentBorderStop) {
                        minDistanceToCurrentBorderStop = distanceToBorderStopOfCurrentNode;
                    }
                }
                distancesToBorderStopsOfCurrentNode.push_back(make_pair(minDistanceToCurrentBorderStop, borderStopId));
            }

            nodeToBorderStopDurations[currentNodeId] = distancesToBorderStopsOfCurrentNode;
        }
        previousNodeId = currentNodeId;
    }

    // get the minimal duration to the target node
    vector<pair<int, int>> distancesToBorderStops = nodeToBorderStopDurations[targetNodeId];
    for (int i = 0; i < distancesToBorderStops.size(); i++) {
        int distanceToBorderStop = distancesToBorderStops[i].first;
        if (distanceToBorderStop < minDuration) {
            minDuration = distanceToBorderStop;
        }
    }

    return minDuration;
}

/*
    Calculate the minimal duration to a target stop from a source stop. Fill the map with the border stop durations of the nodes on the path if they are not already filled.

*/
int GTree::getMinimalDurationToStop(int sourceStopId, int targetStopId, map<int, vector<pair<int, int>>> &nodeToBorderStopDurations) {
    int minDuration = INT_MAX;

    // default case: the source stop is in the same node as the target stop
    if (nodeOfStopId[sourceStopId]->nodeId == nodeOfStopId[targetStopId]->nodeId) {
        return nodeOfStopId[sourceStopId]->borderDurations[make_pair(sourceStopId, targetStopId)];
    }

    int targetNodeId = nodeOfStopId[targetStopId]->nodeId;

    // fill the map with the border stop durations of the target node if it is not already filled
    if (nodeToBorderStopDurations.find(targetNodeId) == nodeToBorderStopDurations.end()) {
        getMinimalDurationToNode(sourceStopId, targetNodeId, nodeToBorderStopDurations);
    }

    vector<pair<int, int>> distancesToBorderStops = nodeToBorderStopDurations[targetNodeId];

    // get the minimal duration to the target stop
    for (int i = 0; i < distancesToBorderStops.size(); i++) {
        int distanceToBorderStop = distancesToBorderStops[i].first;
        int borderStopId = distancesToBorderStops[i].second;
        int distanceToTargetStop = nodeOfStopId[targetStopId]->borderDurations[make_pair(borderStopId, targetStopId)];
        if (distanceToBorderStop == INT_MAX || distanceToTargetStop == INT_MAX) {
            continue;
        }
        int duration = distanceToBorderStop + distanceToTargetStop;

        if (duration < minDuration) {
            minDuration = duration;
        }
    }

    return minDuration;
}

/*
    Get the path from a source stop to a target stop in the G-tree.
*/
vector<int> GTree::getNodePath(int stopId, int nodeId) {
    int rootId = root->nodeId;
    vector<int> path;
    vector<int> upwardPath;
    vector<int> downwardPath;

    upwardPath.push_back(nodeOfStopId[stopId]->nodeId);
    downwardPath.push_back(nodeId);

    while (upwardPath.back() != downwardPath.back()) {
        int upwardPathLastNodeId = upwardPath.back();
        int downwardPathLastNodeId = downwardPath.back();
        if(upwardPathLastNodeId != rootId && 
            downwardPathLastNodeId != rootId && 
            nodeOfNodeId[upwardPathLastNodeId]->parent->nodeId == nodeOfNodeId[downwardPathLastNodeId]->parent->nodeId) {
            break;
        } 
        if(upwardPathLastNodeId < downwardPathLastNodeId) {
            downwardPath.push_back(nodeOfNodeId[downwardPathLastNodeId]->parent->nodeId);
        } else {
            upwardPath.push_back(nodeOfNodeId[upwardPathLastNodeId]->parent->nodeId);
        }
    }

    for (int i = 0; i < upwardPath.size(); i++) {
        path.push_back(upwardPath[i]);
    }

    for (int i = downwardPath.size() - 1; i >= 0; i--) {
        if (path.back() == downwardPath[i]) {  
            continue;
        }
        path.push_back(downwardPath[i]);
    }

    return path;
}

/*
    Check if a stop is in a node.
*/
bool GTree::isVertexInNode(int stopId, int nodeId) {
    if (nodeOfStopId[stopId]->nodeId == nodeId || nodeId == root->nodeId) {
        return true;
    }

    vector<int> upwardPath;

    upwardPath.push_back(nodeOfStopId[stopId]->nodeId);

    while (upwardPath.back() != root->nodeId) {
        int upwardPathLastNodeId = upwardPath.back();
        if (upwardPathLastNodeId == nodeId) {
            return true;
        }
        upwardPath.push_back(nodeOfNodeId[upwardPathLastNodeId]->parent->nodeId);
    }

    return false;
}

/*
    Save the information of the tree in a json file such that it can be loaded.
*/
void GTree::exportTreeAsJson(DataType dataType, int numberOfChildrenPerNode, int maxNumberOfVerticesPerLeaf, int numberOfFiles) {
    cout << "Exporting G-tree as json file..." << endl;

    vector<GNode*> nodes = vector<GNode*>(0);
    vector<GNode*> tmpNodes = vector<GNode*>(0);
    nodes.push_back(this->root);
    tmpNodes.push_back(this->root);

    // get all nodes of the tree
    while(tmpNodes.size() > 0){
        GNode* node = tmpNodes[0];
        tmpNodes.erase(tmpNodes.begin());
        for (int i = 0; i < node->children.size(); i++) {
            nodes.push_back(node->children[i]);
            tmpNodes.push_back(node->children[i]);
        }
    }

    // calculate the number of nodes per file
    int nodesPerFile = nodes.size() / (numberOfFiles - 1);

    string dataTypeString = Importer::getDataTypeString(dataType);
    string folderPath = FOLDER_PREFIX + "graphs/" + dataTypeString + "/";

    for (int fileId = 0; fileId < numberOfFiles; fileId++) {
        string fileName = folderPath + "g-tree-" + to_string(numberOfChildrenPerNode) + "-" + to_string(maxNumberOfVerticesPerLeaf);
        if (USE_FOOTPATHS) {
            fileName += "-with-footpaths";
        }
        fileName += "-" + to_string(fileId) + ".json";

        // delete the file if it already exists
        remove(fileName.c_str());

        ofstream file;
        file.open(fileName);

        file << "{\n";
        file << "  \"dataType\": \"" << dataTypeString << "\",\n";
        file << "  \"numberOfChildrenPerNode\": " << numberOfChildrenPerNode << ",\n";
        file << "  \"maxNumberOfVerticesPerLeaf\": " << maxNumberOfVerticesPerLeaf << ",\n";
        file << "  \"numberOfNodes\": " << nodes.size() << ",\n";
        file << "  \"nodes\": [\n";

        for (int nodeIndex = fileId * nodesPerFile; nodeIndex < (fileId + 1) * nodesPerFile && nodeIndex < nodes.size(); nodeIndex++) {
            GNode* node = nodes[nodeIndex];

            file << "    {\n";
            file << "      \"nodeId\": " << node->nodeId << ",\n";
            if (node->nodeId == 0) {
                file << "      \"parent\": null,\n";
            } else {
                file << "      \"parent\": " << node->parent->nodeId << ",\n";
            }

            file << "      \"stopIds\": [";
            for (int i = 0; i < node->stopIds.size(); i++) {
                file << node->stopIds[i];
                if (i < node->stopIds.size() - 1) {
                    file << ", ";
                }
            }
            file << "],\n";

            file << "      \"borderStopIds\": [";
            for (int i = 0; i < node->borderStopIds.size(); i++) {
                file << node->borderStopIds[i];
                if (i < node->borderStopIds.size() - 1) {
                    file << ", ";
                }
            }
            file << "],\n";

            file << "      \"borderDurations\": [\n";
            for (auto const& entry : node->borderDurations) {
                file << "        {\"source\": " << entry.first.first << ", \"target\": " << entry.first.second << ", \"duration\": " << entry.second << "}";
                file << ",\n";
            }
            file << "      ]\n";

            for (int i = 0; i < node->children.size(); i++) {
                nodes.push_back(node->children[i]);
            }

            file << "    }";
            if (nodes.size() > 0) {
                file << ",";
            }
            file << "\n";
        }
        file << "  ]\n";

        file << "}\n";
        file.close();
    }

    cout << "G-tree exported.\n" << endl;
}

/*
    Import the G-tree from a json file.
*/
void GTree::importTreeFromJson(DataType dataType, int numberOfChildrenPerNode, int maxNumberOfVerticesPerLeaf, int numberOfFiles) {
    cout << "Importing G-tree from json file..." << endl;
    string dataTypeString = Importer::getDataTypeString(dataType);
    string folderPath = FOLDER_PREFIX + "graphs/" + dataTypeString + "/";

    for (int fileId = 0; fileId < numberOfFiles; fileId++) {
        string fileName = folderPath + "g-tree-" + to_string(numberOfChildrenPerNode) + "-" + to_string(maxNumberOfVerticesPerLeaf);
        if (USE_FOOTPATHS) {
            fileName += "-with-footpaths";
        }
        fileName += "-" + to_string(fileId) + ".json";

        ifstream file;
        file.open(fileName);
        Json::Value root;
        file >> root;

        string dataTypeStringFromFile = root["dataType"].asString();
        int numberOfChildrenPerNodeFromFile = root["numberOfChildrenPerNode"].asInt();
        int maxNumberOfVerticesPerLeafFromFile = root["maxNumberOfVerticesPerLeaf"].asInt();

        if (dataTypeString != dataTypeStringFromFile || numberOfChildrenPerNode != numberOfChildrenPerNodeFromFile || maxNumberOfVerticesPerLeaf != maxNumberOfVerticesPerLeafFromFile) {
            cout << "The G-tree could not be imported. The parameters do not match." << endl;
            return;
        }

        if (fileId == 0) {
            int numberOfNodes = root["numberOfNodes"].asInt();
            this->nodeOfNodeId = vector<GNode*>(numberOfNodes, nullptr);
        }
        
        Json::Value nodes = root["nodes"];

        for (int i = 0; i < nodes.size(); i++) {
            Json::Value node = nodes[i];
            GNode* gNode = new GNode();
            gNode->nodeId = node["nodeId"].asInt();

            if (node["parent"].isNull()) {
                gNode->parent = nullptr;
            } else {
                gNode->parent = nodeOfNodeId[node["parent"].asInt()];
            }

            for (int j = 0; j < node["stopIds"].size(); j++) {
                gNode->stopIds.push_back(node["stopIds"][j].asInt());
            }

            for (int j = 0; j < node["borderStopIds"].size(); j++) {
                gNode->borderStopIds.push_back(node["borderStopIds"][j].asInt());
            }

            for (int j = 0; j < node["borderDurations"].size(); j++) {
                Json::Value borderDuration = node["borderDurations"][j];
                gNode->borderDurations[make_pair(borderDuration["source"].asInt(), borderDuration["target"].asInt())] = borderDuration["duration"].asInt();
            }

            gNode->children = vector<GNode*>(0);

            this->nodeOfNodeId[gNode->nodeId] = gNode;
        }

        file.close();
    }

    this->nodeOfStopId = vector<GNode*>(Importer::stops.size(), nullptr);
    this->root = this->nodeOfNodeId[0];

    for (int i = this->nodeOfNodeId.size()-1; i > 0; i--) {
        GNode* node = this->nodeOfNodeId[i];
        if (node->parent != nullptr) {
            node->parent->children.push_back(node);
        }

        if (node->children.size() == 0) {
            for (int j = 0; j < node->stopIds.size(); j++) {
                this->nodeOfStopId[node->stopIds[j]] = node;
            }
        }
    }

    cout << "G-tree imported.\n" << endl;
}

void GTree::calculateBorderDistancesOfStopIds(vector<int> stopIds) {
    auto start = std::chrono::high_resolution_clock::now();
    std::cout << "Max threads: " << omp_get_max_threads() << "\n";
    #pragma omp parallel
    {
        #pragma omp for
        for (int i = 0; i < stopIds.size(); i++) {
            int stopId = stopIds[i];
            vector<int> targetStopIds = vector<int>(0);
            GNode* node = nodeOfStopId[stopId];
            while (node != nullptr) {
                if (find(node->stopIds.begin(), node->stopIds.end(), stopId) != node->stopIds.end()) {
                    for (int j = 0; j < node->stopIds.size(); j++) {
                        if (node->stopIds[j] != stopId && find(targetStopIds.begin(), targetStopIds.end(), node->stopIds[j]) == targetStopIds.end()){
                            targetStopIds.push_back(node->stopIds[j]);
                        }
                    }
                } else {
                    break;
                }
                node = node->parent;
            }

            vector<int> distances = Creator::networkGraph.getDistances(stopId, targetStopIds);

            node = nodeOfStopId[stopId];
            while (node != nullptr) {
                if (find(node->stopIds.begin(), node->stopIds.end(), stopId) != node->stopIds.end()) {
                    for (int j = 0; j < node->stopIds.size(); j++) {
                        node->borderDurations[make_pair(stopId, node->stopIds[j])] = distances[node->stopIds[j]];
                    }
                } else {
                    break;
                }
                node = node->parent;
            }
        }
    }
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::minutes>(end - start).count();
    cout << "Calculated " << stopIds.size() << " distances in " << duration << " minutes." << endl;
}