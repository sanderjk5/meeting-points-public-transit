#include "raptor.h"

#include "../data-handling/importer.h"
#include "../data-handling/converter.h"
#include "../constants.h"

#include <limits.h>

#include <iostream>

void Raptor::setTargetStopIds(vector<int> targetStopIds) {
    this->query.targetStopIds = targetStopIds;
}

void Raptor::initializeRaptor() {
    currentRound = 0;

    previousEarliestArrivalTimes = vector<int>(Importer::stops.size(), INT_MAX);
    currentEarliestArrivalTimes = vector<int>(Importer::stops.size(), INT_MAX);
    previousMarkedStops = vector<bool>(Importer::stops.size(), false);
    currentMarkedStops = vector<bool>(Importer::stops.size(), false);

    currentEarliestArrivalTimes[query.sourceStopId] = query.sourceTime;
    currentMarkedStops[query.sourceStopId] = true;

    q = vector<pair<int, int>>();

    isFinishedFlag = false;
}

void Raptor::processRaptor() {
    while (true) {
        processRaptorRound();
        if (isFinishedFlag) {
            break;
        }
    }
}

void Raptor::processRaptorRound() {
    isFinishedFlag = true;
    currentRound++;
    previousEarliestArrivalTimes = currentEarliestArrivalTimes;
    previousMarkedStops = currentMarkedStops;
    currentMarkedStops = vector<bool>(Importer::stops.size(), false);
    tripIndexPerRoute = vector<int>(Importer::routes.size(), -1);

    fillQ();
    traverseRoutes();
}

bool Raptor::isFinished() {
    return isFinishedFlag;
}

void Raptor::fillQ() {
    q.clear();
    minStopSequencePerRoute = vector<int>(Importer::routes.size(), INT_MAX);
    vector<pair<int, int>> qTemp = vector<pair<int, int>>();
    for (int stopId = 0; stopId < currentMarkedStops.size(); stopId++) {
        if (previousMarkedStops[stopId]) {
            vector<RouteSequencePair>* routes = &Importer::routesOfAStop[stopId];
            for (int i = 0; i < routes->size(); i++) {
                int routeId = (*routes)[i].routeId;
                int stopSequence = (*routes)[i].stopSequence;
                if (stopSequence < minStopSequencePerRoute[routeId]) {
                    minStopSequencePerRoute[routeId] = stopSequence;
                    qTemp.push_back(make_pair(routeId, stopSequence));
                }
            }
        }
    }

    for (int i = 0; i < qTemp.size(); i++) {
        if (qTemp[i].second == minStopSequencePerRoute[qTemp[i].first]) {
            q.push_back(qTemp[i]);
        }
    }
}

void Raptor::traverseRoutes() {
    for (int i = 0; i < q.size(); i++) {
        int routeId = q[i].first;
        int stopSequence = q[i].second;

        vector<int>* stops = &Importer::stopsOfARoute[routeId];
        int currentTripId = -1;
        int currentDayOffset = 0;

        for (int j = stopSequence; j < stops->size(); j++) {
            int stopId = (*stops)[j];
            StopTime stopTime;

            if (currentTripId != -1) {
                stopTime = Importer::stopTimes[Importer::indexOfFirstStopTimeOfATrip[currentTripId] + j];
                int arrivalTime = stopTime.arrivalTime + currentDayOffset;
                if (stopTime.arrivalTime > stopTime.departureTime) {
                    arrivalTime -= SECONDS_PER_DAY;
                }

                if (arrivalTime < currentEarliestArrivalTimes[stopId]) {
                    currentEarliestArrivalTimes[stopId] = arrivalTime;
                    currentMarkedStops[stopId] = true;
                    isFinishedFlag = false;
                }
            }

            if (previousMarkedStops[stopId] && (currentTripId == -1 || previousEarliestArrivalTimes[stopId] < stopTime.departureTime + currentDayOffset)) {
                // cout << "routeId: " << routeId << " stopId: " << stopId << " stopSequence: " << j << endl;
                pair<int, int> tripWithDayOffset = getEarliestTripWithDayOffset(routeId, stopId, j);
                if (tripWithDayOffset.first != -1) {
                    currentTripId = tripWithDayOffset.first;
                    currentDayOffset = tripWithDayOffset.second;
                }
            }
        }
    }
}

pair<int, int> Raptor::getEarliestTripWithDayOffset(int routeId, int stopId, int stopSequence) {
    vector<int>* trips = &Importer::tripsOfARoute[routeId];

    int earliestDepartureTime = previousEarliestArrivalTimes[stopId];
    int dayOffset = TimeConverter::getDayOffset(earliestDepartureTime);
    int weekday = (query.weekday + TimeConverter::getDayDifference(earliestDepartureTime)) % 7;

    StopTime stopTime;

    while (dayOffset <= NUMBER_OF_DAYS * SECONDS_PER_DAY) {
        for (int i = 0; i < trips->size(); i++) {
            int tripId = (*trips)[i];
            stopTime = Importer::stopTimes[Importer::indexOfFirstStopTimeOfATrip[tripId] + stopSequence];

            // check if trip is available
            int weekdayOfTrip = weekday;
            int firstDepartureTimeOfTrip = Importer::stopTimes[Importer::indexOfFirstStopTimeOfATrip[tripId]].departureTime;
            if (firstDepartureTimeOfTrip > stopTime.departureTime) {
                weekdayOfTrip = (weekdayOfTrip + 6) % 7;
            }
            if (!Importer::isTripAvailable(tripId, weekdayOfTrip)) {
                continue;
            }

            if (stopTime.departureTime + dayOffset >= earliestDepartureTime) {
                tripIndexPerRoute[routeId] = i;
                return make_pair(tripId, dayOffset);
            }
        }
        dayOffset += SECONDS_PER_DAY;
        weekday = (weekday + 1) % 7;
    }

    return make_pair(-1, -1);
}

vector<int>* Raptor::getEarliestArrivalTimes() {
    return &currentEarliestArrivalTimes;
}

int Raptor::getEarliestArrivalTime(int stopId) {
    return currentEarliestArrivalTimes[stopId];
}