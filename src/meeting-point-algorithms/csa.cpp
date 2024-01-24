#include "csa.h"

#include "query-processor.h"
#include <../data-handling/importer.h>


#include <vector>

using namespace std;


void CSA::processCSA() {
    // TODO: Implement CSA algorithm
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