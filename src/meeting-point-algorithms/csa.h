#ifndef CMAKE_CSA_H
#define CMAKE_CSA_H

#include <../data-handling/importer.h>
#include "journey.h"

struct CSAQuery {
    int sourceStopId;
    vector<int> targetStopIds;
    int sourceTime;
    int weekday;
};

struct JourneyPointer {
    Connection* enterConnection;
    Connection* exitConnection;
};

/*
    A CSA algorithm that calculates the earliest arrival times for all stops.
*/
class CSA {
    public:
        explicit CSA(CSAQuery query){
            this->query = query;
            this->initializeCSA();
        };
        ~CSA(){};

        void setTargetStopIds(vector<int> targetStopIds);
        void setMaxDepartureTime(int maxDepartureTime);
        void processCSA();
        Journey createJourney(int targetStopId);

        vector<int>* getEarliestArrivalTimes();
        int getEarliestArrivalTime(int stopId);
        int getConnectionCounter();

    private:
        CSAQuery query;
        vector<JourneyPointer> journeyPointers;
        vector<int> s;
        vector<int> t;
        int currentConnectionIndex;
        int previousDepartureTime;
        int dayOffset;
        int maxDepartureTime;
        int connectionCounter;

        vector<int> extendedSourceStopIds;

        void initializeCSA();

        static int findFirstConnectionAfterTime(int departureTime);
};

#endif //CMAKE_CSA_H