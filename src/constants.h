#ifndef CMAKE_HEADER_H
#define CMAKE_HEADER_H

#include <string>

const string FOLDER_PREFIX = "";
// const string FOLDER_PREFIX = "../../";

constexpr int SECONDS_PER_DAY = 86400;
constexpr int MIN_COARSE_GRAPH_VERTEX_DIFF = 10;
constexpr int KL_ITERATIONS = 100;
constexpr int KL_MAX_UNUSED_SWAPS = 300;

constexpr int LOWER_BOUND_FACTOR = 1;
constexpr bool USE_LANDMARKS = true;

constexpr double GTREE_APPROXIMATION_ALPHA = 1;

constexpr int NUMBER_OF_DAYS = 4;

constexpr bool USE_FOOTPATHS = true;

#endif //CMAKE_HEADER_H