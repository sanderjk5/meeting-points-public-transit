#include "algorithm-tester-raptor.h"

#include "query-processor-raptor.h"
#include "query-processor.h"
#include "algorithm-tester.h"
#include "raptor.h"
#include "journey.h"
#include <../data-handling/importer.h>
#include <../data-handling/converter.h>
#include <../constants.h>

#include <vector>
#include <memory>
#include <algorithm>
#include <iostream>
#include <fstream>

using namespace std;

void RaptorEATAlgorithmTester::testRaptorAlgorithms(RaptorQuery raptorQuery) {
    unique_ptr<RaptorQueryEATProcessor> raptorQueryProcessor = unique_ptr<RaptorQueryEATProcessor>(new RaptorQueryEATProcessor(raptorQuery));
    raptorQueryProcessor->processRaptorQuery();
    RaptorQueryResult raptorQueryResult = raptorQueryProcessor->getRaptorQueryResult();

    unique_ptr<RaptorStarBoundQueryProcessor> raptorStarBoundQueryProcessor = unique_ptr<RaptorStarBoundQueryProcessor>(new RaptorStarBoundQueryProcessor(raptorQuery));
    raptorStarBoundQueryProcessor->processRaptorStarBoundQuery();
    RaptorQueryResult raptorStarBoundQueryResult = raptorStarBoundQueryProcessor->getRaptorStarQueryResult();

    unique_ptr<RaptorStarPQQueryProcessor> raptorStarPQQueryProcessor = unique_ptr<RaptorStarPQQueryProcessor>(new RaptorStarPQQueryProcessor(raptorQuery));
    raptorStarPQQueryProcessor->processRaptorStarPQQuery();
    RaptorQueryResult raptorStarPQQueryResult = raptorStarPQQueryProcessor->getRaptorStarQueryResult();

    PrintHelperRaptor::printRaptorQuery(raptorQuery);
    cout << "\nRaptor query result: " << endl;
    PrintHelperRaptor::printRaptorQueryResult(raptorQueryResult);
    cout << "\nRaptor star bound query result: " << endl;
    PrintHelperRaptor::printRaptorQueryResult(raptorStarBoundQueryResult);
    cout << "\nRaptor star PQ query result: " << endl;
    PrintHelperRaptor::printRaptorQueryResult(raptorStarPQQueryResult);
}

void RaptorEATAlgorithmTester::testRaptorEatAlgorithm(RaptorQuery raptorQuery, bool printJourney) {
    unique_ptr<RaptorQueryEATProcessor> raptorQueryProcessor = unique_ptr<RaptorQueryEATProcessor>(new RaptorQueryEATProcessor(raptorQuery));
    raptorQueryProcessor->processRaptorQuery();
    RaptorQueryResult raptorQueryResult = raptorQueryProcessor->getRaptorQueryResult();

    PrintHelperRaptor::printRaptorQuery(raptorQuery);
    cout << "\nRaptor query result: " << endl;
    PrintHelperRaptor::printRaptorQueryResult(raptorQueryResult);

    if (printJourney) {
        Journey journey = raptorQueryProcessor->getJourney();
        cout << "\nJourney: " << endl;
        PrintHelperRaptor::printJourney(journey);
    }
}

void RaptorEATAlgorithmTester::testRaptorStarBoundAlgorithm(RaptorQuery raptorQuery, bool printJourney) {
    unique_ptr<RaptorStarBoundQueryProcessor> raptorStarBoundQueryProcessor = unique_ptr<RaptorStarBoundQueryProcessor>(new RaptorStarBoundQueryProcessor(raptorQuery));
    raptorStarBoundQueryProcessor->processRaptorStarBoundQuery();
    RaptorQueryResult raptorStarBoundQueryResult = raptorStarBoundQueryProcessor->getRaptorStarQueryResult();

    PrintHelperRaptor::printRaptorQuery(raptorQuery);
    cout << "\nRaptor star bound query result: " << endl;
    PrintHelperRaptor::printRaptorQueryResult(raptorStarBoundQueryResult);

    if (printJourney) {
        Journey journey = raptorStarBoundQueryProcessor->getJourney();
        cout << "\nJourney: " << endl;
        PrintHelperRaptor::printJourney(journey);
    }
}

void RaptorEATAlgorithmTester::testRaptorStarPQAlgorithm(RaptorQuery raptorQuery, bool printJourney) {
    unique_ptr<RaptorStarPQQueryProcessor> raptorStarPQQueryProcessor = unique_ptr<RaptorStarPQQueryProcessor>(new RaptorStarPQQueryProcessor(raptorQuery));
    raptorStarPQQueryProcessor->processRaptorStarPQQuery();
    RaptorQueryResult raptorStarPQQueryResult = raptorStarPQQueryProcessor->getRaptorStarQueryResult();

    PrintHelperRaptor::printRaptorQuery(raptorQuery);
    cout << "\nRaptor star PQ query result: " << endl;
    PrintHelperRaptor::printRaptorQueryResult(raptorStarPQQueryResult);

    if (printJourney) {
        Journey journey = raptorStarPQQueryProcessor->getJourney();
        cout << "\nJourney: " << endl;
        PrintHelperRaptor::printJourney(journey);
    }
}

void RaptorEATAlgorithmTester::compareRaptorEATAlgorithms(DataType dataType, int numberOfSuccessfulQueries, bool loadOrStoreQueries) {
    cout << "Processing raptor eat algorithms..." << endl;
    
    string dataTypeString = Importer::getDataTypeString(dataType);
    string folderPathResults = FOLDER_PREFIX + "tests/" + dataTypeString + "/results-eat/";
    string folderPathQueries = FOLDER_PREFIX + "tests/" + dataTypeString + "/queries-eat/";

    time_t now = time(0);
    tm *ltm = localtime(&now);
    string timestamp = to_string(1900 + ltm->tm_year) + "-" + to_string(1 + ltm->tm_mon) + "-" + to_string(ltm->tm_mday) + "-" + to_string(ltm->tm_hour) + "-" + to_string(ltm->tm_min) + "-" + to_string(ltm->tm_sec);

    string resultsFileName = folderPathResults + "raptor-eat-results-" + timestamp + ".csv";

    std::ofstream resultsFile (resultsFileName, std::ofstream::out);
    resultsFile << "avgDuration,medianDuration,maxDuration,minDuration";

    resultsFile << ",avgQueryTimeRaptor,avgQueryTimeRaptorBoundStar,avgQueryTimeRaptorPQStar";
    resultsFile << ",medianQueryTimeRaptor,medianQueryTimeRaptorBoundStar,medianQueryTimeRaptorPQStar";
    resultsFile << ",maxQueryTimeRaptor,maxQueryTimeRaptorBoundStar,maxQueryTimeRaptorPQStar";
    resultsFile << ",minQueryTimeRaptor,minQueryTimeRaptorBoundStar,minQueryTimeRaptorPQStar";

    resultsFile << ",avgExpandedRoutesRaptor,avgExpandedRoutesRaptorBoundStar,avgExpandedRoutesRaptorPQStar";
    resultsFile << ",medianExpandedRoutesRaptor,medianExpandedRoutesRaptorBoundStar,medianExpandedRoutesRaptorPQStar";
    resultsFile << ",maxExpandedRoutesRaptor,maxExpandedRoutesRaptorBoundStar,maxExpandedRoutesRaptorPQStar";
    resultsFile << ",minExpandedRoutesRaptor,minExpandedRoutesRaptorBoundStar,minExpandedRoutesRaptorPQStar";

    resultsFile << ",wrongResultsFractionRaptorBoundStar,wrongResultsFractionRaptorPQStar";

    resultsFile << ",avgLowerBoundSmallerCountRaptorBoundStar,avgLowerBoundSmallerCountRaptorPQStar";
    resultsFile << ",avgLowerBoundGreaterCountRaptorBoundStar,avgLowerBoundGreaterCountRaptorPQStar";
    resultsFile << ",avgLowerBoundSmallerFractionRaptorBoundStar,avgLowerBoundSmallerFractionRaptorPQStar";
    resultsFile << ",avgLowerBoundAbsDiffRaptorBoundStar,avgLowerBoundAbsDiffRaptorPQStar";
    resultsFile << ",avgLowerBoundRelDiffRaptorBoundStar,avgLowerBoundRelDiffRaptorPQStar";

    resultsFile << "\n";

    std::ofstream queriesInfoFile;
    vector<RaptorQuery> raptorQueries;
    if (loadOrStoreQueries) {
        string queriesFileName = folderPathQueries + "raptor-eat-queries-" + to_string(numberOfSuccessfulQueries) + ".csv";
        std::ifstream file(queriesFileName);
        if(file.is_open()) {
            std::string line;
            while(std::getline(file, line)) {
                raptorQueries.push_back(QueryGenerator::parseRaptorQuery(line));
            }
        } else {
            queriesInfoFile.open(queriesFileName, std::ofstream::out);
        }
    }

    int queryCounter = 0;
    int successfulQueryCounter = 0;

    vector<double> durationsRaptor;

    vector<double> queryTimesRaptor;
    vector<double> queryTimesRaptorBoundStar;
    vector<double> queryTimesRaptorPQStar;

    vector<double> expandedRoutesRaptor;
    vector<double> expandedRoutesRaptorBoundStar;
    vector<double> expandedRoutesRaptorPQStar;

    double errorCounterRaptorBoundStar;
    double errorCounterRaptorPQStar;

    vector<double> lowerBoundSmallerCountRaptorBoundStar;
    vector<double> lowerBoundGreaterCountRaptorBoundStar;
    vector<double> lowerBoundSmallerFractionRaptorBoundStar;
    vector<double> lowerBoundAbsDiffRaptorBoundStar;
    vector<double> lowerBoundRelDiffRaptorBoundStar;

    vector<double> lowerBoundSmallerCountRaptorPQStar;
    vector<double> lowerBoundGreaterCountRaptorPQStar;
    vector<double> lowerBoundSmallerFractionRaptorPQStar;
    vector<double> lowerBoundAbsDiffRaptorPQStar;
    vector<double> lowerBoundRelDiffRaptorPQStar;

    while (successfulQueryCounter < numberOfSuccessfulQueries) {
        try {
            queryCounter++;

            RaptorQuery raptorQuery;

            if (loadOrStoreQueries && raptorQueries.size() > 0) {
                raptorQuery = raptorQueries[successfulQueryCounter];
            } else {
                raptorQuery = QueryGenerator::generateRandomRaptorQuery();
            }

            unique_ptr<RaptorQueryEATProcessor> raptorQueryProcessor = unique_ptr<RaptorQueryEATProcessor>(new RaptorQueryEATProcessor(raptorQuery));
            raptorQueryProcessor->processRaptorQuery();
            RaptorQueryResult raptorQueryResult = raptorQueryProcessor->getRaptorQueryResult();

            if (raptorQueryResult.arrivalTime == "") {
                continue;
            }

            successfulQueryCounter++;

            unique_ptr<RaptorStarBoundQueryProcessor> raptorStarBoundQueryProcessor = unique_ptr<RaptorStarBoundQueryProcessor>(new RaptorStarBoundQueryProcessor(raptorQuery));
            raptorStarBoundQueryProcessor->processRaptorStarBoundQuery();
            RaptorQueryResult raptorStarBoundQueryResult = raptorStarBoundQueryProcessor->getRaptorStarQueryResult();

            unique_ptr<RaptorStarPQQueryProcessor> raptorStarPQQueryProcessor = unique_ptr<RaptorStarPQQueryProcessor>(new RaptorStarPQQueryProcessor(raptorQuery));
            raptorStarPQQueryProcessor->processRaptorStarPQQuery();
            RaptorQueryResult raptorStarPQQueryResult = raptorStarPQQueryProcessor->getRaptorStarQueryResult();

            durationsRaptor.push_back((double) raptorQueryResult.durationInSeconds);

            queryTimesRaptor.push_back((double) raptorQueryResult.queryTime);
            queryTimesRaptorBoundStar.push_back((double) raptorStarBoundQueryResult.queryTime);
            queryTimesRaptorPQStar.push_back((double) raptorStarPQQueryResult.queryTime);

            expandedRoutesRaptor.push_back((double) raptorQueryProcessor->numberOfExpandedRoutes);
            expandedRoutesRaptorBoundStar.push_back((double) raptorStarBoundQueryProcessor->numberOfExpandedRoutes);
            expandedRoutesRaptorPQStar.push_back((double) raptorStarPQQueryProcessor->numberOfExpandedRoutes);

            if (raptorQueryResult.arrivalTime != raptorStarBoundQueryResult.arrivalTime) {
                errorCounterRaptorBoundStar++;
            }

            if (raptorQueryResult.arrivalTime != raptorStarPQQueryResult.arrivalTime) {
                errorCounterRaptorPQStar++;
            }

            lowerBoundSmallerCountRaptorBoundStar.push_back((double) raptorStarBoundQueryProcessor->lowerBoundSmallerCounter);
            lowerBoundGreaterCountRaptorBoundStar.push_back((double) raptorStarBoundQueryProcessor->lowerBoundGreaterCounter);
            lowerBoundSmallerFractionRaptorBoundStar.push_back((double) raptorStarBoundQueryProcessor->lowerBoundSmallerFraction);
            lowerBoundAbsDiffRaptorBoundStar.push_back((double) raptorStarBoundQueryProcessor->lowerBoundAbsDiff);
            lowerBoundRelDiffRaptorBoundStar.push_back((double) raptorStarBoundQueryProcessor->lowerBoundRelDiff);

            lowerBoundSmallerCountRaptorPQStar.push_back((double) raptorStarPQQueryProcessor->lowerBoundSmallerCounter);
            lowerBoundGreaterCountRaptorPQStar.push_back((double) raptorStarPQQueryProcessor->lowerBoundGreaterCounter);
            lowerBoundSmallerFractionRaptorPQStar.push_back((double) raptorStarPQQueryProcessor->lowerBoundSmallerFraction);
            lowerBoundAbsDiffRaptorPQStar.push_back((double) raptorStarPQQueryProcessor->lowerBoundAbsDiff);
            lowerBoundRelDiffRaptorPQStar.push_back((double) raptorStarPQQueryProcessor->lowerBoundRelDiff);

            if (loadOrStoreQueries && queriesInfoFile.is_open()) {
                queriesInfoFile << raptorQuery.sourceStopId << "," << raptorQuery.targetStopIds[0] << "," << raptorQuery.sourceTime << "," << raptorQuery.weekday << "\n";
            }

            if (successfulQueryCounter % (numberOfSuccessfulQueries / 5) == 0) {
                cout << "Progress: " << successfulQueryCounter << " / " << numberOfSuccessfulQueries << endl;
            }

        } catch (...) {
            cout << "Exception caught" << endl;
            continue;
        }
    }

    if (loadOrStoreQueries && queriesInfoFile.is_open()) {
        queriesInfoFile.close();
    }

    double avgDuration = Calculator::getAverage(durationsRaptor);
    double medianDuration = Calculator::getMedian(durationsRaptor);
    double maxDuration = Calculator::getMaximum(durationsRaptor);
    double minDuration = Calculator::getMinimum(durationsRaptor);

    double avgQueryTimeRaptor = Calculator::getAverage(queryTimesRaptor);
    double avgQueryTimeRaptorBoundStar = Calculator::getAverage(queryTimesRaptorBoundStar);
    double avgQueryTimeRaptorPQStar = Calculator::getAverage(queryTimesRaptorPQStar);
    double medianQueryTimeRaptor = Calculator::getMedian(queryTimesRaptor);
    double medianQueryTimeRaptorBoundStar = Calculator::getMedian(queryTimesRaptorBoundStar);
    double medianQueryTimeRaptorPQStar = Calculator::getMedian(queryTimesRaptorPQStar);
    double maxQueryTimeRaptor = Calculator::getMaximum(queryTimesRaptor);
    double maxQueryTimeRaptorBoundStar = Calculator::getMaximum(queryTimesRaptorBoundStar);
    double maxQueryTimeRaptorPQStar = Calculator::getMaximum(queryTimesRaptorPQStar);
    double minQueryTimeRaptor = Calculator::getMinimum(queryTimesRaptor);
    double minQueryTimeRaptorBoundStar = Calculator::getMinimum(queryTimesRaptorBoundStar);
    double minQueryTimeRaptorPQStar = Calculator::getMinimum(queryTimesRaptorPQStar);

    double avgExpandedRoutesRaptor = Calculator::getAverage(expandedRoutesRaptor);
    double avgExpandedRoutesRaptorBoundStar = Calculator::getAverage(expandedRoutesRaptorBoundStar);
    double avgExpandedRoutesRaptorPQStar = Calculator::getAverage(expandedRoutesRaptorPQStar);
    double medianExpandedRoutesRaptor = Calculator::getMedian(expandedRoutesRaptor);
    double medianExpandedRoutesRaptorBoundStar = Calculator::getMedian(expandedRoutesRaptorBoundStar);
    double medianExpandedRoutesRaptorPQStar = Calculator::getMedian(expandedRoutesRaptorPQStar);
    double maxExpandedRoutesRaptor = Calculator::getMaximum(expandedRoutesRaptor);
    double maxExpandedRoutesRaptorBoundStar = Calculator::getMaximum(expandedRoutesRaptorBoundStar);
    double maxExpandedRoutesRaptorPQStar = Calculator::getMaximum(expandedRoutesRaptorPQStar);
    double minExpandedRoutesRaptor = Calculator::getMinimum(expandedRoutesRaptor);
    double minExpandedRoutesRaptorBoundStar = Calculator::getMinimum(expandedRoutesRaptorBoundStar);
    double minExpandedRoutesRaptorPQStar = Calculator::getMinimum(expandedRoutesRaptorPQStar);

    double wrongResultsFractionRaptorBoundStar = errorCounterRaptorBoundStar / (double) successfulQueryCounter;
    double wrongResultsFractionRaptorPQStar = errorCounterRaptorPQStar / (double) successfulQueryCounter;

    double avgLowerBoundSmallerCountRaptorBoundStar = Calculator::getAverage(lowerBoundSmallerCountRaptorBoundStar);
    double avgLowerBoundGreaterCountRaptorBoundStar = Calculator::getAverage(lowerBoundGreaterCountRaptorBoundStar);
    double avgLowerBoundSmallerFractionRaptorBoundStar = Calculator::getAverage(lowerBoundSmallerFractionRaptorBoundStar);
    double avgLowerBoundAbsDiffRaptorBoundStar = Calculator::getAverage(lowerBoundAbsDiffRaptorBoundStar);
    double avgLowerBoundRelDiffRaptorBoundStar = Calculator::getAverage(lowerBoundRelDiffRaptorBoundStar);

    double avgLowerBoundSmallerCountRaptorPQStar = Calculator::getAverage(lowerBoundSmallerCountRaptorPQStar);
    double avgLowerBoundGreaterCountRaptorPQStar = Calculator::getAverage(lowerBoundGreaterCountRaptorPQStar);
    double avgLowerBoundSmallerFractionRaptorPQStar = Calculator::getAverage(lowerBoundSmallerFractionRaptorPQStar);
    double avgLowerBoundAbsDiffRaptorPQStar = Calculator::getAverage(lowerBoundAbsDiffRaptorPQStar);
    double avgLowerBoundRelDiffRaptorPQStar = Calculator::getAverage(lowerBoundRelDiffRaptorPQStar);

    resultsFile << avgDuration << "," << medianDuration << "," << maxDuration << "," << minDuration;
    resultsFile << "," << avgQueryTimeRaptor << "," << avgQueryTimeRaptorBoundStar << "," << avgQueryTimeRaptorPQStar;
    resultsFile << "," << medianQueryTimeRaptor << "," << medianQueryTimeRaptorBoundStar << "," << medianQueryTimeRaptorPQStar;
    resultsFile << "," << maxQueryTimeRaptor << "," << maxQueryTimeRaptorBoundStar << "," << maxQueryTimeRaptorPQStar;
    resultsFile << "," << minQueryTimeRaptor << "," << minQueryTimeRaptorBoundStar << "," << minQueryTimeRaptorPQStar;

    resultsFile << "," << avgExpandedRoutesRaptor << "," << avgExpandedRoutesRaptorBoundStar << "," << avgExpandedRoutesRaptorPQStar;
    resultsFile << "," << medianExpandedRoutesRaptor << "," << medianExpandedRoutesRaptorBoundStar << "," << medianExpandedRoutesRaptorPQStar;
    resultsFile << "," << maxExpandedRoutesRaptor << "," << maxExpandedRoutesRaptorBoundStar << "," << maxExpandedRoutesRaptorPQStar;
    resultsFile << "," << minExpandedRoutesRaptor << "," << minExpandedRoutesRaptorBoundStar << "," << minExpandedRoutesRaptorPQStar;

    resultsFile << "," << wrongResultsFractionRaptorBoundStar << "," << wrongResultsFractionRaptorPQStar;

    resultsFile << "," << avgLowerBoundSmallerCountRaptorBoundStar << "," << avgLowerBoundSmallerCountRaptorPQStar;
    resultsFile << "," << avgLowerBoundGreaterCountRaptorBoundStar << "," << avgLowerBoundGreaterCountRaptorPQStar;
    resultsFile << "," << avgLowerBoundSmallerFractionRaptorBoundStar << "," << avgLowerBoundSmallerFractionRaptorPQStar;
    resultsFile << "," << avgLowerBoundAbsDiffRaptorBoundStar << "," << avgLowerBoundAbsDiffRaptorPQStar;
    resultsFile << "," << avgLowerBoundRelDiffRaptorBoundStar << "," << avgLowerBoundRelDiffRaptorPQStar;

    resultsFile << "\n";

    cout << "Rate of successful queries: " << (double) successfulQueryCounter / (double) queryCounter << endl;

    cout << "\nAverage duration: " << avgDuration << " s" << endl;
    cout << "Median duration: " << medianDuration << " s" << endl;
    cout << "Max duration: " << maxDuration << " s" << endl;
    cout << "Min duration: " << minDuration << " s" << endl;

    cout << "\nAverage query time Raptor: " << avgQueryTimeRaptor << " ms" << endl;
    cout << "Average query time Raptor bound star: " << avgQueryTimeRaptorBoundStar << " ms" << endl;
    cout << "Average query time Raptor PQ star: " << avgQueryTimeRaptorPQStar << " ms" << endl;
    cout << "Median query time Raptor: " << medianQueryTimeRaptor << " ms" << endl;
    cout << "Median query time Raptor bound star: " << medianQueryTimeRaptorBoundStar << " ms" << endl;
    cout << "Median query time Raptor PQ star: " << medianQueryTimeRaptorPQStar << " ms" << endl;
    cout << "Max query time Raptor: " << maxQueryTimeRaptor << " ms" << endl;
    cout << "Max query time Raptor bound star: " << maxQueryTimeRaptorBoundStar << " ms" << endl;
    cout << "Max query time Raptor PQ star: " << maxQueryTimeRaptorPQStar << " ms" << endl;
    cout << "Min query time Raptor: " << minQueryTimeRaptor << " ms" << endl;
    cout << "Min query time Raptor bound star: " << minQueryTimeRaptorBoundStar << " ms" << endl;
    cout << "Min query time Raptor PQ star: " << minQueryTimeRaptorPQStar << " ms" << endl;

    cout << "\nAverage expanded routes Raptor: " << avgExpandedRoutesRaptor << endl;
    cout << "Average expanded routes Raptor bound star: " << avgExpandedRoutesRaptorBoundStar << endl;
    cout << "Average expanded routes Raptor PQ star: " << avgExpandedRoutesRaptorPQStar << endl;
    cout << "Median expanded routes Raptor: " << medianExpandedRoutesRaptor << endl;
    cout << "Median expanded routes Raptor bound star: " << medianExpandedRoutesRaptorBoundStar << endl;
    cout << "Median expanded routes Raptor PQ star: " << medianExpandedRoutesRaptorPQStar << endl;
    cout << "Max expanded routes Raptor: " << maxExpandedRoutesRaptor << endl;
    cout << "Max expanded routes Raptor bound star: " << maxExpandedRoutesRaptorBoundStar << endl;
    cout << "Max expanded routes Raptor PQ star: " << maxExpandedRoutesRaptorPQStar << endl;
    cout << "Min expanded routes Raptor: " << minExpandedRoutesRaptor << endl;
    cout << "Min expanded routes Raptor bound star: " << minExpandedRoutesRaptorBoundStar << endl;
    cout << "Min expanded routes Raptor PQ star: " << minExpandedRoutesRaptorPQStar << endl;

    cout << "\nWrong results fraction Raptor bound star: " << wrongResultsFractionRaptorBoundStar << endl;
    cout << "Wrong results fraction Raptor PQ star: " << wrongResultsFractionRaptorPQStar << endl;

    cout << "\nAverage lower bound smaller count Raptor bound star: " << avgLowerBoundSmallerCountRaptorBoundStar << endl;
    cout << "Average lower bound smaller count Raptor PQ star: " << avgLowerBoundSmallerCountRaptorPQStar << endl;
    cout << "Average lower bound greater count Raptor bound star: " << avgLowerBoundGreaterCountRaptorBoundStar << endl;
    cout << "Average lower bound greater count Raptor PQ star: " << avgLowerBoundGreaterCountRaptorPQStar << endl;
    cout << "Average lower bound smaller fraction Raptor bound star: " << avgLowerBoundSmallerFractionRaptorBoundStar << endl;
    cout << "Average lower bound smaller fraction Raptor PQ star: " << avgLowerBoundSmallerFractionRaptorPQStar << endl;
    cout << "Average lower bound abs diff Raptor bound star: " << avgLowerBoundAbsDiffRaptorBoundStar << endl;
    cout << "Average lower bound abs diff Raptor PQ star: " << avgLowerBoundAbsDiffRaptorPQStar << endl;
    cout << "Average lower bound rel diff Raptor bound star: " << avgLowerBoundRelDiffRaptorBoundStar << endl;
    cout << "Average lower bound rel diff Raptor PQ star: " << avgLowerBoundRelDiffRaptorPQStar << endl;
    cout << "\n\n";
}

void PrintHelperRaptor::printRaptorQuery(RaptorQuery raptorQuery) {
    cout << "\nRaptor query: " << endl;
    cout << "Source stop: " << Importer::getStopName(raptorQuery.sourceStopId) << endl;
    cout << "Target stop: " << Importer::getStopName(raptorQuery.targetStopIds[0]) << endl;
    cout << "Source time: " << TimeConverter::convertSecondsToTime(raptorQuery.sourceTime, true) << endl;
    cout << "Weekday: " << WeekdayConverter::convertIntToWeekday(raptorQuery.weekday) << endl;
}

void PrintHelperRaptor::printRaptorQueryResult(RaptorQueryResult raptorQueryResult) {
    if (raptorQueryResult.arrivalTime != ""){
        cout << "Arrival time: " << raptorQueryResult.arrivalTime << endl;
        cout << "Duration: " << TimeConverter::convertSecondsToTime(raptorQueryResult.durationInSeconds, false) << endl;
    } else {
        cout << "No connection found" << endl;
    }
    cout << "Query time: " << raptorQueryResult.queryTime << " ms" << endl;
}

/*
    Print the journey.
*/
void PrintHelperRaptor::printJourney(Journey journey) {
    cout << "Journey duration: " << TimeConverter::convertSecondsToTime(journey.duration, false) << endl;
    for (int i = 0; i < journey.legs.size(); i++) {
        cout << "Leg " << i+1 << " - ";
        cout << "Departure stop: " << journey.legs[i].departureStopName;
        cout << ", Departure time: " << TimeConverter::convertSecondsToTime(journey.legs[i].departureTime, true);
        cout << ", Arrival stop: " << journey.legs[i].arrivalStopName;
        cout << ", Arrival time: " << TimeConverter::convertSecondsToTime(journey.legs[i].arrivalTime, true)<< endl;
    }
}