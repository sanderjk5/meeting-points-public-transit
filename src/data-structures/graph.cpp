#include "graph.h"

#include <../constants.h>
#include <../data-handling/importer.h>

#include <limits.h>

#include <vector>
#include <queue>
#include <fstream>
#include <iostream>

/*
    Use Dijkstra's algorithm to calculate the minimal distance to a target node from a source stop.
*/
vector<int> Graph::getDistances(int sourceStopId, vector<int> targetStopIds) {
    vector<int> distances = vector<int>(this->vertices.size(), INT_MAX);
    vector<bool> visited = vector<bool>(this->vertices.size(), false);
    
    distances[sourceStopId] = 0;
    priority_queue<pair<int, int>, vector<pair<int, int>>, greater<pair<int, int>>> pq;

    int targetStopCounter = 0;
    vector<bool> targetStopIdsFound = vector<bool>(targetStopIds.size(), false);
    
    pq.push(make_pair(0, sourceStopId));

    while(!pq.empty()) {
        int u = pq.top().second;
        pq.pop();

        if (visited[u]) {
            continue;
        }

        for (int i = 0; i < targetStopIds.size(); i++) {
            if (targetStopIds[i] == u) {
                if (!targetStopIdsFound[i]) {
                    targetStopIdsFound[i] = true;
                    targetStopCounter++;
                }
                break;
            }
        }

        if (targetStopCounter == targetStopIds.size()) {
            break;
        }

        for (int i = 0; i < this->adjacencyList[u].size(); i++) {
            int v = this->adjacencyList[u][i].targetStopId;
            int weight = this->adjacencyList[u][i].ewgt;

            if (distances[v] > distances[u] + weight) {
                distances[v] = distances[u] + weight;
                pq.push(make_pair(distances[v], v));
            }
        }

        visited[u] = true;
    }

    return distances;
}


void Graph::exportGraph(DataType dataType) {
    string dataTypeString = Importer::getDataTypeString(dataType);
    string folderPath = FOLDER_PREFIX + "graphs/" + dataTypeString + "/";
    string fileName = folderPath + "graph";
    if (USE_FOOTPATHS) {
        fileName += "-with-footpaths";
    }
    fileName += ".txt";

    remove(fileName.c_str());

    ofstream file;
    file.open(fileName);

    int numberOfVertices = this->vertices.size();
    int numberOfEdges = 0;
    for (int i = 0; i < this->adjacencyList.size(); i++) {
        numberOfEdges += this->adjacencyList[i].size();
    }

    // header
    file << to_string(numberOfVertices) << " " << to_string(numberOfEdges / 2) << " " << "001" << endl;

    // vertices
    for(int i = 0; i < this->vertices.size(); i++) {
        string line;
        for (int j = 0; j < this->adjacencyList[i].size(); j++) {
            int targetStopId = this->adjacencyList[i][j].targetStopId + 1;
            int edgeWeight = this->adjacencyList[i][j].ewgt;
            if (edgeWeight == 0) {
                edgeWeight = 1;
            }
            line += to_string(targetStopId) + " " + to_string(edgeWeight);
            if (j < this->adjacencyList[i].size() - 1) {
                line += " ";
            }
        }
        file << line << endl;
    }

    file.close();
}

void Graph::importPartition(DataType dataType, int numberOfPartitions) {
    cout << "Importing partition" << endl;
    string dataTypeString = Importer::getDataTypeString(dataType);
    string folderPath = FOLDER_PREFIX + "graphs/" + dataTypeString + "/";
    string fileName = folderPath + "partition";
    if (USE_FOOTPATHS) {
        fileName += "-with-footpaths";
    }
    fileName += "-" + to_string(numberOfPartitions) + ".txt";

    ifstream file;
    file.open(fileName);

    if (!file.is_open()) {
        cout << "Could not open file " << fileName << endl;
        return;
    }

    int numberOfVertices = this->vertices.size();

    this->partition = vector<int>(numberOfVertices);

    for (int i = 0; i < numberOfVertices; i++) {
        file >> this->partition[i];
    }

    file.close();
}