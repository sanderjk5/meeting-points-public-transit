#ifndef CMAKE_JOURNEY_H
#define CMAKE_JOURNEY_H

#include <string>
#include <vector>

struct Leg {
    string departureStopName;
    string arrivalStopName;
    int departureTime;
    int arrivalTime;
};

struct Journey {
    vector<Leg> legs;
    int duration;
};

#endif //CMAKE_JOURNEY_H

