#ifndef CONSTANTS_H
#define CONSTANTS_H

#include <QString>
#include <QSerialPortInfo>
#include <QSerialPort>

//this file will define enumerated values and constants used elsewhere in code


//status enums
//------------
//integer length vals are used by csim during random generation
enum TriggerStatus {DISENGAGED=0, ENGAGED=1};
const int NUM_TRIGGER_STATUS = 2;

enum ControllerState {RUNNING=0, BLOCKED=1, TERMINATED=2, SUSPENDED=3};
const int NUM_CONTROLLER_STATE = 4;

enum FiringMode {SAFE=0, SINGLE=1, BURST=2, FULL_AUTO=3};
const int NUM_FIRING_MODE = 4;

enum FeedPosition {FEEDING=0, CHAMBERING=45, LOCKING=90, FIRING=135, UNLOCKING=180, EXTRACTING=225, EJECTING=270, COCKING=315};
const int NUM_FEED_POSITION = 8;
const int FEED_POSITION_INCREMENT_VALUE = 360/NUM_FEED_POSITION;
//------------

enum SerialMessageIdentifier { /*the following identifiers are used to id controller data*/
                               EVENT_DUMP = 0, ERROR_DUMP = 1, ELECTRICAL = 2, EVENT =3, ERROR =4,
                               STATUS =5, CLEAR_ERROR = 6,
                               /*the following identifiers are used for synchronization*/
                               LISTENING = 7, BEGIN = 8, CLOSING_CONNECTION = 9};

//for filtering the event page output
enum EventFilter {ALL, EVENTS, ERRORS, CLEARED_ERRORS, NON_CLEARED_ERRORS};
enum Parity {NO_PARITY, EVEN_PARITY, ODD_PARITY};
enum StopBits {ONE, ONE_AND_A_HALF, TWO};

const QSerialPort::BaudRate INITIAL_BAUD_RATE = QSerialPort::Baud9600;
const QSerialPort::DataBits INITIAL_DATA_BITS = QSerialPort::Data8;
const QSerialPort::Parity INITIAL_PARITY = QSerialPort::NoParity;
const QSerialPort::StopBits INITIAL_STOP_BITS = QSerialPort::OneStop;
const QSerialPort::FlowControl INITIAL_FLOW_CONTROL = QSerialPort::NoFlowControl;

const QString DELIMETER = ",";

//set true to enable handshake timeout after timeout duration has passed if ddm is still not connected
const bool HANDSHAKE_TIMEOUT = false;
const int TIMEOUT_DURATION = 20000; //20 seconds

//rate at which the csim thread will update the status class and send data through serial port (in seconds)
const double CSIM_GENERATION_INTERVAL = 2;

const QString INITIAL_CSIM_PORT = "COM4";
const QString INITIAL_DDM_PORT = "COM5";

const int NUM_EVENT_MESSAGES = 3;
const QString EVENT_MESSAGES[NUM_EVENT_MESSAGES] = {"Sample event message 1", "Sample event message 2; 76; 55.4", "Sample event message 3; 2"};

const int NUM_ERROR_MESSAGES = 3;
const QString ERROR_MESSAGES[NUM_ERROR_MESSAGES] = {"Sample error message 1.22", "Sample error message 2; 5; 0", "Sample error message 3; 677"};

const int NUM_ERROR_DELIMETERS = 3;
const int NUM_EVENT_DELIMETERS = 2;

const int DATA_NOT_FOUND = -101;

#endif // CONSTANTS_H
