# include <converter.h>
# include <string>
# include <../constants.h>

#include <algorithm>
#include <cctype>

using namespace std;

int TimeConverter::convertTimeToSeconds(string time) {
    int hours = stoi(time.substr(0, 2));
    int minutes = stoi(time.substr(3, 2));
    int seconds = stoi(time.substr(6, 2));

    return hours * 3600 + minutes * 60 + seconds;
}

string TimeConverter::convertSecondsToTime(int seconds) {
    int hours = seconds / 3600;
    int minutes = (seconds % 3600) / 60;
    int secondsLeft = seconds % 60;

    string hoursString = to_string(hours);
    string minutesString = to_string(minutes);
    string secondsString = to_string(secondsLeft);

    if (hours < 10) {
        hoursString = "0" + hoursString;
    }

    if (minutes < 10) {
        minutesString = "0" + minutesString;
    }

    if (secondsLeft < 10) {
        secondsString = "0" + secondsString;
    }

    return hoursString + ":" + minutesString + ":" + secondsString;
}

int TimeConverter::getDayOffset(int seconds) {
    int days = seconds / SECONDSPERDAY;
    return days * SECONDSPERDAY;
}

int TimeConverter::getDayDifference(int seconds) {
    return seconds % SECONDSPERDAY;
}

int WeekdayConverter::convertWeekdayToInt(string weekday) {
    transform(weekday.begin(), weekday.end(), weekday.begin(), [](unsigned char c){ return tolower(c); });

    if (weekday == "monday") {
        return 0;
    } else if (weekday == "tuesday") {
        return 1;
    } else if (weekday == "wednesday") {
        return 2;
    } else if (weekday == "thursday") {
        return 3;
    } else if (weekday == "friday") {
        return 4;
    } else if (weekday == "saturday") {
        return 5;
    } else if (weekday == "sunday") {
        return 6;
    } else {
        return -1;
    }
}

string WeekdayConverter::convertIntToWeekday(int weekday) {
    if (weekday == 0) {
        return "monday";
    } else if (weekday == 1) {
        return "tuesday";
    } else if (weekday == 2) {
        return "wednesday";
    } else if (weekday == 3) {
        return "thursday";
    } else if (weekday == 4) {
        return "friday";
    } else if (weekday == 5) {
        return "saturday";
    } else if (weekday == 6) {
        return "sunday";
    } else {
        return "invalid";
    }
}