#include "query-processor.h"

#include "csa.h"
#include "raptor.h"
#include "optimization.h"
#include "landmark-processor.h"
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
#include <memory>

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
    auto start = std::chrono::high_resolution_clock::now();

    // RaptorQueryProcessor raptorQueryProcessor = RaptorQueryProcessor(meetingPointQuery);
    // raptorQueryProcessor.processRaptorQueryUntilFirstResult();
    // MeetingPointQueryResult meetingPointQueryResultRaptor = raptorQueryProcessor.getMeetingPointQueryResult();

    // int maxDepartureTime = meetingPointQuery.sourceTime + meetingPointQueryResultRaptor.minSumDurationInSeconds;

    for (int i = 0; i < meetingPointQuery.sourceStopIds.size(); i++) {
        CSAQuery query;
        query.sourceStopId = meetingPointQuery.sourceStopIds[i];
        query.sourceTime = meetingPointQuery.sourceTime;
        query.weekday = meetingPointQuery.weekday;
        shared_ptr<CSA> csa = shared_ptr<CSA> (new CSA(query));
        // csa->setMaxDepartureTime(maxDepartureTime);
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

            if (journeyMinSum.legs.size() > 1 && journeyMinSum.legs.size() - 1 > maxTransfersMinSum) {
                maxTransfersMinSum = journeyMinSum.legs.size() - 1;
            }

            if (journeyMinMax.legs.size() > 1 && journeyMinMax.legs.size() - 1 > maxTransfersMinMax) {
                maxTransfersMinMax = journeyMinMax.legs.size() - 1;
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
            RaptorQueryProcessor naiveQueryProcessor = RaptorQueryProcessor(randomMeetingPointQuery);
            naiveQueryProcessor.processRaptorQuery();
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
    auto start = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < meetingPointQuery.sourceStopIds.size(); i++) {
        CSAQuery query;
        query.sourceStopId = meetingPointQuery.sourceStopIds[i];
        query.targetStopIds = keyStops;
        query.sourceTime = meetingPointQuery.sourceTime;
        query.weekday = meetingPointQuery.weekday;
        shared_ptr<CSA> csa = shared_ptr<CSA> (new CSA(query));
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

            if (journeyMinSum.legs.size() > 1 && journeyMinSum.legs.size() - 1 > maxTransfersMinSum) {
                maxTransfersMinSum = journeyMinSum.legs.size() - 1;
            }

            if (journeyMinMax.legs.size() > 1 && journeyMinMax.legs.size() - 1 > maxTransfersMinMax) {
                maxTransfersMinMax = journeyMinMax.legs.size() - 1;
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
        meetingPointQuery.sourceStopIds.push_back(Importer::connections[rand() % Importer::connections.size()].departureStopId);
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

RaptorQuery QueryGenerator::parseRaptorQuery(string line) {
    RaptorQuery raptorQuery;
    vector<string> parts;

    // Split the line into substrings
    std::stringstream ss(line);
    std::string substring;
    while (std::getline(ss, substring, ',')) {
        parts.push_back(substring);
    }
    raptorQuery.sourceStopId = stoi(parts[0]);
    raptorQuery.targetStopIds.push_back(stoi(parts[1]));
    raptorQuery.sourceTime = stoi(parts[2]);
    raptorQuery.weekday = stoi(parts[3]);
    return raptorQuery;
}

RaptorQuery QueryGenerator::generateRandomRaptorQuery() {
    RaptorQuery raptorQuery;
    raptorQuery.sourceStopId = Importer::connections[rand() % Importer::connections.size()].departureStopId;
    raptorQuery.targetStopIds.push_back(Importer::connections[rand() % Importer::connections.size()].departureStopId);
    raptorQuery.sourceTime = rand() % SECONDS_PER_DAY;
    raptorQuery.weekday = rand() % 7;
    return raptorQuery;
}

RaptorQuery QueryGenerator::generateRaptorQuery(string sourceStopName, string targetStopName, string sourceTime, string weekday) {
    RaptorQuery raptorQuery;
    raptorQuery.sourceStopId = Importer::getStopId(sourceStopName);
    raptorQuery.targetStopIds.push_back(Importer::getStopId(targetStopName));
    raptorQuery.sourceTime = TimeConverter::convertTimeToSeconds(sourceTime);
    raptorQuery.weekday = WeekdayConverter::convertWeekdayToInt(weekday);
    return raptorQuery;
}

/*
    Process a G-tree query. If useCSA is true, the CSA algorithm is used to calculate the real durations during the algorithm. 
    Otherwise, approximate durations are calculated. In this case are the real durations calculated after the algorithm.
*/
void GTreeQueryProcessor::processGTreeQuery(bool useCSA) {
    for (int i = 0; i < meetingPointQuery.sourceStopIds.size(); i++) {
        CSAQuery query;
        query.sourceStopId = meetingPointQuery.sourceStopIds[i];
        query.sourceTime = meetingPointQuery.sourceTime;
        query.weekday = meetingPointQuery.weekday;
        shared_ptr<CSA> csa = shared_ptr<CSA> (new CSA(query));
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

    double alpha = 1;
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

void RaptorQueryProcessor::initializeRaptors() {
    for (int i = 0; i < meetingPointQuery.sourceStopIds.size(); i++) {
        RaptorQuery query;
        query.sourceStopId = meetingPointQuery.sourceStopIds[i];
        query.sourceTime = meetingPointQuery.sourceTime;
        query.weekday = meetingPointQuery.weekday;
        shared_ptr<Raptor> raptor = shared_ptr<Raptor> (new Raptor(query));
        raptors.push_back(raptor);
    }

    transfers = 0;
    lastRoundMinSumDuration = INT_MAX;
    lastRoundMinMaxDuration = INT_MAX;
}

void RaptorQueryProcessor::processRaptorQueryUntilFirstResult() {
    auto start = std::chrono::high_resolution_clock::now();

    initializeRaptors();

    while (true) {
        bool allFinished = processRaptorRound();
        if (allFinished) {
            break;
        }

        bool foundMeetingPoint = meetingPointQueryResult.meetingPointMinSum != "";

        if (foundMeetingPoint) {
            break;
        }
    }

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    meetingPointQueryResult.queryTime = duration;

    numberOfExpandedRoutes = 0;
    for (int i = 0; i < raptors.size(); i++) {
        numberOfExpandedRoutes += raptors[i]->numberOfExpandedRoutes;
    }
    numberOfExpandedRoutes = numberOfExpandedRoutes / raptors.size();
}

void RaptorQueryProcessor::processRaptorQuery() {
    auto start = std::chrono::high_resolution_clock::now();

    initializeRaptors();

    while (true) {
        bool allFinished = processRaptorRound();
        if (allFinished) {
            break;
        }
    }

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    meetingPointQueryResult.queryTime = duration;

    numberOfExpandedRoutes = 0;
    for (int i = 0; i < raptors.size(); i++) {
        numberOfExpandedRoutes += raptors[i]->numberOfExpandedRoutes;
    }
    numberOfExpandedRoutes = numberOfExpandedRoutes / raptors.size();
}

void RaptorQueryProcessor::processRaptorQueryUntilResultDoesntImprove(Optimization optimization) {
    auto start = std::chrono::high_resolution_clock::now();

    initializeRaptors();

    int minDurationMinSum = INT_MAX;
    int minDurationMinMax = INT_MAX;

    while (true) {
        bool allFinished = processRaptorRound();
        if (allFinished) {
            break;
        }

        bool foundMeetingPoint = meetingPointQueryResult.meetingPointMinSum != "" || meetingPointQueryResult.meetingPointMinMax != "";

        if (foundMeetingPoint) {
            bool improvedResult = false;
            int durationMinSum = meetingPointQueryResult.minSumDurationInSeconds;
            int durationMinMax = meetingPointQueryResult.minMaxDurationInSeconds;
            if ((optimization == min_sum || optimization == both) && durationMinSum < minDurationMinSum) {
                minDurationMinSum = durationMinSum;
                improvedResult = true;
            }
            if ((optimization == min_max || optimization == both) && durationMinMax < minDurationMinMax) {
                minDurationMinMax = durationMinMax;
                improvedResult = true;
            }
            if (!improvedResult) {
                break;
            }
        }
    }

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    meetingPointQueryResult.queryTime = duration;

    numberOfExpandedRoutes = 0;
    for (int i = 0; i < raptors.size(); i++) {
        numberOfExpandedRoutes += raptors[i]->numberOfExpandedRoutes;
    }
    numberOfExpandedRoutes = numberOfExpandedRoutes / raptors.size();
}

bool RaptorQueryProcessor::processRaptorRound() {
    auto start = std::chrono::high_resolution_clock::now();

    bool allFinished = true;
    // omp_set_dynamic(0);
    // omp_set_num_threads(4);
    #pragma omp parallel for
    for (int i = 0; i < raptors.size(); i++) {
        if(!raptors[i]->isFinished()) {
            raptors[i]->processRaptorRound();
            allFinished = false;
        }
    }
    if (allFinished) {
        return true;
    }
    
    int meetingPointMinSum = -1;
    int minDurationMinSum = INT_MAX;
    int meetingTimeMinSum = INT_MAX;

    int meetingPointMinMax = -1;
    int minDurationMinMax = INT_MAX;
    int meetingTimeMinMax = INT_MAX;

    for (int i = 0; i < Importer::stops.size(); i++) {
        int durationMinSum = 0;
        int durationMinMax = 0;
        int meetingTime;
        for (int j = 0; j < raptors.size(); j++) {
            int earliestArrivalTime = raptors[j]->getEarliestArrivalTime(i);
            if (earliestArrivalTime == INT_MAX) {
                durationMinSum = INT_MAX;
                durationMinMax = INT_MAX;
                break;
            }
            int duration = earliestArrivalTime - meetingPointQuery.sourceTime;
            durationMinSum += duration;
            if (duration > durationMinMax) {
                durationMinMax = duration;
                meetingTime = earliestArrivalTime;
            }
        }
        if (durationMinSum < minDurationMinSum) {
            meetingPointMinSum = i;
            minDurationMinSum = durationMinSum;
            meetingTimeMinSum = meetingTime;
        }
        if (durationMinMax < minDurationMinMax) {
            meetingPointMinMax = i;
            minDurationMinMax = durationMinMax;
            meetingTimeMinMax = meetingTime;
        }
    }

    if (minDurationMinSum < lastRoundMinSumDuration) {
        meetingPointQueryResult.meetingPointMinSumStopId = meetingPointMinSum;
        meetingPointQueryResult.meetingPointMinSum = Importer::getStopName(meetingPointMinSum);
        meetingPointQueryResult.meetingTimeMinSum = TimeConverter::convertSecondsToTime(meetingTimeMinSum, true);
        meetingPointQueryResult.minSumDuration = TimeConverter::convertSecondsToTime(minDurationMinSum, false);
        meetingPointQueryResult.minSumDurationInSeconds = minDurationMinSum;
        meetingPointQueryResult.maxTransfersMinSum = transfers;
        lastRoundMinSumDuration = minDurationMinSum;
    }

    if (minDurationMinMax < lastRoundMinMaxDuration) {
        meetingPointQueryResult.meetingPointMinMaxStopId = meetingPointMinMax;
        meetingPointQueryResult.meetingPointMinMax = Importer::getStopName(meetingPointMinMax);
        meetingPointQueryResult.meetingTimeMinMax = TimeConverter::convertSecondsToTime(meetingTimeMinMax, true);
        meetingPointQueryResult.minMaxDuration = TimeConverter::convertSecondsToTime(minDurationMinMax, false);
        meetingPointQueryResult.minMaxDurationInSeconds = minDurationMinMax;
        meetingPointQueryResult.maxTransfersMinMax = transfers;
        lastRoundMinMaxDuration = minDurationMinMax;
    }

    transfers++;

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    durationOfLastRound = duration;

    return false;
}

MeetingPointQueryResult RaptorQueryProcessor::getMeetingPointQueryResult() {
    return meetingPointQueryResult;
}

vector<Journey> RaptorQueryProcessor::getJourneys(Optimization optimization) {
    vector<Journey> journeys;
    int targetStopId;
    if (optimization == min_sum) {
        targetStopId = meetingPointQueryResult.meetingPointMinSumStopId;
    } else {
        targetStopId = meetingPointQueryResult.meetingPointMinMaxStopId;
    }
    for (int i = 0; i < raptors.size(); i++) {
        Journey journey = raptors[i]->createJourney(targetStopId);
        journeys.push_back(journey);
    }
    return journeys;
}

vector<int> RaptorQueryProcessor::getStopsWithGivenAccuracy(double accuracyBound) {
    vector<int> meetingPointsOverAccuracy = vector<int>(0);
    for (int i = 0; i < Importer::stops.size(); i++) {
        int sum = 0;
        int max = 0;
        for (int j = 0; j < meetingPointQuery.sourceStopIds.size(); j++) {
            int earliestArrivalTime = raptors[j]->getEarliestArrivalTime(i);
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

void RaptorBoundQueryProcessor::processRaptorBoundQuery(Optimization optimization) {
    // if (optimization == min_sum) {
    //     cout << "min sum" << endl;
    // } else {
    //     cout << "min max" << endl;
    // }
    map<int, vector<int>> sourceStopIdToAllStops;

    auto start = std::chrono::high_resolution_clock::now();
    if (!USE_LANDMARKS) {
        sourceStopIdToAllStops = Creator::networkGraph.getDistancesWithPhast(meetingPointQuery.sourceStopIds);
    }
    auto endPhast = std::chrono::high_resolution_clock::now();
    durationPhast = std::chrono::duration_cast<std::chrono::milliseconds>(endPhast - start).count();

    auto startInitRaptorBounds = std::chrono::high_resolution_clock::now();
    raptorBounds = vector<shared_ptr<RaptorBound>>(meetingPointQuery.sourceStopIds.size());
    // omp_set_dynamic(0);
    // omp_set_num_threads(4);
    #pragma omp parallel for
    for (int i = 0; i < meetingPointQuery.sourceStopIds.size(); i++) {
        RaptorQuery query;
        query.sourceStopId = meetingPointQuery.sourceStopIds[i];
        query.sourceTime = meetingPointQuery.sourceTime;
        query.weekday = meetingPointQuery.weekday;
        shared_ptr<RaptorBound> raptorBound = shared_ptr<RaptorBound> (new RaptorBound(query, optimization));
        raptorBound->initializeHeuristic(sourceStopIdToAllStops, meetingPointQuery.sourceStopIds);
        raptorBounds[i] = raptorBound;
    }
    auto endInitRaptorBounds = std::chrono::high_resolution_clock::now();
    durationInitRaptorBounds = std::chrono::duration_cast<std::chrono::milliseconds>(endInitRaptorBounds - startInitRaptorBounds).count();

    auto startRaptorBound = std::chrono::high_resolution_clock::now();

    int currentBest = INT_MAX;

    while (true) {
        // cout << "current best: " << currentBest << endl;
        bool allFinished = true;
        // omp_set_dynamic(0);
        // omp_set_num_threads(4);
        #pragma omp parallel for
        for (int i = 0; i < raptorBounds.size(); i++) {
            if(!raptorBounds[i]->isFinished()) {
                raptorBounds[i]->setCurrentBest(currentBest);
                raptorBounds[i]->processRaptorRound();
                allFinished = false;
            }
        }

        if (allFinished) {
            break;
        }
        
        for (int i = 0; i < Importer::stops.size(); i++) {
            int lastRoundBest = 0;

            for (int j = 0; j < raptorBounds.size(); j++) {
                int earliestArrivalTime = raptorBounds[j]->getEarliestArrivalTime(i);
                if (earliestArrivalTime == INT_MAX) {
                    lastRoundBest = INT_MAX;
                    break;
                }
                int duration = earliestArrivalTime - meetingPointQuery.sourceTime;
                if (optimization == min_sum) {
                    lastRoundBest += duration;
                } else {
                    if (duration > lastRoundBest) {
                        lastRoundBest = duration;
                    }
                }
            }

            if (lastRoundBest < currentBest) {
                currentBest = lastRoundBest;
            }
        }
    }

    auto endRaptorBound = std::chrono::high_resolution_clock::now();
    durationRaptorBounds = std::chrono::duration_cast<std::chrono::milliseconds>(endRaptorBound - startRaptorBound).count();

    auto startCreateResult = std::chrono::high_resolution_clock::now();

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
            int earliestArrivalTime = raptorBounds[j]->getEarliestArrivalTime(i);
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
            meetingPointQueryResult.minSumMeetingTimeInSeconds = arrivalTime;
            meetingPointQueryResult.minSumDuration = TimeConverter::convertSecondsToTime(sum, false);
            meetingPointQueryResult.minSumDurationInSeconds = sum;
            stopIdMinSum = i;
            minSum = sum;
        }
        if (max < minMax) {
            meetingPointQueryResult.meetingPointMinMaxStopId = i;
            meetingPointQueryResult.meetingPointMinMax = Importer::getStopName(i);
            meetingPointQueryResult.meetingTimeMinMax = TimeConverter::convertSecondsToTime(arrivalTime, true);
            meetingPointQueryResult.minMaxMeetingTimeInSeconds = arrivalTime;
            meetingPointQueryResult.minMaxDuration = TimeConverter::convertSecondsToTime(max, false);
            meetingPointQueryResult.minMaxDurationInSeconds = max;
            stopIdMinMax = i;
            minMax = max;
        }
    }

    if (stopIdMinSum != -1 && stopIdMinMax != -1) {
        int maxTransfersMinSum = 0;
        int maxTransfersMinMax = 0;
        for (int i = 0; i < raptorBounds.size(); i++) {
            Journey journeyMinSum = raptorBounds[i]->createJourney(stopIdMinSum);
            Journey journeyMinMax = raptorBounds[i]->createJourney(stopIdMinMax);

            if (journeyMinSum.legs.size() > 1 && journeyMinSum.legs.size() - 1 > maxTransfersMinSum) {
                maxTransfersMinSum = journeyMinSum.legs.size() - 1;
            }

            if (journeyMinMax.legs.size() > 1 && journeyMinMax.legs.size() - 1 > maxTransfersMinMax) {
                maxTransfersMinMax = journeyMinMax.legs.size() - 1;
            }
        }
        meetingPointQueryResult.maxTransfersMinSum = maxTransfersMinSum;
        meetingPointQueryResult.maxTransfersMinMax = maxTransfersMinMax;
    }

    auto endCreateResult = std::chrono::high_resolution_clock::now();
    durationCreateResult = std::chrono::duration_cast<std::chrono::milliseconds>(endCreateResult - startCreateResult).count();

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    meetingPointQueryResult.queryTime = duration;

    numberOfExpandedRoutes = 0;
    lowerBoundSmallerCounter = 0;
    lowerBoundGreaterCounter = 0;
    lowerBoundAbsDiff = 0;
    lowerBoundRelDiff = 0;
    for (int i = 0; i < raptorBounds.size(); i++) {
        numberOfExpandedRoutes += raptorBounds[i]->numberOfExpandedRoutes;
        lowerBoundSmallerCounter += raptorBounds[i]->lowerBoundSmallerCounter;
        lowerBoundGreaterCounter += raptorBounds[i]->lowerBoundGreaterCounter;
        lowerBoundAbsDiff += raptorBounds[i]->lowerBoundAbsDifference;
        lowerBoundRelDiff += raptorBounds[i]->lowerBoundRelDifference;
    }
    numberOfExpandedRoutes = numberOfExpandedRoutes / raptorBounds.size();
    lowerBoundAbsDiff = lowerBoundAbsDiff / (lowerBoundSmallerCounter * 3600);
    lowerBoundRelDiff = lowerBoundRelDiff / lowerBoundSmallerCounter;
    lowerBoundSmallerCounter = lowerBoundSmallerCounter / raptorBounds.size();
    lowerBoundGreaterCounter = lowerBoundGreaterCounter / raptorBounds.size();
    lowerBoundSmallerFraction = (double) lowerBoundSmallerCounter / (lowerBoundSmallerCounter + lowerBoundGreaterCounter);
}

MeetingPointQueryResult RaptorBoundQueryProcessor::getMeetingPointQueryResult() {
    return meetingPointQueryResult;
}

vector<Journey> RaptorBoundQueryProcessor::getJourneys(Optimization optimization) {
    vector<Journey> journeys;
    int targetStopId;
    if (optimization == min_sum) {
        targetStopId = meetingPointQueryResult.meetingPointMinSumStopId;
    } else {
        targetStopId = meetingPointQueryResult.meetingPointMinMaxStopId;
    }
    for (int i = 0; i < raptorBounds.size(); i++) {
        Journey journey = raptorBounds[i]->createJourney(targetStopId);
        journeys.push_back(journey);
    }
    return journeys;
}

vector<CandidateInfo> RaptorBoundQueryProcessor::getStopsAndResultsWithSmallerRelativeDifference(double relativeDifference, Optimization optimization) {
    vector<CandidateInfo> meetingPointsWithSmallerRelativeDifference = vector<CandidateInfo>(0);
    int bestResult;
    if (optimization == min_sum) {
        bestResult = meetingPointQueryResult.minSumDurationInSeconds;
    } else {
        bestResult = meetingPointQueryResult.minMaxDurationInSeconds;
    }
    for (int i = 0; i < Importer::stops.size(); i++) {
        int currentResult = 0;
        int arrivalTime = 0;
        for (int j = 0; j < meetingPointQuery.sourceStopIds.size(); j++) {
            int earliestArrivalTime = raptorBounds[j]->getEarliestArrivalTime(i);
            if (earliestArrivalTime == INT_MAX) {
                currentResult = INT_MAX;
                break;
            }

            int duration = earliestArrivalTime - meetingPointQuery.sourceTime;
            if (optimization == min_sum) {
                currentResult += duration;
            } else {
                if (duration > currentResult) {
                    currentResult = duration;
                }
            } 

            if (earliestArrivalTime > arrivalTime) {
                arrivalTime = earliestArrivalTime;
            }
        }
        if (currentResult == INT_MAX) {
            continue;
        }

        int difference = currentResult - bestResult;
        double currentRelativeDifference = (double) difference / currentResult;
        if (currentRelativeDifference < relativeDifference) {
            CandidateInfo candidateInfo;
            candidateInfo.stopId = i;
            candidateInfo.duration = currentResult;
            candidateInfo.meetingTime = arrivalTime;
            meetingPointsWithSmallerRelativeDifference.push_back(candidateInfo);
        }
    }

    // sort candidate infos by result and select the top 25 results
    sort(meetingPointsWithSmallerRelativeDifference.begin(), meetingPointsWithSmallerRelativeDifference.end(), [](const CandidateInfo &a, const CandidateInfo &b) {
        return a.duration < b.duration;
    });

    if (meetingPointsWithSmallerRelativeDifference.size() > 25) {
        meetingPointsWithSmallerRelativeDifference.resize(25);
    }

    return meetingPointsWithSmallerRelativeDifference;
}

void RaptorPQQueryProcessor::processRaptorPQQuery(Optimization optimization) {
    map<int, vector<int>> sourceStopIdToAllStops;

    auto start = std::chrono::high_resolution_clock::now();
    if (!USE_LANDMARKS) {
        sourceStopIdToAllStops = Creator::networkGraph.getDistancesWithPhast(meetingPointQuery.sourceStopIds);
    }
    auto endPhast = std::chrono::high_resolution_clock::now();
    durationPhast = std::chrono::duration_cast<std::chrono::milliseconds>(endPhast - start).count();
    
    auto startRaptorFirstResult = std::chrono::high_resolution_clock::now();
    RaptorQueryProcessor raptorQueryProcessor = RaptorQueryProcessor(meetingPointQuery);
    raptorQueryProcessor.processRaptorQueryUntilFirstResult();
    MeetingPointQueryResult meetingPointQueryResultRaptor = raptorQueryProcessor.getMeetingPointQueryResult();
    auto endRaptorFirstResult = std::chrono::high_resolution_clock::now();
    durationRaptorFirstResult = std::chrono::duration_cast<std::chrono::milliseconds>(endRaptorFirstResult - startRaptorFirstResult).count();

    auto initRaptorPQs = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < meetingPointQuery.sourceStopIds.size(); i++) {
        RaptorQuery query;
        query.sourceStopId = meetingPointQuery.sourceStopIds[i];
        query.sourceTime = meetingPointQuery.sourceTime;
        query.weekday = meetingPointQuery.weekday;
        shared_ptr<RaptorPQ> raptorPQ = shared_ptr<RaptorPQ> (new RaptorPQ(query, optimization));
        raptorPQs.push_back(raptorPQ);
    }
    auto endInitRaptorPQs = std::chrono::high_resolution_clock::now();
    durationInitRaptorPQs = std::chrono::duration_cast<std::chrono::milliseconds>(endInitRaptorPQs - initRaptorPQs).count();

    // cout << "upper bound min sum: " << meetingPointQueryResultRaptor.minSumDurationInSeconds << ", upper bound min max: " << meetingPointQueryResultRaptor.minMaxDurationInSeconds << endl;

    auto startRaptorPQ = std::chrono::high_resolution_clock::now();
    // omp_set_dynamic(0);
    // omp_set_num_threads(4);
    #pragma omp parallel for
    for (int i = 0; i < raptorPQs.size(); i++) {
        raptorPQs[i]->initializeHeuristic(sourceStopIdToAllStops, meetingPointQuery.sourceStopIds);
        if (optimization == min_sum) {
            int upperBound = meetingPointQueryResultRaptor.minSumDurationInSeconds;
            raptorPQs[i]->setCurrentBest(upperBound);
        } else {
            int upperBound = meetingPointQueryResultRaptor.minMaxDurationInSeconds;
            raptorPQs[i]->setCurrentBest(upperBound);
        }
        // raptorPQs[i]->initializeRaptorPQ();
        raptorPQs[i]->transformRaptorToRaptorPQ(raptorQueryProcessor.raptors[i]);
        raptorPQs[i]->processRaptorPQ();
    }
    auto endRaptorPQ = std::chrono::high_resolution_clock::now();
    durationRaptorPQs = std::chrono::duration_cast<std::chrono::milliseconds>(endRaptorPQ - startRaptorPQ).count();

    auto startCreateResult = std::chrono::high_resolution_clock::now();

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
            int earliestArrivalTime = raptorPQs[j]->getEarliestArrivalTime(i);
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
        for (int i = 0; i < raptorPQs.size(); i++) {
            Journey journeyMinSum = raptorPQs[i]->createJourney(stopIdMinSum);
            Journey journeyMinMax = raptorPQs[i]->createJourney(stopIdMinMax);

            if (journeyMinSum.legs.size() > 1 && journeyMinSum.legs.size() - 1 > maxTransfersMinSum) {
                maxTransfersMinSum = journeyMinSum.legs.size() - 1;
            }

            if (journeyMinMax.legs.size() > 1 && journeyMinMax.legs.size() - 1 > maxTransfersMinMax) {
                maxTransfersMinMax = journeyMinMax.legs.size() - 1;
            }
        }
        meetingPointQueryResult.maxTransfersMinSum = maxTransfersMinSum;
        meetingPointQueryResult.maxTransfersMinMax = maxTransfersMinMax;
    }

    auto endCreateResult = std::chrono::high_resolution_clock::now();
    durationCreateResult = std::chrono::duration_cast<std::chrono::milliseconds>(endCreateResult - startCreateResult).count();

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    meetingPointQueryResult.queryTime = duration;

    numberOfExpandedRoutes = 0;
    lowerBoundSmallerCounter = 0;
    lowerBoundGreaterCounter = 0;
    lowerBoundAbsDiff = 0;
    lowerBoundRelDiff = 0;

    durationInitHeuristic = 0;
    durationTransformRaptorToRaptorPQ = 0;
    durationAddRoutesToQueue = 0;
    durationGetEarliestTripWithDayOffset = 0;
    durationTraverseRoute = 0;
    altHeuristicImprovementCounter = 0;
    noHeuristicImprovementCounter = 0;
    for (int i = 0; i < raptorPQs.size(); i++) {
        numberOfExpandedRoutes += raptorPQs[i]->numberOfExpandedRoutes;

        lowerBoundSmallerCounter += raptorPQs[i]->lowerBoundSmallerCounter;
        lowerBoundGreaterCounter += raptorPQs[i]->lowerBoundGreaterCounter;
        lowerBoundAbsDiff += raptorPQs[i]->lowerBoundAbsDifference;
        lowerBoundRelDiff += raptorPQs[i]->lowerBoundRelDifference;

        durationInitHeuristic += raptorPQs[i]->durationInitHeuristic;
        durationTransformRaptorToRaptorPQ += raptorPQs[i]->durationTransformRaptorToRaptorPQ;
        durationAddRoutesToQueue += raptorPQs[i]->durationAddRoutesToQueue;
        durationGetEarliestTripWithDayOffset += raptorPQs[i]->durationGetEarliestTripWithDayOffset;
        durationTraverseRoute += raptorPQs[i]->durationTraverseRoute;
        if (optimization == min_max) {
            altHeuristicImprovementCounter += raptorPQs[i]->altHeuristicImprovementCounter;
            noHeuristicImprovementCounter += raptorPQs[i]->noHeuristicImprovementCounter;
        }
    }
    numberOfExpandedRoutes = numberOfExpandedRoutes / raptorPQs.size();

    lowerBoundAbsDiff = lowerBoundAbsDiff / (lowerBoundSmallerCounter * 3600);
    lowerBoundRelDiff = lowerBoundRelDiff / lowerBoundSmallerCounter;
    lowerBoundSmallerCounter = lowerBoundSmallerCounter / raptorPQs.size();
    lowerBoundGreaterCounter = lowerBoundGreaterCounter / raptorPQs.size();
    lowerBoundSmallerFraction = (double) lowerBoundSmallerCounter / (lowerBoundSmallerCounter + lowerBoundGreaterCounter);

    durationInitHeuristic = durationInitHeuristic / (raptorPQs.size() * 1000);
    durationTransformRaptorToRaptorPQ = durationTransformRaptorToRaptorPQ / (raptorPQs.size() * 1000);
    durationAddRoutesToQueue = durationAddRoutesToQueue / (raptorPQs.size() * 1000);
    durationGetEarliestTripWithDayOffset = durationGetEarliestTripWithDayOffset / (raptorPQs.size() * 1000);
    durationTraverseRoute = durationTraverseRoute / (raptorPQs.size() * 1000);
    altHeuristicImprovementCounter = altHeuristicImprovementCounter / raptorPQs.size();
    noHeuristicImprovementCounter = noHeuristicImprovementCounter / raptorPQs.size();
    altHeuristicImprovementFraction = (double) altHeuristicImprovementCounter / (altHeuristicImprovementCounter + noHeuristicImprovementCounter);
}

MeetingPointQueryResult RaptorPQQueryProcessor::getMeetingPointQueryResult() {
    return meetingPointQueryResult;
}

vector<Journey> RaptorPQQueryProcessor::getJourneys(Optimization optimization) {
    vector<Journey> journeys;
    int targetStopId;
    if (optimization == min_sum) {
        targetStopId = meetingPointQueryResult.meetingPointMinSumStopId;
    } else {
        targetStopId = meetingPointQueryResult.meetingPointMinMaxStopId;
    }
    for (int i = 0; i < raptorPQs.size(); i++) {
        Journey journey = raptorPQs[i]->createJourney(targetStopId);
        journeys.push_back(journey);
    }
    return journeys;
}

void RaptorPQParallelQueryProcessor::processRaptorPQParallelQuery(Optimization optimization) {
    map<int, vector<int>> sourceStopIdToAllStops;

    auto start = std::chrono::high_resolution_clock::now();
    
    sourceStopIdToAllStops = Creator::networkGraph.getDistancesWithPhast(meetingPointQuery.sourceStopIds);
    auto endPhast = std::chrono::high_resolution_clock::now();
    durationPhast = std::chrono::duration_cast<std::chrono::milliseconds>(endPhast - start).count();
    
    auto startRaptorFirstResult = std::chrono::high_resolution_clock::now();
    RaptorQueryProcessor raptorQueryProcessor = RaptorQueryProcessor(meetingPointQuery);
    raptorQueryProcessor.processRaptorQueryUntilFirstResult();
    MeetingPointQueryResult meetingPointQueryResultRaptor = raptorQueryProcessor.getMeetingPointQueryResult();
    auto endRaptorFirstResult = std::chrono::high_resolution_clock::now();
    durationRaptorFirstResult = std::chrono::duration_cast<std::chrono::milliseconds>(endRaptorFirstResult - startRaptorFirstResult).count();

    auto initRaptorPQs = std::chrono::high_resolution_clock::now();
    vector<RaptorQuery> queries = vector<RaptorQuery>(meetingPointQuery.sourceStopIds.size());
    for (int i = 0; i < meetingPointQuery.sourceStopIds.size(); i++) {
        RaptorQuery query;
        query.sourceStopId = meetingPointQuery.sourceStopIds[i];
        query.sourceTime = meetingPointQuery.sourceTime;
        query.weekday = meetingPointQuery.weekday;
        queries[i] = query;
    }
    raptorPQParallel = shared_ptr<RaptorPQParallel> (new RaptorPQParallel(queries, optimization));
    auto endInitRaptorPQs = std::chrono::high_resolution_clock::now();
    durationInitRaptorPQs = std::chrono::duration_cast<std::chrono::milliseconds>(endInitRaptorPQs - initRaptorPQs).count();

    // cout << "upper bound min sum: " << meetingPointQueryResultRaptor.minSumDurationInSeconds << ", upper bound min max: " << meetingPointQueryResultRaptor.minMaxDurationInSeconds << endl;

    auto startRaptorPQ = std::chrono::high_resolution_clock::now();

    raptorPQParallel->initializeHeuristics(sourceStopIdToAllStops, meetingPointQuery.sourceStopIds);
    if (optimization == min_sum) {
        int upperBound = meetingPointQueryResultRaptor.minSumDurationInSeconds;
        raptorPQParallel->setCurrentBest(upperBound);
    } else {
        int upperBound = meetingPointQueryResultRaptor.minMaxDurationInSeconds;
        raptorPQParallel->setCurrentBest(upperBound);
    }
    raptorPQParallel->transformRaptorsToRaptorPQs(raptorQueryProcessor.raptors);
    raptorPQParallel->processRaptorPQ();

    auto endRaptorPQ = std::chrono::high_resolution_clock::now();
    durationRaptorPQs = std::chrono::duration_cast<std::chrono::milliseconds>(endRaptorPQ - startRaptorPQ).count();

    auto startCreateResult = std::chrono::high_resolution_clock::now();

    int minSum = INT_MAX;
    int minMax = INT_MAX;

    int stopIdMinSum = -1;
    int stopIdMinMax = -1;

    // Calculate the sum of the earliest arrival times for all stops and the maximum earliest arrival time for all stops
    for (int i = 0; i < Importer::stops.size(); i++) {
        int sum = 0;
        int max = 0;
        int arrivalTime = 0;
        vector<int> earliestArrivalTimes = raptorPQParallel->getEarliestArrivalTimes(i);
        for (int j = 0; j < earliestArrivalTimes.size(); j++) {
            int earliestArrivalTime = earliestArrivalTimes[j];
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
        vector<Journey> journeysMinSum = raptorPQParallel->createJourneys(stopIdMinSum);
        vector<Journey> journeysMinMax = raptorPQParallel->createJourneys(stopIdMinMax);
        for (int i = 0; i < journeysMinSum.size(); i++) {
            Journey journeyMinSum = journeysMinSum[i];
            Journey journeyMinMax = journeysMinMax[i];

            if (journeyMinSum.legs.size() > 1 && journeyMinSum.legs.size() - 1 > maxTransfersMinSum) {
                maxTransfersMinSum = journeyMinSum.legs.size() - 1;
            }

            if (journeyMinMax.legs.size() > 1 && journeyMinMax.legs.size() - 1 > maxTransfersMinMax) {
                maxTransfersMinMax = journeyMinMax.legs.size() - 1;
            }
        }
        meetingPointQueryResult.maxTransfersMinSum = maxTransfersMinSum;
        meetingPointQueryResult.maxTransfersMinMax = maxTransfersMinMax;
    }

    auto endCreateResult = std::chrono::high_resolution_clock::now();
    durationCreateResult = std::chrono::duration_cast<std::chrono::milliseconds>(endCreateResult - startCreateResult).count();

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    meetingPointQueryResult.queryTime = duration;

    numberOfExpandedRoutes = raptorPQParallel->numberOfExpandedRoutes / meetingPointQuery.sourceStopIds.size();
    durationInitHeuristic = raptorPQParallel->durationInitHeuristic / (meetingPointQuery.sourceStopIds.size() * 1000);
    durationTransformRaptorToRaptorPQ = raptorPQParallel->durationTransformRaptorToRaptorPQ / (meetingPointQuery.sourceStopIds.size() * 1000);
    durationAddRoutesToQueue = raptorPQParallel->durationAddRoutesToQueue / (meetingPointQuery.sourceStopIds.size() * 1000);
    durationGetEarliestTripWithDayOffset = raptorPQParallel->durationGetEarliestTripWithDayOffset / (meetingPointQuery.sourceStopIds.size() * 1000);
    durationTraverseRoute = raptorPQParallel->durationTraverseRoute / (meetingPointQuery.sourceStopIds.size() * 1000);
    altHeuristicImprovementCounter = raptorPQParallel->altHeuristicImprovementCounter / meetingPointQuery.sourceStopIds.size();
    noHeuristicImprovementCounter = raptorPQParallel->noHeuristicImprovementCounter / meetingPointQuery.sourceStopIds.size();
    altHeuristicImprovementFraction = (double) altHeuristicImprovementCounter / (altHeuristicImprovementCounter + noHeuristicImprovementCounter);
}

MeetingPointQueryResult RaptorPQParallelQueryProcessor::getMeetingPointQueryResult() {
    return meetingPointQueryResult; 
}

vector<Journey> RaptorPQParallelQueryProcessor::getJourneys(Optimization optimization) {
    int targetStopId;
    if (optimization == min_sum) {
        targetStopId = meetingPointQueryResult.meetingPointMinSumStopId;
    } else {
        targetStopId = meetingPointQueryResult.meetingPointMinMaxStopId;
    }
    vector<Journey> journeys = raptorPQParallel->createJourneys(targetStopId);
    return journeys;
}

void RaptorApproximationQueryProcessor::processRaptorApproximationQuery(Optimization optimization, bool multipleCandidates) {
    auto start = std::chrono::high_resolution_clock::now();

    this->optimization = optimization;
    
    if (!USE_LANDMARKS) {
        sourceStopIdToAllStops = Creator::networkGraph.getDistancesWithPhast(meetingPointQuery.sourceStopIds);
    }

    int numberOfSources = meetingPointQuery.sourceStopIds.size();
    int numberOfExactSources;
    if (numberOfSources == 2) {
        numberOfExactSources = numberOfSources;
    } else if (numberOfSources < 10) {
        numberOfExactSources = 3;
    } else if (numberOfSources < 25) {
        numberOfExactSources = 4;
    } else {
        numberOfExactSources = 5;
    }
    
    auto startExactSources = std::chrono::high_resolution_clock::now();
    // find the best exact sources (the sources which are the furthest apart using the lower bounds)
    RaptorApproximationQueryProcessor::calculateExactSources(numberOfExactSources);
    auto endExactSources = std::chrono::high_resolution_clock::now();
    durationExactSources = std::chrono::duration_cast<std::chrono::milliseconds>(endExactSources - startExactSources).count();

    MeetingPointQuery meetingPointQueryExact = meetingPointQuery;
    meetingPointQueryExact.sourceStopIds = exactSources;

    auto startExactCalculation = std::chrono::high_resolution_clock::now();
    raptorBoundQueryProcessor = shared_ptr<RaptorBoundQueryProcessor> (new RaptorBoundQueryProcessor(meetingPointQueryExact));
    raptorBoundQueryProcessor->processRaptorBoundQuery(optimization);
    auto endExactCalculation = std::chrono::high_resolution_clock::now();
    durationExactCalculation = std::chrono::duration_cast<std::chrono::milliseconds>(endExactCalculation - startExactCalculation).count();

    auto startCandidates = std::chrono::high_resolution_clock::now();
    if (multipleCandidates) {
        calculateResultWithCandidates();
    } else {
        calculateResultWithOneCandidate();
    }
    auto endCandidates = std::chrono::high_resolution_clock::now();
    durationCandidates = std::chrono::duration_cast<std::chrono::milliseconds>(endCandidates - startCandidates).count();

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    meetingPointQueryResult.queryTime = duration;
}

MeetingPointQueryResult RaptorApproximationQueryProcessor::getMeetingPointQueryResult() {
    return meetingPointQueryResult;
}

void RaptorApproximationQueryProcessor::calculateExactSources(int numberOfExactSourceStops) {
    exactSources = vector<int>(0);

    // calculate the average distance of each source to all other sources
    vector<pair<int, double>> sourceIdToAverageDistance = vector<pair<int, double>>(meetingPointQuery.sourceStopIds.size());
    for (int i = 0; i < meetingPointQuery.sourceStopIds.size(); i++) {
        int sourceId = meetingPointQuery.sourceStopIds[i];
        double averageDistance = 0;
        for (int j = 0; j < meetingPointQuery.sourceStopIds.size(); j++) {
            if (i == j) {
                continue;
            }
            int otherSourceId = meetingPointQuery.sourceStopIds[j];
            int distance;
            if (USE_LANDMARKS) {
                distance = LandmarkProcessor::getLowerBound(sourceId, otherSourceId, meetingPointQuery.weekday);
            } else {
                distance = sourceStopIdToAllStops[sourceId][otherSourceId];
            }
            averageDistance += distance;
        }
        averageDistance = averageDistance / (meetingPointQuery.sourceStopIds.size() - 1);
        sourceIdToAverageDistance[i] = make_pair(sourceId, averageDistance);
    }    

    // sort the sources by average distance
    sort(sourceIdToAverageDistance.begin(), sourceIdToAverageDistance.end(), 
        [](const pair<int, double> &left, const pair<int, double> &right) {
            return left.second > right.second;
        }
    );

    // add the sources with the highest average distance to the exact sources
    for (int i = 0; i < numberOfExactSourceStops; i++) {
        exactSources.push_back(sourceIdToAverageDistance[i].first);
    }
}

void RaptorApproximationQueryProcessor::calculateResultWithCandidates() {
    vector<CandidateInfo> meetingPointsWithSmallerRelativeDifference = raptorBoundQueryProcessor->getStopsAndResultsWithSmallerRelativeDifference(0.1, optimization);

    vector<int> targetStopIds = vector<int>(0);
    for (int i = 0; i < meetingPointsWithSmallerRelativeDifference.size(); i++) {
        targetStopIds.push_back(meetingPointsWithSmallerRelativeDifference[i].stopId);
    }

    int numberOfSources = meetingPointQuery.sourceStopIds.size();

    for (int i = 0; i < numberOfSources; i++) {
        // skip the exact sources
        if (find(exactSources.begin(), exactSources.end(), meetingPointQuery.sourceStopIds[i]) != exactSources.end()) {
            continue;
        }
        RaptorQuery query;
        query.sourceStopId = meetingPointQuery.sourceStopIds[i];
        query.sourceTime = meetingPointQuery.sourceTime;
        query.weekday = meetingPointQuery.weekday;
        query.targetStopIds = targetStopIds;

        shared_ptr<Raptor> raptor = shared_ptr<Raptor> (new Raptor(query));
        raptors.push_back(raptor);
    }

    #pragma omp parallel for
    for (int i = 0; i < raptors.size(); i++) {
        raptors[i]->processRaptor();
    }

    // calculate the best result
    int bestMeetingPoint = -1;
    int bestResult = INT_MAX;
    int bestArrivalTime = 0;

    for (int i = 0; i < meetingPointsWithSmallerRelativeDifference.size(); i++) {
        int currentMeetingPoint = meetingPointsWithSmallerRelativeDifference[i].stopId;
        int currentResult = meetingPointsWithSmallerRelativeDifference[i].duration;
        int arrivalTime = meetingPointsWithSmallerRelativeDifference[i].meetingTime;
        
        for (int j = 0; j < raptors.size(); j++) {
            int earliestArrivalTime = raptors[j]->getEarliestArrivalTime(currentMeetingPoint);
            if (earliestArrivalTime == INT_MAX) {
                currentResult = INT_MAX;
                break;
            }

            int duration = earliestArrivalTime - meetingPointQuery.sourceTime;
            if (optimization == min_sum) {
                currentResult += duration;
            } else {
                if (duration > currentResult) {
                    currentResult = duration;
                }
            }

            if (earliestArrivalTime > arrivalTime) {
                arrivalTime = earliestArrivalTime;
            }
        }

        if (currentResult < bestResult) {
            bestMeetingPoint = currentMeetingPoint;
            bestResult = currentResult;
            bestArrivalTime = arrivalTime;
        }
    }

    if (bestMeetingPoint != -1) {
        if (optimization == min_sum) {
            meetingPointQueryResult.meetingPointMinSumStopId = bestMeetingPoint;
            meetingPointQueryResult.meetingPointMinSum = Importer::getStopName(bestMeetingPoint);
            meetingPointQueryResult.meetingTimeMinSum = TimeConverter::convertSecondsToTime(bestArrivalTime, true);
            meetingPointQueryResult.minSumDuration = TimeConverter::convertSecondsToTime(bestResult, false);
            meetingPointQueryResult.minSumDurationInSeconds = bestResult;
        } else {
            meetingPointQueryResult.meetingPointMinMaxStopId = bestMeetingPoint;
            meetingPointQueryResult.meetingPointMinMax = Importer::getStopName(bestMeetingPoint);
            meetingPointQueryResult.meetingTimeMinMax = TimeConverter::convertSecondsToTime(bestArrivalTime, true);
            meetingPointQueryResult.minMaxDuration = TimeConverter::convertSecondsToTime(bestResult, false);
            meetingPointQueryResult.minMaxDurationInSeconds = bestResult;
        }
    }
}

void RaptorApproximationQueryProcessor::calculateResultWithOneCandidate() {
    MeetingPointQueryResult exactResult = raptorBoundQueryProcessor->getMeetingPointQueryResult();

    vector<int> targetStopIds = vector<int>(0);
    if (optimization == min_sum) {
        targetStopIds.push_back(exactResult.meetingPointMinSumStopId);
    } else {
        targetStopIds.push_back(exactResult.meetingPointMinMaxStopId);
    }

    int numberOfSources = meetingPointQuery.sourceStopIds.size();

    for (int i = 0; i < numberOfSources; i++) {
        // skip the exact sources
        if (find(exactSources.begin(), exactSources.end(), meetingPointQuery.sourceStopIds[i]) != exactSources.end()) {
            continue;
        }
        RaptorQuery query;
        query.sourceStopId = meetingPointQuery.sourceStopIds[i];
        query.sourceTime = meetingPointQuery.sourceTime;
        query.weekday = meetingPointQuery.weekday;
        query.targetStopIds = targetStopIds;

        shared_ptr<RaptorPQStar> raptorPQStar = shared_ptr<RaptorPQStar> (new RaptorPQStar(query, sourceStopIdToAllStops[targetStopIds[0]]));
        raptorPQStars.push_back(raptorPQStar);
    }
    
    #pragma omp parallel for
    for (int i = 0; i < raptorPQStars.size(); i++) {
        raptorPQStars[i]->processRaptorPQStar();
    }

    int bestResult;
    int meetingTime;
    if (optimization == min_sum) {
        bestResult = exactResult.minSumDurationInSeconds;
        meetingTime = exactResult.minSumMeetingTimeInSeconds;
    } else {
        bestResult = exactResult.minMaxDurationInSeconds;
        meetingTime = exactResult.minMaxMeetingTimeInSeconds;
    }

    for (int i = 0; i < raptorPQStars.size(); i++) {
        int earliestArrivalTime = raptorPQStars[i]->getEarliestArrivalTime(targetStopIds[0]);
        if (earliestArrivalTime == INT_MAX) {
            bestResult = INT_MAX;
            break;
        }
        if (earliestArrivalTime > meetingTime) {
            meetingTime = earliestArrivalTime;
        }

        int duration = earliestArrivalTime - meetingPointQuery.sourceTime;
        if (optimization == min_sum) {
            bestResult += duration;
        } else {
            if (duration > bestResult) {
                bestResult = duration;
            }
        }
    }

    if (bestResult != INT_MAX) {
        if (optimization == min_sum) {
            meetingPointQueryResult.meetingPointMinSumStopId = targetStopIds[0];
            meetingPointQueryResult.meetingPointMinSum = Importer::getStopName(targetStopIds[0]);
            meetingPointQueryResult.meetingTimeMinSum = TimeConverter::convertSecondsToTime(meetingTime, true);
            meetingPointQueryResult.minSumDuration = TimeConverter::convertSecondsToTime(bestResult, false);
            meetingPointQueryResult.minSumDurationInSeconds = bestResult;
        } else {
            meetingPointQueryResult.meetingPointMinMaxStopId = targetStopIds[0];
            meetingPointQueryResult.meetingPointMinMax = Importer::getStopName(targetStopIds[0]);
            meetingPointQueryResult.meetingTimeMinMax = TimeConverter::convertSecondsToTime(meetingTime, true);
            meetingPointQueryResult.minMaxDuration = TimeConverter::convertSecondsToTime(bestResult, false);
            meetingPointQueryResult.minMaxDurationInSeconds = bestResult;
        }
    }
}
