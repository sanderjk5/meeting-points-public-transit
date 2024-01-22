#include <importer.h>
#include <converter.h>

#include <iostream>
#include <chrono>

#include <fstream>
#include <sstream>
#include <vector>
#include <map>
#include <string>

using namespace std;

void Importer::import(string folderName, bool cleanData) {
    // Start the timer
    auto start = std::chrono::high_resolution_clock::now();

    string folderPath = "../../data/" + folderName + "/";

    // Import the data
    cout << "Importing data..." << endl;
    importCalendars(folderPath);
    importRoutes(folderPath);
    importStops(folderPath);
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
        std::istringstream iss(line);
        std::string field;

        Calendar calendar;

        // Read each field and assign it to the calendar variable
        std::getline(iss, field, ',');
        calendar.serviceId = id;
        serviceIdOldToNew[field] = id;

        for (int i = 0; i < 7; i++) {
            std::getline(iss, field, ',');
            calendar.isAvailable.push_back(field == "1");
        }

        std::getline(iss, field, ',');
        calendar.startDate = field;

        std::getline(iss, field, ',');
        calendar.endDate = field;

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
        std::istringstream iss(line);
        std::string field;

        Route route;

        // Read each field and assign it to the routes variable
        std::getline(iss, field, ',');
        route.id = id;
        routeIdOldToNew[field] = id;

        routes.push_back(route);
        id++;
    }

    file.close();

    cout << "Imported " << routes.size() << " routes." << endl;
}

void Importer::importStops(string folderPath) {
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
        std::istringstream iss(line);
        std::string field;

        Stop stop;

        // Read each field and assign it to the stop variable
        std::getline(iss, field, ',');
        stop.id = id;
        stopIdOldToNew[field] = id;

        std::getline(iss, field, ',');

        std::getline(iss, field, ',');
        stop.name = field;

        std::getline(iss, field, ',');

        std::getline(iss, field, ',');
        stop.lat = std::stod(field);

        std::getline(iss, field, ',');
        stop.lon = std::stod(field);

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
        std::istringstream iss(line);
        std::string field;

        StopTime stopTime;

        // Read each field and assign it to the stop time variable
        std::getline(iss, field, ',');
        stopTime.tripId = tripIdOldToNew[field];

        std::getline(iss, field, ',');
        stopTime.arrivalTime = TimeConverter::convertTimeToSeconds(field);

        std::getline(iss, field, ',');
        stopTime.departureTime = TimeConverter::convertTimeToSeconds(field);

        std::getline(iss, field, ',');
        stopTime.stopId = stopIdOldToNew[field];

        std::getline(iss, field, ',');
        stopTime.stopSequence = std::stoi(field);

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
        std::istringstream iss(line);
        std::string field;

        Trip trip;

        // Read each field and assign it to the trip variable
        std::getline(iss, field, ',');
        trip.routeId = routeIdOldToNew[field];

        std::getline(iss, field, ',');
        trip.serviceId = serviceIdOldToNew[field];

        std::getline(iss, field, ',');
        trip.id = id;
        tripIdOldToNew[field] = id;

        std::getline(iss, field, ',');

        std::getline(iss, field, ',');

        trip.isAvailable = 1111111;

        trips.push_back(trip);
        id++;
    }

    file.close();

    cout << "Imported " << trips.size() << " trips." << endl;
}

void Importer::combineStops() {
    // Import the data
    cout << "Combining stops..." << endl;
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