#include "csa.h"

#include "query-processor.h"
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

    for (int i = 0; i < Importer::connections.size(); i++) {
        Connection* connection = &Importer::connections[currentConnectionIndex];
        int weekdayOfTrip = query.weekday;

        int firstDepartureTimeOfTrip = Importer::stopTimes[Importer::indexOfFirstStopTimeOfATrip[connection->tripId]].departureTime;

        if (firstDepartureTimeOfTrip > connection->departureTime) {
            weekdayOfTrip = (weekdayOfTrip + 6) % 7;
        }

        if (!Importer::isTripAvailable(connection->tripId, weekdayOfTrip)) {
            currentConnectionIndex = (currentConnectionIndex + 1) % Importer::connections.size();
            continue;
        }

        int connectionDepartureTime = connection->departureTime;
        int connectionArrivalTime = connection->arrivalTime;
    
        if (connectionDepartureTime < query.sourceTime) {
            connectionDepartureTime += SECONDSPERDAY;
            connectionArrivalTime += SECONDSPERDAY;
        }

        if (connectionArrivalTime < query.sourceTime) {
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

Journey CSA::createJourney() {
    // TODO: Implement journey creation
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