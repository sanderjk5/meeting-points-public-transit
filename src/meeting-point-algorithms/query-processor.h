#ifndef CMAKE_QUERY_PROCESSOR_H
#define CMAKE_QUERY_PROCESSOR_H

#include <../data-handling/importer.h>
#include <../data-structures/g-tree.h>
#include "csa.h" 
#include "journey.h"

#include <vector>
#include <map>
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

class GTreeQueryProcessor {
    public:
        explicit GTreeQueryProcessor(MeetingPointQuery meetingPointQuery, GTree* gTree){
            this->meetingPointQuery = meetingPointQuery;
            this->gTree = gTree;
        };
        ~GTreeQueryProcessor(){};

        void processGTreeQuery(bool printTime = false);
        MeetingPointQueryResult getMeetingPointQueryResult();
    
    private:
        GTree* gTree;
        MeetingPointQuery meetingPointQuery;
        MeetingPointQueryResult meetingPointQueryResult;
        map<pair<int, int>, vector<pair<int, int>>> queryPointAndNodeToBorderStopDurations;

        void processGTreeQueryWithOptimization(Optimization optimization);
        int getLowerBoundToNode(int nodeId, map<pair<int, int>, vector<pair<int, int>>> &queryPointAndNodeToBorderStopDurations, Optimization optimization);
        int getCostsToStop(int stopId, map<pair<int, int>, vector<pair<int, int>>> &queryPointAndNodeToBorderStopDurations, Optimization optimization);
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