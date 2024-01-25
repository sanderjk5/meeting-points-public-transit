#include "algorithm-tester.h"

#include "query-processor.h"
#include <../data-handling/importer.h>
#include <../data-handling/converter.h>

#include <iostream>

void NaiveAlgorithmTester::testNaiveAlgorithm(int numberOfQueries, int numberOfSources, bool printTime) {
    for (int i = 0; i < numberOfQueries; i++) {
        MeetingPointQuery meetingPointQuery = QueryProcessor::generateRandomMeetingPointQuery(numberOfSources);
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
        continue;
        }
        cout << "Min sum - meeting point: " << meetingPointQueryResult.meetingPointMinSum << ", meeting time: " << meetingPointQueryResult.meetingTimeMinSum << ",  travel time sum: " << meetingPointQueryResult.minSumDuration << endl;
        cout << "Min max - meeting point: " << meetingPointQueryResult.meetingPointMinMax << ", meeting time: " << meetingPointQueryResult.meetingTimeMinMax << ", travel time max: " << meetingPointQueryResult.minMaxDuration << endl;
        cout << "Query time: " << meetingPointQueryResult.queryTime << " milliseconds \n" << endl;
    }
}