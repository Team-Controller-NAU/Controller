#ifndef CONSTANTS_H
#define CONSTANTS_H

#include <QString>

//this file will define enumerated values and constants used elsewhere in code

enum TriggerStatus {DISENGAGED=0, ENGAGED=1};
const int NUM_TRIGGER_STATUS = 2;

enum ControllerState {RUNNING=0, BLOCKED=1, TERMINATED=2, SUSPENDED=3};
const int NUM_CONTROLLER_STATE = 4;

enum FiringMode {SAFE=0, SINGLE=1, BURST=2, FULLAUTO=3};
const int NUM_FIRING_MODE = 4;

enum FeedPosition {FEEDING=0, CHAMBERING=45, LOCKING=90, FIRING=135, UNLOCKING=180, EXTRACTING=225, EJECTING=270, COCKING=315};
const int NUM_FEED_POSITION = 8;
const int FEED_POSITION_INCREMENT_VALUE = 360/NUM_FEED_POSITION;

enum SerialMessageIdentifier {EVENT_DUMP = 0, ERROR_DUMP = 1, ELECTRICAL = 2, EVENT =3, ERROR =4, STATUS =5};

const QString DELIMETER = ",";

//rate at which the csim thread will update the status class and send data through serial port in seconds
const int CSIM_GENERATION_INTERVAL = 2;

const QString CSIM_PORT = "COM4";
const QString DDM_PORT = "COM5";

#endif // CONSTANTS_H
