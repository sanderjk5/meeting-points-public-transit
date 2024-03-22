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
        static void testRaptorAlgorithm(MeetingPointQuery meetingPointQuery);
};

class AlgorithmComparer {
    public:
        explicit AlgorithmComparer(){};
        ~AlgorithmComparer(){};

        static void compareAlgorithmsRandom(DataType dataType, GTree* gTree, int numberOfSuccessfulQueries, vector<int> numberOfSources, bool printResults = false, bool loadOrStoreQueries = false);
        static void compareAlgorithms(DataType dataType, GTree* gTree, MeetingPointQuery meetingPointQuery);
};

class PrintHelper {
    public:
        explicit PrintHelper(){};
        ~PrintHelper(){};

        static void printMeetingPointQuery(MeetingPointQuery meetingPointQuery);
        static void printMeetingPointQueryResult(MeetingPointQueryResult meetingPointQueryResult);
        static void printMeetingPointQueryRaptorResult(MeetingPointQueryRaptorResult meetingPointQueryResult);
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