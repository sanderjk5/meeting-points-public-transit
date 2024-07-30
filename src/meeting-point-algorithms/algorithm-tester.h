#ifndef CMAKE_ALGORITHM_TESTER_H
#define CMAKE_ALGORITHM_TESTER_H

#include "query-processor.h"
#include <../data-structures/g-tree.h>
#include <../data-handling/importer.h>
#include "journey.h"

#include <vector>

struct AverageRunTimeAndAccuracy {
    double averageRunTimeGTreeCSA;
    double averageRunTimeGTreeApproximation;
    double averageAccuracyMinSum;
    double averageAccuracyMinMax;
};

class NaiveAlgorithmTester {
    public:
        explicit NaiveAlgorithmTester(){};
        ~NaiveAlgorithmTester(){};

        static void testNaiveAlgorithmRandom(int numberOfSuccessfulQueries, int numberOfSources, bool printOnlySuccessful = false);
        static void testNaiveAlgorithm(MeetingPointQuery meetingPointQuery, bool printJourneys = false);
};

class NaiveKeyStopAlgorithmTester {
    public:
        explicit NaiveKeyStopAlgorithmTester(){};
        ~NaiveKeyStopAlgorithmTester(){};

        static void testNaiveKeyStopAlgorithmRandom(DataType dataType, int numberOfSuccessfulQueries, int numberOfSources, bool printOnlySuccessful = false);
        static void testNaiveKeyStopAlgorithm(DataType dataType, MeetingPointQuery meetingPointQuery, bool printJourneys = false);
};

class GTreeAlgorithmTester {
    public:
        explicit GTreeAlgorithmTester(){};
        ~GTreeAlgorithmTester(){};

        static void testGTreeAlgorithmRandom(GTree* gTree, bool useCSA, int numberOfSuccessfulQueries, int numberOfSources, bool printOnlySuccessful = false);
        static void testGTreeAlgorithm(GTree* gTree, MeetingPointQuery meetingPointQuery, bool useCSA, bool printJourneys = false);

        static AverageRunTimeAndAccuracy getAverageRunTimeAndAccuracy(DataType dataType, GTree* gTree, int numberOfSourceStops, int numberOfSuccessfulQueries);
};

class RaptorAlgorithmTester {
    public:
        explicit RaptorAlgorithmTester(){};
        ~RaptorAlgorithmTester(){};

        static void testRaptorAlgorithmRandom(int numberOfSuccessfulQueries, int numberOfSources, bool printOnlySuccessful = false);
        static void testRaptorNaiveAlgorithmRandom(int numberOfSuccessfulQueries, vector<int> numberOfSources);
        static void testRaptorAlgorithm(MeetingPointQuery meetingPointQuery, bool printJourneys = true);
        static void testRaptorFirstAlgorithm(MeetingPointQuery meetingPointQuery, bool printJourneys = true);
        static void compareRaptorAlgorithms(DataType dataType, int numberOfSuccessfulQueries, vector<int> numberOfSources, bool loadOrStoreQueries);
};

class RaptorPQAlgorithmTester {
    public:
        explicit RaptorPQAlgorithmTester(){};
        ~RaptorPQAlgorithmTester(){};

        static void testRaptorPQAlgorithm(MeetingPointQuery meetingPointQuery, bool printJourneys = true);
        static void compareRaptorPQAlgorithms(DataType dataType, int numberOfSuccessfulQueries, vector<int> numberOfSources, bool loadOrStoreQueries);
};

class RaptorBoundAlgorithmTester {
    public:
        explicit RaptorBoundAlgorithmTester(){};
        ~RaptorBoundAlgorithmTester(){};

        static void testRaptorBoundAlgorithm(MeetingPointQuery meetingPointQuery, bool printJourneys = true);
};

class RaptorApproximationAlgorithmTester {
    public:
        explicit RaptorApproximationAlgorithmTester(){};
        ~RaptorApproximationAlgorithmTester(){};

        static void testRaptorApproximationAlgorithm(MeetingPointQuery meetingPointQuery, bool useCandidates);
        static void testRaptorLoopAlgorithm(MeetingPointQuery meetingPointQuery);
        static void compareRaptorApproximationAlgorithms(DataType dataType, int numberOfSuccessfulQueries, vector<int> numberOfSources, bool loadOrStoreQueries);
        static void testRaptorApproximationAlgorithmForLargeNofSources(DataType dataType, int numberOfSuccessfulQueries, vector<int> numberOfSources, bool loadOrStoreQueries);
};

class AlgorithmComparer {
    public:
        explicit AlgorithmComparer(){};
        ~AlgorithmComparer(){};

        static void compareAlgorithmsRandom(DataType dataType, GTree* gTree, int numberOfSuccessfulQueries, vector<int> numberOfSources, bool printResults = false, bool loadOrStoreQueries = false);
        static void compareAlgorithmsWithoutGTreesRandom(DataType dataType, int numberOfSuccessfulQueries, vector<int> numberOfSources, bool printResults = false, bool loadOrStoreQueries = false);
        static void compareAlgorithms(DataType dataType, GTree* gTree, MeetingPointQuery meetingPointQuery);
        static void compareAlgorithmsWithoutGTree(DataType dataType, MeetingPointQuery meetingPointQuery);
};

class PrintHelper {
    public:
        explicit PrintHelper(){};
        ~PrintHelper(){};

        static void printMeetingPointQuery(MeetingPointQuery meetingPointQuery);
        static void printMeetingPointQueryResult(MeetingPointQueryResult meetingPointQueryResult);
        static void printMeetingPointQueryResultOfOptimization(MeetingPointQueryResult meetingPointQueryResult, Optimization optimization);
        static void printGTreeCSAInfo(MeetingPointQueryGTreeCSAInfo meetingPointQueryGTreeCSAInfo);
        static void printGTreeApproxInfo(GTreeQueryProcessor gTreeQueryProcessor);
        static void printJourney(Journey journey);
};

class Calculator {
    public:
        explicit Calculator(){};
        ~Calculator(){};

        static double getAverage(vector<double> numbers);
        static double getMedian(vector<double> numbers);
        static double getMaximum(vector<double> numbers);
        static double getMinimum(vector<double> numbers);
};

#endif //CMAKE_ALGORITHM_TESTER_H