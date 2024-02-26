#include "algorithm-tester.h"

#include "query-processor.h"
#include "journey.h"
#include <../data-handling/importer.h>
#include <../data-handling/converter.h>
#include <../data-structures/g-tree.h>
#include <../constants.h>
#include <algorithm>

#include <iostream>
#include <fstream>

using namespace std;

/*
    Execute the naive algorithm for a given number of successful random queries and print the results.
*/
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

/*
    Execute the naive algorithm for a given meeting point query and print the results.
*/
void NaiveAlgorithmTester::testNaiveAlgorithm(MeetingPointQuery meetingPointQuery, bool printJourneys) {
    PrintHelper::printMeetingPointQuery(meetingPointQuery);

    NaiveQueryProcessor naiveQueryProcessor = NaiveQueryProcessor(meetingPointQuery);
    naiveQueryProcessor.processNaiveQuery();
    MeetingPointQueryResult meetingPointQueryResult = naiveQueryProcessor.getMeetingPointQueryResult();
    
    PrintHelper::printMeetingPointQueryResult(meetingPointQueryResult);

    bool querySuccessful = meetingPointQueryResult.meetingPointMinSum != "" && meetingPointQueryResult.meetingPointMinMax != "";

    if (querySuccessful && printJourneys) {
        vector<Journey> journeysMinSum = naiveQueryProcessor.getJourneys(min_sum);
        vector<Journey> journeysMinMax = naiveQueryProcessor.getJourneys(min_max);

        cout << "Journeys min sum: " << endl;
        for (int i = 0; i < journeysMinSum.size(); i++) {
            PrintHelper::printJourney(journeysMinSum[i]);
        }
        cout << "\nJourneys min max: " << endl;
        for (int i = 0; i < journeysMinMax.size(); i++) {
            PrintHelper::printJourney(journeysMinMax[i]);
        }
    }
}

/*
    Execute the naive key stop algorithm for a given number of successful random queries and print the results.
*/
void NaiveKeyStopAlgorithmTester::testNaiveKeyStopAlgorithmRandom(DataType dataType, int numberOfSuccessfulQueries, int numberOfSources, int numberOfDays, bool printOnlySuccessful) {
    int successfulQueryCounter = 0;

    vector<int> keyStops = NaiveKeyStopQueryProcessor::getKeyStops(dataType, numberOfSources);

    for (int i = 0; i < numberOfSuccessfulQueries; i++) {
        MeetingPointQuery meetingPointQuery = QueryGenerator::generateRandomMeetingPointQuery(numberOfSources, numberOfDays);
        
        NaiveKeyStopQueryProcessor naiveKeyStopQueryProcessor = NaiveKeyStopQueryProcessor(meetingPointQuery);
        naiveKeyStopQueryProcessor.processNaiveKeyStopQuery(keyStops);
        MeetingPointQueryResult meetingPointQueryResult = naiveKeyStopQueryProcessor.getMeetingPointQueryResult();

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

/*
    Execute the naive key stop algorithm for a given meeting point query and print the results.
*/
void NaiveKeyStopAlgorithmTester::testNaiveKeyStopAlgorithm(DataType dataType, MeetingPointQuery meetingPointQuery, bool printJourneys) {
    PrintHelper::printMeetingPointQuery(meetingPointQuery);

    vector<int> keyStops = NaiveKeyStopQueryProcessor::getKeyStops(dataType, meetingPointQuery.sourceStopIds.size());

    NaiveKeyStopQueryProcessor naiveKeyStopQueryProcessor = NaiveKeyStopQueryProcessor(meetingPointQuery);
    naiveKeyStopQueryProcessor.processNaiveKeyStopQuery(keyStops);
    MeetingPointQueryResult meetingPointQueryResult = naiveKeyStopQueryProcessor.getMeetingPointQueryResult();
    
    PrintHelper::printMeetingPointQueryResult(meetingPointQueryResult);

    bool querySuccessful = meetingPointQueryResult.meetingPointMinSum != "" && meetingPointQueryResult.meetingPointMinMax != "";

    if (querySuccessful && printJourneys) {
        vector<Journey> journeysMinSum = naiveKeyStopQueryProcessor.getJourneys(min_sum);
        vector<Journey> journeysMinMax = naiveKeyStopQueryProcessor.getJourneys(min_max);

        cout << "Journeys min sum: " << endl;
        for (int i = 0; i < journeysMinSum.size(); i++) {
            PrintHelper::printJourney(journeysMinSum[i]);
        }
        cout << "\nJourneys min max: " << endl;
        for (int i = 0; i < journeysMinMax.size(); i++) {
            PrintHelper::printJourney(journeysMinMax[i]);
        }
    }
}

/*
    Execute the g-tree algorithm for a given number of successful random queries and print the results.
*/
void GTreeAlgorithmTester::testGTreeAlgorithmRandom(GTree* gTree, bool useCSA, int numberOfSuccessfulQueries, int numberOfSources, int numberOfDays, bool printOnlySuccessful) {
    int successfulQueryCounter = 0;
    for (int i = 0; i < numberOfSuccessfulQueries; i++) {
        MeetingPointQuery meetingPointQuery = QueryGenerator::generateRandomMeetingPointQuery(numberOfSources, numberOfDays);
        
        GTreeQueryProcessor gTreeQueryProcessor = GTreeQueryProcessor(meetingPointQuery, gTree);
        gTreeQueryProcessor.processGTreeQuery(useCSA);
        MeetingPointQueryResult meetingPointQueryResult = gTreeQueryProcessor.getMeetingPointQueryResult();

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

/*
    Execute the g-tree algorithm for a given meeting point query and print the results.
*/
void GTreeAlgorithmTester::testGTreeAlgorithm(GTree* gTree, MeetingPointQuery meetingPointQuery, bool useCSA, bool printJourneys) {
    PrintHelper::printMeetingPointQuery(meetingPointQuery);

    GTreeQueryProcessor gTreeQueryProcessor = GTreeQueryProcessor(meetingPointQuery, gTree);
    gTreeQueryProcessor.processGTreeQuery(useCSA);
    MeetingPointQueryResult meetingPointQueryResult = gTreeQueryProcessor.getMeetingPointQueryResult();
    
    PrintHelper::printMeetingPointQueryResult(meetingPointQueryResult);

    bool querySuccessful = meetingPointQueryResult.meetingPointMinSum != "" && meetingPointQueryResult.meetingPointMinMax != "";

    if (querySuccessful && printJourneys) {
        vector<Journey> journeysMinSum = gTreeQueryProcessor.getJourneys(min_sum);
        vector<Journey> journeysMinMax = gTreeQueryProcessor.getJourneys(min_max);

        cout << "Journeys min sum: " << endl;
        for (int i = 0; i < journeysMinSum.size(); i++) {
            PrintHelper::printJourney(journeysMinSum[i]);
        }
        cout << "\nJourneys min max: " << endl;
        for (int i = 0; i < journeysMinMax.size(); i++) {
            PrintHelper::printJourney(journeysMinMax[i]);
        }
    }
}

/*
    Execute the naive and g-tree approximation algorithm for a given number of successful random queries and return the average runtime and accuracy.
*/
AverageRunTimeAndAccuracy GTreeAlgorithmTester::getAverageRunTimeAndAccuracy(DataType dataType, GTree* gTree, int numberOfSourceStops, int numberOfSuccessfulQueries) {
    vector<double> queryTimesCSA;
    vector<double> queryTimesApproximation;
    vector<double> absolutDifferenceMinSum;
    vector<double> absolutDifferenceMinMax;
    vector<double> accuracyMinSum;
    vector<double> accuracyMinMax;

    int successfulQueryCounter = 0;

    while (successfulQueryCounter < numberOfSuccessfulQueries) {
        MeetingPointQuery meetingPointQuery = QueryGenerator::generateRandomMeetingPointQuery(numberOfSourceStops, NUMBER_OF_DAYS);

        GTreeQueryProcessor gTreeQueryProcessorCSA = GTreeQueryProcessor(meetingPointQuery, gTree);
        gTreeQueryProcessorCSA.processGTreeQuery(true);
        MeetingPointQueryResult meetingPointQueryResultGTreeCSA = gTreeQueryProcessorCSA.getMeetingPointQueryResult();
        
        GTreeQueryProcessor gTreeQueryProcessorApproximation = GTreeQueryProcessor(meetingPointQuery, gTree);
        gTreeQueryProcessorApproximation.processGTreeQuery();
        MeetingPointQueryResult meetingPointQueryResultApprox = gTreeQueryProcessorApproximation.getMeetingPointQueryResult();

        bool gTreeCSAQuerySuccessful = meetingPointQueryResultGTreeCSA.meetingPointMinSum != "" && meetingPointQueryResultGTreeCSA.meetingPointMinMax != "";
        bool gTreeApproximationQuerySuccessful = meetingPointQueryResultApprox.meetingPointMinSum != "" && meetingPointQueryResultApprox.meetingPointMinMax != "";

        if (!gTreeCSAQuerySuccessful || !gTreeApproximationQuerySuccessful) {
            continue;
        }

        successfulQueryCounter++;

        queryTimesCSA.push_back((double) meetingPointQueryResultGTreeCSA.queryTime);
        queryTimesApproximation.push_back((double) meetingPointQueryResultApprox.queryTime);

        int differenceMinSum = meetingPointQueryResultApprox.minSumDurationInSeconds - meetingPointQueryResultGTreeCSA.minSumDurationInSeconds;
        int differenceMinMax = meetingPointQueryResultApprox.minMaxDurationInSeconds - meetingPointQueryResultGTreeCSA.minMaxDurationInSeconds;

        double relativeDifferenceMinSum = (double) differenceMinSum / meetingPointQueryResultApprox.minSumDurationInSeconds;
        double relativeDifferenceMinMax = (double) differenceMinMax / meetingPointQueryResultApprox.minMaxDurationInSeconds;

        accuracyMinSum.push_back(1 - relativeDifferenceMinSum);
        accuracyMinMax.push_back(1 - relativeDifferenceMinMax);
    }

    AverageRunTimeAndAccuracy averageRunTimeAndAccuracy;
    averageRunTimeAndAccuracy.averageRunTimeGTreeCSA = Calculator::getAverage(queryTimesCSA);
    averageRunTimeAndAccuracy.averageRunTimeGTreeApproximation = Calculator::getAverage(queryTimesApproximation);
    averageRunTimeAndAccuracy.averageAccuracyMinSum = Calculator::getAverage(accuracyMinSum);
    averageRunTimeAndAccuracy.averageAccuracyMinMax = Calculator::getAverage(accuracyMinMax);

    return averageRunTimeAndAccuracy;
}

/*
    Execute all algorithms for a given number of successful random queries and print the results. Compare the query times and the accuracies of the results.
    Store the queries and results in csv files.
*/
void AlgorithmComparer::compareAlgorithmsRandom(DataType dataType, GTree* gTree, int numberOfSuccessfulQueries, vector<int> numberOfSources, int numberOfDays, bool printResults, bool loadOrStoreQueries) {
    string dataTypeString = Importer::getDataTypeString(dataType);
    string folderPathResults = FOLDER_PREFIX + "tests/" + dataTypeString + "/results/";
    string folderPathQueries = FOLDER_PREFIX + "tests/" + dataTypeString + "/queries/";

    // get the current timestamp
    time_t now = time(0);
    tm *ltm = localtime(&now);
    string timestamp = to_string(1900 + ltm->tm_year) + "-" + to_string(1 + ltm->tm_mon) + "-" + to_string(ltm->tm_mday) + "-" + to_string(ltm->tm_hour) + "-" + to_string(ltm->tm_min) + "-" + to_string(ltm->tm_sec);
    
    string resultsFileName = folderPathResults + "results-" + timestamp + ".csv";
    
    // Create a csv file to store the results
    std::ofstream resultsFile (resultsFileName, std::ofstream::out);
    resultsFile << "numberOfSourceStops,avgQueryTimeNaive,avgQueryTimeNaiveKeyStop,avgQueryTimeGTreeCSA,avgQueryTimeGTreeApprox,medianQueryTimeNaive,medianQueryTimeNaiveKeyStop,medianQueryTimeGTreeCSA,medianQueryTimeGTreeApprox,maxQueryTimeNaive,maxQueryTimeNaiveKeyStop,maxQueryTimeGTreeCSA,maxQueryTimeGTreeApprox,minQueryTimeNaive,minQueryTimeNaiveKeyStop,minQueryTimeGTreeCSA,minQueryTimeGTreeApprox,avgAbsDiffMinSumGTree,avgAbsDiffMinMaxGTree,medianAbsDiffMinSumGTree,medianAbsDiffMinMaxGTree,maxAbsDiffMinSumGTree,maxAbsDiffMinMaxGTree,minAbsDiffMinSumGTree,minAbsDiffMinMaxGTree,avgAccMinSumGTree,avgAccMinMaxGTree,medianAccMinSumGTree,medianAccMinMaxGTree,maxAccMinSumGTree,maxAccMinMaxGTree,minAccMinSumGTree,minAccMinMaxGTree,avgAbsDiffMinSumKeyStop,avgAbsDiffMinMaxKeyStop,medianAbsDiffMinSumKeyStop,medianAbsDiffMinMaxKeyStop,maxAbsDiffMinSumKeyStop,maxAbsDiffMinMaxKeyStop,minAbsDiffMinSumKeyStop,minAbsDiffMinMaxKeyStop,avgAccMinSumKeyStop,avgAccMinMaxKeyStop,medianAccMinSumKeyStop,medianAccMinMaxKeyStop,maxAccMinSumKeyStop,maxAccMinMaxKeyStop,minAccMinSumKeyStop,minAccMinMaxKeyStop\n";
    
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
        queriesFile << "sourceStopIds,sourceTime,weekday,queryTimeNaive,queryTimeNaiveKeyStop,queryTimeGTreeCSA,queryTimeGTreeApprox,absolutDifferenceMinSumGTree,absolutDifferenceMinMaxGTree,accuracyMinSumGTree,accuracyMinMaxGTree,absolutDifferenceMinSumKeyStop,absolutDifferenceMinMaxKeyStop,accuracyMinSumKeyStop,accuracyMinMaxKeyStop\n";

        vector<int> keyStops = NaiveKeyStopQueryProcessor::getKeyStops(dataType, numberOfSourceStops);

        int successfulQueriesNaive = 0;
        int successfulQueriesNaiveKeyStop = 0;
        int successfulQueriesGTreeCSA = 0;
        int successfulQueriesGTreeApproximation = 0;

        int queryCounter = 0;
        int successfulQueryCounter = 0;

        vector<double> queryTimesNaive;
        vector<double> queryTimesNaiveKeyStop;
        vector<double> queryTimesGTreeCSA;
        vector<double> queryTimesGTreeApproximation;

        vector<double> absolutDifferenceMinSumGTree;
        vector<double> absolutDifferenceMinMaxGTree;

        vector<double> accuracyMinSumGTree;
        vector<double> accuracyMinMaxGTree;

        vector<double> absolutDifferenceMinSumKeyStop;
        vector<double> absolutDifferenceMinMaxKeyStop;

        vector<double> accuracyMinSumKeyStop;
        vector<double> accuracyMinMaxKeyStop;

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

            NaiveKeyStopQueryProcessor naiveKeyStopQueryProcessor = NaiveKeyStopQueryProcessor(meetingPointQuery);
            naiveKeyStopQueryProcessor.processNaiveKeyStopQuery(keyStops);
            MeetingPointQueryResult meetingPointQueryResultNaiveKeyStop = naiveKeyStopQueryProcessor.getMeetingPointQueryResult();

            GTreeQueryProcessor gTreeQueryProcessorCSA = GTreeQueryProcessor(meetingPointQuery, gTree);
            gTreeQueryProcessorCSA.processGTreeQuery(true);
            MeetingPointQueryResult meetingPointQueryResultGTreeCSA = gTreeQueryProcessorCSA.getMeetingPointQueryResult();

            GTreeQueryProcessor gTreeQueryProcessorApproximation = GTreeQueryProcessor(meetingPointQuery, gTree);
            gTreeQueryProcessorApproximation.processGTreeQuery(false);
            MeetingPointQueryResult meetingPointQueryResultGTreeApproximation = gTreeQueryProcessorApproximation.getMeetingPointQueryResult();

            bool naiveQuerySuccessful = meetingPointQueryResultNaive.meetingPointMinSum != "" && meetingPointQueryResultNaive.meetingPointMinMax != "";
            bool naiveKeyStopQuerySuccessful = meetingPointQueryResultNaiveKeyStop.meetingPointMinSum != "" && meetingPointQueryResultNaiveKeyStop.meetingPointMinMax != "";
            bool gTreeCSAQuerySuccessful = meetingPointQueryResultGTreeCSA.meetingPointMinSum != "" && meetingPointQueryResultGTreeCSA.meetingPointMinMax != "";
            bool gTreeApproximationQuerySuccessful = meetingPointQueryResultGTreeApproximation.meetingPointMinSum != "" && meetingPointQueryResultGTreeApproximation.meetingPointMinMax != "";

            if (naiveQuerySuccessful) {
                successfulQueriesNaive++;
            }

            if (naiveKeyStopQuerySuccessful) {
                successfulQueriesNaiveKeyStop++;
            }

            if (gTreeCSAQuerySuccessful) {
                successfulQueriesGTreeCSA++;
            }

            if (gTreeApproximationQuerySuccessful) {
                successfulQueriesGTreeApproximation++;
            }

            if (!naiveQuerySuccessful || !naiveKeyStopQuerySuccessful || !gTreeCSAQuerySuccessful || !gTreeApproximationQuerySuccessful) {
                continue;
            }

            successfulQueryCounter++;

            queryTimesNaive.push_back((double) meetingPointQueryResultNaive.queryTime);
            queryTimesNaiveKeyStop.push_back((double) meetingPointQueryResultNaiveKeyStop.queryTime);
            queryTimesGTreeCSA.push_back((double) meetingPointQueryResultGTreeCSA.queryTime);
            queryTimesGTreeApproximation.push_back((double) meetingPointQueryResultGTreeApproximation.queryTime);

            int differenceMinSumGTree = meetingPointQueryResultGTreeApproximation.minSumDurationInSeconds - meetingPointQueryResultNaive.minSumDurationInSeconds;
            int differenceMinMaxGTree = meetingPointQueryResultGTreeApproximation.minMaxDurationInSeconds - meetingPointQueryResultNaive.minMaxDurationInSeconds;

            absolutDifferenceMinSumGTree.push_back((double) differenceMinSumGTree / 60);
            absolutDifferenceMinMaxGTree.push_back((double) differenceMinMaxGTree / 60);

            double relativeDifferenceMinSumGTree = (double) differenceMinSumGTree / meetingPointQueryResultGTreeApproximation.minSumDurationInSeconds;
            double relativeDifferenceMinMaxGTree = (double) differenceMinMaxGTree / meetingPointQueryResultGTreeApproximation.minMaxDurationInSeconds;

            double accuracyMinSumValGTree = 1 - relativeDifferenceMinSumGTree;
            double accuracyMinMaxValGTree = 1 - relativeDifferenceMinMaxGTree;

            accuracyMinSumGTree.push_back(accuracyMinSumValGTree);
            accuracyMinMaxGTree.push_back(accuracyMinMaxValGTree);

            int differenceMinSumKeyStop = meetingPointQueryResultNaiveKeyStop.minSumDurationInSeconds - meetingPointQueryResultNaive.minSumDurationInSeconds;
            int differenceMinMaxKeyStop = meetingPointQueryResultNaiveKeyStop.minMaxDurationInSeconds - meetingPointQueryResultNaive.minMaxDurationInSeconds;

            absolutDifferenceMinSumKeyStop.push_back((double) differenceMinSumKeyStop / 60);
            absolutDifferenceMinMaxKeyStop.push_back((double) differenceMinMaxKeyStop / 60);

            double relativeDifferenceMinSumKeyStop = (double) differenceMinSumKeyStop / meetingPointQueryResultNaiveKeyStop.minSumDurationInSeconds;
            double relativeDifferenceMinMaxKeyStop = (double) differenceMinMaxKeyStop / meetingPointQueryResultNaiveKeyStop.minMaxDurationInSeconds;

            double accuracyMinSumValKeyStop = 1 - relativeDifferenceMinSumKeyStop;
            double accuracyMinMaxValKeyStop = 1 - relativeDifferenceMinMaxKeyStop;

            accuracyMinSumKeyStop.push_back(accuracyMinSumValKeyStop);
            accuracyMinMaxKeyStop.push_back(accuracyMinMaxValKeyStop);

            // Store the query information in a csv file
            string sourceStopNames = "";
            for (int j = 0; j < meetingPointQuery.sourceStopIds.size()-1; j++) {
                sourceStopNames += Importer::getStopName(meetingPointQuery.sourceStopIds[j]) + "-";
            }
            sourceStopNames += Importer::getStopName(meetingPointQuery.sourceStopIds[meetingPointQuery.sourceStopIds.size()-1]);

            queriesFile << sourceStopNames << "," << meetingPointQuery.sourceTime << "," << meetingPointQuery.weekday << "," << meetingPointQueryResultNaive.queryTime << "," << meetingPointQueryResultNaiveKeyStop.queryTime << "," << meetingPointQueryResultGTreeCSA.queryTime << "," << meetingPointQueryResultGTreeApproximation.queryTime << "," << differenceMinSumGTree << "," << differenceMinMaxGTree << "," << accuracyMinSumValGTree << "," << accuracyMinMaxValGTree << "," << differenceMinSumKeyStop << "," << differenceMinMaxKeyStop << "," << accuracyMinSumValKeyStop << "," << accuracyMinMaxValKeyStop << "\n";

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
        double rateOfSuccessfulQueriesNaiveKeyStop = (double) successfulQueriesNaive / queryCounter;
        double rateOfSuccessfulCSAQueriesGTree = (double) successfulQueriesGTreeCSA / queryCounter;
        double rateOfSuccessfulApproximationQueriesGTree = (double) successfulQueriesGTreeApproximation / queryCounter;
        double rateOfSuccessfulQueries = (double) successfulQueryCounter / queryCounter;

        resultsFile << numberOfSourceStops << "," << Calculator::getAverage(queryTimesNaive) << "," << Calculator::getAverage(queryTimesNaiveKeyStop) << "," << Calculator::getAverage(queryTimesGTreeCSA) << "," << Calculator::getAverage(queryTimesGTreeApproximation); 
        resultsFile << "," << Calculator::getMedian(queryTimesNaive) << ","  << Calculator::getMedian(queryTimesNaiveKeyStop) << "," << Calculator::getMedian(queryTimesGTreeCSA) << "," << Calculator::getMedian(queryTimesGTreeApproximation);
        resultsFile << "," << Calculator::getMaximum(queryTimesNaive) << "," << Calculator::getMaximum(queryTimesNaiveKeyStop) << "," << Calculator::getMaximum(queryTimesGTreeCSA) << "," << Calculator::getMaximum(queryTimesGTreeApproximation);
        resultsFile << "," << Calculator::getMinimum(queryTimesNaive) << "," << Calculator::getMinimum(queryTimesNaiveKeyStop) << "," << Calculator::getMinimum(queryTimesGTreeCSA) << "," << Calculator::getMinimum(queryTimesGTreeApproximation);
        resultsFile << "," << Calculator::getAverage(absolutDifferenceMinSumGTree) << "," << Calculator::getAverage(absolutDifferenceMinMaxGTree);
        resultsFile << "," << Calculator::getMedian(absolutDifferenceMinSumGTree) << "," << Calculator::getMedian(absolutDifferenceMinMaxGTree);
        resultsFile << "," << Calculator::getMaximum(absolutDifferenceMinSumGTree) << "," << Calculator::getMaximum(absolutDifferenceMinMaxGTree);
        resultsFile << "," << Calculator::getMinimum(absolutDifferenceMinSumGTree) << "," << Calculator::getMinimum(absolutDifferenceMinMaxGTree);
        resultsFile << "," << Calculator::getAverage(accuracyMinSumGTree) << "," << Calculator::getAverage(accuracyMinMaxGTree);
        resultsFile << "," << Calculator::getMedian(accuracyMinSumGTree) << "," << Calculator::getMedian(accuracyMinMaxGTree);
        resultsFile << "," << Calculator::getMaximum(accuracyMinSumGTree) << "," << Calculator::getMaximum(accuracyMinMaxGTree);
        resultsFile << "," << Calculator::getMinimum(accuracyMinSumGTree) << "," << Calculator::getMinimum(accuracyMinMaxGTree);
        resultsFile << "," << Calculator::getAverage(absolutDifferenceMinSumKeyStop) << "," << Calculator::getAverage(absolutDifferenceMinMaxKeyStop);
        resultsFile << "," << Calculator::getMedian(absolutDifferenceMinSumKeyStop) << "," << Calculator::getMedian(absolutDifferenceMinMaxKeyStop);
        resultsFile << "," << Calculator::getMaximum(absolutDifferenceMinSumKeyStop) << "," << Calculator::getMaximum(absolutDifferenceMinMaxKeyStop);
        resultsFile << "," << Calculator::getMinimum(absolutDifferenceMinSumKeyStop) << "," << Calculator::getMinimum(absolutDifferenceMinMaxKeyStop);
        resultsFile << "," << Calculator::getAverage(accuracyMinSumKeyStop) << "," << Calculator::getAverage(accuracyMinMaxKeyStop);
        resultsFile << "," << Calculator::getMedian(accuracyMinSumKeyStop) << "," << Calculator::getMedian(accuracyMinMaxKeyStop);
        resultsFile << "," << Calculator::getMaximum(accuracyMinSumKeyStop) << "," << Calculator::getMaximum(accuracyMinMaxKeyStop);
        resultsFile << "," << Calculator::getMinimum(accuracyMinSumKeyStop) << "," << Calculator::getMinimum(accuracyMinMaxKeyStop);
        resultsFile << "\n";

        if (printResults) {
            cout << "\nCompare naive and g-tree algorithm: \n" << endl;
            cout << "Rate of successful queries naive: " << rateOfSuccessfulQueriesNaive << endl;
            cout << "Rate of successful queries naive (key stop): " << rateOfSuccessfulQueriesNaiveKeyStop << endl;
            cout << "Rate of successful queries gTree (csa): " << rateOfSuccessfulCSAQueriesGTree << endl;
            cout << "Rate of successful queries gTree (approximation): " << rateOfSuccessfulApproximationQueriesGTree << endl;
            cout << "Rate of successful queries for all of them: " << rateOfSuccessfulQueries << endl;

            cout << "\nQuery times:" << endl;
            cout << "Average query time naive: " << Calculator::getAverage(queryTimesNaive) << " milliseconds" << endl;
            cout << "Average query time naive (key stop): " << Calculator::getAverage(queryTimesNaiveKeyStop) << " milliseconds" << endl;
            cout << "Average query time gTree (csa): " << Calculator::getAverage(queryTimesGTreeCSA) << " milliseconds" << endl;
            cout << "Average query time gTree (approximation): " << Calculator::getAverage(queryTimesGTreeApproximation) << " milliseconds" << endl;
            cout << "Median query time naive: " << Calculator::getMedian(queryTimesNaive) << " milliseconds" << endl;
            cout << "Median query time naive (key stop): " << Calculator::getMedian(queryTimesNaiveKeyStop) << " milliseconds" << endl;
            cout << "Median query time gTree (csa): " << Calculator::getMedian(queryTimesGTreeCSA) << " milliseconds" << endl;
            cout << "Median query time gTree (approximation): " << Calculator::getMedian(queryTimesGTreeApproximation) << " milliseconds" << endl;
            cout << "Maximum query time naive: " << Calculator::getMaximum(queryTimesNaive) << " milliseconds" << endl;
            cout << "Maximum query time naive (key stop): " << Calculator::getMaximum(queryTimesNaiveKeyStop) << " milliseconds" << endl;
            cout << "Maximum query time gTree (csa): " << Calculator::getMaximum(queryTimesGTreeCSA) << " milliseconds" << endl;
            cout << "Maximum query time gTree (approximation): " << Calculator::getMaximum(queryTimesGTreeApproximation) << " milliseconds" << endl;
            cout << "Minimum query time naive: " << Calculator::getMinimum(queryTimesNaive) << " milliseconds" << endl;
            cout << "Minimum query time naive (key stop): " << Calculator::getMinimum(queryTimesNaiveKeyStop) << " milliseconds" << endl;
            cout << "Minimum query time gTree (csa): " << Calculator::getMinimum(queryTimesGTreeCSA) << " milliseconds" << endl;
            cout << "Minimum query time gTree (approximation): " << Calculator::getMinimum(queryTimesGTreeApproximation) << " milliseconds" << endl;

            cout << "\nAbsolut result differences (g tree approximation):" << endl;
            cout << "Average absolut difference min sum: " << Calculator::getAverage(absolutDifferenceMinSumGTree) << " minutes" << endl;
            cout << "Average absolut difference min max: " << Calculator::getAverage(absolutDifferenceMinMaxGTree) << " minutes" << endl;
            cout << "Median absolut difference min sum: " << Calculator::getMedian(absolutDifferenceMinSumGTree) << " minutes" << endl;
            cout << "Median absolut difference min max: " << Calculator::getMedian(absolutDifferenceMinMaxGTree) << " minutes" << endl;
            cout << "Maximum absolut difference min sum: " << Calculator::getMaximum(absolutDifferenceMinSumGTree) << " minutes" << endl;
            cout << "Maximum absolut difference min max: " << Calculator::getMaximum(absolutDifferenceMinMaxGTree) << " minutes" << endl;
            cout << "Minimum absolut difference min sum: " << Calculator::getMinimum(absolutDifferenceMinSumGTree) << " minutes" << endl;
            cout << "Minimum absolut difference min max: " << Calculator::getMinimum(absolutDifferenceMinMaxGTree) << " minutes" << endl;

            cout << "\nRelative result differences (g tree approximation):" << endl;
            cout << "Average accuracy min sum: " << Calculator::getAverage(accuracyMinSumGTree) << endl;
            cout << "Average accuracy min max: " << Calculator::getAverage(accuracyMinMaxGTree) << endl;
            cout << "Median accuracy min sum: " << Calculator::getMedian(accuracyMinSumGTree) << endl;
            cout << "Median accuracy min max: " << Calculator::getMedian(accuracyMinMaxGTree) << endl;
            cout << "Maximum accuracy min sum: " << Calculator::getMaximum(accuracyMinSumGTree) << endl;
            cout << "Maximum accuracy min max: " << Calculator::getMaximum(accuracyMinMaxGTree) << endl;
            cout << "Minimum accuracy min sum: " << Calculator::getMinimum(accuracyMinSumGTree) << endl;
            cout << "Minimum accuracy min max: " << Calculator::getMinimum(accuracyMinMaxGTree) << endl;

            cout << "\nAbsolut result differences (key stop approximation):" << endl;
            cout << "Average absolut difference min sum: " << Calculator::getAverage(absolutDifferenceMinSumKeyStop) << " minutes" << endl;
            cout << "Average absolut difference min max: " << Calculator::getAverage(absolutDifferenceMinMaxKeyStop) << " minutes" << endl;
            cout << "Median absolut difference min sum: " << Calculator::getMedian(absolutDifferenceMinSumKeyStop) << " minutes" << endl;
            cout << "Median absolut difference min max: " << Calculator::getMedian(absolutDifferenceMinMaxKeyStop) << " minutes" << endl;
            cout << "Maximum absolut difference min sum: " << Calculator::getMaximum(absolutDifferenceMinSumKeyStop) << " minutes" << endl;
            cout << "Maximum absolut difference min max: " << Calculator::getMaximum(absolutDifferenceMinMaxKeyStop) << " minutes" << endl;
            cout << "Minimum absolut difference min sum: " << Calculator::getMinimum(absolutDifferenceMinSumKeyStop) << " minutes" << endl;
            cout << "Minimum absolut difference min max: " << Calculator::getMinimum(absolutDifferenceMinMaxKeyStop) << " minutes" << endl;

            cout << "\nRelative result differences (key stop approximation):" << endl;
            cout << "Average accuracy min sum: " << Calculator::getAverage(accuracyMinSumKeyStop) << endl;
            cout << "Average accuracy min max: " << Calculator::getAverage(accuracyMinMaxKeyStop) << endl;
            cout << "Median accuracy min sum: " << Calculator::getMedian(accuracyMinSumKeyStop) << endl;
            cout << "Median accuracy min max: " << Calculator::getMedian(accuracyMinMaxKeyStop) << endl;
            cout << "Maximum accuracy min sum: " << Calculator::getMaximum(accuracyMinSumKeyStop) << endl;
            cout << "Maximum accuracy min max: " << Calculator::getMaximum(accuracyMinMaxKeyStop) << endl;
            cout << "Minimum accuracy min sum: " << Calculator::getMinimum(accuracyMinSumKeyStop) << endl;
            cout << "Minimum accuracy min max: " << Calculator::getMinimum(accuracyMinMaxKeyStop) << endl;
        }

        queriesFile.close();
        queriesInfoFile.close();
    }
    resultsFile.close();
}

/*
    Execute all algorithms for a given meeting point query and print the results. Compare the query times and the accuracies of the results.
*/
void AlgorithmComparer::compareAlgorithms(DataType dataType, GTree* gTree, MeetingPointQuery meetingPointQuery){
    NaiveQueryProcessor naiveQueryProcessor = NaiveQueryProcessor(meetingPointQuery);
    naiveQueryProcessor.processNaiveQuery();
    MeetingPointQueryResult meetingPointQueryResultNaive = naiveQueryProcessor.getMeetingPointQueryResult();

    vector<int> keyStops = NaiveKeyStopQueryProcessor::getKeyStops(dataType, meetingPointQuery.sourceStopIds.size());
    NaiveKeyStopQueryProcessor naiveKeyStopQueryProcessor = NaiveKeyStopQueryProcessor(meetingPointQuery);
    MeetingPointQueryResult meetingPointQueryResultNaiveKeyStop;
    bool executeKeyStopQuery = false;
    if (keyStops.size() > 0) {
        naiveKeyStopQueryProcessor.processNaiveKeyStopQuery(keyStops);
        meetingPointQueryResultNaiveKeyStop = naiveKeyStopQueryProcessor.getMeetingPointQueryResult();
        executeKeyStopQuery = true;
    }

    GTreeQueryProcessor gTreeQueryProcessorCSA = GTreeQueryProcessor(meetingPointQuery, gTree);
    gTreeQueryProcessorCSA.processGTreeQuery(true);
    MeetingPointQueryResult meetingPointQueryResultGTreeCSA = gTreeQueryProcessorCSA.getMeetingPointQueryResult();

    GTreeQueryProcessor gTreeQueryProcessorApproximation = GTreeQueryProcessor(meetingPointQuery, gTree);
    gTreeQueryProcessorApproximation.processGTreeQuery(false);
    MeetingPointQueryResult meetingPointQueryResultGTreeApproximation = gTreeQueryProcessorApproximation.getMeetingPointQueryResult();

    PrintHelper::printMeetingPointQuery(meetingPointQuery);
    cout << "Naive: " << endl;
    PrintHelper::printMeetingPointQueryResult(meetingPointQueryResultNaive);

    if(executeKeyStopQuery) {
        cout << "Naive - Key Stop: " << endl;
        PrintHelper::printMeetingPointQueryResult(meetingPointQueryResultNaiveKeyStop);
    }
    
    cout << "GTree - CSA: " << endl;
    PrintHelper::printMeetingPointQueryResult(meetingPointQueryResultGTreeCSA);

    cout << "GTree - Approximation: " << endl;
    PrintHelper::printMeetingPointQueryResult(meetingPointQueryResultGTreeApproximation);

    bool naiveQuerySuccessful = meetingPointQueryResultNaive.meetingPointMinSum != "" && meetingPointQueryResultNaive.meetingPointMinMax != "";
    bool naiveKeyStopQuerySuccessful = true;
    if (executeKeyStopQuery) {
        naiveKeyStopQuerySuccessful = meetingPointQueryResultNaiveKeyStop.meetingPointMinSum != "" && meetingPointQueryResultNaiveKeyStop.meetingPointMinMax != "";
    }
    bool gTreeCSAQuerySuccessful = meetingPointQueryResultGTreeCSA.meetingPointMinSum != "" && meetingPointQueryResultGTreeCSA.meetingPointMinMax != "";
    bool gTreeApproximationQuerySuccessful = meetingPointQueryResultGTreeApproximation.meetingPointMinSum != "" && meetingPointQueryResultGTreeApproximation.meetingPointMinMax != "";

    if(naiveQuerySuccessful && naiveKeyStopQuerySuccessful && gTreeApproximationQuerySuccessful && gTreeCSAQuerySuccessful) {
        double absolutDifferenceMinSumGTree = (double) (meetingPointQueryResultGTreeApproximation.minSumDurationInSeconds - meetingPointQueryResultNaive.minSumDurationInSeconds);
        double absolutDifferenceMinMaxGTree = (double) (meetingPointQueryResultGTreeApproximation.minMaxDurationInSeconds - meetingPointQueryResultNaive.minMaxDurationInSeconds);

        double accuracyMinSumGTree = (double) 1 - (absolutDifferenceMinSumGTree / meetingPointQueryResultGTreeApproximation.minSumDurationInSeconds);
        double accuracyMinMaxGTree = (double) 1 - (absolutDifferenceMinMaxGTree / meetingPointQueryResultGTreeApproximation.minMaxDurationInSeconds);

        cout << "Result differences (GTree Approximation):" << endl;
        cout << "Absolut difference min sum: " << absolutDifferenceMinSumGTree / 60 << " minutes" << endl;
        cout << "Absolut difference min max: " << absolutDifferenceMinMaxGTree / 60 << " minutes" << endl;
        cout << "Accuracy min sum: " << accuracyMinSumGTree << endl;
        cout << "Accuracy min max: " << accuracyMinMaxGTree << endl;

        if (executeKeyStopQuery) {
            double absolutDifferenceMinSumKeyStop = (double) (meetingPointQueryResultNaiveKeyStop.minSumDurationInSeconds - meetingPointQueryResultNaive.minSumDurationInSeconds);
            double absolutDifferenceMinMaxKeyStop = (double) (meetingPointQueryResultNaiveKeyStop.minMaxDurationInSeconds - meetingPointQueryResultNaive.minMaxDurationInSeconds);

            double accuracyMinSumKeyStop = (double) 1 - (absolutDifferenceMinSumKeyStop / meetingPointQueryResultNaiveKeyStop.minSumDurationInSeconds);
            double accuracyMinMaxKeyStop = (double) 1 - (absolutDifferenceMinMaxKeyStop / meetingPointQueryResultNaiveKeyStop.minMaxDurationInSeconds);

            cout << "\nResult differences (Key Stop Approximation):" << endl;
            cout << "Absolut difference min sum: " << absolutDifferenceMinSumKeyStop / 60 << " minutes" << endl;
            cout << "Absolut difference min max: " << absolutDifferenceMinMaxKeyStop / 60 << " minutes" << endl;
            cout << "Accuracy min sum: " << accuracyMinSumKeyStop << endl;
            cout << "Accuracy min max: " << accuracyMinMaxKeyStop << endl;
        }
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

/*
    Print the meeting point query.
*/
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

/*
    Print the meeting point query result.
*/
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

/*
    Print the journey.
*/
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