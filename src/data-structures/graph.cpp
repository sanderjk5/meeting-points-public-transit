#include "graph.h"

#include <../constants.h>
#include <../data-handling/importer.h>

#include <limits.h>

#include <vector>
#include <queue>
#include <fstream>
#include <iostream>
#include <chrono>

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

void Graph::createContractionHierarchie() {
    cout << "Creating contraction hierarchie..." << endl;
    auto start = chrono::high_resolution_clock::now();

    stopIdsSortedByLevel = vector<int>();

    priority_queue<pair<int, int>, vector<pair<int, int>>, greater<pair<int, int>>> pq;

    for (int i = 0; i < this->vertices.size(); i++) {
        int edgeDifference = this->calculateEdgeDifferenceAndGetShortcuts(i).first;
        pq.push(make_pair(edgeDifference, i));
    }

    int currentLevel = 0;

    while (!pq.empty()) {
        int edgeDifference = pq.top().first;
        int vertexIndex = pq.top().second;
        pq.pop();

        pair<int, vector<Shortcut>> updatedEdgeDifferenceAndShortcuts = this->calculateEdgeDifferenceAndGetShortcuts(vertexIndex);
        int updatedEdgeDifference = updatedEdgeDifferenceAndShortcuts.first;

        if (updatedEdgeDifference > edgeDifference) {
            pq.push(make_pair(updatedEdgeDifference, vertexIndex));
            continue;
        }

        this->vertices[vertexIndex].level = currentLevel;
        stopIdsSortedByLevel.push_back(vertexIndex);

        vector<Shortcut> shortcuts = updatedEdgeDifferenceAndShortcuts.second;

        for (int i = 0; i < shortcuts.size(); i++) {
            Edge edge;
            edge.targetStopId = shortcuts[i].targetStopId;
            edge.ewgt = shortcuts[i].ewgt;
            this->adjacencyList[shortcuts[i].sourceStopId].push_back(edge);
        }

        currentLevel++;
    }

    auto end = chrono::high_resolution_clock::now();
    auto duration = chrono::duration_cast<chrono::milliseconds>(end - start);
    cout << "Contraction hierarchie created in " << duration.count() << "ms.\n" << endl;
}

pair<int, vector<Shortcut>> Graph::calculateEdgeDifferenceAndGetShortcuts(int vertexIndex) {
    int edgeDifference = 0;
    vector<Shortcut> shortcuts;

    // count the number of edges that would be saved by contracting the vertex
    for (int i = 0; i < this->adjacencyList[vertexIndex].size(); i++) {
        int targetStopId = this->adjacencyList[vertexIndex][i].targetStopId;
        
        if (this->vertices[targetStopId].level == -1) {
            edgeDifference = edgeDifference - 2;
        }
    }

    int contractedNeighborsCounter = 0;

    // find the shortcuts that would be created by contracting the vertex
    for (int i = 0; i < this->adjacencyList[vertexIndex].size(); i++) {
        int sourceStopId = this->adjacencyList[vertexIndex][i].targetStopId;
        map<int, int> distancesViaVertex;
        int maxDistance = -1;

        for (int j = i+1; j < this->adjacencyList[vertexIndex].size(); j++) {
            int targetStopId = this->adjacencyList[vertexIndex][j].targetStopId;

            if (this->vertices[targetStopId].level != -1) {
                contractedNeighborsCounter++;
                continue;
            }

            int distance = this->adjacencyList[vertexIndex][i].ewgt + this->adjacencyList[vertexIndex][j].ewgt;
            distancesViaVertex[targetStopId] = distance;
            if (distance > maxDistance) {
                maxDistance = distance;
            }
        }

        if (distancesViaVertex.size() == -1) {
            continue;
        }

        vector<int> minimalDistances = this->getDistancesForCHCreation(sourceStopId, vertexIndex, maxDistance);

        // interate over all entries of the map and create the shortcuts
        for (auto it = distancesViaVertex.begin(); it != distancesViaVertex.end(); it++) {
            int targetStopId = it->first;
            int distance = it->second;
            if (distance < minimalDistances[targetStopId]) {
                Shortcut shortcut1;
                shortcut1.sourceStopId = sourceStopId;
                shortcut1.targetStopId = targetStopId;
                shortcut1.ewgt = distance;
                shortcuts.push_back(shortcut1);

                Shortcut shortcut2;
                shortcut2.sourceStopId = targetStopId;
                shortcut2.targetStopId = sourceStopId;
                shortcut2.ewgt = distance;
                shortcuts.push_back(shortcut2);
            }
        }
    }

    edgeDifference += shortcuts.size();

    edgeDifference += contractedNeighborsCounter;

    return make_pair(edgeDifference, shortcuts);
}

vector<int> Graph::getDistancesForCHCreation(int sourceStopId, int excludeStopId, int maxDistance) {
    vector<int> distances = vector<int>(this->vertices.size(), INT_MAX);
    vector<bool> visited = vector<bool>(this->vertices.size(), false);
    
    distances[sourceStopId] = 0;
    priority_queue<pair<int, int>, vector<pair<int, int>>, greater<pair<int, int>>> pq;
    
    pq.push(make_pair(0, sourceStopId));

    while(!pq.empty()) {
        int u = pq.top().second;
        pq.pop();

        if (distances[u] > maxDistance) {
            break;
        }

        if (visited[u]) {
            continue;
        }

        for (int i = 0; i < this->adjacencyList[u].size(); i++) {
            int v = this->adjacencyList[u][i].targetStopId;

            if (v == excludeStopId || this->vertices[v].level != -1) {
                continue;
            }

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

map<int, vector<int>> Graph::getDistancesWithPhast(vector<int> sourceStopIds) {
    auto start = chrono::high_resolution_clock::now();
    map<int, vector<int>> distances;

    for (int i = 0; i < sourceStopIds.size(); i++) {
        int sourceStopId = sourceStopIds[i];
        distances[sourceStopId] = vector<int>(this->vertices.size(), INT_MAX);
    }

    // perform upward searches from the source stops
    #pragma omp parallel for
    for (int i = 0; i < sourceStopIds.size(); i++) {
        int sourceStopId = sourceStopIds[i];
        
        priority_queue<pair<int, int>, vector<pair<int, int>>, greater<pair<int, int>>> pq;
        pq.push(make_pair(0, sourceStopId));

        while(!pq.empty()) {
            int u = pq.top().second;
            pq.pop();

            for (int i = 0; i < this->adjacencyList[u].size(); i++) {
                int v = this->adjacencyList[u][i].targetStopId;

                if (this->vertices[v].level <= this->vertices[u].level) {
                    continue;
                }

                int weight = this->adjacencyList[u][i].ewgt;

                if (distances[sourceStopId][v] > distances[sourceStopId][u] + weight) {
                    distances[sourceStopId][v] = distances[sourceStopId][u] + weight;
                    pq.push(make_pair(distances[sourceStopId][v], v));
                }
            }
        }
    }

    // perform downward scans from the highest level
    for (int i = stopIdsSortedByLevel.size() - 2; i >= 0; i--) {
        int vertexIndex = stopIdsSortedByLevel[i];
        int level = this->vertices[vertexIndex].level;

        for (int j = 0; j < this->adjacencyList[vertexIndex].size(); j++) {
            int targetStopId = this->adjacencyList[vertexIndex][j].targetStopId;

            if (this->vertices[targetStopId].level < level) {
                continue;
            }

            int weight = this->adjacencyList[vertexIndex][j].ewgt;

            for (int k = 0; k < sourceStopIds.size(); k++) {
                int sourceStopId = sourceStopIds[k];

                if (distances[sourceStopId][vertexIndex] > distances[sourceStopId][targetStopId] + weight) {
                    distances[sourceStopId][vertexIndex] = distances[sourceStopId][targetStopId] + weight;
                }
            }
        }
    }

    auto end = chrono::high_resolution_clock::now();
    auto duration = chrono::duration_cast<chrono::milliseconds>(end - start);
    cout << "Phast completed in " << duration.count() << "ms.\n" << endl;

    return distances;
}