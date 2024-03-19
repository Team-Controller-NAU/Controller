#ifndef CONSTANTS_H
#define CONSTANTS_H

#include <QString>
#include <QtSerialPort/QtSerialPort>
#include <QDir>
#ifdef _WIN32 // check if we are compiling on Windows
#include <QPixMap>
#else
#include <qpixmap.h> // for linux
#endif
#include <QRegularExpression>

//this file will define enumerated values and constants used elsewhere in code

#if DEV_MODE
    const QString QDEBUG_OUTPUT_FORMAT = "[%{time h:mm:ss}] %{message}";

    const QString INITIAL_CSIM_PORT = "COM4";

    //rate at which the csim thread will update the status class and send data through serial port (in seconds)
    const double CSIM_GENERATION_INTERVAL = 2;

    const int NUM_EVENT_MESSAGES = 3;
    const QString EVENT_MESSAGES[NUM_EVENT_MESSAGES] = {"Sample event message 1", "Sample event message 2; 76; 55.4", "Sample event message 3; 2"};

    const int NUM_ERROR_MESSAGES = 3;
    const QString ERROR_MESSAGES[NUM_ERROR_MESSAGES] = {"Sample error message 1.22", "Sample error message 2; 5; 0", "Sample error message 3; 677"};

    const int NUM_ELECTRICAL_MESSAGES = 4;
    const QString ELECTRICAL_MESSAGES[NUM_ELECTRICAL_MESSAGES] = {"Servo Motor, 20, 4",
                                                              "Piston, 14, 29,,Alternator, 96, 3",
                                                              "Servo Motor, 19, 2,,Pump Cooler, 2, 3,,Internal Temp Sensor, 33, 4",
                                                              "Fuel Injector, 27, 1,,Voltage Regulator, 19, 3,,Rotor 1, 9, 13,,Rotor 3, 9, 9,,Exciter Stator, 19, 2"};

    const QString CRC_VERSION = "2F5A1D3E7B9";
    const QString CONTROLLER_VERSION = "6.7.2";
#endif

//status enums
//------------
//integer length vals are used by csim during random generation
enum TriggerStatus {DISENGAGED=0, ENGAGED=1, NA=2};
const int NUM_TRIGGER_STATUS = 3;

enum ControllerState {RUNNING=0, BLOCKED=1, TERMINATED=2, SUSPENDED=3};
const int NUM_CONTROLLER_STATE = 4;

enum FiringMode {SAFE=0, SINGLE=1, BURST=2, FULL_AUTO=3};
const int NUM_FIRING_MODE = 4;

enum FeedPosition {FEEDING=0, CHAMBERING=45, LOCKING=90, FIRING=135, UNLOCKING=180,
                    EXTRACTING=225, EJECTING=270, COCKING=315};
const int NUM_FEED_POSITION = 8;
const int FEED_POSITION_INCREMENT_VALUE = 360/NUM_FEED_POSITION;
//------------

enum SerialMessageIdentifier { /*the following identifiers are used to id controller data*/
                               EVENT_DUMP = 0, ERROR_DUMP = 1, ELECTRICAL = 2, EVENT =3, ERROR =4,
                               STATUS =5, CLEAR_ERROR = 6,
                               /*the following identifiers are used for synchronization*/
                               LISTENING = 7, BEGIN = 8, CLOSING_CONNECTION = 9};

//for filtering the event page output
enum EventFilter {ALL=0, EVENTS=1, ERRORS=2, CLEARED_ERRORS=3, NON_CLEARED_ERRORS=4};
enum Parity {NO_PARITY, EVEN_PARITY, ODD_PARITY};
enum StopBits {ONE, ONE_AND_A_HALF, TWO};

//initial values for user settings
const QSerialPort::BaudRate INITIAL_BAUD_RATE = QSerialPort::Baud9600;
const QSerialPort::DataBits INITIAL_DATA_BITS = QSerialPort::Data8;
const QSerialPort::Parity INITIAL_PARITY = QSerialPort::NoParity;
const QSerialPort::StopBits INITIAL_STOP_BITS = QSerialPort::OneStop;
const QSerialPort::FlowControl INITIAL_FLOW_CONTROL = QSerialPort::NoFlowControl;
const bool INITIAL_COLORED_EVENTS_OUTPUT = true;
const QString INITIAL_LOGFILE_LOCATION = "WSSS_Logfiles/";
const int INITIAL_AUTO_SAVE_LIMIT = 5;
const QString INITIAL_DDM_PORT = "COM5";

//delimeter in serial messages
const QString DELIMETER = ",";

//rate at which handshake messages are sent
const int HANDSHAKE_INTERVAL = 2000;
const int ONE_SECOND = 1000;

// the max number of electrical componenets to expect from a weapon
const int MAX_ELECTRICAL_COMPONENTS = 120;

//for checking message integrity within loadData functions
const int NUM_ERROR_DELIMETERS = 3;
const int NUM_EVENT_DELIMETERS = 2;
const int NUM_ELECTRIC_DELIMETERS = 2;

//error codes
const int DATA_NOT_FOUND = -101;
const int INCORRECT_FORMAT = -102;
const int SUCCESS = 1;

#endif // CONSTANTS_H
