#ifndef CMAKE_RAPTOR_H
#define CMAKE_RAPTOR_H

#include <../data-handling/importer.h>
#include "optimization.h"
#include <journey.h>
#include <vector>
#include <set>
#include <queue>
#include <limits.h>
#include <memory>

struct RaptorQuery {
    int sourceStopId;
    vector<int> targetStopIds;
    int sourceTime;
    int weekday;
};

struct RaptorBackwardQuery {
    int targetStopId;
    vector<int> sourceStopIds;
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

struct TripInfoBackward {
    int tripId;
    int dayOffset;
    int tripArrivalTime;
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
        return lowerBound > other.lowerBound;
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

class RaptorBackward {
    public:
        explicit RaptorBackward(RaptorBackwardQuery query){
            this->query = query;
            this->initializeRaptorBackward();
        };
        ~RaptorBackward(){};

        void setSourceStopIds(vector<int> sourceStopIds);
        void processRaptorBackward();
        void processRaptorBackwardRound();

        bool isFinished();

        vector<int>* getLatestDepartureTimes();
        int getLatestDepartureTime(int stopId);

        Journey createJourney(int targetStopId);

        vector<JourneyPointerRaptor> journeyPointers;
        vector<int> extendedTargetStopIds;
        vector<int> currentLatestDepartureTimes;
        vector<bool> currentMarkedStops;

        double numberOfExpandedRoutes;

    private:
        RaptorBackwardQuery query;
        int currentRound;
        vector<int> previousLatestDepartureTimes;
        vector<bool> previousMarkedStops;
        
        vector<int> maxStopSequencePerRoute;
        vector<pair<int, int>> q;
        bool isFinishedFlag;

        void initializeRaptorBackward();
        void fillQ();
        void traverseRoutes();
        TripInfoBackward getLatestTripWithDayOffset(int routeId, int stopId, int stopSequence);
};

class RaptorBound {
    public:
        explicit RaptorBound(RaptorQuery query, Optimization optimization){
            this->query = query;
            this->optimization = optimization;
            this->initializeRaptorBound();
        };
        ~RaptorBound(){};

        void processRaptorRound();
        void setCurrentBest(int currentBest);
        void initializeHeuristic(map<int, vector<int>> sourceStopIdsToAllStops, vector<int> sourceStopIds, int baseHeuristic, vector<int> closestLandmarkIndices);

        bool isFinished();

        vector<int>* getEarliestArrivalTimes();
        int getEarliestArrivalTime(int stopId);
        Journey createJourney(int targetStopId);

        vector<JourneyPointerRaptor> journeyPointers;
        vector<int> extendedSourceStopIds;
        vector<int> currentEarliestArrivalTimes;
        vector<bool> currentMarkedStops;

        double numberOfExpandedRoutes;
        double lowerBoundSmallerCounter;
        double lowerBoundGreaterCounter;
        double lowerBoundAbsDifference;
        double lowerBoundRelDifference;

    private:
        RaptorQuery query;
        int currentRound;
        vector<int> previousEarliestArrivalTimes;
        vector<bool> previousMarkedStops;
        
        vector<int> minStopSequencePerRoute;
        vector<pair<int, int>> q;
        bool isFinishedFlag;

        int currentBest;
        Optimization optimization;

        map<int, vector<int>> sourceStopIdsToAllStops;
        vector<int> sourceStopIds;
        int numberOfSourceStopIds;
        double baseHeuristic;
        vector<int> closestLandmarkIndices;

        vector<int> heuristicPerStopId;

        void initializeRaptorBound();
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
            this->altHeuristicImprovementCounter = 0;
            this->noHeuristicImprovementCounter = 0;
            this->lowerBoundSmallerCounter = 0;
            this->lowerBoundGreaterCounter = 0;
            this->lowerBoundAbsDifference = 0;
            this->lowerBoundRelDifference = 0;
        };
        ~RaptorPQ(){};

        void initializeRaptorPQ();
        void transformRaptorToRaptorPQ(shared_ptr<Raptor> raptor);
        void setCurrentBest(int currentBest);
        void initializeHeuristic(map<int, vector<int>> sourceStopIdsToAllStops, vector<int> sourceStopIds, int baseHeuristic, vector<int> closestLandmarkIndices);
        void processRaptorPQ();
        bool isFinished();

        vector<int>* getEarliestArrivalTimes();
        int getEarliestArrivalTime(int stopId);
        Journey createJourney(int targetStopId);

        double numberOfExpandedRoutes;
        double lowerBoundSmallerCounter;
        double lowerBoundGreaterCounter;
        double lowerBoundAbsDifference;
        double lowerBoundRelDifference;

        double durationInitHeuristic;
        double durationTransformRaptorToRaptorPQ;
        double durationAddRoutesToQueue;
        double durationGetEarliestTripWithDayOffset;
        double durationTraverseRoute;

        int altHeuristicImprovementCounter;
        int noHeuristicImprovementCounter;

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
        vector<int> closestLandmarkIndices;

        vector<int> heuristicPerStopId;
        
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
            this->altHeuristicImprovementCounter = 0;
            this->noHeuristicImprovementCounter = 0;
        };
        ~RaptorPQParallel(){};

        void transformRaptorsToRaptorPQs(vector<shared_ptr<Raptor>> raptors);
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

        int altHeuristicImprovementCounter;
        int noHeuristicImprovementCounter;

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
        set<int> getNewRoutes(set<int> stopIds, int excludeRouteId, int raptorIndex);
        void addRoutesToQueue(set<int> routes, int raptorIndex);
        TripInfo getEarliestTripWithDayOffset(int routeId, int stopId, int stopSequence, int previousEarliestArrivalTime);
        void updateCurrentBest(set<int> stopIds);
};

class RaptorBoundStar {
    public:
        explicit RaptorBoundStar(RaptorQuery query, vector<int> targetStopIdToAllStops, vector<int> landmarkIndices){
            this->query = query;
            this->targetStopId = query.targetStopIds[0];  
            this->targetStopIdToAllStops = targetStopIdToAllStops;
            this->landmarkIndices = landmarkIndices;
            this->initializeRaptorBound();
        };
        ~RaptorBoundStar(){};

        void processRaptorBoundStar();

        vector<int>* getEarliestArrivalTimes();
        int getEarliestArrivalTime(int stopId);
        Journey createJourney(int targetStopId);

        vector<JourneyPointerRaptor> journeyPointers;
        vector<int> extendedSourceStopIds;
        vector<int> currentEarliestArrivalTimes;
        vector<bool> currentMarkedStops;

        double numberOfExpandedRoutes;
        double lowerBoundSmallerCounter;
        double lowerBoundGreaterCounter;
        double lowerBoundAbsDifference;
        double lowerBoundRelDifference;

    private:
        RaptorQuery query;
        int targetStopId;
        int currentRound;
        vector<int> previousEarliestArrivalTimes;
        vector<bool> previousMarkedStops;
        
        vector<int> minStopSequencePerRoute;
        vector<pair<int, int>> q;
        bool isFinishedFlag;

        vector<int> targetStopIdToAllStops;

        vector<int> heuristicPerStopId;

        vector<int> landmarkIndices;

        void initializeRaptorBound();
        void fillQ();
        void traverseRoutes();
        TripInfo getEarliestTripWithDayOffset(int routeId, int stopId, int stopSequence);
};

class RaptorPQStar {
    public:
        explicit RaptorPQStar(RaptorQuery query, vector<int> targetStopIdToAllStops, vector<int> landmarkIndices){
            this->query = query;
            this->targetStopId = query.targetStopIds[0];
            this->targetStopIdToAllStops = targetStopIdToAllStops;
            this->initializeRaptorPQStar();
        };
        ~RaptorPQStar(){};

        void initializeRaptorPQStar();
        void processRaptorPQStar();

        vector<int>* getEarliestArrivalTimes();
        int getEarliestArrivalTime(int stopId);
        Journey createJourney(int targetStopId);

        double numberOfExpandedRoutes;
        double lowerBoundSmallerCounter;
        double lowerBoundGreaterCounter;
        double lowerBoundAbsDifference;
        double lowerBoundRelDifference;

    private:
        RaptorQuery query;
        int targetStopId;

        priority_queue<pair<double, int>, vector<pair<double, int>>, greater<pair<double, int>>> pq;
        vector<int> firstStopSequencePerRoute;
        vector<double> lowestLowerBoundPerRoute;

        vector<int> earliestArrivalTimes;

        bool isFinishedFlag;

        vector<JourneyPointerRaptor> journeyPointers;
        vector<int> extendedSourceStopIds;

        vector<int> targetStopIdToAllStops;

        vector<int> landmarkIndices;
        
        void traverseRoute();
        void addRoutesToQueue(set<int> stopIds, int excludeRouteId);
        TripInfo getEarliestTripWithDayOffset(int routeId, int stopId, int stopSequence, int previousEarliestArrivalTime);
};

#endif //CMAKE_RAPTOR_H