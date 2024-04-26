#include "importer.h"

#include <converter.h>
#include <comparator.h>
#include <calculator.h>

#include <../constants.h>


#include <iostream>
#include <algorithm>
#include <chrono>
#include <cctype>

#include <fstream>
#include <sstream>
#include <vector>
#include <map>
#include <string>
#include <regex>
#include <fstream>

using namespace std;

// Initialize all static variables
vector<Calendar> Importer::calendars = vector<Calendar>(0);
map<string, int> Importer::serviceIdOldToNew = map<string, int>();
vector<Route> Importer::routes = vector<Route>(0);
map<string, int> Importer::routeIdOldToNew = map<string, int>();    
vector<Stop> Importer::stops = vector<Stop>(0);
map<string, int> Importer::stopIdOldToNew = map<string, int>();
vector<StopTime> Importer::stopTimes = vector<StopTime>(0);
vector<Trip> Importer::trips = vector<Trip>(0);
map<string, int> Importer::tripIdOldToNew = map<string, int>();
vector<int> Importer::indexOfFirstStopTimeOfATrip = vector<int>(0);
vector<vector<int>> Importer::tripsOfARoute = vector<vector<int>>(0);
vector<vector<int>> Importer::stopsOfARoute = vector<vector<int>>(0);
vector<vector<RouteSequencePair>> Importer::routesOfAStop = vector<vector<RouteSequencePair>>(0);
vector<Connection> Importer::connections = vector<Connection>(0);
vector<FootPath> Importer::footPaths = vector<FootPath>(0);
vector<int> Importer::indexOfFirstFootPathOfAStop = vector<int>(0);

/*
    Import the data from the GTFS files and prepare it for the algorithms.
    The data is imported from the folder with the given name.
    If prepareData is true, the data is prepared for the algorithms.
    The dataType parameter is used to determine the format of the GTFS files.
*/
void Importer::import(string folderName, bool prepareData, DataType dataType) {
    // Start the timer
    auto start = std::chrono::high_resolution_clock::now();

    string folderPathData = FOLDER_PREFIX + "data/" + folderName + "/";

    // Import the data
    cout << "\nImporting data..." << endl;
    importCalendars(folderPathData, dataType);
    importRoutes(folderPathData, dataType);
    importStops(folderPathData, dataType);
    importTrips(folderPathData, dataType);
    importStopTimes(folderPathData, dataType);

    // Prepare the data for the algorithms
    if (prepareData) {
        combineStops();
        generateValidRoutes();
        setIsAvailableOfTrips();
        clearAndSortTrips();
        generateSortedConnections();
        loadOrGenerateFootPaths(dataType);
    }

    // Stop the timer and calculate the duration
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

    // Print the duration
    cout << "Import duration: " << duration << " milliseconds\n" << endl;
}

/*
    Import the calendar data from the GTFS files.
    The data is imported from the folder with the given name.
    The dataType parameter is used to determine the format of the GTFS files.
*/
void Importer::importCalendars(string folderPathResults, DataType dataType) {
    std::string filePath = folderPathResults + "calendar.txt";

    std::ifstream file(filePath);
    if (!file.is_open()) {
        std::cerr << "Failed to open file: " << filePath << std::endl;
    }

    std::string line;
    std::getline(file, line); // Skip the header line

    int id = calendars.size();

    while (std::getline(file, line)) {
        vector<string> fields = splitCsvLine(line);

        Calendar calendar;

        // Read each field and assign it to the calendar variable
        if (dataType == vvs_j24){
            calendar.serviceId = id;
            serviceIdOldToNew[fields[0]] = id;

            for (int i = 1; i < 8; i++) {
                calendar.isAvailable.push_back(fields[i] == "1");
            }

            calendar.startDate = fields[8];
            calendar.endDate = fields[9];
        } else {
            calendar.serviceId = id;
            if (dataType == schienenregionalverkehr_de || dataType == schienenfern_und_regionalverkehr_de){
                serviceIdOldToNew["re-" + fields[9]] = id;
            } else if (dataType == schienenfernverkehr_de) {
                serviceIdOldToNew["fe-" + fields[9]] = id;
            } else {
                serviceIdOldToNew[fields[9]] = id;
            }

            for (int i = 0; i < 7; i++) {
                calendar.isAvailable.push_back(fields[i] == "1");
            }

            calendar.startDate = fields[7];
            calendar.endDate = fields[8];
        }
        

        calendars.push_back(calendar);
        id++;
    }

    file.close();

    cout << "Imported " << calendars.size() << " calendars." << endl;
}

/*
    Import the route data from the GTFS files.
    The data is imported from the folder with the given name.
    The dataType parameter is used to determine the format of the GTFS files.
*/
void Importer::importRoutes(string folderPathResults, DataType dataType) {
    // combine folder path with file name
    std::string filePath = folderPathResults + "routes.txt";

    vector<string> longDistanceShortNames = {"EC", "EN", "RJ", "IC", "ICE"};

    std::ifstream file(filePath);
    if (!file.is_open()) {
        std::cerr << "Failed to open file: " << filePath << std::endl;
    }

    std::string line;
    std::getline(file, line); // Skip the header line

    int id = routes.size();

    vector<int> routeTypesCounter = vector<int>(10, 0);

    while (std::getline(file, line)) {
        vector<string> fields = splitCsvLine(line);

        Route route;

        // Read each field and assign it to the routes variable
        route.id = id;
        if (dataType == schienenregionalverkehr_de || dataType == schienenfern_und_regionalverkehr_de) {
            routeIdOldToNew["re-" + fields[4]] = id;
        } else if (dataType == schienenfernverkehr_de) {
            routeIdOldToNew["fe-" + fields[4]] = id;
        } else if (dataType == gesamt_de) {
            routeIdOldToNew[fields[4]] = id;
        } else {
            routeIdOldToNew[fields[0]] = id;
        }

        if (dataType != vvs_j24 && find(longDistanceShortNames.begin(), longDistanceShortNames.end(), fields[1]) != longDistanceShortNames.end()) {
            route.isLongDistance = true;
        } else {
            route.isLongDistance = false;
        }
        
        routes.push_back(route);
        id++;
    }

    file.close();

    cout << "Imported " << routes.size() << " routes." << endl;
}

/*
    Import the stop data from the GTFS files.
    The data is imported from the folder with the given name.
    The dataType parameter is used to determine the format of the GTFS files.
*/
void Importer::importStops(string folderPathResults, DataType dataType) {
    std::string filePath = folderPathResults + "stops.txt";

    std::ifstream file(filePath);
    if (!file.is_open()) {
        std::cerr << "Failed to open file: " << filePath << std::endl;
    }

    std::string line;
    std::getline(file, line); // Skip the header line

    int id = stops.size();

    while (std::getline(file, line)) {
        vector<string> fields = splitCsvLine(line);

        Stop stop;

        // Read each field and assign it to the stop variable
        if (dataType == vvs_j24){
            stop.id = id;
            stopIdOldToNew[fields[0]] = id;

            stop.name = fields[1];
            stop.lat = std::stod(fields[2]);
            stop.lon = std::stod(fields[3]);
        } else {
            stop.id = id;
            if (dataType == schienenregionalverkehr_de || dataType == schienenfern_und_regionalverkehr_de){
                stopIdOldToNew["re-" + fields[2]] = id;
            } else if(dataType == schienenfernverkehr_de) {
                stopIdOldToNew["fe-" + fields[2]] = id;
            } else {
                stopIdOldToNew[fields[2]] = id;
            }

            stop.name = fields[0];
            stop.lat = std::stod(fields[3]);
            stop.lon = std::stod(fields[4]);
        }

        stops.push_back(stop);
        id++;
    }

    file.close();

    cout << "Imported " << stops.size() << " stops." << endl;
}

/*
    Import the stop time data from the GTFS files.
    The data is imported from the folder with the given name.
    The dataType parameter is used to determine the format of the GTFS files.
*/
void Importer::importStopTimes(string folderPathResults, DataType dataType) {
    std::string filePath = folderPathResults + "stop_times.txt";

    std::ifstream file(filePath);
    if (!file.is_open()) {
        std::cerr << "Failed to open file: " << filePath << std::endl;
    }

    std::string line;
    std::getline(file, line); // Skip the header line

    int id = stopTimes.size();

    while (std::getline(file, line)) {
        vector<string> fields = splitCsvLine(line);

        StopTime stopTime;

        // Read each field and assign it to the stop time variable
        if (dataType == schienenregionalverkehr_de || dataType == schienenfern_und_regionalverkehr_de){
            stopTime.tripId = tripIdOldToNew["re-" + fields[0]];
            stopTime.stopId = stopIdOldToNew["re-" + fields[3]];
        } else if (dataType == schienenfernverkehr_de) {
            stopTime.tripId = tripIdOldToNew["fe-" + fields[0]];
            stopTime.stopId = stopIdOldToNew["fe-" + fields[3]];
        } else {
            stopTime.tripId = tripIdOldToNew[fields[0]];
            stopTime.stopId = stopIdOldToNew[fields[3]];
        }
        stopTime.arrivalTime = TimeConverter::convertTimeToSeconds(fields[1]);
        stopTime.departureTime = TimeConverter::convertTimeToSeconds(fields[2]);
        stopTime.stopSequence = std::stoi(fields[4]);

        stopTimes.push_back(stopTime);
        id++;
    }

    file.close();

    cout << "Imported " << stopTimes.size() << " stop times." << endl;
}

/*
    Import the trip data from the GTFS files.
    The data is imported from the folder with the given name.
    The dataType parameter is used to determine the format of the GTFS files.
*/
void Importer::importTrips(string folderPathResults, DataType dataType) {
    std::string filePath = folderPathResults + "trips.txt";

    std::ifstream file(filePath);
    if (!file.is_open()) {
        std::cerr << "Failed to open file: " << filePath << std::endl;
    }

    std::string line;
    std::getline(file, line); // Skip the header line

    int id = trips.size();

    while (std::getline(file, line)) {
        vector<string> fields = splitCsvLine(line);

        Trip trip;

        trip.id = id;

        // Read each field and assign it to the trip variable
        if (dataType == schienenregionalverkehr_de || dataType == schienenfern_und_regionalverkehr_de){
            trip.routeId = routeIdOldToNew["re-" + fields[0]];
            trip.serviceId = serviceIdOldToNew["re-" + fields[1]];
            tripIdOldToNew["re-" + fields[2]] = id;
        } else if (dataType == schienenfernverkehr_de){
            trip.routeId = routeIdOldToNew["fe-" + fields[0]];
            trip.serviceId = serviceIdOldToNew["fe-" + fields[1]];
            tripIdOldToNew["fe-" + fields[2]] = id;
        } else {
            trip.routeId = routeIdOldToNew[fields[0]];
            trip.serviceId = serviceIdOldToNew[fields[1]];
            tripIdOldToNew[fields[2]] = id;
        }

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
    map<string, vector<Stop>> stopNameToStops = map<string, vector<Stop>>();

    vector<Stop> newStops;
    int id = 0;

    for (Stop stop : stops) {
        if (stopNameToStops.find(stop.name) == stopNameToStops.end()) {
            stopNameToStops[stop.name] = vector<Stop>(0);
            stopIdOldToCombined[stop.id] = id;
            stop.id = id;
            newStops.push_back(stop);
            stopNameToStops[stop.name].push_back(stop);
            id++;
        } else {
            bool combinedStop = false;
            for (Stop stopInMap : stopNameToStops[stop.name]) {
                double distance = DistanceCalculator::calculateDistance(stop.lat, stop.lon, stopInMap.lat, stopInMap.lon);
                if (distance < 1) {
                    stopIdOldToCombined[stop.id] = stopInMap.id;
                    stop.id = stopInMap.id;
                    combinedStop = true;
                    break;
                }
            }
            if (!combinedStop) {
                stopIdOldToCombined[stop.id] = id;
                stop.id = id;
                newStops.push_back(stop);
                stopNameToStops[stop.name].push_back(stop);
                id++;
            }
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
                newRoute.isLongDistance = routes[trips[lastTripId].routeId].isLongDistance;
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
            // If the arrival time is the same as the departure time, the the arrival time is increased by 30 seconds.
            // if (currentStopTime.arrivalTime == previousStopTime.departureTime){
            //     connection.arrivalTime += 30;
            // }
            connection.tripId = tripId;

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

void Importer::loadOrGenerateFootPaths(DataType dataType) {
    string dataTypeString = getDataTypeString(dataType);
    string folderPathData = FOLDER_PREFIX + "data/" + dataTypeString + "/";
    string filePath = folderPathData + "foot_paths.txt";

    ifstream file(filePath);

    if (file.is_open()) {
        file.close();
        importFootPaths(dataType);
    } else {
        generateFootPaths();
        exportFootPaths(dataType);
    }
}

void Importer::generateFootPaths() {
    cout << "Generating foot paths..." << endl;
    footPaths = vector<FootPath>(0);
    indexOfFirstFootPathOfAStop = vector<int>(stops.size());
    for (int i = 0; i < stops.size(); i++){
        indexOfFirstFootPathOfAStop[i] = footPaths.size();
        if (USE_FOOTPATHS) {
            for (int j = 0; j < stops.size(); j++){
                double distance = DistanceCalculator::calculateDistance(stops[i].lat, stops[i].lon, stops[j].lat, stops[j].lon);
                if (distance < 0.5){
                    FootPath footPath;
                    footPath.departureStopId = i;
                    footPath.arrivalStopId = j;
                    // assume 4km/h walking speed
                    footPath.duration = distance * 900;
                    footPaths.push_back(footPath);
                }
            }
        } else {
            FootPath footPath;
            footPath.departureStopId = i;
            footPath.arrivalStopId = i;
            footPath.duration = 0;
            footPaths.push_back(footPath);
        }
    }
    cout << "Generated " << footPaths.size() << " foot paths." << endl;
}

void Importer::importFootPaths(DataType dataType) {
    string dataTypeString = getDataTypeString(dataType);
    string folderPathData = FOLDER_PREFIX + "data/" + dataTypeString + "/";
    string filePath = folderPathData + "foot_paths.txt";

    ifstream file(filePath);

    if (!file.is_open()) {
        std::cerr << "Failed to open footpath file: " << filePath << std::endl;
    }

    footPaths = vector<FootPath>(0);
    indexOfFirstFootPathOfAStop = vector<int>(stops.size());

    string line;
    getline(file, line); // Skip the header line

    int id = 0;
    int lastStopId = -1;
    while (getline(file, line)) {
        vector<string> fields = splitCsvLine(line);

        FootPath footPath;
        footPath.departureStopId = stoi(fields[0]);
        footPath.arrivalStopId = stoi(fields[1]);
        footPath.duration = stoi(fields[2]);

        if (footPath.departureStopId != lastStopId) {
            indexOfFirstFootPathOfAStop[footPath.departureStopId] = id;
            lastStopId = footPath.departureStopId;
        }

        footPaths.push_back(footPath);
        id++;
    }

    file.close();

    cout << "Imported " << footPaths.size() << " foot paths." << endl;
}

void Importer::exportFootPaths(DataType dataType) {
    string dataTypeString = getDataTypeString(dataType);
    string folderPathData = FOLDER_PREFIX + "data/" + dataTypeString + "/";
    string filePath = folderPathData + "foot_paths.txt";

    ofstream file(filePath);

    if (!file.is_open()) {
        std::cerr << "Failed to open footpath file: " << filePath << std::endl;
    }

    file << "departure_stop_id,arrival_stop_id,duration\n";
    for (FootPath footPath : footPaths) {
        file << footPath.departureStopId << "," << footPath.arrivalStopId << "," << footPath.duration << "\n";
    }

    file.close();
}

/*
    Get the stop times of a trip ordered by their sequence.
*/
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

/*
    Check if a trip is available on a specific day of the week.
*/
bool Importer::isTripAvailable(int tripId, int dayOfWeek) {
    return (trips[tripId].isAvailable >> dayOfWeek) & 1;
}

/*
    Split a line of a CSV file into its fields.
*/
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

/*
    Get the name of a stop by its id.
*/
string Importer::getStopName(int stopId) {
    if (stopId == -1) {
        return "";
    }
    return stops[stopId].name;
}

/*
    Get the id of a stop by its name.
*/
int Importer::getStopId(string stopName) {
    for (int i = 0; i < stops.size(); i++) {
        if (stops[i].name == stopName) {
            return stops[i].id;
        }
    }
    return -1;
}

/*
    Get the data type as a string.
*/
string Importer::getDataTypeString(DataType dataType) {
    string dataTypeString = "";
    if(dataType == vvs_j24) {
        dataTypeString = "vvs_j24";
    } else if (dataType == schienenfernverkehr_de) {
        dataTypeString = "schienenfernverkehr_de";
    } else if (dataType == schienenregionalverkehr_de) {
        dataTypeString = "schienenregionalverkehr_de";
    } else if (dataType == schienenfern_und_regionalverkehr_de) {
        dataTypeString = "schienenfern_und_regionalverkehr_de";
    } else if (dataType == gesamt_de) {
        dataTypeString = "gesamt_de";
    }

    return dataTypeString;
}