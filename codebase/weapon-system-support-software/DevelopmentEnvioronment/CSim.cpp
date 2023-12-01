#include "Classes.hpp"

/*
CSim = basic command line utility
it is used by developers, so minimal input check is necessary

will need a supporting header file to define a library of events, errors and parameters to be 
chosen from during random generation

Upon startup...
    *launch connection and wait for handshake
    prompt: "\n\nWelcome to CSim\n"
            "-------------------\n"
            "This is a testing tool for the corresponding data display module, which must be run in parallel.\n"
            "For instructions on setting up a serial testing environment please visit *insert website link which has tutorial or maybe pdf included in installation?*"
            "Purpose: generate status information, and events/errors to send via RS-422 serial communication to\n"
            "         the data display module.\n\n"
            "This program will attempt to connect to the data display module via\n"
            *show user connection settings*

            "Program Start:\n--------------\n
            "Input options: 
                1 to start random generation
                2 to change random generation rate
                3 to input custom event
                4 to input custom error"
            
            "waiting for handshake..."
       then     
            "connection established"

user responds 1:
    prompt "press q at any point to stop data generation"
           "starting in 2 seconds..."

logs activity to screen, examples:

     -new event "xyz 1 2 3"
     -new status generated:
        *log generated data
     -Data display module disconnected!

other than option 1, simply prompt for input then return to main menu use do while loops for inputs
each time the user enters a new menu add spacing with a few \n then print the menu title before printing menu contents


*/
using namespace std;

int main() {
    //declare classes

    //initialize connection class (csim will listen for a handshake from ddm)

    //print welcome 
    cout << "\n\nWelcome to CSim\n-------------------\n";
    cout << "This is a testing tool for the corresponding data display module, which must be run in parallel.\n";
    cout << "For instructions on setting up a serial testing environment please visit *insert website link which has tutorial or maybe pdf included in installation?*\n";
    cout << "Purpose: generate status information, and events/errors to send via RS-422 serial communication to the data display module.\n\n";
    cout << "This program will attempt to connect to the data display module via\n*show user connection settings*";

    //call function to print connection settings

    cout << "Program Start:\n--------------\n";
    cout << "Input options:\n 1 to start random generation\n 2 to change random generation rate\n 3 to input custom event\n 4 to input custom error\n\n";

    //take user input
    return 0;
}