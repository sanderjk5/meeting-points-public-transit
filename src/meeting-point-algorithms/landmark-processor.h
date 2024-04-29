#ifndef CMAKE_LANDMARK_PROCESSOR_H
#define CMAKE_LANDMARK_PROCESSOR_H

#include "../data-handling/importer.h"

#include <vector>
#include <set>


class LandmarkProcessor {
    public:
        explicit LandmarkProcessor(){};
        ~LandmarkProcessor(){};

    	static void loadOrCalculateLandmarkDurations(DataType dataType);
        static int getLowerBound(int stopId1, int stopId2);

        static vector<set<int>> getAllArrivalTimesOfStop(int stopId);
        

    private:
        static void importLandmarkDurations(DataType dataType);
        static void exportLandmarkDurations(DataType dataType);
        static void calculateLandmarkDurations(DataType dataType);  

        static vector<vector<int>> landmarkDurations;
};

#endif //CMAKE_LANDMARK_PROCESSOR_H