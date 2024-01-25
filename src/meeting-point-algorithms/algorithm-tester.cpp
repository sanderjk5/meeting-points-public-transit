#include "algorithm-tester.h"

#include "query-processor.h"
#include <../data-handling/importer.h>
#include <../data-handling/converter.h>

#include <iostream>

void NaiveAlgorithmTester::testNaiveAlgorithmRandom(int numberOfQueries, int numberOfSources, int numberOfDays, bool printTime, bool printOnlySuccessful) {
    int successfulQueries = 0;
    for (int i = 0; i < numberOfQueries; i++) {
        MeetingPointQuery meetingPointQuery = QueryProcessor::generateRandomMeetingPointQuery(numberOfSources, numberOfDays);
        

        MeetingPointQueryResult meetingPointQueryResult = QueryProcessor::processNaiveQuery(meetingPointQuery, printTime);

        if (printOnlySuccessful && (meetingPointQueryResult.meetingPointMinSum == "" || meetingPointQueryResult.meetingPointMinMax == "")) {
            continue;
        }


        cout << "Source stops: ";
        for (int j = 0; j < meetingPointQuery.sourceStopIds.size()-1; j++) {
        cout << Importer::getStopName(meetingPointQuery.sourceStopIds[j]) << ", ";
        }
        cout << Importer::getStopName(meetingPointQuery.sourceStopIds[meetingPointQuery.sourceStopIds.size()-1]);
        cout << endl;
        cout << "Source time: " << TimeConverter::convertSecondsToTime(meetingPointQuery.sourceTime, true) << endl;
        cout << "Weekday: " << WeekdayConverter::convertIntToWeekday(meetingPointQuery.weekday) << endl;
        if (meetingPointQueryResult.meetingPointMinSum == "" || meetingPointQueryResult.meetingPointMinMax == "") {
            cout << "No meeting point found" << endl;
            cout << "Query time: " << meetingPointQueryResult.queryTime << " milliseconds \n" << endl;
            continue;
        }
        successfulQueries++;
        cout << "Min sum - meeting point: " << meetingPointQueryResult.meetingPointMinSum << ", meeting time: " << meetingPointQueryResult.meetingTimeMinSum << ",  travel time sum: " << meetingPointQueryResult.minSumDuration << endl;
        cout << "Min max - meeting point: " << meetingPointQueryResult.meetingPointMinMax << ", meeting time: " << meetingPointQueryResult.meetingTimeMinMax << ", travel time max: " << meetingPointQueryResult.minMaxDuration << endl;
        cout << "Query time: " << meetingPointQueryResult.queryTime << " milliseconds \n" << endl;
    }

    cout << "Number of successful queries: " << successfulQueries << endl;
}

void NaiveAlgorithmTester::testNaiveAlgorithm(MeetingPointQuery meetingPointQuery, bool printTime) {
    cout << "Source stops: ";
    for (int j = 0; j < meetingPointQuery.sourceStopIds.size()-1; j++) {
        cout << Importer::getStopName(meetingPointQuery.sourceStopIds[j]) << ", ";
    }
    cout << Importer::getStopName(meetingPointQuery.sourceStopIds[meetingPointQuery.sourceStopIds.size()-1]);
    cout << endl;
    cout << "Source time: " << TimeConverter::convertSecondsToTime(meetingPointQuery.sourceTime, true) << endl;
    cout << "Weekday: " << WeekdayConverter::convertIntToWeekday(meetingPointQuery.weekday) << endl;

    MeetingPointQueryResult meetingPointQueryResult = QueryProcessor::processNaiveQuery(meetingPointQuery, printTime);

    if (meetingPointQueryResult.meetingPointMinSum == "" || meetingPointQueryResult.meetingPointMinMax == "") {
        cout << "No meeting point found" << endl;
        cout << "Query time: " << meetingPointQueryResult.queryTime << " milliseconds \n" << endl;
        return;
    }
    cout << "Min sum - meeting point: " << meetingPointQueryResult.meetingPointMinSum << ", meeting time: " << meetingPointQueryResult.meetingTimeMinSum << ",  travel time sum: " << meetingPointQueryResult.minSumDuration << endl;
    cout << "Min max - meeting point: " << meetingPointQueryResult.meetingPointMinMax << ", meeting time: " << meetingPointQueryResult.meetingTimeMinMax << ", travel time max: " << meetingPointQueryResult.minMaxDuration << endl;
    cout << "Query time: " << meetingPointQueryResult.queryTime << " milliseconds \n" << endl;
}