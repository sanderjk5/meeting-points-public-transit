#ifndef CMAKE_CALCULATOR_H
#define CMAKE_CALCULATOR_H

class DistanceCalculator {
    public:
        explicit DistanceCalculator(){};
        ~DistanceCalculator(){};

        static double calculateDistance(double lat1, double lon1, double lat2, double lon2);
};

#endif // CMAKE_CALCULATOR_H