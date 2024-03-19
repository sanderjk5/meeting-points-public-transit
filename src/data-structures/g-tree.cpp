#include "g-tree.h"

#include <../constants.h>
#include <../data-handling/importer.h>
#include <iostream>
#include <vector>
#include <map>
#include <algorithm>
#include <limits.h>
#include <fstream>

using namespace std;

/*
    Dummy method to initialize the GTree.
*/
void GTree::initializeGTree() {
    return;
}

/*
    Calculate the minimal duration to a target node from a source stop. Fill the map with the border stop durations of the nodes on the path if they are not already filled.
*/
int GTree::getMinimalDurationToNode(int sourceStopId, int targetNodeId, map<pair<int, int>, vector<pair<int, int>>> &queryPointAndNodeToBorderStopDurations) {
    int minDuration = INT_MAX;
    
    // get the path from the source stop to the target node in the G-tree
    vector<int> path = getNodePath(sourceStopId, targetNodeId);

    // default case: the source stop is in the target node
    if(isVertexInNode(sourceStopId, targetNodeId)) {
        return 0;
    }

    // fill the map with the border stop durations of the first node if it is not already filled
    pair<int, int> queryPointAndNode = make_pair(sourceStopId, path[0]);
    if (queryPointAndNodeToBorderStopDurations.find(queryPointAndNode) == queryPointAndNodeToBorderStopDurations.end()) {
        vector<pair<int, int>> distancesToBorderStops = vector<pair<int, int>>(0);
        for (int i = 0; i < nodeOfNodeId[path[0]]->borderStopIds.size(); i++) {
            int borderStopId = nodeOfNodeId[path[0]]->borderStopIds[i];
            int distanceToBorderStop = nodeOfStopId[sourceStopId]->borderDurations[make_pair(sourceStopId, borderStopId)];
            distancesToBorderStops.push_back(make_pair(distanceToBorderStop, borderStopId));
        }
        queryPointAndNodeToBorderStopDurations[queryPointAndNode] = distancesToBorderStops;
    }

    // fill the map with the border stop durations of the other nodes on the path if they are not already filled
    int previousNodeId = path[0];
    for(int i = 1; i < path.size(); i++) {
        int currentNodeId = path[i];
        queryPointAndNode = make_pair(sourceStopId, currentNodeId);
        if (queryPointAndNodeToBorderStopDurations.find(queryPointAndNode) == queryPointAndNodeToBorderStopDurations.end()) {
            vector<pair<int, int>> distancesToBorderStopsOfPreviousNode = queryPointAndNodeToBorderStopDurations[make_pair(sourceStopId, previousNodeId)];
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

            queryPointAndNodeToBorderStopDurations[queryPointAndNode] = distancesToBorderStopsOfCurrentNode;
        }
        previousNodeId = currentNodeId;
    }

    // get the minimal duration to the target node
    queryPointAndNode = make_pair(sourceStopId, path.back());
    vector<pair<int, int>> distancesToBorderStops = queryPointAndNodeToBorderStopDurations[queryPointAndNode];
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
int GTree::getMinimalDurationToStop(int sourceStopId, int targetStopId, map<pair<int, int>, vector<pair<int, int>>> &queryPointAndNodeToBorderStopDurations) {
    int minDuration = INT_MAX;

    // default case: the source stop is in the same node as the target stop
    if (nodeOfStopId[sourceStopId]->nodeId == nodeOfStopId[targetStopId]->nodeId) {
        return nodeOfStopId[sourceStopId]->borderDurations[make_pair(sourceStopId, targetStopId)];
    }

    pair<int, int> queryPointAndNode = make_pair(sourceStopId, nodeOfStopId[targetStopId]->nodeId);

    // fill the map with the border stop durations of the target node if it is not already filled
    if (queryPointAndNodeToBorderStopDurations.find(queryPointAndNode) != queryPointAndNodeToBorderStopDurations.end()) {
        getMinimalDurationToNode(sourceStopId, nodeOfStopId[targetStopId]->nodeId, queryPointAndNodeToBorderStopDurations);
    }

    vector<pair<int, int>> distancesToBorderStops = queryPointAndNodeToBorderStopDurations[queryPointAndNode];

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
void GTree::saveTreeAsJson(DataType dataType, int numberOfChildrenPerNode, int maxNumberOfVerticesPerLeaf) {
    string dataTypeString = Importer::getDataTypeString(dataType);
    string folderPath = FOLDER_PREFIX + "graphs/" + dataTypeString + "/";

    string fileName = folderPath + "g-tree-" + to_string(numberOfChildrenPerNode) + "-" + to_string(maxNumberOfVerticesPerLeaf) + ".json";

    ofstream file;
    file.open(fileName);

    file << "{\n";
    file << "  \"dataType\": \"" << dataTypeString << "\",\n";
    file << "  \"numberOfChildrenPerNode\": " << numberOfChildrenPerNode << ",\n";
    file << "  \"maxNumberOfVerticesPerLeaf\": " << maxNumberOfVerticesPerLeaf << ",\n";
    file << "  \"nodes\": [\n";

    vector<GNode*> nodes = vector<GNode*>(0);
    nodes.push_back(this->root);
    
    while(nodes.size() > 0){
        GNode* node = nodes[0];
        nodes.erase(nodes.begin());
        file << "    {\n";
        file << "      \"nodeId\": " << node->nodeId << ",\n";
        if (node->parent == nullptr) {
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
            if (entry.first.first != node->borderDurations.rbegin()->first.first) {
                file << ",";
            }
            file << "\n";
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

/*
    Import the G-tree from a json file.
*/
//void GTree::importTreeFromJson(DataType dataType, int numberOfChildrenPerNode, int maxNumberOfVerticesPerLeaf) {
    // string dataTypeString = Importer::getDataTypeString(dataType);
    // string folderPath = FOLDER_PREFIX + "graphs/" + dataTypeString + "/";

    // string fileName = folderPath + "g-tree-" + to_string(numberOfChildrenPerNode) + "-" + to_string(maxNumberOfVerticesPerLeaf) + ".json";

    // ifstream file;
    // file.open(fileName);

    // string line;
    // string dataTypeStringFromFile;
    // int numberOfChildrenPerNodeFromFile;
    // int maxNumberOfVerticesPerLeafFromFile;

    // getline(file, line);
    // getline(file, line);
    // dataTypeStringFromFile = line.substr(14, line.length() - 15);
    // getline(file, line);
    // numberOfChildrenPerNodeFromFile = stoi(line.substr(28, line.length() - 29));
    // getline(file, line);
    // maxNumberOfVerticesPerLeafFromFile = stoi(line.substr(30, line.length() - 31));

    // if (dataTypeString != dataTypeStringFromFile || numberOfChildrenPerNode != numberOfChildrenPerNodeFromFile || maxNumberOfVerticesPerLeaf != maxNumberOfVerticesPerLeafFromFile) {
    //     cout << "The G-tree could not be imported. The parameters do not match." << endl;
    //     return;
    // }

    // vector<GNode*> nodes = vector<GNode*>(0);

    // while (getline(file, line)) {
    //     if (line.find("nodeId") != string::npos) {
    //         int nodeId = stoi(line.substr(14, line.length() - 15));
    //         GNode* node = new GNode();
    //         node->nodeId = nodeId;

    //         getline(file, line);
    //         if (line.find("parent") != string::npos) {
    //             if (line.find("null") == string::npos) {
    //                 int parentId = stoi(line.substr(14, line.length() - 15));
    //                 node->parent = nodeOfNodeId[parentId];
    //             }
    //         }

    //         getline(file, line);
    //         if (line.find("stopIds") != string::npos) {
    //             vector<int> stopIds = vector<int>(0);
    //             string stopIdsString = line.substr(16, line.length() - 17);
    //             size_t pos = 0;
    //             string delimiter = ", ";
    //             while ((pos = stopIdsString.find(delimiter)) != string::npos) {
    //                 stopIds.push_back(stoi(stopIdsString.substr(0, pos)));
    //                 stopIdsString.erase(0, pos + delimiter.length());
    //             }
    //             stopIds.push_back(stoi(stopIdsString));
    //             node->stopIds = stopIds;
    //         }

    //         getline(file, line);
    //         if (line.find("borderStopIds") != string::npos) {
    //             vector<int> borderStopIds = vector<int>(0);
    //             string borderStopIdsString = line.substr(20, line.length() - 21);
    //             size_t pos = 0;
    //             string delimiter = ", ";
    //             while ((pos = borderStopIdsString.find(delimiter)) != string::npos) {
    //                 borderStopIds.push_back(stoi(borderStopIdsString.substr(0, pos)));
    //                 borderStopIdsString.erase(0, pos + delimiter.length());
    //             }
    //             borderStopIds.push_back(stoi(borderStopIdsString));
    //             node->borderStopIds = borderStopIds;
    //         }

    //         getline(file, line);
    //         if (line.find("borderDurations") != string::npos) {
    //             map<pair<int, int>, int> borderDurations = map<pair<int, int>, int>();
    //             while (getline(file, line)) {
    //                 if (line.find("}") != string::npos) {
    //                     break;
    //                 }
    //                 int source = stoi(line.substr(14, line.find(",") - 14));
    //                 int target = stoi(line.substr(line.find("target") + 9, line.find(",") - line.find("target") - 9));
    //                 int duration = stoi(line.substr(line.find("duration") + 10, line.length() - line.find("duration") - 11));
    //                 borderDurations[make_pair(source, target)] = duration;
    //             }
    //             node->borderDurations = borderDurations;
    //         }

    //         nodes.push_back(node);
    //     }
    // }



    // int currentNodeIndex = 0;
    // this->root = nodes[currentNodeIndex];
    // this->nodeOfStopId = vector<GNode*>(Importer::stops.size(), nullptr);
    // this->nodeOfNodeId = vector<GNode*>(0);

    // while (currentNodeIndex < nodes.size()) {
    //     this->nodeOfNodeId.push_back(nodes[currentNodeIndex]);


    // }

    // file.close();
//}