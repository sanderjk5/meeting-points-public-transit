#include<iostream>
#include<importer.h>
#include<string>
#include"meeting-point-algorithms/query-processor.h"
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

  for (int i = 0; i < 10; i++) {
    MeetingPointQuery meetingPointQuery = QueryProcessor::generateRandomMeetingPointQuery(5);
    cout << "Source stops: ";
    for (int j = 0; j < meetingPointQuery.sourceStopIds.size()-1; j++) {
      cout << Importer::getStopName(meetingPointQuery.sourceStopIds[j]) << ", ";
    }
    cout << Importer::getStopName(meetingPointQuery.sourceStopIds[meetingPointQuery.sourceStopIds.size()-1]);
    cout << endl;
    cout << "Source time: " << TimeConverter::convertSecondsToTime(meetingPointQuery.sourceTime, true) << endl;
    cout << "Weekday: " << WeekdayConverter::convertIntToWeekday(meetingPointQuery.weekday) << endl;

    MeetingPointQueryResult meetingPointQueryResult = QueryProcessor::processNaiveQuery(meetingPointQuery, false);
    if (meetingPointQueryResult.meetingPointMinSum == "" || meetingPointQueryResult.meetingPointMinMax == "") {
      cout << "No meeting point found" << endl;
      cout << "Query time: " << meetingPointQueryResult.queryTime << " milliseconds \n" << endl;
      continue;
    }
    cout << "Min sum - meeting point: " << meetingPointQueryResult.meetingPointMinSum << ", meeting time: " << meetingPointQueryResult.meetingTimeMinSum << ",  travel time sum: " << meetingPointQueryResult.minSumDuration << endl;
    cout << "Min max - meeting point: " << meetingPointQueryResult.meetingPointMinMax << ", meeting time: " << meetingPointQueryResult.meetingTimeMinMax << ", travel time max: " << meetingPointQueryResult.minMaxDuration << endl;
    cout << "Query time: " << meetingPointQueryResult.queryTime << " milliseconds \n" << endl;
  }

  return 0;
}