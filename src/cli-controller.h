#ifndef CMAKE_CLI_CONTROLLER_H
#define CMAKE_CLI_CONTROLLER_H

#include"data-handling/importer.h"
#include"data-structures/g-tree.h"

class CliController {
    public:
        explicit CliController(){};
        ~CliController(){};

        static void runCli(DataType dataType, GTree* gTree);
        static void runCliEat(DataType dataType);
};

#endif //CMAKE_CLI_CONTROLLER_H