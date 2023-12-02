#ifndef CLASSES
#define CLASSES

//This header file defines data types structures constants and classes which are shared between CSim and DataDisplay

#include <string>
//#include <boost/asio.hpp>
#include <iostream>

//check if compiling on windows
#ifdef _WIN32
    #include <windows.h> // Include the Windows API header
#endif

using namespace std;

//enums
typedef enum { ENGAGED, DISENGAGED, NA } TriggerStatusType;
typedef enum { INITIALIZING, STANDBY, FIRING, WEAPON_ERROR, OFFLINE, CONTROLLER_ERROR } ControllerStateType;
typedef enum { SINGLE, AUTOMATIC, BURST, SAFE } FiringModeType;

#ifdef _WIN32
// Define color constants for terminal outputs (windows exclusive)
enum ConsoleColor {
    BLACK = 0,
    BLUE = 1,
    GREEN = 2,
    CYAN = 3,
    RED = 4,
    MAGENTA = 5,
    YELLOW = 6,
    WHITE = 7
};
#endif

struct ElectricalComponent 
   {
    string name;
    int voltage;
    int current;
   };

class Status
   {
    bool armed;
    TriggerStatusType trigger1;
    TriggerStatusType trigger2;
    string controllerVersion;
    string controllerCRC;
    ControllerStateType state;
    int baudRate;
    int totalErrors;
    int totalFiringEvents;
    int totalEvents;
    FiringModeType firingMode;
    int feedPosition;
    ElectricalComponent component;

    //void updateGUI();
    //string serializeStatus();
    //string deserializeStatus();

   };

/*void Status::updateGUI()
   {

   }
string Status::serializeStatus()
   {

   }

string Status::deserializeStatus()
   {

   }*/

#endif