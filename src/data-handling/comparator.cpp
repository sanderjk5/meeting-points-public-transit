#include <comparator.h>
#include <importer.h>
#include <string>

using namespace std;

bool StopTimeComparator::compareByDepartureTime(StopTime stopTime1, StopTime stopTime2) {
    return stopTime1.departureTime < stopTime2.departureTime;
}

bool StopTimeComparator::compareByTripIdAndSequence(StopTime stopTime1, StopTime stopTime2) {
    if (stopTime1.tripId == stopTime2.tripId) {
        return stopTime1.stopSequence < stopTime2.stopSequence;
    }

    return stopTime1.tripId < stopTime2.tripId;
}

bool ConnectionComparator::compareByDepartureTime(Connection connection1, Connection connection2) {
    return connection1.departureTime < connection2.departureTime;
}

bool TripDepartureTimePairComparator::compareByDepartureTime(TripDepartureTimePair tripDepartureTimePair1, TripDepartureTimePair tripDepartureTimePair2) {
    return tripDepartureTimePair1.departureTime < tripDepartureTimePair2.departureTime;
}

bool FootPathComparator::compareByArrivalStop(FootPath footPath1, FootPath footPath2) {
    return footPath1.arrivalStopId < footPath2.arrivalStopId;
}