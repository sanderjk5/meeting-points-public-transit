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

struct FootPath {
    int departureStopId;
    int arrivalStopId;
    int duration;
};

enum DataType {
    vvs_j24,
    schienenregionalverkehr_de,
    schienenfernverkehr_de,
    schienenfern_und_regionalverkehr_de,
    gesamt_de
};

/*
    * This class is responsible for importing the data from the GTFS files and preparing it for the algorithm.
    * It also provides some utility functions to access the imported data.
*/
class Importer {
    public:
        explicit Importer(){};
        ~Importer(){};

        // The following vectors store the imported data
        static vector<Calendar> calendars;
        static vector<Route> routes;
        static vector<Stop> stops;
        static vector<StopTime> stopTimes;
        static vector<Trip> trips;

        static vector<Connection> connections;

        // The following vectors store the prepared data to make it easier to access the imported data
        static vector<int> indexOfFirstStopTimeOfATrip;
        static vector<vector<int>> tripsOfARoute;
        static vector<vector<int>> stopsOfARoute;
        static vector<vector<RouteSequencePair>> routesOfAStop;

        static vector<FootPath> footPaths;
        static vector<int> indexOfFirstFootPathOfAStop;

        static void import(string folderName, bool prepareData, DataType dataType);
        static vector<StopTime> getStopTimesOfATrip(int tripId);
        static bool isTripAvailable(int tripId, int dayOfWeek);

        static string getStopName(int stopId);
        static int getStopId(string stopName);

        static string getDataTypeString(DataType dataType);

    private:
        // The following maps are used to map the old ids to new ids
        static map<string, int> serviceIdOldToNew;
        static map<string, int> routeIdOldToNew;
        static map<string, int> stopIdOldToNew;
        static map<string, int> tripIdOldToNew;

        static vector<string> splitCsvLine(string &line);

        static void importCalendars(string folderPath, DataType dataType);
        static void importRoutes(string folderPath, DataType dataType);
        static void importStops(string folderPath, DataType dataType);
        static void importStopTimes(string folderPath, DataType dataType);
        static void importTrips(string folderPath, DataType dataType);

        static void combineStops();
        static void generateValidRoutes();
        static void setIsAvailableOfTrips();
        static void clearAndSortTrips();
        static void generateSortedConnections();
        static void loadOrGenerateFootPaths(DataType dataType);
        static void generateFootPaths();
        static void exportFootPaths(DataType dataType);
        static void importFootPaths(DataType dataType);
};

#endif //CMAKE_IMPORTER_H