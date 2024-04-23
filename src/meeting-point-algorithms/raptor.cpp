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
#include <memory>

#include <chrono>

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
    numberOfExpandedRoutes = 0;
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

void RaptorBound::setCurrentBest(int currentBest) {
    this->currentBest = currentBest;
}

void RaptorBound::initializeHeuristic(map<int, vector<int>> sourceStopIdsToAllStops, vector<int> sourceStopIds) {
    this->sourceStopIdsToAllStops = sourceStopIdsToAllStops;
    this->sourceStopIds = sourceStopIds;
    this->numberOfSourceStopIds = sourceStopIds.size();

    baseHeuristic = 0;
    for (int i = 0; i < numberOfSourceStopIds; i++) {
        int s1 = sourceStopIds[i];
        if (s1 == query.sourceStopId) {
            continue;
        }
        for (int j = i+1; j < numberOfSourceStopIds; j++) {
            int s2 = sourceStopIds[j];
            if (s2 == query.sourceStopId) {
                continue;
            }
            if (sourceStopIdsToAllStops[s1][s2] == INT_MAX) {
                continue;
            }
            baseHeuristic += sourceStopIdsToAllStops[s1][s2];
        }
    }
}

void RaptorBound::initializeRaptorBound() {
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
    numberOfExpandedRoutes = 0;
    currentBest = INT_MAX;

    heuristicPerStopId = vector<int>(Importer::stops.size(), -1);
}

void RaptorBound::processRaptorRound() {
    isFinishedFlag = true;
    currentRound++;
    previousEarliestArrivalTimes = currentEarliestArrivalTimes;
    previousMarkedStops = currentMarkedStops;
    currentMarkedStops = vector<bool>(Importer::stops.size(), false);

    fillQ();
    traverseRoutes();
}

bool RaptorBound::isFinished() {
    return isFinishedFlag;
}

void RaptorBound::fillQ() {
    q.clear();
    minStopSequencePerRoute = vector<int>(Importer::routes.size(), INT_MAX);
    vector<pair<int, int>> qTemp = vector<pair<int, int>>();
    for (int stopId = 0; stopId < currentMarkedStops.size(); stopId++) {
        if (previousMarkedStops[stopId]) {
            double lowerBound = currentEarliestArrivalTimes[stopId] - query.sourceTime;

            if (heuristicPerStopId[stopId] == -1) {
                // calculate clique heuristic
                double heuristic = baseHeuristic;
                double maxDist = 0;
                for (int j = 0; j < sourceStopIds.size(); j++) {
                    int s1 = sourceStopIds[j];
                    if (s1 == query.sourceStopId) {
                        continue;
                    }
                    heuristic += sourceStopIdsToAllStops[s1][stopId];
                    // if (sourceStopIdsToAllStops[s1][stopId] > maxDist) {
                    //     maxDist = sourceStopIdsToAllStops[s1][stopId];
                    // }
                }
                heuristic = heuristic / (numberOfSourceStopIds - 1);

                heuristicPerStopId[stopId] = heuristic;
            }

            double heuristic = heuristicPerStopId[stopId];            

            if (optimization == min_sum || optimization == both) {
                lowerBound += heuristic;
            } else if (optimization == min_max) {
                double secondPart = (double) lowerBound + heuristic;
                secondPart = secondPart / numberOfSourceStopIds;

                lowerBound = max(lowerBound, secondPart);

                // double alternativeHeuristic = (double) lowerBound + maxDist;
                // alternativeHeuristic = alternativeHeuristic / 2;

                // if (alternativeHeuristic > lowerBound && alternativeHeuristic > secondPart) {
                //     lowerBound = alternativeHeuristic;
                // } else {
                //     lowerBound = max(lowerBound, secondPart);
                // }
            }

            if (lowerBound > currentBest) {
                continue;
            }

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

void RaptorBound::traverseRoutes() {
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

                    int upperBound = currentBest;
                    if (upperBound < INT_MAX) {
                        upperBound += query.sourceTime;
                    }
                    int newArrivalTime = arrivalTime + Importer::footPaths[k].duration;

                    if (newArrivalTime < currentEarliestArrivalTimes[Importer::footPaths[k].arrivalStopId] && newArrivalTime < upperBound) {
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

TripInfo RaptorBound::getEarliestTripWithDayOffset(int routeId, int stopId, int stopSequence) {
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

vector<int>* RaptorBound::getEarliestArrivalTimes() {
    return &currentEarliestArrivalTimes;
}

int RaptorBound::getEarliestArrivalTime(int stopId) {
    return currentEarliestArrivalTimes[stopId];
}

Journey RaptorBound::createJourney(int targetStopId) {
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
    auto start = chrono::high_resolution_clock::now();
    this->sourceStopIdsToAllStops = sourceStopIdsToAllStops;
    this->sourceStopIds = sourceStopIds;
    this->numberOfSourceStopIds = sourceStopIds.size();

    baseHeuristic = 0;
    for (int i = 0; i < numberOfSourceStopIds; i++) {
        int s1 = sourceStopIds[i];
        if (s1 == query.sourceStopId) {
            continue;
        }
        for (int j = i+1; j < numberOfSourceStopIds; j++) {
            int s2 = sourceStopIds[j];
            if (s2 == query.sourceStopId) {
                continue;
            }
            baseHeuristic += sourceStopIdsToAllStops[s1][s2];
        }
    }
    auto end = chrono::high_resolution_clock::now();
    durationInitHeuristic = chrono::duration_cast<chrono::microseconds>(end - start).count();
    // cout << "base heuristic: " << baseHeuristic << endl;
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

void RaptorPQ::transformRaptorToRaptorPQ(shared_ptr<Raptor> raptor) {
    auto start = chrono::high_resolution_clock::now();
    earliestArrivalTimes = raptor->currentEarliestArrivalTimes;
    journeyPointers = raptor->journeyPointers;
    extendedSourceStopIds = raptor->extendedSourceStopIds;

    firstStopSequencePerRoute = vector<int>(Importer::routes.size(), INT_MAX);
    lowestLowerBoundPerRoute = vector<double>(Importer::routes.size(), DBL_MAX);
    markedStopsPerRoute = vector<set<int>>(Importer::routes.size(), set<int>());

    heuristicPerStopId = vector<int>(Importer::stops.size(), -1);

    set<int> arrivalStops;
    
    for (int i = 0; i < raptor->currentMarkedStops.size(); i++) {
        if (raptor->currentMarkedStops[i]) {
            arrivalStops.insert(i);
        }
    }
    addRoutesToQueue(arrivalStops, -1);

    isFinishedFlag = false;
    numberOfExpandedRoutes = 0;
    auto end = chrono::high_resolution_clock::now();
    durationTransformRaptorToRaptorPQ = chrono::duration_cast<chrono::microseconds>(end - start).count();
}

void RaptorPQ::initializeRaptorPQ() {
    earliestArrivalTimes = vector<int>(Importer::stops.size(), INT_MAX);

    firstStopSequencePerRoute = vector<int>(Importer::routes.size(), INT_MAX);
    lowestLowerBoundPerRoute = vector<double>(Importer::routes.size(), DBL_MAX);
    markedStopsPerRoute = vector<set<int>>(Importer::routes.size(), set<int>());

    heuristicPerStopId = vector<int>(Importer::stops.size(), -1);

    extendedSourceStopIds = vector<int>();
    journeyPointers = vector<JourneyPointerRaptor>(Importer::stops.size(), JourneyPointerRaptor());

    set<int> arrivalStops;

    int indexOfFirstFootpathOfSourceStop = Importer::indexOfFirstFootPathOfAStop[query.sourceStopId];
    for (int i = indexOfFirstFootpathOfSourceStop; i < Importer::footPaths.size(); i++) {
        if (Importer::footPaths[i].departureStopId != query.sourceStopId) {
            break;
        }
        earliestArrivalTimes[Importer::footPaths[i].arrivalStopId] = query.sourceTime + Importer::footPaths[i].duration;
        extendedSourceStopIds.push_back(Importer::footPaths[i].arrivalStopId);
        arrivalStops.insert(Importer::footPaths[i].arrivalStopId);
    }

    addRoutesToQueue(arrivalStops, -1);

    isFinishedFlag = false;
    numberOfExpandedRoutes = 0;
}

void RaptorPQ::addRoutesToQueue(set<int> stopIds, int excludeRouteId) {
    auto start = chrono::high_resolution_clock::now();
    set<int> newRoutes;

    for (set<int>::iterator it = stopIds.begin(); it != stopIds.end(); it++) {
        int stopId = *it;
        double lowerBound = earliestArrivalTimes[stopId] - query.sourceTime;

        // calculate clique heuristic
        if(heuristicPerStopId[stopId] == -1) {
            double heuristic = baseHeuristic;
            double maxDist = 0;
            for (int j = 0; j < sourceStopIds.size(); j++) {
                int s1 = sourceStopIds[j];
                if (s1 == query.sourceStopId) {
                    continue;
                }
                heuristic += sourceStopIdsToAllStops[s1][stopId];
                // if (sourceStopIdsToAllStops[s1][stopId] > maxDist) {
                //     maxDist = sourceStopIdsToAllStops[s1][stopId];
                // }
            }
            heuristic = heuristic / (numberOfSourceStopIds - 1);

            heuristicPerStopId[stopId] = heuristic;
        }

        double heuristic = heuristicPerStopId[stopId];

        if (optimization == min_sum || optimization == both) {
            lowerBound += heuristic;
        } else if (optimization == min_max) {
            double secondPart = (double) lowerBound + heuristic;
            secondPart = secondPart / numberOfSourceStopIds;

            lowerBound = max(lowerBound, secondPart);

            // double alternativeHeuristic = (double) lowerBound + maxDist;
            // alternativeHeuristic = alternativeHeuristic / 2;

            // if (alternativeHeuristic > lowerBound && alternativeHeuristic > secondPart) {
            //     altHeuristicImprovementCounter++;
            //     lowerBound = alternativeHeuristic;
            // } else {
            //     lowerBound = max(lowerBound, secondPart);
            //     noHeuristicImprovementCounter++;
            // }
        }

        if (lowerBound > currentBest) {
            continue;
        }

        vector<RouteSequencePair>* routes = &Importer::routesOfAStop[stopId];
        for (int j = 0; j < routes->size(); j++) {
            int routeId = (*routes)[j].routeId;
            if (routeId == excludeRouteId) {
                continue;
            }
            int stopSequence = (*routes)[j].stopSequence;

            if (lowerBound < lowestLowerBoundPerRoute[routeId]) {
                lowestLowerBoundPerRoute[routeId] = lowerBound;
                newRoutes.insert(routeId);
            }

            if (stopSequence < firstStopSequencePerRoute[routeId]) {
                firstStopSequencePerRoute[routeId] = stopSequence;
            }    

            // markedStopsPerRoute[routeId].insert(stopId);        
        }
    }

    for (set<int>::iterator it = newRoutes.begin(); it != newRoutes.end(); it++) {
        pq.push(make_pair(lowestLowerBoundPerRoute[*it], *it));
    }

    auto end = chrono::high_resolution_clock::now();
    durationAddRoutesToQueue += chrono::duration_cast<chrono::microseconds>(end - start).count();
}

void RaptorPQ::traverseRoute() {
    auto start = chrono::high_resolution_clock::now();
    double lowerBound = pq.top().first;

    if (lowerBound > currentBest) {
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

    set<int> arrivalStops;

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

            int indexOfFirstFootpathOfArrivalStop = Importer::indexOfFirstFootPathOfAStop[stopId];
            for (int j = indexOfFirstFootpathOfArrivalStop; j < Importer::footPaths.size(); j++) {
                if (Importer::footPaths[j].departureStopId != stopId) {
                    break;
                }

                int arrivalStopId = Importer::footPaths[j].arrivalStopId;
                int newArrivalTime = arrivalTime + Importer::footPaths[j].duration;

                if (newArrivalTime < earliestArrivalTimes[arrivalStopId] && newArrivalTime < currentBest + query.sourceTime) {
                    earliestArrivalTimes[arrivalStopId] = newArrivalTime;
                    journeyPointers[arrivalStopId] = JourneyPointerRaptor{enterTripAtStop, stopId, currentTripDepartureTime, newArrivalTime, currentTripId};
                    arrivalStops.insert(arrivalStopId);
                }
            }
        }

        // check if stop is in markedStops set of route
        // bool stopIsMarked = markedStopsPerRoute[routeId].find(stopId) != markedStopsPerRoute[routeId].end();

        // if (stopIsMarked && (currentTripId == -1 || previousEarliestArrivalTime < stopTime.departureTime + currentDayOffset)) {
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

    addRoutesToQueue(arrivalStops, routeId);

    firstStopSequencePerRoute[routeId] = INT_MAX;
    lowestLowerBoundPerRoute[routeId] = INT_MAX;
    // markedStopsPerRoute[routeId] = set<int>();
    auto end = chrono::high_resolution_clock::now();
    durationTraverseRoute += chrono::duration_cast<chrono::microseconds>(end - start).count();
}

TripInfo RaptorPQ::getEarliestTripWithDayOffset(int routeId, int stopId, int stopSequence, int previousEarliestArrivalTime) {
    auto start = chrono::high_resolution_clock::now();
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
                auto end = chrono::high_resolution_clock::now();
                durationGetEarliestTripWithDayOffset += chrono::duration_cast<chrono::microseconds>(end - start).count();
                return tripInfo;
            }
        }
        dayOffset += SECONDS_PER_DAY;
        weekday = (weekday + 1) % 7;
    }

    auto end = chrono::high_resolution_clock::now();
    durationGetEarliestTripWithDayOffset += chrono::duration_cast<chrono::microseconds>(end - start).count();

    return {-1, -1, -1};
} 

void RaptorPQParallel::setCurrentBest(int currentBest) {
    this->currentBest = currentBest;
}

void RaptorPQParallel::transformRaptorsToRaptorPQs(vector<shared_ptr<Raptor>> raptors) {
    auto start = chrono::high_resolution_clock::now();

    earliestArrivalTimes = vector<vector<int>>(raptors.size());
    journeyPointers = vector<vector<JourneyPointerRaptor>>(raptors.size());
    extendedSourceStopIds = vector<vector<int>>(raptors.size());
    firstStopSequencePerRoute = vector<vector<int>>(raptors.size());
    lowestLowerBoundPerRoute = vector<vector<double>>(raptors.size());

    vector<set<int>> routesPerRatorIndex = vector<set<int>>(raptors.size());

    #pragma omp parallel for
    for (int i = 0; i < raptors.size(); i++) {
        shared_ptr<Raptor> raptor = raptors[i];
        earliestArrivalTimes[i] = raptor->currentEarliestArrivalTimes;
        journeyPointers[i] = raptor->journeyPointers;
        extendedSourceStopIds[i] = raptor->extendedSourceStopIds;

        firstStopSequencePerRoute[i] = vector<int>(Importer::routes.size(), INT_MAX);
        lowestLowerBoundPerRoute[i] = vector<double>(Importer::routes.size(), DBL_MAX);
        
        set<int> arrivalStops;
        for (int j = 0; j < raptor->currentMarkedStops.size(); j++) {
            if (raptor->currentMarkedStops[j]) {
                arrivalStops.insert(j);
            }
        }
        routesPerRatorIndex[i] = getNewRoutes(arrivalStops, -1, i);
    }

    for (int i = 0; i < raptors.size(); i++) {
        addRoutesToQueue(routesPerRatorIndex[i], i);
    }
    auto end = chrono::high_resolution_clock::now();
    durationTransformRaptorToRaptorPQ = chrono::duration_cast<chrono::microseconds>(end - start).count();
}

void RaptorPQParallel::initializeHeuristics(map<int, vector<int>> sourceStopIdsToAllStops, vector<int> sourceStopIds) {
    auto start = chrono::high_resolution_clock::now();
    this->sourceStopIdsToAllStops = sourceStopIdsToAllStops;
    this->sourceStopIds = sourceStopIds;
    this->numberOfSourceStopIds = sourceStopIds.size();

    this->baseHeuristics = vector<double>(sourceStopIds.size(), 0);

    #pragma omp parallel for
    for (int i = 0; i < numberOfSourceStopIds; i++) {
        int querySourceStopId = sourceStopIds[i];
        for (int j = 0; j < numberOfSourceStopIds; j++) {
            int s1 = sourceStopIds[j];
            if (s1 == querySourceStopId) {
                continue;
            }
            for (int k = j+1; k < numberOfSourceStopIds; k++) {
                int s2 = sourceStopIds[k];
                if (s2 == querySourceStopId) {
                    continue;
                }
                baseHeuristics[i] += sourceStopIdsToAllStops[s1][s2];
            }
        }
    }
    auto end = chrono::high_resolution_clock::now();
    durationInitHeuristic = chrono::duration_cast<chrono::microseconds>(end - start).count();
}

void RaptorPQParallel::processRaptorPQ() {
    while (!pq.empty() && !isFinishedFlag) {
        traverseRoute();
    }
}

vector<int> RaptorPQParallel::getEarliestArrivalTimes(int stopId) {
    vector<int> earliestArrivalTimes;
    for (int i = 0; i < this->earliestArrivalTimes.size(); i++) {
        earliestArrivalTimes.push_back(this->earliestArrivalTimes[i][stopId]);
    }
    return earliestArrivalTimes;
}

vector<Journey> RaptorPQParallel::createJourneys(int targetStopId) {
    vector<Journey> journeys;
    for (int i = 0; i < earliestArrivalTimes.size(); i++) {
        Journey journey;
        journey.duration = earliestArrivalTimes[i][targetStopId] - queries[0].sourceTime;

        vector<Leg> legs;

        int currentStopId = targetStopId;
        while (find(extendedSourceStopIds[i].begin(), extendedSourceStopIds[i].end(), currentStopId) == extendedSourceStopIds[i].end()) {
            JourneyPointerRaptor journeyPointer = journeyPointers[i][currentStopId];

            Leg leg;
            leg.departureStopName = Importer::stops[journeyPointer.enterTripAtStop].name;
            leg.arrivalStopName = Importer::stops[journeyPointer.leaveTripAtStop].name;
            leg.departureTime = journeyPointer.departureTime;
            leg.arrivalTime = journeyPointer.arrivalTime;

            legs.push_back(leg);

            currentStopId = journeyPointer.enterTripAtStop;
        }

        for (int j = legs.size() - 1; j >= 0; j--) {
            journey.legs.push_back(legs[j]);
        }

        journeys.push_back(journey);
    }
    return journeys;
}

void RaptorPQParallel::traverseRoute() {
    auto start = chrono::high_resolution_clock::now();

    PQEntry pqEntry = pq.top();
    pq.pop();

    double lowerBound = pqEntry.lowerBound;
    int routeId = pqEntry.routeId;
    int raptorIndex = pqEntry.raptorIndex;

    if (lowerBound > currentBest) {
        isFinishedFlag = true;
        auto end = chrono::high_resolution_clock::now();
        durationTraverseRoute += chrono::duration_cast<chrono::microseconds>(end - start).count();
        return;
    }

    int firstStopSequenceOfRoute = firstStopSequencePerRoute[raptorIndex][routeId];

    if (lowerBound != lowestLowerBoundPerRoute[raptorIndex][routeId] || firstStopSequenceOfRoute == INT_MAX) {
        auto end = chrono::high_resolution_clock::now();
        durationTraverseRoute += chrono::duration_cast<chrono::microseconds>(end - start).count();
        return;
    }

    numberOfExpandedRoutes++;

    vector<int>* stops = &Importer::stopsOfARoute[routeId];
    int currentTripId = -1;
    int currentDayOffset = 0;

    int enterTripAtStop = (*stops)[firstStopSequenceOfRoute];
    int currentTripDepartureTime = 0;

    set<int> arrivalStops;

    for (int i = firstStopSequenceOfRoute; i < stops->size(); i++) {
        int stopId = (*stops)[i];
        StopTime stopTime;

        int previousEarliestArrivalTime = earliestArrivalTimes[raptorIndex][stopId];

        if (currentTripId != -1) {
            stopTime = Importer::stopTimes[Importer::indexOfFirstStopTimeOfATrip[currentTripId] + i];
            int arrivalTime = stopTime.arrivalTime + currentDayOffset;
            if (stopTime.arrivalTime > stopTime.departureTime) {
                arrivalTime -= SECONDS_PER_DAY;
            }

            int indexOfFirstFootpathOfArrivalStop = Importer::indexOfFirstFootPathOfAStop[stopId];
            for (int j = indexOfFirstFootpathOfArrivalStop; j < Importer::footPaths.size(); j++) {
                if (Importer::footPaths[j].departureStopId != stopId) {
                    break;
                }

                int arrivalStopId = Importer::footPaths[j].arrivalStopId;
                int newArrivalTime = arrivalTime + Importer::footPaths[j].duration;

                if (newArrivalTime < earliestArrivalTimes[raptorIndex][arrivalStopId] && newArrivalTime < currentBest + queries[raptorIndex].sourceTime) {
                    earliestArrivalTimes[raptorIndex][arrivalStopId] = newArrivalTime;
                    journeyPointers[raptorIndex][arrivalStopId] = JourneyPointerRaptor{enterTripAtStop, stopId, currentTripDepartureTime, newArrivalTime, currentTripId};
                    arrivalStops.insert(arrivalStopId);
                }
            }
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

    auto startAddRoutes = chrono::high_resolution_clock::now();
    set<int> newRoutes = getNewRoutes(arrivalStops, routeId, raptorIndex);
    addRoutesToQueue(newRoutes, raptorIndex);
    auto endAddRoutes = chrono::high_resolution_clock::now();
    durationAddRoutesToQueue += chrono::duration_cast<chrono::microseconds>(endAddRoutes - startAddRoutes).count();
    updateCurrentBest(arrivalStops);

    firstStopSequencePerRoute[raptorIndex][routeId] = INT_MAX;
    lowestLowerBoundPerRoute[raptorIndex][routeId] = INT_MAX;
    auto end = chrono::high_resolution_clock::now();
    durationTraverseRoute += chrono::duration_cast<chrono::microseconds>(end - start).count();
}

set<int> RaptorPQParallel::getNewRoutes(set<int> stopIds, int excludeRouteId, int raptorIndex) {
    set<int> newRoutes;

    for (set<int>::iterator it = stopIds.begin(); it != stopIds.end(); it++) {
        int stopId = *it;
        double lowerBound = earliestArrivalTimes[raptorIndex][stopId] - queries[raptorIndex].sourceTime;
        double maxDist = 0;

        // calculate clique heuristic
        double heuristic = baseHeuristics[raptorIndex];
        for (int j = 0; j < sourceStopIds.size(); j++) {
            int s1 = sourceStopIds[j];
            if (s1 == queries[raptorIndex].sourceStopId) {
                continue;
            }
            heuristic += sourceStopIdsToAllStops[s1][stopId];
            if (sourceStopIdsToAllStops[s1][stopId] > maxDist) {
                maxDist = sourceStopIdsToAllStops[s1][stopId];
            }
        }
        heuristic = heuristic / (numberOfSourceStopIds - 1);

        
        if (optimization == min_sum || optimization == both) {
            lowerBound += heuristic;
        } else if (optimization == min_max) {
            double secondPart = (double) lowerBound + heuristic;
            secondPart = secondPart / numberOfSourceStopIds;

            double alternativeHeuristic = (double) lowerBound + maxDist;
            alternativeHeuristic = alternativeHeuristic / 2;

            if (alternativeHeuristic > lowerBound && alternativeHeuristic > secondPart) {
                altHeuristicImprovementCounter++;
                lowerBound = alternativeHeuristic;
            } else {
                lowerBound = max(lowerBound, secondPart);
                noHeuristicImprovementCounter++;
            }
        }

        if (lowerBound > currentBest) {
            continue;
        }

        vector<RouteSequencePair>* routes = &Importer::routesOfAStop[stopId];
        for (int j = 0; j < routes->size(); j++) {
            int routeId = (*routes)[j].routeId;
            if (routeId == excludeRouteId) {
                continue;
            }
            int stopSequence = (*routes)[j].stopSequence;

            if (lowerBound < lowestLowerBoundPerRoute[raptorIndex][routeId]) {
                lowestLowerBoundPerRoute[raptorIndex][routeId] = lowerBound;
                newRoutes.insert(routeId);
            }

            if (stopSequence < firstStopSequencePerRoute[raptorIndex][routeId]) {
                firstStopSequencePerRoute[raptorIndex][routeId] = stopSequence;
            }    
        }
    }

    return newRoutes;
}

void RaptorPQParallel::addRoutesToQueue(set<int> routes, int raptorIndex) {
    for (set<int>::iterator it = routes.begin(); it != routes.end(); it++) {
        PQEntry pqEntry = PQEntry(lowestLowerBoundPerRoute[raptorIndex][*it], *it, raptorIndex);
        pq.push(pqEntry);
    }
}

TripInfo RaptorPQParallel::getEarliestTripWithDayOffset(int routeId, int stopId, int stopSequence, int previousEarliestArrivalTime) {
    auto start = chrono::high_resolution_clock::now();
    vector<int>* trips = &Importer::tripsOfARoute[routeId];

    int earliestDepartureTime = previousEarliestArrivalTime;
    int dayOffset = TimeConverter::getDayOffset(earliestDepartureTime);
    int weekday = (queries[0].weekday + TimeConverter::getDayDifference(earliestDepartureTime)) % 7;

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
                auto end = chrono::high_resolution_clock::now();
                durationGetEarliestTripWithDayOffset += chrono::duration_cast<chrono::microseconds>(end - start).count();
                return tripInfo;
            }
        }
        dayOffset += SECONDS_PER_DAY;
        weekday = (weekday + 1) % 7;
    }

    auto end = chrono::high_resolution_clock::now();
    durationGetEarliestTripWithDayOffset += chrono::duration_cast<chrono::microseconds>(end - start).count();

    return {-1, -1, -1};
} 

void RaptorPQParallel::updateCurrentBest(set<int> stopIds) {
    for (set<int>::iterator it = stopIds.begin(); it != stopIds.end(); it++) {
        int stopId = *it;
        int sum = 0;
        int max = 0;
        for (int j = 0; j < earliestArrivalTimes.size(); j++) {
            if (earliestArrivalTimes[j][stopId] == INT_MAX) {
                sum = INT_MAX;
                break;
            }
            int duration = earliestArrivalTimes[j][stopId] - queries[j].sourceTime;
            sum += duration;
            if (duration > max) {
                max = duration;
            }
        }
        if (sum == INT_MAX) {
            continue;
        }
        
        if (optimization == min_sum || optimization == both) {
            if (sum < currentBest) {
                currentBest = sum;
            }
        } else if (optimization == min_max) {
            if (max < currentBest) {
                currentBest = max;
            }
        }
    }
}