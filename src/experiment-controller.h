#ifndef CMAKE_EXPERIMENT_CONTROLLER_H
#define CMAKE_EXPERIMENT_CONTROLLER_H

#include"data-handling/importer.h"

class ExperimentController {
    public:
        explicit ExperimentController(){};
        ~ExperimentController(){};

        static void findBestGTreeParameters(DataType dataType, int numberOfSourceStops, int numberOfSuccessfulQueries);
        static void testAndCompareAlgorithmsRandom(DataType dataType, int numberOfSuccessfulQueries, vector<int> numberOfSourceStops);
        static void compareRaptorAlgorithms(DataType dataType, int numberOfSuccessfulQueries, vector<int> numberOfSourceStops);
        static void compareRaptorPQAlgorithms(DataType dataType, int numberOfSuccessfulQueries, vector<int> numberOfSourceStops);
        static void compareRaptorApproxAlgorithms(DataType dataType, int numberOfSuccessfulQueries, vector<int> numberOfSourceStops);
        static void testRaptorApproxAlgorithmForLargeNofSources(DataType dataType, int numberOfSuccessfulQueries, vector<int> numberOfSourceStops);
        static void compareRaptorEATAlgorithms(DataType dataType, int numberOfSuccessfulQueries);
        static void evaluateLowerBounds(int numberOfSources, int numberOfTargetsPerSource);
        static void testRaptorNaiveAlgorithmMultipleResult(int numberOfSuccessfulQueries, vector<int> numberOfSources);
};

#endif //CMAKE_EXPERIMENT_CONTROLLER_H