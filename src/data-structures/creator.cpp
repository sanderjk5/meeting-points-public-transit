#include <creator.h>

#include <../data-handling/importer.h>
#include <../constants.h>

Graph Creator::networkGraph = Graph();
GTree Creator::networkGTree = GTree();

void Creator::createNetworkGraph() {
    networkGraph.adjacencyList = vector<vector<Edge>>(Importer::stops.size(), vector<Edge>(0));

    for (int i = 0; i < Importer::connections.size(); i++) {
        Connection connection = Importer::connections[i];
        int connectionArrivalTime = connection.arrivalTime;
        if (connectionArrivalTime < connection.departureTime) {
            connectionArrivalTime += SECONDSPERDAY;
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
}


void Creator::createNetworkGTree(int numberOfChildrenPerNode, int numberOfVerticesPerLeaf) {
    // TODO: implement
}

vector<Graph> Creator::partitionateGraph(Graph &graph, int numberOfPartitions) {
    // TODO: implement
    return vector<Graph>();
}

void Creator::coarseGraph(Graph &graph) {
    // TODO: implement
}

vector<Graph> Creator::partitionateCoarsedGraph(Graph &graph) {
    // TODO: implement
    return vector<Graph>();
}

void Creator::refineGraphs(vector<Graph> &graphs) {
    // TODO: implement
}

GTree Creator::createGTree(vector<Graph> &graphs, int depth) {
    // TODO: implement
    return GTree();
}

void Creator::setBorderDurations(GTree &gTree) {
    // TODO: implement
}