#include "algorithm-tester-raptor.h"

#include "query-processor-raptor.h"
#include "raptor.h"
#include "journey.h"
#include <../data-handling/importer.h>
#include <../data-handling/converter.h>
#include <../constants.h>


#include <vector>
#include <memory>
#include <iostream>

using namespace std;

void RaptorEATAlgorithmTester::testRaptorAlgorithms(RaptorQuery raptorQuery) {
    unique_ptr<RaptorQueryEATProcessor> raptorQueryProcessor = unique_ptr<RaptorQueryEATProcessor>(new RaptorQueryEATProcessor(raptorQuery));
    raptorQueryProcessor->processRaptorQuery();
    RaptorQueryResult raptorQueryResult = raptorQueryProcessor->getRaptorQueryResult();

    unique_ptr<RaptorStarBoundQueryProcessor> raptorStarBoundQueryProcessor = unique_ptr<RaptorStarBoundQueryProcessor>(new RaptorStarBoundQueryProcessor(raptorQuery));
    raptorStarBoundQueryProcessor->processRaptorStarBoundQuery();
    RaptorQueryResult raptorStarBoundQueryResult = raptorStarBoundQueryProcessor->getRaptorStarQueryResult();

    unique_ptr<RaptorStarPQQueryProcessor> raptorStarPQQueryProcessor = unique_ptr<RaptorStarPQQueryProcessor>(new RaptorStarPQQueryProcessor(raptorQuery));
    raptorStarPQQueryProcessor->processRaptorStarPQQuery();
    RaptorQueryResult raptorStarPQQueryResult = raptorStarPQQueryProcessor->getRaptorStarQueryResult();

    PrintHelperRaptor::printRaptorQuery(raptorQuery);
    cout << "\nRaptor query result: " << endl;
    PrintHelperRaptor::printRaptorQueryResult(raptorQueryResult);
    cout << "Number of expanded routes: " << raptorQueryProcessor->numberOfExpandedRoutes << endl;
    cout << "\nRaptor star bound query result: " << endl;
    PrintHelperRaptor::printRaptorQueryResult(raptorStarBoundQueryResult);
    cout << "Number of expanded routes: " << raptorStarBoundQueryProcessor->numberOfExpandedRoutes << endl;
    cout << "\nRaptor star PQ query result: " << endl;
    PrintHelperRaptor::printRaptorQueryResult(raptorStarPQQueryResult);
    cout << "Number of expanded routes: " << raptorStarPQQueryProcessor->numberOfExpandedRoutes << endl;
}

void PrintHelperRaptor::printRaptorQuery(RaptorQuery raptorQuery) {
    cout << "\nRaptor query: " << endl;
    cout << "Source stop: " << Importer::getStopName(raptorQuery.sourceStopId) << endl;
    cout << "Target stop: " << Importer::getStopName(raptorQuery.targetStopIds[0]) << endl;
    cout << "Source time: " << TimeConverter::convertSecondsToTime(raptorQuery.sourceTime, true) << endl;
    cout << "Weekday: " << WeekdayConverter::convertIntToWeekday(raptorQuery.weekday) << endl;
}

void PrintHelperRaptor::printRaptorQueryResult(RaptorQueryResult raptorQueryResult) {
    if (raptorQueryResult.arrivalTime != ""){
        cout << "Arrival time: " << raptorQueryResult.arrivalTime << endl;
        cout << "Duration: " << TimeConverter::convertSecondsToTime(raptorQueryResult.durationInSeconds, false) << endl;
    } else {
        cout << "No connection found" << endl;
    }
    cout << "Query time: " << raptorQueryResult.queryTime << " ms" << endl;
}