#ifndef CMAKE_COMPARATOR_H
#define CMAKE_COMPARATOR_H

#include <importer.h>
#include <string>

using namespace std;

class StopTimeComparator {
    public:
        explicit StopTimeComparator(){};
        ~StopTimeComparator(){};

        static bool compareByDepartureTime(StopTime stopTime1, StopTime stopTime2);
        static bool compareByTripIdAndSequence(StopTime stopTime1, StopTime stopTime2);
};

class ConnectionComparator {
    public:
        explicit ConnectionComparator(){};
        ~ConnectionComparator(){};

        static bool compareByDepartureTime(Connection connection1, Connection connection2);
};

class TripDepartureTimePairComparator {
    public:
        explicit TripDepartureTimePairComparator(){};
        ~TripDepartureTimePairComparator(){};

        static bool compareByDepartureTime(TripDepartureTimePair tripDepartureTimePair1, TripDepartureTimePair tripDepartureTimePair2);
};


#endif //CMAKE_COMPARATOR_H