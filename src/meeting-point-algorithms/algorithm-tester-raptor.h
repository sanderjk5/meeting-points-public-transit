#ifndef CMAKE_ALGORITHM_TESTER_RAPTOR_H
#define CMAKE_ALGORITHM_TESTER_RAPTOR_H

#include "query-processor-raptor.h"
#include "raptor.h"
#include <../data-handling/importer.h>
#include "journey.h"

#include <vector>
#include <memory>

class RaptorEATAlgorithmTester {
    public:
        explicit RaptorEATAlgorithmTester(){};
        ~RaptorEATAlgorithmTester(){};

        static void testRaptorAlgorithms(RaptorQuery raptorQuery);
        static void testRaptorEatAlgorithm(RaptorQuery raptorQuery, bool printJourney);
        static void testRaptorStarBoundAlgorithm(RaptorQuery raptorQuery, bool printJourney);
        static void testRaptorStarPQAlgorithm(RaptorQuery raptorQuery, bool printJourney);
        static void compareRaptorEATAlgorithms(DataType dataType, int numberOfSuccessfulQueries, bool loadOrStoreQueries);
};

class PrintHelperRaptor {
    public:
        explicit PrintHelperRaptor(){};
        ~PrintHelperRaptor(){};

        static void printRaptorQuery(RaptorQuery raptorQuery);
        static void printRaptorQueryResult(RaptorQueryResult raptorQueryResult);
        static void printJourney(Journey journey);
};

#endif //CMAKE_ALGORITHM_TESTER_RAPTOR_H