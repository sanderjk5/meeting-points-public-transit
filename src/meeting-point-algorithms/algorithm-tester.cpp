#include "algorithm-tester.h"

#include "query-processor.h"
#include "journey.h"
#include <../data-handling/importer.h>
#include <../data-handling/converter.h>
#include <../data-structures/g-tree.h>
#include <algorithm>

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

void GTreeAlgorithmTester::testGTreeAlgorithmRandom(GTree* gTree, int numberOfQueries, int numberOfSources, int numberOfDays, bool printTime, bool printOnlySuccessful) {
    int successfulQueries = 0;
    for (int i = 0; i < numberOfQueries; i++) {
        MeetingPointQuery meetingPointQuery = QueryProcessor::generateRandomMeetingPointQuery(numberOfSources, numberOfDays);
        
        GTreeQueryProcessor gTreeQueryProcessor = GTreeQueryProcessor(meetingPointQuery, gTree);
        gTreeQueryProcessor.processGTreeQuery(printTime);
        MeetingPointQueryResult meetingPointQueryResult = gTreeQueryProcessor.getMeetingPointQueryResult();

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

void GTreeAlgorithmTester::testGTreeAlgorithm(GTree* gTree, MeetingPointQuery meetingPointQuery, bool printTime, bool printJourneys) {
    PrintHelper::printMeetingPointQuery(meetingPointQuery);

    GTreeQueryProcessor gTreeQueryProcessor = GTreeQueryProcessor(meetingPointQuery, gTree);
    gTreeQueryProcessor.processGTreeQuery(printTime);
    MeetingPointQueryResult meetingPointQueryResult = gTreeQueryProcessor.getMeetingPointQueryResult();
    
    PrintHelper::printMeetingPointQueryResult(meetingPointQueryResult);

    if (printJourneys) {
        vector<Journey> journeysMinSum = gTreeQueryProcessor.getJourneys(min_sum);
        vector<Journey> journeysMinMax = gTreeQueryProcessor.getJourneys(min_max);

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

void AlgorithmComparer::compareAlgorithmsRandom(GTree* gTree, int numberOfQueries, int numberOfSources, int numberOfDays, bool printTime, bool printEveryResult) {
    int successfulQueriesNaive = 0;
    int successfulQueriesGTree = 0;
    int successfulQueriesBoth = 0;

    vector<double> queryTimesNaive;
    vector<double> queryTimesGTree;

    vector<double> absolutDifferenceMinSum;
    vector<double> absolutDifferenceMinMax;

    vector<double> relativeDifferenceMinSum;
    vector<double> relativeDifferenceMinMax;

    for (int i = 0; i < numberOfQueries; i++) {
        MeetingPointQuery meetingPointQuery = QueryProcessor::generateRandomMeetingPointQuery(numberOfSources, numberOfDays);
        
        NaiveQueryProcessor naiveQueryProcessor = NaiveQueryProcessor(meetingPointQuery);
        naiveQueryProcessor.processNaiveQuery(printTime);
        MeetingPointQueryResult meetingPointQueryResultNaive = naiveQueryProcessor.getMeetingPointQueryResult();

        GTreeQueryProcessor gTreeQueryProcessor = GTreeQueryProcessor(meetingPointQuery, gTree);
        gTreeQueryProcessor.processGTreeQuery(printTime);
        MeetingPointQueryResult meetingPointQueryResultGTree = gTreeQueryProcessor.getMeetingPointQueryResult();

        bool naiveQuerySuccessful = meetingPointQueryResultNaive.meetingPointMinSum != "" && meetingPointQueryResultNaive.meetingPointMinMax != "";
        bool gTreeQuerySuccessful = meetingPointQueryResultGTree.meetingPointMinSum != "" && meetingPointQueryResultGTree.meetingPointMinMax != "";

        if(naiveQuerySuccessful){
            successfulQueriesNaive++;
        }

        if(gTreeQuerySuccessful){
            successfulQueriesGTree++;
        }

        if (!naiveQuerySuccessful && !gTreeQuerySuccessful) {
            continue;
        }

        successfulQueriesBoth++;

        queryTimesNaive.push_back((double) meetingPointQueryResultNaive.queryTime);
        queryTimesGTree.push_back((double) meetingPointQueryResultGTree.queryTime);

        int differenceMinSum = meetingPointQueryResultGTree.minSumDurationInSeconds - meetingPointQueryResultNaive.minSumDurationInSeconds;
        int differenceMinMax = meetingPointQueryResultGTree.minMaxDurationInSeconds - meetingPointQueryResultNaive.minMaxDurationInSeconds;

        absolutDifferenceMinSum.push_back((double) differenceMinSum);
        absolutDifferenceMinMax.push_back((double) differenceMinMax);

        relativeDifferenceMinSum.push_back((double) differenceMinSum / meetingPointQueryResultNaive.minSumDurationInSeconds);
        relativeDifferenceMinMax.push_back((double) differenceMinMax / meetingPointQueryResultNaive.minMaxDurationInSeconds);


        if (printEveryResult) {
            PrintHelper::printMeetingPointQuery(meetingPointQuery);
            cout << "Naive: " << endl;
            PrintHelper::printMeetingPointQueryResult(meetingPointQueryResultNaive);

            cout << "GTree: " << endl;
            PrintHelper::printMeetingPointQueryResult(meetingPointQueryResultGTree);
        }
    }

    double rateOfSuccessfulQueriesNaive = (double) successfulQueriesNaive / numberOfQueries;
    double rateOfSuccessfulQueriesGTree = (double) successfulQueriesGTree / numberOfQueries;
    double rateOfSuccessfulQueriesBoth = (double) successfulQueriesBoth / numberOfQueries;

    cout << "\nCompare naive and g-tree algorithm: \n" << endl;
    cout << "Rate of successful queries naive: " << rateOfSuccessfulQueriesNaive << endl;
    cout << "Rate of successful queries gTree: " << rateOfSuccessfulQueriesGTree << endl;
    cout << "Rate of successful queries both: " << rateOfSuccessfulQueriesBoth << endl;

    cout << "\nQuery times:" << endl;
    cout << "Average query time naive: " << Calculator::getAverage(queryTimesNaive) << " milliseconds" << endl;
    cout << "Average query time gTree: " << Calculator::getAverage(queryTimesGTree) << " milliseconds" << endl;
    cout << "Median query time naive: " << Calculator::getMedian(queryTimesNaive) << " milliseconds" << endl;
    cout << "Median query time gTree: " << Calculator::getMedian(queryTimesGTree) << " milliseconds" << endl;
    cout << "Maximum query time naive: " << Calculator::getMaximum(queryTimesNaive) << " milliseconds" << endl;
    cout << "Maximum query time gTree: " << Calculator::getMaximum(queryTimesGTree) << " milliseconds" << endl;
    cout << "Minimum query time naive: " << Calculator::getMinimum(queryTimesNaive) << " milliseconds" << endl;
    cout << "Minimum query time gTree: " << Calculator::getMinimum(queryTimesGTree) << " milliseconds" << endl;

    cout << "\nAbsolut result differences:" << endl;
    cout << "Average absolut difference min sum: " << Calculator::getAverage(absolutDifferenceMinSum) / 60 << " minutes" << endl;
    cout << "Average absolut difference min max: " << Calculator::getAverage(absolutDifferenceMinMax) / 60 << " minutes" << endl;
    cout << "Median absolut difference min sum: " << Calculator::getMedian(absolutDifferenceMinSum) / 60 << " minutes" << endl;
    cout << "Median absolut difference min max: " << Calculator::getMedian(absolutDifferenceMinMax) / 60 << " minutes" << endl;
    cout << "Maximum absolut difference min sum: " << Calculator::getMaximum(absolutDifferenceMinSum) / 60 << " minutes" << endl;
    cout << "Maximum absolut difference min max: " << Calculator::getMaximum(absolutDifferenceMinMax) / 60 << " minutes" << endl;
    cout << "Minimum absolut difference min sum: " << Calculator::getMinimum(absolutDifferenceMinSum) / 60 << " minutes" << endl;
    cout << "Minimum absolut difference min max: " << Calculator::getMinimum(absolutDifferenceMinMax) / 60 << " minutes" << endl;

    cout << "\nRelative result differences:" << endl;
    cout << "Average relative difference min sum: " << Calculator::getAverage(relativeDifferenceMinSum) << endl;
    cout << "Average relative difference min max: " << Calculator::getAverage(relativeDifferenceMinMax) << endl;
    cout << "Median relative difference min sum: " << Calculator::getMedian(relativeDifferenceMinSum) << endl;
    cout << "Median relative difference min max: " << Calculator::getMedian(relativeDifferenceMinMax) << endl;
    cout << "Maximum relative difference min sum: " << Calculator::getMaximum(relativeDifferenceMinSum) << endl;
    cout << "Maximum relative difference min max: " << Calculator::getMaximum(relativeDifferenceMinMax) << endl;
    cout << "Minimum relative difference min sum: " << Calculator::getMinimum(relativeDifferenceMinSum) << endl;
    cout << "Minimum relative difference min max: " << Calculator::getMinimum(relativeDifferenceMinMax) << endl;
}

void AlgorithmComparer::compareAlgorithms(GTree* gTree, MeetingPointQuery meetingPointQuery, bool printTime){
    NaiveQueryProcessor naiveQueryProcessor = NaiveQueryProcessor(meetingPointQuery);
    naiveQueryProcessor.processNaiveQuery(printTime);
    MeetingPointQueryResult meetingPointQueryResultNaive = naiveQueryProcessor.getMeetingPointQueryResult();

    GTreeQueryProcessor gTreeQueryProcessor = GTreeQueryProcessor(meetingPointQuery, gTree);
    gTreeQueryProcessor.processGTreeQuery(printTime);
    MeetingPointQueryResult meetingPointQueryResultGTree = gTreeQueryProcessor.getMeetingPointQueryResult();

    PrintHelper::printMeetingPointQuery(meetingPointQuery);
    cout << "Naive: " << endl;
    PrintHelper::printMeetingPointQueryResult(meetingPointQueryResultNaive);

    cout << "GTree: " << endl;
    PrintHelper::printMeetingPointQueryResult(meetingPointQueryResultGTree);

    bool naiveQuerySuccessful = meetingPointQueryResultNaive.meetingPointMinSum != "" && meetingPointQueryResultNaive.meetingPointMinMax != "";
    bool gTreeQuerySuccessful = meetingPointQueryResultGTree.meetingPointMinSum != "" && meetingPointQueryResultGTree.meetingPointMinMax != "";

    if(naiveQuerySuccessful && gTreeQuerySuccessful) {
        double absolutDifferenceMinSum = (double) (meetingPointQueryResultGTree.minSumDurationInSeconds - meetingPointQueryResultNaive.minSumDurationInSeconds) / 60;
        double absolutDifferenceMinMax = (double) (meetingPointQueryResultGTree.minMaxDurationInSeconds - meetingPointQueryResultNaive.minMaxDurationInSeconds) / 60;

        double relativeDifferenceMinSum = (double) absolutDifferenceMinSum / meetingPointQueryResultNaive.minSumDurationInSeconds;
        double relativeDifferenceMinMax = (double) absolutDifferenceMinMax / meetingPointQueryResultNaive.minMaxDurationInSeconds;

        cout << "Result differences:" << endl;
        cout << "Absolut difference min sum: " << absolutDifferenceMinSum << " minutes" << endl;
        cout << "Absolut difference min max: " << absolutDifferenceMinMax << " minutes" << endl;
        cout << "Relative difference min sum: " << relativeDifferenceMinSum << endl;
        cout << "Relative difference min max: " << relativeDifferenceMinMax << endl;
    }
}

double Calculator::getAverage(vector<double> numbers) {
    double sum = 0;
    for (int i = 0; i < numbers.size(); i++) {
        sum += numbers[i];
    }
    return sum / numbers.size();
}

double Calculator::getMedian(vector<double> numbers) {
    sort(numbers.begin(), numbers.end());
    int size = numbers.size();
    if (size % 2 == 0) {
        return (numbers[size / 2 - 1] + numbers[size / 2]) / 2;
    } else {
        return numbers[size / 2];
    }
}

double Calculator::getMaximum(vector<double> numbers) {
    double max = numbers[0];
    for (int i = 1; i < numbers.size(); i++) {
        if (numbers[i] > max) {
            max = numbers[i];
        }
    }
    return max;
}

double Calculator::getMinimum(vector<double> numbers) {
    double min = numbers[0];
    for (int i = 1; i < numbers.size(); i++) {
        if (numbers[i] < min) {
            min = numbers[i];
        }
    }
    return min;
}

void PrintHelper::printMeetingPointQuery(MeetingPointQuery meetingPointQuery) {
    cout << "\nSource stops: ";
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