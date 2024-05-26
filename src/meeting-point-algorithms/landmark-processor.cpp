#include "landmark-processor.h"

#include "raptor.h"
#include "../constants.h"


#include <../data-handling/importer.h>

#include <chrono>
#include <iostream>
#include <sstream>
#include <fstream>
#include <vector>
#include <set>

using namespace std;

vector<vector<vector<int>>> LandmarkProcessor::landmarkDurations = vector<vector<vector<int>>>(0);

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

    landmarkDurations = vector<vector<vector<int>>>(14, vector<vector<int>>(numberOfLandmarks, vector<int>(Importer::stops.size(), 0)));

    for (int i = 0; i < numberOfLandmarks; i++) {
        for (int j = 0; j < Importer::stops.size(); j++) {
            getline(file, line);
            // Split the line into substrings
            std::stringstream ss(line);
            std::string substring;
            for (int k = 0; k < 14; k++) {
                std::getline(ss, substring, ',');
                landmarkDurations[k][i][j] = stoi(substring);
            }
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

    file << landmarkDurations[0].size() << endl;
    
    for (int i = 0; i < landmarkDurations[0].size(); i++) {
        for (int j = 0; j < landmarkDurations[0][i].size(); j++) {
            file << landmarkDurations[0][i][j] << "," << landmarkDurations[1][i][j] << "," << landmarkDurations[2][i][j] << "," << landmarkDurations[3][i][j] << "," << landmarkDurations[4][i][j] << "," << landmarkDurations[5][i][j] << "," << landmarkDurations[6][i][j];
            file << "," << landmarkDurations[7][i][j] << "," << landmarkDurations[8][i][j] << "," << landmarkDurations[9][i][j] << "," << landmarkDurations[10][i][j] << "," << landmarkDurations[11][i][j] << "," << landmarkDurations[12][i][j] << "," << landmarkDurations[13][i][j] << endl;
        }
    }

    file.close();
    cout << "Exported landmark durations.\n" << endl;
}

void LandmarkProcessor::calculateLandmarkDurations(DataType dataType) {
    auto start = std::chrono::high_resolution_clock::now(); 
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

    landmarkDurations = vector<vector<vector<int>>>(14, vector<vector<int>>(landmarkIds.size(), vector<int>(Importer::stops.size(), INT_MAX)));

    #pragma omp parallel for
    for (int weekday = 0; weekday < 7; weekday++) {
        for (int i = 0; i < landmarkIds.size(); i++) {
            for (int j = 0; j < 24; j++) {
                int landmarkId = landmarkIds[i];
                int sourceTime = j * 3600;

                RaptorBackwardQuery raptorBackwardQuery;
                raptorBackwardQuery.targetStopId = landmarkId;
                raptorBackwardQuery.sourceTime = sourceTime;
                raptorBackwardQuery.weekday = weekday;

                RaptorQuery raptorQuery;
                raptorQuery.sourceStopId = landmarkId;
                raptorQuery.sourceTime = sourceTime;
                raptorQuery.weekday = weekday;

                RaptorBackward raptorBackward = RaptorBackward(raptorBackwardQuery);
                raptorBackward.processRaptorBackward();

                Raptor raptor = Raptor(raptorQuery);
                raptor.processRaptor();

                int sourceTimeBackward = sourceTime + (NUMBER_OF_DAYS * SECONDS_PER_DAY);

                for (int j = 0; j < Importer::stops.size(); j++) {
                    int latestDepartureTime = raptorBackward.getLatestDepartureTime(j);
                    if (latestDepartureTime != -1) {
                        int duration = sourceTimeBackward - latestDepartureTime;
                        if (duration < landmarkDurations[weekday][i][j]) {
                            landmarkDurations[weekday][i][j] = duration;
                        }
                    }

                    int earliestArrivalTime = raptor.getEarliestArrivalTime(j);
                    if (earliestArrivalTime != INT_MAX) {
                        int duration = earliestArrivalTime - sourceTime;
                        if (duration < landmarkDurations[7+weekday][i][j]) {
                            landmarkDurations[7+weekday][i][j] = duration;
                        }
                    }
                }
            }
        }
    }
    

    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

    // Print the duration
    cout << "Calculated landmark durations in " << duration << " milliseconds." << endl;
}

int LandmarkProcessor::getLowerBound(int stopId1, int stopId2, int weekday) {
    int lowerBound = 0;

    for (int i = 0; i < landmarkDurations[weekday].size(); i++) {
        int duration1 = landmarkDurations[weekday][i][stopId1];
        int duration2 = landmarkDurations[weekday][i][stopId2];
        if (duration1 != INT_MAX && duration2 != INT_MAX) {
            int duration = duration1 - duration2;
            if (duration > lowerBound) {
                lowerBound = duration;
            }
        }

        duration1 = landmarkDurations[7+weekday][i][stopId2];
        duration2 = landmarkDurations[7+weekday][i][stopId1];
        if (duration1 != INT_MAX && duration2 != INT_MAX) {
            int duration = duration1 - duration2;
            if (duration > lowerBound) {
                lowerBound = duration;
            }
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

