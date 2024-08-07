#ifndef CMAKE_LANDMARK_PROCESSOR_H
#define CMAKE_LANDMARK_PROCESSOR_H

#include "../data-handling/importer.h"

#include <vector>
#include <set>


class LandmarkProcessor {
    public:
        explicit LandmarkProcessor(){};
        ~LandmarkProcessor(){};

    	static void loadOrCalculateLandmarkDurations(DataType dataType, bool importDurations);
        static int getLowerBound(int stopId1, int stopId2, int weekday);

        static vector<set<int>> getAllArrivalTimesOfStop(int stopId);
        static vector<set<int>> getAllDepartureTimesOfStop(int stopId);

        static void countAllArrivalAndDepartureTimesOfTheLandmarks(DataType dataType);

        static int getLowerBoundUsingLandmarks(int stopId1, int stopId2, vector<int> landmarkIndices);

        static vector<int> getTopKLandmarks(int k, int stopId1, int stopId2);
        static int getClosestLandmark(int stopId);
        static void findAndCalculateLandmarks(DataType dataType, int numberOfLandmarks);

    private:
        static void importLandmarkDurations(DataType dataType);
        static void exportLandmarkDurations(DataType dataType, int id);
        static void calculateLandmarkDurations(DataType dataType);  
        static void calculateExactLandmarkDurationsForStops(vector<int> landmarkIds);

        static vector<int> getLandmarkIds(DataType dataType, int lowerIndex, int upperIndex);

        static vector<vector<int>> landmarkDurations;
};

#endif //CMAKE_LANDMARK_PROCESSOR_H