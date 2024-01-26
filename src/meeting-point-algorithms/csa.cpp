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
    Processes the CSA algorithm to calculate the earliest arrival times for all stops.
    If a target stop is specified, the algorithm will only calculate the earliest arrival time for that stop.
    Search for a maximum time of 24 hours after the source time.
*/
void CSA::processCSA(bool printTime) {
    auto start = std::chrono::high_resolution_clock::now();

    // Find first connection after the source time
    int currentConnectionIndex = CSA::findFirstConnectionAfterTime(query.sourceTime);

    for (int i = 0; i < Importer::stops.size(); i++) {
        s[i] = INT_MAX;
    }

    s[query.sourceStopId] = query.sourceTime;

    for(int i = 0; i < Importer::trips.size(); i++) {
        t[i] = -1;
    }

    int previousDepartureTime = query.sourceTime;
    int dayOffset = 0;

    int maxNumberOfConnections = Importer::connections.size() * query.numberOfDays;

    for (int i = 0; i < maxNumberOfConnections; i++) {
        Connection* connection = &Importer::connections[currentConnectionIndex];

        if (previousDepartureTime > connection->departureTime) {
            dayOffset++;
        }
        previousDepartureTime = connection->departureTime;

        int weekdayOfTrip = (query.weekday + dayOffset) % 7;

        int firstDepartureTimeOfTrip = Importer::stopTimes[Importer::indexOfFirstStopTimeOfATrip[connection->tripId]].departureTime;

        if (firstDepartureTimeOfTrip > connection->departureTime) {
            weekdayOfTrip = (weekdayOfTrip + 6) % 7;
        }

        if (!Importer::isTripAvailable(connection->tripId, weekdayOfTrip)) {
            currentConnectionIndex = (currentConnectionIndex + 1) % Importer::connections.size();
            continue;
        }

        int connectionDepartureTime = connection->departureTime + dayOffset * SECONDSPERDAY;
        int connectionArrivalTime = connection->arrivalTime + dayOffset * SECONDSPERDAY;

        if (connectionArrivalTime < connectionDepartureTime) {
            connectionArrivalTime += SECONDSPERDAY;
        }

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

                if (query.targetStopIds.size() > 0){
                    bool reachedTargetStop = false;
                    for (int j = 0; j < query.targetStopIds.size(); j++) {
                        if (connection->arrivalStopId == query.targetStopIds[j]) {
                            reachedTargetStop = true;
                            break;
                        }
                    }
                    if(reachedTargetStop) {
                        break;
                    }
                }
            }
        }

        currentConnectionIndex = (currentConnectionIndex + 1) % Importer::connections.size();
    }

    // Stop the timer and calculate the duration
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

    // Print the duration
    if (printTime) {
        cout << "CSA duration : " << duration << " milliseconds" << endl;
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

vector<int>* CSA::getEarliestArrivalTimes() {
    return &s;
}

int CSA::getEarliestArrivalTime(int stopId) {
    return s[stopId];
}

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