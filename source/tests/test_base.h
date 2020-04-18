#pragma once

//if this is set to 1, test cases will be run on the test cart
#define _TEST 0

//if this flag is set, only failures get printed to console
#define _PRINT_SUCCESS 1

#if _TEST

#include <string>

bool assertStringsEqual(std::string expected, std::string actual, std::string testName);

void printTestOuput(std::string testName, bool success);

#endif