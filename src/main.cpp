#include<iostream>
#include<importer.h>
#include<string>
#include"meeting-point-algorithms/algorithm-tester.h"
#include"data-handling/converter.h"


using namespace std;

static const char *const HEADER = "\nMeeting points - Public Transit\n\n";
static const char *const USAGE = "\nExecute meeting-points-public-transit\n\n";

int main(int argc, const char *argv[]) {
  cout << HEADER;
  cout << USAGE;

  // string folderName = "gtfs_s_bahn_stuttgart";
  string folderName = "vvs_gtfs_j24";

  Importer::import(folderName, true, false);

  NaiveAlgorithmTester::testNaiveAlgorithm(10, 3, false);

  return 0;
}