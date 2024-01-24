#ifndef CMAKE_IMPORTER_H
#define CMAKE_IMPORTER_H

#include <string>
#include <vector>
#include <map>

using namespace std;


struct Calendar
{
    int serviceId;
    string startDate;
    string endDate;
    vector <bool> isAvailable; // list of days of the week
};

struct Route
{
    int id;
};

struct Stop
{
    int id;
    string name;
    double lat;
    double lon;
};

struct StopTime
{
    int tripId;
    int arrivalTime;
    int departureTime;
    int stopId;
    int stopSequence;
};

struct Trip
{
    int routeId;
    int serviceId;
    int id;
    int isAvailable;
};

struct Connection
{
    int id;
    int departureStopId;
    int departureTime;
    int arrivalStopId;
    int arrivalTime;
    int tripId;
    int stopSequence;
};

struct RouteSequencePair
{
    int routeId;
    int stopSequence;
};

struct TripDepartureTimePair
{
    int tripId;
    int departureTime;
};

class Importer {
    public:
        explicit Importer(){};
        ~Importer(){};

        vector<Calendar> calendars;
        vector<Route> routes;
        vector<Stop> stops;
        vector<StopTime> stopTimes;
        vector<Trip> trips;

        vector<Connection> connections;

        vector<int> indexOfFirstStopTimeOfATrip;
        vector<vector<int>> tripsOfARoute;
        vector<vector<int>> stopsOfARoute;
        vector<vector<RouteSequencePair>> routesOfAStop;

        void import(string folderName, bool cleanData, bool extendedGtfsVersion);
        vector<StopTime> getStopTimesOfATrip(int tripId);
        bool isTripAvailable(int tripId, int dayOfWeek);

    private:
        map<string, int> serviceIdOldToNew;
        map<string, int> routeIdOldToNew;
        map<string, int> stopIdOldToNew;
        map<string, int> tripIdOldToNew;

        vector<string> splitCsvLine(string &line);

        void importCalendars(string folderPath);
        void importRoutes(string folderPath);
        void importStops(string folderPath, bool extendedGtfsVersion);
        void importStopTimes(string folderPath);
        void importTrips(string folderPath);

        void combineStops();
        void generateValidRoutes();
        void setIsAvailableOfTrips();
        void clearAndSortTrips();
        void generateSortedConnections();
};

#endif //CMAKE_IMPORTER_H