#ifndef CMAKE_QUERY_PROCESSOR_H
#define CMAKE_QUERY_PROCESSOR_H

#include <../data-handling/importer.h>

#include <vector>
#include <string>

struct Query {
    int sourceStopId;
    int targetStopId;
    int sourceTime;
};

struct Leg {
    string departureStopName;
    string arrivalStopName;
    int departureTime;
    int arrivalTime;
};

struct Journey {
    vector<Leg> legs;
    int duration;
};

class QueryProcessor {
    public:
        explicit QueryProcessor(){};
        ~QueryProcessor(){};

        static vector<Journey> processNaiveQuery(string sourceStopName, string targetStopName, string sourceTime);
    
    private:
        static Query createQuery(string sourceStopName, string targetStopName, string sourceTime);
};


#endif //CMAKE_QUERY_PROCESSOR_H