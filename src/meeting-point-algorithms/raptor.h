#ifndef CMAKE_RAPTOR_H
#define CMAKE_RAPTOR_H

#include <../data-handling/importer.h>
#include "optimization.h"
#include <journey.h>
#include <vector>
#include <queue>

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

    private:
        RaptorQuery query;

        priority_queue<pair<double, int>, vector<pair<double, int>>, greater<pair<double, int>>> pq;
        vector<int> firstStopSequencePerRoute;
        vector<double> lowestLowerBoundPerRoute;

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
        void addRoutesToQueue(vector<int> stopIds, int excludeRouteId);
        TripInfo getEarliestTripWithDayOffset(int routeId, int stopId, int stopSequence, int previousEarliestArrivalTime);
    
};



#endif //CMAKE_RAPTOR_H