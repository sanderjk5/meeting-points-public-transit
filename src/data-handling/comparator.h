#ifndef CMAKE_COMPARATOR_H
#define CMAKE_COMPARATOR_H

#include <importer.h>
#include <string>

using namespace std;

class StopTimesComparator {
    public:
        explicit StopTimesComparator(){};
        ~StopTimesComparator(){};

        static bool compareByDeparture(StopTime stopTime1, StopTime stopTime2);
        static bool compareByTripIdAndSequence(StopTime stopTime1, StopTime stopTime2);
};


#endif //CMAKE_COMPARATOR_H