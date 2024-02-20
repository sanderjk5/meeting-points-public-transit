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

#include "experiment-controller.h"


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

  vector<string> sourceStopNames;

  if (dataType == vvs_j24){
    string folderName = "vvs_gtfs_j24";
    Importer::import(folderName, true, vvs_j24);
    sourceStopNames = {"Rohr", "Waldau"};
  } else if (dataType == schienenfernverkehr_de){
    string folderName = "schienenfernverkehr_de";
    Importer::import(folderName, true, schienenfernverkehr_de);
    sourceStopNames = {"Falkensee", "Stuttgart-Rohr"};
  } else if (dataType == schienenregionalverkehr_de){
    string folderName = "schienenregionalverkehr_de";
    Importer::import(folderName, true, schienenregionalverkehr_de);
    sourceStopNames = {"Falkensee", "Stuttgart-Rohr"};
  } else if (dataType == schienenfern_und_regionalverkehr_de){
    string folderName0 = "schienenfernverkehr_de";
    string folderName1 = "schienenregionalverkehr_de";
    Importer::import(folderName0, false, schienenfernverkehr_de);
    Importer::import(folderName1, true, schienenregionalverkehr_de);
    sourceStopNames = {"Falkensee", "Stuttgart-Rohr"};
  } else if (dataType == gesamt_de){
    string folderName = "gesamt_de";
    Importer::import(folderName, true, gesamt_de);
    sourceStopNames = {"Falkensee", "Stuttgart-Rohr"};
  }

  Creator::createNetworkGraph();
    
  // GTree networkGTree = Creator::createNetworkGTree(2, 16);

  // GTree* networkGTreePointer = &networkGTree;
  // networkGTreePointer->initializeGTree();

  // MeetingPointQuery meetingPointQuery = QueryGenerator::generateMeetingPointQuery(sourceStopNames, "09:00:00", "monday", NUMBER_OF_DAYS);

  // AlgorithmComparer::compareAlgorithms(dataType, networkGTreePointer, meetingPointQuery);

  vector<int> numberOfSourceStops = {2, 5, 10};

  // NaiveKeyStopQueryProcessor::findKeyStops(dataType, numberOfSourceStops, 10, 10, 0.90);

  // ExperimentController::findBestGTreeParameters(dataType, 2, 10);
  ExperimentController::testAndCompareAlgorithmsRandom(dataType, 10, numberOfSourceStops);

  // vector<int> numberOfSourceStops = {2, 3, 4, 5, 6, 7, 8, 9, 10, 15, 20, 25, 30, 40, 50, 75, 100, 150, 200};

  // NaiveKeyStopQueryProcessor::findKeyStops(dataType, numberOfSourceStops, 1000, 50, 0.90);

  // ExperimentController::findBestGTreeParameters(dataType, 10, 1000);
  // ExperimentController::testAndCompareAlgorithmsRandom(dataType, 1000, numberOfSourceStops);
  
  return 0;
}

