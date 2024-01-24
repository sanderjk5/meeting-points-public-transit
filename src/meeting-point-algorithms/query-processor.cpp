#include "query-processor.h"

#include "csa.h"
#include <../data-handling/importer.h>

#include <vector>
#include <string>

using namespace std;

string QueryProcessor::processNaiveQuery(MeetingPointQuery meetingPointQuery) {
    string meetingPoint;
    return meetingPoint;
}

CSAQuery QueryProcessor::createCSAQuery(string sourceStopName, string sourceTime, string weekday) {
    CSAQuery query;
    return query;
}

CSAQuery QueryProcessor::createCSAQueryWithTargetStop(string sourceStopName, string targetStopName, string sourceTime, string weekday) {
    CSAQuery query;
    return query;
}