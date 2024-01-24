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
};

struct MeetingPointQuery {
    vector<int> sourceStopIds;
    int sourceTime;
    int weekday;
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

        string processNaiveQuery(MeetingPointQuery meetingPointQuery);
    
    private:
        static CSAQuery createCSAQuery(string sourceStopName, string sourceTime, string weekday);
        static CSAQuery createCSAQueryWithTargetStop(string sourceStopName, string targetStopName, string sourceTime, string weekday);
};

#endif //CMAKE_QUERY_PROCESSOR_H