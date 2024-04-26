#ifndef CMAKE_LOWER_BOUND_TESTER_H
#define CMAKE_LOWER_BOUND_TESTER_H

class LowerBoundTester {
    public:
        explicit LowerBoundTester(){};
        ~LowerBoundTester(){};

        static void getLowerBoundDiffs(int numberOfSources, int numberOfTargetsPerSource);
};

#endif //CMAKE_LOWER_BOUND_TESTER_H