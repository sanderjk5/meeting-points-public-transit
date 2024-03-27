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
void NaiveAlgorithmTester::testNaiveAlgorithmRandom(int numberOfSuccessfulQueries, int numberOfSources, bool printOnlySuccessful) {
    int successfulQueryCounter = 0;
    for (int i = 0; i < numberOfSuccessfulQueries; i++) {
        MeetingPointQuery meetingPointQuery = QueryGenerator::generateRandomMeetingPointQuery(numberOfSources);
        
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
void NaiveKeyStopAlgorithmTester::testNaiveKeyStopAlgorithmRandom(DataType dataType, int numberOfSuccessfulQueries, int numberOfSources, bool printOnlySuccessful) {
    int successfulQueryCounter = 0;

    vector<int> keyStops = NaiveKeyStopQueryProcessor::getKeyStops(dataType, numberOfSources);

    for (int i = 0; i < numberOfSuccessfulQueries; i++) {
        MeetingPointQuery meetingPointQuery = QueryGenerator::generateRandomMeetingPointQuery(numberOfSources);
        
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
void GTreeAlgorithmTester::testGTreeAlgorithmRandom(GTree* gTree, bool useCSA, int numberOfSuccessfulQueries, int numberOfSources, bool printOnlySuccessful) {
    int successfulQueryCounter = 0;
    for (int i = 0; i < numberOfSuccessfulQueries; i++) {
        MeetingPointQuery meetingPointQuery = QueryGenerator::generateRandomMeetingPointQuery(numberOfSources);
        
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
        if(useCSA) {
            MeetingPointQueryGTreeCSAInfo meetingPointQueryGTreeCSAInfo = gTreeQueryProcessor.getMeetingPointQueryGTreeCSAInfo();
            PrintHelper::printGTreeCSAInfo(meetingPointQueryGTreeCSAInfo);
        } else {
            PrintHelper::printGTreeApproxInfo(gTreeQueryProcessor);
        }
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
    if(useCSA) {
        MeetingPointQueryGTreeCSAInfo meetingPointQueryGTreeCSAInfo = gTreeQueryProcessor.getMeetingPointQueryGTreeCSAInfo();
        PrintHelper::printGTreeCSAInfo(meetingPointQueryGTreeCSAInfo);
    } else {
        PrintHelper::printGTreeApproxInfo(gTreeQueryProcessor);
    }

    bool querySuccessful = meetingPointQueryResult.meetingPointMinSum != "" && meetingPointQueryResult.meetingPointMinMax != "";

    if (querySuccessful && printJourneys) {
        vector<Journey> journeysMinSum = gTreeQueryProcessor.getJourneys(min_sum);
        vector<Journey> journeysMinMax = gTreeQueryProcessor.getJourneys(min_max);

        cout << "\nJourneys min sum: " << endl;
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
        MeetingPointQuery meetingPointQuery = QueryGenerator::generateRandomMeetingPointQuery(numberOfSourceStops);

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

void RaptorAlgorithmTester::testRaptorAlgorithmRandom(int numberOfSuccessfulQueries, int numberOfSources, bool printOnlySuccessful) {
    int successfulQueryCounter = 0;
    for (int i = 0; i < numberOfSuccessfulQueries; i++) {
        MeetingPointQuery meetingPointQuery = QueryGenerator::generateRandomMeetingPointQuery(numberOfSources);
        
        RaptorQueryProcessor raptorQueryProcessor = RaptorQueryProcessor(meetingPointQuery);
        raptorQueryProcessor.processRaptorQueryUntilFirstResult();
        MeetingPointQueryResult meetingPointQueryResult = raptorQueryProcessor.getMeetingPointQueryResult();

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

void RaptorAlgorithmTester::testRaptorAlgorithm(MeetingPointQuery meetingPointQuery) {
    PrintHelper::printMeetingPointQuery(meetingPointQuery);

    RaptorQueryProcessor raptorQueryProcessor = RaptorQueryProcessor(meetingPointQuery);
    raptorQueryProcessor.processRaptorQueryUntilFirstResult();
    MeetingPointQueryResult meetingPointQueryResult = raptorQueryProcessor.getMeetingPointQueryResult();
    
    PrintHelper::printMeetingPointQueryResult(meetingPointQueryResult);
}

void RaptorAlgorithmTester::compareRaptorAlgorithms(DataType dataType, int numberOfSuccessfulQueries, vector<int> numberOfSources, bool loadOrStoreQueries) {
    string dataTypeString = Importer::getDataTypeString(dataType);
    string folderPathResults = FOLDER_PREFIX + "tests/" + dataTypeString + "/results/";
    string folderPathQueries = FOLDER_PREFIX + "tests/" + dataTypeString + "/queries/";

    time_t now = time(0);
    tm *ltm = localtime(&now);
    string timestamp = to_string(1900 + ltm->tm_year) + "-" + to_string(1 + ltm->tm_mon) + "-" + to_string(ltm->tm_mday) + "-" + to_string(ltm->tm_hour) + "-" + to_string(ltm->tm_min) + "-" + to_string(ltm->tm_sec);
    
    string resultsFileName = folderPathResults + "raptor-results-" + timestamp + ".csv";    

    std::ofstream resultsFile (resultsFileName, std::ofstream::out);
    resultsFile << "numberOfSourceStops,avgMinSumDuration,avgMinMaxDuration,medianMinSumDuration,medianMinMaxDuration,maxMinSumDuration";
    resultsFile << ",maxMinMaxDuration,minMinSumDuration,minMinMaxDuration";

    resultsFile << ",avgQueryTimeRaptorFirst,avgQueryTimeRaptorNoImprovement,avgQueryTimeRaptorOptimalResult";
    resultsFile << ",medianQueryTimeRaptorFirst,medianQueryTimeRaptorNoImprovement,medianQueryTimeRaptorOptimalResult";
    resultsFile << ",maxQueryTimeRaptorFirst,maxQueryTimeRaptorNoImprovement,maxQueryTimeRaptorOptimalResult";
    resultsFile << ",minQueryTimeRaptorFirst,minQueryTimeRaptorNoImprovement,minQueryTimeRaptorOptimalResult";

    resultsFile << ",avgQueryTimeOneRound,avgQueryTimeTwoRounds,avgQueryTimeThreeRounds,avgQueryTimeFourRounds,avgQueryTimeFiveRounds,avgQueryTimeSixRounds";
    resultsFile << ",medianQueryTimeOneRound,medianQueryTimeTwoRounds,medianQueryTimeThreeRounds,medianQueryTimeFourRounds,medianQueryTimeFiveRounds,medianQueryTimeSixRounds";
    resultsFile << ",maxQueryTimeOneRound,maxQueryTimeTwoRounds,maxQueryTimeThreeRounds,maxQueryTimeFourRounds,maxQueryTimeFiveRounds,maxQueryTimeSixRounds";
    resultsFile << ",minQueryTimeOneRound,minQueryTimeTwoRounds,minQueryTimeThreeRounds,minQueryTimeFourRounds,minQueryTimeFiveRounds,minQueryTimeSixRounds";

    resultsFile << ",avgRoundsToFirstResult,avgRoundsToNoImprovement,avgRoundsToOptimalResult,avgTotalRoundsOptimalResult";
    resultsFile << ",medianRoundsToFirstResult,medianRoundsToNoImprovement,medianRoundsToOptimalResult,medianTotalRoundsOptimalResult";
    resultsFile << ",maxRoundsToFirstResult,maxRoundsToNoImprovement,maxRoundsToOptimalResult,maxTotalRoundsOptimalResult";
    resultsFile << ",minRoundsToFirstResult,minRoundsToNoImprovement,minRoundsToOptimalResult,minTotalRoundsOptimalResult";

    resultsFile << ",fractionOfOptimalResultsMinSumRaptorFirst,fractionOfOptimalResultsMinMaxRaptorFirst";
    resultsFile << ",fractionOfLessThan10PercentRelDiffMinSumRaptorFirst,fractionOfLessThan10PercentRelDiffMinMaxRaptorFirst";

    resultsFile << ",fractionOfOptimalResultsMinSumRaptorNoImprovement,fractionOfOptimalResultsMinMaxRaptorNoImprovement";
    resultsFile << ",fractionOfLessThan10PercentRelDiffMinSumRaptorNoImprovement,fractionOfLessThan10PercentRelDiffMinMaxRaptorNoImprovement";

    resultsFile << ",fractionOfOptimalResultsMinSumRaptorOneRound,fractionOfOptimalResultsMinMaxRaptorOneRound";
    resultsFile << ",fractionOfLessThan10PercentRelDiffMinSumRaptorOneRound,fractionOfLessThan10PercentRelDiffMinMaxRaptorOneRound";
    resultsFile << ",fractionOfNoResultRaptorOneRound";
    resultsFile << ",fractionOfOptimalResultsMinSumRaptorTwoRounds,fractionOfOptimalResultsMinMaxRaptorTwoRounds";
    resultsFile << ",fractionOfLessThan10PercentRelDiffMinSumRaptorTwoRounds,fractionOfLessThan10PercentRelDiffMinMaxRaptorTwoRounds";
    resultsFile << ",fractionOfNoResultRaptorTwoRounds";
    resultsFile << ",fractionOfOptimalResultsMinSumRaptorThreeRounds,fractionOfOptimalResultsMinMaxRaptorThreeRounds";
    resultsFile << ",fractionOfLessThan10PercentRelDiffMinSumRaptorThreeRounds,fractionOfLessThan10PercentRelDiffMinMaxRaptorThreeRounds";
    resultsFile << ",fractionOfNoResultRaptorThreeRounds";
    resultsFile << ",fractionOfOptimalResultsMinSumRaptorFourRounds,fractionOfOptimalResultsMinMaxRaptorFourRounds";
    resultsFile << ",fractionOfLessThan10PercentRelDiffMinSumRaptorFourRounds,fractionOfLessThan10PercentRelDiffMinMaxRaptorFourRounds";
    resultsFile << ",fractionOfNoResultRaptorFourRounds";
    resultsFile << ",fractionOfOptimalResultsMinSumRaptorFiveRounds,fractionOfOptimalResultsMinMaxRaptorFiveRounds";
    resultsFile << ",fractionOfLessThan10PercentRelDiffMinSumRaptorFiveRounds,fractionOfLessThan10PercentRelDiffMinMaxRaptorFiveRounds";
    resultsFile << ",fractionOfNoResultRaptorFiveRounds";
    resultsFile << ",fractionOfOptimalResultsMinSumRaptorSixRounds,fractionOfOptimalResultsMinMaxRaptorSixRounds";
    resultsFile << ",fractionOfLessThan10PercentRelDiffMinSumRaptorSixRounds,fractionOfLessThan10PercentRelDiffMinMaxRaptorSixRounds";
    resultsFile << ",fractionOfNoResultRaptorSixRounds\n";

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

        int queryCounter = 0;
        int successfulQueryCounter = 0;

        vector<double> minSumDurations;
        vector<double> minMaxDurations;

        vector<double> queryTimesRaptorFirst;
        vector<double> queryTimesRaptorNoImprovement;
        vector<double> queryTimesRaptorOptimalResult;

        vector<vector<double>> queryTimesRaptorRounds = vector<vector<double>>(6, vector<double>());

        vector<double> roundsToFirstResult;
        vector<double> roundsToNoImprovement;
        vector<double> roundsToOptimalResult;
        vector<double> totalRoundsOptimalResult;

        vector<int> resultsCounterRaptorFirst = vector<int>(4, 0);
        vector<int> resultsCounterRaptorNoImprovement = vector<int>(4, 0);

        vector<vector<int>> resultCountersRaptorRounds = vector<vector<int>>(6, vector<int>(5, 0));

        while(successfulQueryCounter < numberOfSuccessfulQueries) {
            queryCounter++;

            MeetingPointQuery meetingPointQuery;
            if (loadOrStoreQueries && meetingPointQueries.size() > 0) {
                meetingPointQuery = meetingPointQueries[successfulQueryCounter];
            } else {
                meetingPointQuery = QueryGenerator::generateRandomMeetingPointQuery(numberOfSourceStops);
            }

            RaptorQueryProcessor raptorQueryProcessorOptimal = RaptorQueryProcessor(meetingPointQuery);
            raptorQueryProcessorOptimal.processRaptorQuery();
            MeetingPointQueryResult meetingPointQueryResultRaptorOptimal = raptorQueryProcessorOptimal.getMeetingPointQueryResult();

            if (meetingPointQueryResultRaptorOptimal.meetingPointMinSum == "" || meetingPointQueryResultRaptorOptimal.meetingPointMinMax == "") {
                continue;
            }

            successfulQueryCounter++;

            // NaiveQueryProcessor naiveQueryProcessor = NaiveQueryProcessor(meetingPointQuery);
            // naiveQueryProcessor.processNaiveQuery();
            // MeetingPointQueryResult meetingPointQueryResultNaive = naiveQueryProcessor.getMeetingPointQueryResult();

            // cout << meetingPointQueryResultRaptorOptimal.minSumDurationInSeconds << " " << meetingPointQueryResultNaive.minSumDurationInSeconds << endl;
            // cout << meetingPointQueryResultRaptorOptimal.minMaxDurationInSeconds << " " << meetingPointQueryResultNaive.minMaxDurationInSeconds << endl;

            RaptorQueryProcessor raptorQueryProcessorFirst = RaptorQueryProcessor(meetingPointQuery);
            raptorQueryProcessorFirst.processRaptorQueryUntilFirstResult();
            MeetingPointQueryResult meetingPointQueryResultRaptorFirst = raptorQueryProcessorFirst.getMeetingPointQueryResult();

            RaptorQueryProcessor raptorQueryProcessorNoImprovement = RaptorQueryProcessor(meetingPointQuery);
            raptorQueryProcessorNoImprovement.processRaptorQueryUntilResultDoesntImprove(both);
            MeetingPointQueryResult meetingPointQueryResultRaptorNoImprovement = raptorQueryProcessorNoImprovement.getMeetingPointQueryResult();

            auto queryDurationRounds = 0;
            RaptorQueryProcessor raptorQueryProcessorRounds = RaptorQueryProcessor(meetingPointQuery);
            raptorQueryProcessorRounds.initializeRaptors();
            vector<MeetingPointQueryResult> meetingPointQueryResultsRounds;
            for (int i = 0; i < 6; i++) {
                raptorQueryProcessorRounds.processRaptorRound();
                MeetingPointQueryResult meetingPointQueryResultRound = raptorQueryProcessorRounds.getMeetingPointQueryResult();
                queryDurationRounds += raptorQueryProcessorRounds.durationOfLastRound;
                meetingPointQueryResultsRounds.push_back(meetingPointQueryResultRound);
                queryTimesRaptorRounds[i].push_back((double) queryDurationRounds);
            }

            minSumDurations.push_back((double) meetingPointQueryResultRaptorOptimal.minSumDurationInSeconds);
            minMaxDurations.push_back((double) meetingPointQueryResultRaptorOptimal.minMaxDurationInSeconds);

            queryTimesRaptorFirst.push_back((double) meetingPointQueryResultRaptorFirst.queryTime);
            queryTimesRaptorNoImprovement.push_back((double) meetingPointQueryResultRaptorNoImprovement.queryTime);
            queryTimesRaptorOptimalResult.push_back((double) meetingPointQueryResultRaptorOptimal.queryTime);

            roundsToFirstResult.push_back((double) meetingPointQueryResultRaptorFirst.maxTransfersMinSum + 1);
            roundsToNoImprovement.push_back((double) meetingPointQueryResultRaptorNoImprovement.maxTransfersMinSum + 1);
            roundsToNoImprovement.push_back((double) meetingPointQueryResultRaptorNoImprovement.maxTransfersMinMax + 1);
            roundsToOptimalResult.push_back((double) meetingPointQueryResultRaptorOptimal.maxTransfersMinSum + 1);
            roundsToOptimalResult.push_back((double) meetingPointQueryResultRaptorOptimal.maxTransfersMinMax + 1);
            totalRoundsOptimalResult.push_back((double) raptorQueryProcessorOptimal.transfers + 1);

            int differenceMinSumRaptorFirst = meetingPointQueryResultRaptorFirst.minSumDurationInSeconds - meetingPointQueryResultRaptorOptimal.minSumDurationInSeconds;
            int differenceMinMaxRaptorFirst = meetingPointQueryResultRaptorFirst.minMaxDurationInSeconds - meetingPointQueryResultRaptorOptimal.minMaxDurationInSeconds;

            double relativeDifferenceMinSumRaptorFirst = (double) differenceMinSumRaptorFirst / meetingPointQueryResultRaptorOptimal.minSumDurationInSeconds;
            double relativeDifferenceMinMaxRaptorFirst = (double) differenceMinMaxRaptorFirst / meetingPointQueryResultRaptorOptimal.minMaxDurationInSeconds;

            if (differenceMinSumRaptorFirst == 0) {
                resultsCounterRaptorFirst[0]++;
            } 

            if (differenceMinMaxRaptorFirst == 0) {
                resultsCounterRaptorFirst[1]++;
            } 
            
            if (relativeDifferenceMinSumRaptorFirst < 0.1) {
                resultsCounterRaptorFirst[2]++;
            }
            
            if (relativeDifferenceMinMaxRaptorFirst < 0.1) {
                resultsCounterRaptorFirst[3]++;
            }

            int differenceMinSumRaptorNoImprovement = meetingPointQueryResultRaptorNoImprovement.minSumDurationInSeconds - meetingPointQueryResultRaptorOptimal.minSumDurationInSeconds;
            int differenceMinMaxRaptorNoImprovement = meetingPointQueryResultRaptorNoImprovement.minMaxDurationInSeconds - meetingPointQueryResultRaptorOptimal.minMaxDurationInSeconds;

            double relativeDifferenceMinSumRaptorNoImprovement = (double) differenceMinSumRaptorNoImprovement / meetingPointQueryResultRaptorOptimal.minSumDurationInSeconds;
            double relativeDifferenceMinMaxRaptorNoImprovement = (double) differenceMinMaxRaptorNoImprovement / meetingPointQueryResultRaptorOptimal.minMaxDurationInSeconds;

            if (differenceMinSumRaptorNoImprovement == 0) {
                resultsCounterRaptorNoImprovement[0]++;
            } 

            if (differenceMinMaxRaptorNoImprovement == 0) {
                resultsCounterRaptorNoImprovement[1]++;
            } 
            
            if (relativeDifferenceMinSumRaptorNoImprovement < 0.1) {
                resultsCounterRaptorNoImprovement[2]++;
            }  
            
            if (relativeDifferenceMinMaxRaptorNoImprovement < 0.1) {
                resultsCounterRaptorNoImprovement[3]++;
            }

            for (int i = 0; i < 6; i++) {
                bool foundResult = meetingPointQueryResultsRounds[i].meetingPointMinSum != "" && meetingPointQueryResultsRounds[i].meetingPointMinMax != "";

                if (!foundResult) {
                    resultCountersRaptorRounds[i][4]++;
                    continue;
                }

                int differenceMinSumRaptorRound = meetingPointQueryResultsRounds[i].minSumDurationInSeconds - meetingPointQueryResultRaptorOptimal.minSumDurationInSeconds;
                int differenceMinMaxRaptorRound = meetingPointQueryResultsRounds[i].minMaxDurationInSeconds - meetingPointQueryResultRaptorOptimal.minMaxDurationInSeconds;

                double relativeDifferenceMinSumRaptorRound = (double) differenceMinSumRaptorRound / meetingPointQueryResultRaptorOptimal.minSumDurationInSeconds;
                double relativeDifferenceMinMaxRaptorRound = (double) differenceMinMaxRaptorRound / meetingPointQueryResultRaptorOptimal.minMaxDurationInSeconds;

                if (differenceMinSumRaptorRound == 0) {
                    resultCountersRaptorRounds[i][0]++;
                } 

                if (differenceMinMaxRaptorRound == 0) {
                    resultCountersRaptorRounds[i][1]++;
                } 
                
                if (relativeDifferenceMinSumRaptorRound < 0.1) {
                    resultCountersRaptorRounds[i][2]++;
                }
  
                if (relativeDifferenceMinMaxRaptorRound < 0.1) {
                    resultCountersRaptorRounds[i][3]++;
                }
            }

            string sourceStopNames = "";
            for (int j = 0; j < meetingPointQuery.sourceStopIds.size()-1; j++) {
                sourceStopNames += Importer::getStopName(meetingPointQuery.sourceStopIds[j]) + "-";
            }
            sourceStopNames += Importer::getStopName(meetingPointQuery.sourceStopIds[meetingPointQuery.sourceStopIds.size()-1]);

            if (loadOrStoreQueries && meetingPointQueries.size() == 0) {
                for (int i = 0; i < meetingPointQuery.sourceStopIds.size(); i++) {
                    queriesInfoFile << meetingPointQuery.sourceStopIds[i] << ",";
                }
                queriesInfoFile << meetingPointQuery.sourceTime << "," << meetingPointQuery.weekday << "\n";
            }

            // Print progress every 20% of the queries
            if (successfulQueryCounter % (numberOfSuccessfulQueries / 5) == 0) {
                cout << "Progress: " << successfulQueryCounter << " / " << numberOfSuccessfulQueries << endl;
            }
        }

        if (loadOrStoreQueries && meetingPointQueries.size() == 0) {
            queriesInfoFile.close();
        }

        double avgMinSumDuration = Calculator::getAverage(minSumDurations) / 60;
        double avgMinMaxDuration = Calculator::getAverage(minMaxDurations) / 60;
        double medianMinSumDuration = Calculator::getMedian(minSumDurations) / 60;
        double medianMinMaxDuration = Calculator::getMedian(minMaxDurations) / 60;
        double maxMinSumDuration = Calculator::getMaximum(minSumDurations) / 60;
        double maxMinMaxDuration = Calculator::getMaximum(minMaxDurations) / 60;
        double minMinSumDuration = Calculator::getMinimum(minSumDurations) / 60;
        double minMinMaxDuration = Calculator::getMinimum(minMaxDurations) / 60;

        double avgQueryTimeRaptorFirst = Calculator::getAverage(queryTimesRaptorFirst);
        double avgQueryTimeRaptorNoImprovement = Calculator::getAverage(queryTimesRaptorNoImprovement);
        double avgQueryTimeRaptorOptimalResult = Calculator::getAverage(queryTimesRaptorOptimalResult);
        double medianQueryTimeRaptorFirst = Calculator::getMedian(queryTimesRaptorFirst);
        double medianQueryTimeRaptorNoImprovement = Calculator::getMedian(queryTimesRaptorNoImprovement);
        double medianQueryTimeRaptorOptimalResult = Calculator::getMedian(queryTimesRaptorOptimalResult);
        double maxQueryTimeRaptorFirst = Calculator::getMaximum(queryTimesRaptorFirst);
        double maxQueryTimeRaptorNoImprovement = Calculator::getMaximum(queryTimesRaptorNoImprovement);
        double maxQueryTimeRaptorOptimalResult = Calculator::getMaximum(queryTimesRaptorOptimalResult);
        double minQueryTimeRaptorFirst = Calculator::getMinimum(queryTimesRaptorFirst);
        double minQueryTimeRaptorNoImprovement = Calculator::getMinimum(queryTimesRaptorNoImprovement);
        double minQueryTimeRaptorOptimalResult = Calculator::getMinimum(queryTimesRaptorOptimalResult);

        vector<double> avgQueryTimesRaptorRounds;
        vector<double> medianQueryTimesRaptorRounds;
        vector<double> maxQueryTimesRaptorRounds;
        vector<double> minQueryTimesRaptorRounds;

        for (int i = 0; i < 6; i++) {
            avgQueryTimesRaptorRounds.push_back(Calculator::getAverage(queryTimesRaptorRounds[i]));
            medianQueryTimesRaptorRounds.push_back(Calculator::getMedian(queryTimesRaptorRounds[i]));
            maxQueryTimesRaptorRounds.push_back(Calculator::getMaximum(queryTimesRaptorRounds[i]));
            minQueryTimesRaptorRounds.push_back(Calculator::getMinimum(queryTimesRaptorRounds[i]));
        }

        double avgRoundsToFirstResult = Calculator::getAverage(roundsToFirstResult);
        double avgRoundsToNoImprovement = Calculator::getAverage(roundsToNoImprovement);
        double avgRoundsToOptimalResult = Calculator::getAverage(roundsToOptimalResult);
        double avgTotalRoundsOptimalResult = Calculator::getAverage(totalRoundsOptimalResult);
        double medianRoundsToFirstResult = Calculator::getMedian(roundsToFirstResult);
        double medianRoundsToNoImprovement = Calculator::getMedian(roundsToNoImprovement);
        double medianRoundsToOptimalResult = Calculator::getMedian(roundsToOptimalResult);
        double medianTotalRoundsOptimalResult = Calculator::getMedian(totalRoundsOptimalResult);
        double maxRoundsToFirstResult = Calculator::getMaximum(roundsToFirstResult);
        double maxRoundsToNoImprovement = Calculator::getMaximum(roundsToNoImprovement);
        double maxRoundsToOptimalResult = Calculator::getMaximum(roundsToOptimalResult);
        double maxTotalRoundsOptimalResult = Calculator::getMaximum(totalRoundsOptimalResult);
        double minRoundsToFirstResult = Calculator::getMinimum(roundsToFirstResult);
        double minRoundsToNoImprovement = Calculator::getMinimum(roundsToNoImprovement);
        double minRoundsToOptimalResult = Calculator::getMinimum(roundsToOptimalResult);
        double minTotalRoundsOptimalResult = Calculator::getMinimum(totalRoundsOptimalResult);

        vector<double> resultFractionsRaptorFirst;
        for (int i = 0; i < 4; i++) {
            resultFractionsRaptorFirst.push_back((double) resultsCounterRaptorFirst[i] / numberOfSuccessfulQueries);
        }

        vector<double> resultFractionsRaptorNoImprovement;
        for (int i = 0; i < 4; i++) {
            resultFractionsRaptorNoImprovement.push_back((double) resultsCounterRaptorNoImprovement[i] / numberOfSuccessfulQueries);
        }

        vector<vector<double>> resultFractionsRaptorRounds;
        for (int i = 0; i < 6; i++) {
            vector<double> resultFractionsRaptorRound;
            for (int j = 0; j < 5; j++) {
                resultFractionsRaptorRound.push_back((double) resultCountersRaptorRounds[i][j] / numberOfSuccessfulQueries);
            }
            resultFractionsRaptorRounds.push_back(resultFractionsRaptorRound);
        }

        resultsFile << numberOfSourceStops << "," << avgMinSumDuration << "," << avgMinMaxDuration << "," << medianMinSumDuration << "," << medianMinMaxDuration << "," << maxMinSumDuration;
        resultsFile << "," << maxMinMaxDuration << "," << minMinSumDuration << "," << minMinMaxDuration;
        resultsFile << "," << avgQueryTimeRaptorFirst << "," << avgQueryTimeRaptorNoImprovement << "," << avgQueryTimeRaptorOptimalResult;
        resultsFile << "," << medianQueryTimeRaptorFirst << "," << medianQueryTimeRaptorNoImprovement << "," << medianQueryTimeRaptorOptimalResult;
        resultsFile << "," << maxQueryTimeRaptorFirst << "," << maxQueryTimeRaptorNoImprovement << "," << maxQueryTimeRaptorOptimalResult;
        resultsFile << "," << minQueryTimeRaptorFirst << "," << minQueryTimeRaptorNoImprovement << "," << minQueryTimeRaptorOptimalResult;
        resultsFile << "," << avgQueryTimesRaptorRounds[0] << "," << avgQueryTimesRaptorRounds[1] << "," << avgQueryTimesRaptorRounds[2] << "," << avgQueryTimesRaptorRounds[3] << "," << avgQueryTimesRaptorRounds[4] << "," << avgQueryTimesRaptorRounds[5];
        resultsFile << "," << medianQueryTimesRaptorRounds[0] << "," << medianQueryTimesRaptorRounds[1] << "," << medianQueryTimesRaptorRounds[2] << "," << medianQueryTimesRaptorRounds[3] << "," << medianQueryTimesRaptorRounds[4] << "," << medianQueryTimesRaptorRounds[5];
        resultsFile << "," << maxQueryTimesRaptorRounds[0] << "," << maxQueryTimesRaptorRounds[1] << "," << maxQueryTimesRaptorRounds[2] << "," << maxQueryTimesRaptorRounds[3] << "," << maxQueryTimesRaptorRounds[4] << "," << maxQueryTimesRaptorRounds[5];
        resultsFile << "," << minQueryTimesRaptorRounds[0] << "," << minQueryTimesRaptorRounds[1] << "," << minQueryTimesRaptorRounds[2] << "," << minQueryTimesRaptorRounds[3] << "," << minQueryTimesRaptorRounds[4] << "," << minQueryTimesRaptorRounds[5];
        resultsFile << "," << avgRoundsToFirstResult << "," << avgRoundsToNoImprovement << "," << avgRoundsToOptimalResult << "," << avgTotalRoundsOptimalResult;
        resultsFile << "," << medianRoundsToFirstResult << "," << medianRoundsToNoImprovement << "," << medianRoundsToOptimalResult << "," << medianTotalRoundsOptimalResult;
        resultsFile << "," << maxRoundsToFirstResult << "," << maxRoundsToNoImprovement << "," << maxRoundsToOptimalResult << "," << maxTotalRoundsOptimalResult;
        resultsFile << "," << minRoundsToFirstResult << "," << minRoundsToNoImprovement << "," << minRoundsToOptimalResult << "," << minTotalRoundsOptimalResult;
        resultsFile << "," << resultFractionsRaptorFirst[0] << "," << resultFractionsRaptorFirst[1] << "," << resultFractionsRaptorFirst[2] << "," << resultFractionsRaptorFirst[3];
        resultsFile << "," << resultFractionsRaptorNoImprovement[0] << "," << resultFractionsRaptorNoImprovement[1] << "," << resultFractionsRaptorNoImprovement[2] << "," << resultFractionsRaptorNoImprovement[3];
        for (int i = 0; i < 6; i++) {
            resultsFile << "," << resultFractionsRaptorRounds[i][0] << "," << resultFractionsRaptorRounds[i][1] << "," << resultFractionsRaptorRounds[i][2] << "," << resultFractionsRaptorRounds[i][3] << "," << resultFractionsRaptorRounds[i][4];
        }
        resultsFile << "\n";

        cout << "Rate of successful queries: " << (double) successfulQueryCounter / numberOfSuccessfulQueries << endl;

        cout << "\nAverage min sum duration: " << avgMinSumDuration << endl;
        cout << "Average min max duration: " << avgMinMaxDuration << endl;
        cout << "Median min sum duration: " << medianMinSumDuration << endl;
        cout << "Median min max duration: " << medianMinMaxDuration << endl;
        cout << "Max min sum duration: " << maxMinSumDuration << endl;
        cout << "Max min max duration: " << maxMinMaxDuration << endl;
        cout << "Min min sum duration: " << minMinSumDuration << endl;
        cout << "Min min max duration: " << minMinMaxDuration << endl;

        cout << "\nAverage query time raptor first: " << avgQueryTimeRaptorFirst << endl;
        cout << "Average query time raptor no improvement: " << avgQueryTimeRaptorNoImprovement << endl;
        cout << "Average query time raptor optimal result: " << avgQueryTimeRaptorOptimalResult << endl;
        cout << "Median query time raptor first: " << medianQueryTimeRaptorFirst << endl;
        cout << "Median query time raptor no improvement: " << medianQueryTimeRaptorNoImprovement << endl;
        cout << "Median query time raptor optimal result: " << medianQueryTimeRaptorOptimalResult << endl;
        cout << "Max query time raptor first: " << maxQueryTimeRaptorFirst << endl;
        cout << "Max query time raptor no improvement: " << maxQueryTimeRaptorNoImprovement << endl;
        cout << "Max query time raptor optimal result: " << maxQueryTimeRaptorOptimalResult << endl;
        cout << "Min query time raptor first: " << minQueryTimeRaptorFirst << endl;
        cout << "Min query time raptor no improvement: " << minQueryTimeRaptorNoImprovement << endl;
        cout << "Min query time raptor optimal result: " << minQueryTimeRaptorOptimalResult << endl;

        cout << "\nAverage query time raptor one round: " << avgQueryTimesRaptorRounds[0] << endl;
        cout << "Average query time raptor two rounds: " << avgQueryTimesRaptorRounds[1] << endl;
        cout << "Average query time raptor three rounds: " << avgQueryTimesRaptorRounds[2] << endl;
        cout << "Average query time raptor four rounds: " << avgQueryTimesRaptorRounds[3] << endl;
        cout << "Average query time raptor five rounds: " << avgQueryTimesRaptorRounds[4] << endl;
        cout << "Average query time raptor six rounds: " << avgQueryTimesRaptorRounds[5] << endl;
        cout << "Median query time raptor one round: " << medianQueryTimesRaptorRounds[0] << endl;
        cout << "Median query time raptor two rounds: " << medianQueryTimesRaptorRounds[1] << endl;
        cout << "Median query time raptor three rounds: " << medianQueryTimesRaptorRounds[2] << endl;
        cout << "Median query time raptor four rounds: " << medianQueryTimesRaptorRounds[3] << endl;
        cout << "Median query time raptor five rounds: " << medianQueryTimesRaptorRounds[4] << endl;
        cout << "Median query time raptor six rounds: " << medianQueryTimesRaptorRounds[5] << endl;

        cout << "\nAverage rounds to first result: " << avgRoundsToFirstResult << endl;
        cout << "Average rounds to no improvement: " << avgRoundsToNoImprovement << endl;
        cout << "Average rounds to optimal result: " << avgRoundsToOptimalResult << endl;
        cout << "Average total rounds: " << avgTotalRoundsOptimalResult << endl;
        cout << "Median rounds to first result: " << medianRoundsToFirstResult << endl;
        cout << "Median rounds to no improvement: " << medianRoundsToNoImprovement << endl;
        cout << "Median rounds to optimal result: " << medianRoundsToOptimalResult << endl;
        cout << "Median total rounds: " << medianTotalRoundsOptimalResult << endl;
        cout << "Max rounds to first result: " << maxRoundsToFirstResult << endl;
        cout << "Max rounds to no improvement: " << maxRoundsToNoImprovement << endl;
        cout << "Max rounds to optimal result: " << maxRoundsToOptimalResult << endl;
        cout << "Max total rounds: " << maxTotalRoundsOptimalResult << endl;
        cout << "Min rounds to first result: " << minRoundsToFirstResult << endl;
        cout << "Min rounds to no improvement: " << minRoundsToNoImprovement << endl;
        cout << "Min rounds to optimal result: " << minRoundsToOptimalResult << endl;
        cout << "Min total rounds: " << minTotalRoundsOptimalResult << endl;

        cout << "\nFraction of optimal results min sum raptor first: " << resultFractionsRaptorFirst[0] << endl;
        cout << "Fraction of optimal results min max raptor first: " << resultFractionsRaptorFirst[1] << endl;
        cout << "Fraction of less than 10% relative difference min sum raptor first: " << resultFractionsRaptorFirst[2] << endl;
        cout << "Fraction of less than 10% relative difference min max raptor first: " << resultFractionsRaptorFirst[3] << endl;

        cout << "\nFraction of optimal results min sum raptor no improvement: " << resultFractionsRaptorNoImprovement[0] << endl;
        cout << "Fraction of optimal results min max raptor no improvement: " << resultFractionsRaptorNoImprovement[1] << endl;
        cout << "Fraction of less than 10% relative difference min sum raptor no improvement: " << resultFractionsRaptorNoImprovement[2] << endl;
        cout << "Fraction of less than 10% relative difference min max raptor no improvement: " << resultFractionsRaptorNoImprovement[3] << endl;

        cout << "\nFraction of optimal results min sum raptor one round: " << resultFractionsRaptorRounds[0][0] << endl;
        cout << "Fraction of optimal results min max raptor one round: " << resultFractionsRaptorRounds[0][1] << endl;
        cout << "Fraction of less than 10% relative difference min sum raptor one round: " << resultFractionsRaptorRounds[0][2] << endl;
        cout << "Fraction of less than 10% relative difference min max raptor one round: " << resultFractionsRaptorRounds[0][3] << endl;
        cout << "Fraction of no result raptor one round: " << resultFractionsRaptorRounds[0][4] << endl;

        cout << "\nFraction of optimal results min sum raptor two rounds: " << resultFractionsRaptorRounds[1][0] << endl;
        cout << "Fraction of optimal results min max raptor two rounds: " << resultFractionsRaptorRounds[1][1] << endl;
        cout << "Fraction of less than 10% relative difference min sum raptor two rounds: " << resultFractionsRaptorRounds[1][2] << endl;
        cout << "Fraction of less than 10% relative difference min max raptor two rounds: " << resultFractionsRaptorRounds[1][3] << endl;
        cout << "Fraction of no result raptor two rounds: " << resultFractionsRaptorRounds[1][4] << endl;

        cout << "\nFraction of optimal results min sum raptor three rounds: " << resultFractionsRaptorRounds[2][0] << endl;
        cout << "Fraction of optimal results min max raptor three rounds: " << resultFractionsRaptorRounds[2][1] << endl;
        cout << "Fraction of less than 10% relative difference min sum raptor three rounds: " << resultFractionsRaptorRounds[2][2] << endl;
        cout << "Fraction of less than 10% relative difference min max raptor three rounds: " << resultFractionsRaptorRounds[2][3] << endl;
        cout << "Fraction of no result raptor three rounds: " << resultFractionsRaptorRounds[2][4] << endl;

        cout << "\nFraction of optimal results min sum raptor four rounds: " << resultFractionsRaptorRounds[3][0] << endl;
        cout << "Fraction of optimal results min max raptor four rounds: " << resultFractionsRaptorRounds[3][1] << endl;
        cout << "Fraction of less than 10% relative difference min sum raptor four rounds: " << resultFractionsRaptorRounds[3][2] << endl;
        cout << "Fraction of less than 10% relative difference min max raptor four rounds: " << resultFractionsRaptorRounds[3][3] << endl;
        cout << "Fraction of no result raptor four rounds: " << resultFractionsRaptorRounds[3][4] << endl;

        cout << "\nFraction of optimal results min sum raptor five rounds: " << resultFractionsRaptorRounds[4][0] << endl;
        cout << "Fraction of optimal results min max raptor five rounds: " << resultFractionsRaptorRounds[4][1] << endl;
        cout << "Fraction of less than 10% relative difference min sum raptor five rounds: " << resultFractionsRaptorRounds[4][2] << endl;
        cout << "Fraction of less than 10% relative difference min max raptor five rounds: " << resultFractionsRaptorRounds[4][3] << endl;
        cout << "Fraction of no result raptor five rounds: " << resultFractionsRaptorRounds[4][4] << endl;

        cout << "\nFraction of optimal results min sum raptor six rounds: " << resultFractionsRaptorRounds[5][0] << endl;
        cout << "Fraction of optimal results min max raptor six rounds: " << resultFractionsRaptorRounds[5][1] << endl;
        cout << "Fraction of less than 10% relative difference min sum raptor six rounds: " << resultFractionsRaptorRounds[5][2] << endl;
        cout << "Fraction of less than 10% relative difference min max raptor six rounds: " << resultFractionsRaptorRounds[5][3] << endl;
        cout << "Fraction of no result raptor six rounds: " << resultFractionsRaptorRounds[5][4] << endl;

        cout << "\n\n";
    }  
}

/*
    Execute all algorithms for a given number of successful random queries and print the results. Compare the query times and the accuracies of the results.
    Store the queries and results in csv files.
*/
void AlgorithmComparer::compareAlgorithmsRandom(DataType dataType, GTree* gTree, int numberOfSuccessfulQueries, vector<int> numberOfSources, bool printResults, bool loadOrStoreQueries) {
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
    resultsFile << "numberOfSourceStops,avgQueryTimeNaive,avgQueryTimeNaiveKeyStop,avgQueryTimeGTreeCSA,avgQueryTimeGTreeApprox,avgQueryTimeRaptor";
    resultsFile << ",medianQueryTimeNaive,medianQueryTimeNaiveKeyStop,medianQueryTimeGTreeCSA,medianQueryTimeGTreeApprox,medianQueryTimeRaptor";
    resultsFile << ",maxQueryTimeNaive,maxQueryTimeNaiveKeyStop,maxQueryTimeGTreeCSA,maxQueryTimeGTreeApprox,maxQueryTimeRaptor";
    resultsFile << ",minQueryTimeNaive,minQueryTimeNaiveKeyStop,minQueryTimeGTreeCSA,minQueryTimeGTreeApprox,minQueryTimeRaptor";
    resultsFile << ",avgCSATargetStopFractionMinSum,avgCSATargetStopFractionMinMax,avgCSAVisitedConnectionsFraction";
    resultsFile << ",medianCSATargetStopFractionMinSum,medianCSATargetStopFractionMinMax,medianCSAVisitedConnectionsFraction";
    resultsFile << ",maxCSATargetStopFractionMinSum,maxCSATargetStopFractionMinMax,maxCSAVisitedConnectionsFraction";
    resultsFile << ",minCSATargetStopFractionMinSum,minCSATargetStopFractionMinMax,minCSAVisitedConnectionsFraction";
    resultsFile << ",avgVisitedNodesAvgFraction,medianVisitedNodesAvgFraction,maxVisitedNodesAvgFraction,minVisitedNodesAvgFraction";
    resultsFile << ",avgMaxTransfersNaive,avgMaxTransfersNaiveKeyStop,avgMaxTransfersGTreeCSA,avgMaxTransfersGTreeApprox,avgMaxTransfersRaptor";
    resultsFile << ",medianMaxTransfersNaive,medianMaxTransfersNaiveKeyStop,medianMaxTransfersGTreeCSA,medianMaxTransfersGTreeApprox,medianMaxTransfersRaptor";
    resultsFile << ",maxMaxTransfersNaive,maxMaxTransfersNaiveKeyStop,maxMaxTransfersGTreeCSA,maxMaxTransfersGTreeApprox,maxMaxTransfersRaptor";
    resultsFile << ",minMaxTransfersNaive,minMaxTransfersNaiveKeyStop,minMaxTransfersGTreeCSA,minMaxTransfersGTreeApprox,minMaxTransfersRaptor";
    resultsFile << ",avgAbsDiffMinSumGTree,avgAbsDiffMinMaxGTree,medianAbsDiffMinSumGTree,medianAbsDiffMinMaxGTree";
    resultsFile << ",maxAbsDiffMinSumGTree,maxAbsDiffMinMaxGTree,minAbsDiffMinSumGTree,minAbsDiffMinMaxGTree";
    resultsFile << ",avgAccMinSumGTree,avgAccMinMaxGTree,medianAccMinSumGTree,medianAccMinMaxGTree";
    resultsFile << ",maxAccMinSumGTree,maxAccMinMaxGTree,minAccMinSumGTree,minAccMinMaxGTree";
    resultsFile << ",avgAbsDiffMinSumKeyStop,avgAbsDiffMinMaxKeyStop,medianAbsDiffMinSumKeyStop,medianAbsDiffMinMaxKeyStop";
    resultsFile << ",maxAbsDiffMinSumKeyStop,maxAbsDiffMinMaxKeyStop,minAbsDiffMinSumKeyStop,minAbsDiffMinMaxKeyStop";
    resultsFile << ",avgAccMinSumKeyStop,avgAccMinMaxKeyStop,medianAccMinSumKeyStop,medianAccMinMaxKeyStop";
    resultsFile << ",maxAccMinSumKeyStop,maxAccMinMaxKeyStop,minAccMinSumKeyStop,minAccMinMaxKeyStop";
    resultsFile << ",avgAbsDiffMinSumRaptor,avgAbsDiffMinMaxRaptor,medianAbsDiffMinSumRaptor,medianAbsDiffMinMaxRaptor";
    resultsFile << ",maxAbsDiffMinSumRaptor,maxAbsDiffMinMaxRaptor,minAbsDiffMinSumRaptor,minAbsDiffMinMaxRaptor";
    resultsFile << ",avgAccMinSumRaptor,avgAccMinMaxRaptor,medianAccMinSumRaptor,medianAccMinMaxRaptor";
    resultsFile << ",maxAccMinSumRaptor,maxAccMinMaxRaptor,minAccMinSumRaptor,minAccMinMaxRaptor\n";
    
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
        queriesFile << "sourceStopIds,sourceTime,weekday";
        queriesFile << ",queryTimeNaive,queryTimeNaiveKeyStop,queryTimeGTreeCSA,queryTimeGTreeApprox,queryTimeRaptor";
        queriesFile << ",csaTargetStopFractionMinSum,csaTargetStopFractionMinMax,csaVisitedConnectionsFraction";
        queriesFile << ",visitedNodesAvgFraction";
        queriesFile << ",maxTransfersMinSumNaive,maxTransfersMinMaxNaive,maxTransfersMinSumNaiveKeyStop,maxTransfersMinMaxNaiveKeyStop";
        queriesFile << ",maxTransfersMinSumGTreeCSA,maxTransfersMinMaxGTreeCSA,maxTransfersMinSumGTreeApprox,maxTransfersMinMaxGTreeApprox,maxTransfersRaptor";
        queriesFile << ",absolutDifferenceMinSumGTree,absolutDifferenceMinMaxGTree,accuracyMinSumGTree,accuracyMinMaxGTree";
        queriesFile << ",absolutDifferenceMinSumKeyStop,absolutDifferenceMinMaxKeyStop,accuracyMinSumKeyStop,accuracyMinMaxKeyStop";
        queriesFile << ",absolutDifferenceMinSumRaptor,absolutDifferenceMinMaxRaptor,accuracyMinSumRaptor,accuracyMinMaxRaptor\n";

        vector<int> keyStops = NaiveKeyStopQueryProcessor::getKeyStops(dataType, numberOfSourceStops);

        int successfulQueriesNaive = 0;
        int successfulQueriesNaiveKeyStop = 0;
        int successfulQueriesGTreeCSA = 0;
        int successfulQueriesGTreeApproximation = 0;
        int successfulQueriesRaptor = 0;

        int queryCounter = 0;
        int successfulQueryCounter = 0;

        vector<double> queryTimesNaive;
        vector<double> queryTimesNaiveKeyStop;
        vector<double> queryTimesGTreeCSA;
        vector<double> queryTimesGTreeApproximation;
        vector<double> queryTimesRaptor;

        vector<double> csaTargetStopFractionMinSum;
        vector<double> csaTargetStopFractionMinMax;
        vector<double> csaVisitedConnectionsFraction;

        vector<double> visitedNodesAvgFraction;

        vector<double> maxTransfersNaive;
        vector<double> maxTransfersNaiveKeyStop;
        vector<double> maxTransfersGTreeCSA;
        vector<double> maxTransfersGTreeApproximation;
        vector<double> maxTransfersRaptor;

        vector<double> absolutDifferenceMinSumGTree;
        vector<double> absolutDifferenceMinMaxGTree;

        vector<double> accuracyMinSumGTree;
        vector<double> accuracyMinMaxGTree;

        vector<double> absolutDifferenceMinSumKeyStop;
        vector<double> absolutDifferenceMinMaxKeyStop;

        vector<double> accuracyMinSumKeyStop;
        vector<double> accuracyMinMaxKeyStop;

        vector<double> absolutDifferenceMinSumRaptor;
        vector<double> absolutDifferenceMinMaxRaptor;

        vector<double> accuracyMinSumRaptor;
        vector<double> accuracyMinMaxRaptor;

        while(successfulQueryCounter < numberOfSuccessfulQueries) {
            queryCounter++;

            MeetingPointQuery meetingPointQuery;
            if (loadOrStoreQueries && meetingPointQueries.size() > 0) {
                meetingPointQuery = meetingPointQueries[successfulQueryCounter];
            } else {
                meetingPointQuery = QueryGenerator::generateRandomMeetingPointQuery(numberOfSourceStops);
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
            MeetingPointQueryGTreeCSAInfo meetingPointQueryGTreeCSAInfo = gTreeQueryProcessorCSA.getMeetingPointQueryGTreeCSAInfo();

            GTreeQueryProcessor gTreeQueryProcessorApproximation = GTreeQueryProcessor(meetingPointQuery, gTree);
            gTreeQueryProcessorApproximation.processGTreeQuery(false);
            MeetingPointQueryResult meetingPointQueryResultGTreeApproximation = gTreeQueryProcessorApproximation.getMeetingPointQueryResult();

            RaptorQueryProcessor raptorQueryProcessor = RaptorQueryProcessor(meetingPointQuery);
            raptorQueryProcessor.processRaptorQueryUntilFirstResult();
            MeetingPointQueryResult meetingPointQueryResultRaptor = raptorQueryProcessor.getMeetingPointQueryResult();

            bool naiveQuerySuccessful = meetingPointQueryResultNaive.meetingPointMinSum != "" && meetingPointQueryResultNaive.meetingPointMinMax != "";
            bool naiveKeyStopQuerySuccessful = meetingPointQueryResultNaiveKeyStop.meetingPointMinSum != "" && meetingPointQueryResultNaiveKeyStop.meetingPointMinMax != "";
            bool gTreeCSAQuerySuccessful = meetingPointQueryResultGTreeCSA.meetingPointMinSum != "" && meetingPointQueryResultGTreeCSA.meetingPointMinMax != "";
            bool gTreeApproximationQuerySuccessful = meetingPointQueryResultGTreeApproximation.meetingPointMinSum != "" && meetingPointQueryResultGTreeApproximation.meetingPointMinMax != "";
            bool raptorQuerySuccessful = meetingPointQueryResultRaptor.meetingPointMinSum != "" && meetingPointQueryResultRaptor.meetingPointMinMax != "";

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

            if (raptorQuerySuccessful) {
                successfulQueriesRaptor++;
            }

            if (!naiveQuerySuccessful || !naiveKeyStopQuerySuccessful || !gTreeCSAQuerySuccessful || !gTreeApproximationQuerySuccessful || !raptorQuerySuccessful) {
                continue;
            }

            if (meetingPointQueryResultRaptor.minSumDurationInSeconds < meetingPointQueryResultNaive.minSumDurationInSeconds || meetingPointQueryResultRaptor.minMaxDurationInSeconds < meetingPointQueryResultNaive.minMaxDurationInSeconds) {
                successfulQueriesRaptor--;
                continue;
            }

            successfulQueryCounter++;

            queryTimesNaive.push_back((double) meetingPointQueryResultNaive.queryTime);
            queryTimesNaiveKeyStop.push_back((double) meetingPointQueryResultNaiveKeyStop.queryTime);
            queryTimesGTreeCSA.push_back((double) meetingPointQueryResultGTreeCSA.queryTime);
            queryTimesGTreeApproximation.push_back((double) meetingPointQueryResultGTreeApproximation.queryTime);
            queryTimesRaptor.push_back((double) meetingPointQueryResultRaptor.queryTime);

            csaTargetStopFractionMinSum.push_back(meetingPointQueryGTreeCSAInfo.csaTargetStopFractionMinSum);
            csaTargetStopFractionMinMax.push_back(meetingPointQueryGTreeCSAInfo.csaTargetStopFractionMinMax);
            csaVisitedConnectionsFraction.push_back(meetingPointQueryGTreeCSAInfo.csaVisitedConnectionsFraction);

            visitedNodesAvgFraction.push_back(gTreeQueryProcessorApproximation.visitedNodesAvgFraction);

            maxTransfersNaive.push_back(meetingPointQueryResultNaive.maxTransfersMinSum);
            maxTransfersNaive.push_back(meetingPointQueryResultNaive.maxTransfersMinMax);
            maxTransfersNaiveKeyStop.push_back(meetingPointQueryResultNaiveKeyStop.maxTransfersMinSum);
            maxTransfersNaiveKeyStop.push_back(meetingPointQueryResultNaiveKeyStop.maxTransfersMinMax);
            maxTransfersGTreeCSA.push_back(meetingPointQueryResultGTreeCSA.maxTransfersMinSum);
            maxTransfersGTreeCSA.push_back(meetingPointQueryResultGTreeCSA.maxTransfersMinMax);
            maxTransfersGTreeApproximation.push_back(meetingPointQueryResultGTreeApproximation.maxTransfersMinSum);
            maxTransfersGTreeApproximation.push_back(meetingPointQueryResultGTreeApproximation.maxTransfersMinMax);
            maxTransfersRaptor.push_back(meetingPointQueryResultRaptor.maxTransfersMinSum);
            maxTransfersRaptor.push_back(meetingPointQueryResultRaptor.maxTransfersMinMax);

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

            int differenceMinSumRaptor = meetingPointQueryResultRaptor.minSumDurationInSeconds - meetingPointQueryResultNaive.minSumDurationInSeconds;
            int differenceMinMaxRaptor = meetingPointQueryResultRaptor.minMaxDurationInSeconds - meetingPointQueryResultNaive.minMaxDurationInSeconds;

            absolutDifferenceMinSumRaptor.push_back((double) differenceMinSumRaptor / 60);
            absolutDifferenceMinMaxRaptor.push_back((double) differenceMinMaxRaptor / 60);

            double relativeDifferenceMinSumRaptor = (double) differenceMinSumRaptor / meetingPointQueryResultRaptor.minSumDurationInSeconds;
            double relativeDifferenceMinMaxRaptor = (double) differenceMinMaxRaptor / meetingPointQueryResultRaptor.minMaxDurationInSeconds;

            double accuracyMinSumValRaptor = 1 - relativeDifferenceMinSumRaptor;
            double accuracyMinMaxValRaptor = 1 - relativeDifferenceMinMaxRaptor;

            accuracyMinSumRaptor.push_back(accuracyMinSumValRaptor);
            accuracyMinMaxRaptor.push_back(accuracyMinMaxValRaptor);

            // Store the query information in a csv file
            string sourceStopNames = "";
            for (int j = 0; j < meetingPointQuery.sourceStopIds.size()-1; j++) {
                sourceStopNames += Importer::getStopName(meetingPointQuery.sourceStopIds[j]) + "-";
            }
            sourceStopNames += Importer::getStopName(meetingPointQuery.sourceStopIds[meetingPointQuery.sourceStopIds.size()-1]);

            queriesFile << sourceStopNames << "," << meetingPointQuery.sourceTime << "," << meetingPointQuery.weekday;
            queriesFile << "," << meetingPointQueryResultNaive.queryTime << "," << meetingPointQueryResultNaiveKeyStop.queryTime;
            queriesFile  << "," << meetingPointQueryResultGTreeCSA.queryTime << "," << meetingPointQueryResultGTreeApproximation.queryTime;
            queriesFile  << "," << meetingPointQueryResultRaptor.queryTime;
            queriesFile << "," << meetingPointQueryGTreeCSAInfo.csaTargetStopFractionMinSum << "," << meetingPointQueryGTreeCSAInfo.csaTargetStopFractionMinMax;
            queriesFile << "," << meetingPointQueryGTreeCSAInfo.csaVisitedConnectionsFraction;
            queriesFile << "," << gTreeQueryProcessorApproximation.visitedNodesAvgFraction;
            queriesFile << "," << meetingPointQueryResultNaive.maxTransfersMinSum << "," << meetingPointQueryResultNaive.maxTransfersMinMax;
            queriesFile << "," << meetingPointQueryResultNaiveKeyStop.maxTransfersMinSum << "," << meetingPointQueryResultNaiveKeyStop.maxTransfersMinMax;
            queriesFile << "," << meetingPointQueryResultGTreeCSA.maxTransfersMinSum << "," << meetingPointQueryResultGTreeCSA.maxTransfersMinMax;
            queriesFile << "," << meetingPointQueryResultGTreeApproximation.maxTransfersMinSum << "," << meetingPointQueryResultGTreeApproximation.maxTransfersMinMax;
            queriesFile << "," << meetingPointQueryResultRaptor.maxTransfersMinMax;
            queriesFile  << "," << differenceMinSumGTree << "," << differenceMinMaxGTree;
            queriesFile  << "," << accuracyMinSumValGTree << "," << accuracyMinMaxValGTree;
            queriesFile  << "," << differenceMinSumKeyStop << "," << differenceMinMaxKeyStop;
            queriesFile  << "," << accuracyMinSumValKeyStop << "," << accuracyMinMaxValKeyStop;
            queriesFile  << "," << differenceMinSumRaptor << "," << differenceMinMaxRaptor;
            queriesFile  << "," << accuracyMinSumValRaptor << "," << accuracyMinMaxValRaptor << "\n";

            if (loadOrStoreQueries && meetingPointQueries.size() == 0) {
                for (int i = 0; i < meetingPointQuery.sourceStopIds.size(); i++) {
                    queriesInfoFile << meetingPointQuery.sourceStopIds[i] << ",";
                }
                queriesInfoFile << meetingPointQuery.sourceTime << "," << meetingPointQuery.weekday << "\n";
            }
            // Print progress every 20% of the queries
            if (successfulQueryCounter % (numberOfSuccessfulQueries / 5) == 0) {
                cout << "Progress: " << successfulQueryCounter << " / " << numberOfSuccessfulQueries << endl;
            }
        }

        double rateOfSuccessfulQueriesNaive = (double) successfulQueriesNaive / queryCounter;
        double rateOfSuccessfulQueriesNaiveKeyStop = (double) successfulQueriesNaive / queryCounter;
        double rateOfSuccessfulCSAQueriesGTree = (double) successfulQueriesGTreeCSA / queryCounter;
        double rateOfSuccessfulApproximationQueriesGTree = (double) successfulQueriesGTreeApproximation / queryCounter;
        double rateOfSuccessfulQueriesRaptor = (double) successfulQueriesRaptor / queryCounter;
        double rateOfSuccessfulQueries = (double) successfulQueryCounter / queryCounter;

        resultsFile << numberOfSourceStops << "," << Calculator::getAverage(queryTimesNaive) << "," << Calculator::getAverage(queryTimesNaiveKeyStop) << "," << Calculator::getAverage(queryTimesGTreeCSA) << "," << Calculator::getAverage(queryTimesGTreeApproximation) << "," << Calculator::getAverage(queryTimesRaptor); 
        resultsFile << "," << Calculator::getMedian(queryTimesNaive) << ","  << Calculator::getMedian(queryTimesNaiveKeyStop) << "," << Calculator::getMedian(queryTimesGTreeCSA) << "," << Calculator::getMedian(queryTimesGTreeApproximation) << "," << Calculator::getMedian(queryTimesRaptor);
        resultsFile << "," << Calculator::getMaximum(queryTimesNaive) << "," << Calculator::getMaximum(queryTimesNaiveKeyStop) << "," << Calculator::getMaximum(queryTimesGTreeCSA) << "," << Calculator::getMaximum(queryTimesGTreeApproximation) << "," << Calculator::getMaximum(queryTimesRaptor);
        resultsFile << "," << Calculator::getMinimum(queryTimesNaive) << "," << Calculator::getMinimum(queryTimesNaiveKeyStop) << "," << Calculator::getMinimum(queryTimesGTreeCSA) << "," << Calculator::getMinimum(queryTimesGTreeApproximation) << "," << Calculator::getMinimum(queryTimesRaptor);
        resultsFile << "," << Calculator::getAverage(csaTargetStopFractionMinSum) << "," << Calculator::getAverage(csaTargetStopFractionMinMax) << "," << Calculator::getAverage(csaVisitedConnectionsFraction);
        resultsFile << "," << Calculator::getMedian(csaTargetStopFractionMinSum) << "," << Calculator::getMedian(csaTargetStopFractionMinMax) << "," << Calculator::getMedian(csaVisitedConnectionsFraction);
        resultsFile << "," << Calculator::getMaximum(csaTargetStopFractionMinSum) << "," << Calculator::getMaximum(csaTargetStopFractionMinMax) << "," << Calculator::getMaximum(csaVisitedConnectionsFraction);
        resultsFile << "," << Calculator::getMinimum(csaTargetStopFractionMinSum) << "," << Calculator::getMinimum(csaTargetStopFractionMinMax) << "," << Calculator::getMinimum(csaVisitedConnectionsFraction);
        resultsFile << "," << Calculator::getAverage(visitedNodesAvgFraction) << "," << Calculator::getMedian(visitedNodesAvgFraction) << "," << Calculator::getMaximum(visitedNodesAvgFraction) << "," << Calculator::getMinimum(visitedNodesAvgFraction);
        resultsFile << "," << Calculator::getAverage(maxTransfersNaive) << "," << Calculator::getAverage(maxTransfersNaiveKeyStop) << "," << Calculator::getAverage(maxTransfersGTreeCSA) << "," << Calculator::getAverage(maxTransfersGTreeApproximation) << "," << Calculator::getAverage(maxTransfersRaptor);
        resultsFile << "," << Calculator::getMedian(maxTransfersNaive) << "," << Calculator::getMedian(maxTransfersNaiveKeyStop) << "," << Calculator::getMedian(maxTransfersGTreeCSA) << "," << Calculator::getMedian(maxTransfersGTreeApproximation) << "," << Calculator::getMedian(maxTransfersRaptor);
        resultsFile << "," << Calculator::getMaximum(maxTransfersNaive) << "," << Calculator::getMaximum(maxTransfersNaiveKeyStop) << "," << Calculator::getMaximum(maxTransfersGTreeCSA) << "," << Calculator::getMaximum(maxTransfersGTreeApproximation) << "," << Calculator::getMaximum(maxTransfersRaptor);
        resultsFile << "," << Calculator::getMinimum(maxTransfersNaive) << "," << Calculator::getMinimum(maxTransfersNaiveKeyStop) << "," << Calculator::getMinimum(maxTransfersGTreeCSA) << "," << Calculator::getMinimum(maxTransfersGTreeApproximation) << "," << Calculator::getMinimum(maxTransfersRaptor);
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
        resultsFile << "," << Calculator::getAverage(absolutDifferenceMinSumRaptor) << "," << Calculator::getAverage(absolutDifferenceMinMaxRaptor);
        resultsFile << "," << Calculator::getMedian(absolutDifferenceMinSumRaptor) << "," << Calculator::getMedian(absolutDifferenceMinMaxRaptor);
        resultsFile << "," << Calculator::getMaximum(absolutDifferenceMinSumRaptor) << "," << Calculator::getMaximum(absolutDifferenceMinMaxRaptor);
        resultsFile << "," << Calculator::getMinimum(absolutDifferenceMinSumRaptor) << "," << Calculator::getMinimum(absolutDifferenceMinMaxRaptor);
        resultsFile << "," << Calculator::getAverage(accuracyMinSumRaptor) << "," << Calculator::getAverage(accuracyMinMaxRaptor);
        resultsFile << "," << Calculator::getMedian(accuracyMinSumRaptor) << "," << Calculator::getMedian(accuracyMinMaxRaptor);
        resultsFile << "," << Calculator::getMaximum(accuracyMinSumRaptor) << "," << Calculator::getMaximum(accuracyMinMaxRaptor);
        resultsFile << "," << Calculator::getMinimum(accuracyMinSumRaptor) << "," << Calculator::getMinimum(accuracyMinMaxRaptor);
        resultsFile << "\n";

        if (printResults) {
            cout << "\nCompare naive and g-tree algorithm: \n" << endl;
            cout << "Rate of successful queries naive: " << rateOfSuccessfulQueriesNaive << endl;
            cout << "Rate of successful queries naive (key stop): " << rateOfSuccessfulQueriesNaiveKeyStop << endl;
            cout << "Rate of successful queries gTree (csa): " << rateOfSuccessfulCSAQueriesGTree << endl;
            cout << "Rate of successful queries gTree (approximation): " << rateOfSuccessfulApproximationQueriesGTree << endl;
            cout << "Rate of successful queries raptor: " << rateOfSuccessfulQueriesRaptor << endl;
            cout << "Rate of successful queries for all of them: " << rateOfSuccessfulQueries << endl;

            cout << "\nQuery times:" << endl;
            cout << "Average query time naive: " << Calculator::getAverage(queryTimesNaive) << " milliseconds" << endl;
            cout << "Average query time naive (key stop): " << Calculator::getAverage(queryTimesNaiveKeyStop) << " milliseconds" << endl;
            cout << "Average query time gTree (csa): " << Calculator::getAverage(queryTimesGTreeCSA) << " milliseconds" << endl;
            cout << "Average query time gTree (approximation): " << Calculator::getAverage(queryTimesGTreeApproximation) << " milliseconds" << endl;
            cout << "Average query time raptor: " << Calculator::getAverage(queryTimesRaptor) << " milliseconds" << endl;
            cout << "Median query time naive: " << Calculator::getMedian(queryTimesNaive) << " milliseconds" << endl;
            cout << "Median query time naive (key stop): " << Calculator::getMedian(queryTimesNaiveKeyStop) << " milliseconds" << endl;
            cout << "Median query time gTree (csa): " << Calculator::getMedian(queryTimesGTreeCSA) << " milliseconds" << endl;
            cout << "Median query time gTree (approximation): " << Calculator::getMedian(queryTimesGTreeApproximation) << " milliseconds" << endl;
            cout << "Median query time raptor: " << Calculator::getMedian(queryTimesRaptor) << " milliseconds" << endl;
            cout << "Maximum query time naive: " << Calculator::getMaximum(queryTimesNaive) << " milliseconds" << endl;
            cout << "Maximum query time naive (key stop): " << Calculator::getMaximum(queryTimesNaiveKeyStop) << " milliseconds" << endl;
            cout << "Maximum query time gTree (csa): " << Calculator::getMaximum(queryTimesGTreeCSA) << " milliseconds" << endl;
            cout << "Maximum query time gTree (approximation): " << Calculator::getMaximum(queryTimesGTreeApproximation) << " milliseconds" << endl;
            cout << "Maximum query time raptor: " << Calculator::getMaximum(queryTimesRaptor) << " milliseconds" << endl;
            cout << "Minimum query time naive: " << Calculator::getMinimum(queryTimesNaive) << " milliseconds" << endl;
            cout << "Minimum query time naive (key stop): " << Calculator::getMinimum(queryTimesNaiveKeyStop) << " milliseconds" << endl;
            cout << "Minimum query time gTree (csa): " << Calculator::getMinimum(queryTimesGTreeCSA) << " milliseconds" << endl;
            cout << "Minimum query time gTree (approximation): " << Calculator::getMinimum(queryTimesGTreeApproximation) << " milliseconds" << endl;
            cout << "Minimum query time raptor: " << Calculator::getMinimum(queryTimesRaptor) << " milliseconds" << endl;

            cout << "\nCSA information:" << endl;
            cout << "Average target stop fraction min sum: " << Calculator::getAverage(csaTargetStopFractionMinSum) << endl;
            cout << "Average target stop fraction min max: " << Calculator::getAverage(csaTargetStopFractionMinMax) << endl;
            cout << "Average visited connections fraction: " << Calculator::getAverage(csaVisitedConnectionsFraction) << endl;
            cout << "Median target stop fraction min sum: " << Calculator::getMedian(csaTargetStopFractionMinSum) << endl;
            cout << "Median target stop fraction min max: " << Calculator::getMedian(csaTargetStopFractionMinMax) << endl;
            cout << "Median visited connections fraction: " << Calculator::getMedian(csaVisitedConnectionsFraction) << endl;
            cout << "Maximum target stop fraction min sum: " << Calculator::getMaximum(csaTargetStopFractionMinSum) << endl;
            cout << "Maximum target stop fraction min max: " << Calculator::getMaximum(csaTargetStopFractionMinMax) << endl;
            cout << "Maximum visited connections fraction: " << Calculator::getMaximum(csaVisitedConnectionsFraction) << endl;
            cout << "Minimum target stop fraction min sum: " << Calculator::getMinimum(csaTargetStopFractionMinSum) << endl;
            cout << "Minimum target stop fraction min max: " << Calculator::getMinimum(csaTargetStopFractionMinMax) << endl;
            cout << "Minimum visited connections fraction: " << Calculator::getMinimum(csaVisitedConnectionsFraction) << endl;

            cout << "\nGTree Approximation information:" << endl;
            cout << "Average target stop fraction: " << Calculator::getAverage(visitedNodesAvgFraction) << endl;
            cout << "Median target stop fraction: " << Calculator::getMedian(visitedNodesAvgFraction) << endl;
            cout << "Maximum target stop fraction: " << Calculator::getMaximum(visitedNodesAvgFraction) << endl;
            cout << "Minimum target stop fraction: " << Calculator::getMinimum(visitedNodesAvgFraction) << endl;

            cout << "\nMax transfers:" << endl;
            cout << "Average max transfers naive: " << Calculator::getAverage(maxTransfersNaive) << endl;
            cout << "Average max transfers naive (key stop): " << Calculator::getAverage(maxTransfersNaiveKeyStop) << endl;
            cout << "Average max transfers gTree (csa): " << Calculator::getAverage(maxTransfersGTreeCSA) << endl;
            cout << "Average max transfers gTree (approximation): " << Calculator::getAverage(maxTransfersGTreeApproximation) << endl;
            cout << "Average max transfers raptor: " << Calculator::getAverage(maxTransfersRaptor) << endl;
            cout << "Median max transfers naive: " << Calculator::getMedian(maxTransfersNaive) << endl;
            cout << "Median max transfers naive (key stop): " << Calculator::getMedian(maxTransfersNaiveKeyStop) << endl;
            cout << "Median max transfers gTree (csa): " << Calculator::getMedian(maxTransfersGTreeCSA) << endl;
            cout << "Median max transfers gTree (approximation): " << Calculator::getMedian(maxTransfersGTreeApproximation) << endl;
            cout << "Median max transfers raptor: " << Calculator::getMedian(maxTransfersRaptor) << endl;
            cout << "Maximum max transfers naive: " << Calculator::getMaximum(maxTransfersNaive) << endl;
            cout << "Maximum max transfers naive (key stop): " << Calculator::getMaximum(maxTransfersNaiveKeyStop) << endl;
            cout << "Maximum max transfers gTree (csa): " << Calculator::getMaximum(maxTransfersGTreeCSA) << endl;
            cout << "Maximum max transfers gTree (approximation): " << Calculator::getMaximum(maxTransfersGTreeApproximation) << endl;
            cout << "Maximum max transfers raptor: " << Calculator::getMaximum(maxTransfersRaptor) << endl;
            cout << "Minimum max transfers naive: " << Calculator::getMinimum(maxTransfersNaive) << endl;
            cout << "Minimum max transfers naive (key stop): " << Calculator::getMinimum(maxTransfersNaiveKeyStop) << endl;
            cout << "Minimum max transfers gTree (csa): " << Calculator::getMinimum(maxTransfersGTreeCSA) << endl;
            cout << "Minimum max transfers gTree (approximation): " << Calculator::getMinimum(maxTransfersGTreeApproximation) << endl;
            cout << "Minimum max transfers raptor: " << Calculator::getMinimum(maxTransfersRaptor) << endl;

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

            cout << "\nAbsolut result differences (raptor):" << endl;
            cout << "Average absolut difference min sum: " << Calculator::getAverage(absolutDifferenceMinSumRaptor) << " minutes" << endl;
            cout << "Average absolut difference min max: " << Calculator::getAverage(absolutDifferenceMinMaxRaptor) << " minutes" << endl;
            cout << "Median absolut difference min sum: " << Calculator::getMedian(absolutDifferenceMinSumRaptor) << " minutes" << endl;
            cout << "Median absolut difference min max: " << Calculator::getMedian(absolutDifferenceMinMaxRaptor) << " minutes" << endl;
            cout << "Maximum absolut difference min sum: " << Calculator::getMaximum(absolutDifferenceMinSumRaptor) << " minutes" << endl;
            cout << "Maximum absolut difference min max: " << Calculator::getMaximum(absolutDifferenceMinMaxRaptor) << " minutes" << endl;
            cout << "Minimum absolut difference min sum: " << Calculator::getMinimum(absolutDifferenceMinSumRaptor) << " minutes" << endl;
            cout << "Minimum absolut difference min max: " << Calculator::getMinimum(absolutDifferenceMinMaxRaptor) << " minutes" << endl;

            cout << "\nRelative result differences (raptor):" << endl;
            cout << "Average accuracy min sum: " << Calculator::getAverage(accuracyMinSumRaptor) << endl;
            cout << "Average accuracy min max: " << Calculator::getAverage(accuracyMinMaxRaptor) << endl;
            cout << "Median accuracy min sum: " << Calculator::getMedian(accuracyMinSumRaptor) << endl;
            cout << "Median accuracy min max: " << Calculator::getMedian(accuracyMinMaxRaptor) << endl;
            cout << "Maximum accuracy min sum: " << Calculator::getMaximum(accuracyMinSumRaptor) << endl;
            cout << "Maximum accuracy min max: " << Calculator::getMaximum(accuracyMinMaxRaptor) << endl;
            cout << "Minimum accuracy min sum: " << Calculator::getMinimum(accuracyMinSumRaptor) << endl;
            cout << "Minimum accuracy min max: " << Calculator::getMinimum(accuracyMinMaxRaptor) << endl;
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
    MeetingPointQueryGTreeCSAInfo meetingPointQueryGTreeCSAInfo = gTreeQueryProcessorCSA.getMeetingPointQueryGTreeCSAInfo();

    GTreeQueryProcessor gTreeQueryProcessorApproximation = GTreeQueryProcessor(meetingPointQuery, gTree);
    gTreeQueryProcessorApproximation.processGTreeQuery(false);
    MeetingPointQueryResult meetingPointQueryResultGTreeApproximation = gTreeQueryProcessorApproximation.getMeetingPointQueryResult();

    RaptorQueryProcessor raptorQueryProcessor = RaptorQueryProcessor(meetingPointQuery);
    raptorQueryProcessor.processRaptorQueryUntilFirstResult();
    MeetingPointQueryResult meetingPointQueryResultRaptor = raptorQueryProcessor.getMeetingPointQueryResult();

    PrintHelper::printMeetingPointQuery(meetingPointQuery);
    cout << "Naive: " << endl;
    PrintHelper::printMeetingPointQueryResult(meetingPointQueryResultNaive);

    if(executeKeyStopQuery) {
        cout << "Naive - Key Stop: " << endl;
        PrintHelper::printMeetingPointQueryResult(meetingPointQueryResultNaiveKeyStop);
    }
    
    cout << "GTree - CSA: " << endl;
    PrintHelper::printMeetingPointQueryResult(meetingPointQueryResultGTreeCSA);
    PrintHelper::printGTreeCSAInfo(meetingPointQueryGTreeCSAInfo);

    cout << "GTree - Approximation: " << endl;
    PrintHelper::printMeetingPointQueryResult(meetingPointQueryResultGTreeApproximation);
    PrintHelper::printGTreeApproxInfo(gTreeQueryProcessorApproximation);

    cout << "Raptor: " << endl;
    PrintHelper::printMeetingPointQueryResult(meetingPointQueryResultRaptor);

    bool naiveQuerySuccessful = meetingPointQueryResultNaive.meetingPointMinSum != "" && meetingPointQueryResultNaive.meetingPointMinMax != "";
    bool naiveKeyStopQuerySuccessful = true;
    if (executeKeyStopQuery) {
        naiveKeyStopQuerySuccessful = meetingPointQueryResultNaiveKeyStop.meetingPointMinSum != "" && meetingPointQueryResultNaiveKeyStop.meetingPointMinMax != "";
    }
    bool gTreeCSAQuerySuccessful = meetingPointQueryResultGTreeCSA.meetingPointMinSum != "" && meetingPointQueryResultGTreeCSA.meetingPointMinMax != "";
    bool gTreeApproximationQuerySuccessful = meetingPointQueryResultGTreeApproximation.meetingPointMinSum != "" && meetingPointQueryResultGTreeApproximation.meetingPointMinMax != "";
    bool raptorQuerySuccessful = meetingPointQueryResultRaptor.meetingPointMinSum != "" && meetingPointQueryResultRaptor.meetingPointMinMax != "";

    if(naiveQuerySuccessful && gTreeApproximationQuerySuccessful) {
        double absolutDifferenceMinSumGTree = (double) (meetingPointQueryResultGTreeApproximation.minSumDurationInSeconds - meetingPointQueryResultNaive.minSumDurationInSeconds);
        double absolutDifferenceMinMaxGTree = (double) (meetingPointQueryResultGTreeApproximation.minMaxDurationInSeconds - meetingPointQueryResultNaive.minMaxDurationInSeconds);

        double accuracyMinSumGTree = (double) 1 - (absolutDifferenceMinSumGTree / meetingPointQueryResultGTreeApproximation.minSumDurationInSeconds);
        double accuracyMinMaxGTree = (double) 1 - (absolutDifferenceMinMaxGTree / meetingPointQueryResultGTreeApproximation.minMaxDurationInSeconds);

        cout << "Result differences (GTree Approximation):" << endl;
        cout << "Absolut difference min sum: " << absolutDifferenceMinSumGTree / 60 << " minutes" << endl;
        cout << "Absolut difference min max: " << absolutDifferenceMinMaxGTree / 60 << " minutes" << endl;
        cout << "Accuracy min sum: " << accuracyMinSumGTree << endl;
        cout << "Accuracy min max: " << accuracyMinMaxGTree << endl;
    }

    if (naiveQuerySuccessful && naiveKeyStopQuerySuccessful && executeKeyStopQuery) {
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

    if (naiveQuerySuccessful && raptorQuerySuccessful) {
        double absolutDifferenceMinSumRaptor = (double) (meetingPointQueryResultRaptor.minSumDurationInSeconds - meetingPointQueryResultNaive.minSumDurationInSeconds);
        double absolutDifferenceMinMaxRaptor = (double) (meetingPointQueryResultRaptor.minMaxDurationInSeconds - meetingPointQueryResultNaive.minMaxDurationInSeconds);
        double accuracyMinSumRaptor = (double) 1 - (absolutDifferenceMinSumRaptor / meetingPointQueryResultRaptor.minSumDurationInSeconds);
        double accuracyMinMaxRaptor = (double) 1 - (absolutDifferenceMinMaxRaptor / meetingPointQueryResultRaptor.minMaxDurationInSeconds);

        cout << "\nResult differences (Raptor):" << endl;
        cout << "Absolut difference min sum: " << absolutDifferenceMinSumRaptor / 60 << " minutes" << endl;
        cout << "Absolut difference min max: " << absolutDifferenceMinMaxRaptor / 60 << " minutes" << endl;
        cout << "Accuracy min sum: " << accuracyMinSumRaptor << endl;
        cout << "Accuracy min max: " << accuracyMinMaxRaptor << endl;    
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
    cout << "Min sum - meeting point: " << meetingPointQueryResult.meetingPointMinSum << ", meeting time: " << meetingPointQueryResult.meetingTimeMinSum << ",  travel time sum: " << meetingPointQueryResult.minSumDuration << ", max transfers: " << meetingPointQueryResult.maxTransfersMinSum << endl;
    cout << "Min max - meeting point: " << meetingPointQueryResult.meetingPointMinMax << ", meeting time: " << meetingPointQueryResult.meetingTimeMinMax << ", travel time max: " << meetingPointQueryResult.minMaxDuration << ", max transfers: " << meetingPointQueryResult.maxTransfersMinMax << endl;
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

void PrintHelper::printGTreeCSAInfo(MeetingPointQueryGTreeCSAInfo meetingPointQueryGTreeCSAInfo) {
    cout << "GTree CSA Info: " << endl;
    cout << "Min Sum - fraction of target stops: " << meetingPointQueryGTreeCSAInfo.csaTargetStopFractionMinSum << endl;
    cout << "Min Max - fraction of target stops: " << meetingPointQueryGTreeCSAInfo.csaTargetStopFractionMinMax << endl;
    cout << "Fraction of visited connections: " << meetingPointQueryGTreeCSAInfo.csaVisitedConnectionsFraction << endl;
    cout << endl;
}

void PrintHelper::printGTreeApproxInfo(GTreeQueryProcessor gTreeQueryProcessor) {
    cout << "GTree Approximation Info: " << endl;
    cout << "Avg Fraction of visitedNodes: " << gTreeQueryProcessor.visitedNodesAvgFraction << endl;
    cout << endl;
}