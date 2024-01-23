#ifndef CMAKE_IMPORTER_H
#define CMAKE_IMPORTER_H

#include <string>
#include <vector>
#include <map>

using namespace std;


struct Calendar
{
    long serviceId;
    string startDate;
    string endDate;
    vector <bool> isAvailable; // list of days of the week
};

struct Route
{
    long id;
};

struct Stop
{
    long id;
    string name;
    double lat;
    double lon;
};

struct StopTime
{
    long tripId;
    int arrivalTime;
    int departureTime;
    long stopId;
    int stopSequence;
};

struct Trip
{
    long routeId;
    long serviceId;
    long id;
    int isAvailable;
};

struct RouteSequencePair
{
    long routeId;
    int stopSequence;
};

struct TripDepartureTimePair
{
    long tripId;
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

        vector<long> indexOfFirstStopTimeOfATrip;
        vector<vector<long>> tripsOfARoute;
        vector<vector<long>> stopsOfARoute;
        vector<vector<RouteSequencePair>> routesOfAStop;

        void import(string folderName, bool cleanData, bool extendedGtfsVersion);
        vector<StopTime> getStopTimesOfATrip(long tripId);
        bool isTripAvailable(long tripId, int dayOfWeek);

    private:
        map<string, long> serviceIdOldToNew;
        map<string, long> routeIdOldToNew;
        map<string, long> stopIdOldToNew;
        map<string, long> tripIdOldToNew;

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
};

#endif //CMAKE_IMPORTER_H