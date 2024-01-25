#include "query-processor.h"

#include "csa.h"
#include <../data-handling/importer.h>
#include <../data-handling/converter.h>
#include <../constants.h>
#include <limits.h>

#include <iostream>
#include <chrono>

#include <vector>
#include <string>

using namespace std;

MeetingPointQueryResult QueryProcessor::processNaiveQuery(MeetingPointQuery meetingPointQuery, bool printTime) {
    auto start = std::chrono::high_resolution_clock::now();

    MeetingPointQueryResult meetingPointQueryResult;

    vector<CSA*> csas;

    for (int i = 0; i < meetingPointQuery.sourceStopIds.size(); i++) {
        CSAQuery query;
        query.sourceStopId = meetingPointQuery.sourceStopIds[i];
        query.sourceTime = meetingPointQuery.sourceTime;
        query.weekday = meetingPointQuery.weekday;
        query.numberOfDays = meetingPointQuery.numberOfDays;
        CSA* csa = new CSA(query);
        csa->processCSA(printTime);
        csas.push_back(csa);
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

    for (CSA* csa : csas) {
        delete csa;
    }
    csas.clear();

    // Stop the timer and calculate the duration
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

    // Print the duration
    if (printTime) {
        cout << "Naive Query duration : " << duration << " milliseconds" << endl;
    }

    meetingPointQueryResult.queryTime = to_string(duration);
    
    return meetingPointQueryResult;
}

MeetingPointQuery QueryProcessor::generateRandomMeetingPointQuery(int numberOfSources, int numberOfDays) {
    MeetingPointQuery meetingPointQuery;
    for (int i = 0; i < numberOfSources; i++) {
        meetingPointQuery.sourceStopIds.push_back(rand() % Importer::stops.size());
    }
    meetingPointQuery.sourceTime = rand() % SECONDSPERDAY;
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