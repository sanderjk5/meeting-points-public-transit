#ifndef CMAKE_QUERY_PROCESSOR_H
#define CMAKE_QUERY_PROCESSOR_H

#include <../data-handling/importer.h>
#include "csa.h" 
#include "journey.h"

#include <vector>
#include <string>


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

enum Optimization {
    min_sum,
    min_max
};

class NaiveQueryProcessor {
    public:
        explicit NaiveQueryProcessor(MeetingPointQuery meetingPointQuery){
            this->meetingPointQuery = meetingPointQuery;
        };
        ~NaiveQueryProcessor(){};

        void processNaiveQuery(bool printTime = false);
        MeetingPointQueryResult getMeetingPointQueryResult();
        vector<Journey> getJourneys(Optimization optimization);
    
    private:
        MeetingPointQuery meetingPointQuery;
        MeetingPointQueryResult meetingPointQueryResult;
        vector<CSA*> csas;
};

class QueryProcessor {
    public:
        explicit QueryProcessor(){};
        ~QueryProcessor(){};

        static MeetingPointQuery generateRandomMeetingPointQuery(int numberOfSources, int numberOfDays = 1);
        static MeetingPointQuery generateMeetingPointQuery(vector<string> sourceStopNames, string sourceTime, string weekday, int numberOfDays = 1);
        static CSAQuery createCSAQuery(string sourceStopName, string sourceTime, string weekday);
        static CSAQuery createCSAQueryWithTargetStops(string sourceStopName, vector<string> targetStopNames, string sourceTime, string weekday);
};



#endif //CMAKE_QUERY_PROCESSOR_H