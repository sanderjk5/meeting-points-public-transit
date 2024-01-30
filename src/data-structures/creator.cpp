#include "creator.h"

#include <../data-handling/importer.h>
#include <../constants.h>
#include "gtree.h"

#include <vector>
#include <map>
#include <iostream>
#include <chrono>
#include <cmath>
#include <algorithm>

using namespace std;

Graph Creator::networkGraph = Graph();
GTree Creator::networkGTree = GTree();

void Creator::createNetworkGraph() {
    cout << "Creating network graph..." << endl;

    // Start the timer
    auto start = std::chrono::high_resolution_clock::now();

    networkGraph.adjacencyList = vector<vector<Edge>>(Importer::stops.size(), vector<Edge>(0));

    for (int i = 0; i < Importer::connections.size(); i++) {
        Connection connection = Importer::connections[i];
        int connectionArrivalTime = connection.arrivalTime;
        if (connectionArrivalTime < connection.departureTime) {
            connectionArrivalTime += SECONDS_PER_DAY;
        }

        Edge edge;
        edge.targetStopId = connection.arrivalStopId;
        edge.ewgt = connectionArrivalTime - connection.departureTime;

        bool addEdge = true;

        for (int j = 0; j < networkGraph.adjacencyList[connection.departureStopId].size(); j++) {
            if (networkGraph.adjacencyList[connection.departureStopId][j].targetStopId == edge.targetStopId) {
                if (networkGraph.adjacencyList[connection.departureStopId][j].ewgt > edge.ewgt) {
                    networkGraph.adjacencyList[connection.departureStopId][j].ewgt = edge.ewgt;
                }
                addEdge = false;
                break;
            }
        }

        if (addEdge) {
            networkGraph.adjacencyList[connection.departureStopId].push_back(edge);
        }
    }

    for (int i = 0; i < networkGraph.adjacencyList.size(); i++) {
        Vertex vertex;
        vertex.stopId = i;
        vertex.vwgt = 1;
        vertex.nedges = networkGraph.adjacencyList[i].size();
        vertex.cewgt = 0;
        vertex.adjwgt = 0;

        for (int j = 0; j < networkGraph.adjacencyList[i].size(); j++) {
            vertex.adjwgt += networkGraph.adjacencyList[i][j].ewgt;
        }

        networkGraph.vertices.push_back(vertex);
    }

    // Stop the timer and calculate the duration
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

    // Print the duration
    cout << "Network graph creation duration: " << duration << " milliseconds\n" << endl;
}


void Creator::createNetworkGTree(int numberOfChildrenPerNode, int maxNumberOfVerticesPerLeaf) {
    cout << "Creating network g-tree..." << endl;

    // Start the timer
    auto start = std::chrono::high_resolution_clock::now();

    int numberOfVertices = networkGraph.vertices.size();
    int leafs = numberOfVertices / maxNumberOfVerticesPerLeaf;
    int depth = (log(leafs) / log(numberOfChildrenPerNode)) + 1;
    leafs = pow(numberOfChildrenPerNode, depth);

    vector<Graph> graphs = partitionateGraph(networkGraph, leafs, maxNumberOfVerticesPerLeaf);
    networkGTree = createGTree(networkGraph, graphs, numberOfChildrenPerNode, depth);

    // Stop the timer and calculate the duration
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

    // Print the duration
    cout << "Network g-tree creation duration: " << duration << " milliseconds\n" << endl;
}

vector<Graph> Creator::partitionateGraph(Graph graph, int numberOfPartitions, int maxNumberOfVerticesInGraph) {
    vector<Graph> previousGraphs = vector<Graph>(0);
    previousGraphs.push_back(graph);

    while(previousGraphs.size() < numberOfPartitions) {
        vector<Graph> newGraphs = vector<Graph>(0);

        for (int j = 0; j < previousGraphs.size(); j++) {
            vector<Graph> coarsedGraphs = coarseGraph(previousGraphs[j], maxNumberOfVerticesInGraph);
            partitionateCoarsedGraph(coarsedGraphs[coarsedGraphs.size()-1], KL_ITERATIONS);
            refineGraphs(coarsedGraphs);
            vector<Graph> partitionatedGraphs = splitPartitionatedGraph(coarsedGraphs[0]);
            for(int k = 0; k < partitionatedGraphs.size(); k++) {
                newGraphs.push_back(partitionatedGraphs[k]);
            }
        }

        previousGraphs = newGraphs;
    }

    return previousGraphs;
}

vector<Graph> Creator::coarseGraph(Graph &graph, int maxNumberOfVerticesInGraph) {
    vector<Graph> coarsedGraphs = vector<Graph>(0);
    coarsedGraphs.push_back(graph);

    while(coarsedGraphs[coarsedGraphs.size()-1].vertices.size() > maxNumberOfVerticesInGraph){
        Graph* previousGraph = &coarsedGraphs[coarsedGraphs.size()-1];
        Graph coarsedGraph = Graph();

        // map vertices to vertices
        vector<int> visitingOrder = vector<int>(previousGraph->vertices.size());
        for (int i = 0; i < visitingOrder.size(); i++) {
            visitingOrder[i] = i;
        }
        random_shuffle(visitingOrder.begin(), visitingOrder.end());

        vector<int> matched = vector<int>(previousGraph->vertices.size(), 0);

        previousGraph->matchings = vector<int>(previousGraph->vertices.size(), -1);
        previousGraph->indexToCoarserGraphIndex = map<int, int>();

        for(int i = 0; i < visitingOrder.size(); i++) {
            int vertexIndex = visitingOrder[i];
            if (matched[vertexIndex] == 0) {
                int maxEdgeWeight = 0;
                int maxEdgeWeightIndex = -1;

                for (int j = 0; j < previousGraph->adjacencyList[vertexIndex].size(); j++) {
                    int mappedVertexIndex = previousGraph->adjacencyList[vertexIndex][j].targetStopId;
                    if (matched[mappedVertexIndex] == 0) {
                        if (previousGraph->adjacencyList[vertexIndex][j].ewgt > maxEdgeWeight) {
                            maxEdgeWeight = previousGraph->adjacencyList[vertexIndex][j].ewgt;
                            maxEdgeWeightIndex = mappedVertexIndex;
                        }
                    }
                }

                if(maxEdgeWeightIndex != -1) {
                    matched[vertexIndex] = 1;
                    matched[maxEdgeWeightIndex] = -1;
                    previousGraph->matchings[vertexIndex] = maxEdgeWeightIndex;
                    previousGraph->matchings[maxEdgeWeightIndex] = vertexIndex;
                }
            }
        }

        // contract vertices
        int indexInCoarsedGraph = 0;
        int numberOfVerticesInPreviousGraph = previousGraph->vertices.size();
        for (int i = 0; i < numberOfVerticesInPreviousGraph; i++){
            if (matched[i] != -1){
                previousGraph->indexToCoarserGraphIndex[i] = indexInCoarsedGraph;
                indexInCoarsedGraph++;
            }
        }

        for (int i = 0; i < numberOfVerticesInPreviousGraph; i++){
            if (matched[i] == -1) {
                previousGraph->indexToCoarserGraphIndex[i] = previousGraph->indexToCoarserGraphIndex[previousGraph->matchings[i]];
            }
        }


        coarsedGraph.adjacencyList = vector<vector<Edge>>(indexInCoarsedGraph, vector<Edge>(0));
        for (int i = 0; i < numberOfVerticesInPreviousGraph; i++){
            if (matched[i] != -1){
                int indexInCoarsedGraphOfCurrentVertex = previousGraph->indexToCoarserGraphIndex[i];
                int mappedVertexIndex = previousGraph->matchings[i];
                map<int, int> targetStopIdToWeight = map<int, int>();
                int numberOfOutgoingEdges = previousGraph->adjacencyList[i].size();
                for (int j = 0; j < numberOfOutgoingEdges; j++) {
                    int targetStopId = previousGraph->indexToCoarserGraphIndex[previousGraph->adjacencyList[i][j].targetStopId];
                    int edgeWeight = previousGraph->adjacencyList[i][j].ewgt;
                    if (targetStopIdToWeight.find(targetStopId) == targetStopIdToWeight.end()) {
                        targetStopIdToWeight[targetStopId] = edgeWeight;
                    } else {
                        targetStopIdToWeight[targetStopId] += edgeWeight;
                    }
                }
                if(matched[i] == 1){
                    int numberOfOutgoingEdges = previousGraph->adjacencyList[mappedVertexIndex].size();
                    for (int j = 0; j < numberOfOutgoingEdges; j++) {
                        int targetStopId = previousGraph->indexToCoarserGraphIndex[previousGraph->adjacencyList[mappedVertexIndex][j].targetStopId];
                        int edgeWeight = previousGraph->adjacencyList[mappedVertexIndex][j].ewgt;
                        if (targetStopIdToWeight.find(targetStopId) == targetStopIdToWeight.end()) {
                            targetStopIdToWeight[targetStopId] = edgeWeight;
                        } else {
                            targetStopIdToWeight[targetStopId] += edgeWeight;
                        }
                    }
                }

                int weightSum = 0;
                for (auto it = targetStopIdToWeight.begin(); it != targetStopIdToWeight.end(); it++) {
                    if (it->first == indexInCoarsedGraphOfCurrentVertex) {
                        continue;
                    }
                    Edge edge;
                    edge.targetStopId = it->first;
                    edge.ewgt = it->second;
                    coarsedGraph.adjacencyList[indexInCoarsedGraphOfCurrentVertex].push_back(edge);
                    weightSum += it->second;
                }

                Vertex vertex;
                if(matched[i] == 1) {
                    vertex.vwgt = previousGraph->vertices[i].vwgt + previousGraph->vertices[mappedVertexIndex].vwgt;
                    vertex.cewgt = previousGraph->vertices[i].cewgt + targetStopIdToWeight[indexInCoarsedGraphOfCurrentVertex];
                } else {
                    vertex.vwgt = previousGraph->vertices[i].vwgt;
                    vertex.cewgt = previousGraph->vertices[i].cewgt;
                }
                vertex.nedges = coarsedGraph.adjacencyList[indexInCoarsedGraphOfCurrentVertex].size();
                vertex.adjwgt = weightSum;
                coarsedGraph.vertices.push_back(vertex);
            }
        }

        if(coarsedGraph.vertices.size() == previousGraph->vertices.size()){
            break;
        }
        coarsedGraphs.push_back(coarsedGraph);
    }

    return coarsedGraphs;
}

void Creator::partitionateCoarsedGraph(Graph &graph, int klIterations) {
    vector<vector<int>> partitions = vector<vector<int>>(0);
    vector<int> edgeCutOfPartitions = vector<int>(0);

    for (int i = 0; i < klIterations; i++){
        int numberOfUnusedSwaps = 0;
        int swaps = 0;

        vector<int> partition = vector<int>(graph.vertices.size(), 0);

        // set initial partition
        if (graph.partition.size() > 0) {
            partition = graph.partition;
        } else {
            // generate random partition
            vector<int> visitingOrder = vector<int>(graph.vertices.size());
            for (int i = 0; i < visitingOrder.size(); i++) {
                visitingOrder[i] = i;
            }
            random_shuffle(visitingOrder.begin(), visitingOrder.end());
            for (int i = 0; i < graph.vertices.size() / 2; i++) {
                partition[visitingOrder[i]] = 1;
            }
        }

        vector<bool> swappedVertex = vector<bool>(graph.vertices.size(), false);

        while(numberOfUnusedSwaps < KL_MAX_UNUSED_SWAPS && swaps < graph.vertices.size()) {
            // calculate edge cut for each vertex and find max gain vertex
            vector<int> externalDegree = vector<int>(graph.vertices.size(), 0);
            vector<int> internalDegree = vector<int>(graph.vertices.size(), 0);
            int partSize = 0;
            vector<int> maxGain = vector<int>(2, 0);
            vector<int> maxGainVertex = vector<int>(2, -1);
            for (int j = 0; j < graph.vertices.size(); j++) {
                if(swappedVertex[j]){
                    continue;
                }
                for (int k = 0; k < graph.adjacencyList[j].size(); k++) {
                    if (partition[j] != partition[graph.adjacencyList[j][k].targetStopId]) {
                        externalDegree[j] += graph.adjacencyList[j][k].ewgt;
                    } else {
                        internalDegree[j] += graph.adjacencyList[j][k].ewgt;
                    }
                }
                int gain = externalDegree[j] - internalDegree[j];
                if (gain > maxGain[partition[j]]) {
                    maxGain[partition[j]] = gain;
                    maxGainVertex[partition[j]] = j;
                }
                if (partition[j] == 0) {
                    partSize++;
                }
            }

            int largerPart = 0;
            if (partSize < graph.vertices.size() / 2) {
                largerPart = 1;
            }

            // swap max gain vertex
            if(maxGain[largerPart] > 0){
                partition[maxGainVertex[largerPart]] = 1 - partition[maxGainVertex[largerPart]];
                numberOfUnusedSwaps = 0;
            } else {
                numberOfUnusedSwaps++;
            }
            swaps++;
        }

        // calculate edge cut of partition
        int edgeCut = 0;
        for (int j = 0; j < graph.vertices.size(); j++) {
            for (int k = 0; k < graph.adjacencyList[j].size(); k++) {
                if (partition[j] != partition[graph.adjacencyList[j][k].targetStopId]) {
                    edgeCut += graph.adjacencyList[j][k].ewgt;
                }
            }
        }

        partitions.push_back(partition);
        edgeCutOfPartitions.push_back(edgeCut);
    }

    // find best partition
    int minEdgeCut = edgeCutOfPartitions[0];
    int minEdgeCutIndex = 0;
    for (int i = 1; i < edgeCutOfPartitions.size(); i++) {
        if (edgeCutOfPartitions[i] < minEdgeCut) {
            minEdgeCut = edgeCutOfPartitions[i];
            minEdgeCutIndex = i;
        }
    }

    // set partition
    graph.partition = partitions[minEdgeCutIndex];
}

void Creator::refineGraphs(vector<Graph> &graphs) {
    for(int i = graphs.size()-2; i>=0; i--){
        Graph* previousGraph = &graphs[i+1];
        Graph* graph = &graphs[i];

        // set the partition of the graph
        int numberOfVertices = graph->vertices.size();
        graph->partition = vector<int>(numberOfVertices, 0);
        for (int j = 0; j < numberOfVertices; j++) {
            graph->partition[j] = previousGraph->partition[graph->indexToCoarserGraphIndex[j]];
        }

        // refine the partition of the graph
        partitionateCoarsedGraph(*graph, 1);
    }
}

vector<Graph> Creator::splitPartitionatedGraph(Graph &graph) {
    vector<Graph> graphs = vector<Graph>(2, Graph());

    vector<int> indices = vector<int>(2, 0);
    vector<map<int, int>> oldToNewIndex = vector<map<int, int>>(2, map<int, int>());

    for (int i = 0; i < graph.vertices.size(); i++) {
        if (graph.partition[i] == 0) {
            oldToNewIndex[0][i] = indices[0];
            indices[0]++;
            graphs[0].vertices.push_back(graph.vertices[i]);
            vector<Edge> adjacencyList = vector<Edge>(0);
            for (int j = 0; j < graph.adjacencyList[i].size(); j++) {
                if(graph.partition[graph.adjacencyList[i][j].targetStopId] != 0){
                    continue;
                }
                Edge edge;
                edge.targetStopId = graph.adjacencyList[i][j].targetStopId;
                edge.ewgt = graph.adjacencyList[i][j].ewgt;
                adjacencyList.push_back(edge);
            }
            graphs[0].adjacencyList.push_back(adjacencyList);
        } else {
            oldToNewIndex[1][i] = indices[1];
            indices[1]++;
            graphs[1].vertices.push_back(graph.vertices[i]);
            vector<Edge> adjacencyList = vector<Edge>(0);
            for (int j = 0; j < graph.adjacencyList[i].size(); j++) {
                if(graph.partition[graph.adjacencyList[i][j].targetStopId] != 1){
                    continue;
                }
                Edge edge;
                edge.targetStopId = graph.adjacencyList[i][j].targetStopId;
                edge.ewgt = graph.adjacencyList[i][j].ewgt;
                adjacencyList.push_back(edge);
            }
            graphs[1].adjacencyList.push_back(adjacencyList);
        }
    }

    for (int i = 0; i < graphs.size(); i++) {
        for (int j = 0; j < graphs[i].adjacencyList.size(); j++) {
            for (int k = 0; k < graphs[i].adjacencyList[j].size(); k++) {
                graphs[i].adjacencyList[j][k].targetStopId = oldToNewIndex[i][graphs[i].adjacencyList[j][k].targetStopId];
            }
        }
    }

    return graphs;
}

GTree Creator::createGTree(Graph &originalGraph, vector<Graph> &graphs, int numberOfChildrenPerNode, int depth) {
    GTree gTree = GTree();
    gTree.nodeOfStopId = vector<GNode*>(Importer::stops.size(), nullptr);

    vector<GNode*> previousLevelNodes = vector<GNode*>(0);

    // create leaf nodes using the graphs
    for (int i = 0;  i < graphs.size(); i++){
        GNode* node = new GNode();
        node->stopIds = vector<int>(0);
        node->borderStopIds = vector<int>(0);
        node->borderDurations = map<pair<int, int>, int>();

        for (int j = 0; j < graphs[i].vertices.size(); j++) {
            int stopId = graphs[i].vertices[j].stopId;
            node->stopIds.push_back(stopId);
            gTree.nodeOfStopId[stopId] = node;
            for (int k = 0; k < originalGraph.adjacencyList[stopId].size(); k++) {
                int targetStopId = originalGraph.adjacencyList[stopId][k].targetStopId;
                if(find(node->stopIds.begin(), node->stopIds.end(), targetStopId) == node->stopIds.end()){
                    node->borderStopIds.push_back(stopId);
                    break;
                }
            }
        }

        for (int j = 0; j < node->stopIds.size(); j++) {
            vector<int> distances = originalGraph.getDistances(node->stopIds[j], node->borderStopIds);
            for (int k = 0; k < node->borderStopIds.size(); k++) {
                node->borderDurations[make_pair(node->stopIds[j], node->borderStopIds[k])] = distances[node->borderStopIds[k]];
            }
        }

        for (int j = 0; j < node->borderStopIds.size(); j++) {
            vector<int> distances = originalGraph.getDistances(node->borderStopIds[j], node->stopIds);
            for (int k = 0; k < node->stopIds.size(); k++) {
                node->borderDurations[make_pair(node->borderStopIds[j], node->stopIds[k])] = distances[node->stopIds[k]];
            }
        }

        previousLevelNodes.push_back(node);
    }

    // create the rest of the nodes
    while(previousLevelNodes.size() > 1){
        vector<GNode*> currentLevelNodes = vector<GNode*>(0);

        for (int j = 0; j < previousLevelNodes.size()/numberOfChildrenPerNode; j++) {
            GNode* node = new GNode();
            
            node->children = vector<GNode*>(0);
            node->borderStopIds = vector<int>(0);

            for (int k = 0; k < numberOfChildrenPerNode; k++) {
                GNode* child = previousLevelNodes[j*numberOfChildrenPerNode+k];
                node->children.push_back(child);
                child->parent = node;
                for (int l = 0; l < child->borderStopIds.size(); l++) {
                    node->borderStopIds.push_back(child->borderStopIds[l]);
                }
            }

            for(int k = 0; k < node->borderStopIds.size(); k++) {
                vector<int> distances = originalGraph.getDistances(node->borderStopIds[k], node->borderStopIds);
                for (int l = 0; l < node->borderStopIds.size(); l++) {
                    node->borderDurations[make_pair(node->borderStopIds[k], node->borderStopIds[l])] = distances[node->borderStopIds[l]];
                }
            }

            currentLevelNodes.push_back(node);
        }

        previousLevelNodes = currentLevelNodes;
    }

    gTree.root = previousLevelNodes[0];

    return gTree;
}