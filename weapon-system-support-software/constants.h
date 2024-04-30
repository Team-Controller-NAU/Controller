#ifndef CONSTANTS_H
#define CONSTANTS_H

#include <QString>
#include <QtSerialPort/QtSerialPort>

/********************************************************************************
** constants.h
**
** This file will define enumerated values and constants used elsewhere in code
**
** @author Team Controller
********************************************************************************/

//======================================================================================
// General
//======================================================================================

// delimeter in serial messages
const QString DELIMETER = ",";

/**
 * These are the strings which denote cleared versus not cleared errors
 * in the log file. In order to make them interchangeable in the log file,
 * we have to add spaces to the end of the shorter string. This is done in
 * the events constructor.
 */
const QString CLEARED_INDICATOR = "CLEARED";
const QString ACTIVE_INDICATOR = "ACTIVE";

// denotes advanced log file entries
const QString ADVANCED_LOG_FILE_INDICATOR = "***";

// minimum value allowed to be set for max data nodes in user settings
const int MIN_DATA_NODES_BEFORE_RAM_CLEAR = 1500;

// minimum value allowed for timeout duration
const int MIN_TIMEOUT_DURATION = 200;

// number of bytes in a newline - linux uses \n =1 byte windows uses \r\n = 2 bytes
#ifdef Q_OS_LINUX
const int NEW_LINE_SIZE = 1;
#else
const int NEW_LINE_SIZE=2;
#endif

//======================================================================================
// Enumerations and enum related vals
//======================================================================================

// these integer vals denote the trigger status values
enum TriggerStatus {DISENGAGED=0, ENGAGED=1, NA=2};

// denotes the trigger status names and amount of names possible
const int NUM_TRIGGER_STATUS = 3;
const QString TRIGGER_STATUS_NAMES[NUM_TRIGGER_STATUS]{"Disengaged", "Engaged", "N/A"};

//======================================================================================

// these integer vals denote the controller state values
enum ControllerState {RUNNING=0, BLOCKED=1, TERMINATED=2, SUSPENDED=3};

// denotes the controller state names and amount of names possible
const int NUM_CONTROLLER_STATE = 4;
const QString CONTROLLER_STATE_NAMES[NUM_CONTROLLER_STATE]{"Running", "Blocked", "Terminated", "Suspended"};

//======================================================================================

// armed is a bool value, not an enumeration - only names are provided
const QString ARMED_NAMES[2]{"Disarmed", "Armed"};

//======================================================================================

// these integer vals denote the firing mode values
enum FiringMode {SAFE=305, SINGLE=330, BURST=30, FULL_AUTO=55};

// denotes the firing mode names and amount of names possible
const int NUM_FIRING_MODE = 4;
const QString FIRING_MODE_NAMES[NUM_FIRING_MODE]{"Safe", "Single", "Burst", "Auto"};

//======================================================================================

// these integer vals denote the feed position values
enum FeedPosition {CHAMBERING=0, LOCKING=45, FIRING=90, UNLOCKING=135,
                    EXTRACTING=180, EJECTING=225, COCKING=270, FEEDING=315};

// denotes the feed position names and amount of names possible
const int NUM_FEED_POSITION = 8;
const QString FEED_POSITION_NAMES[NUM_FEED_POSITION]{ "Chambering", "Locking", "Firing","Unlocking",
                                                     "Extracting", "Ejecting", "Cocking", "Feeding"};
const int FEED_POSITION_INCREMENT_VALUE = 360/NUM_FEED_POSITION;

//======================================================================================

/**
 * These integer vals denote the serial message identifiers so that we can identify what
 * type of message has been received through the serial port, assuming the real controller
 * has a way to identify the type of message.
 */
enum SerialMessageIdentifier { /*the following identifiers are used to id controller data*/
                               EVENT_DUMP = 0, ERROR_DUMP = 1, ELECTRICAL = 2, EVENT =3, ERROR =4,
                               STATUS =5, CLEAR_ERROR = 6,
                               /*the following identifiers are used for synchronization*/
                               LISTENING = 7, BEGIN = 8, CLOSING_CONNECTION = 9};

//======================================================================================

// these integer vals denote what filters are available in the GUI
enum EventFilter {ALL=0, EVENTS=1, ERRORS=2, CLEARED_ERRORS=3, NON_CLEARED_ERRORS=4};

// denotes the event filter names and amount of names possible
const int NUM_EVENT_FILTER = 5;
const QString EVENT_FILTER_NAMES[NUM_EVENT_FILTER]{"All", "Events", "Errors", "Cleared Errors", "Active Errors"};

//======================================================================================

/**
 * More enumerations for the GUI.
 * Names for these enumerations are handled by the toString and fromString methods
 * in the mainwindow_connection_settings.cpp file
 */
enum Parity {NO_PARITY, EVEN_PARITY, ODD_PARITY};
enum StopBits {ONE, ONE_AND_A_HALF, TWO};

//======================================================================================
// Initial settings
//======================================================================================

/**
 * Initial values for user settings.
 * Upon first startup of the application, these values are saved to the registry
 * as default settings until they are modified by the user.
 */
// serial connection settings
const QSerialPort::BaudRate INITIAL_BAUD_RATE = QSerialPort::Baud9600;
const QSerialPort::DataBits INITIAL_DATA_BITS = QSerialPort::Data8;
const QSerialPort::Parity INITIAL_PARITY = QSerialPort::NoParity;
const QSerialPort::StopBits INITIAL_STOP_BITS = QSerialPort::OneStop;
const QSerialPort::FlowControl INITIAL_FLOW_CONTROL = QSerialPort::NoFlowControl;
const QString INITIAL_DDM_PORT = "COM5";
const int INITIAL_CONNECTION_TIMEOUT = 20000; // time out the connection if a message has not been received in 20sec

// personalization settings
const bool INITIAL_COLORED_EVENTS_OUTPUT = true;
const bool INITIAL_ADVANCED_LOG_FILE = false;
const bool INITIAL_NOTIFY_ON_ERROR_CLEARED = false;
const bool INITIAL_RAM_CLEARING = false; // used for improving CPU performance with large amount of nodes
const int INITIAL_MAX_DATA_NODES = 10000;

// logfile settings
const QString INITIAL_LOGFILE_LOCATION = "WSSS_Logfiles/";
const int INITIAL_AUTO_SAVE_LIMIT = 5;

//======================================================================================
// Timer vals (ints represent msec)
//======================================================================================

// rate at which handshake messages are sent
const int HANDSHAKE_INTERVAL = 2000;

// time allowed for controller to send final messages before disconnect occurs
// this is used when user clicks disconnect button
const int DISCONNECT_GRACE_PERIOD = 500;

// added for better readability
const int ONE_SECOND = 1000;

// time before clearing notification pop ups
const int NOTIFICATION_DURATION = 3000;

// cooldown for preventing spamming of handshake button (spamming causes crash)
const int HANDSHAKE_COOLDOWN_TIME = 200;

// default time format - some conversion methods rely on being in this format
const QString TIME_FORMAT = "HH:mm:ss";

//======================================================================================
// Load data integrity checks
//======================================================================================

// number of parts to expect from an error message (id, timestamp, message, status)
const int NUM_ERROR_ELEMENTS = 4;

// number of parts to expect from an event message (id, timestamp, message)
const int NUM_EVENT_ELEMENTS = 3;

const int NUM_ELECTRIC_ELEMENTS = 2; // name, voltage
const int NUM_BEGIN_ELEMENTS = 3;
const int NUM_STATUS_ELEMENTS = 9;

//======================================================================================
// Integer codes
//======================================================================================

// used to identify error messages that may occur in the program
const int DATA_NOT_FOUND = -101;
const int INCORRECT_FORMAT = -102;
const int UNINITIALIZED = -103;

// return codes for clearError and clearErrorInLogFile
const int FAILED_TO_CLEAR_FROM_LOGFILE = -104;
const int FAILED_TO_CLEAR= -105;
const int FAILED_TO_CLEAR_FROM_LL = -106;

//return codes for check for valid message
const int VALID_MESSAGE=100;
const int EMPTY_BUFFER = -107;
const int UNTERMINATED_MESSAGE = -108;

// default success code
const int SUCCESS = 1;

//======================================================================================
// GUI styles
//======================================================================================

// navbar CSS properties
const QString NAV_BUTTON_STYLE = "QPushButton {color: rgb(255, 255, 255);background-color: rgb(39, 39, 39);font: 16pt Segoe UI;} "
                                 "QPushButton:hover {color: #9747FF;}";

const QString SELECTED_NAV_BUTTON_STYLE = "color: rgb(210, 210, 210);background-color: #9747FF;font: 16pt Segoe UI; ";

// jpg resource paths
const QString SETTINGS_ICON = "border-image: url(://resources/Images/whiteSettings.png)";
const QString NOTIFICATIONS_ICON = "border-image: url(://resources/Images/notificationBell.png);";
const QString SELECTED_SETTINGS_ICON = "border-image: url(://resources/Images/purpleSettings.png)";
const QString SELECTED_NOTIFICATIONS_ICON = "border-image: url(://resources/Images/purpleNotificationBell.png);";
const QString URGENT_NOTIFICATION_ICON = "border-image: url(://resources/Images/newNotification.png);";

// colored output will apply these styles (toggle off to apply event color to all nodes)
const QString EVENT_OUTPUT_SIZE = "16"; //px
const QString EVENT_COLOR = "rgb(255, 255, 255)";
const QString CLEARED_ERROR_COLOR = "rgb(20, 174, 92)"; //changes here must also be made in clearErrorFromEventsOutput found in mainwindow.cpp
const QString ACTIVE_ERROR_COLOR = "rgb(254, 28, 28)";

// properties applied to notifications
const QString INVISIBLE = "background-color: transparent; border: none;";
const QString NOTIFICATION_SIZE = "16"; //px
const QString NOTIFICATION_TIMESTAMP_STYLE = "color: white; font-size: 16px";
const QString ERROR_COLOR = "red";
const QString STANDARD_COLOR = "green";
const QString POP_UP_STYLE = "color: white; text-align: center; font-size: 16px;";

// handshake button CSS properties
const QString CONNECTING_STYLE = "QPushButton { padding-bottom: 3px; color: rgb(255, 255, 255); background-color: "
                                 "#FF7518; border: 1px solid; border-color: #e65c00; font: 15pt 'Segoe UI'; } "
                                 "QPushButton::hover { background-color: #ff8533; } "
                                 "QPushButton::pressed { background-color: #ffa366;}";

const QString CONNECTED_STYLE = "QPushButton { padding-bottom: 3px; color: rgb(255, 255, 255); background-color: "
                                "#FE1C1C; border: 1px solid; border-color: #cb0101; font: 15pt 'Segoe UI'; } "
                                "QPushButton::hover { background-color: #fe3434; } "
                                "QPushButton::pressed { background-color: #fe8080;}";

const QString DISCONNECTED_STYLE = "QPushButton { padding-bottom: 3px; color: rgb(255, 255, 255); background-color: "
                                   "#14AE5C; border: 1px solid; border-color: #0d723c; font: 15pt 'Segoe UI'; } "
                                   "QPushButton::hover { background-color: #1be479; } "
                                   "QPushButton::pressed { background-color: #76efae;}";


// electrical page boxes CSS properties
const QString ELECTRICAL_BOX_HEADER_STYLE = "font: 20pt 'Segoe UI'; background-color: rgb(179, 179, 179);"
                                            "color: black;";

const QString ELECTRICAL_BOX_CONTENT_STYLE = "color: rgb(30, 30, 30); font: 20pt 'Segoe UI';"
                                             "background-color: rgb(105, 105, 105); color: white;";


//======================================================================================
// CSIM exclusive constants
//======================================================================================
#if DEV_MODE
//adds timestamps to qdebug outputs
const QString QDEBUG_OUTPUT_FORMAT = "[%{time h:mm:ss}] %{message}";

const QString INITIAL_CSIM_PORT = "COM4";

//initial rate at which the csim thread will update the status class and send data through serial port (in msec)
const int CSIM_GENERATION_INTERVAL = 2000;

const int NUM_EVENT_MESSAGES = 3;
const QString EVENT_MESSAGES[NUM_EVENT_MESSAGES] = {"Sample event message 1", "Sample event message 2; 76; 55.4", "Sample event message 3; 2"};

const int NUM_ERROR_MESSAGES = 3;
const QString ERROR_MESSAGES[NUM_ERROR_MESSAGES] = {"Sample error message 1.22", "Sample error message 2; 5; 0", "Sample error message 3; 677"};

const int NUM_ELECTRICAL_MESSAGES = 4;
const QString ELECTRICAL_MESSAGES[NUM_ELECTRICAL_MESSAGES] = {"Servo Motor, 20.2, 4.9",
                                                              "Piston, 14.33, 29.1,,Alternator, 96.5, 3.33,,Servo Motor, 20.2, 4.9",
                                                              "Servo Motor, 19.1, 2.34,,Pump Cooler, 2.6, 3.7,,Internal Temp Sensor, 33.9, 4.2,,Alternator, 96.5, 3.33",
                                                              "Fuel Injector, 27.5, 1.12,,Alternator, 96.5, 3.33,,Voltage Regulator, 19.4, 3.3,,Servo Motor, 20.2, 4.9,,"
                                                              "Rotor 3.22, 9.3, 9.7,,Piston, 14.33, 29.1,,Alternator, 96.5, 3.33,,Exciter Stator, 19.11, 2.72,,"
                                                              "sample component, 19.11, 2.72,,sample component, 11, 2.72,,sample component, 20.2, 2.72,,"
                                                              "sample component, 19.11, 96.5,,sample component, 19.11, 3.33"};

const QString TEST_LOG_FILE = "/testLogFile.txt";

const QString CRC_VERSION = "2F5A1D3E7B9";
const QString CONTROLLER_VERSION = "6.7.2";
#endif // DEV_MODE

#endif // CONSTANTS_H
