#include <importer.h>
#include <converter.h>

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

    // Stop the timer and calculate the duration
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

    // Print the duration
    cout << "Import time: " << duration << " milliseconds" << endl;
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

    long id = 0;

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

    long id = 0;

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

    long id = 0;

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

    long id = 0;

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

    long id = 0;

    while (std::getline(file, line)) {
        vector<string> fields = splitCsvLine(line);

        Trip trip;

        // Read each field and assign it to the trip variable
        trip.routeId = routeIdOldToNew[fields[0]];
        trip.serviceId = serviceIdOldToNew[fields[1]];
        trip.id = id;
        tripIdOldToNew[fields[2]] = id;

        trip.isAvailable = 1111111;

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
    map<long, long> stopIdOldToNew = map<long, long>();
    map<string, long> stopNameToId = map<string, long>();

    vector<Stop> newStops;
    long id = 0;

    for (Stop stop : stops) {
        if (stopNameToId.find(stop.name) == stopNameToId.end()) {
            stopNameToId[stop.name] = id;
            stopIdOldToNew[stop.id] = id;
            stop.id = id;
            newStops.push_back(stop);
            id++;
        } else {
            long newId = stopNameToId[stop.name];
            stopIdOldToNew[stop.id] = newId;
            stop.id = newId;
        }
    }

    stops = newStops;

    for (StopTime stopTime : stopTimes) {
        stopTime.stopId = stopIdOldToNew[stopTime.stopId];
    }

    cout << "Combined " << stops.size() << " stops." << endl;
}

void Importer::generateValidRoutes() {
    // Import the data
    cout << "Generating valid routes..." << endl;
}

void Importer::setIsAvailableOfTrips() {
    // Import the data
    cout << "Setting isAvailable of trips..." << endl;
}

void Importer::clearAndSortTrips() {
    // Import the data
    cout << "Clearing and sorting trips..." << endl;
}