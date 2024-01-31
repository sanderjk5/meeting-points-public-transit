#include "query-processor.h"

#include "csa.h"
#include <../data-structures/creator.h>
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

void NaiveQueryProcessor::processNaiveQuery(bool printTime) {
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

    #pragma omp parallel for
    for (int i = 0; i < csas.size(); i++) {
        csas[i]->processCSA(printTime);
    }

    int minSum = INT_MAX;
    int minMax = INT_MAX;

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
            minSum = sum;
        }
        if (max < minMax) {
            meetingPointQueryResult.meetingPointMinMax = Importer::getStopName(i);
            meetingPointQueryResult.meetingTimeMinMax = TimeConverter::convertSecondsToTime(arrivalTime, true);
            meetingPointQueryResult.minMaxDuration = TimeConverter::convertSecondsToTime(max, false);
            minMax = max;
        }
    }

    // Stop the timer and calculate the duration
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

    // Print the duration
    if (printTime) {
        cout << "Naive Query duration : " << duration << " milliseconds" << endl;
    }

    meetingPointQueryResult.queryTime = to_string(duration);
}

MeetingPointQueryResult NaiveQueryProcessor::getMeetingPointQueryResult() {
    return meetingPointQueryResult;
}

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

MeetingPointQuery QueryProcessor::generateRandomMeetingPointQuery(int numberOfSources, int numberOfDays) {
    MeetingPointQuery meetingPointQuery;
    for (int i = 0; i < numberOfSources; i++) {
        meetingPointQuery.sourceStopIds.push_back(rand() % Importer::stops.size());
    }
    meetingPointQuery.sourceTime = rand() % SECONDS_PER_DAY;
    meetingPointQuery.weekday = rand() % 7;
    meetingPointQuery.numberOfDays = numberOfDays;
    return meetingPointQuery;
}

MeetingPointQuery QueryProcessor::generateMeetingPointQuery(vector<string> sourceStopNames, string sourceTime, string weekday, int numberOfDays) {
    MeetingPointQuery meetingPointQuery;
    for (string sourceStopName : sourceStopNames) {
        meetingPointQuery.sourceStopIds.push_back(Importer::getStopId(sourceStopName));
    }
    meetingPointQuery.sourceTime = TimeConverter::convertTimeToSeconds(sourceTime);
    meetingPointQuery.weekday = WeekdayConverter::convertWeekdayToInt(weekday);
    meetingPointQuery.numberOfDays = numberOfDays;
    return meetingPointQuery;
}

CSAQuery QueryProcessor::createCSAQuery(string sourceStopName, string sourceTime, string weekday) {
    CSAQuery query;
    query.sourceStopId = Importer::getStopId(sourceStopName);
    query.sourceTime = TimeConverter::convertTimeToSeconds(sourceTime);
    query.weekday = WeekdayConverter::convertWeekdayToInt(weekday);
    return query;
}

CSAQuery QueryProcessor::createCSAQueryWithTargetStops(string sourceStopName, vector<string> targetStopNames, string sourceTime, string weekday) {
    CSAQuery query;
    query.sourceStopId = Importer::getStopId(sourceStopName);
    for (string targetStopName : targetStopNames) {
        query.targetStopIds.push_back(Importer::getStopId(targetStopName));
    }
    return query;
}

void GTreeQueryProcessor::processGTreeQuery(bool printTime) {
    queryPointAndNodeToBorderStopDurations = map<pair<int, int>, vector<pair<int, int>>>();

    auto start = std::chrono::high_resolution_clock::now();

    processGTreeQueryWithOptimization(min_sum);
    processGTreeQueryWithOptimization(min_max);

    // Stop the timer and calculate the duration
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

    meetingPointQueryResult.queryTime = to_string(duration);

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

    for (int i = 0; i < meetingPointQuery.sourceStopIds.size(); i++) {
        int sourceStopId = meetingPointQuery.sourceStopIds[i];
        
        CSAQuery query;
        query.sourceStopId = sourceStopId;
        query.targetStopIds = targetStopIds;
        query.sourceTime = meetingPointQuery.sourceTime;
        query.weekday = meetingPointQuery.weekday;
        query.numberOfDays = meetingPointQuery.numberOfDays;

        CSA csa(query);
        csa.processCSA(false);

        meetingPointMinSumDuration += csa.getEarliestArrivalTime(meetingPointMinSumStopId) - meetingPointQuery.sourceTime;
        if (meetingPointMinSumArrivalTime < csa.getEarliestArrivalTime(meetingPointMinSumStopId)) {
            meetingPointMinSumArrivalTime = csa.getEarliestArrivalTime(meetingPointMinSumStopId);
        }

        int minMaxDuration = csa.getEarliestArrivalTime(meetingPointMinMaxStopId) - meetingPointQuery.sourceTime;
        if (meetingPointMinMaxDuration < minMaxDuration) {
            meetingPointMinMaxDuration = minMaxDuration;
            meetingPointMinMaxArrivalTime = csa.getEarliestArrivalTime(meetingPointMinMaxStopId);
        }
    }

    meetingPointQueryResult.minSumDuration = TimeConverter::convertSecondsToTime(meetingPointMinSumDuration, false);
    meetingPointQueryResult.meetingTimeMinSum = TimeConverter::convertSecondsToTime(meetingPointMinSumArrivalTime, true);

    meetingPointQueryResult.minMaxDuration = TimeConverter::convertSecondsToTime(meetingPointMinMaxDuration, false);
    meetingPointQueryResult.meetingTimeMinMax = TimeConverter::convertSecondsToTime(meetingPointMinMaxArrivalTime, true);

    // Print the duration
    if (printTime) {
        cout << "GTree Query duration : " << duration << " milliseconds" << endl;
    }
}

void GTreeQueryProcessor::processGTreeQueryWithOptimization(Optimization optimization) {
    priority_queue<pair<int, int>, vector<pair<int, int>>, greater<pair<int, int>>> pq;
    int optimalMeetingPointStopId = -1;
    int currentBest = INT_MAX;

    int l = getLowerBoundToNode(Creator::networkGTree.root->nodeId, queryPointAndNodeToBorderStopDurations, optimization);
    pq.push(make_pair(l, Creator::networkGTree.root->nodeId));

    while(!pq.empty()){
        pair<int, int> current = pq.top();
        pq.pop();
        int currentLowerBound = current.first;
        int currentNodeId = current.second;

        if(currentLowerBound >= currentBest){
            continue;
        }

        GNode* currentNode = Creator::networkGTree.nodeOfNodeId[currentNodeId];
        if(currentNode->children.size() > 0){
            for(int i = 0; i < currentNode->children.size(); i++){
                GNode* childNode = currentNode->children[i];
                int childNodeId = childNode->nodeId;
                int childLowerBound = getLowerBoundToNode(childNodeId, queryPointAndNodeToBorderStopDurations, optimization);
                if (childLowerBound < currentBest) {
                    pq.push(make_pair(childLowerBound, childNodeId));
                }
            }
        } else {
            for (int i = 0; i < currentNode->stopIds.size(); i++) {
                int stopId = currentNode->stopIds[i];
                int costs = getCostsToStop(stopId, queryPointAndNodeToBorderStopDurations, optimization);
                if (costs < currentBest) {
                    currentBest = costs;
                    optimalMeetingPointStopId = stopId;
                }
            }
        }
    }

    if (optimization == min_sum) {
        meetingPointQueryResult.meetingPointMinSum = Importer::getStopName(optimalMeetingPointStopId);
    } else {
        meetingPointQueryResult.meetingPointMinMax = Importer::getStopName(optimalMeetingPointStopId);
    }
}

int GTreeQueryProcessor::getLowerBoundToNode(int nodeId, map<pair<int, int>, vector<pair<int, int>>> &queryPointAndNodeToBorderStopDurations, Optimization optimization) {
    int lowerBound = 0;
    for (int i = 0; i < meetingPointQuery.sourceStopIds.size(); i++) {
        int duration = Creator::networkGTree.getMinimalDurationToNode(meetingPointQuery.sourceStopIds[i], nodeId, queryPointAndNodeToBorderStopDurations);
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

int GTreeQueryProcessor::getCostsToStop(int stopId, map<pair<int, int>, vector<pair<int, int>>> &queryPointAndNodeToBorderStopDurations, Optimization optimization) {
    int costs = 0;
    // for (int i = 0; i < meetingPointQuery.sourceStopIds.size(); i++) {
    //     int duration = Creator::networkGTree.getMinimalDurationToStop(meetingPointQuery.sourceStopIds[i], stopId, queryPointAndNodeToBorderStopDurations);
    //     if (optimization == min_sum) {
    //         costs += duration;
    //     } else {
    //         if (duration > costs) {
    //             costs = duration;
    //         }
    //     }
    // }
    return costs;
}

MeetingPointQueryResult GTreeQueryProcessor::getMeetingPointQueryResult() {
    return meetingPointQueryResult;
}