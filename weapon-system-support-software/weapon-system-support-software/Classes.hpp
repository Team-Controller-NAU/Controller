//This header file defines data types structures constants and classes which are shared between CSim and DataDisplay

#include <string>
//#include <boost/asio.hpp>
#include <iostream>
 
using namespace std;

typedef enum { ENGAGED, DISENGAGED, NA } TriggerStatusType;
typedef enum { INITIALIZING, STANDBY, FIRING, WEAPON_ERROR, OFFLINE, CONTROLLER_ERROR } ControllerStateType;
typedef enum { SINGLE, AUTOMATIC, BURST, SAFE } FiringModeType;

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

    void updateGUI();
    string serializeStatus();
    string deserializeStatus();

   };

void Status::updateGUI()
   {

   }
string Status::serializeStatus()
   {

   }

string Status::deserializeStatus()
   {
    
   }