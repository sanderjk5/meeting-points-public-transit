#include "query-processor.h"

#include "csa.h"
#include "raptor.h"
#include <../data-structures/creator.h>
#include <../data-structures/g-tree.h>
#include <../constants.h>
#include <../data-handling/importer.h>
#include <../data-handling/converter.h>

#include <limits.h>
#include <omp.h>

#include <iostream>
#include <sstream>
#include <fstream>
#include <chrono>
#include <queue>
#include <map>
#include <algorithm>

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

    int stopIdMinSum = -1;
    int stopIdMinMax = -1;

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
            meetingPointQueryResult.meetingPointMinSumStopId = i;
            meetingPointQueryResult.meetingPointMinSum = Importer::getStopName(i);
            meetingPointQueryResult.meetingTimeMinSum = TimeConverter::convertSecondsToTime(arrivalTime, true);
            meetingPointQueryResult.minSumDuration = TimeConverter::convertSecondsToTime(sum, false);
            meetingPointQueryResult.minSumDurationInSeconds = sum;
            stopIdMinSum = i;
            minSum = sum;
        }
        if (max < minMax) {
            meetingPointQueryResult.meetingPointMinMaxStopId = i;
            meetingPointQueryResult.meetingPointMinMax = Importer::getStopName(i);
            meetingPointQueryResult.meetingTimeMinMax = TimeConverter::convertSecondsToTime(arrivalTime, true);
            meetingPointQueryResult.minMaxDuration = TimeConverter::convertSecondsToTime(max, false);
            meetingPointQueryResult.minMaxDurationInSeconds = max;
            stopIdMinMax = i;
            minMax = max;
        }
    }

    if (stopIdMinSum != -1 && stopIdMinMax != -1) {
        int maxTransfersMinSum = 0;
        int maxTransfersMinMax = 0;
        for (int i = 0; i < csas.size(); i++) {
            Journey journeyMinSum = csas[i]->createJourney(stopIdMinSum);
            Journey journeyMinMax = csas[i]->createJourney(stopIdMinMax);

            if (journeyMinSum.legs.size() > maxTransfersMinSum) {
                maxTransfersMinSum = journeyMinSum.legs.size();
            }

            if (journeyMinMax.legs.size() > maxTransfersMinMax) {
                maxTransfersMinMax = journeyMinMax.legs.size();
            }
        }
        meetingPointQueryResult.maxTransfersMinSum = maxTransfersMinSum;
        meetingPointQueryResult.maxTransfersMinMax = maxTransfersMinMax;
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
        targetStopId = meetingPointQueryResult.meetingPointMinSumStopId;
    } else {
        targetStopId = meetingPointQueryResult.meetingPointMinMaxStopId;
    }
    for (int i = 0; i < csas.size(); i++) {
        Journey journey = csas[i]->createJourney(targetStopId);
        journeys.push_back(journey);
    }
    return journeys;
}

/*
    Get the stops with a given accuracy.
    Calculate the sum of the earliest arrival times for all source stops and the maximum earliest arrival time for all source stops.
    Then calculate the relative difference between the sum and the maximum and the real sum and the real maximum. Use these values to calculate the accuracy.
    If the accuracy is higher than the accuracy bound, the stop is added to the result.
*/
vector<int> NaiveQueryProcessor::getStopsWithGivenAccuracy(double accuracyBound) {
    vector<int> meetingPointsOverAccuracy = vector<int>(0);
    for (int i = 0; i < Importer::stops.size(); i++) {
        int sum = 0;
        int max = 0;
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
            }            
        }
        if (sum == INT_MAX || max == INT_MAX) {
            continue;
        }

        int differenceMinSum = sum - meetingPointQueryResult.minSumDurationInSeconds;
        int differenceMinMax = max - meetingPointQueryResult.minMaxDurationInSeconds;

        double relativeDifferenceMinSum = (double) differenceMinSum / sum;
        double relativeDifferenceMinMax = (double) differenceMinMax / max;

        double accuracyMinSum = 1 - relativeDifferenceMinSum;
        double accuracyMinMax = 1 - relativeDifferenceMinMax;

        if (accuracyMinSum > accuracyBound && accuracyMinMax > accuracyBound) {
            meetingPointsOverAccuracy.push_back(Importer::stops[i].id);
        }
    }

    return meetingPointsOverAccuracy;
}

/*
    Find the key stops for a given number of source stops. Execute a number of queries and find the stops that are the most common meeting points.
*/
void NaiveKeyStopQueryProcessor::findKeyStops(DataType dataType, vector<int> numberOfSourceStopsVec, int numberOfQueries, int numberOfKeyStops, double accuracyBound){
    for (int i = 0; i < numberOfSourceStopsVec.size(); i++) {
        int numberOfSourceStops = numberOfSourceStopsVec[i];
        cout << "Finding key stops for " << numberOfSourceStops << " source stops..." << endl;
    
        map<int, int> stopIdToMeetingPointCounter = map<int, int>();

        int successfulQueries = 0;

        while(successfulQueries < numberOfQueries) {
            MeetingPointQuery randomMeetingPointQuery = QueryGenerator::generateRandomMeetingPointQuery(numberOfSourceStops);
            NaiveQueryProcessor naiveQueryProcessor = NaiveQueryProcessor(randomMeetingPointQuery);
            naiveQueryProcessor.processNaiveQuery();
            MeetingPointQueryResult meetingPointQueryResult = naiveQueryProcessor.getMeetingPointQueryResult();

            if (meetingPointQueryResult.meetingPointMinSum != "" && meetingPointQueryResult.meetingPointMinMax != "") {
                successfulQueries++;
            } else {
                continue;
            }
            
            vector<int> meetingPointStopIds = naiveQueryProcessor.getStopsWithGivenAccuracy(accuracyBound);

            for (int k = 0; k < meetingPointStopIds.size(); k++) {
                int meetingPointStopId = meetingPointStopIds[k];
                if (stopIdToMeetingPointCounter.find(meetingPointStopId) == stopIdToMeetingPointCounter.end()) {
                    stopIdToMeetingPointCounter[meetingPointStopId] = 1;
                } else {
                    stopIdToMeetingPointCounter[meetingPointStopId] += 1;
                }
            }
        }

        vector<pair<int, int>> counterStopIdPairs = vector<pair<int, int>>(0);

        for (auto mapEntry : stopIdToMeetingPointCounter) {
            counterStopIdPairs.push_back(make_pair(mapEntry.second, mapEntry.first));
        }

        sort(counterStopIdPairs.begin(), counterStopIdPairs.end(), greater<pair<int, int>>());

        // Write the key stops to a file
        string dataTypeString = Importer::getDataTypeString(dataType);
        string folderPathKeyStops = FOLDER_PREFIX + "tests/" + dataTypeString + "/key_stops/";
        string numberOfSourceStopsString = "";
        if (numberOfSourceStops < 10) {
            numberOfSourceStopsString = "00" + to_string(numberOfSourceStops);
        } else if (numberOfSourceStops < 100) {
            numberOfSourceStopsString = "0" + to_string(numberOfSourceStops);
        } else {
            numberOfSourceStopsString = to_string(numberOfSourceStops);
        }

        string filePath = folderPathKeyStops + "key_stops-" + numberOfSourceStopsString + ".csv";

        std::ofstream keyStopsFile;
        keyStopsFile.open(filePath, std::ofstream::out);

        for (int j = 0; j < numberOfKeyStops; j++) {
            keyStopsFile << counterStopIdPairs[j].second << "," << Importer::getStopName(counterStopIdPairs[j].second) << "\n";
        }

        keyStopsFile.close();

        cout << "Found " << numberOfKeyStops << " key stops." << endl;
    }
}

/*
    Get the key stops for a given number of source stops from a file if it exists.
*/
vector<int> NaiveKeyStopQueryProcessor::getKeyStops(DataType dataType, int numberOfSourceStops) {
    vector<int> keyStops = vector<int>(0);

    int currentNumberOfSourceStops = numberOfSourceStops;

    while (currentNumberOfSourceStops > 1) {
        string dataTypeString = Importer::getDataTypeString(dataType);
        string folderPathKeyStops = FOLDER_PREFIX + "tests/" + dataTypeString + "/key_stops/";
        string currentNumberOfSourceStopsString = "";
        if (currentNumberOfSourceStops < 10) {
            currentNumberOfSourceStopsString = "00" + to_string(currentNumberOfSourceStops);
        } else if (currentNumberOfSourceStops < 100) {
            currentNumberOfSourceStopsString = "0" + to_string(currentNumberOfSourceStops);
        } else {
            currentNumberOfSourceStopsString = to_string(currentNumberOfSourceStops);
        }

        string filePath = folderPathKeyStops + "key_stops-" + currentNumberOfSourceStopsString + ".csv";
        
        std::ifstream file(filePath);
        if (file.is_open()) {
            std::string line;
            while (std::getline(file, line)) {
                vector<string> parts;

                // Split the line into substrings
                std::stringstream ss(line);
                std::string substring;
                while (std::getline(ss, substring, ',')) {
                    parts.push_back(substring);
                }

                keyStops.push_back(stoi(parts[0]));
            }

            break;
        } 

        currentNumberOfSourceStops--;
    }

    if (keyStops.size() == 0) {
        cout << "Couldn't find key stops for the dataset." << endl;
    }

    return keyStops;
}

/*
    Process the naive key stop query.
    For each source stop, calculate the earliest arrival time for all key stop.
    Then calculate the sum of the earliest arrival times for all stops and the maximum earliest arrival time for all stops.
    The stop with the lowest sum and the stop with the highest maximum earliest arrival time are the meeting points.

*/
void NaiveKeyStopQueryProcessor::processNaiveKeyStopQuery(vector<int> keyStops) {
    for (CSA* csa : csas) {
        delete csa;
    }
    csas.clear();

    auto start = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < meetingPointQuery.sourceStopIds.size(); i++) {
        CSAQuery query;
        query.sourceStopId = meetingPointQuery.sourceStopIds[i];
        query.targetStopIds = keyStops;
        query.sourceTime = meetingPointQuery.sourceTime;
        query.weekday = meetingPointQuery.weekday;
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

    int stopIdMinSum = -1;
    int stopIdMinMax = -1;

    // Calculate the sum of the earliest arrival times for all stops and the maximum earliest arrival time for all stops
    for (int i = 0; i < keyStops.size(); i++) {
        int stopId = keyStops[i];
        int sum = 0;
        int max = 0;
        int arrivalTime = 0;
        for (int j = 0; j < meetingPointQuery.sourceStopIds.size(); j++) {
            int earliestArrivalTime = csas[j]->getEarliestArrivalTime(stopId);
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
            meetingPointQueryResult.meetingPointMinSumStopId = stopId;
            meetingPointQueryResult.meetingPointMinSum = Importer::getStopName(stopId);
            meetingPointQueryResult.meetingTimeMinSum = TimeConverter::convertSecondsToTime(arrivalTime, true);
            meetingPointQueryResult.minSumDuration = TimeConverter::convertSecondsToTime(sum, false);
            meetingPointQueryResult.minSumDurationInSeconds = sum;
            stopIdMinSum = stopId;
            minSum = sum;
        }
        if (max < minMax) {
            meetingPointQueryResult.meetingPointMinMaxStopId = stopId;
            meetingPointQueryResult.meetingPointMinMax = Importer::getStopName(stopId);
            meetingPointQueryResult.meetingTimeMinMax = TimeConverter::convertSecondsToTime(arrivalTime, true);
            meetingPointQueryResult.minMaxDuration = TimeConverter::convertSecondsToTime(max, false);
            meetingPointQueryResult.minMaxDurationInSeconds = max;
            stopIdMinMax = stopId;
            minMax = max;
        }
    }

    if (stopIdMinSum != -1 && stopIdMinMax != -1) {
        int maxTransfersMinSum = 0;
        int maxTransfersMinMax = 0;
        for (int i = 0; i < csas.size(); i++) {
            Journey journeyMinSum = csas[i]->createJourney(stopIdMinSum);
            Journey journeyMinMax = csas[i]->createJourney(stopIdMinMax);

            if (journeyMinSum.legs.size() > maxTransfersMinSum) {
                maxTransfersMinSum = journeyMinSum.legs.size();
            }

            if (journeyMinMax.legs.size() > maxTransfersMinMax) {
                maxTransfersMinMax = journeyMinMax.legs.size();
            }
        }
        meetingPointQueryResult.maxTransfersMinSum = maxTransfersMinSum;
        meetingPointQueryResult.maxTransfersMinMax = maxTransfersMinMax;
    }

    // Stop the timer and calculate the duration
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    meetingPointQueryResult.queryTime = duration;
}

/*
    Get the meeting point query result.
*/
MeetingPointQueryResult NaiveKeyStopQueryProcessor::getMeetingPointQueryResult() {
    return meetingPointQueryResult;
}

/*
    Get the journeys to a meeting point.
*/
vector<Journey> NaiveKeyStopQueryProcessor::getJourneys(Optimization optimization) {
    vector<Journey> journeys;
    int targetStopId;
    if (optimization == min_sum) {
        targetStopId = meetingPointQueryResult.meetingPointMinSumStopId;
    } else {
        targetStopId = meetingPointQueryResult.meetingPointMinMaxStopId;
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
MeetingPointQuery QueryGenerator::generateRandomMeetingPointQuery(int numberOfSources) {
    MeetingPointQuery meetingPointQuery;
    for (int i = 0; i < numberOfSources; i++) {
        meetingPointQuery.sourceStopIds.push_back(rand() % Importer::stops.size());
    }
    meetingPointQuery.sourceTime = rand() % SECONDS_PER_DAY;
    meetingPointQuery.weekday = rand() % 7;
    return meetingPointQuery;
}

/*
    Generate a meeting point query.
*/
MeetingPointQuery QueryGenerator::generateMeetingPointQuery(vector<string> sourceStopNames, string sourceTime, string weekday) {
    MeetingPointQuery meetingPointQuery;
    for (string sourceStopName : sourceStopNames) {
        meetingPointQuery.sourceStopIds.push_back(Importer::getStopId(sourceStopName));
    }
    meetingPointQuery.sourceTime = TimeConverter::convertTimeToSeconds(sourceTime);
    meetingPointQuery.weekday = WeekdayConverter::convertWeekdayToInt(weekday);
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
    Parse a meeting point query.
*/
MeetingPointQuery QueryGenerator::parseMeetingPointQuery(string line, int numberOfSourceStops) {
    MeetingPointQuery meetingPointQuery;
    vector<string> parts;

    // Split the line into substrings
    std::stringstream ss(line);
    std::string substring;
    while (std::getline(ss, substring, ',')) {
        parts.push_back(substring);
    }

    for (int i = 0; i < numberOfSourceStops; i++) {
        meetingPointQuery.sourceStopIds.push_back(stoi(parts[i]));
    }
    meetingPointQuery.sourceTime = stoi(parts[numberOfSourceStops]);
    meetingPointQuery.weekday = stoi(parts[numberOfSourceStops + 1]);
    return meetingPointQuery;
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
        CSA* csa = new CSA(query);
        csas.push_back(csa);
    }

    queryPointAndNodeToBorderStopDurations = vector<map<int, vector<pair<int, int>>>>(meetingPointQuery.sourceStopIds.size(), map<int, vector<pair<int, int>>>());

    auto start = std::chrono::high_resolution_clock::now();

    // Process the query for both optimizations
    processGTreeQueryWithOptimization(min_sum, useCSA);
    processGTreeQueryWithOptimization(min_max, useCSA);
    
    // Stop the timer and calculate the duration
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

    int totalNumberOfNodes = gTree->nodeOfNodeId.size() * meetingPointQuery.sourceStopIds.size();
    int visitedNodes = 0;
    for (int i = 0; i < meetingPointQuery.sourceStopIds.size(); i++) {
        visitedNodes += queryPointAndNodeToBorderStopDurations[i].size();
    }
    visitedNodesAvgFraction = (double) visitedNodes / totalNumberOfNodes;

    meetingPointQueryResult.queryTime = duration;

    if (meetingPointQueryResult.meetingPointMinSum != "" || meetingPointQueryResult.meetingPointMinMax != "") {
        // Calculate the real durations to the meeting point
        int meetingPointMinSumStopId = meetingPointQueryResult.meetingPointMinSumStopId;
        int meetingPointMinMaxStopId = meetingPointQueryResult.meetingPointMinMaxStopId;

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

                // don't calculate the real durations if the meeting point is not reachable
                for (int j = 0; j < targetStopIds.size(); j++) {
                    int targetStopId = targetStopIds[j];
                    if (csas[i]->getEarliestArrivalTime(targetStopId) == INT_MAX) {
                        meetingPointQueryResult.meetingPointMinSum = "";
                        meetingPointQueryResult.meetingPointMinMax = "";
                        return;
                    }
                }
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


        int maxTransfersMinSum = 0;
        int maxTransfersMinMax = 0;
        for (int i = 0; i < csas.size(); i++) {
            Journey journeyMinSum = csas[i]->createJourney(meetingPointMinSumStopId);
            Journey journeyMinMax = csas[i]->createJourney(meetingPointMinMaxStopId);

            if (journeyMinSum.legs.size() > maxTransfersMinSum) {
                maxTransfersMinSum = journeyMinSum.legs.size();
            }

            if (journeyMinMax.legs.size() > maxTransfersMinMax) {
                maxTransfersMinMax = journeyMinMax.legs.size();
            }
        }
        meetingPointQueryResult.maxTransfersMinSum = maxTransfersMinSum;
        meetingPointQueryResult.maxTransfersMinMax = maxTransfersMinMax;
    

        if (useCSA) {
            int connectionCounter = 0;
            for (int i = 0; i < csas.size(); i++) {
                connectionCounter += csas[i]->getConnectionCounter();
            }

            int maxNumberOfConnections = meetingPointQuery.sourceStopIds.size() * NUMBER_OF_DAYS * Importer::connections.size();
            meetingPointQueryGTreeCSAInfo.csaVisitedConnectionsFraction = (double) connectionCounter / maxNumberOfConnections;
        }
    }
}

/*
    Process a G-tree query for a given optimization criteria. If useCSA is true, the CSA algorithm is used to calculate the real durations during the algorithm.
*/
void GTreeQueryProcessor::processGTreeQueryWithOptimization(Optimization optimization, bool useCSA) {
    priority_queue<pair<int, int>, vector<pair<int, int>>, greater<pair<int, int>>> pq;
    int optimalMeetingPointStopId = -1;
    int currentBest = INT_MAX;

    int alpha = 1;
    if(!useCSA) {
        alpha = GTREE_APPROXIMATION_ALPHA;
    }

    int csaTargetStops = 0;

    // Calculate the initial lower bound of the root node
    int l = getLowerBoundToNode(gTree->root->nodeId, optimization);
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
                int childLowerBound = getLowerBoundToNode(childNodeId, optimization);
                if (childLowerBound < currentBest * alpha) {
                    pq.push(make_pair(childLowerBound, childNodeId));
                }
            }
        } 
        // Case 2: The current node is a leaf node. Calculate the costs to the stops and update the current best if necessary.
        else {
            vector<int> reachableTargetStopIds = vector<int>(0);
            for (int i = 0; i < currentNode->stopIds.size(); i++) {
                int stopId = currentNode->stopIds[i];
                int costs = getApproximatedCostsToStop(stopId, optimization);
                if (costs < currentBest) {
                    reachableTargetStopIds.push_back(stopId);
                    if (!useCSA) {
                        currentBest = costs;
                        optimalMeetingPointStopId = stopId;
                    }
                }
            }
            if (useCSA) {
                processCSAToTargetStops(reachableTargetStopIds, currentBest);
                #pragma omp parallel for
                for (int i = 0; i < reachableTargetStopIds.size(); i++) {
                    int stopId = reachableTargetStopIds[i];
                    int costs = getCostsToStop(stopId, optimization);
                    if (costs < currentBest) {
                        currentBest = costs;
                        optimalMeetingPointStopId = stopId;
                    }
                }
                csaTargetStops += reachableTargetStopIds.size();
            }
        }
    }
    
    // Set the optimal meeting point stop name
    string optimalMeetingPointStopName = "";
    if(optimalMeetingPointStopId != -1){
        optimalMeetingPointStopName = Importer::getStopName(optimalMeetingPointStopId);
    }
    if (optimization == min_sum) {
        meetingPointQueryResult.meetingPointMinSumStopId = optimalMeetingPointStopId;
        meetingPointQueryResult.meetingPointMinSum = optimalMeetingPointStopName;
        meetingPointQueryGTreeCSAInfo.csaTargetStopFractionMinSum = (double) csaTargetStops / Importer::stops.size();
    } else {
        meetingPointQueryResult.meetingPointMinMaxStopId = optimalMeetingPointStopId;
        meetingPointQueryResult.meetingPointMinMax = optimalMeetingPointStopName;
        meetingPointQueryGTreeCSAInfo.csaTargetStopFractionMinMax = (double) csaTargetStops / Importer::stops.size();
    }
}

/*
    Get the lower bound to a node.
*/
int GTreeQueryProcessor::getLowerBoundToNode(int nodeId, Optimization optimization) {
    int lowerBound = 0;
    vector<int> durations = vector<int>(meetingPointQuery.sourceStopIds.size(), 0);
    #pragma omp parallel for
    for (int i = 0; i < meetingPointQuery.sourceStopIds.size(); i++) {
        durations[i] = gTree->getMinimalDurationToNode(meetingPointQuery.sourceStopIds[i], nodeId, queryPointAndNodeToBorderStopDurations[i]); 
    }

    for (int i = 0; i < meetingPointQuery.sourceStopIds.size(); i++) {
        int duration = durations[i];
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
int GTreeQueryProcessor::getApproximatedCostsToStop(int stopId, Optimization optimization) {
    int costs = 0;
    vector<int> durations = vector<int>(meetingPointQuery.sourceStopIds.size(), 0);
    #pragma omp parallel for
    for (int i = 0; i < meetingPointQuery.sourceStopIds.size(); i++) {
        durations[i] = gTree->getMinimalDurationToStop(meetingPointQuery.sourceStopIds[i], stopId, queryPointAndNodeToBorderStopDurations[i]); 
    }

    for (int i = 0; i < meetingPointQuery.sourceStopIds.size(); i++) {
        int duration = durations[i];
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

int GTreeQueryProcessor::getCostsToStop(int stopId, Optimization optimization) {
    int costs = 0;
    for (int i = 0; i < meetingPointQuery.sourceStopIds.size(); i++) {
        int earliestArrivalTime = csas[i]->getEarliestArrivalTime(stopId);
        if (earliestArrivalTime == INT_MAX) {
            return INT_MAX;
        }
        int duration = earliestArrivalTime - meetingPointQuery.sourceTime;
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
            int maxDepartureTime = min(meetingPointQuery.sourceTime + currentBest, meetingPointQuery.sourceTime + (NUMBER_OF_DAYS * SECONDS_PER_DAY));
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

MeetingPointQueryGTreeCSAInfo GTreeQueryProcessor::getMeetingPointQueryGTreeCSAInfo() {
    return meetingPointQueryGTreeCSAInfo;
}

/*
    Get the journeys to the meeting points.
*/
vector<Journey> GTreeQueryProcessor::getJourneys(Optimization optimization) {
    vector<Journey> journeys;
    int targetStopId;
    if (optimization == min_sum) {
        targetStopId = meetingPointQueryResult.meetingPointMinSumStopId;
    } else {
        targetStopId = meetingPointQueryResult.meetingPointMinMaxStopId;
    }
    for (int i = 0; i < csas.size(); i++) {
        Journey journey = csas[i]->createJourney(targetStopId);
        journeys.push_back(journey);
    }
    return journeys;
}

void RaptorQueryProcessor::processRaptorQuery() {
    auto start = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < meetingPointQuery.sourceStopIds.size(); i++) {
        RaptorQuery query;
        query.sourceStopId = meetingPointQuery.sourceStopIds[i];
        query.sourceTime = meetingPointQuery.sourceTime;
        query.weekday = meetingPointQuery.weekday;
        Raptor* raptor = new Raptor(query);
        raptors.push_back(raptor);
    }

    int transfers = 0;

    while (true) {
        bool allFinished = true;
        #pragma omp parallel for
        for (int i = 0; i < raptors.size(); i++) {
            if(!raptors[i]->isFinished()) {
                raptors[i]->processRaptorRound();
                allFinished = false;
            }
        }
        if (allFinished) {
            break;
        }
        
        int minMeetingTime = INT_MAX;
        int durationSumOfMeetingPoint = 0;
        int meetingStopId = -1;

        for (int i = 0; i < Importer::stops.size(); i++) {
            int meetingTime = 0;
            int durationSum = 0;
            for (int j = 0; j < raptors.size(); j++) {
                int earliestArrivalTime = raptors[j]->getEarliestArrivalTime(i);
                if (earliestArrivalTime == INT_MAX) {
                    meetingTime = INT_MAX;
                    break;
                }
                durationSum += earliestArrivalTime - meetingPointQuery.sourceTime;
                if (earliestArrivalTime > meetingTime) {
                    meetingTime = earliestArrivalTime;
                }
            }
            if (meetingTime < minMeetingTime) {
                minMeetingTime = meetingTime;
                durationSumOfMeetingPoint = durationSum;
                meetingStopId = i;
            }
        }

        if (minMeetingTime != INT_MAX) {
            meetingPointQueryResult.meetingPointStopId = meetingStopId;
            meetingPointQueryResult.meetingPoint = Importer::getStopName(meetingStopId);
            meetingPointQueryResult.meetingTime = TimeConverter::convertSecondsToTime(minMeetingTime, true);
            meetingPointQueryResult.durationInSeconds = minMeetingTime - meetingPointQuery.sourceTime;
            meetingPointQueryResult.duration = TimeConverter::convertSecondsToTime(meetingPointQueryResult.durationInSeconds, false);
            meetingPointQueryResult.durationSum = TimeConverter::convertSecondsToTime(durationSumOfMeetingPoint, false);
            meetingPointQueryResult.durationSumInSeconds = durationSumOfMeetingPoint;
            meetingPointQueryResult.maxNumberOfTransfers = transfers;

            break;
        }

        transfers++;
    }

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    meetingPointQueryResult.queryTime = duration;
}

MeetingPointQueryRaptorResult RaptorQueryProcessor::getMeetingPointQueryResult() {
    return meetingPointQueryResult;
}