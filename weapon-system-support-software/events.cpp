#include "events.h"
#include "constants.h"


Events::Events()
{
    //initialize variables
    totalEvents = 0;
    totalErrors= 0;
    totalNodes= 0;

    headEventNode= nullptr;
    lastEventNode= nullptr;

    headErrorNode= nullptr;
    lastErrorNode= nullptr;
}

Events::~Events()
{
    qDebug() << "Deleting event and error data";
    freeLinkedLists();
}

//add an event node to the end of event ll
void Events::addEvent(int id, QString timeStamp, QString eventString)
{
    //allocate memory for node
    EventNode *newNode = new EventNode;

    //assign node values
    newNode->timeStamp = timeStamp;
    newNode->eventString = eventString;
    newNode->nextPtr = nullptr;
    newNode->id = id;

    //increment counters
    totalNodes++;
    totalEvents++;

    //log: new event created
    qDebug() << "New event node created. Total nodes: " << totalNodes;

    //check if linked list is currently empty
    if (headEventNode == nullptr)
    {
        //assign new node as head of list
        headEventNode = newNode;

        //assign tail ptr
        lastEventNode = newNode;
    }
    //otherwise, ll is not empty
    else
    {
        //append node to list
        lastEventNode->nextPtr = newNode;

        lastEventNode = newNode;
    }
}

//add and error node to the end of error ll
void Events::addError(int id, QString timeStamp, QString eventString, bool cleared)
{
    //allocate memory for node
    EventNode *newNode = new EventNode;

    //assign node values
    newNode->id = id;
    newNode->timeStamp = timeStamp;
    newNode->eventString = eventString;
    newNode->cleared = cleared;
    newNode->nextPtr = nullptr;

    //increment counters
    totalNodes++;
    totalErrors++;


    //check if linked list is currently empty
    if (headEventNode == nullptr)
    {
        //assign new node as head of list
        headEventNode = newNode;

        //assign tail ptr
        lastEventNode = newNode;
    }
    //otherwise, ll is not empty
    else
    {
        //append node to list
        lastEventNode->nextPtr = newNode;

        lastEventNode = newNode;
    }

    //log: new err created
    qDebug() << "New error node created. Total nodes: " << totalNodes << " total errors: " << totalErrors;
}

//free memory allocated to error and event linked lists
void Events::freeLinkedLists()
{
    // Free memory for the event linked list
    //get ptr to head node
    EventNode *wkgPtr = headEventNode;
    EventNode *wkgPtr2;

    //loop through the list
    while (wkgPtr != nullptr)
    {
        //save location of next node
        wkgPtr2 = wkgPtr->nextPtr;

        //delete current node
        delete wkgPtr;

        //iterate to next node
        wkgPtr = wkgPtr2;
    }
    //ensure head and tail point to null symbolizing empty list
    headEventNode = lastEventNode = nullptr;

    // Free memory for the error linked list
    wkgPtr = headErrorNode;

    //loop through list
    while (wkgPtr != nullptr)
    {
        //save location of next node
        wkgPtr2 = wkgPtr->nextPtr;

        //delete current node
        delete wkgPtr;

        //iterate to next node
        wkgPtr = wkgPtr2;
    }

    //ensure head and tail point to null symbolizing empty list
    headErrorNode = lastErrorNode = nullptr;
}

//attempts to find the error node with given id, and update its cleared variable to true
//if the error can not be found, return false.
bool Events::clearError(int id)
{
    //init vars
    EventNode *wkgPtr = headErrorNode;

    //loop through error linked list
    while (wkgPtr != nullptr)
    {
        //if id is found update cleared status and return success
        if (wkgPtr->id == id)
        {
            wkgPtr->cleared = true;

            qDebug() << "Error " << id << " cleared";

            return true;
        }

        //iterate to next error node
        wkgPtr = wkgPtr->nextPtr;
    }

    //no id was found and we reached end of list, return failure
    return false;
}

void Events::outputToLogFile(std::string logFileName)
{
    // Open the log file in overwrite mode
    std::ofstream logFile(logFileName, std::ios::out);

    if (logFile.is_open())
    {
        //declare variables
        EventNode *wkgErrPtr = headErrorNode;
        EventNode *wkgEventPtr = headEventNode;
        EventNode *nextPrintPtr;
        bool printErr;

        //get todays date
        QString currentDateString = QDateTime::currentDateTime().date().toString("MM/dd/yyyy");

        //print log file header
        logFile << "====== " << currentDateString.toStdString() << " ======" << std::endl << std::endl;

        //loop through events and errors
        while (wkgErrPtr != nullptr || wkgEventPtr != nullptr)
        {
            //check if both linked lists have valid ptrs to nodes
            if (wkgErrPtr != nullptr && wkgEventPtr != nullptr)
            {
                //check if next event has lower id than next error
                if (wkgErrPtr->id > wkgEventPtr->id)
                {
                    //choose this event to print
                    nextPrintPtr = wkgEventPtr;

                    //set no error flag
                    printErr = false;

                    //get next event
                    wkgEventPtr = wkgEventPtr->nextPtr;
                }
                else
                {
                    //choose this error to print
                    nextPrintPtr = wkgErrPtr;

                    //set err flag
                    printErr = true;

                    //get next error
                    wkgErrPtr = wkgErrPtr->nextPtr;
                }
            }
            //check for only events
            else if (wkgEventPtr != nullptr)
            {
                //choose this event to print
                nextPrintPtr = wkgEventPtr;

                //set no err flag
                printErr = false;

                //get next event
                wkgEventPtr = wkgEventPtr->nextPtr;
            }
            //otherwise there are only errors
            else
            {
                //choose this err to print
                nextPrintPtr = wkgErrPtr;

                //set err flag
                printErr = true;

                //get next err
                wkgErrPtr = wkgErrPtr->nextPtr;
            }

            //print data of chosen node
            logFile << nextPrintPtr->timeStamp.toStdString() << " " << nextPrintPtr->eventString.toStdString();

            //check if chosen node is error node
            if (printErr)
            {
                //print cleared status
                if (nextPrintPtr->cleared)
                {
                    logFile << ", CLEARED";
                }
                else
                {
                    logFile << ", NOT CLEARED";
                }
            }

            //end current line
            logFile << std::endl;
        }

        logFile.close();

        qDebug() << "Log file written successfully";
    }
    else
    {
        //the log file could not be opened
        qDebug() << "Unable to open the log file";
    }
}

//generate message containing data from given event or error, csim will attach an identifier to the
//begining of message to tell ddm if it is an error or event
QString Events::generateNodeMessage(EventNode *event)
{
    /*the message will format data in this order
    int id;
    QString timeStamp;
    QString eventString;
    double param1;
    double param2;
    double param3;
    bool cleared;*/

    QString message = QString::number(event->id) + DELIMETER;

    message += event->timeStamp + DELIMETER;

    message += event->eventString + DELIMETER;

    message += QString::number(event->cleared) + DELIMETER;

    return message + '\n';

}

//given the head to either the error linked list or the event linked list, a message will be created containing
//data from all nodes in the list (data dump is used when the controller has been running prior to the ddm being connected
//the data dump will catch the ddm up to any events and errors that have occured before connection)
QString Events::generateDataDump(EventNode *headPtr)
{
    QString message = "";
    EventNode *wkgPtr = headPtr;

    while (wkgPtr != nullptr)
    {
        message += QString::number(wkgPtr->id) + DELIMETER;

        message += wkgPtr->timeStamp + DELIMETER;

        message += wkgPtr->eventString + DELIMETER;

        message += QString::number(wkgPtr->cleared) + DELIMETER;

        wkgPtr = wkgPtr->nextPtr;
    }

    //add new line for data parsing and return
    return message + '\n';
}


//function developed to handle reading a node message and create a new error node with given data
void Events::loadErrorData(QString message)
{
    //locate first comma
    int commaPosition = message.indexOf(DELIMETER);

    //extract id
    int id = message.left(commaPosition).toInt();

    //cut message
    message = message.right(commaPosition);



    //get comma pos
    commaPosition = message.indexOf(DELIMETER);

    //extract time stamp
    QString timeStamp = message.left(commaPosition);

    //cut message
    message = message.right(commaPosition);



    //get comma pos
    commaPosition = message.indexOf(DELIMETER);

    //extract event string
    QString eventString = message.left(commaPosition);

    //cut message
    message = message.right(commaPosition);



    //get comma pos
    commaPosition = message.indexOf(DELIMETER);

    //extract cleared status
    bool cleared = (message.left(commaPosition) == "1");


    //using extracted data, add an error to the end of the error linked list
    addError(id, timeStamp, eventString, cleared);
}

//function developed to handle reading a node message and create a new event node with given data
void Events::loadEventData(QString message)
{
    //locate first comma
    int commaPosition = message.indexOf(DELIMETER);

    //extract id
    int id = message.left(commaPosition).toInt();

    //cut message
    message = message.right(commaPosition);



    //get comma pos
    commaPosition = message.indexOf(DELIMETER);

    //extract time stamp
    QString timeStamp = message.left(commaPosition);

    //cut message
    message = message.right(commaPosition);



    //get comma pos
    commaPosition = message.indexOf(DELIMETER);

    //extract event string
    QString eventString = message.left(commaPosition);

    //cut message
    message = message.right(commaPosition);


    //using extracted data add a new event to the end of the events linked list
    addEvent( id, timeStamp, eventString);
}

void Events::loadErrorDump(QString message)
{
    // Split the dump messages into individual error sets
    QStringList errorSets = message.split(",,", Qt::SkipEmptyParts);

    // Iterate through the error sets and call loadErrorData for each
    for (const QString &errorSet : errorSets)
    {
        // Call loadErrorData for each individual error set
        loadErrorData(errorSet);
    }
}

void Events::loadEventDump(QString message)
{
    // Split the dump messages into individual event sets
    QStringList eventSets = message.split(",,", Qt::SkipEmptyParts);

    // Iterate through the event sets and call loadEventData for each
    for (const QString &eventSet : eventSets)
    {
        // Call loadEventData for each individual event set
        loadEventData(eventSet);
    }
}
