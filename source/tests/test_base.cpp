#include "test_base.h"

#if _TEST


#include <string>
#include <sstream>

#include "../logger.h"

bool assertStringsEqual(std::string expected, std::string actual, std::string testName){
    bool equal = expected == actual;
    printTestOuput(testName, equal);

    if (!equal){
        Logger_Write("start expected: \n");
        Logger_Write(expected.c_str());
        Logger_Write("end expected: \n");

        Logger_Write("start actual: \n");
        Logger_Write(actual.c_str());
        Logger_Write("end actual: \n");
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