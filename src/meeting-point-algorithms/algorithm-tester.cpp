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
        MeetingPointQuery meetingPointQuery = QueryGenerator::generateRandomMeetingPointQuery(numberOfSources, numberOfDays);
        
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
        MeetingPointQuery meetingPointQuery = QueryGenerator::generateRandomMeetingPointQuery(numberOfSources, numberOfDays);
        
        GTreeQueryProcessor gTreeQueryProcessorApproximation = GTreeQueryProcessor(meetingPointQuery, gTree);
        gTreeQueryProcessorApproximation.processGTreeQuery(printTime);
        MeetingPointQueryResult meetingPointQueryResult = gTreeQueryProcessorApproximation.getMeetingPointQueryResult();

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

    GTreeQueryProcessor gTreeQueryProcessorApproximation = GTreeQueryProcessor(meetingPointQuery, gTree);
    gTreeQueryProcessorApproximation.processGTreeQuery(printTime);
    MeetingPointQueryResult meetingPointQueryResult = gTreeQueryProcessorApproximation.getMeetingPointQueryResult();
    
    PrintHelper::printMeetingPointQueryResult(meetingPointQueryResult);

    if (printJourneys) {
        vector<Journey> journeysMinSum = gTreeQueryProcessorApproximation.getJourneys(min_sum);
        vector<Journey> journeysMinMax = gTreeQueryProcessorApproximation.getJourneys(min_max);

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
    int successfulQueriesGTreeCSA = 0;
    int successfulQueriesGTreeApproximation = 0;
    int successfulQueries = 0;

    vector<double> queryTimesNaive;
    vector<double> queryTimesGTreeCSA;
    vector<double> queryTimesGTreeApproximation;

    vector<double> absolutDifferenceMinSum;
    vector<double> absolutDifferenceMinMax;

    vector<double> accuracyMinSum;
    vector<double> accuracyMinMax;

    for (int i = 0; i < numberOfQueries; i++) {
        MeetingPointQuery meetingPointQuery = QueryGenerator::generateRandomMeetingPointQuery(numberOfSources, numberOfDays);
        
        NaiveQueryProcessor naiveQueryProcessor = NaiveQueryProcessor(meetingPointQuery);
        naiveQueryProcessor.processNaiveQuery(printTime);
        MeetingPointQueryResult meetingPointQueryResultNaive = naiveQueryProcessor.getMeetingPointQueryResult();

        GTreeQueryProcessor gTreeQueryProcessorCSA = GTreeQueryProcessor(meetingPointQuery, gTree);
        gTreeQueryProcessorCSA.processGTreeQuery(printTime, true);
        MeetingPointQueryResult meetingPointQueryResultGTreeCSA = gTreeQueryProcessorCSA.getMeetingPointQueryResult();

        GTreeQueryProcessor gTreeQueryProcessorApproximation = GTreeQueryProcessor(meetingPointQuery, gTree);
        gTreeQueryProcessorApproximation.processGTreeQuery(printTime, false);
        MeetingPointQueryResult meetingPointQueryResultGTreeApproximation = gTreeQueryProcessorApproximation.getMeetingPointQueryResult();

        bool naiveQuerySuccessful = meetingPointQueryResultNaive.meetingPointMinSum != "" && meetingPointQueryResultNaive.meetingPointMinMax != "";
        bool gTreeCSAQuerySuccessful = meetingPointQueryResultGTreeCSA.meetingPointMinSum != "" && meetingPointQueryResultGTreeCSA.meetingPointMinMax != "";
        bool gTreeApproximationQuerySuccessful = meetingPointQueryResultGTreeApproximation.meetingPointMinSum != "" && meetingPointQueryResultGTreeApproximation.meetingPointMinMax != "";

        if(naiveQuerySuccessful){
            successfulQueriesNaive++;
        }

        if(gTreeCSAQuerySuccessful){
            successfulQueriesGTreeCSA++;
        }

        if(gTreeApproximationQuerySuccessful){
            successfulQueriesGTreeApproximation++;
        }

        if (!naiveQuerySuccessful || !gTreeCSAQuerySuccessful || !gTreeApproximationQuerySuccessful) {
            continue;
        }

        successfulQueries++;

        queryTimesNaive.push_back((double) meetingPointQueryResultNaive.queryTime);
        queryTimesGTreeCSA.push_back((double) meetingPointQueryResultGTreeCSA.queryTime);
        queryTimesGTreeApproximation.push_back((double) meetingPointQueryResultGTreeApproximation.queryTime);

        int differenceMinSum = meetingPointQueryResultGTreeApproximation.minSumDurationInSeconds - meetingPointQueryResultNaive.minSumDurationInSeconds;
        int differenceMinMax = meetingPointQueryResultGTreeApproximation.minMaxDurationInSeconds - meetingPointQueryResultNaive.minMaxDurationInSeconds;

        absolutDifferenceMinSum.push_back((double) differenceMinSum);
        absolutDifferenceMinMax.push_back((double) differenceMinMax);

        double relativeDifferenceMinSum = (double) differenceMinSum / meetingPointQueryResultNaive.minSumDurationInSeconds;
        double relativeDifferenceMinMax = (double) differenceMinMax / meetingPointQueryResultNaive.minMaxDurationInSeconds;

        accuracyMinSum.push_back(1 - relativeDifferenceMinSum);
        accuracyMinMax.push_back(1 - relativeDifferenceMinMax);


        if (printEveryResult) {
            PrintHelper::printMeetingPointQuery(meetingPointQuery);
            cout << "Naive: " << endl;
            PrintHelper::printMeetingPointQueryResult(meetingPointQueryResultNaive);

            cout << "GTree (csa): " << endl;
            PrintHelper::printMeetingPointQueryResult(meetingPointQueryResultGTreeCSA);

            cout << "GTree (approximation): " << endl;
            PrintHelper::printMeetingPointQueryResult(meetingPointQueryResultGTreeApproximation);
        }
    }

    double rateOfSuccessfulQueriesNaive = (double) successfulQueriesNaive / numberOfQueries;
    double rateOfSuccessfulCSAQueriesGTree = (double) successfulQueriesGTreeCSA / numberOfQueries;
    double rateOfSuccessfulApproximationQueriesGTree = (double) successfulQueriesGTreeApproximation / numberOfQueries;
    double rateOfSuccessfulQueries = (double) successfulQueries / numberOfQueries;

    cout << "\nCompare naive and g-tree algorithm: \n" << endl;
    cout << "Rate of successful queries naive: " << rateOfSuccessfulQueriesNaive << endl;
    cout << "Rate of successful queries gTree (csa): " << rateOfSuccessfulCSAQueriesGTree << endl;
    cout << "Rate of successful queries gTree (approximation): " << rateOfSuccessfulApproximationQueriesGTree << endl;
    cout << "Rate of successful queries for all of them: " << rateOfSuccessfulQueries << endl;

    cout << "\nQuery times:" << endl;
    cout << "Average query time naive: " << Calculator::getAverage(queryTimesNaive) << " milliseconds" << endl;
    cout << "Average query time gTree (csa): " << Calculator::getAverage(queryTimesGTreeCSA) << " milliseconds" << endl;
    cout << "Average query time gTree (approximation): " << Calculator::getAverage(queryTimesGTreeApproximation) << " milliseconds" << endl;
    cout << "Median query time naive: " << Calculator::getMedian(queryTimesNaive) << " milliseconds" << endl;
    cout << "Median query time gTree (csa): " << Calculator::getMedian(queryTimesGTreeCSA) << " milliseconds" << endl;
    cout << "Median query time gTree (approximation): " << Calculator::getMedian(queryTimesGTreeApproximation) << " milliseconds" << endl;
    cout << "Maximum query time naive: " << Calculator::getMaximum(queryTimesNaive) << " milliseconds" << endl;
    cout << "Maximum query time gTree (csa): " << Calculator::getMaximum(queryTimesGTreeCSA) << " milliseconds" << endl;
    cout << "Maximum query time gTree (approximation): " << Calculator::getMaximum(queryTimesGTreeApproximation) << " milliseconds" << endl;
    cout << "Minimum query time naive: " << Calculator::getMinimum(queryTimesNaive) << " milliseconds" << endl;
    cout << "Minimum query time gTree (csa): " << Calculator::getMinimum(queryTimesGTreeCSA) << " milliseconds" << endl;
    cout << "Minimum query time gTree (approximation): " << Calculator::getMinimum(queryTimesGTreeApproximation) << " milliseconds" << endl;

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
    cout << "Average accuracy min sum: " << Calculator::getAverage(accuracyMinSum) << endl;
    cout << "Average accuracy min max: " << Calculator::getAverage(accuracyMinMax) << endl;
    cout << "Median accuracy min sum: " << Calculator::getMedian(accuracyMinSum) << endl;
    cout << "Median accuracy min max: " << Calculator::getMedian(accuracyMinMax) << endl;
    cout << "Maximum accuracy min sum: " << Calculator::getMaximum(accuracyMinSum) << endl;
    cout << "Maximum accuracy min max: " << Calculator::getMaximum(accuracyMinMax) << endl;
    cout << "Minimum accuracy min sum: " << Calculator::getMinimum(accuracyMinSum) << endl;
    cout << "Minimum accuracy min max: " << Calculator::getMinimum(accuracyMinMax) << endl;
}

void AlgorithmComparer::compareAlgorithms(GTree* gTree, MeetingPointQuery meetingPointQuery, bool printTime){
    NaiveQueryProcessor naiveQueryProcessor = NaiveQueryProcessor(meetingPointQuery);
    naiveQueryProcessor.processNaiveQuery(printTime);
    MeetingPointQueryResult meetingPointQueryResultNaive = naiveQueryProcessor.getMeetingPointQueryResult();

    GTreeQueryProcessor gTreeQueryProcessorCSA = GTreeQueryProcessor(meetingPointQuery, gTree);
    gTreeQueryProcessorCSA.processGTreeQuery(printTime, true);
    MeetingPointQueryResult meetingPointQueryResultGTreeCSA = gTreeQueryProcessorCSA.getMeetingPointQueryResult();

    GTreeQueryProcessor gTreeQueryProcessorApproximation = GTreeQueryProcessor(meetingPointQuery, gTree);
    gTreeQueryProcessorApproximation.processGTreeQuery(printTime, false);
    MeetingPointQueryResult meetingPointQueryResultGTreeApproximation = gTreeQueryProcessorApproximation.getMeetingPointQueryResult();

    PrintHelper::printMeetingPointQuery(meetingPointQuery);
    cout << "Naive: " << endl;
    PrintHelper::printMeetingPointQueryResult(meetingPointQueryResultNaive);

    cout << "GTree - CSA: " << endl;
    PrintHelper::printMeetingPointQueryResult(meetingPointQueryResultGTreeCSA);

    cout << "GTree - Approximation: " << endl;
    PrintHelper::printMeetingPointQueryResult(meetingPointQueryResultGTreeApproximation);

    bool naiveQuerySuccessful = meetingPointQueryResultNaive.meetingPointMinSum != "" && meetingPointQueryResultNaive.meetingPointMinMax != "";
    bool gTreeApproximationQuerySuccessful = meetingPointQueryResultGTreeApproximation.meetingPointMinSum != "" && meetingPointQueryResultGTreeApproximation.meetingPointMinMax != "";

    if(naiveQuerySuccessful && gTreeApproximationQuerySuccessful) {
        double absolutDifferenceMinSum = (double) (meetingPointQueryResultGTreeApproximation.minSumDurationInSeconds - meetingPointQueryResultNaive.minSumDurationInSeconds);
        double absolutDifferenceMinMax = (double) (meetingPointQueryResultGTreeApproximation.minMaxDurationInSeconds - meetingPointQueryResultNaive.minMaxDurationInSeconds);

        double accuracyMinSum = (double) 1 - (absolutDifferenceMinSum / meetingPointQueryResultNaive.minSumDurationInSeconds);
        double accuracyMinMax = (double) 1 - (absolutDifferenceMinMax / meetingPointQueryResultNaive.minMaxDurationInSeconds);

        cout << "Result differences:" << endl;
        cout << "Absolut difference min sum: " << absolutDifferenceMinSum / 60 << " minutes" << endl;
        cout << "Absolut difference min max: " << absolutDifferenceMinMax / 60 << " minutes" << endl;
        cout << "Accuracy min sum: " << accuracyMinSum << endl;
        cout << "Accuracy min max: " << accuracyMinMax << endl;
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