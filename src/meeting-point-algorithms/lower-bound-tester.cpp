#include "lower-bound-tester.h"

#include <iostream>
#include <vector>

#include "csa.h"
#include "../data-structures/graph.h"
#include "algorithm-tester.h"
#include "../data-structures/creator.h"
#include "../constants.h"

using namespace std;

void LowerBoundTester::getLowerBoundDiffs(int numberOfSources, int numberOfTargetsPerSource) {
    vector<double> absoluteDiffs = vector<double>();
    vector<double> relativeDiffs = vector<double>();
    int lowerBoundGreaterCounter = 0;

    vector<int> sources = vector<int>();

    for (int i = 0; i < numberOfSources; i++) {
        sources.push_back(rand() % Creator::networkGraph.vertices.size());
    }

    map<int, vector<int>> sourceStopIdToAllStops = Creator::networkGraph.getDistancesWithPhast(sources);

    for (int i = 0; i < numberOfSources; i++) {
        vector<int> targets = vector<int>();

        for (int j = 0; j < numberOfTargetsPerSource; j++) {
            targets.push_back(rand() % Creator::networkGraph.vertices.size());
        }

        CSAQuery csaQuery;
        csaQuery.sourceStopId = sources[i];
        csaQuery.targetStopIds = targets;
        csaQuery.sourceTime = rand() % SECONDS_PER_DAY;
        csaQuery.weekday = rand() % 7;

        CSA csa = CSA(csaQuery);
        csa.processCSA();

        for (int j = 0; j < numberOfTargetsPerSource; j++) {
            int earliestArrivalTime = csa.getEarliestArrivalTime(targets[j]);
            if (earliestArrivalTime == INT_MAX) {
                continue;
            }

            int duration = earliestArrivalTime - csaQuery.sourceTime;
            int lowerBound = sourceStopIdToAllStops[csaQuery.sourceStopId][targets[j]];

            double absoluteDiff = (double) (duration - lowerBound);
            double relativeDiff = absoluteDiff / (double) duration;
            if (absoluteDiff < 0) {
                relativeDiff = 0;
                lowerBoundGreaterCounter++;
            }
            absoluteDiffs.push_back(absoluteDiff);
            relativeDiffs.push_back(relativeDiff);
        }

        // print the progress
        if (i % (numberOfSources / 5) == 0) {
            cout << "Progress: " << i << " / " << numberOfSources << endl;
        }
    }

    double avgAbsoluteDiff = (Calculator::getAverage(absoluteDiffs) / 3600);
    double avgRelativeDiff = Calculator::getAverage(relativeDiffs);
    double medianAbsoluteDiff = (Calculator::getMedian(absoluteDiffs) / 3600);
    double medianRelativeDiff = Calculator::getMedian(relativeDiffs);
    double maxAbsoluteDiff = (Calculator::getMaximum(absoluteDiffs) / 3600);
    double maxRelativeDiff = Calculator::getMaximum(relativeDiffs);
    double minAbsoluteDiff = (Calculator::getMinimum(absoluteDiffs) / 3600);
    double minRelativeDiff = Calculator::getMinimum(relativeDiffs);

    double lowerBoundGreaterFraction = (double) lowerBoundGreaterCounter / absoluteDiffs.size();

    cout << "Average absolute diff (in hours): " << avgAbsoluteDiff << endl;
    cout << "Average relative diff: " << avgRelativeDiff << endl;
    cout << "Median absolute diff (in hours): " << medianAbsoluteDiff << endl;
    cout << "Median relative diff: " << medianRelativeDiff << endl;
    cout << "Max absolute diff (in hours): " << maxAbsoluteDiff << endl;
    cout << "Max relative diff: " << maxRelativeDiff << endl;
    cout << "Min absolute diff (in hours): " << minAbsoluteDiff << endl;
    cout << "Min relative diff: " << minRelativeDiff << endl;

    cout << "Lower bound greater fraction: " << lowerBoundGreaterFraction << endl;
}