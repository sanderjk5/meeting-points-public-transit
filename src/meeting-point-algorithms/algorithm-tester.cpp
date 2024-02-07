#include "algorithm-tester.h"

#include "query-processor.h"
#include "journey.h"
#include <../data-handling/importer.h>
#include <../data-handling/converter.h>
#include <../data-structures/g-tree.h>
#include <algorithm>

#include <iostream>
#include <fstream>

using namespace std;

void NaiveAlgorithmTester::testNaiveAlgorithmRandom(int numberOfSuccessfulQueries, int numberOfSources, int numberOfDays, bool printOnlySuccessful) {
    int successfulQueryCounter = 0;
    for (int i = 0; i < numberOfSuccessfulQueries; i++) {
        MeetingPointQuery meetingPointQuery = QueryGenerator::generateRandomMeetingPointQuery(numberOfSources, numberOfDays);
        
        NaiveQueryProcessor naiveQueryProcessor = NaiveQueryProcessor(meetingPointQuery);
        naiveQueryProcessor.processNaiveQuery();
        MeetingPointQueryResult meetingPointQueryResult = naiveQueryProcessor.getMeetingPointQueryResult();

        if (meetingPointQueryResult.meetingPointMinSum == "" || meetingPointQueryResult.meetingPointMinMax == "") {
            if(printOnlySuccessful) {
                continue;
            }
        } else {
            successfulQueryCounter++;
        }

        PrintHelper::printMeetingPointQuery(meetingPointQuery);
        PrintHelper::printMeetingPointQueryResult(meetingPointQueryResult);
    }

    double rateOfSuccessfulQueries = (double) successfulQueryCounter / numberOfSuccessfulQueries;

    cout << "Rate of successful queries: " << rateOfSuccessfulQueries << endl;
}

void NaiveAlgorithmTester::testNaiveAlgorithm(MeetingPointQuery meetingPointQuery, bool printJourneys) {
    PrintHelper::printMeetingPointQuery(meetingPointQuery);

    NaiveQueryProcessor naiveQueryProcessor = NaiveQueryProcessor(meetingPointQuery);
    naiveQueryProcessor.processNaiveQuery();
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

void GTreeAlgorithmTester::testGTreeAlgorithmRandom(GTree* gTree, int numberOfSuccessfulQueries, int numberOfSources, int numberOfDays, bool printOnlySuccessful) {
    int successfulQueryCounter = 0;
    for (int i = 0; i < numberOfSuccessfulQueries; i++) {
        MeetingPointQuery meetingPointQuery = QueryGenerator::generateRandomMeetingPointQuery(numberOfSources, numberOfDays);
        
        GTreeQueryProcessor gTreeQueryProcessorApproximation = GTreeQueryProcessor(meetingPointQuery, gTree);
        gTreeQueryProcessorApproximation.processGTreeQuery();
        MeetingPointQueryResult meetingPointQueryResult = gTreeQueryProcessorApproximation.getMeetingPointQueryResult();

        if (meetingPointQueryResult.meetingPointMinSum == "" || meetingPointQueryResult.meetingPointMinMax == "") {
            if(printOnlySuccessful) {
                continue;
            }
        } else {
            successfulQueryCounter++;
        }

        PrintHelper::printMeetingPointQuery(meetingPointQuery);
        PrintHelper::printMeetingPointQueryResult(meetingPointQueryResult);
    }

    double rateOfSuccessfulQueries = (double) successfulQueryCounter / numberOfSuccessfulQueries;

    cout << "Rate of successful queries: " << rateOfSuccessfulQueries << endl;
}

void GTreeAlgorithmTester::testGTreeAlgorithm(GTree* gTree, MeetingPointQuery meetingPointQuery, bool printJourneys) {
    PrintHelper::printMeetingPointQuery(meetingPointQuery);

    GTreeQueryProcessor gTreeQueryProcessorApproximation = GTreeQueryProcessor(meetingPointQuery, gTree);
    gTreeQueryProcessorApproximation.processGTreeQuery();
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

void AlgorithmComparer::compareAlgorithmsRandom(GTree* gTree, int numberOfSuccessfulQueries, vector<int> numberOfSources, int numberOfDays, bool printResults, bool loadOrStoreQueries) {
    string folderPathResults = "../../tests/results/";
    string folderPathQueries = "../../tests/queries/";
    // get the current timestamp
    time_t now = time(0);
    tm *ltm = localtime(&now);
    string timestamp = to_string(1900 + ltm->tm_year) + "-" + to_string(1 + ltm->tm_mon) + "-" + to_string(ltm->tm_mday) + "-" + to_string(ltm->tm_hour) + "-" + to_string(ltm->tm_min) + "-" + to_string(ltm->tm_sec);
    
    string resultsFileName = folderPathResults + "results-" + timestamp + ".csv";
    
    // Create a csv file to store the results
    std::ofstream resultsFile (resultsFileName, std::ofstream::out);
    resultsFile << "numberOfSourceStops,avgQueryTimeNaive,avgQueryTimeGTreeCSA,avgQueryTimeGTreeApprox,medianQueryTimeNaive,medianQueryTimeGTreeCSA,medianQueryTimeGTreeApprox,maxQueryTimeNaive,maxQueryTimeGTreeCSA,maxQueryTimeGTreeApprox,minQueryTimeNaive,minQueryTimeGTreeCSA,minQueryTimeGTreeApprox,avgAbsDiffMinSum,avgAbsDiffMinMax,medianAbsDiffMinSum,medianAbsDiffMinMax,maxAbsDiffMinSum,maxAbsDiffMinMax,minAbsDiffMinSum,minAbsDiffMinMax,avgAccMinSum,avgAccMinMax,medianAccMinSum,medianAccMinMax,maxAccMinSum,maxAccMinMax,minAccMinSum,minAccMinMax\n";
    
    for (int i = 0; i < numberOfSources.size(); i++) {
        int numberOfSourceStops = numberOfSources[i];

        cout << "\nProcess queries for " << numberOfSourceStops << " source stops..." << endl;

        string numberOfSourceStopsString = "";
        if (numberOfSourceStops < 10) {
            numberOfSourceStopsString = "00" + to_string(numberOfSourceStops);
        } else if (numberOfSourceStops < 100) {
            numberOfSourceStopsString = "0" + to_string(numberOfSourceStops);
        } else {
            numberOfSourceStopsString = to_string(numberOfSourceStops);
        }

        std::ofstream queriesInfoFile;
        vector<MeetingPointQuery> meetingPointQueries;
        if (loadOrStoreQueries) {
            string filePath = folderPathQueries + "meeting-point-query-" + numberOfSourceStopsString + "-" + to_string(numberOfSuccessfulQueries) + ".csv";
            std::ifstream file(filePath);
            if (file.is_open()) {
                std::string line;
                while (std::getline(file, line)) {
                    meetingPointQueries.push_back(QueryGenerator::parseMeetingPointQuery(line, numberOfSourceStops));
                }
            } else {
                queriesInfoFile.open(filePath, std::ofstream::out);
            }
        }

        // Create a csv file to store the queries
        string queriesFileName = folderPathResults + "queries-" + to_string(numberOfSourceStops) + "-" + timestamp + ".csv";
        std::ofstream queriesFile (queriesFileName, std::ofstream::out);
        queriesFile << "sourceStopIds,sourceTime,weekday,queryTimeNaive,queryTimeGTreeCSA,queryTimeGTreeApprox,absolutDifferenceMinSum,absolutDifferenceMinMax,accuracyMinSum,accuracyMinMax\n";

        int successfulQueriesNaive = 0;
        int successfulQueriesGTreeCSA = 0;
        int successfulQueriesGTreeApproximation = 0;

        int queryCounter = 0;
        int successfulQueryCounter = 0;

        vector<double> queryTimesNaive;
        vector<double> queryTimesGTreeCSA;
        vector<double> queryTimesGTreeApproximation;

        vector<double> absolutDifferenceMinSum;
        vector<double> absolutDifferenceMinMax;

        vector<double> accuracyMinSum;
        vector<double> accuracyMinMax;

        while(successfulQueryCounter < numberOfSuccessfulQueries) {
            queryCounter++;

            MeetingPointQuery meetingPointQuery;
            if (loadOrStoreQueries && meetingPointQueries.size() > 0) {
                meetingPointQuery = meetingPointQueries[successfulQueryCounter];
            } else {
                meetingPointQuery = QueryGenerator::generateRandomMeetingPointQuery(numberOfSourceStops, numberOfDays);
            }
            
            NaiveQueryProcessor naiveQueryProcessor = NaiveQueryProcessor(meetingPointQuery);
            naiveQueryProcessor.processNaiveQuery();
            MeetingPointQueryResult meetingPointQueryResultNaive = naiveQueryProcessor.getMeetingPointQueryResult();

            GTreeQueryProcessor gTreeQueryProcessorCSA = GTreeQueryProcessor(meetingPointQuery, gTree);
            gTreeQueryProcessorCSA.processGTreeQuery(true);
            MeetingPointQueryResult meetingPointQueryResultGTreeCSA = gTreeQueryProcessorCSA.getMeetingPointQueryResult();

            GTreeQueryProcessor gTreeQueryProcessorApproximation = GTreeQueryProcessor(meetingPointQuery, gTree);
            gTreeQueryProcessorApproximation.processGTreeQuery(false);
            MeetingPointQueryResult meetingPointQueryResultGTreeApproximation = gTreeQueryProcessorApproximation.getMeetingPointQueryResult();

            bool naiveQuerySuccessful = meetingPointQueryResultNaive.meetingPointMinSum != "" && meetingPointQueryResultNaive.meetingPointMinMax != "";
            bool gTreeCSAQuerySuccessful = meetingPointQueryResultGTreeCSA.meetingPointMinSum != "" && meetingPointQueryResultGTreeCSA.meetingPointMinMax != "";
            bool gTreeApproximationQuerySuccessful = meetingPointQueryResultGTreeApproximation.meetingPointMinSum != "" && meetingPointQueryResultGTreeApproximation.meetingPointMinMax != "";

            if (naiveQuerySuccessful) {
                successfulQueriesNaive++;
            }

            if (gTreeCSAQuerySuccessful) {
                successfulQueriesGTreeCSA++;
            }

            if (gTreeApproximationQuerySuccessful) {
                successfulQueriesGTreeApproximation++;
            }

            if (!naiveQuerySuccessful || !gTreeCSAQuerySuccessful || !gTreeApproximationQuerySuccessful) {
                continue;
            }

            successfulQueryCounter++;

            queryTimesNaive.push_back((double) meetingPointQueryResultNaive.queryTime);
            queryTimesGTreeCSA.push_back((double) meetingPointQueryResultGTreeCSA.queryTime);
            queryTimesGTreeApproximation.push_back((double) meetingPointQueryResultGTreeApproximation.queryTime);

            int differenceMinSum = meetingPointQueryResultGTreeApproximation.minSumDurationInSeconds - meetingPointQueryResultNaive.minSumDurationInSeconds;
            int differenceMinMax = meetingPointQueryResultGTreeApproximation.minMaxDurationInSeconds - meetingPointQueryResultNaive.minMaxDurationInSeconds;

            absolutDifferenceMinSum.push_back((double) differenceMinSum / 60);
            absolutDifferenceMinMax.push_back((double) differenceMinMax / 60);

            double relativeDifferenceMinSum = (double) differenceMinSum / meetingPointQueryResultGTreeApproximation.minSumDurationInSeconds;
            double relativeDifferenceMinMax = (double) differenceMinMax / meetingPointQueryResultGTreeApproximation.minMaxDurationInSeconds;

            double accuracyMinSumVal = 1 - relativeDifferenceMinSum;
            double accuracyMinMaxVal = 1 - relativeDifferenceMinMax;

            accuracyMinSum.push_back(accuracyMinSumVal);
            accuracyMinMax.push_back(accuracyMinMaxVal);

            // Store the query information in a csv file
            string sourceStopNames = "";
            for (int j = 0; j < meetingPointQuery.sourceStopIds.size()-1; j++) {
                sourceStopNames += Importer::getStopName(meetingPointQuery.sourceStopIds[j]) + "-";
            }
            sourceStopNames += Importer::getStopName(meetingPointQuery.sourceStopIds[meetingPointQuery.sourceStopIds.size()-1]);

            queriesFile << sourceStopNames << "," << meetingPointQuery.sourceTime << "," << meetingPointQuery.weekday << "," << meetingPointQueryResultNaive.queryTime << "," << meetingPointQueryResultGTreeCSA.queryTime << "," << meetingPointQueryResultGTreeApproximation.queryTime << "," << differenceMinSum << "," << differenceMinMax << "," << accuracyMinSumVal << "," << accuracyMinMaxVal << "\n";

            if (loadOrStoreQueries && meetingPointQueries.size() == 0) {
                for (int i = 0; i < meetingPointQuery.sourceStopIds.size(); i++) {
                    queriesInfoFile << meetingPointQuery.sourceStopIds[i] << ",";
                }
                queriesInfoFile << meetingPointQuery.sourceTime << "," << meetingPointQuery.weekday << "," << meetingPointQuery.numberOfDays << "\n";
            }
            // Print progress every 10% of the queries
            if (successfulQueryCounter % (numberOfSuccessfulQueries / 5) == 0) {
                cout << "Progress: " << successfulQueryCounter << " / " << numberOfSuccessfulQueries << endl;
            }
        }

        double rateOfSuccessfulQueriesNaive = (double) successfulQueriesNaive / queryCounter;
        double rateOfSuccessfulCSAQueriesGTree = (double) successfulQueriesGTreeCSA / queryCounter;
        double rateOfSuccessfulApproximationQueriesGTree = (double) successfulQueriesGTreeApproximation / queryCounter;
        double rateOfSuccessfulQueries = (double) successfulQueryCounter / queryCounter;

        resultsFile << numberOfSourceStops << "," << Calculator::getAverage(queryTimesNaive) << "," << Calculator::getAverage(queryTimesGTreeCSA) << "," << Calculator::getAverage(queryTimesGTreeApproximation) << "," << Calculator::getMedian(queryTimesNaive) << "," << Calculator::getMedian(queryTimesGTreeCSA) << "," << Calculator::getMedian(queryTimesGTreeApproximation) << "," << Calculator::getMaximum(queryTimesNaive) << "," << Calculator::getMaximum(queryTimesGTreeCSA) << "," << Calculator::getMaximum(queryTimesGTreeApproximation) << "," << Calculator::getMinimum(queryTimesNaive) << "," << Calculator::getMinimum(queryTimesGTreeCSA) << "," << Calculator::getMinimum(queryTimesGTreeApproximation) << "," << Calculator::getAverage(absolutDifferenceMinSum) << "," << Calculator::getAverage(absolutDifferenceMinMax) << "," << Calculator::getMedian(absolutDifferenceMinSum) << "," << Calculator::getMedian(absolutDifferenceMinMax) << "," << Calculator::getMaximum(absolutDifferenceMinSum) << "," << Calculator::getMaximum(absolutDifferenceMinMax) << "," << Calculator::getMinimum(absolutDifferenceMinSum) << "," << Calculator::getMinimum(absolutDifferenceMinMax) << "," << Calculator::getAverage(accuracyMinSum) << "," << Calculator::getAverage(accuracyMinMax) << "," << Calculator::getMedian(accuracyMinSum) << "," << Calculator::getMedian(accuracyMinMax) << "," << Calculator::getMaximum(accuracyMinSum) << "," << Calculator::getMaximum(accuracyMinMax) << "," << Calculator::getMinimum(accuracyMinSum) << "," << Calculator::getMinimum(accuracyMinMax) << "\n";

        if (printResults) {
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
            cout << "Average absolut difference min sum: " << Calculator::getAverage(absolutDifferenceMinSum) << " minutes" << endl;
            cout << "Average absolut difference min max: " << Calculator::getAverage(absolutDifferenceMinMax) << " minutes" << endl;
            cout << "Median absolut difference min sum: " << Calculator::getMedian(absolutDifferenceMinSum) << " minutes" << endl;
            cout << "Median absolut difference min max: " << Calculator::getMedian(absolutDifferenceMinMax) << " minutes" << endl;
            cout << "Maximum absolut difference min sum: " << Calculator::getMaximum(absolutDifferenceMinSum) << " minutes" << endl;
            cout << "Maximum absolut difference min max: " << Calculator::getMaximum(absolutDifferenceMinMax) << " minutes" << endl;
            cout << "Minimum absolut difference min sum: " << Calculator::getMinimum(absolutDifferenceMinSum) << " minutes" << endl;
            cout << "Minimum absolut difference min max: " << Calculator::getMinimum(absolutDifferenceMinMax) << " minutes" << endl;

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
        queriesFile.close();
        queriesInfoFile.close();
    }
    resultsFile.close();
}

void AlgorithmComparer::compareAlgorithms(GTree* gTree, MeetingPointQuery meetingPointQuery){
    NaiveQueryProcessor naiveQueryProcessor = NaiveQueryProcessor(meetingPointQuery);
    naiveQueryProcessor.processNaiveQuery();
    MeetingPointQueryResult meetingPointQueryResultNaive = naiveQueryProcessor.getMeetingPointQueryResult();

    GTreeQueryProcessor gTreeQueryProcessorCSA = GTreeQueryProcessor(meetingPointQuery, gTree);
    gTreeQueryProcessorCSA.processGTreeQuery(true);
    MeetingPointQueryResult meetingPointQueryResultGTreeCSA = gTreeQueryProcessorCSA.getMeetingPointQueryResult();

    GTreeQueryProcessor gTreeQueryProcessorApproximation = GTreeQueryProcessor(meetingPointQuery, gTree);
    gTreeQueryProcessorApproximation.processGTreeQuery(false);
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