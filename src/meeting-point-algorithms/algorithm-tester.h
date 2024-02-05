#ifndef CMAKE_ALGORITHM_TESTER_H
#define CMAKE_ALGORITHM_TESTER_H

#include "query-processor.h"
#include <../data-structures/g-tree.h>
#include "journey.h"

class NaiveAlgorithmTester {
    public:
        explicit NaiveAlgorithmTester(){};
        ~NaiveAlgorithmTester(){};

        static void testNaiveAlgorithmRandom(int numberOfQueries, int numberOfSources, int numberOfDays = 1, bool printTime = false, bool printOnlySuccessful = false);
        static void testNaiveAlgorithm(MeetingPointQuery meetingPointQuery, bool printTime = false, bool printJourneys = false);
};

class GTreeAlgorithmTester {
    public:
        explicit GTreeAlgorithmTester(){};
        ~GTreeAlgorithmTester(){};

        static void testGTreeAlgorithmRandom(GTree* gTree, int numberOfQueries, int numberOfSources, int numberOfDays = 1, bool printTime = false, bool printOnlySuccessful = false);
        static void testGTreeAlgorithm(GTree* gTree, MeetingPointQuery meetingPointQuery, bool printTime = false, bool printJourneys = false);
};

class AlgorithmComparer {
    public:
        explicit AlgorithmComparer(){};
        ~AlgorithmComparer(){};

        static void compareAlgorithmsRandom(GTree* gTree, int numberOfQueries, int numberOfSources, int numberOfDays = 1, bool printTime = false, bool printEveryResult = false);
        static void compareAlgorithms(GTree* gTree, MeetingPointQuery meetingPointQuery, bool printTime = false);
};

class PrintHelper {
    public:
        explicit PrintHelper(){};
        ~PrintHelper(){};

        static void printMeetingPointQuery(MeetingPointQuery meetingPointQuery);
        static void printMeetingPointQueryResult(MeetingPointQueryResult meetingPointQueryResult);
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