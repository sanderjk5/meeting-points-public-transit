#include "lower-bound-tester.h"

#include <iostream>
#include <vector>
#include <memory>

#include "csa.h"
#include "../data-structures/graph.h"
#include "algorithm-tester.h"
#include "landmark-processor.h"
#include "../data-structures/creator.h"
#include "../constants.h"

using namespace std;

void LowerBoundTester::getLowerBoundDiffs(int numberOfSources, int numberOfTargetsPerSource) {
    vector<double> absoluteDiffs = vector<double>();
    vector<double> relativeDiffs = vector<double>();
    int lowerBoundGreaterCounter = 0;

    vector<double> absoluteDiffsLandmarks = vector<double>();
    vector<double> relativeDiffsLandmarks = vector<double>();
    int lowerBoundGreaterCounterLandmarks = 0;

    vector<int> sources = vector<int>();

    for (int i = 0; i < numberOfSources; i++) {
        sources.push_back(Importer::connections[rand() % Importer::connections.size()].departureStopId);
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

        unique_ptr<CSA> csa = unique_ptr<CSA>(new CSA(csaQuery));
        csa->processCSA();

        for (int j = 0; j < numberOfTargetsPerSource; j++) {
            int earliestArrivalTime = csa->getEarliestArrivalTime(targets[j]);
            if (earliestArrivalTime == INT_MAX) {
                continue;
            }

            int duration = earliestArrivalTime - csaQuery.sourceTime;
            int lowerBound = sourceStopIdToAllStops[csaQuery.sourceStopId][targets[j]];

            double absoluteDiff = (double) (duration - lowerBound);
            double relativeDiff = 1;
            if (duration > 0) {
                relativeDiff = absoluteDiff / (double) duration;
                if (absoluteDiff < 0) {
                    relativeDiff = 0;
                    lowerBoundGreaterCounter++;
                }
            }
            absoluteDiffs.push_back(absoluteDiff);
            relativeDiffs.push_back(relativeDiff);

            // Landmarks
            double lowerBoundLandmark = LandmarkProcessor::getLowerBound(sources[i], targets[j], csaQuery.weekday);
            double absoluteDiffLandmark = (double) (duration - lowerBoundLandmark);
            double relativeDiffLandmark = 1;
            if (duration > 0) {
                relativeDiffLandmark = absoluteDiffLandmark / (double) duration;
                if (absoluteDiffLandmark < 0) {
                    relativeDiffLandmark = 0;
                    lowerBoundGreaterCounterLandmarks++;
                }
            }
            absoluteDiffsLandmarks.push_back(absoluteDiffLandmark);
            relativeDiffsLandmarks.push_back(relativeDiffLandmark);
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

    double avgAbsoluteDiffLandmarks = (Calculator::getAverage(absoluteDiffsLandmarks) / 3600);
    double avgRelativeDiffLandmarks = Calculator::getAverage(relativeDiffsLandmarks);
    double medianAbsoluteDiffLandmarks = (Calculator::getMedian(absoluteDiffsLandmarks) / 3600);
    double medianRelativeDiffLandmarks = Calculator::getMedian(relativeDiffsLandmarks);
    double maxAbsoluteDiffLandmarks = (Calculator::getMaximum(absoluteDiffsLandmarks) / 3600);
    double maxRelativeDiffLandmarks = Calculator::getMaximum(relativeDiffsLandmarks);
    double minAbsoluteDiffLandmarks = (Calculator::getMinimum(absoluteDiffsLandmarks) / 3600);
    double minRelativeDiffLandmarks = Calculator::getMinimum(relativeDiffsLandmarks);

    double lowerBoundGreaterFractionLandmarks = (double) lowerBoundGreaterCounterLandmarks / absoluteDiffsLandmarks.size();

    cout << "\nAverage absolute diff (in hours): " << avgAbsoluteDiff << endl;
    cout << "Average relative diff: " << avgRelativeDiff << endl;
    cout << "Median absolute diff (in hours): " << medianAbsoluteDiff << endl;
    cout << "Median relative diff: " << medianRelativeDiff << endl;
    cout << "Max absolute diff (in hours): " << maxAbsoluteDiff << endl;
    cout << "Max relative diff: " << maxRelativeDiff << endl;
    cout << "Min absolute diff (in hours): " << minAbsoluteDiff << endl;
    cout << "Min relative diff: " << minRelativeDiff << endl;

    cout << "Lower bound greater fraction: " << lowerBoundGreaterFraction << endl;

    cout << "\nAverage absolute diff landmarks (in hours): " << avgAbsoluteDiffLandmarks << endl;
    cout << "Average relative diff landmarks: " << avgRelativeDiffLandmarks << endl;
    cout << "Median absolute diff landmarks (in hours): " << medianAbsoluteDiffLandmarks << endl;
    cout << "Median relative diff landmarks: " << medianRelativeDiffLandmarks << endl;
    cout << "Max absolute diff landmarks (in hours): " << maxAbsoluteDiffLandmarks << endl;
    cout << "Max relative diff landmarks: " << maxRelativeDiffLandmarks << endl;
    cout << "Min absolute diff landmarks (in hours): " << minAbsoluteDiffLandmarks << endl;
    cout << "Min relative diff landmarks: " << minRelativeDiffLandmarks << endl;

    cout << "Lower bound greater fraction landmarks: " << lowerBoundGreaterFractionLandmarks << endl;
}