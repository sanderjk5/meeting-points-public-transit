#ifndef CMAKE_CONVERTER_H
#define CMAKE_CONVERTER_H

#include <string>

using namespace std;

class TimeConverter {
    public:
        explicit TimeConverter(){};
        ~TimeConverter(){};

        static int convertTimeToSeconds(string time);
        static string convertSecondsToTime(int seconds, bool removeDays = true);
        static int getDayOffset(int seconds);
        static int getDayDifference(int seconds);
};

class WeekdayConverter {
    public:
        explicit WeekdayConverter(){};
        ~WeekdayConverter(){};

        static int convertWeekdayToInt(string weekday);
        static string convertIntToWeekday(int weekday);
};

#endif //CMAKE_CONVERTER_H