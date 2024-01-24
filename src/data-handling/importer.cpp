#include <importer.h>
#include <converter.h>
#include <comparator.h>

#include <iostream>
#include <algorithm>
#include <chrono>

#include <fstream>
#include <sstream>
#include <vector>
#include <map>
#include <string>
#include <regex>

using namespace std;

void Importer::import(string folderName, bool cleanData, bool extendedGtfsVersion) {
    // Start the timer
    auto start = std::chrono::high_resolution_clock::now();

    string folderPath = "../../data/" + folderName + "/";

    // Import the data
    cout << "Importing data..." << endl;
    importCalendars(folderPath);
    importRoutes(folderPath);
    importStops(folderPath, extendedGtfsVersion);
    importTrips(folderPath);
    importStopTimes(folderPath);

    if (cleanData) {
        combineStops();
        generateValidRoutes();
        setIsAvailableOfTrips();
        clearAndSortTrips();
    }

    generateSortedConnections();

    // Stop the timer and calculate the duration
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

    // Print the duration
    cout << "Import time: " << duration << " milliseconds" << endl;
}

vector<StopTime> Importer::getStopTimesOfATrip(int tripId) {
    vector<StopTime> stopTimesOfATrip = vector<StopTime>(0);
    int indexOfFirstStopTime = indexOfFirstStopTimeOfATrip[tripId];
    int indexOfLastStopTime = stopTimes.size() - 1;
    for(int i = indexOfFirstStopTime; i < indexOfLastStopTime; i++) {
        if (stopTimes[i].tripId == tripId) {
            stopTimesOfATrip.push_back(stopTimes[i]);
        } else {
            break;
        }
    }
    return stopTimesOfATrip;
}

bool Importer::isTripAvailable(int tripId, int dayOfWeek) {
    return (trips[tripId].isAvailable >> dayOfWeek) & 1;
}

vector<string> Importer::splitCsvLine(string &line) {
    vector<string> fields;
    string field;
    bool inQuotes = false;

    for (char c : line) {
        if (c == '\"') {
            inQuotes = !inQuotes;
        } else if (c == ',' && !inQuotes) {
            fields.push_back(field);
            field.clear();
        } else {
            field.push_back(c);
        }
    }
    fields.push_back(field); // add last field

    // Remove enclosing double quotes from fields, if present
    for (auto& field : fields) {
        if (field.front() == '"' && field.back() == '"') {
            field = field.substr(1, field.size() - 2);
        }
    }

    return fields;
}

void Importer::importCalendars(string folderPath) {
    // Import the data
    std::string filePath = folderPath + "calendar.txt";

    std::ifstream file(filePath);
    if (!file.is_open()) {
        std::cerr << "Failed to open file: " << filePath << std::endl;
    }

    std::string line;
    std::getline(file, line); // Skip the header line

    int id = 0;

    while (std::getline(file, line)) {
        vector<string> fields = splitCsvLine(line);

        Calendar calendar;

        // Read each field and assign it to the calendar variable
        calendar.serviceId = id;
        serviceIdOldToNew[fields[0]] = id;

        for (int i = 1; i < 8; i++) {
            calendar.isAvailable.push_back(fields[i] == "1");
        }

        calendar.startDate = fields[8];
        calendar.endDate = fields[9];

        calendars.push_back(calendar);
        id++;
    }

    file.close();

    cout << "Imported " << calendars.size() << " calendars." << endl;
}

void Importer::importRoutes(string folderPath) {
    // Import the data

    // combine folder path with file name
    std::string filePath = folderPath + "routes.txt";

    std::ifstream file(filePath);
    if (!file.is_open()) {
        std::cerr << "Failed to open file: " << filePath << std::endl;
    }

    std::string line;
    std::getline(file, line); // Skip the header line

    int id = 0;

    while (std::getline(file, line)) {
        vector<string> fields = splitCsvLine(line);

        Route route;

        // Read each field and assign it to the routes variable
        route.id = id;
        routeIdOldToNew[fields[0]] = id;

        routes.push_back(route);
        id++;
    }

    file.close();

    cout << "Imported " << routes.size() << " routes." << endl;
}

void Importer::importStops(string folderPath, bool extendedGtfsVersion) {
    // Import the data

    std::string filePath = folderPath + "stops.txt";

    std::ifstream file(filePath);
    if (!file.is_open()) {
        std::cerr << "Failed to open file: " << filePath << std::endl;
    }

    std::string line;
    std::getline(file, line); // Skip the header line

    int id = 0;

    while (std::getline(file, line)) {
        vector<string> fields = splitCsvLine(line);

        Stop stop;

        // Read each field and assign it to the stop variable
        stop.id = id;
        stopIdOldToNew[fields[0]] = id;

        if (extendedGtfsVersion){
            stop.name = fields[2];
            stop.lat = std::stod(fields[4]);
            stop.lon = std::stod(fields[5]);
        } else {
            stop.name = fields[1];
            stop.lat = std::stod(fields[2]);
            stop.lon = std::stod(fields[3]);
        }

        stops.push_back(stop);
        id++;
    }

    file.close();

    cout << "Imported " << stops.size() << " stops." << endl;
}

void Importer::importStopTimes(string folderPath) {
    // Import the data

    std::string filePath = folderPath + "stop_times.txt";

    std::ifstream file(filePath);
    if (!file.is_open()) {
        std::cerr << "Failed to open file: " << filePath << std::endl;
    }

    std::string line;
    std::getline(file, line); // Skip the header line

    int id = 0;

    while (std::getline(file, line)) {
        vector<string> fields = splitCsvLine(line);

        StopTime stopTime;

        // Read each field and assign it to the stop time variable
        stopTime.tripId = tripIdOldToNew[fields[0]];
        stopTime.arrivalTime = TimeConverter::convertTimeToSeconds(fields[1]);
        stopTime.departureTime = TimeConverter::convertTimeToSeconds(fields[2]);
        stopTime.stopId = stopIdOldToNew[fields[3]];
        stopTime.stopSequence = std::stoi(fields[4]);

        stopTimes.push_back(stopTime);
        id++;
    }

    file.close();

    cout << "Imported " << stopTimes.size() << " stop times." << endl;
}

void Importer::importTrips(string folderPath) {
    // Import the data

    std::string filePath = folderPath + "trips.txt";

    std::ifstream file(filePath);
    if (!file.is_open()) {
        std::cerr << "Failed to open file: " << filePath << std::endl;
    }

    std::string line;
    std::getline(file, line); // Skip the header line

    int id = 0;

    while (std::getline(file, line)) {
        vector<string> fields = splitCsvLine(line);

        Trip trip;

        // Read each field and assign it to the trip variable
        trip.routeId = routeIdOldToNew[fields[0]];
        trip.serviceId = serviceIdOldToNew[fields[1]];
        trip.id = id;
        tripIdOldToNew[fields[2]] = id;

        trip.isAvailable = 127;

        trips.push_back(trip);
        id++;
    }

    file.close();

    cout << "Imported " << trips.size() << " trips." << endl;
}

/*
    Combine stops with the same name and update the stop times accordingly.
*/
void Importer::combineStops() {
    map<int, int> stopIdOldToCombined = map<int, int>();
    map<string, int> stopNameToId = map<string, int>();

    vector<Stop> newStops;
    int id = 0;

    for (Stop stop : stops) {
        if (stopNameToId.find(stop.name) == stopNameToId.end()) {
            stopNameToId[stop.name] = id;
            stopIdOldToCombined[stop.id] = id;
            stop.id = id;
            newStops.push_back(stop);
            id++;
        } else {
            int newId = stopNameToId[stop.name];
            stopIdOldToCombined[stop.id] = newId;
            stop.id = newId;
        }
    }

    stops = newStops;

    for (int i = 0; i < stopTimes.size(); i++) {
        stopTimes[i].stopId = stopIdOldToCombined[stopTimes[i].stopId];
    }

    cout << "Combined " << stops.size() << " stops." << endl;
}

/*
    Generate valid routes that satisfy the following condition of the raptor algorithms:
    All trips of a route must have the same sequence of stops.

    Fill the following variables:
        - indexOfFirstStopTimeOfATrip
        - tripsOfARoute
        - stopsOfARoute
        - routesOfAStop
*/
void Importer::generateValidRoutes() {
    vector<Route> newRoutes;
    sort(stopTimes.begin(), stopTimes.end(), StopTimeComparator::compareByTripIdAndSequence);

    indexOfFirstStopTimeOfATrip = vector<int>(trips.size());
    tripsOfARoute = vector<vector<int>>(0);
    stopsOfARoute = vector<vector<int>>(0);
    routesOfAStop = vector<vector<RouteSequencePair>>(stops.size());
    for (int i = 0; i < stops.size(); i++) {
        routesOfAStop[i] = vector<RouteSequencePair>(0);
    }

    map<string, int> routeIdMapping = map<string, int>();
    int lastTripId = stopTimes[0].tripId;
    string stopIdString = "";
    vector<int> stopIds = vector<int>(0);
    int stopSequence = 0;
    indexOfFirstStopTimeOfATrip[lastTripId] = 0;

    for (int i = 0; i < stopTimes.size(); i++) {
        StopTime stopTime = stopTimes[i];

        if (stopTime.tripId != lastTripId) {
            if(routeIdMapping.find(stopIdString) == routeIdMapping.end()) {
                Route newRoute;
                int newRouteId = newRoutes.size();
                newRoute.id = newRouteId;
                routeIdMapping[stopIdString] = newRouteId;
                newRoutes.push_back(newRoute);
                stopsOfARoute.push_back(stopIds);
                for(int j = 0; j < stopIds.size(); j++) {
                    RouteSequencePair routeSequencePair;
                    routeSequencePair.routeId = newRouteId;
                    routeSequencePair.stopSequence = j;
                    routesOfAStop[stopIds[j]].push_back(routeSequencePair);
                }
                tripsOfARoute.push_back(vector<int>(0));
                tripsOfARoute[newRouteId].push_back(lastTripId);
            } else {
                int routeId = routeIdMapping[stopIdString];
                tripsOfARoute[routeId].push_back(lastTripId);
            }
            trips[lastTripId].routeId = routeIdMapping[stopIdString];
            stopIdString = "";
            stopIds = vector<int>(0);
            stopSequence = 0;
            indexOfFirstStopTimeOfATrip[stopTime.tripId] = i;
        } 
        stopIdString += to_string(stopTime.stopId) + ",";
        stopIds.push_back(stopTime.stopId);
        stopSequence++;
        lastTripId = stopTime.tripId;
    }

    routes = newRoutes;

    cout << "Generated " << routes.size() << " valid routes." << endl;
}

/*
    Set the isAvailable variable of each trip using the calendar data.
*/
void Importer::setIsAvailableOfTrips() {
    map<int, int> serviceIdToCalendarId = map<int, int>();
    for (Calendar calendar : calendars) {
        int bit = 1;
        int binaryNumber = 0;
        for (int i = 0; i < 7; i++){
            if (calendar.isAvailable[i]){
                binaryNumber += bit;
            }
            bit *= 2;
        }
        serviceIdToCalendarId[calendar.serviceId] = binaryNumber;
    }
    for (int i = 0; i < trips.size(); i++) {
        trips[i].isAvailable = serviceIdToCalendarId[trips[i].serviceId];
    }

    cout << "Set isAvailable variable of " << trips.size() << " trips." << endl;
}

/*
    Sort the trips of a route by departure time. Make trips unavailable if they overtake another trip.
*/
void Importer::clearAndSortTrips() {
    for (int i = 0; i < tripsOfARoute.size(); i++) {
        vector<int> sortedTripsOfARoute = vector<int>(0);
        vector<TripDepartureTimePair> tripDepartureTimePairs = vector<TripDepartureTimePair>(0);
        for (int j = 0; j < tripsOfARoute[i].size(); j++) {
            TripDepartureTimePair tripDepartureTimePair;
            tripDepartureTimePair.tripId = tripsOfARoute[i][j];
            tripDepartureTimePair.departureTime = stopTimes[indexOfFirstStopTimeOfATrip[tripsOfARoute[i][j]]].departureTime;
            tripDepartureTimePairs.push_back(tripDepartureTimePair);
        }
        sort(tripDepartureTimePairs.begin(), tripDepartureTimePairs.end(), TripDepartureTimePairComparator::compareByDepartureTime);
        
        vector<vector<StopTime>> lastStopTimePerDay = vector<vector<StopTime>>(7);
        for (int j = 0; j < 7; j++){
            lastStopTimePerDay[j] = vector<StopTime>(0);
        }

        for (int j = 0; j < tripDepartureTimePairs.size(); j++){
            int tripId = tripDepartureTimePairs[j].tripId;
            vector<StopTime> stopTimesOfTrip = getStopTimesOfATrip(tripId);

            int bit = 1;
            for (int l = 0; l < 7; l++){
                if (isTripAvailable(tripId, l)){
                    bool removeStopTimesOfWeekday = false;
                    for (int k = 0; k < lastStopTimePerDay[l].size(); k++){
                        if(lastStopTimePerDay[l][k].departureTime >= stopTimesOfTrip[k].departureTime){
                            removeStopTimesOfWeekday = true;
                            break;
                        }
                    }
                    if (removeStopTimesOfWeekday) {
                        trips[tripId].isAvailable - bit;
                    } else {
                        lastStopTimePerDay[l] = stopTimesOfTrip;
                    }
                }
                bit *= 2;
            }
            sortedTripsOfARoute.push_back(tripId);
        }
        tripsOfARoute[i] = sortedTripsOfARoute;
    }
    cout << "Sorted and cleared " << trips.size() << " trips." << endl;
}

/*
    Use the stop times to generate connections so that they can be used by the CSA. 
    Store them sorted by their departure time.
*/
void Importer::generateSortedConnections() {
    for(int i = 0; i < trips.size(); i++){
        int tripId = trips[i].id;
        int indexOfFirstStopTime = indexOfFirstStopTimeOfATrip[tripId];

        StopTime previousStopTime = stopTimes[indexOfFirstStopTime];
        for (int j = indexOfFirstStopTime + 1; j < stopTimes.size(); j ++){
            StopTime currentStopTime = stopTimes[j];
            if (currentStopTime.tripId != tripId) {
                break;
            }
            Connection connection;
            connection.departureStopId = previousStopTime.stopId;
            connection.departureTime = previousStopTime.departureTime;
            connection.arrivalStopId = currentStopTime.stopId;
            connection.arrivalTime = currentStopTime.arrivalTime;

            connections.push_back(connection);
            previousStopTime = currentStopTime;
        }
    }

    sort(connections.begin(), connections.end(), ConnectionComparator::compareByDepartureTime);

    for (int i = 0; i < connections.size(); i++){
        connections[i].id = i;
    }

    cout << "Generated " << connections.size() << " connections." << endl;
}