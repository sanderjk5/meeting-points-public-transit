#include<iostream>
#include<importer.h>
#include<string>
#include"meeting-point-algorithms/algorithm-tester.h"
#include"meeting-point-algorithms/query-processor.h"
#include"data-structures/creator.h"
#include"data-handling/converter.h"


using namespace std;

static const char *const HEADER = "\nMeeting points - Public Transit\n";

int main(int argc, const char *argv[]) {
  srand(time(0));
  cout << HEADER;

  // string folderName = "gtfs_s_bahn_stuttgart";
  // Importer::import(folderName, true, s_bahn_stuttgart);

  // string folderName = "vvs_gtfs_j24";
  // Importer::import(folderName, true, vvs_j24);

  string folderName0 = "schienenfernverkehr_de";
  string folderName1 = "schienenregionalverkehr_de";
  Importer::import(folderName0, false, schienenfernverkehr_de);
  Importer::import(folderName1, true, schienenregionalverkehr_de);

  Creator::createNetworkGraph();
  Creator::createNetworkGTree(4, 512);

  cout << "Border stops of root node: " << Creator::networkGTree.root->borderStopIds.size() << endl;

  // NaiveAlgorithmTester::testNaiveAlgorithmRandom(10, 10, 3, false, true);

  // vector<string> sourceStopNames = {"Falkensee", "Stuttgart-Rohr"};
  // MeetingPointQuery meetingPointQuery = QueryProcessor::generateMeetingPointQuery(sourceStopNames, "09:00:00", "monday", 1);
  // NaiveAlgorithmTester::testNaiveAlgorithm(meetingPointQuery, false, false);

  return 0;
}