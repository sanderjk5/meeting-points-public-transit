#ifndef CMAKE_ALGORITHM_TESTER_H
#define CMAKE_ALGORITHM_TESTER_H

#include "query-processor.h"

class NaiveAlgorithmTester {
    public:
        explicit NaiveAlgorithmTester(){};
        ~NaiveAlgorithmTester(){};

        static void testNaiveAlgorithmRandom(int numberOfQueries, int numberOfSources, int numberOfDays = 1, bool printTime = false, bool printOnlySuccessful = false);
        static void testNaiveAlgorithm(MeetingPointQuery meetingPointQuery, bool printTime = false);
};

#endif //CMAKE_ALGORITHM_TESTER_H