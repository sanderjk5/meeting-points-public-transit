#include "landmark-processor.h"

#include "raptor.h"
#include "../constants.h"


#include <../data-handling/importer.h>


#include <iostream>
#include <sstream>
#include <fstream>
#include <vector>
#include <set>

using namespace std;

vector<vector<int>> LandmarkProcessor::landmarkDurations = vector<vector<int>>(0);

void LandmarkProcessor::loadOrCalculateLandmarkDurations(DataType dataType) {
    string dataTypeString = Importer::getDataTypeString(dataType);
    string folderPathData = FOLDER_PREFIX + "graphs/" + dataTypeString + "/";
    string filePath = folderPathData + "landmark_durations.txt";

    ifstream file(filePath);

    if (file.is_open()) {
        file.close();
        importLandmarkDurations(dataType);
    } else {
        calculateLandmarkDurations(dataType);
        exportLandmarkDurations(dataType);
    }
}

void LandmarkProcessor::importLandmarkDurations(DataType dataType) {
    cout << "Importing landmark durations..." << endl;
    string dataTypeString = Importer::getDataTypeString(dataType);
    string folderPathData = FOLDER_PREFIX + "graphs/" + dataTypeString + "/";
    string filePath = folderPathData + "landmark_durations.txt";

    ifstream file(filePath);

    if (!file.is_open()) {
        cout << "Error: Could not open file " << filePath << endl;
        return;
    }
    
    string line;
    getline(file, line);

    stringstream ss(line);
    string nofLandmarksString;
    ss >> nofLandmarksString;
    int numberOfLandmarks = stoi(nofLandmarksString);

    landmarkDurations = vector<vector<int>>(numberOfLandmarks, vector<int>(Importer::stops.size(), 0));

    for (int i = 0; i < numberOfLandmarks; i++) {
        for (int j = 0; j < Importer::stops.size(); j++) {
            getline(file, line);
            string durationString;
            stringstream ss(line);
            ss >> durationString;
            landmarkDurations[i][j] = stoi(durationString);
        }
    }

    file.close();
    cout << "Imported landmark durations.\n" << endl;
}

void LandmarkProcessor::exportLandmarkDurations(DataType dataType) {
    cout << "Exporting landmark durations..." << endl;
    string dataTypeString = Importer::getDataTypeString(dataType);
    string folderPathData = FOLDER_PREFIX + "graphs/" + dataTypeString + "/";
    string filePath = folderPathData + "landmark_durations.txt";

    ofstream file(filePath);

    if (!file.is_open()) {
        cout << "Error: Could not open file " << filePath << endl;
        return;
    }

    file << landmarkDurations.size() << endl;

    for (int i = 0; i < landmarkDurations.size(); i++) {
        for (int j = 0; j < landmarkDurations[i].size(); j++) {
            file << landmarkDurations[i][j] << endl;
        }
    }

    file.close();
    cout << "Exported landmark durations.\n" << endl;
}

void LandmarkProcessor::calculateLandmarkDurations(DataType dataType) {
    cout << "Calculating landmark durations..." << endl;

    // load landmark ids from file
    vector<int> landmarkIds;

    string dataTypeString = Importer::getDataTypeString(dataType);
    string folderPathData = FOLDER_PREFIX + "graphs/" + dataTypeString + "/";
    string filePath = folderPathData + "landmarks.txt";

    ifstream file(filePath);

    if (file.is_open()) {
        std::string line;
        while (std::getline(file, line)) {
            vector<string> parts;

            // Split the line into substrings
            std::stringstream ss(line);
            std::string substring;
            while (std::getline(ss, substring, ',')) {
                parts.push_back(substring);
            }

            landmarkIds.push_back(stoi(parts[0]));
        }
    } else {
        cout << "Error: Could not open file " << filePath << endl;
        return;
    }

    landmarkDurations = vector<vector<int>>(landmarkIds.size(), vector<int>(Importer::stops.size(), INT_MAX));

    #pragma omp parallel for
    for (int i = 0; i < landmarkIds.size(); i++) {
        for (int j = 0; j < 10; j++) {
            RaptorBackwardQuery raptorBackwardQuery;
            raptorBackwardQuery.targetStopId = landmarkIds[i];
            raptorBackwardQuery.sourceTime = rand() % SECONDS_PER_DAY;
            raptorBackwardQuery.weekday = rand() % 7;

            RaptorBackward raptorBackward = RaptorBackward(raptorBackwardQuery);
            raptorBackward.processRaptorBackward();

            int sourceTime = raptorBackwardQuery.sourceTime + (NUMBER_OF_DAYS * SECONDS_PER_DAY);

            for (int j = 0; j < Importer::stops.size(); j++) {
                int latestDepartureTime = raptorBackward.getLatestDepartureTime(j);
                if (latestDepartureTime == -1) {
                    continue;
                }
                int duration = sourceTime - latestDepartureTime;
                if (duration < landmarkDurations[i][j]) {
                    landmarkDurations[i][j] = duration;
                }
            }
        }
        
    }

    cout << "Calculated landmark durations." << endl;
}

int LandmarkProcessor::getLowerBound(int stopId1, int stopId2) {
    int lowerBound = 0;

    for (int i = 0; i < landmarkDurations.size(); i++) {
        int duration1 = landmarkDurations[i][stopId1];
        int duration2 = landmarkDurations[i][stopId2];
        if (duration1 == INT_MAX || duration2 == INT_MAX) {
            continue;
        }

        int duration = duration1 - duration2;
        if (duration < 0) {
            duration = -duration;
        }
        if (duration > lowerBound) {
            lowerBound = duration;
        }
    }

    return lowerBound;
}

vector<set<int>> LandmarkProcessor::getAllArrivalTimesOfStop(int stopId) {
    vector<set<int>> arrivalTimesPerWeekday = vector<set<int>>(7, set<int>());
    vector<int> reachableStopIds = vector<int>(0);

    int indexOfFirstFootpathOfTargetStop = Importer::indexOfFirstFootPathOfAStopBackward[stopId];
    for (int i = indexOfFirstFootpathOfTargetStop; i < Importer::footPathsBackward.size(); i++) {
        if (Importer::footPathsBackward[i].arrivalStopId != stopId) {
            break;
        }
        reachableStopIds.push_back(Importer::footPathsBackward[i].departureStopId);
    }

    for (int i = 0; i < reachableStopIds.size(); i++) {
        vector<RouteSequencePair>* routes = &Importer::routesOfAStop[stopId];
        for (int j = 0; j < routes->size(); j++) {
            int routeId = (*routes)[j].routeId;
            int stopSequence = (*routes)[j].stopSequence;
            vector<int>* trips = &Importer::tripsOfARoute[routeId];
            for (int k = 0; k < trips->size(); k++) {
                int tripId = (*trips)[k];
                StopTime stopTime = Importer::stopTimes[Importer::indexOfFirstStopTimeOfATrip[tripId] + stopSequence];
                for (int weekday = 0; weekday < 7; weekday++) {
                    if (Importer::isTripAvailable(tripId, weekday)) {
                        arrivalTimesPerWeekday[weekday].insert(stopTime.arrivalTime);
                    }
                }
            }
        }
    }

    for (int weekday = 0; weekday < 7; weekday++) {
        cout << "Weekday: " << weekday << ", nof arrival times: " << arrivalTimesPerWeekday[weekday].size() << endl;
    }

    return arrivalTimesPerWeekday;
}

