#include "g-tree.h"

#include <iostream>
#include <vector>
#include <map>
#include <algorithm>
#include <limits.h>

using namespace std;

int GTree::getMinimalDurationToNode(int sourceStopId, int targetNodeId, map<pair<int, int>, vector<pair<int, int>>> &queryPointAndNodeToBorderStopDurations) {
    int minDuration = INT_MAX;
    
    vector<int> path = getNodePath(sourceStopId, targetNodeId);

    if (path.size() == 0) {
        return 0;
    }

    // fill the map with the border stop durations of the first node if it is not already filled
    pair<int, int> queryPointAndNode = make_pair(sourceStopId, path[0]);
    if (queryPointAndNodeToBorderStopDurations.find(queryPointAndNode) != queryPointAndNodeToBorderStopDurations.end()) {
        vector<pair<int, int>> distancesToBorderStops;
        for (int i = 0; i < nodeOfNodeId[path[0]]->borderStopIds.size(); i++) {
            int borderStopId = nodeOfNodeId[path[0]]->borderStopIds[i];
            int distanceToBorderStop = nodeOfStopId[sourceStopId]->borderDurations[make_pair(sourceStopId, borderStopId)];
            distancesToBorderStops.push_back(make_pair(distanceToBorderStop, borderStopId));
        }
    }

    // fill the map with the border stop durations of the other nodes on the path if they are not already filled
    int previousNodeId = path[0];
    for(int i = 1; i < path.size() - 1; i++) {
        int currentNodeId = path[i];
        pair<int, int> queryPointAndNode = make_pair(sourceStopId, currentNodeId);
        if (queryPointAndNodeToBorderStopDurations.find(queryPointAndNode) != queryPointAndNodeToBorderStopDurations.end()) {
            vector<pair<int, int>> distancesToBorderStopsOfPreviousNode = queryPointAndNodeToBorderStopDurations[make_pair(sourceStopId, previousNodeId)];
            vector<pair<int, int>> distancesToBorderStopsOfCurrentNode;
            int parentNodeId;
            // case 1: the node is a parent of the previous node
            if (nodeOfNodeId[previousNodeId]->parent->nodeId == currentNodeId) {
                parentNodeId = currentNodeId;
            }
            // case 2: the node is a sibling of the previous node
            else if (nodeOfNodeId[previousNodeId]->parent->nodeId == nodeOfNodeId[currentNodeId]->parent->nodeId) {
                int parentNodeId = nodeOfNodeId[currentNodeId]->parent->nodeId;
            }
            // case 3: the node is a child of the previous node
            else {
                int parentNodeId = previousNodeId;
            }

            for (int j = 0; j < nodeOfNodeId[currentNodeId]->borderStopIds.size(); j++) {
                int borderStopId = nodeOfNodeId[currentNodeId]->borderStopIds[j];
                int minDistanceToCurrentBorderStop = INT_MAX;
                for (int k = 0; k < distancesToBorderStopsOfPreviousNode.size(); k++) {
                    int distanceToPreviousBorderStop = distancesToBorderStopsOfPreviousNode[k].first;
                    int previousBorderStopId = distancesToBorderStopsOfPreviousNode[k].second;
                    int distanceBetweenBorderStops = nodeOfNodeId[parentNodeId]->borderDurations[make_pair(previousBorderStopId, borderStopId)];
                    int distanceToBorderStopOfCurrentNode = distanceToPreviousBorderStop + distanceBetweenBorderStops;
                    if (distanceToBorderStopOfCurrentNode < minDistanceToCurrentBorderStop) {
                        minDistanceToCurrentBorderStop = distanceToBorderStopOfCurrentNode;
                    }
                }
                distancesToBorderStopsOfCurrentNode.push_back(make_pair(minDistanceToCurrentBorderStop, borderStopId));
            }

            queryPointAndNodeToBorderStopDurations[queryPointAndNode] = distancesToBorderStopsOfCurrentNode;
        }
    }

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

int GTree::getMinimalDurationToStop(int sourceStopId, int targetStopId, map<pair<int, int>, vector<pair<int, int>>> &queryPointAndNodeToBorderStopDurations) {
    int minDuration = INT_MAX;

    if (nodeOfStopId[sourceStopId]->nodeId == nodeOfStopId[targetStopId]->nodeId) {
        return nodeOfStopId[sourceStopId]->borderDurations[make_pair(sourceStopId, targetStopId)];
    }

    pair<int, int> queryPointAndNode = make_pair(sourceStopId, nodeOfStopId[targetStopId]->nodeId);

    if (queryPointAndNodeToBorderStopDurations.find(queryPointAndNode) != queryPointAndNodeToBorderStopDurations.end()) {
        getMinimalDurationToNode(sourceStopId, nodeOfStopId[targetStopId]->nodeId, queryPointAndNodeToBorderStopDurations);
    }

    vector<pair<int, int>> distancesToBorderStops = queryPointAndNodeToBorderStopDurations[queryPointAndNode];

    for (int i = 0; i < distancesToBorderStops.size(); i++) {
        int distanceToBorderStop = distancesToBorderStops[i].first;
        int borderStopId = distancesToBorderStops[i].second;
        int distanceToTargetStop = nodeOfStopId[targetStopId]->borderDurations[make_pair(borderStopId, targetStopId)];
        int duration = distanceToBorderStop + distanceToTargetStop;

        if (duration < minDuration) {
            minDuration = duration;
        }
    }

    return minDuration;
}

vector<int> GTree::getNodePath(int stopId, int nodeId) {
    vector<int> path;
    vector<int> upwardPath;
    vector<int> downwardPath;

    upwardPath.push_back(nodeOfStopId[stopId]->nodeId);
    downwardPath.push_back(nodeId);

    while (upwardPath.back() != downwardPath.back()) {
        if(upwardPath.size() < downwardPath.size()) {
            downwardPath.push_back(nodeOfNodeId[downwardPath.back()]->parent->nodeId);
        } else {
            upwardPath.push_back(nodeOfNodeId[upwardPath.back()]->parent->nodeId);
        }
    }

    for (int i = 0; i < upwardPath.size() - 1; i++) {
        path.push_back(upwardPath[i]);
    }

    for (int i = downwardPath.size() - 2; i >= 0; i--) {
        path.push_back(downwardPath[i]);
    }

    return path;
}