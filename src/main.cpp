#include<iostream>
#include"data-handling/importer.h"
#include<string>
#include"meeting-point-algorithms/algorithm-tester.h"
#include"meeting-point-algorithms/query-processor.h"
#include"data-structures/g-tree.h"
#include"data-structures/creator.h"
#include"data-handling/converter.h"
#include"data-structures/graph.h"


using namespace std;

static const char *const HEADER = "\nMeeting points - Public Transit\n \n";

int main(int argc, const char *argv[]) {
  srand(time(0));
  cout << HEADER;

  // string folderName = "gtfs_s_bahn_stuttgart";
  // Importer::import(folderName, true, s_bahn_stuttgart);

  string folderName = "vvs_gtfs_j24";
  Importer::import(folderName, true, vvs_j24);
  vector<string> sourceStopNames = {"Rohr", "Waldau"};
  // vector<string> sourceStopNames = {"Rohr", "Waldau", "Göppingen Bahnhofssteg"};

  // string folderName0 = "schienenfernverkehr_de";
  // string folderName1 = "schienenregionalverkehr_de";
  // Importer::import(folderName0, false, schienenfernverkehr_de);
  // Importer::import(folderName1, true, schienenregionalverkehr_de);
  // vector<string> sourceStopNames = {"Falkensee", "Stuttgart-Rohr"};
  // vector<string> sourceStopNames = {"Hamburg Hbf", "Stuttgart-Rohr"};

  Creator::createNetworkGraph();
  GTree networkGTree = Creator::createNetworkGTree(4, 128);

  GTree* networkGTreePointer = &networkGTree;
  networkGTreePointer->initializeGTree();

  // NaiveAlgorithmTester::testNaiveAlgorithmRandom(10, 10, 3, false, true);
  // GTreeAlgorithmTester::testGTreeAlgorithmRandom(networkGTreePointer, 20, 3, 3, false, true);

  MeetingPointQuery meetingPointQuery = QueryProcessor::generateMeetingPointQuery(sourceStopNames, "09:00:00", "monday", 7);

  // NaiveAlgorithmTester::testNaiveAlgorithm(meetingPointQuery, false, false);
  // GTreeAlgorithmTester::testGTreeAlgorithm(networkGTreePointer, meetingPointQuery, true, false);

  // AlgorithmComparer::compareAlgorithms(networkGTreePointer, meetingPointQuery, false);

  AlgorithmComparer::compareAlgorithmsRandom(networkGTreePointer, 20, 5, 7, false, false);

  return 0;
}