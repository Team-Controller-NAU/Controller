#include "CSim_Utilities.h"

using namespace std;

//function definitions

//generates random string with length of RANDOM_STRING_LEN, returns via outString parameter
void generateRandomString(string* outString)
{
    int randInt, i;
    int min = 97; //97 on ascii table = a
    int max = 132; // 123 on ascii table = z, add 10 to max to account for integers

    //loop through each char of string
    for (i = 0; i < RANDOM_STRING_LEN; i++)
    {
        //get random integer between 97 and 132 (a-z on ascii table + 10 for numbers)
        randInt = min + (rand() % static_cast<int>(max - min + 1));

        if (randInt > 123)
        {
            //assign the corresponding number to this index of the string (0-9)
            (*outString)[i] = (char)(randInt - 75);
        }
        else if (rand() % 2 == 0)
        {
            //assign corresponding uppercase letter to outsring (A-Z)
            (*outString)[i] = (char)(randInt - 32);
        }
        else
        {
            //assign lowercase letter to outstring (a-z)
            (*outString)[i] = (char)(randInt);
        }
    }

    //end string 
    (*outString)[i] = '\0';
}