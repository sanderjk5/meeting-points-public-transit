#include "cli-controller.h"

#include"data-handling/importer.h"
#include"data-handling/converter.h"
#include"meeting-point-algorithms/query-processor.h"
#include"meeting-point-algorithms/algorithm-tester.h"
#include"meeting-point-algorithms/algorithm-tester-raptor.h"
#include "constants.h"
#include <iostream>
#include <regex>

using namespace std;

/*
    Run the command line interface for the meeting point query.
*/
void CliController::runCli(DataType dataType, GTree* gTree) {
    bool stop = false;

    while(!stop) {
        MeetingPointQuery query;

        // Ask if the user wants to run a random or a specific query
        cout << "Do you want to run a random query? (y/N)" << endl;
        bool randomQuery = false;
        string input;
        getline(cin, input);
        if (input == "y") {
            randomQuery = true;
        }

        int numberOfSourceStops = 0;
        string numberOfSourceStopsInput;
        while(numberOfSourceStops < 2) {
            cout << "How many source stops do you want to use?" << endl;
            getline(cin, numberOfSourceStopsInput);

            try {
                numberOfSourceStops = stoi(numberOfSourceStopsInput);
            } catch (invalid_argument& e) {
                cout << "Invalid input. Please enter a valid number." << endl;
                continue;
            }

            if (numberOfSourceStops < 2) {
                cout << "The number of source stops must be at least 2." << endl;
            }
        }

        if (randomQuery) {
            query = QueryGenerator::generateRandomMeetingPointQuery(numberOfSourceStops);
        } else {
            vector<string> sourceStopNames;
            cout << "Enter the names of the source stops:" << endl;
            while(sourceStopNames.size() < numberOfSourceStops) {
                string sourceStopName;
                getline(cin, sourceStopName);
                int sourceStopId = Importer::getStopId(sourceStopName);
                if (sourceStopId == -1) {
                    cout << "The stop " << sourceStopName << " does not exist. Please enter a valid stop name." << endl;
                    continue;
                }
                sourceStopNames.push_back(sourceStopName);
            }

            string sourceTime;
            bool validTime = false;
            while (!validTime) {
                cout << "Enter the source time (format: HH:MM:SS):" << endl;
                getline(cin, sourceTime);

                // Validate the format of the sourceTime using a regular expression
                regex timeRegex("^([0-1][0-9]|2[0-3]):([0-5][0-9]):([0-5][0-9])$");
                validTime = regex_match(sourceTime, timeRegex);

                if (!validTime) {
                    cout << "Invalid time. Please enter the time in the format HH:MM:SS." << endl;
                }
            }

            string weekday;
            bool validWeekday = false;
            while(!validWeekday) {
                cout << "Enter the weekday (format: Monday, Tuesday, ...):" << endl;
                getline(cin, weekday);
                validWeekday = WeekdayConverter::convertWeekdayToInt(weekday) != -1;
                if (!validWeekday) {
                    cout << "Invalid weekday. Please enter a valid weekday." << endl;
                }
            }

            query = QueryGenerator::generateMeetingPointQuery(sourceStopNames, sourceTime, weekday);
        }

        bool validAlgorithm = false;
        string algorithm;
        while(!validAlgorithm) {
            cout << "Which algorithm do you want to use? \nThe options are (enter the abbrevation): compare all algorithms (a), naive raptor algorithm (n), raptor first algorithm (r), raptor pq algorithm (q), raptor bound algorithm (b), raptor select algorithm (x), raptor select algorithm with candidates (c), raptor select loop algorithm (l)." << endl;
            getline(cin, algorithm);
            if (algorithm == "a" || algorithm == "n" || algorithm == "r" || algorithm == "q" || algorithm == "b" || algorithm == "x" || algorithm == "c" || algorithm == "l") {
                validAlgorithm = true;
            } else {
                cout << "Invalid algorithm. Please enter the abbrevation of a valid algorithm." << endl;
            }
        }

        cout << "\nRunning the query..." << endl;

        if (algorithm == "a") {
            AlgorithmComparer::compareAlgorithmsWithoutGTree(dataType, query);
        } else if (algorithm == "n") {
            RaptorAlgorithmTester::testRaptorAlgorithm(query, true);
        } else if (algorithm == "r") {
            RaptorAlgorithmTester::testRaptorFirstAlgorithm(query, true);
        } else if (algorithm == "q") {
            RaptorPQAlgorithmTester::testRaptorPQAlgorithm(query, true);
        } else if (algorithm == "b") {
            RaptorBoundAlgorithmTester::testRaptorBoundAlgorithm(query, true);
        } else if (algorithm == "x") {
            RaptorApproximationAlgorithmTester::testRaptorApproximationAlgorithm(query, false);
        } else if (algorithm == "c") {
            RaptorApproximationAlgorithmTester::testRaptorApproximationAlgorithm(query, true);
        } else if (algorithm == "l") {
            RaptorApproximationAlgorithmTester::testRaptorLoopAlgorithm(query);
        }

        cout << "\nDo you want to run another query? (y/N)" << endl;
        string runAgain;
        getline(cin, runAgain);
        if (runAgain != "y") {
            stop = true;
        }
    }
}

void CliController::runCliEat(DataType dataType) {
    bool stop = false;

    while(!stop) {
        RaptorQuery query;

        // Ask if the user wants to run a random or a specific query
        cout << "Do you want to run a random query? (y/N)" << endl;
        bool randomQuery = false;
        string input;
        getline(cin, input);
        if (input == "y") {
            randomQuery = true;
        }

        if (randomQuery) {
            query = QueryGenerator::generateRandomRaptorQuery();
        } else {
            string sourceStopName;
            bool validStop = false;
            while(!validStop) {
                cout << "Enter the name of the source stop:" << endl;
                getline(cin, sourceStopName);
                int sourceStopId = Importer::getStopId(sourceStopName);
                if (sourceStopId == -1) {
                    cout << "The stop " << sourceStopName << " does not exist. Please enter a valid stop name." << endl;
                    continue;
                }
                validStop = true;
            }
            
            string targetStopName;
            bool validTargetStop = false;
            while(!validTargetStop) {
                cout << "Enter the name of the target stop:" << endl;
                getline(cin, targetStopName);
                int targetStopId = Importer::getStopId(targetStopName);
                if (targetStopId == -1) {
                    cout << "The stop " << targetStopName << " does not exist. Please enter a valid stop name." << endl;
                    continue;
                }
                validTargetStop = true;
            }

            string sourceTime;
            bool validTime = false;
            while (!validTime) {
                cout << "Enter the source time (format: HH:MM:SS):" << endl;
                getline(cin, sourceTime);

                // Validate the format of the sourceTime using a regular expression
                regex timeRegex("^([0-1][0-9]|2[0-3]):([0-5][0-9]):([0-5][0-9])$");
                validTime = regex_match(sourceTime, timeRegex);

                if (!validTime) {
                    cout << "Invalid time. Please enter the time in the format HH:MM:SS." << endl;
                }
            }

            string weekday;
            bool validWeekday = false;
            while(!validWeekday) {
                cout << "Enter the weekday (format: Monday, Tuesday, ...):" << endl;
                getline(cin, weekday);
                validWeekday = WeekdayConverter::convertWeekdayToInt(weekday) != -1;
                if (!validWeekday) {
                    cout << "Invalid weekday. Please enter a valid weekday." << endl;
                }
            }

            query = QueryGenerator::generateRaptorQuery(sourceStopName, targetStopName, sourceTime, weekday);
        }

        bool validAlgorithm = false;
        string algorithm;
        while(!validAlgorithm) {
            cout << "Which algorithm do you want to use? \nThe options are (enter the abbrevation): compare all algorithms (a), basic raptor algorithm (r), raptor bound star algorithm (b), raptor pq star algorithm (q)." << endl;
            getline(cin, algorithm);
            if (algorithm == "a" || algorithm == "r" || algorithm == "q" || algorithm == "b") {
                validAlgorithm = true;
            } else {
                cout << "Invalid algorithm. Please enter the abbrevation of a valid algorithm." << endl;
            }
        }

        cout << "\nRunning the query..." << endl;

        if (algorithm == "a") {
            RaptorEATAlgorithmTester::testRaptorAlgorithms(query);
        } else if (algorithm == "r") {
            RaptorEATAlgorithmTester::testRaptorEatAlgorithm(query, true);
        } else if (algorithm == "q") {
            RaptorEATAlgorithmTester::testRaptorStarPQAlgorithm(query, true);
        } else if (algorithm == "b") {
            RaptorEATAlgorithmTester::testRaptorStarBoundAlgorithm(query, true);
        }

        cout << "\nDo you want to run another query? (y/N)" << endl;
        string runAgain;
        getline(cin, runAgain);
        if (runAgain != "y") {
            stop = true;
        } else {
            cout << endl;
        }
    }
}