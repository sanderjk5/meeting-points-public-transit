#ifndef CMAKE_QUERY_PROCESSOR_H
#define CMAKE_QUERY_PROCESSOR_H

#include <../data-handling/importer.h>
#include <../data-structures/g-tree.h>
#include "csa.h" 
#include "raptor.h" 
#include "journey.h"
#include "optimization.h"

#include <vector>
#include <map>
#include <string>
#include <memory>

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
    int minSumMeetingTimeInSeconds;
    int maxTransfersMinSum;
    int meetingPointMinMaxStopId;
    string meetingPointMinMax;
    string meetingTimeMinMax;
    string minMaxDuration;
    int minMaxDurationInSeconds;
    int minMaxMeetingTimeInSeconds;
    int maxTransfersMinMax;
    double queryTime;
};

struct MeetingPointQueryGTreeCSAInfo {
    double csaTargetStopFractionMinSum;
    double csaTargetStopFractionMinMax;
    double csaVisitedConnectionsFraction;
};

struct CandidateInfo {
    int stopId;
    int duration;
    int meetingTime;
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
        vector<shared_ptr<CSA>> csas;
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
        vector<shared_ptr<CSA>> csas;
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
        vector<shared_ptr<CSA>> csas;

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
        vector<Journey> getJourneys(Optimization optimization);
        vector<int> getStopsWithGivenAccuracy(double accuracyBound);

        double durationOfLastRound;
        int transfers;
        vector<shared_ptr<Raptor>> raptors;

        double numberOfExpandedRoutes;

        int multipleResultMinSum;
        int multipleResultMinMax;

    private:
        MeetingPointQuery meetingPointQuery;
        MeetingPointQueryResult meetingPointQueryResult;
        int lastRoundMinSumDuration;
        int lastRoundMinMaxDuration;
};

class RaptorBoundQueryProcessor {
    public:
        explicit RaptorBoundQueryProcessor(MeetingPointQuery meetingPointQuery){
            this->meetingPointQuery = meetingPointQuery;
        };
        ~RaptorBoundQueryProcessor(){};

        void processRaptorBoundQuery(Optimization optimization);
        vector<CandidateInfo> getStopsAndResultsWithSmallerRelativeDifference(double relativeDifference, Optimization optimization);
        MeetingPointQueryResult getMeetingPointQueryResult();
        vector<Journey> getJourneys(Optimization optimization);

        double numberOfExpandedRoutes;
        double durationPhast;
        double durationInitRaptorBounds;
        double durationRaptorBounds;
        double durationCreateResult;

        double lowerBoundSmallerCounter;
        double lowerBoundGreaterCounter;
        double lowerBoundSmallerFraction;
        double lowerBoundAbsDiff;
        double lowerBoundRelDiff;

    private:
        MeetingPointQuery meetingPointQuery;
        MeetingPointQueryResult meetingPointQueryResult;
        vector<shared_ptr<RaptorBound>> raptorBounds;
};

class RaptorPQQueryProcessor {
    public:
        explicit RaptorPQQueryProcessor(MeetingPointQuery meetingPointQuery){
            this->meetingPointQuery = meetingPointQuery;
        };
        ~RaptorPQQueryProcessor(){};

        void processRaptorPQQuery(Optimization optimization);
        MeetingPointQueryResult getMeetingPointQueryResult();
        vector<Journey> getJourneys(Optimization optimization);

        double numberOfExpandedRoutes;
        double durationPhast;
        double durationRaptorFirstResult;
        double durationInitRaptorPQs;
        double durationRaptorPQs;
        double durationCreateResult;

        double durationInitHeuristic;
        double durationTransformRaptorToRaptorPQ;
        double durationAddRoutesToQueue;
        double durationGetEarliestTripWithDayOffset;
        double durationTraverseRoute;

        int altHeuristicImprovementCounter;
        int noHeuristicImprovementCounter;
        double altHeuristicImprovementFraction;

        double lowerBoundSmallerCounter;
        double lowerBoundGreaterCounter;
        double lowerBoundSmallerFraction;
        double lowerBoundAbsDiff;
        double lowerBoundRelDiff;

    private:
        MeetingPointQuery meetingPointQuery;
        MeetingPointQueryResult meetingPointQueryResult;
        vector<shared_ptr<RaptorPQ>> raptorPQs;
};

class RaptorPQParallelQueryProcessor {
    public:
        explicit RaptorPQParallelQueryProcessor(MeetingPointQuery meetingPointQuery){
            this->meetingPointQuery = meetingPointQuery;
        };
        ~RaptorPQParallelQueryProcessor(){};

        void processRaptorPQParallelQuery(Optimization optimization);
        MeetingPointQueryResult getMeetingPointQueryResult();
        vector<Journey> getJourneys(Optimization optimization);

        double numberOfExpandedRoutes;
        double durationPhast;
        double durationRaptorFirstResult;
        double durationInitRaptorPQs;
        double durationRaptorPQs;
        double durationCreateResult;

        double durationInitHeuristic;
        double durationTransformRaptorToRaptorPQ;
        double durationAddRoutesToQueue;
        double durationGetEarliestTripWithDayOffset;
        double durationTraverseRoute;

        int altHeuristicImprovementCounter;
        int noHeuristicImprovementCounter;
        double altHeuristicImprovementFraction;

    private:
        MeetingPointQuery meetingPointQuery;
        MeetingPointQueryResult meetingPointQueryResult;
        shared_ptr<RaptorPQParallel> raptorPQParallel;
};

class RaptorApproximationQueryProcessor {
    public:
        explicit RaptorApproximationQueryProcessor(MeetingPointQuery meetingPointQuery){
            this->meetingPointQuery = meetingPointQuery;
        };
        ~RaptorApproximationQueryProcessor(){};

        void processRaptorApproximationQuery(Optimization optimization, bool multipleCandidates = false, bool useResultVerification = true, bool calculateExactResult = false);
        void processRaptorApproximationLoopQuery(int maxNumberOfSources);
        MeetingPointQueryResult getMeetingPointQueryResult();

        double durationExactSources;
        double durationExactCalculation;
        double durationCandidates;

        bool wrongResult;
        int numberOfRounds;
        int numberOfQueries;

    private:
        MeetingPointQuery meetingPointQuery;
        MeetingPointQueryResult meetingPointQueryResult;
        shared_ptr<RaptorBoundQueryProcessor> raptorBoundQueryProcessor;
        vector<shared_ptr<RaptorBound>> raptorBounds;
        vector<shared_ptr<Raptor>> raptors;
        vector<shared_ptr<RaptorPQStar>> raptorPQStars;

        map<int, vector<int>> sourceStopIdToAllStops;

        Optimization optimization;
        vector<int> exactSources;

        vector<pair<int, int>> sourceStopsWithErrorAndDuration;

        

        void calculateExactSources(int numberOfExactSourceStops);
        void calculateResultWithCandidates(bool useResultVerification);
        void calculateResultWithOneCandidate();
        bool verifyResult(int targetStopId, int meetingTime);
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
        static RaptorQuery parseRaptorQuery(string line);
        static CSAQuery createCSAQuery(string sourceStopName, string sourceTime, string weekday);
        static CSAQuery createCSAQueryWithTargetStops(string sourceStopName, vector<string> targetStopNames, string sourceTime, string weekday);
        static RaptorQuery generateRandomRaptorQuery();
        static RaptorQuery generateRaptorQuery(string sourceStopName, string targetStopName, string sourceTime, string weekday);
};



#endif //CMAKE_QUERY_PROCESSOR_H