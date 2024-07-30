#include "query-processor-raptor.h"

#include "raptor.h"
#include "journey.h"
#include "landmark-processor.h"
#include "../constants.h"
#include <../data-handling/importer.h>
#include <../data-handling/converter.h>
#include <../data-structures/creator.h>

#include <vector>
#include <chrono>
#include <memory>

using namespace std;

void RaptorQueryEATProcessor::processRaptorQuery() {
    auto start = chrono::high_resolution_clock::now();
    raptor = shared_ptr<Raptor>(new Raptor(raptorQuery));
    raptor->processRaptor();
    auto end = chrono::high_resolution_clock::now();
    raptorQueryResult.queryTime = chrono::duration_cast<chrono::milliseconds>(end - start).count();
    
    int earliestArrivalTime = raptor->getEarliestArrivalTime(raptorQuery.targetStopIds[0]);
    raptorQueryResult.arrivalTime = TimeConverter::convertSecondsToTime(earliestArrivalTime, true);
    raptorQueryResult.durationInSeconds = earliestArrivalTime - raptorQuery.sourceTime;

    numberOfExpandedRoutes = raptor->numberOfExpandedRoutes;
}

RaptorQueryResult RaptorQueryEATProcessor::getRaptorQueryResult() {
    return raptorQueryResult;
}

Journey RaptorQueryEATProcessor::getJourney() {
    Journey journey = raptor->createJourney(raptorQuery.targetStopIds[0]);
    return journey;
}

void RaptorStarBoundQueryProcessor::processRaptorStarBoundQuery() {
    auto start = chrono::high_resolution_clock::now();
    map<int, vector<int>> targetStopIdToAllStops;

    vector<int> landmarkIndices = vector<int>();

    if (!USE_LANDMARKS) {
        targetStopIdToAllStops = Creator::networkGraph.getDistancesWithPhast(raptorStarQuery.targetStopIds);
        landmarkIndices = LandmarkProcessor::getTopKLandmarks(NUMBER_OF_LANDMARKS, raptorStarQuery.sourceStopId, raptorStarQuery.targetStopIds[0]);
    }

    raptorBoundStar = shared_ptr<RaptorBoundStar> (new RaptorBoundStar(raptorStarQuery, targetStopIdToAllStops[raptorStarQuery.targetStopIds[0]], landmarkIndices));
    raptorBoundStar->processRaptorBoundStar();

    auto end = chrono::high_resolution_clock::now();

    raptorStarQueryResult.queryTime = chrono::duration_cast<chrono::milliseconds>(end - start).count();
    int earliestArrivalTime = raptorBoundStar->getEarliestArrivalTime(raptorStarQuery.targetStopIds[0]);
    raptorStarQueryResult.arrivalTime = TimeConverter::convertSecondsToTime(earliestArrivalTime, true);
    raptorStarQueryResult.durationInSeconds = earliestArrivalTime - raptorStarQuery.sourceTime;

    numberOfExpandedRoutes = raptorBoundStar->numberOfExpandedRoutes;

    lowerBoundSmallerCounter = raptorBoundStar->lowerBoundSmallerCounter;
    lowerBoundGreaterCounter = raptorBoundStar->lowerBoundGreaterCounter;
    lowerBoundSmallerFraction = (double) lowerBoundSmallerCounter / (lowerBoundSmallerCounter + lowerBoundGreaterCounter);
    lowerBoundAbsDiff = (double) raptorBoundStar->lowerBoundAbsDifference / lowerBoundSmallerCounter;
    lowerBoundRelDiff = (double) raptorBoundStar->lowerBoundRelDifference / lowerBoundSmallerCounter;
}

RaptorQueryResult RaptorStarBoundQueryProcessor::getRaptorStarQueryResult() {
    return raptorStarQueryResult;
}

Journey RaptorStarBoundQueryProcessor::getJourney() {
    Journey journey = raptorBoundStar->createJourney(raptorStarQuery.targetStopIds[0]);
    return journey;
}

void RaptorStarPQQueryProcessor::processRaptorStarPQQuery() {
    auto start = chrono::high_resolution_clock::now();
    map<int, vector<int>> targetStopIdToAllStops;

    vector<int> landmarkIndices = vector<int>();

    if (!USE_LANDMARKS) {
        targetStopIdToAllStops = Creator::networkGraph.getDistancesWithPhast(raptorStarQuery.targetStopIds);
        landmarkIndices = LandmarkProcessor::getTopKLandmarks(NUMBER_OF_LANDMARKS, raptorStarQuery.sourceStopId, raptorStarQuery.targetStopIds[0]);
    }

    raptorPQStar = shared_ptr<RaptorPQStar> (new RaptorPQStar(raptorStarQuery, targetStopIdToAllStops[raptorStarQuery.targetStopIds[0]], landmarkIndices));
    raptorPQStar->processRaptorPQStar();

    auto end = chrono::high_resolution_clock::now();

    raptorStarQueryResult.queryTime = chrono::duration_cast<chrono::milliseconds>(end - start).count();
    int earliestArrivalTime = raptorPQStar->getEarliestArrivalTime(raptorStarQuery.targetStopIds[0]);
    raptorStarQueryResult.arrivalTime = TimeConverter::convertSecondsToTime(earliestArrivalTime, true);
    raptorStarQueryResult.durationInSeconds = earliestArrivalTime - raptorStarQuery.sourceTime;

    numberOfExpandedRoutes = raptorPQStar->numberOfExpandedRoutes;

    lowerBoundSmallerCounter = raptorPQStar->lowerBoundSmallerCounter;
    lowerBoundGreaterCounter = raptorPQStar->lowerBoundGreaterCounter;
    lowerBoundSmallerFraction = (double) lowerBoundSmallerCounter / (lowerBoundSmallerCounter + lowerBoundGreaterCounter);
    lowerBoundAbsDiff = (double) raptorPQStar->lowerBoundAbsDifference / lowerBoundSmallerCounter;
    lowerBoundRelDiff = (double) raptorPQStar->lowerBoundRelDifference / lowerBoundSmallerCounter;
}

RaptorQueryResult RaptorStarPQQueryProcessor::getRaptorStarQueryResult() {
    return raptorStarQueryResult;
}

Journey RaptorStarPQQueryProcessor::getJourney() {
    Journey journey = raptorPQStar->createJourney(raptorStarQuery.targetStopIds[0]);
    return journey;
}