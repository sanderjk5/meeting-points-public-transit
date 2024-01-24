# include <converter.h>
# include <string>
# include <../header.h>

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