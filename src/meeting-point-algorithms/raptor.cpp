#include "raptor.h"

#include "../data-handling/importer.h"
#include "../data-handling/converter.h"
#include "journey.h"
#include "optimization.h"
#include "../constants.h"

#include <limits.h>
#include <cfloat>

#include <algorithm>
#include <vector>
#include <set>
#include <cmath>

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

    extendedSourceStopIds = vector<int>();
    journeyPointers = vector<JourneyPointerRaptor>(Importer::stops.size(), JourneyPointerRaptor());

    int indexOfFirstFootpathOfSourceStop = Importer::indexOfFirstFootPathOfAStop[query.sourceStopId];
    for (int i = indexOfFirstFootpathOfSourceStop; i < Importer::footPaths.size(); i++) {
        if (Importer::footPaths[i].departureStopId != query.sourceStopId) {
            break;
        }
        currentEarliestArrivalTimes[Importer::footPaths[i].arrivalStopId] = query.sourceTime + Importer::footPaths[i].duration;
        currentMarkedStops[Importer::footPaths[i].arrivalStopId] = true;
        extendedSourceStopIds.push_back(Importer::footPaths[i].arrivalStopId);
    }

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
        numberOfExpandedRoutes++;

        int routeId = q[i].first;
        int stopSequence = q[i].second;

        vector<int>* stops = &Importer::stopsOfARoute[routeId];
        int currentTripId = -1;
        int currentDayOffset = 0;

        int enterTripAtStop = (*stops)[stopSequence];
        int currentTripDepartureTime = 0;
        for (int j = stopSequence; j < stops->size(); j++) {
            int stopId = (*stops)[j];
            StopTime stopTime;

            if (currentTripId != -1) {
                stopTime = Importer::stopTimes[Importer::indexOfFirstStopTimeOfATrip[currentTripId] + j];
                int arrivalTime = stopTime.arrivalTime + currentDayOffset;
                if (stopTime.arrivalTime > stopTime.departureTime) {
                    arrivalTime -= SECONDS_PER_DAY;
                }

                int indexOfFirstFootpathOfArrivalStop = Importer::indexOfFirstFootPathOfAStop[stopId];
                for (int k = indexOfFirstFootpathOfArrivalStop; k < Importer::footPaths.size(); k++) {
                    if (Importer::footPaths[k].departureStopId != stopId) {
                        break;
                    }
                    int newArrivalTime = arrivalTime + Importer::footPaths[k].duration;
                    if (newArrivalTime < currentEarliestArrivalTimes[Importer::footPaths[k].arrivalStopId]) {
                        currentEarliestArrivalTimes[Importer::footPaths[k].arrivalStopId] = newArrivalTime;
                        currentMarkedStops[Importer::footPaths[k].arrivalStopId] = true;
                        isFinishedFlag = false;
                        journeyPointers[Importer::footPaths[k].arrivalStopId] = JourneyPointerRaptor{enterTripAtStop, stopId, currentTripDepartureTime, newArrivalTime, currentTripId};
                    }
                }
            }

            if (previousMarkedStops[stopId] && (currentTripId == -1 || previousEarliestArrivalTimes[stopId] < stopTime.departureTime + currentDayOffset)) {
                TripInfo tripInfo = getEarliestTripWithDayOffset(routeId, stopId, j);
                if (tripInfo.tripId != -1) {
                    currentTripId = tripInfo.tripId;
                    currentDayOffset = tripInfo.dayOffset;
                    currentTripDepartureTime = tripInfo.tripDepartureTime;
                    enterTripAtStop = stopId;
                }
            }
        }
    }
}

TripInfo Raptor::getEarliestTripWithDayOffset(int routeId, int stopId, int stopSequence) {
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

            if (stopTime.departureTime + dayOffset > earliestDepartureTime) {
                TripInfo tripInfo = {tripId, dayOffset, stopTime.departureTime + dayOffset};
                return tripInfo;
            }
        }
        dayOffset += SECONDS_PER_DAY;
        weekday = (weekday + 1) % 7;
    }

    return {-1, -1, -1};
}

vector<int>* Raptor::getEarliestArrivalTimes() {
    return &currentEarliestArrivalTimes;
}

int Raptor::getEarliestArrivalTime(int stopId) {
    return currentEarliestArrivalTimes[stopId];
}

Journey Raptor::createJourney(int targetStopId) {
    Journey journey;
    journey.duration = currentEarliestArrivalTimes[targetStopId] - query.sourceTime;

    vector<Leg> legs;

    int currentStopId = targetStopId;
    while (find(extendedSourceStopIds.begin(), extendedSourceStopIds.end(), currentStopId) == extendedSourceStopIds.end()) {
        JourneyPointerRaptor journeyPointer = journeyPointers[currentStopId];

        Leg leg;
        leg.departureStopName = Importer::stops[journeyPointer.enterTripAtStop].name;
        leg.arrivalStopName = Importer::stops[journeyPointer.leaveTripAtStop].name;
        leg.departureTime = journeyPointer.departureTime;
        leg.arrivalTime = journeyPointer.arrivalTime;

        legs.push_back(leg);

        currentStopId = journeyPointer.enterTripAtStop;
    }

    for (int i = legs.size() - 1; i >= 0; i--) {
        journey.legs.push_back(legs[i]);
    }

    return journey;
}

void RaptorPQ::processRaptorPQ() {
    while (!pq.empty() && !isFinishedFlag) {
        traverseRoute();
    }
}

void RaptorPQ::setCurrentBest(int currentBest) {
    this->currentBest = currentBest;
}

void RaptorPQ::initializeHeuristic(map<int, vector<int>> sourceStopIdsToAllStops, vector<int> sourceStopIds) {
    this->sourceStopIdsToAllStops = sourceStopIdsToAllStops;
    this->sourceStopIds = sourceStopIds;
    this->numberOfSourceStopIds = sourceStopIds.size();

    baseHeuristic = 0;
    for (int i = 0; i < sourceStopIds.size(); i++) {
        int s1 = sourceStopIds[i];
        if (s1 == query.sourceStopId) {
            continue;
        }
        for (int j = i+1; j < sourceStopIds.size(); j++) {
            int s2 = sourceStopIds[j];
            if (s2 == query.sourceStopId) {
                continue;
            }
            baseHeuristic += sourceStopIdsToAllStops[s1][s2];
        }
    }
}

bool RaptorPQ::isFinished() {
    return isFinishedFlag;
}

vector<int>* RaptorPQ::getEarliestArrivalTimes() {
    return &earliestArrivalTimes;
}

int RaptorPQ::getEarliestArrivalTime(int stopId) {
    return earliestArrivalTimes[stopId];
}

Journey RaptorPQ::createJourney(int targetStopId) {
    Journey journey;
    journey.duration = earliestArrivalTimes[targetStopId] - query.sourceTime;

    vector<Leg> legs;

    int currentStopId = targetStopId;
    while (find(extendedSourceStopIds.begin(), extendedSourceStopIds.end(), currentStopId) == extendedSourceStopIds.end()) {
        JourneyPointerRaptor journeyPointer = journeyPointers[currentStopId];

        Leg leg;
        leg.departureStopName = Importer::stops[journeyPointer.enterTripAtStop].name;
        leg.arrivalStopName = Importer::stops[journeyPointer.leaveTripAtStop].name;
        leg.departureTime = journeyPointer.departureTime;
        leg.arrivalTime = journeyPointer.arrivalTime;

        legs.push_back(leg);

        currentStopId = journeyPointer.enterTripAtStop;
    }

    for (int i = legs.size() - 1; i >= 0; i--) {
        journey.legs.push_back(legs[i]);
    }

    return journey;
}

void RaptorPQ::transformRaptorToRaptorPQ(Raptor* raptor) {
    earliestArrivalTimes = raptor->currentEarliestArrivalTimes;
    journeyPointers = raptor->journeyPointers;
    extendedSourceStopIds = raptor->extendedSourceStopIds;

    firstStopSequencePerRoute = vector<int>(Importer::routes.size(), INT_MAX);
    lowestLowerBoundPerRoute = vector<double>(Importer::routes.size(), DBL_MAX);
    currentBest = INT_MAX;

    vector<int> arrivalStops;
    for (int i = 0; i < raptor->currentMarkedStops.size(); i++) {
        if (raptor->currentMarkedStops[i]) {
            arrivalStops.push_back(i);
        }
    }
    addRoutesToQueue(arrivalStops, -1);

    isFinishedFlag = false;
}

void RaptorPQ::initializeRaptorPQ() {
    earliestArrivalTimes = vector<int>(Importer::stops.size(), INT_MAX);

    firstStopSequencePerRoute = vector<int>(Importer::routes.size(), INT_MAX);
    lowestLowerBoundPerRoute = vector<double>(Importer::routes.size(), DBL_MAX);
    currentBest = INT_MAX;

    extendedSourceStopIds = vector<int>();
    journeyPointers = vector<JourneyPointerRaptor>(Importer::stops.size(), JourneyPointerRaptor());

    vector<int> arrivalStops;

    int indexOfFirstFootpathOfSourceStop = Importer::indexOfFirstFootPathOfAStop[query.sourceStopId];
    for (int i = indexOfFirstFootpathOfSourceStop; i < Importer::footPaths.size(); i++) {
        if (Importer::footPaths[i].departureStopId != query.sourceStopId) {
            break;
        }
        earliestArrivalTimes[Importer::footPaths[i].arrivalStopId] = query.sourceTime + Importer::footPaths[i].duration;
        extendedSourceStopIds.push_back(Importer::footPaths[i].arrivalStopId);
        arrivalStops.push_back(Importer::footPaths[i].arrivalStopId);
    }

    addRoutesToQueue(arrivalStops, -1);

    isFinishedFlag = false;
}

void RaptorPQ::addRoutesToQueue(vector<int> stopIds, int excludeRouteId) {
    set<int> newRoutes;

    for (int i = 0; i < stopIds.size(); i++) {
        int stopId = stopIds[i];
        vector<RouteSequencePair>* routes = &Importer::routesOfAStop[stopId];
        for (int i = 0; i < routes->size(); i++) {
            int routeId = (*routes)[i].routeId;
            if (routeId == excludeRouteId) {
                continue;
            }
            int stopSequence = (*routes)[i].stopSequence;

            double lowerBound = earliestArrivalTimes[stopId];

            // calculate clique heuristic
            double heuristic = baseHeuristic;
            for (int j = 0; j < sourceStopIds.size(); j++) {
                int s1 = sourceStopIds[j];
                if (s1 == query.sourceStopId) {
                    continue;
                }
                heuristic += sourceStopIdsToAllStops[s1][stopId];
            }
            heuristic = heuristic / (numberOfSourceStopIds - 1);

            if (optimization == min_sum || optimization == both) {
                lowerBound += heuristic;
            } else if (optimization == min_max) {
                double secondPart = (double) lowerBound + heuristic;
                secondPart = secondPart / numberOfSourceStopIds;
                lowerBound = max(lowerBound, secondPart);
            }

            if (lowerBound < lowestLowerBoundPerRoute[routeId]) {
                lowestLowerBoundPerRoute[routeId] = lowerBound;
                newRoutes.insert(routeId);
            }

            if (stopSequence < firstStopSequencePerRoute[routeId]) {
                firstStopSequencePerRoute[routeId] = stopSequence;
            }            
        }
    }

    for (set<int>::iterator it = newRoutes.begin(); it != newRoutes.end(); it++) {
        pq.push(make_pair(lowestLowerBoundPerRoute[*it], *it));
    }
}

void RaptorPQ::traverseRoute() {
    int lowerBound = pq.top().first;

    if (lowerBound > currentBest) {
        cout << "optimization: " << optimization << ", queue size: " << pq.size() << endl;
        isFinishedFlag = true;
        return;
    }

    int routeId = pq.top().second;
    pq.pop();

    if (lowerBound != lowestLowerBoundPerRoute[routeId] || firstStopSequencePerRoute[routeId] == INT_MAX) {
        return;
    }

    numberOfExpandedRoutes++;

    vector<int>* stops = &Importer::stopsOfARoute[routeId];
    int currentTripId = -1;
    int currentDayOffset = 0;

    int enterTripAtStop = (*stops)[firstStopSequencePerRoute[routeId]];
    int currentTripDepartureTime = 0;
    for (int i = firstStopSequencePerRoute[routeId]; i < stops->size(); i++) {
        int stopId = (*stops)[i];
        StopTime stopTime;

        int previousEarliestArrivalTime = earliestArrivalTimes[stopId];

        if (currentTripId != -1) {
            stopTime = Importer::stopTimes[Importer::indexOfFirstStopTimeOfATrip[currentTripId] + i];
            int arrivalTime = stopTime.arrivalTime + currentDayOffset;
            if (stopTime.arrivalTime > stopTime.departureTime) {
                arrivalTime -= SECONDS_PER_DAY;
            }

            vector<int> arrivalStops;
            int indexOfFirstFootpathOfArrivalStop = Importer::indexOfFirstFootPathOfAStop[stopId];
            for (int j = indexOfFirstFootpathOfArrivalStop; j < Importer::footPaths.size(); j++) {
                if (Importer::footPaths[j].departureStopId != stopId) {
                    break;
                }

                int newArrivalTime = arrivalTime + Importer::footPaths[j].duration;

                if (newArrivalTime < earliestArrivalTimes[Importer::footPaths[j].arrivalStopId]) {
                    earliestArrivalTimes[Importer::footPaths[j].arrivalStopId] = newArrivalTime;
                    journeyPointers[Importer::footPaths[j].arrivalStopId] = JourneyPointerRaptor{enterTripAtStop, stopId, currentTripDepartureTime, newArrivalTime, currentTripId};
                    arrivalStops.push_back(Importer::footPaths[j].arrivalStopId);
                }
            }

            addRoutesToQueue(arrivalStops, routeId);
        }

        if (currentTripId == -1 || previousEarliestArrivalTime < stopTime.departureTime + currentDayOffset) {
            TripInfo tripInfo = getEarliestTripWithDayOffset(routeId, stopId, i, previousEarliestArrivalTime);
            if (tripInfo.tripId != -1) {
                currentTripId = tripInfo.tripId;
                currentDayOffset = tripInfo.dayOffset;
                currentTripDepartureTime = tripInfo.tripDepartureTime;
                enterTripAtStop = stopId;
            }
        }
    }

    firstStopSequencePerRoute[routeId] = INT_MAX;
    lowestLowerBoundPerRoute[routeId] = INT_MAX;
}

TripInfo RaptorPQ::getEarliestTripWithDayOffset(int routeId, int stopId, int stopSequence, int previousEarliestArrivalTime) {
    vector<int>* trips = &Importer::tripsOfARoute[routeId];

    int earliestDepartureTime = previousEarliestArrivalTime;
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

            if (stopTime.departureTime + dayOffset > earliestDepartureTime) {
                TripInfo tripInfo = {tripId, dayOffset, stopTime.departureTime + dayOffset};
                return tripInfo;
            }
        }
        dayOffset += SECONDS_PER_DAY;
        weekday = (weekday + 1) % 7;
    }

    return {-1, -1, -1};
} 