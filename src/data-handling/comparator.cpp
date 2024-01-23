#include <comparator.h>
#include <importer.h>
#include <string>

using namespace std;

bool StopTimesComparator::compareByDeparture(StopTime stopTime1, StopTime stopTime2) {
    return stopTime1.departureTime < stopTime2.departureTime;
}

bool StopTimesComparator::compareByTripIdAndSequence(StopTime stopTime1, StopTime stopTime2) {
    if (stopTime1.tripId == stopTime2.tripId) {
        return stopTime1.stopSequence < stopTime2.stopSequence;
    }

    return stopTime1.tripId < stopTime2.tripId;
}

bool TripDepartureTimePairComparator::compareByDeparture(TripDepartureTimePair tripDepartureTimePair1, TripDepartureTimePair tripDepartureTimePair2) {
    return tripDepartureTimePair1.departureTime < tripDepartureTimePair2.departureTime;
}