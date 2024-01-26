#include "algorithm-tester.h"

#include "query-processor.h"
#include "journey.h"
#include <../data-handling/importer.h>
#include <../data-handling/converter.h>

#include <iostream>

void NaiveAlgorithmTester::testNaiveAlgorithmRandom(int numberOfQueries, int numberOfSources, int numberOfDays, bool printTime, bool printOnlySuccessful) {
    int successfulQueries = 0;
    for (int i = 0; i < numberOfQueries; i++) {
        MeetingPointQuery meetingPointQuery = QueryProcessor::generateRandomMeetingPointQuery(numberOfSources, numberOfDays);
        
        NaiveQueryProcessor naiveQueryProcessor = NaiveQueryProcessor(meetingPointQuery);
        naiveQueryProcessor.processNaiveQuery(printTime);
        MeetingPointQueryResult meetingPointQueryResult = naiveQueryProcessor.getMeetingPointQueryResult();

        if (meetingPointQueryResult.meetingPointMinSum == "" || meetingPointQueryResult.meetingPointMinMax == "") {
            if(printOnlySuccessful) {
                continue;
            }
        } else {
            successfulQueries++;
        }

        PrintHelper::printMeetingPointQuery(meetingPointQuery);
        PrintHelper::printMeetingPointQueryResult(meetingPointQueryResult);
    }

    double rateOfSuccessfulQueries = (double) successfulQueries / numberOfQueries;

    cout << "Rate of successful queries: " << rateOfSuccessfulQueries << endl;
}

void NaiveAlgorithmTester::testNaiveAlgorithm(MeetingPointQuery meetingPointQuery, bool printTime, bool printJourneys) {
    PrintHelper::printMeetingPointQuery(meetingPointQuery);

    NaiveQueryProcessor naiveQueryProcessor = NaiveQueryProcessor(meetingPointQuery);
    naiveQueryProcessor.processNaiveQuery(printTime);
    MeetingPointQueryResult meetingPointQueryResult = naiveQueryProcessor.getMeetingPointQueryResult();
    
    PrintHelper::printMeetingPointQueryResult(meetingPointQueryResult);

    if (printJourneys) {
        vector<Journey> journeysMinSum = naiveQueryProcessor.getJourneys(min_sum);
        vector<Journey> journeysMinMax = naiveQueryProcessor.getJourneys(min_max);

        cout << "Journeys min sum: " << endl;
        for (int i = 0; i < journeysMinSum.size(); i++) {
            PrintHelper::printJourney(journeysMinSum[i]);
        }
        cout << "Journeys min max: " << endl;
        for (int i = 0; i < journeysMinMax.size(); i++) {
            PrintHelper::printJourney(journeysMinMax[i]);
        }
    }
}

void PrintHelper::printMeetingPointQuery(MeetingPointQuery meetingPointQuery) {
    cout << "Source stops: ";
    for (int j = 0; j < meetingPointQuery.sourceStopIds.size()-1; j++) {
        cout << Importer::getStopName(meetingPointQuery.sourceStopIds[j]) << ", ";
    }
    cout << Importer::getStopName(meetingPointQuery.sourceStopIds[meetingPointQuery.sourceStopIds.size()-1]);
    cout << endl;
    cout << "Source time: " << TimeConverter::convertSecondsToTime(meetingPointQuery.sourceTime, true) << endl;
    cout << "Weekday: " << WeekdayConverter::convertIntToWeekday(meetingPointQuery.weekday) << endl;
}

void PrintHelper::printMeetingPointQueryResult(MeetingPointQueryResult meetingPointQueryResult) {
    if (meetingPointQueryResult.meetingPointMinSum == "" || meetingPointQueryResult.meetingPointMinMax == "") {
        cout << "No meeting point found" << endl;
        cout << "Query time: " << meetingPointQueryResult.queryTime << " milliseconds \n" << endl;
        return;
    }
    cout << "Min sum - meeting point: " << meetingPointQueryResult.meetingPointMinSum << ", meeting time: " << meetingPointQueryResult.meetingTimeMinSum << ",  travel time sum: " << meetingPointQueryResult.minSumDuration << endl;
    cout << "Min max - meeting point: " << meetingPointQueryResult.meetingPointMinMax << ", meeting time: " << meetingPointQueryResult.meetingTimeMinMax << ", travel time max: " << meetingPointQueryResult.minMaxDuration << endl;
    cout << "Query time: " << meetingPointQueryResult.queryTime << " milliseconds \n" << endl;
}

void PrintHelper::printJourney(Journey journey) {
    cout << "Journey duration: " << TimeConverter::convertSecondsToTime(journey.duration, false) << endl;
    for (int i = 0; i < journey.legs.size(); i++) {
        cout << "Leg " << i+1 << " - ";
        cout << "Departure stop: " << journey.legs[i].departureStopName;
        cout << ", Departure time: " << TimeConverter::convertSecondsToTime(journey.legs[i].departureTime, true);
        cout << ", Arrival stop: " << journey.legs[i].arrivalStopName;
        cout << ", Arrival time: " << TimeConverter::convertSecondsToTime(journey.legs[i].arrivalTime, true)<< endl;
    }
}