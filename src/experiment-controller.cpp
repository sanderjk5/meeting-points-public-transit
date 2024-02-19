#include"experiment-controller.h"

#include<iostream>
#include"data-handling/importer.h"
#include<string>
#include"meeting-point-algorithms/algorithm-tester.h"
#include"meeting-point-algorithms/query-processor.h"
#include"data-structures/g-tree.h"
#include"data-structures/creator.h"
#include"data-handling/converter.h"
#include"data-structures/graph.h"
#include"constants.h"
#include"limits.h"

void ExperimentController::findBestGTreeParameters(DataType dataType, int numberOfSourceStops, int numberOfSuccessfulQueries) {
  vector<int> numberOfChildrenPerNodeParams = {2};
  vector<int> maxNumberOfVerticesPerLeafParams = {16, 256};

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
        GTree gTree = Creator::createNetworkGTree(numberOfChildrenPerNode, maxNumberOfVerticesPerLeaf);
        GTree* gTreePointer = &gTree;
        gTreePointer->initializeGTree();

        AverageRunTimeAndAccuracy averageRunTimeAndAccuracy = GTreeAlgorithmTester::getAverageRunTimeAndAccuracy(dataType, gTreePointer, 2, 10);

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

  cout << "Best parameters for CSA run time: " << bestNumberOfChildrenPerNodeRunTimeCSA << " children per node, " << bestMaxNumberOfVerticesPerLeafRunTimeCSA << " vertices per leaf" << endl;
  cout << "Best parameters for approximation run time: " << bestNumberOfChildrenPerNodeRunTimeApprox << " children per node, " << bestMaxNumberOfVerticesPerLeafRunTimeApprox << " vertices per leaf" << endl;
  cout << "Best parameters for accuracy min sum: " << bestNumberOfChildrenPerNodeAccuracyMinSum << " children per node, " << bestMaxNumberOfVerticesPerLeafAccuracyMinSum << " vertices per leaf" << endl;
  cout << "Best parameters for accuracy min max: " << bestNumberOfChildrenPerNodeAccuracyMinMax << " children per node, " << bestMaxNumberOfVerticesPerLeafAccuracyMinMax << " vertices per leaf" << endl;
}

void ExperimentController::testAndCompareAlgorithmsRandom(DataType dataType, int numberOfSuccessfulQueries) {
    vector<int> numberOfSourceStops = {2, 5, 10};

    GTree networkGTree = Creator::createNetworkGTree(2, 128);
    GTree* networkGTreePointer = &networkGTree;
    networkGTreePointer->initializeGTree();

    AlgorithmComparer::compareAlgorithmsRandom(dataType, networkGTreePointer, numberOfSuccessfulQueries, numberOfSourceStops, NUMBER_OF_DAYS, true, true);
}