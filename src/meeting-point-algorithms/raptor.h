#ifndef CMAKE_RAPTOR_H
#define CMAKE_RAPTOR_H

#include <../data-handling/importer.h>
#include <vector>

struct RaptorQuery {
    int sourceStopId;
    vector<int> targetStopIds;
    int sourceTime;
    int weekday;
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

    private:
        RaptorQuery query;
        int currentRound;
        vector<int> previousEarliestArrivalTimes;
        vector<int> currentEarliestArrivalTimes;
        vector<bool> previousMarkedStops;
        vector<bool> currentMarkedStops;
        vector<int> minStopSequencePerRoute;
        vector<int> tripIndexPerRoute;
        vector<pair<int, int>> q;

        void initializeRaptor();
        void fillQ();
        void traverseRoutes();
        pair<int, int> getEarliestTripWithDayOffset(int routeId, int stopId, int stopSequence);
};



#endif //CMAKE_RAPTOR_H