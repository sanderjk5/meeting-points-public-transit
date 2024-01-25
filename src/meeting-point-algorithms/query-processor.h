#ifndef CMAKE_QUERY_PROCESSOR_H
#define CMAKE_QUERY_PROCESSOR_H

#include <../data-handling/importer.h>

#include <vector>
#include <string>

struct CSAQuery {
    int sourceStopId;
    vector<int> targetStopIds;
    int sourceTime;
    int weekday;
    int numberOfDays;
};

struct MeetingPointQuery {
    vector<int> sourceStopIds;
    int sourceTime;
    int weekday;
    int numberOfDays;
};

struct MeetingPointQueryResult {
    string meetingPointMinSum;
    string meetingTimeMinSum;
    string minSumDuration;
    string meetingPointMinMax;
    string meetingTimeMinMax;
    string minMaxDuration;
    string queryTime;
};

struct Leg {
    string departureStopName;
    string arrivalStopName;
    int departureTime;
    int arrivalTime;
};

struct Journey {
    vector<Leg> legs;
    int duration;
};

class QueryProcessor {
    public:
        explicit QueryProcessor(){};
        ~QueryProcessor(){};

        static MeetingPointQueryResult processNaiveQuery(MeetingPointQuery meetingPointQuery, bool printTime = false);
        static MeetingPointQuery generateRandomMeetingPointQuery(int numberOfSources, int numberOfDays = 1);
        static MeetingPointQuery generateMeetingPointQuery(vector<string> sourceStopNames, string sourceTime, string weekday, int numberOfDays = 1);
    
    private:
        static CSAQuery createCSAQuery(string sourceStopName, string sourceTime, string weekday);
        static CSAQuery createCSAQueryWithTargetStops(string sourceStopName, vector<string> targetStopNames, string sourceTime, string weekday);
};

#endif //CMAKE_QUERY_PROCESSOR_H