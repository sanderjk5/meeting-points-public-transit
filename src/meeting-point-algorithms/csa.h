#ifndef CMAKE_CSA_H
#define CMAKE_CSA_H

#include <../data-handling/importer.h>
#include "journey.h"

struct CSAQuery {
    int sourceStopId;
    vector<int> targetStopIds;
    int sourceTime;
    int weekday;
    int numberOfDays;
};

struct JourneyPointer {
    Connection* enterConnection;
    Connection* exitConnection;
};

class CSA {
    public:
        explicit CSA(CSAQuery query){
            this->query = query;
            this->journeyPointers = vector<JourneyPointer>(Importer::stops.size());
            this->s = vector<int>(Importer::stops.size());
            this->t = vector<int>(Importer::trips.size());
        };
        ~CSA(){};

        void processCSA(bool printTime = false);
        Journey createJourney(int targetStopId);

        vector<int>* getEarliestArrivalTimes();
        int getEarliestArrivalTime(int stopId);

    private:
        CSAQuery query;
        vector<JourneyPointer> journeyPointers;
        vector<int> s;
        vector<int> t;

        static int findFirstConnectionAfterTime(int departureTime);
};

#endif //CMAKE_CSA_H