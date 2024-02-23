#include "csim.h"

//constructor
CSim::CSim(QObject *parent, QString portName)
    : QThread(parent), stop(false), portName(portName),
    connPtr(nullptr), eventsPtr(nullptr), startupTime(QDateTime::currentMSecsSinceEpoch()),
    secondTrigger(true)
{
    // Avoid class initialization until thread is running
}

//destructor
CSim::~CSim()
{
    qDebug() << "Terminating controller simulator thread";

    // Stop the thread
    stop = true;
    quit();

    // clear dump messages
    eventDumpMessage = "";
    errorDumpMessage = "";

    // Waits for the thread to finish
    wait();
}

//slot, clears error then deletes from events class + sends clear error message to ddm,
//can be called by csim as part of routine operation or can be called by user through ddm developer interface
void CSim::clearError(int clearedId)
{
    //if events ptr is null, return
    if (eventsPtr == nullptr)
    {
        qDebug() << "Unable to clear error, no events class declared";
        return;
    }

    //free the error from the linked list
    eventsPtr->freeError(clearedId);

    //transmit error cleared message to ddm
    connPtr->transmit(QString::number(CLEAR_ERROR) + DELIMETER + QString::number(clearedId) + DELIMETER);

    //store message
    messagesSent += QString::number(CLEAR_ERROR) + DELIMETER + QString::number(clearedId) + DELIMETER;
}

//slot connected to transmissionRequest signal in ddm,
//will simply send the given message through csims serial port
void CSim::completeTransmissionRequest(const QString &message)
{
    // Check if conn is active
    if (connPtr != nullptr)
    {
        qDebug() << "[CSIM] transmission request received from DDM.";

        // Split the message into individual lines manually
        QStringList lines = message.split('\n', Qt::SkipEmptyParts);

        // Transmit each line in the list
        for (const QString& line : lines)
        {
            // Append newline character to each line
            QString lineWithNewline = line + "\n";

            // Transmit the line
            connPtr->transmit(lineWithNewline);

            // Store line
            messagesSent += lineWithNewline;
        }

        return;
    }

    qDebug() << "[CSIM] transmission request declined, no active CSIM connection";
}


//can be called to end the csim event loop
void CSim::stopSimulation()
{
    qDebug() << "Terminating controller simulator thread";

    // Stop the thread
    stop = true;
    quit();

    // clear dump messages
    eventDumpMessage = "";
    errorDumpMessage = "";

    // reset startup time
    startupTime = 0;
}

//starts csim in seperate thread, messages will be sent through given port
void CSim::startCSim(QString portNameInput)
{
    portName = portNameInput;

    if (!isRunning())
    {
        // notify
        qDebug() << "[CSIM] Starting CSim in seperate thread";

        // reset startup time
        startupTime = QDateTime::currentMSecsSinceEpoch();

        //sets up thread and calls run()
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

            electrical *elec(new electrical());

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

                // Send message to begin serial comm (controller version and crc are included in the begin message)
                conn->transmit(QString::number(BEGIN) + DELIMETER + getTimeStamp() + DELIMETER + CONTROLLER_VERSION + DELIMETER + CRC_VERSION + DELIMETER + '\n');

                //store message
                messagesSent += QString::number(BEGIN) + DELIMETER + getTimeStamp() + DELIMETER + CONTROLLER_VERSION + DELIMETER + CRC_VERSION + DELIMETER + '\n';

                // transmit the electrial signal
                conn->transmit(QString::number(ELECTRICAL) + DELIMETER + ELECTRICAL_MESSAGES + '\n');

                //store message
                messagesSent += QString::number(ELECTRICAL) + DELIMETER + ELECTRICAL_MESSAGES + '\n';

                //check for existing event dump message
                if (eventDumpMessage.length() > 0)
                {
                    //send event dump
                    conn->transmit( eventDumpMessage + '\n');

                    //store message
                    messagesSent += eventDumpMessage + '\n';

                    //empty event dump
                    eventDumpMessage = "";
                }

                //check for existing error dump message
                if (errorDumpMessage.length() > 0)
                {
                    //send error dump
                    conn->transmit( errorDumpMessage + '\n');

                    //store message
                    messagesSent += errorDumpMessage + '\n';

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

//returns a qstring containing the time since start up in D:H:M:S
QString CSim::getTimeStamp()
{
    // Calculate elapsed time since startup
    qint64 elapsedTime = QDateTime::currentMSecsSinceEpoch() - startupTime;

    // Convert milliseconds to days, hours, minutes, and seconds
    int days = elapsedTime / (1000 * 60 * 60 * 24);
    int hours = (elapsedTime % (1000 * 60 * 60 * 24)) / (1000 * 60 * 60);
    int minutes = (elapsedTime % (1000 * 60 * 60)) / (1000 * 60);
    int seconds = (elapsedTime % (1000 * 60)) / 1000;

    // Format the timestamp as "D:H:M:S" and return
    return QString("%1:%2:%3:%4").arg(days).arg(hours, 2, 10, QLatin1Char('0')).arg(minutes, 2, 10,
                                     QLatin1Char('0')).arg(seconds, 2, 10, QLatin1Char('0'));
}

//this function contains main event loop for simulating weapon controller.
//generates data and sends through serial port at time intervals defined by CSIM_GENERATION_INTERVAL
//alter time interval from constants.h
void CSim::run()
{
    //error handling (isolates potential crash to this thread, and reports reason for crash)
    try
    {
        //init connection
        Connection *conn(new Connection(portName));
        connPtr = conn;

        //init events class (csim only uses this to store non cleared errors so that it can
        //clear them later)
        Events *events(new Events());
        eventsPtr = events;


        //for status use smart pointer for automatic memory management (resources auto free when function exits)
        std::unique_ptr<Status> status(new Status());

        //manually set feed position to starting val
        status->feedPosition = FEEDING;

        // Get time based seed for rng
        qint64 seed = QDateTime::currentMSecsSinceEpoch();

        // Seed the random number generator
        QRandomGenerator randomGenerator(seed);

        //init vars
        int i = 0;
        QString message;
        int eventId = 0;
        stop = false;

        //variables for saving non-cleared errors
        bool cleared;
        QString timeStamp;
        QString errorMessage;
        int clearedId;

        conn->connected = false;

        //loop until told to stop by owner of csim handle
        while (!stop)
        {
            i++;

            logEmptyLine();

            qDebug() << "[CSIM] Iteration: " << i;

            //replace status values with randomized ones
            status->randomize(secondTrigger);

            //check conn before transmission
            checkConnection(conn);

            //check if currently connected
            if (conn->connected)
            {
                //put message id as first csv in message so ddm can tell this is a status message
                message = QString::number(STATUS) + DELIMETER;

                //generate message using given status values
                message += status->generateMessage();

                //send message through serial port
                conn->transmit(message);

                //store message
                messagesSent += message;
            }

            //clear message
            message = "";

            //20% chance of generating event
            if ( randomGenerator.bounded(1, 6) == 1)
            {
                //check conn before transmission
                checkConnection(conn);

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
                message += getTimeStamp() + DELIMETER;

                //put random event message in message
                message += EVENT_MESSAGES[randomGenerator.bounded(0, NUM_EVENT_MESSAGES)] + DELIMETER;

                //check if currently connected
                if (conn->connected)
                {
                    //transmit message through serial port
                    conn->transmit(message +'\n');

                    //store message
                    messagesSent += message + '\n';
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
            //TEMPORARILY SET TO GENERATE ERROR EACH ITERATION
            if ( randomGenerator.bounded(1, 6) == 1)
            {
                //check conn before transmission
                checkConnection(conn);

                //check if currently connected
                if (conn->connected)
                {
                    //put error message identifier in message
                    message = QString::number(ERROR) + DELIMETER;
                }

                //put event id in message
                message += QString::number(eventId) + DELIMETER;

                //generate time stamp
                timeStamp = QTime::currentTime().toString("[hh:mm:ss]");

                //put time stamp in message
                message += getTimeStamp() + DELIMETER;

                //get random error message
                errorMessage = ERROR_MESSAGES[randomGenerator.bounded(0, NUM_ERROR_MESSAGES)];

                //put error message in message
                message += errorMessage + DELIMETER;

                //set cleared / not cleared
                cleared = randomGenerator.bounded(0,2);

                // append cleared val
                message += QString::number(cleared) + DELIMETER;

                //if cleared is false, store error in case of later clear
                if (!cleared)
                {
                    events->addError(eventId, timeStamp, errorMessage, cleared);
                }

                //increment event id
                eventId++;

                //check if currently connected
                if (conn->connected)
                {
                    //transmit message through serial port
                    conn->transmit(message + '\n');

                    //store message
                    messagesSent += message + '\n';
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

            //10% chance of clearing an error (if no errors, bypass)
            if (randomGenerator.bounded(1, 11) == 1 && events->totalErrors > 0)
            {
                //pick a random error to clear
                clearedId = events->getErrorIdByPosition(randomGenerator.bounded(0,events->totalErrors));

                //check if data found
                if (clearedId != DATA_NOT_FOUND)
                {
                    //remove error from linked list and transmit clear error msg to ddm
                    clearError(clearedId);
                }

            }

            //check for signals from ddm
            QCoreApplication::processEvents();

            //wait for 2 seconds while monitoring serial port
            conn->serialPort.waitForReadyRead(2000);

        } //end main execution loop

        //if connected, add disconnect message to messagesSent (sent in destructor)
        if (conn->connected)
        {
            messagesSent += QString::number(static_cast<int>(CLOSING_CONNECTION)) + DELIMETER + '\n';
        }

        //free connection
        delete conn;
        connPtr = nullptr;

        //free events class
        delete events;
        eventsPtr = nullptr;
    }
    //error handling triggered, report error
    catch (const std::exception &ex)
    {
        qDebug() << "[CSIM] Exception in CSim::run(): " << ex.what();
    }
}

//outputs messages sent this session, can be called via dev tools page
void CSim::outputMessagesSent()
{
    logEmptyLine();

    qDebug() << "[CSIM] Messages sent from " << portName << "this session:\n" << messagesSent;
}

//logs empty line to qdebug
void CSim::logEmptyLine()
{
    //revert to standard output format
    qSetMessagePattern("%{message}");

    //log empty line
    qDebug();

    //enable custom message format
    qSetMessagePattern(QDEBUG_OUTPUT_FORMAT);
}
