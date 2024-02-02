#include "graph.h"

#include <limits.h>

#include <vector>
#include <queue>

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