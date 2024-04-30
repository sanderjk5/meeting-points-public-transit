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

        static void compareRaptorAlgorithms(DataType dataType, int numberOfSuccessfulQueries, bool loadOrStoreQueries);
};

class PrintHelperRaptor {
    public:
        explicit PrintHelperRaptor(){};
        ~PrintHelperRaptor(){};

        static void printRaptorQuery(RaptorQuery raptorQuery);
        static void printRaptorQueryResult(RaptorQueryResult raptorQueryResult);
};

#endif //CMAKE_ALGORITHM_TESTER_RAPTOR_H