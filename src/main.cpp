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
#include <cstring>

#include "experiment-controller.h"
#include "cli-controller.h"
#include "gtree-controller.h"


using namespace std;

static const char *const HEADER = "\nMeeting points - Public Transit\n \n";



int main(int argc, const char *argv[]) {
  srand(time(0));
  cout << HEADER;

  DataType dataType = vvs_j24;
  // DataType dataType = schienenfernverkehr_de;
  // DataType dataType = schienenregionalverkehr_de;
  // DataType dataType = schienenfern_und_regionalverkehr_de;
  // DataType dataType = gesamt_de;

  bool startExperiments = false;

  if (argc > 1){
    if (strcmp(argv[1], "vvs") == 0){
      dataType = vvs_j24;
    } else if (strcmp(argv[1], "fern") == 0){
      dataType = schienenfernverkehr_de;
    } else if (strcmp(argv[1], "regio") == 0){
      dataType = schienenregionalverkehr_de;
    } else if (strcmp(argv[1], "fern_regio") == 0){
      dataType = schienenfern_und_regionalverkehr_de;
    } else if (strcmp(argv[1], "de") == 0){
      dataType = gesamt_de;
    } else {
      cout << "Invalid data type. Using default data type vvs." << endl;
    }
  } else {
    cout << "No data type specified. Using default data type vvs." << endl;
  }

  if (argc > 2){
    if (strcmp(argv[2], "exp") == 0){
      startExperiments = true;
    }
  }

  vector<string> sourceStopNames;

  if (dataType == vvs_j24){
    string folderName = "vvs_j24";
    Importer::import(folderName, true, vvs_j24);
  } else if (dataType == schienenfernverkehr_de){
    string folderName = "schienenfernverkehr_de";
    Importer::import(folderName, true, schienenfernverkehr_de);
  } else if (dataType == schienenregionalverkehr_de){
    string folderName = "schienenregionalverkehr_de";
    Importer::import(folderName, true, schienenregionalverkehr_de);
  } else if (dataType == schienenfern_und_regionalverkehr_de){
    string folderName0 = "schienenfernverkehr_de";
    string folderName1 = "schienenregionalverkehr_de";
    Importer::import(folderName0, false, schienenfernverkehr_de);
    Importer::import(folderName1, true, schienenfern_und_regionalverkehr_de);
  } else if (dataType == gesamt_de){
    string folderName = "gesamt_de";
    Importer::import(folderName, true, gesamt_de);
  }

  Creator::createNetworkGraph();

  if (startExperiments){
    // Real experiments
    vector<int> numberOfSourceStops = {50};

    // NaiveKeyStopQueryProcessor::findKeyStops(dataType, numberOfSourceStops, 1000, 25, 0.90);

    // ExperimentController::findBestGTreeParameters(dataType, 10, 100);
    ExperimentController::testAndCompareAlgorithmsRandom(dataType, 1000, numberOfSourceStops);

    // Test experiments
    // vector<int> numberOfSourceStops = {2};

    // NaiveKeyStopQueryProcessor::findKeyStops(dataType, numberOfSourceStops, 10, 30, 0.90);

    // ExperimentController::findBestGTreeParameters(dataType, 2, 10);
    // ExperimentController::testAndCompareAlgorithmsRandom(dataType, 10, numberOfSourceStops);
  } else {
    
    int numberOfChildrenPerNode;
    int maxNumberOfVerticesPerLeaf;
    if(dataType == vvs_j24){
      numberOfChildrenPerNode = 4;
      maxNumberOfVerticesPerLeaf = 64;
    } else if (dataType == schienenfernverkehr_de) {
      numberOfChildrenPerNode = 2;
      maxNumberOfVerticesPerLeaf = 32;
    } else if (dataType == schienenregionalverkehr_de || dataType == schienenfern_und_regionalverkehr_de) {
      numberOfChildrenPerNode = 4;
      maxNumberOfVerticesPerLeaf = 128;
    } else if (dataType == gesamt_de) {
      numberOfChildrenPerNode = 8;
      maxNumberOfVerticesPerLeaf = 256;
    }

    GTree* networkGTreePointer = GTreeController::createOrLoadNetworkGTree(dataType, numberOfChildrenPerNode, maxNumberOfVerticesPerLeaf);

    CliController::runCli(dataType, networkGTreePointer);
  }
  
  return 0;
}

