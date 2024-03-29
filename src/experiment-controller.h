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
};

#endif //CMAKE_EXPERIMENT_CONTROLLER_H