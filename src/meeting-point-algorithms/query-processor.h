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
    int minSumDurationInSeconds;
    string meetingPointMinMax;
    string meetingTimeMinMax;
    string minMaxDuration;
    int minMaxDurationInSeconds;
    double queryTime;
};

enum Optimization {
    min_sum,
    min_max
};

/*
    A class that processes the naive algorithm.
*/
class NaiveQueryProcessor {
    public:
        explicit NaiveQueryProcessor(MeetingPointQuery meetingPointQuery){
            this->meetingPointQuery = meetingPointQuery;
        };
        ~NaiveQueryProcessor(){};

        void processNaiveQuery();
        MeetingPointQueryResult getMeetingPointQueryResult();
        vector<Journey> getJourneys(Optimization optimization);
        vector<int> getStopsWithGivenAccuracy(double accuracyBound);
    
    private:
        MeetingPointQuery meetingPointQuery;
        MeetingPointQueryResult meetingPointQueryResult;
        vector<CSA*> csas;
};

class NaiveKeyStopQueryProcessor {
    public:
        explicit NaiveKeyStopQueryProcessor(MeetingPointQuery meetingPointQuery){
            this->meetingPointQuery = meetingPointQuery;
        };
        ~NaiveKeyStopQueryProcessor(){};

        static void findKeyStops(DataType dataType, vector<int> numberOfSourceStopsVec, int numberOfQueries, int numberOfKeyStops, double accuracyBound);
        static vector<int> getKeyStops(DataType dataType, int numberOfSourceStops);
        void processNaiveKeyStopQuery(vector<int> keyStops);
        MeetingPointQueryResult getMeetingPointQueryResult();
        vector<Journey> getJourneys(Optimization optimization);

    private:
        MeetingPointQuery meetingPointQuery;
        MeetingPointQueryResult meetingPointQueryResult;
        vector<CSA*> csas;
};

/*
    A class that processes the G-Tree algorithm.
*/
class GTreeQueryProcessor {
    public:
        explicit GTreeQueryProcessor(MeetingPointQuery meetingPointQuery, GTree* gTree){
            this->meetingPointQuery = meetingPointQuery;
            this->gTree = gTree;
        };
        ~GTreeQueryProcessor(){};

        void processGTreeQuery(bool useCSA = false);
        MeetingPointQueryResult getMeetingPointQueryResult();
        vector<Journey> getJourneys(Optimization optimization);
    
    private:
        GTree* gTree;
        MeetingPointQuery meetingPointQuery;
        MeetingPointQueryResult meetingPointQueryResult;
        map<pair<int, int>, vector<pair<int, int>>> queryPointAndNodeToBorderStopDurations;
        vector<CSA*> csas;

        void processGTreeQueryWithOptimization(Optimization optimization, bool useCSA);
        int getLowerBoundToNode(int nodeId, map<pair<int, int>, vector<pair<int, int>>> &queryPointAndNodeToBorderStopDurations, Optimization optimization);
        int getCostsToStop(int stopId, map<pair<int, int>, vector<pair<int, int>>> &queryPointAndNodeToBorderStopDurations, Optimization optimization, bool useCSA);
        void processCSAToTargetStops(vector<int> targetStopIds, int currentBest);
};

/*
    A class that generates queries for the meeting point algorithms.
*/
class QueryGenerator {
    public:
        explicit QueryGenerator(){};
        ~QueryGenerator(){};

        static MeetingPointQuery generateRandomMeetingPointQuery(int numberOfSources, int numberOfDays = 1);
        static MeetingPointQuery generateMeetingPointQuery(vector<string> sourceStopNames, string sourceTime, string weekday, int numberOfDays = 1);
        static MeetingPointQuery parseMeetingPointQuery(string line, int numberOfSourceStops);
        static CSAQuery createCSAQuery(string sourceStopName, string sourceTime, string weekday);
        static CSAQuery createCSAQueryWithTargetStops(string sourceStopName, vector<string> targetStopNames, string sourceTime, string weekday);
};



#endif //CMAKE_QUERY_PROCESSOR_H