#include "query-processor.h"

#include "csa.h"
#include <../data-handling/importer.h>

#include <vector>
#include <string>

using namespace std;

vector<Journey> QueryProcessor::processNaiveQuery(string sourceStopName, string targetStopName, string sourceTime) {
    Query query = createQuery(sourceStopName, targetStopName, sourceTime);
    vector<Journey> journeys = CSA::processCSAQuery(query);
    return journeys;
}

Query QueryProcessor::createQuery(string sourceStopName, string targetStopName, string sourceTime) {
    Query query;
    return query;
}