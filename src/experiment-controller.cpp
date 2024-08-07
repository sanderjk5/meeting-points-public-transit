#include"experiment-controller.h"

#include <iostream>
#include <sstream>
#include <fstream>
#include"data-handling/importer.h"
#include<string>
#include"meeting-point-algorithms/algorithm-tester.h"
#include"meeting-point-algorithms/algorithm-tester-raptor.h"
#include"meeting-point-algorithms/lower-bound-tester.h"
#include"meeting-point-algorithms/query-processor.h"
#include"data-structures/g-tree.h"
#include"data-structures/creator.h"
#include"data-handling/converter.h"
#include"data-structures/graph.h"
#include"constants.h"
#include"limits.h"
#include "gtree-controller.h"

/*
    Find the best parameters for the GTree algorithm. Compares the run time and accuracy of the GTree algorithm with different parameters.
*/
void ExperimentController::findBestGTreeParameters(DataType dataType, int numberOfSourceStops, int numberOfSuccessfulQueries) {
    vector<int> numberOfChildrenPerNodeParams = {2, 4};
    vector<int> maxNumberOfVerticesPerLeafParams = {16, 32, 64, 128};

    // vector<int> numberOfChildrenPerNodeParams = {2};
    // vector<int> maxNumberOfVerticesPerLeafParams = {16, 64};

    double bestRunTimeCSA = INT_MAX;
    int bestNumberOfChildrenPerNodeRunTimeCSA = 0;
    int bestMaxNumberOfVerticesPerLeafRunTimeCSA = 0;

    double bestRunTimeApprox = INT_MAX;
    int bestNumberOfChildrenPerNodeRunTimeApprox = 0;
    int bestMaxNumberOfVerticesPerLeafRunTimeApprox = 0;

    double bestAccuracyMinSum = 0;
    int bestNumberOfChildrenPerNodeAccuracyMinSum = 0;
    int bestMaxNumberOfVerticesPerLeafAccuracyMinSum = 0;

    double bestAccuracyMinMax = 0;
    int bestNumberOfChildrenPerNodeAccuracyMinMax = 0;
    int bestMaxNumberOfVerticesPerLeafAccuracyMinMax = 0;
    
    for (int numberOfChildrenPerNode : numberOfChildrenPerNodeParams) {
        for (int maxNumberOfVerticesPerLeaf : maxNumberOfVerticesPerLeafParams) {
            GTree* gTreePointer = GTreeController::createOrLoadNetworkGTree(dataType, numberOfChildrenPerNode, maxNumberOfVerticesPerLeaf);
            
            AverageRunTimeAndAccuracy averageRunTimeAndAccuracy = GTreeAlgorithmTester::getAverageRunTimeAndAccuracy(dataType, gTreePointer, numberOfSourceStops, numberOfSuccessfulQueries);

            if (averageRunTimeAndAccuracy.averageRunTimeGTreeCSA < bestRunTimeCSA) {
                bestRunTimeCSA = averageRunTimeAndAccuracy.averageRunTimeGTreeCSA;
                bestNumberOfChildrenPerNodeRunTimeCSA = numberOfChildrenPerNode;
                bestMaxNumberOfVerticesPerLeafRunTimeCSA = maxNumberOfVerticesPerLeaf;
            }

            if (averageRunTimeAndAccuracy.averageRunTimeGTreeApproximation < bestRunTimeApprox) {
                bestRunTimeApprox = averageRunTimeAndAccuracy.averageRunTimeGTreeApproximation;
                bestNumberOfChildrenPerNodeRunTimeApprox = numberOfChildrenPerNode;
                bestMaxNumberOfVerticesPerLeafRunTimeApprox = maxNumberOfVerticesPerLeaf;
            }

            if (averageRunTimeAndAccuracy.averageAccuracyMinSum > bestAccuracyMinSum) {
                bestAccuracyMinSum = averageRunTimeAndAccuracy.averageAccuracyMinSum;
                bestNumberOfChildrenPerNodeAccuracyMinSum = numberOfChildrenPerNode;
                bestMaxNumberOfVerticesPerLeafAccuracyMinSum = maxNumberOfVerticesPerLeaf;
            }

            if (averageRunTimeAndAccuracy.averageAccuracyMinMax > bestAccuracyMinMax) {
                bestAccuracyMinMax = averageRunTimeAndAccuracy.averageAccuracyMinMax;
                bestNumberOfChildrenPerNodeAccuracyMinMax = numberOfChildrenPerNode;
                bestMaxNumberOfVerticesPerLeafAccuracyMinMax = maxNumberOfVerticesPerLeaf;
            }
        }
    }

    string dataTypeString = Importer::getDataTypeString(dataType);
    string folderPathParams = FOLDER_PREFIX + "tests/" + dataTypeString + "/gtree_params/";

    string filePath = folderPathParams + "gtree_best_params.csv";

    std::ofstream paramsFile;
    paramsFile.open(filePath, std::ofstream::out);

    paramsFile << "numberOfChildrenPerNodeRunTimeCSA,maxNumberOfVerticesPerLeafRunTimeCSA,numberOfChildrenPerNodeRunTimeApprox,maxNumberOfVerticesPerLeafRunTimeApprox,numberOfChildrenPerNodeAccuracyMinSum,maxNumberOfVerticesPerLeafAccuracyMinSum,numberOfChildrenPerNodeAccuracyMinMax,maxNumberOfVerticesPerLeafAccuracyMinMax" << endl;
    paramsFile << bestNumberOfChildrenPerNodeRunTimeCSA << "," << bestMaxNumberOfVerticesPerLeafRunTimeCSA << "," << bestNumberOfChildrenPerNodeRunTimeApprox << "," << bestMaxNumberOfVerticesPerLeafRunTimeApprox << "," << bestNumberOfChildrenPerNodeAccuracyMinSum << "," << bestMaxNumberOfVerticesPerLeafAccuracyMinSum << "," << bestNumberOfChildrenPerNodeAccuracyMinMax << "," << bestMaxNumberOfVerticesPerLeafAccuracyMinMax << endl;

    paramsFile.close();

    cout << "Best parameters for CSA run time: " << bestNumberOfChildrenPerNodeRunTimeCSA << " children per node, " << bestMaxNumberOfVerticesPerLeafRunTimeCSA << " vertices per leaf" << endl;
    cout << "Best parameters for approximation run time: " << bestNumberOfChildrenPerNodeRunTimeApprox << " children per node, " << bestMaxNumberOfVerticesPerLeafRunTimeApprox << " vertices per leaf" << endl;
    cout << "Best parameters for accuracy min sum: " << bestNumberOfChildrenPerNodeAccuracyMinSum << " children per node, " << bestMaxNumberOfVerticesPerLeafAccuracyMinSum << " vertices per leaf" << endl;
    cout << "Best parameters for accuracy min max: " << bestNumberOfChildrenPerNodeAccuracyMinMax << " children per node, " << bestMaxNumberOfVerticesPerLeafAccuracyMinMax << " vertices per leaf" << endl;
}

/*
    Test and compare the different algorithms with a set of queries. The results are written to a file.
*/
void ExperimentController::testAndCompareAlgorithmsRandom(DataType dataType, int numberOfSuccessfulQueries, vector<int> numberOfSourceStops) {
    // GTree* networkGTreePointer = GTreeController::createOrLoadNetworkGTree(dataType, 4, 64);
    // AlgorithmComparer::compareAlgorithmsRandom(dataType, networkGTreePointer, numberOfSuccessfulQueries, numberOfSourceStops, true, true);
    AlgorithmComparer::compareAlgorithmsWithoutGTreesRandom(dataType, numberOfSuccessfulQueries, numberOfSourceStops, true, true);
}

void ExperimentController::compareRaptorAlgorithms(DataType dataType, int numberOfSuccessfulQueries, vector<int> numberOfSourceStops) {
    RaptorAlgorithmTester::compareRaptorAlgorithms(dataType, numberOfSuccessfulQueries, numberOfSourceStops, true);
}

void ExperimentController::compareRaptorPQAlgorithms(DataType dataType, int numberOfSuccessfulQueries, vector<int> numberOfSourceStops) {
    RaptorPQAlgorithmTester::compareRaptorPQAlgorithms(dataType, numberOfSuccessfulQueries, numberOfSourceStops, true);
}

void ExperimentController::compareRaptorApproxAlgorithms(DataType dataType, int numberOfSuccessfulQueries, vector<int> numberOfSourceStops) {
    RaptorApproximationAlgorithmTester::compareRaptorApproximationAlgorithms(dataType, numberOfSuccessfulQueries, numberOfSourceStops, true);
}

void ExperimentController::testRaptorApproxAlgorithmForLargeNofSources(DataType dataType, int numberOfSuccessfulQueries, vector<int> numberOfSourceStops) {
    RaptorApproximationAlgorithmTester::testRaptorApproximationAlgorithmForLargeNofSources(dataType, numberOfSuccessfulQueries, numberOfSourceStops, true);
}

void ExperimentController::compareRaptorEATAlgorithms(DataType dataType, int numberOfSuccessfulQueries) {
    RaptorEATAlgorithmTester::compareRaptorEATAlgorithms(dataType, numberOfSuccessfulQueries, true);
}

void ExperimentController::evaluateLowerBounds(int numberOfSources, int numberOfTargetsPerSource) {
    LowerBoundTester::getLowerBoundDiffs(numberOfSources, numberOfTargetsPerSource);
}

void ExperimentController::testRaptorNaiveAlgorithmMultipleResult(int numberOfSuccessfulQueries, vector<int> numberOfSources) {
    RaptorAlgorithmTester::testRaptorNaiveAlgorithmRandom(numberOfSuccessfulQueries, numberOfSources);
}