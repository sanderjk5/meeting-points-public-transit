#include "csa.h"

#include "journey.h"
#include <../data-handling/importer.h>
#include "../data-handling/converter.h"
#include <../constants.h>
#include <limits.h>
#include <iostream>
#include <chrono>


#include <vector>

using namespace std;

/*
    Initialize the CSA algorithm.
*/
void CSA::initializeCSA() {
    journeyPointers = vector<JourneyPointer>(Importer::stops.size());
    s = vector<int>(Importer::stops.size());
    t = vector<int>(Importer::trips.size());

    // Find first connection after the source time
    currentConnectionIndex = CSA::findFirstConnectionAfterTime(query.sourceTime);

    for (int i = 0; i < Importer::stops.size(); i++) {
        s[i] = INT_MAX;
    }

    s[query.sourceStopId] = query.sourceTime;

    for(int i = 0; i < Importer::trips.size(); i++) {
        t[i] = -1;
    }

    previousDepartureTime = query.sourceTime;
    dayOffset = 0;

    maxDepartureTime = query.sourceTime + (NUMBER_OF_DAYS * SECONDS_PER_DAY);

    connectionCounter = 0;
}

void CSA::setTargetStopIds(vector<int> targetStopIds) {
    query.targetStopIds = targetStopIds;
}

void CSA::setMaxDepartureTime(int maxDepartureTime) {
    this->maxDepartureTime = maxDepartureTime;
}

/*
    Processes the CSA algorithm to calculate the earliest arrival times for all stops.
    If target stops are specified, the algorithm will only calculate the earliest arrival time for these stops.
    Search for connections until the max departure time is reached.
*/
void CSA::processCSA() {
    while(previousDepartureTime + dayOffset * SECONDS_PER_DAY < maxDepartureTime) {
        Connection* connection = &Importer::connections[currentConnectionIndex];

        if (previousDepartureTime > connection->departureTime) {
            dayOffset++;
        }
        previousDepartureTime = connection->departureTime;

        // Calculate the arrival and departure time of the connection
        int connectionDepartureTime = connection->departureTime + dayOffset * SECONDS_PER_DAY;
        int connectionArrivalTime = connection->arrivalTime + dayOffset * SECONDS_PER_DAY;

        if (connectionArrivalTime < connectionDepartureTime) {
            connectionArrivalTime += SECONDS_PER_DAY;
        }

        // check if all target stops are reached
        if (query.targetStopIds.size() > 0){
            bool reachedAllTargetStops = true;
            for (int j = 0; j < query.targetStopIds.size(); j++) {
                if (s[query.targetStopIds[j]] > connectionDepartureTime) {
                    reachedAllTargetStops = false;
                    break;
                }
            }
            if(reachedAllTargetStops) {
                break;
            }
        }

        currentConnectionIndex = (currentConnectionIndex + 1) % Importer::connections.size();
        connectionCounter++;

        int weekdayOfTrip = (query.weekday + dayOffset) % 7;

        int firstDepartureTimeOfTrip = Importer::stopTimes[Importer::indexOfFirstStopTimeOfATrip[connection->tripId]].departureTime;

        if (firstDepartureTimeOfTrip > connection->departureTime) {
            weekdayOfTrip = (weekdayOfTrip + 6) % 7;
        }

        if (!Importer::isTripAvailable(connection->tripId, weekdayOfTrip)) {
            continue;
        }

        // Update the earliest arrival time for the arrival stop of the connection
        if (t[connection->tripId] != -1 || s[connection->departureStopId] < connectionDepartureTime) {
            if (t[connection->tripId] == -1) {
                t[connection->tripId] = connection->id;
            }

            if(connectionArrivalTime < s[connection->arrivalStopId]) {
                int arrivalStopId = connection->arrivalStopId;
                int tripId = connection->tripId;

                s[connection->arrivalStopId] = connectionArrivalTime;

                journeyPointers[connection->arrivalStopId].enterConnection = &Importer::connections[t[connection->tripId]];
                journeyPointers[connection->arrivalStopId].exitConnection = connection;
            }
        }
    }
}

/* 
    Use the journey pointers to create a journey from the target stop to the source stop.
*/
Journey CSA::createJourney(int targetStopId) {
    Journey journey;
    journey.duration = s[targetStopId] - query.sourceTime;

    vector<Leg> legs;

    int currentStopId = targetStopId;
    while (currentStopId != query.sourceStopId) {
        JourneyPointer journeyPointer = journeyPointers[currentStopId];

        Leg leg;
        leg.departureStopName = Importer::stops[journeyPointer.enterConnection->departureStopId].name;
        leg.arrivalStopName = Importer::stops[journeyPointer.exitConnection->arrivalStopId].name;
        leg.departureTime = journeyPointer.enterConnection->departureTime;
        leg.arrivalTime = journeyPointer.exitConnection->arrivalTime;

        legs.push_back(leg);

        currentStopId = journeyPointer.enterConnection->departureStopId;
    }

    for (int i = legs.size() - 1; i >= 0; i--) {
        journey.legs.push_back(legs[i]);
    }

    return journey;
}

/*
    Get the earliest arrival times for all stops.
*/
vector<int>* CSA::getEarliestArrivalTimes() {
    return &s;
}

/*
    Get the earliest arrival time for a specific stop.
*/
int CSA::getEarliestArrivalTime(int stopId) {
    return s[stopId];
}

/*
    Search for the first connection after a specific time using binary search.
*/
int CSA::findFirstConnectionAfterTime(int departureTime){
    int left = 0;
    int right = Importer::connections.size() - 1;
    int mid = 0;

    while (left <= right)
    {
        mid = (left + right) / 2;

        if (Importer::connections[mid].departureTime >= departureTime && (mid == 0 || Importer::connections[mid - 1].departureTime < departureTime))
        {
            return mid;
        }
        else if (Importer::connections[mid].departureTime < departureTime)
        {
            left = mid + 1;
        }
        else
        {
            right = mid - 1;
        }
    }

    return mid;
}

int CSA::getConnectionCounter() {
    return connectionCounter;
}