#ifndef CMAKE_QUERY_PROCESSOR_RAPTOR_H
#define CMAKE_QUERY_PROCESSOR_RAPTOR_H

#include "raptor.h"
#include "journey.h"
#include <memory>

struct RaptorQueryResult {
    string arrivalTime;
    int durationInSeconds;
    double queryTime;
};

class RaptorQueryEATProcessor {
    public:
        explicit RaptorQueryEATProcessor(RaptorQuery raptorQuery){
            this->raptorQuery = raptorQuery;
        }
        ~RaptorQueryEATProcessor(){};

        void processRaptorQuery();
        RaptorQueryResult getRaptorQueryResult();
        Journey getJourney();

        int numberOfExpandedRoutes;

    private:
        RaptorQuery raptorQuery;
        RaptorQueryResult raptorQueryResult;

        shared_ptr<Raptor> raptor;
};

class RaptorStarBoundQueryProcessor {
    public:
        explicit RaptorStarBoundQueryProcessor(RaptorQuery raptorStarQuery){
            this->raptorStarQuery = raptorStarQuery;
        };
        ~RaptorStarBoundQueryProcessor(){};

        void processRaptorStarBoundQuery();
        RaptorQueryResult getRaptorStarQueryResult();
        Journey getJourney();

        int numberOfExpandedRoutes;

        double lowerBoundSmallerCounter;
        double lowerBoundGreaterCounter;
        double lowerBoundSmallerFraction;
        double lowerBoundAbsDiff;
        double lowerBoundRelDiff;

    private:
        RaptorQuery raptorStarQuery;
        RaptorQueryResult raptorStarQueryResult;

        shared_ptr<RaptorBoundStar> raptorBoundStar;
};

class RaptorStarPQQueryProcessor {
    public:
        explicit RaptorStarPQQueryProcessor(RaptorQuery raptorStarQuery){
            this->raptorStarQuery = raptorStarQuery;
        };
        ~RaptorStarPQQueryProcessor(){};

        void processRaptorStarPQQuery();
        RaptorQueryResult getRaptorStarQueryResult();
        Journey getJourney();

        int numberOfExpandedRoutes;

        double lowerBoundSmallerCounter;
        double lowerBoundGreaterCounter;
        double lowerBoundSmallerFraction;
        double lowerBoundAbsDiff;
        double lowerBoundRelDiff;

    private:
        RaptorQuery raptorStarQuery;
        RaptorQueryResult raptorStarQueryResult;

        shared_ptr<RaptorPQStar> raptorPQStar;
};

#endif //CMAKE_QUERY_PROCESSOR_RAPTOR_H