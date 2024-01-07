#include "csim.h"

//constructor
CSim::CSim(QObject *parent, QString portName)
    : QThread(parent), stop(false), portName(portName)
{
    // Avoid class initialization until thread is running
}

//destructor
CSim::~CSim()
{
    qDebug() << "Terminating controller simulator thread";

    // Stop the thread
    stop = true;

    // Waits for the thread to finish
    wait();
}

//can be called to end the csim event loop
void CSim::stopSimulation()
{
    // Stop the simulation
    stop = true;
}

//starts csim in seperate thread, messages will be sent through given port
void CSim::startCSim(QString portNameInput)
{
    portName = portNameInput;

    if (!isRunning())
    {
        qDebug() << "[CSIM] Starting CSim in seperate thread";
        start();
    }
}

//reads messages from ddm
void CSim::checkConnection(Connection *conn)
{
    // Ensure port is open to prevent possible errors
    if (conn->serialPort.isOpen())
    {
        // Check for message from ddm
        if (conn->serialPort.bytesAvailable() > 0)
        {
            // Get serialized string from port
            QByteArray serializedMessage = conn->serialPort.readAll();

            // Deserialize string
            QString message = QString::fromUtf8(serializedMessage);

            // Extract message id
            SerialMessageIdentifier messageId = static_cast<SerialMessageIdentifier>(QString(message[0]).toInt());

            // Determine what kind of message this is
            switch (messageId)
            {
            case CLOSING_CONNECTION:
                // Log
                qDebug() << "[CSIM] Disconnect message received from DDM. Serial communication halted" << qPrintable("\n");

                // Update flag
                conn->connected = false;

                break;

            case LISTENING:
                // Log
                qDebug() << "[CSIM] DDM listening signal received. Serial communication beginning"<< qPrintable("\n");

                // Send message to begin serial comm
                conn->transmit(QString::number(BEGIN) + DELIMETER + '\n');

                //check for existing event dump message
                if (eventDumpMessage.length() > 0)
                {
                    //send event dump
                    conn->transmit( eventDumpMessage + '\n');

                    //empty event dump
                    eventDumpMessage = "";
                }

                //check for existing error dump message
                if (errorDumpMessage.length() > 0)
                {
                    //send error dump
                    conn->transmit( errorDumpMessage + '\n');

                    //empty error dump
                    errorDumpMessage = "";
                }

                // Update flag
                conn->connected = true;

                break;

            default:
                // Log
                qDebug() << "[CSIM] Unrecognized serial message received: " << message;

                //disconnect, synchronization failed
                conn->connected = false;

                break;
            }
        }
    }
    else
    {
        qDebug() << "[CSIM] Serial port is closed, unable to read data";
    }
}

//this function contains main event loop for simulating weapon controller.
//generates data and sends through serial port at time intervals defined by CSIM_GENERATION_INTERVAL
//alter time interval from constants.h
void CSim::run()
{
    //error handling (isolates potential crash to this thread, and reports reason for crash)
    try
    {
        //Use smart pointers for automatic memory management (resources auto free when function exits)
        std::unique_ptr<Connection> conn(new Connection(portName));
        std::unique_ptr<Status> status(new Status());
        std::unique_ptr<Events> events(new Events());

        // Get time based seed for rng
        qint64 seed = QDateTime::currentMSecsSinceEpoch();

        // Seed the random number generator
        QRandomGenerator randomGenerator(seed);

        //init vars
        int i = 0;
        QString message;
        int eventId = 0;
        stop = false;

        //loop until told to stop by owner of csim handle
        while (!stop)
        {
            i++;

            //qDebug() << "bytes available: " << conn->serialPort.bytesAvailable();

            //log empty line (for output formatting)
            qSetMessagePattern("%{message}");
            qDebug();
            qSetMessagePattern("[%{time h:mm:ss}] %{message}");

            qDebug() << "[CSIM] Iteration: " << i;

            //replace status values with randomized ones
            status->randomize();

            //check conn before transmission
            checkConnection(conn.get());

            //check if currently connected
            if (conn->connected)
            {
                //put message id as first csv in message so ddm can tell this is a status message
                message = QString::number(STATUS) + DELIMETER;

                //generate message using given status values
                message += status->generateMessage();

                //send message through serial port
                conn->transmit(message);
            }

            //clear message
            message = "";

            //20% chance of generating event
            if ( randomGenerator.bounded(1, 6) == 1)
            {
                //check conn before transmission
                checkConnection(conn.get());

                //check if currently connected
                if (conn->connected)
                {
                    //put event message identifier in message
                    message = QString::number(EVENT) + DELIMETER;
                }

                //put event id in message
                message += QString::number(eventId) + DELIMETER;

                //increment event id
                eventId++;

                //put time stamp in message
                message += QTime::currentTime().toString("[hh:mm:ss]") + DELIMETER;

                //put random event message in message
                message += EVENT_MESSAGES[randomGenerator.bounded(0, NUM_EVENT_MESSAGES)] + DELIMETER;

                //check if currently connected
                if (conn->connected)
                {
                    //transmit message through serial port
                    conn->transmit(message +'\n');
                }
                //otherwise, not currently connected
                else
                {
                    //check if event dump is empty
                    if (eventDumpMessage.length() == 0)
                    {
                        //add event dump message identifier
                        eventDumpMessage = QString::number(EVENT_DUMP) + DELIMETER;
                    }

                    //add event to event dump
                    eventDumpMessage += message + DELIMETER;

                    qDebug() << "[CSIM] 1 event added to event dump";
                }

                //clear message
                message = "";
            }

            //20% chance of generating error
            if ( randomGenerator.bounded(1, 6) == 1)
            {
                //check conn before transmission
                checkConnection(conn.get());

                //check if currently connected
                if (conn->connected)
                {
                    //put error message identifier in message
                    message = QString::number(ERROR) + DELIMETER;
                }

                //put event id in message
                message += QString::number(eventId) + DELIMETER;

                //increment event id
                eventId++;

                //put time stamp in message
                message += QTime::currentTime().toString("[hh:mm:ss]") + DELIMETER;

                //put random event message in message
                message += ERROR_MESSAGES[randomGenerator.bounded(0, NUM_ERROR_MESSAGES)] + DELIMETER;

                //check if currently connected
                if (conn->connected)
                {
                    //transmit message through serial port
                    conn->transmit(message + '\n');
                }
                //otherwise, not currently connected
                else
                {
                    //check if error dump is empty
                    if (errorDumpMessage.length() == 0)
                    {
                        //add error dump message identifier
                        errorDumpMessage = QString::number(ERROR_DUMP) + DELIMETER;
                    }

                    //add error to event dump
                    errorDumpMessage += message + DELIMETER;

                    qDebug() << "[CSIM] 1 error added to error dump";
                }
            }

            //wait for 2 seconds while monitoring serial port
            conn->serialPort.waitForReadyRead(2000);
        }
    }
    //error handling triggered, report error
    catch (const std::exception &ex)
    {
        qDebug() << "[CSIM] Exception in CSim::run(): " << ex.what();
    }
}
