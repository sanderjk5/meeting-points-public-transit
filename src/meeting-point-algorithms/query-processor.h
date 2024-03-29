#ifndef CMAKE_QUERY_PROCESSOR_H
#define CMAKE_QUERY_PROCESSOR_H

#include <../data-handling/importer.h>
#include <../data-structures/g-tree.h>
#include "csa.h" 
#include "raptor.h" 
#include "journey.h"

#include <vector>
#include <map>
#include <string>

struct MeetingPointQuery {
    vector<int> sourceStopIds;
    int sourceTime;
    int weekday;
};

struct MeetingPointQueryResult {
    int meetingPointMinSumStopId;
    string meetingPointMinSum;
    string meetingTimeMinSum;
    string minSumDuration;
    int minSumDurationInSeconds;
    int maxTransfersMinSum;
    int meetingPointMinMaxStopId;
    string meetingPointMinMax;
    string meetingTimeMinMax;
    string minMaxDuration;
    int minMaxDurationInSeconds;
    int maxTransfersMinMax;
    double queryTime;
};

struct MeetingPointQueryGTreeCSAInfo {
    double csaTargetStopFractionMinSum;
    double csaTargetStopFractionMinMax;
    double csaVisitedConnectionsFraction;
};

enum Optimization {
    min_sum,
    min_max,
    both
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
        MeetingPointQueryGTreeCSAInfo getMeetingPointQueryGTreeCSAInfo();
        vector<Journey> getJourneys(Optimization optimization);
        double visitedNodesAvgFraction;
        
    private:
        GTree* gTree;
        MeetingPointQuery meetingPointQuery;
        MeetingPointQueryResult meetingPointQueryResult;
        MeetingPointQueryGTreeCSAInfo meetingPointQueryGTreeCSAInfo;
        vector<map<int, vector<pair<int, int>>>> queryPointAndNodeToBorderStopDurations;
        vector<CSA*> csas;

        void processGTreeQueryWithOptimization(Optimization optimization, bool useCSA);
        int getLowerBoundToNode(int nodeId, Optimization optimization);
        int getApproximatedCostsToStop(int stopId, Optimization optimization);
        int getCostsToStop(int stopId, Optimization optimization);
        void processCSAToTargetStops(vector<int> targetStopIds, int currentBest);
};

class RaptorQueryProcessor {
    public:
        explicit RaptorQueryProcessor(MeetingPointQuery meetingPointQuery){
            this->meetingPointQuery = meetingPointQuery;
        };
        ~RaptorQueryProcessor(){};

        void initializeRaptors();
        void processRaptorQueryUntilFirstResult();
        void processRaptorQueryUntilResultDoesntImprove(Optimization optimization);
        void processRaptorQuery();
        bool processRaptorRound();
        MeetingPointQueryResult getMeetingPointQueryResult();
        double durationOfLastRound;
        int transfers;

    private:
        MeetingPointQuery meetingPointQuery;
        MeetingPointQueryResult meetingPointQueryResult;
        vector<Raptor*> raptors;
        int lastRoundMinSumDuration;
        int lastRoundMinMaxDuration;
};

/*
    A class that generates queries for the meeting point algorithms.
*/
class QueryGenerator {
    public:
        explicit QueryGenerator(){};
        ~QueryGenerator(){};

        static MeetingPointQuery generateRandomMeetingPointQuery(int numberOfSources);
        static MeetingPointQuery generateMeetingPointQuery(vector<string> sourceStopNames, string sourceTime, string weekday);
        static MeetingPointQuery parseMeetingPointQuery(string line, int numberOfSourceStops);
        static CSAQuery createCSAQuery(string sourceStopName, string sourceTime, string weekday);
        static CSAQuery createCSAQueryWithTargetStops(string sourceStopName, vector<string> targetStopNames, string sourceTime, string weekday);
};



#endif //CMAKE_QUERY_PROCESSOR_H