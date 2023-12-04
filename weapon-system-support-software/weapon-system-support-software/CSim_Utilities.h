#ifndef CSIM_UTILS
#define CSIM_UTILS

#include <string>
#include <cstdlib>
#include <iostream>

using namespace std;

//constants
const int RANDOM_STRING_LEN = 6;

//function declarations

//generates random string with length of RANDOM_STRING_LEN, returns via outString parameter
void generateRandomString(string* outString);



#endif