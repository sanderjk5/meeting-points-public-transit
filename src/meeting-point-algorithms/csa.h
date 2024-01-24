#ifndef CMAKE_CSA_H
#define CMAKE_CSA_H

#include <../data-handling/importer.h>
#include "query-processor.h"

struct JourneyPointer {
    Connection* enterConnection;
    Connection* exitConnection;
};

class CSA {
    public:
        explicit CSA(){};
        ~CSA(){};

        static vector<Journey> processCSAQuery(Query query);
};

#endif //CMAKE_CSA_H