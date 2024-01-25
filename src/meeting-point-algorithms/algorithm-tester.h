#ifndef CMAKE_ALGORITHM_TESTER_H
#define CMAKE_ALGORITHM_TESTER_H

class NaiveAlgorithmTester {
    public:
        explicit NaiveAlgorithmTester(){};
        ~NaiveAlgorithmTester(){};

        static void testNaiveAlgorithm(int numberOfQueries, int numberOfSources, bool printTime = false);
};

#endif //CMAKE_ALGORITHM_TESTER_H