#include "test_base.h"

#if _TEST


#include <string>
#include <sstream>

#include "../logger.h"

bool assertStringsEqual(std::string expected, std::string actual, std::string testName){
    bool equal = expected == actual;
    printTestOuput(testName, equal);

    if (!equal){
        Logger::Write("start expected: \n");
        Logger::Write(expected.c_str());
        Logger::Write("end expected: \n");

        Logger::Write("start actual: \n");
        Logger::Write(actual.c_str());
        Logger::Write("end actual: \n");
    }
    
    return equal;
}

void printTestOuput(std::string testName, bool success) {
    #if _PRINT_SUCCESS
    bool print = true;
    #else
    bool print = !success;
    #endif
    
    if (print) {
        std::stringstream output;
        std::string status = success ? "PASS" : "FAIL";
        output <<  testName << ": " << status << "\n";
        printf(output.str().c_str());
    }
}
#endif