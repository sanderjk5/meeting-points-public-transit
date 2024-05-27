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
#include <memory>

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
        // calculateLandmarkDurations(dataType);

        vector<int> landmarkIds = getLandmarkIds(dataType);
        calculateExactLandmarkDurationsForStops(landmarkIds);

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

    landmarkDurations = vector<vector<vector<int>>>(2, vector<vector<int>>(numberOfLandmarks, vector<int>(Importer::stops.size(), 0)));

    for (int i = 0; i < numberOfLandmarks; i++) {
        for (int j = 0; j < Importer::stops.size(); j++) {
            getline(file, line);
            // Split the line into substrings
            std::stringstream ss(line);
            std::string substring;
            for (int k = 0; k < 2; k++) {
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
            file << landmarkDurations[0][i][j] << "," << landmarkDurations[1][i][j] << endl;
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

    landmarkDurations = vector<vector<vector<int>>>(2, vector<vector<int>>(landmarkIds.size(), vector<int>(Importer::stops.size(), INT_MAX)));

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
                        if (duration < landmarkDurations[0][i][j]) {
                            landmarkDurations[0][i][j] = duration;
                        }
                    }

                    int earliestArrivalTime = raptor.getEarliestArrivalTime(j);
                    if (earliestArrivalTime != INT_MAX) {
                        int duration = earliestArrivalTime - sourceTime;
                        if (duration < landmarkDurations[1][i][j]) {
                            landmarkDurations[1][i][j] = duration;
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

void LandmarkProcessor::calculateExactLandmarkDurationsForStops(vector<int> landmarkIds) {
    auto start = std::chrono::high_resolution_clock::now(); 
    cout << "Calculating exact landmark durations for stops..." << endl;

    landmarkDurations = vector<vector<vector<int>>>(2, vector<vector<int>>(landmarkIds.size(), vector<int>(Importer::stops.size(), INT_MAX)));

    #pragma omp parallel for
    for (int i = 0; i < landmarkIds.size(); i++) {
        vector<set<int>> arrivalTimes = getAllArrivalTimesOfStop(landmarkIds[i]);

        for (int j = 0; j < arrivalTimes.size(); j++) {
            // for (int k = 0; k < arrivalTimes[j].size(); k++) {
            //     int index = k;

            for (int k = 0; k < 24; k++) {
                int index = k * (arrivalTimes[j].size() / 24);

                int sourceTime = *next(arrivalTimes[j].begin(), k);
                RaptorBackwardQuery raptorBackwardQuery;
                raptorBackwardQuery.targetStopId = landmarkIds[i];
                raptorBackwardQuery.sourceTime = sourceTime;
                raptorBackwardQuery.weekday = j;

                unique_ptr<RaptorBackward> raptorBackward = unique_ptr<RaptorBackward>(new RaptorBackward(raptorBackwardQuery));
                raptorBackward->processRaptorBackward();

                sourceTime = sourceTime + (NUMBER_OF_DAYS * SECONDS_PER_DAY);

                for (int l = 0; l < Importer::stops.size(); l++) {
                    int latestDepartureTime = raptorBackward->getLatestDepartureTime(l);
                    if (latestDepartureTime != -1) {
                        int duration = sourceTime - latestDepartureTime;
                        if (duration < landmarkDurations[0][i][l]) {
                            landmarkDurations[0][i][l] = duration;
                        }
                    }
                }
            }
        }

        vector<set<int>> departureTimes = getAllDepartureTimesOfStop(landmarkIds[i]);

        for (int j = 0; j < departureTimes.size(); j++) {
            // for (int k = 0; k < departureTimes[j].size(); k++) {
            //     int index = k;

            for (int k = 0; k < 24; k++) {
                int index = k * (departureTimes[j].size() / 24);

                int sourceTime = *next(departureTimes[j].begin(), index);
                RaptorQuery raptorQuery;
                raptorQuery.sourceStopId = landmarkIds[i];
                raptorQuery.sourceTime = sourceTime;
                raptorQuery.weekday = j;

                unique_ptr<Raptor> raptor = unique_ptr<Raptor>(new Raptor(raptorQuery));
                raptor->processRaptor();

                for (int l = 0; l < Importer::stops.size(); l++) {
                    int earliestArrivalTime = raptor->getEarliestArrivalTime(l);
                    if (earliestArrivalTime != INT_MAX) {
                        int duration = earliestArrivalTime - sourceTime;
                        if (duration < landmarkDurations[1][i][l]) {
                            landmarkDurations[1][i][l] = duration;
                        }
                    }
                }
            }
        }
    }

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

    // Print the duration
    cout << "Calculated exact landmark durations for stops in " << duration << " milliseconds." << endl;
}

vector<int> LandmarkProcessor::getLandmarkIds(DataType dataType) {
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
    }

    return landmarkIds;
}
        

int LandmarkProcessor::getLowerBound(int stopId1, int stopId2, int weekday) {
    int lowerBound = 0;

    for (int i = 0; i < landmarkDurations[0].size(); i++) {
        int duration1 = landmarkDurations[0][i][stopId1];
        int duration2 = landmarkDurations[0][i][stopId2];
        if (duration1 != INT_MAX && duration2 != INT_MAX) {
            int duration = duration1 - duration2;
            if (duration > lowerBound) {
                lowerBound = duration;
            }
        }

        duration1 = landmarkDurations[1][i][stopId2];
        duration2 = landmarkDurations[1][i][stopId1];
        if (duration1 != INT_MAX && duration2 != INT_MAX) {
            int duration = duration1 - duration2;
            if (duration > lowerBound) {
                lowerBound = duration;
            }
        }
    }

    return lowerBound;
}

vector<set<int>> LandmarkProcessor::getAllDepartureTimesOfStop(int stopId) {
    vector<set<int>> departureTimesPerWeekday = vector<set<int>>(7, set<int>());
    vector<pair<int, int>> reachableStopIdsAndFootpathDurations = vector<pair<int, int>>(0);

    int indexOfFirstFootpathOfTargetStop = Importer::indexOfFirstFootPathOfAStop[stopId];
    for (int i = indexOfFirstFootpathOfTargetStop; i < Importer::footPaths.size(); i++) {
        if (Importer::footPaths[i].departureStopId != stopId) {
            break;
        }
        reachableStopIdsAndFootpathDurations.push_back(make_pair(Importer::footPaths[i].arrivalStopId, Importer::footPaths[i].duration));
    }

    for (int i = 0; i < reachableStopIdsAndFootpathDurations.size(); i++) {
        int currentStopId = reachableStopIdsAndFootpathDurations[i].first;
        int footpathDuration = reachableStopIdsAndFootpathDurations[i].second;
        vector<RouteSequencePair>* routes = &Importer::routesOfAStop[currentStopId];
        for (int j = 0; j < routes->size(); j++) {
            int routeId = (*routes)[j].routeId;
            int stopSequence = (*routes)[j].stopSequence;
            vector<int>* trips = &Importer::tripsOfARoute[routeId];
            for (int k = 0; k < trips->size(); k++) {
                int tripId = (*trips)[k];
                StopTime stopTime = Importer::stopTimes[Importer::indexOfFirstStopTimeOfATrip[tripId] + stopSequence];
                int departureTime = stopTime.departureTime - footpathDuration;

                bool decreaseWeekday = false;
                StopTime firstStopTimeOfTrip = Importer::stopTimes[Importer::indexOfFirstStopTimeOfATrip[tripId]];
                if (firstStopTimeOfTrip.departureTime > stopTime.departureTime) {
                    decreaseWeekday = true;
                }

                for (int weekday = 0; weekday < 7; weekday++) {
                    int weekdayOfTrip = weekday;
                    if (decreaseWeekday) {
                        weekdayOfTrip = (weekday + 6) % 7;
                    }
                    if (Importer::isTripAvailable(tripId, weekdayOfTrip)) {
                        if (departureTime >= 0) {
                            departureTimesPerWeekday[weekday].insert(departureTime);
                        } else {
                            int previousDay = (weekday + 6) % 7;
                            departureTime = departureTime + SECONDS_PER_DAY;
                            departureTimesPerWeekday[previousDay].insert(departureTime);
                        }
                    }
                }
            }
        }
    }

    return departureTimesPerWeekday;
}

vector<set<int>> LandmarkProcessor::getAllArrivalTimesOfStop(int stopId) {
    vector<set<int>> arrivalTimesPerWeekday = vector<set<int>>(7, set<int>());
    vector<pair<int, int>> reachableStopIdsAndFootpathDurations = vector<pair<int, int>>(0);

    int indexOfFirstFootpathOfTargetStop = Importer::indexOfFirstFootPathOfAStopBackward[stopId];
    for (int i = indexOfFirstFootpathOfTargetStop; i < Importer::footPathsBackward.size(); i++) {
        if (Importer::footPathsBackward[i].arrivalStopId != stopId) {
            break;
        }
        reachableStopIdsAndFootpathDurations.push_back(make_pair(Importer::footPathsBackward[i].departureStopId, Importer::footPathsBackward[i].duration));
    }

    for (int i = 0; i < reachableStopIdsAndFootpathDurations.size(); i++) {
        int currentStopId = reachableStopIdsAndFootpathDurations[i].first;
        int footpathDuration = reachableStopIdsAndFootpathDurations[i].second;
        vector<RouteSequencePair>* routes = &Importer::routesOfAStop[currentStopId];
        for (int j = 0; j < routes->size(); j++) {
            int routeId = (*routes)[j].routeId;
            int stopSequence = (*routes)[j].stopSequence;
            vector<int>* trips = &Importer::tripsOfARoute[routeId];
            for (int k = 0; k < trips->size(); k++) {
                int tripId = (*trips)[k];
                StopTime stopTime = Importer::stopTimes[Importer::indexOfFirstStopTimeOfATrip[tripId] + stopSequence];
                int arrivalTime = stopTime.arrivalTime + footpathDuration;

                bool decreaseWeekday = false;
                StopTime firstStopTimeOfTrip = Importer::stopTimes[Importer::indexOfFirstStopTimeOfATrip[tripId]];
                if (firstStopTimeOfTrip.departureTime > stopTime.departureTime) {
                    decreaseWeekday = true;
                }

                for (int weekday = 0; weekday < 7; weekday++) {
                    int weekdayOfTrip = weekday;
                    if (decreaseWeekday) {
                        weekdayOfTrip = (weekday + 6) % 7;
                    }
                    if (Importer::isTripAvailable(tripId, weekdayOfTrip)) {
                        if (arrivalTime < SECONDS_PER_DAY) {
                            arrivalTimesPerWeekday[weekday].insert(arrivalTime);
                        } else {
                            int nextDay = (weekday + 1) % 7;
                            arrivalTime = arrivalTime - SECONDS_PER_DAY;
                            arrivalTimesPerWeekday[nextDay].insert(arrivalTime);   
                        }
                    }
                }
            }
        }
    }

    return arrivalTimesPerWeekday;
}

void LandmarkProcessor::countAllArrivalAndDepartureTimesOfTheLandmarks(DataType dataType) {
    vector<int> arrivalTimesPerWeekday = vector<int>(7, 0);
    vector<int> departureTimesPerWeekday = vector<int>(7, 0);

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

    int totalArrivalTimes = 0;
    int totalDepartureTimes = 0;
    for (int i = 0; i < landmarkIds.size(); i++) {
        int landmarkId = landmarkIds[i];
        vector<set<int>> arrivalTimes = getAllArrivalTimesOfStop(landmarkId);
        vector<set<int>> departureTimes = getAllDepartureTimesOfStop(landmarkId);

        for (int j = 0; j < 7; j++) {
            arrivalTimesPerWeekday[j] += arrivalTimes[j].size();
            totalArrivalTimes += arrivalTimes[j].size();
            departureTimesPerWeekday[j] += departureTimes[j].size();
            totalDepartureTimes += departureTimes[j].size();
        }
    }

    for (int i = 0; i < 7; i++) {
        cout << "Arrival times on weekday " << i << ": " << arrivalTimesPerWeekday[i] << endl;
        cout << "Departure times on weekday " << i << ": " << departureTimesPerWeekday[i] << endl;
    }
    cout << "Total arrival times: " << totalArrivalTimes << endl;
    cout << "Total departure times: " << totalDepartureTimes << endl;
}

