#include "query-processor.h"

#include "csa.h"
#include <../data-structures/creator.h>
#include <../data-structures/g-tree.h>
#include <../constants.h>
#include <../data-handling/importer.h>
#include <../data-handling/converter.h>

#include <limits.h>
#include <omp.h>

#include <iostream>
#include <chrono>
#include <queue>
#include <map>

#include <vector>
#include <string>

using namespace std;

/*
    Process the naive query.
    For each source stop, calculate the earliest arrival time for all stops.
    Then calculate the sum of the earliest arrival times for all stops and the maximum earliest arrival time for all stops.
    The stop with the lowest sum and the stop with the highest maximum earliest arrival time are the meeting points.

*/
void NaiveQueryProcessor::processNaiveQuery() {
    for (CSA* csa : csas) {
        delete csa;
    }
    csas.clear();

    auto start = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < meetingPointQuery.sourceStopIds.size(); i++) {
        CSAQuery query;
        query.sourceStopId = meetingPointQuery.sourceStopIds[i];
        query.sourceTime = meetingPointQuery.sourceTime;
        query.weekday = meetingPointQuery.weekday;
        query.numberOfDays = meetingPointQuery.numberOfDays;
        CSA* csa = new CSA(query);
        csas.push_back(csa);
    }

    // Process the CSA algorithm for each source stop
    #pragma omp parallel for
    for (int i = 0; i < csas.size(); i++) {
        csas[i]->processCSA();
    }

    int minSum = INT_MAX;
    int minMax = INT_MAX;

    // Calculate the sum of the earliest arrival times for all stops and the maximum earliest arrival time for all stops
    for (int i = 0; i < Importer::stops.size(); i++) {
        int sum = 0;
        int max = 0;
        int arrivalTime = 0;
        for (int j = 0; j < meetingPointQuery.sourceStopIds.size(); j++) {
            int earliestArrivalTime = csas[j]->getEarliestArrivalTime(i);
            if (earliestArrivalTime == INT_MAX) {
                sum = INT_MAX;
                max = INT_MAX;
                break;
            }

            int duration = earliestArrivalTime - meetingPointQuery.sourceTime;
            sum += duration;
            if (duration > max) {
                max = duration;
                arrivalTime = earliestArrivalTime;
            }
        }

        if (sum < minSum) {
            meetingPointQueryResult.meetingPointMinSum = Importer::getStopName(i);
            meetingPointQueryResult.meetingTimeMinSum = TimeConverter::convertSecondsToTime(arrivalTime, true);
            meetingPointQueryResult.minSumDuration = TimeConverter::convertSecondsToTime(sum, false);
            meetingPointQueryResult.minSumDurationInSeconds = sum;
            minSum = sum;
        }
        if (max < minMax) {
            meetingPointQueryResult.meetingPointMinMax = Importer::getStopName(i);
            meetingPointQueryResult.meetingTimeMinMax = TimeConverter::convertSecondsToTime(arrivalTime, true);
            meetingPointQueryResult.minMaxDuration = TimeConverter::convertSecondsToTime(max, false);
            meetingPointQueryResult.minMaxDurationInSeconds = max;
            minMax = max;
        }
    }

    // Stop the timer and calculate the duration
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    meetingPointQueryResult.queryTime = duration;
}

/*
    Get the meeting point query result.
*/
MeetingPointQueryResult NaiveQueryProcessor::getMeetingPointQueryResult() {
    return meetingPointQueryResult;
}

/*
    Get the journeys to a meeting point.
*/
vector<Journey> NaiveQueryProcessor::getJourneys(Optimization optimization) {
    vector<Journey> journeys;
    int targetStopId;
    if (optimization == min_sum) {
        targetStopId = Importer::getStopId(meetingPointQueryResult.meetingPointMinSum);
    } else {
        targetStopId = Importer::getStopId(meetingPointQueryResult.meetingPointMinMax);
    }
    for (int i = 0; i < csas.size(); i++) {
        Journey journey = csas[i]->createJourney(targetStopId);
        journeys.push_back(journey);
    }
    return journeys;
}

/*
    Generate a random meeting point query.
*/
MeetingPointQuery QueryGenerator::generateRandomMeetingPointQuery(int numberOfSources, int numberOfDays) {
    MeetingPointQuery meetingPointQuery;
    for (int i = 0; i < numberOfSources; i++) {
        meetingPointQuery.sourceStopIds.push_back(rand() % Importer::stops.size());
    }
    meetingPointQuery.sourceTime = rand() % SECONDS_PER_DAY;
    meetingPointQuery.weekday = rand() % 7;
    meetingPointQuery.numberOfDays = numberOfDays;
    return meetingPointQuery;
}

/*
    Generate a meeting point query.
*/
MeetingPointQuery QueryGenerator::generateMeetingPointQuery(vector<string> sourceStopNames, string sourceTime, string weekday, int numberOfDays) {
    MeetingPointQuery meetingPointQuery;
    for (string sourceStopName : sourceStopNames) {
        meetingPointQuery.sourceStopIds.push_back(Importer::getStopId(sourceStopName));
    }
    meetingPointQuery.sourceTime = TimeConverter::convertTimeToSeconds(sourceTime);
    meetingPointQuery.weekday = WeekdayConverter::convertWeekdayToInt(weekday);
    meetingPointQuery.numberOfDays = numberOfDays;
    return meetingPointQuery;
}

/*
    Create a CSA query.
*/
CSAQuery QueryGenerator::createCSAQuery(string sourceStopName, string sourceTime, string weekday) {
    CSAQuery query;
    query.sourceStopId = Importer::getStopId(sourceStopName);
    query.sourceTime = TimeConverter::convertTimeToSeconds(sourceTime);
    query.weekday = WeekdayConverter::convertWeekdayToInt(weekday);
    return query;
}

/*
    Create a CSA query with target stops.
*/
CSAQuery QueryGenerator::createCSAQueryWithTargetStops(string sourceStopName, vector<string> targetStopNames, string sourceTime, string weekday) {
    CSAQuery query;
    query.sourceStopId = Importer::getStopId(sourceStopName);
    for (string targetStopName : targetStopNames) {
        query.targetStopIds.push_back(Importer::getStopId(targetStopName));
    }
    return query;
}

/*
    Process a G-tree query. If useCSA is true, the CSA algorithm is used to calculate the real durations during the algorithm. 
    Otherwise, approximate durations are calculated. In this case are the real durations calculated after the algorithm.
*/
void GTreeQueryProcessor::processGTreeQuery(bool useCSA) {
    for (CSA* csa : csas) {
        delete csa;
    }
    csas.clear();

    for (int i = 0; i < meetingPointQuery.sourceStopIds.size(); i++) {
        CSAQuery query;
        query.sourceStopId = meetingPointQuery.sourceStopIds[i];
        query.sourceTime = meetingPointQuery.sourceTime;
        query.weekday = meetingPointQuery.weekday;
        query.numberOfDays = meetingPointQuery.numberOfDays;
        CSA* csa = new CSA(query);
        csas.push_back(csa);
    }

    queryPointAndNodeToBorderStopDurations = map<pair<int, int>, vector<pair<int, int>>>();

    auto start = std::chrono::high_resolution_clock::now();

    // Process the query for both optimizations
    processGTreeQueryWithOptimization(min_sum, useCSA);
    processGTreeQueryWithOptimization(min_max, useCSA);

    // Stop the timer and calculate the duration
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

    meetingPointQueryResult.queryTime = duration;

    if (meetingPointQueryResult.meetingPointMinSum != "" || meetingPointQueryResult.meetingPointMinMax != "") {
        // Calculate the real durations to the meeting point
        int meetingPointMinSumStopId = Importer::getStopId(meetingPointQueryResult.meetingPointMinSum);
        int meetingPointMinMaxStopId = Importer::getStopId(meetingPointQueryResult.meetingPointMinMax);

        vector<int> targetStopIds = vector<int>(1, meetingPointMinSumStopId);
        if (meetingPointMinSumStopId != meetingPointMinMaxStopId){
            targetStopIds.push_back(meetingPointMinMaxStopId);
        }

        int meetingPointMinSumDuration = 0;
        int meetingPointMinMaxDuration = 0;

        int meetingPointMinSumArrivalTime = 0;
        int meetingPointMinMaxArrivalTime = 0;

        // Calculate the real durations
        if(!useCSA) {
            for(int i = 0; i < csas.size(); i++){
                csas[i]->setTargetStopIds(targetStopIds);
                csas[i]->processCSA();
            }
        }

        // Set the values of the result
        for (int i = 0; i < meetingPointQuery.sourceStopIds.size(); i++) {
            int sourceStopId = meetingPointQuery.sourceStopIds[i];

            meetingPointMinSumDuration += (csas[i]->getEarliestArrivalTime(meetingPointMinSumStopId) - meetingPointQuery.sourceTime);
            if (meetingPointMinSumArrivalTime < csas[i]->getEarliestArrivalTime(meetingPointMinSumStopId)) {
                meetingPointMinSumArrivalTime = csas[i]->getEarliestArrivalTime(meetingPointMinSumStopId);
            }

            int minMaxDuration = csas[i]->getEarliestArrivalTime(meetingPointMinMaxStopId) - meetingPointQuery.sourceTime;
            if (meetingPointMinMaxDuration < minMaxDuration) {
                meetingPointMinMaxDuration = minMaxDuration;
                meetingPointMinMaxArrivalTime = csas[i]->getEarliestArrivalTime(meetingPointMinMaxStopId);
            }
        }

        meetingPointQueryResult.minSumDuration = TimeConverter::convertSecondsToTime(meetingPointMinSumDuration, false);
        meetingPointQueryResult.minSumDurationInSeconds = meetingPointMinSumDuration;
        meetingPointQueryResult.meetingTimeMinSum = TimeConverter::convertSecondsToTime(meetingPointMinSumArrivalTime, true);

        meetingPointQueryResult.minMaxDuration = TimeConverter::convertSecondsToTime(meetingPointMinMaxDuration, false);
        meetingPointQueryResult.minMaxDurationInSeconds = meetingPointMinMaxDuration;
        meetingPointQueryResult.meetingTimeMinMax = TimeConverter::convertSecondsToTime(meetingPointMinMaxArrivalTime, true);
    }
}

/*
    Process a G-tree query for a given optimization criteria. If useCSA is true, the CSA algorithm is used to calculate the real durations during the algorithm.
*/
void GTreeQueryProcessor::processGTreeQueryWithOptimization(Optimization optimization, bool useCSA) {
    priority_queue<pair<int, int>, vector<pair<int, int>>, greater<pair<int, int>>> pq;
    int optimalMeetingPointStopId = -1;
    int currentBest = INT_MAX;

    // Calculate the initial lower bound of the root node
    int l = getLowerBoundToNode(gTree->root->nodeId, queryPointAndNodeToBorderStopDurations, optimization);
    pq.push(make_pair(l, gTree->root->nodeId));

    // Process the query until the priority queue is empty or the current lower bound is greater than the current best
    while(!pq.empty()){
        pair<int, int> current = pq.top();
        pq.pop();
        int currentLowerBound = current.first;
        int currentNodeId = current.second;

        if(currentLowerBound >= currentBest){
            break;
        }

        GNode* currentNode = gTree->nodeOfNodeId[currentNodeId];
        // Case 1: The current node is a inner node. Calculate the lower bounds of the children and add them to the priority queue.
        if(currentNode->children.size() > 0){
            for(int i = 0; i < currentNode->children.size(); i++){
                GNode* childNode = currentNode->children[i];
                int childNodeId = childNode->nodeId;
                int childLowerBound = getLowerBoundToNode(childNodeId, queryPointAndNodeToBorderStopDurations, optimization);
                if (childLowerBound < currentBest) {
                    pq.push(make_pair(childLowerBound, childNodeId));
                }
            }
        } 
        // Case 2: The current node is a leaf node. Calculate the costs to the stops and update the current best if necessary.
        else {
            if(useCSA) {
                processCSAToTargetStops(currentNode->stopIds, currentBest);
            }
            for (int i = 0; i < currentNode->stopIds.size(); i++) {
                int stopId = currentNode->stopIds[i];
                int costs = getCostsToStop(stopId, queryPointAndNodeToBorderStopDurations, optimization, useCSA);
                if (costs < currentBest) {
                    currentBest = costs;
                    optimalMeetingPointStopId = stopId;
                }
            }
        }
    }
    
    // Set the optimal meeting point stop name
    string optimalMeetingPointStopName = "";
    if(optimalMeetingPointStopId != -1){
        optimalMeetingPointStopName = Importer::getStopName(optimalMeetingPointStopId);
    }
    if (optimization == min_sum) {
        meetingPointQueryResult.meetingPointMinSum = optimalMeetingPointStopName;
    } else {
        meetingPointQueryResult.meetingPointMinMax = optimalMeetingPointStopName;
    }
}

/*
    Get the lower bound to a node.
*/
int GTreeQueryProcessor::getLowerBoundToNode(int nodeId, map<pair<int, int>, vector<pair<int, int>>> &queryPointAndNodeToBorderStopDurations, Optimization optimization) {
    int lowerBound = 0;
    for (int i = 0; i < meetingPointQuery.sourceStopIds.size(); i++) {
        int duration = gTree->getMinimalDurationToNode(meetingPointQuery.sourceStopIds[i], nodeId, queryPointAndNodeToBorderStopDurations);
        if (duration == INT_MAX) {
            return INT_MAX;
        }
        if (optimization == min_sum) {
            lowerBound += duration;
        } else {
            if (duration > lowerBound) {
                lowerBound = duration;
            }
        }
    }
    return lowerBound;
}

/*
    Get the costs to a stop.
*/
int GTreeQueryProcessor::getCostsToStop(int stopId, map<pair<int, int>, vector<pair<int, int>>> &queryPointAndNodeToBorderStopDurations, Optimization optimization, bool useCSA) {
    int costs = 0;
    for (int i = 0; i < meetingPointQuery.sourceStopIds.size(); i++) {
        int duration = INT_MAX;
        if (useCSA) {
            int earliestArrivalTime = csas[i]->getEarliestArrivalTime(stopId);
            if (earliestArrivalTime != INT_MAX) {
                duration = earliestArrivalTime - meetingPointQuery.sourceTime;
            }
        } else {
            duration = gTree->getMinimalDurationToStop(meetingPointQuery.sourceStopIds[i], stopId, queryPointAndNodeToBorderStopDurations);
        }
        if (duration == INT_MAX) {
            return INT_MAX;
        }
        if (optimization == min_sum) {
            costs += duration;
        } else {
            if (duration > costs) {
                costs = duration;
            }
        }
    }
    return costs;
}

/*
    Process the CSA algorithm for the target stops.
*/
void GTreeQueryProcessor::processCSAToTargetStops(vector<int> targetStopIds, int currentBest) {
    for (int i = 0; i < csas.size(); i++) {
        if (currentBest != INT_MAX) {
            int maxDepartureTime = meetingPointQuery.sourceTime + currentBest;
            csas[i]->setMaxDepartureTime(maxDepartureTime);
        }
        csas[i]->setTargetStopIds(targetStopIds);
        csas[i]->processCSA();
    }
}

/*
    Get the meeting point query result.
*/
MeetingPointQueryResult GTreeQueryProcessor::getMeetingPointQueryResult() {
    return meetingPointQueryResult;
}

/*
    Get the journeys to the meeting points.
*/
vector<Journey> GTreeQueryProcessor::getJourneys(Optimization optimization) {
    vector<Journey> journeys;
    int targetStopId;
    if (optimization == min_sum) {
        targetStopId = Importer::getStopId(meetingPointQueryResult.meetingPointMinSum);
    } else {
        targetStopId = Importer::getStopId(meetingPointQueryResult.meetingPointMinMax);
    }
    for (int i = 0; i < csas.size(); i++) {
        Journey journey = csas[i]->createJourney(targetStopId);
        journeys.push_back(journey);
    }
    return journeys;
}