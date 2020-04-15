#include "test_base.h"

#if _TEST


#include <string>
#include <sstream>

bool assertStringsEqual(std::string expected, std::string actual, std::string testName){
    bool equal = expected == actual;
    printTestOuput(testName, equal);
    
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