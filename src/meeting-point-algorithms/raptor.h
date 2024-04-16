#ifndef CMAKE_RAPTOR_H
#define CMAKE_RAPTOR_H

#include <../data-handling/importer.h>
#include "optimization.h"
#include <journey.h>
#include <vector>
#include <set>
#include <queue>
#include <limits.h>

struct RaptorQuery {
    int sourceStopId;
    vector<int> targetStopIds;
    int sourceTime;
    int weekday;
};

struct JourneyPointerRaptor {
    int enterTripAtStop;
    int leaveTripAtStop;
    int departureTime;
    int arrivalTime;
    int tripId;
};

struct TripInfo {
    int tripId;
    int dayOffset;
    int tripDepartureTime;
};

struct PQEntry {
    double lowerBound;
    int routeId;
    int raptorIndex;

    PQEntry(double lowerBound, int routeId, int raptorIndex){
        this->lowerBound = lowerBound;
        this->routeId = routeId;
        this->raptorIndex = raptorIndex;
    }

    bool operator<(const PQEntry& other) const {
        return lowerBound < other.lowerBound;
    }
};

class Raptor {
    public:
        explicit Raptor(RaptorQuery query){
            this->query = query;
            this->initializeRaptor();
        };
        ~Raptor(){};

        void setTargetStopIds(vector<int> targetStopIds);
        void processRaptor();
        void processRaptorRound();

        bool isFinished();

        vector<int>* getEarliestArrivalTimes();
        int getEarliestArrivalTime(int stopId);

        Journey createJourney(int targetStopId);

        vector<JourneyPointerRaptor> journeyPointers;
        vector<int> extendedSourceStopIds;
        vector<int> currentEarliestArrivalTimes;
        vector<bool> currentMarkedStops;

        double numberOfExpandedRoutes;

    private:
        RaptorQuery query;
        int currentRound;
        vector<int> previousEarliestArrivalTimes;
        vector<bool> previousMarkedStops;
        
        vector<int> minStopSequencePerRoute;
        vector<pair<int, int>> q;
        bool isFinishedFlag;

        void initializeRaptor();
        void fillQ();
        void traverseRoutes();
        TripInfo getEarliestTripWithDayOffset(int routeId, int stopId, int stopSequence);
};

class RaptorPQ {
    public:
        explicit RaptorPQ(RaptorQuery query, Optimization optimization){
            this->query = query;
            this->optimization = optimization;
            this->currentBest = INT_MAX;
            this->durationAddRoutesToQueue = 0;
            this->durationGetEarliestTripWithDayOffset = 0;
            this->durationTraverseRoute = 0;
        };
        ~RaptorPQ(){};

        void initializeRaptorPQ();
        void transformRaptorToRaptorPQ(Raptor* raptor);
        void setCurrentBest(int currentBest);
        void initializeHeuristic(map<int, vector<int>> sourceStopIdsToAllStops, vector<int> sourceStopIds);
        void processRaptorPQ();
        bool isFinished();

        vector<int>* getEarliestArrivalTimes();
        int getEarliestArrivalTime(int stopId);
        Journey createJourney(int targetStopId);

        double numberOfExpandedRoutes;

        double durationInitHeuristic;
        double durationTransformRaptorToRaptorPQ;
        double durationAddRoutesToQueue;
        double durationGetEarliestTripWithDayOffset;
        double durationTraverseRoute;

    private:
        RaptorQuery query;

        priority_queue<pair<double, int>, vector<pair<double, int>>, greater<pair<double, int>>> pq;
        vector<int> firstStopSequencePerRoute;
        vector<double> lowestLowerBoundPerRoute;
        vector<set<int>> markedStopsPerRoute;

        int currentBest;
        Optimization optimization;

        vector<int> earliestArrivalTimes;

        bool isFinishedFlag;

        vector<JourneyPointerRaptor> journeyPointers;
        vector<int> extendedSourceStopIds;

        map<int, vector<int>> sourceStopIdsToAllStops;
        vector<int> sourceStopIds;
        int numberOfSourceStopIds;
        double baseHeuristic;
        
        void traverseRoute();
        void addRoutesToQueue(set<int> stopIds, int excludeRouteId);
        TripInfo getEarliestTripWithDayOffset(int routeId, int stopId, int stopSequence, int previousEarliestArrivalTime);
};

class RaptorPQParallel {
    public:
        explicit RaptorPQParallel(vector<RaptorQuery> queries, Optimization optimization){
            this->queries = queries;
            this->optimization = optimization;
            this->currentBest = INT_MAX;
            this->isFinishedFlag = false;
            this->numberOfExpandedRoutes = 0;
            this->durationAddRoutesToQueue = 0;
            this->durationGetEarliestTripWithDayOffset = 0;
            this->durationTraverseRoute = 0;
        };
        ~RaptorPQParallel(){};

        void transformRaptorsToRaptorPQs(vector<Raptor*> raptors);
        void setCurrentBest(int currentBest);
        void initializeHeuristics(map<int, vector<int>> sourceStopIdsToAllStops, vector<int> sourceStopIds);
        void processRaptorPQ();

        vector<int> getEarliestArrivalTimes(int stopId);
        vector<Journey> createJourneys(int targetStopId);

        double numberOfExpandedRoutes;

        double durationInitHeuristic;
        double durationTransformRaptorToRaptorPQ;
        double durationAddRoutesToQueue;
        double durationGetEarliestTripWithDayOffset;
        double durationTraverseRoute;

    private:
        vector<RaptorQuery> queries;

        priority_queue<PQEntry> pq;
        vector<vector<int>> firstStopSequencePerRoute;
        vector<vector<double>> lowestLowerBoundPerRoute;

        int currentBest;
        Optimization optimization;

        bool isFinishedFlag;

        vector<vector<int>> earliestArrivalTimes;

        vector<vector<JourneyPointerRaptor>> journeyPointers;
        vector<vector<int>> extendedSourceStopIds;

        map<int, vector<int>> sourceStopIdsToAllStops;
        vector<int> sourceStopIds;
        int numberOfSourceStopIds;
        vector<double> baseHeuristics;
        
        void traverseRoute();
        void addRoutesToQueue(set<int> stopIds, int excludeRouteId, int raptorIndex);
        TripInfo getEarliestTripWithDayOffset(int routeId, int stopId, int stopSequence, int previousEarliestArrivalTime);
        void updateCurrentBest(set<int> stopIds);
};

#endif //CMAKE_RAPTOR_H