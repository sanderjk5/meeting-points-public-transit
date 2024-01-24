#ifndef CMAKE_SEARCHER_H
#define CMAKE_SEARCHER_H

#include <importer.h>

using namespace std;

class ConnectionSearcher
{
public:
    explicit ConnectionSearcher(){};
    ~ConnectionSearcher(){};

    static int binarySearch(int departureTime);
};

#endif //CMAKE_SEARCHER_H