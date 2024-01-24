#ifndef CMAKE_CONVERTER_H
#define CMAKE_CONVERTER_H

#include <string>

using namespace std;

class TimeConverter {
    public:
        explicit TimeConverter(){};
        ~TimeConverter(){};

        static int convertTimeToSeconds(string time);
        static string convertSecondsToTime(int seconds);
        static int getDayOffset(int seconds);
        static int getDayDifference(int seconds);
};

#endif //CMAKE_CONVERTER_H